#include <ntifs.h>
#include "utils.h"
#include "shared.h"
#include "nic.h"

void NIC::SpoofStatic()
{
	PVOID base = Utils::GetModuleBase("ndis.sys");
	if (!base)
		return;

	PNDIS_FILTER_BLOCK ndisGlobalFilterList = (PNDIS_FILTER_BLOCK)Utils::FindPatternImage(base, "\x40\x8A\xF0\x48\x8B\x05", "xxxxxx");
	if (!ndisGlobalFilterList)
		return;

	unsigned long* ndisFilter_IfBlock = (unsigned long*)Utils::FindPatternImage(base, "\x48\x85\x00\x0F\x84\x00\x00\x00\x00\x00\x8B\x00\x00\x00\x00\x00\x33", "xx?xx?????x???xxx");
	if (!ndisFilter_IfBlock)
		return;

	unsigned long ndisFilter_IfBlock_offset = *(unsigned long*)((unsigned char*)ndisFilter_IfBlock + 0xC);

	ndisGlobalFilterList = (PNDIS_FILTER_BLOCK)((unsigned char*)ndisGlobalFilterList + 0x3);
	if (!ndisGlobalFilterList)
		return;

	ndisGlobalFilterList = *(PNDIS_FILTER_BLOCK*)((unsigned char*)ndisGlobalFilterList + 0x7 + *(int*)((unsigned char*)ndisGlobalFilterList + 0x3));
	if (!ndisGlobalFilterList)
		return;

	for (PNDIS_FILTER_BLOCK filter = ndisGlobalFilterList; filter; filter = filter->NextFilter) 
	{
		PNDIS_IF_BLOCK block = *(PNDIS_IF_BLOCK*)((unsigned char*)filter + ndisFilter_IfBlock_offset);
		if (!block)
			continue;

		// Current MAC
		PIF_PHYSICAL_ADDRESS_LH addr = &block->ifPhysAddress;
		Utils::SpoofBuffer(Utils::SEED, addr->Address, addr->Length);

		addr = &block->PermanentPhysAddress;
		Utils::SpoofBuffer(Utils::SEED, addr->Address, addr->Length);
	}
}
