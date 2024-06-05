#include <ntifs.h>
#include "shared.h"

#include "disks.h"
#include "smbios.h"
#include "nic.h"
#include "utils.h"
#include "nvidia.h"
#include <stdio.h>
#include "gpu.hpp"


// If it works, it works. Thank me later - Ausbloke
NTSTATUS convertStringToGUID(const char* strGUID, GUID& newUUID) {
	NTSTATUS status = STATUS_SUCCESS;

	// Fields for the GUID
	unsigned long p0;
	unsigned int p1, p2;
	unsigned int p3, p4, p5, p6, p7, p8, p9, p10;

	// Parse the string into the GUID fields
	int fieldsAssigned = sscanf_s(strGUID, "%8lx-%4hx-%4hx-%2hx%2hx-%2hx%2hx%2hx%2hx%2hx%2hx",
		&p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10);

	if (fieldsAssigned == 11) {
		newUUID.Data1 = _byteswap_ulong(p0);
		newUUID.Data2 = _byteswap_ushort((USHORT)p1);
		newUUID.Data3 = _byteswap_ushort((USHORT)p2);
		newUUID.Data4[0] = (UCHAR)p3;
		newUUID.Data4[1] = (UCHAR)p4;
		newUUID.Data4[2] = (UCHAR)p5;
		newUUID.Data4[3] = (UCHAR)p6;
		newUUID.Data4[4] = (UCHAR)p7;
		newUUID.Data4[5] = (UCHAR)p8;
		newUUID.Data4[6] = (UCHAR)p9;
		newUUID.Data4[7] = (UCHAR)p10;
	}
	else {
		status = STATUS_INVALID_PARAMETER;
	}

	return status;
}




//VOID EntryPoint() {
//
//
//	Utils::SetSEED();
//
//	Disks::DisableSmart();
//	Disks::ChangeDiskSerials();
//	//Disks::ChangeVolumeID();
//	Smbios::ChangeSmbiosSerials();
//	Smbios::ChangeBootInformation();
//	//Smbios::SpoofSMBIOS();
//	NIC::SpoofStatic();
//
//	const char* strGUID = "00000000-0000-0000-0000-000000000000";
//	GUID newUUID;
//	convertStringToGUID(strGUID, newUUID);
//
//	uintptr_t result = n_gpu::handleNVIDIAUUID(newUUID);
//
//}
//
//
//
//extern "C" {
//	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
//
//	NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(PEPROCESS Process);
//}
//
//NTSTATUS unsupported_io(PDEVICE_OBJECT device_obj, PIRP irp) {
//	irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
//	IoCompleteRequest(irp, IO_NO_INCREMENT);
//	return irp->IoStatus.Status;
//}
//
//NTSTATUS create_io(PDEVICE_OBJECT device_obj, PIRP irp) {
//	UNREFERENCED_PARAMETER(device_obj);
//
//	IoCompleteRequest(irp, IO_NO_INCREMENT);
//	return irp->IoStatus.Status;
//}
//
//NTSTATUS close_io(PDEVICE_OBJECT device_obj, PIRP irp) {
//	UNREFERENCED_PARAMETER(device_obj);
//	IoCompleteRequest(irp, IO_NO_INCREMENT);
//	return irp->IoStatus.Status;
//}
//
//#define ctl_spoof    CTL_CODE(FILE_DEVICE_UNKNOWN, 1104, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
//
//NTSTATUS ioctl_spoofer(PDEVICE_OBJECT device_obj, PIRP irp) {
//	irp->IoStatus.Status = STATUS_SUCCESS;
//	irp->IoStatus.Information = sizeof(NULL);
//
//	auto stack = IoGetCurrentIrpStackLocation(irp);
//
//	size_t size = 0;
//
//	if (stack) {
//
//		if (stack->Parameters.DeviceIoControl.IoControlCode == ctl_spoof) {
//			EntryPoint();
//		}
//
//	}
//
//	IoCompleteRequest(irp, IO_NO_INCREMENT);
//
//	return irp->IoStatus.Status;
//}
//NTSTATUS HookedDriver(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registery_path) {
//	auto  status = STATUS_SUCCESS;
//	UNICODE_STRING  sym_link, dev_name;
//	PDEVICE_OBJECT  dev_obj;
//
//	RtlInitUnicodeString(&dev_name, L"\\Device\\nvidia");
//	status = IoCreateDevice(driver_obj, 0, &dev_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &dev_obj);
//
//	if (status != STATUS_SUCCESS) {
//		return status;
//	}
//
//	RtlInitUnicodeString(&sym_link, L"\\DosDevices\\nvidia");
//	status = IoCreateSymbolicLink(&sym_link, &dev_name);
//
//	if (status != STATUS_SUCCESS) {
//		return status;
//	}
//
//	dev_obj->Flags |= DO_BUFFERED_IO;
//
//	for (int t = 0; t <= IRP_MJ_MAXIMUM_FUNCTION; t++)
//		driver_obj->MajorFunction[t] = unsupported_io;
//
//	driver_obj->MajorFunction[IRP_MJ_CREATE] = create_io;
//	driver_obj->MajorFunction[IRP_MJ_CLOSE] = close_io;
//	driver_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ioctl_spoofer;
//	driver_obj->DriverUnload = NULL;
//
//	dev_obj->Flags &= ~DO_DEVICE_INITIALIZING;
//
//	return status;
//}

NTSTATUS DriverEntry(PDRIVER_OBJECT object, PUNICODE_STRING registry)
{
	UNREFERENCED_PARAMETER(object);
	UNREFERENCED_PARAMETER(registry);

	Utils::SetSEED();

	Disks::DisableSmart();
	Disks::ChangeDiskSerials();
	//Disks::ChangeVolumeID();
	Smbios::ChangeSmbiosSerials();
	Smbios::ChangeBootInformation();
	//Smbios::SpoofSMBIOS();
	NIC::SpoofStatic();

	const char* strGUID = "00000000-0000-0000-0000-000000000000";
	GUID newUUID;
	convertStringToGUID(strGUID, newUUID);

	uintptr_t result = n_gpu::handleNVIDIAUUID(newUUID);

	/*auto status = STATUS_SUCCESS;
	UNICODE_STRING handle;

	RtlInitUnicodeString(&handle, L"\\Driver\\nvidia");
	status = IoCreateDriver(&handle, &HookedDriver);*/


	return STATUS_SUCCESS;
}





