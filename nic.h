#pragma once

#include <ntddndis.h>

typedef struct _NDIS_IF_BLOCK {
	char _padding_0[0x464];
	IF_PHYSICAL_ADDRESS_LH ifPhysAddress; // 0x464
	IF_PHYSICAL_ADDRESS_LH PermanentPhysAddress; // 0x486
} NDIS_IF_BLOCK, * PNDIS_IF_BLOCK;

typedef struct _KSTRING {
	char _padding_0[0x10];
	WCHAR Buffer[1]; // 0x10 at least
} KSTRING, * PKSTRING;

// dt ndis!_NDIS_FILTER_BLOCK
typedef struct _NDIS_FILTER_BLOCK {
	char _padding_0[0x8];
	struct _NDIS_FILTER_BLOCK* NextFilter; // 0x8
	char _padding_1[0x18];
	PKSTRING FilterInstanceName; // 0x28
} NDIS_FILTER_BLOCK, * PNDIS_FILTER_BLOCK;

namespace NIC
{
	void SpoofStatic();
}