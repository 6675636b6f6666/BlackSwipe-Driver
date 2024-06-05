#include <ntifs.h>
#include <ntstrsafe.h>
#include "utils.h"
#include "shared.h"
#include "disks.h"
//#include "stdafx.h"

void Disks::DisableSmartBit(PRAID_UNIT_EXTENSION extension)
{
	extension->_Smart.Telemetry.SmartMask = 0;
}

PDEVICE_OBJECT Disks::GetRaidDevice(const wchar_t* deviceName)
{
	UNICODE_STRING raidPort;
	RtlInitUnicodeString(&raidPort, deviceName);

	PFILE_OBJECT fileObject = nullptr;
	PDEVICE_OBJECT deviceObject = nullptr;
	auto status = IoGetDeviceObjectPointer(&raidPort, FILE_READ_DATA, &fileObject, &deviceObject);
	if (!NT_SUCCESS(status))
	{
		return nullptr;
	}

	return deviceObject->DriverObject->DeviceObject; // not sure about this
}

NTSTATUS Disks::DiskLoop(PDEVICE_OBJECT deviceArray, RaidUnitRegisterInterfaces registerInterfaces)
{
	/*
	LOG("SerialNumber: %s\n-> ProductId: %s\n-> VendorId: %s\n-> DeviceGuid: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n-> SpoofedDeviceGuid: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n\n",
				extension2->SerialNumber,
				extension2->ProductId,
				extension2->VendorId,
				original_guid.Data1, original_guid.Data2, original_guid.Data3,
				original_guid.Data4[0], original_guid.Data4[1], original_guid.Data4[2], original_guid.Data4[3],
				original_guid.Data4[4], original_guid.Data4[5], original_guid.Data4[6], original_guid.Data4[7],

				spoofed_guid.Data1, spoofed_guid.Data2, spoofed_guid.Data3,
				spoofed_guid.Data4[0], spoofed_guid.Data4[1], spoofed_guid.Data4[2], spoofed_guid.Data4[3],
				spoofed_guid.Data4[4], spoofed_guid.Data4[5], spoofed_guid.Data4[6], spoofed_guid.Data4[7]
			);
	*/

	auto status = STATUS_NOT_FOUND;
	
	while (deviceArray->NextDevice)
	{
		if (deviceArray->DeviceType == FILE_DEVICE_DISK)
		{
			auto* extension = static_cast<PRAID_UNIT_EXTENSION>(deviceArray->DeviceExtension);
			if (!extension)
				continue;
		
			auto* extension2 = static_cast<PRAID_UNIT_EXTENSION_2>(deviceArray->DeviceExtension);
			if (!extension2)
				continue;

			/*** SPOOF EXTENSION2 ***/
			Utils::SpoofGUID(&extension2->DeviceGuid);
			Utils::SpoofIdentifier<UCHAR, 256>(extension2->SerialNumber, true);			
			Utils::SpoofIdentifier<UCHAR, 17>(extension2->ProductId, true);
			Utils::SpoofIdentifier<UCHAR, 9>(extension2->VendorId, true); 
			
			const auto length = strlen(extension->_Identity.Identity.SerialNumber.Buffer);
			//"VMWare NVME_0000"

			// Get the original serial number
			auto* original = static_cast<char*>(ExAllocatePool(NonPagedPool, length));
			strcpy(original, extension->_Identity.Identity.SerialNumber.Buffer);
			original[length] = '\0';

			// Copy the original serial, and make a spoofed version of it by slighlty changing it
			auto* spoofed_serial = static_cast<char*>(ExAllocatePoolWithTag(NonPagedPool, length, POOL_TAG));
			strcpy(spoofed_serial, original);
			spoofed_serial[length] = '\0';

			// Spoof the characters to fake values
			Utils::RandomChars(spoofed_serial, length);
			RtlInitString(&extension->_Identity.Identity.SerialNumber, spoofed_serial);

			// Free allocated strings
			ExFreePool(original);
			ExFreePoolWithTag(spoofed_serial, POOL_TAG);	

			status = STATUS_SUCCESS;

			DisableSmartBit(extension);
			registerInterfaces(extension);
		}

		deviceArray = deviceArray->NextDevice;
	}

	return status;
}

extern "C" POBJECT_TYPE * IoDriverObjectType;

NTSTATUS Disks::ChangeDiskSerials()
{
	auto* base = Utils::GetModuleBase("storport.sys");
	if (!base)
	{
		return STATUS_UNSUCCESSFUL;
	}

	const auto registerInterfaces = static_cast<RaidUnitRegisterInterfaces>(Utils::FindPatternImage(base, "\x48\x89\x5C\x24\x00\x55\x56\x57\x48\x83\xEC\x50", "xxxx?xxxxxxx")); // RaidUnitRegisterInterfaces
	if (!registerInterfaces)
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	auto status = STATUS_NOT_FOUND;
	for (auto i = 0; i < 5; i++)
	{
		const auto* raidFormat = L"\\Device\\RaidPort%d";
		wchar_t raidBuffer[18];
		RtlStringCbPrintfW(raidBuffer, 18 * sizeof(wchar_t), raidFormat, i);

		auto* device = GetRaidDevice(raidBuffer);
		if (!device)
			continue;

		const auto loopStatus = DiskLoop(device, registerInterfaces);
		if (NT_SUCCESS(loopStatus))
			status = loopStatus;
	}

	return status;


	/*UNICODE_STRING driverDisk;
	RtlInitUnicodeString(&driverDisk, L"\\Driver\\Disk");

	PDRIVER_OBJECT driverObject;
	auto status = ObReferenceObjectByName(&driverDisk, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, 0, *IoDriverObjectType, KernelMode, nullptr, reinterpret_cast<PVOID*>(&driverObject));
	if (!NT_SUCCESS(status))
		return STATUS_UNSUCCESSFUL;

	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION];

	ObDereferenceObject(driverObject);
	return STATUS_SUCCESS;*/
}

NTSTATUS Disks::DisableSmart()
{
	auto* base = Utils::GetModuleBase("disk.sys");
	if (!base)
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	// Windows 10 Signature:
	auto disableFailurePrediction = static_cast<DiskEnableDisableFailurePrediction>(Utils::FindPatternImage(base, "\x4C\x8B\xDC\x49\x89\x5B\x10\x49\x89\x7B\x18\x55\x49\x8D\x6B\xA1\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x45\x4F", "xxxxxxxxxxxxxxxxxxx????xxx????xxxxxxx")); // DiskEnableDisableFailurePrediction
	if (!disableFailurePrediction)
	{
		// Windows 11 Signature:
		disableFailurePrediction = static_cast<DiskEnableDisableFailurePrediction>(Utils::FindPatternImage(base, "\x48\x89\x5C\x24\x00\x48\x89\x7C\x24\x00\x55", "xxxx?xxxx?x"));
		if (!disableFailurePrediction)
		{
			return STATUS_UNSUCCESSFUL;
		}
	}

	UNICODE_STRING driverDisk;
	RtlInitUnicodeString(&driverDisk, L"\\Driver\\Disk");

	PDRIVER_OBJECT driverObject = nullptr;
	auto status = ObReferenceObjectByName(&driverDisk, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, 0, *IoDriverObjectType, KernelMode, nullptr, reinterpret_cast<PVOID*>(&driverObject));
	if (!NT_SUCCESS(status))
	{
		return STATUS_UNSUCCESSFUL;
	}

	PDEVICE_OBJECT deviceObjectList[64];
	RtlZeroMemory(deviceObjectList, sizeof(deviceObjectList));

	ULONG numberOfDeviceObjects = 0;
	status = IoEnumerateDeviceObjectList(driverObject, deviceObjectList, sizeof(deviceObjectList), &numberOfDeviceObjects);
	if (!NT_SUCCESS(status))
	{
		return STATUS_UNSUCCESSFUL;
	}

	for (ULONG i = 0; i < numberOfDeviceObjects; ++i)
	{
		auto* deviceObject = deviceObjectList[i];
		disableFailurePrediction(deviceObject->DeviceExtension, false);
		ObDereferenceObject(deviceObject);
	}

	ObDereferenceObject(driverObject);
	return STATUS_SUCCESS;
}

//typedef struct _IOC_REQUEST 
//{
//	PVOID Buffer;
//	ULONG BufferLength;
//	PVOID OldContext;
//	PIO_COMPLETION_ROUTINE OldRoutine;
//} IOC_REQUEST, * PIOC_REQUEST;
//
//PDRIVER_DISPATCH DiskControlOriginal = 0, MountControlOriginal = 0, PartControlOriginal = 0, NsiControlOriginal = 0, GpuControlOriginal = 0;
//
//VOID ChangeIoc(PIO_STACK_LOCATION ioc, PIRP irp, PIO_COMPLETION_ROUTINE routine) 
//{
//	PIOC_REQUEST request = (PIOC_REQUEST)ExAllocatePool(NonPagedPool, sizeof(IOC_REQUEST));
//	//if (!request) {
//	//	printf("! failed to allocate IOC_REQUEST !\n");
//	//	return;
//	//}
//
//	request->Buffer = irp->AssociatedIrp.SystemBuffer;
//	request->BufferLength = ioc->Parameters.DeviceIoControl.OutputBufferLength;
//	request->OldContext = ioc->Context;
//	request->OldRoutine = ioc->CompletionRoutine;
//
//	ioc->Control = SL_INVOKE_ON_SUCCESS;
//	ioc->Context = request;
//	ioc->CompletionRoutine = routine;
//}
//
//NTSTATUS MountPointsIoc(PDEVICE_OBJECT device, PIRP irp, PVOID context) 
//{
//	if (context) {
//		IOC_REQUEST request = *(PIOC_REQUEST)context;
//		ExFreePool(context);
//
//		if (request.BufferLength >= sizeof(MOUNTMGR_MOUNT_POINTS)) {
//			PMOUNTMGR_MOUNT_POINTS points = (PMOUNTMGR_MOUNT_POINTS)request.Buffer;
//			for (DWORD i = 0; i < points->NumberOfMountPoints; ++i) {
//				PMOUNTMGR_MOUNT_POINT point = &points->MountPoints[i];
//				if (point->UniqueIdOffset) {
//					point->UniqueIdLength = 0;
//				}
//
//				if (point->SymbolicLinkNameOffset) {
//					point->SymbolicLinkNameLength = 0;
//				}
//			}
//		}
//
//		if (request.OldRoutine && irp->StackCount > 1) {
//			return request.OldRoutine(device, irp, request.OldContext);
//		}
//	}
//
//	return STATUS_SUCCESS;
//}
//
//NTSTATUS MountUniqueIoc(PDEVICE_OBJECT device, PIRP irp, PVOID context) 
//{
//	if (context) {
//		IOC_REQUEST request = *(PIOC_REQUEST)context;
//		ExFreePool(context);
//
//		if (request.BufferLength >= sizeof(MOUNTDEV_UNIQUE_ID)) {
//			((PMOUNTDEV_UNIQUE_ID)request.Buffer)->UniqueIdLength = 0;
//		}
//
//		if (request.OldRoutine && irp->StackCount > 1) {
//			return request.OldRoutine(device, irp, request.OldContext);
//		}
//	}
//
//	return STATUS_SUCCESS;
//}
//
//NTSTATUS MountControl(PDEVICE_OBJECT device, PIRP irp) 
//{
//	PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);
//	switch (ioc->Parameters.DeviceIoControl.IoControlCode) {
//	case IOCTL_MOUNTMGR_QUERY_POINTS:
//		ChangeIoc(ioc, irp, MountPointsIoc);
//		break;
//	case IOCTL_MOUNTDEV_QUERY_UNIQUE_ID:
//		ChangeIoc(ioc, irp, MountUniqueIoc);
//		break;
//	}
//
//	return MountControlOriginal(device, irp);
//}
//
//void ChangeVolumeID() 
//{
//	SwapControl(RTL_CONSTANT_STRING(L"\\Driver\\mountmgr"), MountControl, MountControlOriginal);
//}