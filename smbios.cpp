#include <ntifs.h>
#include "utils.h"
#include "shared.h"
#include "smbios.h"

#define printf(Message, ...) DbgPrintEx(0, 0, Message, __VA_ARGS__)

char* Smbios::GetString(SMBIOS_HEADER* header, SMBIOS_STRING string)
{
	const auto* start = reinterpret_cast<const char*>(header) + header->Length;

	if (!string || *start == 0)
		return nullptr;

	while (--string)
	{
		start += strlen(start) + 1;
	}

	return const_cast<char*>(start);
}

void Smbios::RandomizeString(char* string)
{
	const auto length = static_cast<int>(strlen(string));

	auto* buffer = static_cast<char*>(ExAllocatePoolWithTag(NonPagedPool, length, POOL_TAG));
	Utils::RandomText(buffer, length);
	buffer[length] = '\0';

	memcpy(string, buffer, length);

	ExFreePool(buffer);
}

NTSTATUS Smbios::ProcessTable(SMBIOS_HEADER* header)
{
	if (!header->Length)
		return STATUS_UNSUCCESSFUL;

	if (header->Type == 0)
	{
		auto* type0 = reinterpret_cast<SMBIOS_TYPE0*>(header);

		auto* vendor = GetString(header, type0->Vendor);
		RandomizeString(vendor);
	}

	if (header->Type == 1)
	{
		auto* type1 = reinterpret_cast<SMBIOS_TYPE1*>(header);

		auto* manufacturer = GetString(header, type1->Manufacturer);
		RandomizeString(manufacturer);

		auto* productName = GetString(header, type1->ProductName);
		RandomizeString(productName);

		auto* serialNumber = GetString(header, type1->SerialNumber);
		RandomizeString(serialNumber);
	}

	if (header->Type == 2)
	{
		auto* type2 = reinterpret_cast<SMBIOS_TYPE2*>(header);

		auto* manufacturer = GetString(header, type2->Manufacturer);
		RandomizeString(manufacturer);

		auto* productName = GetString(header, type2->ProductName);
		RandomizeString(productName);

		auto* serialNumber = GetString(header, type2->SerialNumber);
		RandomizeString(serialNumber);
	}

	if (header->Type == 3)
	{
		auto* type3 = reinterpret_cast<SMBIOS_TYPE3*>(header);

		auto* manufacturer = GetString(header, type3->Manufacturer);
		RandomizeString(manufacturer);

		auto* serialNumber = GetString(header, type3->SerialNumber);
		RandomizeString(serialNumber);
	}

	return STATUS_SUCCESS;
}


NTSTATUS Smbios::LoopTables(void* mapped, ULONG size)
{
	auto* endAddress = static_cast<char*>(mapped) + size;
	while (true)
	{
		auto* header = static_cast<SMBIOS_HEADER*>(mapped);
		if (header->Type == 127 && header->Length == 4)
			break;
		
		ProcessTable(header);
		auto* end = static_cast<char*>(mapped) + header->Length;
		while (0 != (*end | *(end + 1))) end++;
		end += 2;
		if (end >= endAddress)
			break;	

		mapped = end;
	}
	
	return STATUS_SUCCESS;
}

NTSTATUS Smbios::ChangeSmbiosSerials()
{
	auto* base = Utils::GetModuleBase("ntoskrnl.exe");
	if (!base)
	{
		return STATUS_UNSUCCESSFUL;
	}

	auto* physicalAddress = static_cast<PPHYSICAL_ADDRESS>(Utils::FindPatternImage(base, "\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x8B\x15", "xxx????xxxx?xx")); // WmipFindSMBiosStructure -> WmipSMBiosTablePhysicalAddress
	if (!physicalAddress)
	{
		return STATUS_UNSUCCESSFUL;
	}

	physicalAddress = reinterpret_cast<PPHYSICAL_ADDRESS>(reinterpret_cast<char*>(physicalAddress) + 7 + *reinterpret_cast<int*>(reinterpret_cast<char*>(physicalAddress) + 3));
	if (!physicalAddress)
	{
		return STATUS_UNSUCCESSFUL;
	}

	auto* sizeScan = Utils::FindPatternImage(base, "\x8B\x1D\x00\x00\x00\x00\x48\x8B\xD0\x44\x8B\xC3\x48\x8B\xCD\xE8\x00\x00\x00\x00\x8B\xD3\x48\x8B", "xx????xxxxxxxxxx????xxxx");  // WmipFindSMBiosStructure -> WmipSMBiosTableLength
	if (!sizeScan)
	{
		return STATUS_UNSUCCESSFUL;
	}

	const auto size = *reinterpret_cast<ULONG*>(static_cast<char*>(sizeScan) + 6 + *reinterpret_cast<int*>(static_cast<char*>(sizeScan) + 2));
	if (!size)
	{
		return STATUS_UNSUCCESSFUL;
	}

	auto* mapped = MmMapIoSpace(*physicalAddress, size, MmNonCached);
	if (!mapped)
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	LoopTables(mapped, size);
	
	MmUnmapIoSpace(mapped, size);
	
	return STATUS_SUCCESS;
}

NTSTATUS Smbios::ChangeBootInformation()
{
	PVOID base = Utils::GetModuleBase("ntoskrnl.exe");
	if (!base)
		return STATUS_UNSUCCESSFUL;

	// ExpBootEnvironmentInformation

	unsigned char* boot_information = (unsigned char*)Utils::FindPatternImage(base, "\x0F\x10\x05\x00\x00\x00\x00\x0F\x11\x00\x8B", "xxx????xx?x");
	if (!boot_information)
		return STATUS_UNSUCCESSFUL;

	boot_information = boot_information + 7 + *(int*)(boot_information + 3);
	if (!boot_information)
		return STATUS_UNSUCCESSFUL;

	//for (int i = 0; i < 16; i++)
	//	LOG("%X", boot_information[i]);

	Utils::SpoofBuffer(Utils::SEED, boot_information, 16);
	return STATUS_SUCCESS;
}

void Smbios::SpoofSMBIOS() 
{
	PVOID base = Utils::GetModuleBase("ntoskrnl.exe");
	//if (!base) {
	//	//printf("! failed to get \"ntoskrnl.exe\" !\n");
	//	return;
	//}

	PBYTE ExpBootEnvironmentInformation = (PBYTE)Utils::FindPatternImage(base, "\x0F\x10\x05\x00\x00\x00\x00\x0F\x11\x00\x8B", "xxx????xx?x");
	if (ExpBootEnvironmentInformation) {
		ExpBootEnvironmentInformation = ExpBootEnvironmentInformation + 7 + *(PINT)(ExpBootEnvironmentInformation + 3);
		Utils::SpoofBuffer(Utils::SEED, ExpBootEnvironmentInformation, 16);

		//printf("handled ExpBootEnvironmentInformation\n");
	}
	//else {
	//	//printf("! ExpBootEnvironmentInformation not found !\n");
	//}

	PPHYSICAL_ADDRESS WmipSMBiosTablePhysicalAddress = (PPHYSICAL_ADDRESS)Utils::FindPatternImage(base, "\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x8B\x15", "xxx????xxxx?xx");
	if (WmipSMBiosTablePhysicalAddress) {
		WmipSMBiosTablePhysicalAddress = (PPHYSICAL_ADDRESS)((PBYTE)WmipSMBiosTablePhysicalAddress + 7 + *(PINT)((PBYTE)WmipSMBiosTablePhysicalAddress + 3));
		memset(WmipSMBiosTablePhysicalAddress, 0, sizeof(PHYSICAL_ADDRESS));

		//printf("nulled SMBIOS table physical address\n");
	}
	//else {
	//	//printf("! WmipSMBiosTablePhysicalAddress not found !\n");
	//}
}