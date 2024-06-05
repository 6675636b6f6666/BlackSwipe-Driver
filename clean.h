#pragma once

#include <ntdef.h>
#include <ntifs.h>
#include <ntddk.h>
#include <ntimage.h>
#include <windef.h>
#include <ntstrsafe.h>

namespace clean {
	void clean_piddb(UNICODE_STRING DriverName, ULONG timeDateStamp);
}