#pragma once
#include <windef.h>
#include <ntifs.h>

#define POOL_TAG 'oamL'

#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;}

//
// Custom
// 

typedef struct _LARGE_INTEGER_TIME {
	LARGE_INTEGER counter1;
	LARGE_INTEGER counter2;
} LARGE_INTEGER_TIME, * PLARGE_INTEGER_TIME;


// ------------------------------------------------
// ntoskrnl.exe
// ------------------------------------------------

typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemInformationClassMin = 0,
	SystemBasicInformation = 0,
	SystemProcessorInformation = 1,
	SystemPerformanceInformation = 2,
	SystemTimeOfDayInformation = 3,
	SystemPathInformation = 4,
	SystemNotImplemented1 = 4,
	SystemProcessInformation = 5,
	SystemProcessesAndThreadsInformation = 5,
	SystemCallCountInfoInformation = 6,
	SystemCallCounts = 6,
	SystemDeviceInformation = 7,
	SystemConfigurationInformation = 7,
	SystemProcessorPerformanceInformation = 8,
	SystemProcessorTimes = 8,
	SystemFlagsInformation = 9,
	SystemGlobalFlag = 9,
	SystemCallTimeInformation = 10,
	SystemNotImplemented2 = 10,
	SystemModuleInformation = 11,
	SystemLocksInformation = 12,
	SystemLockInformation = 12,
	SystemStackTraceInformation = 13,
	SystemNotImplemented3 = 13,
	SystemPagedPoolInformation = 14,
	SystemNotImplemented4 = 14,
	SystemNonPagedPoolInformation = 15,
	SystemNotImplemented5 = 15,
	SystemHandleInformation = 16,
	SystemObjectInformation = 17,
	SystemPageFileInformation = 18,
	SystemPagefileInformation = 18,
	SystemVdmInstemulInformation = 19,
	SystemInstructionEmulationCounts = 19,
	SystemVdmBopInformation = 20,
	SystemInvalidInfoClass1 = 20,
	SystemFileCacheInformation = 21,
	SystemCacheInformation = 21,
	SystemPoolTagInformation = 22,
	SystemInterruptInformation = 23,
	SystemProcessorStatistics = 23,
	SystemDpcBehaviourInformation = 24,
	SystemDpcInformation = 24,
	SystemFullMemoryInformation = 25,
	SystemNotImplemented6 = 25,
	SystemLoadImage = 26,
	SystemUnloadImage = 27,
	SystemTimeAdjustmentInformation = 28,
	SystemTimeAdjustment = 28,
	SystemSummaryMemoryInformation = 29,
	SystemNotImplemented7 = 29,
	SystemNextEventIdInformation = 30,
	SystemNotImplemented8 = 30,
	SystemEventIdsInformation = 31,
	SystemNotImplemented9 = 31,
	SystemCrashDumpInformation = 32,
	SystemExceptionInformation = 33,
	SystemCrashDumpStateInformation = 34,
	SystemKernelDebuggerInformation = 35,
	SystemContextSwitchInformation = 36,
	SystemRegistryQuotaInformation = 37,
	SystemLoadAndCallImage = 38,
	SystemPrioritySeparation = 39,
	SystemPlugPlayBusInformation = 40,
	SystemNotImplemented10 = 40,
	SystemDockInformation = 41,
	SystemNotImplemented11 = 41,
	SystemInvalidInfoClass2 = 42,
	SystemProcessorSpeedInformation = 43,
	SystemInvalidInfoClass3 = 43,
	SystemCurrentTimeZoneInformation = 44,
	SystemTimeZoneInformation = 44,
	SystemLookasideInformation = 45,
	SystemSetTimeSlipEvent = 46,
	SystemCreateSession = 47,
	SystemDeleteSession = 48,
	SystemInvalidInfoClass4 = 49,
	SystemRangeStartInformation = 50,
	SystemVerifierInformation = 51,
	SystemAddVerifier = 52,
	SystemSessionProcessesInformation = 53,
	SystemInformationClassMax
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_MODULE
{
	ULONG_PTR Reserved[2];
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT Index;
	USHORT Unknown;
	USHORT LoadCount;
	USHORT ModuleNameOffset;
	CHAR ImageName[256];
} SYSTEM_MODULE, * PSYSTEM_MODULE;

typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG_PTR ulModuleCount;
	SYSTEM_MODULE Modules[1];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

extern "C"
{
	NTSTATUS ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS systemInformationClass, PVOID systemInformation, ULONG systemInformationLength, PULONG returnLength);
	NTSTATUS ObReferenceObjectByName(PUNICODE_STRING objectName, ULONG attributes, PACCESS_STATE accessState, ACCESS_MASK desiredAccess, POBJECT_TYPE objectType, KPROCESSOR_MODE accessMode, PVOID parseContext, PVOID* object);
}

// ------------------------------------------------
// storport.sys
// ------------------------------------------------

typedef struct _STOR_SCSI_IDENTITY
{
	char Space[0x8]; // +0x008 SerialNumber     : _STRING
	STRING SerialNumber;
} STOR_SCSI_IDENTITY, *PSTOR_SCSI_IDENTITY;

typedef struct _TELEMETRY_UNIT_EXTENSION
{
	/*
		+0x000 Flags            : <anonymous-tag>
		+0x000 DeviceHealthEventsLogged : Pos 0, 1 Bit
		+0x000 FailedFirstSMARTCommand : Pos 1, 1 Bit
		+0x000 FailedFirstDeviceStatisticsLogCommand : Pos 2, 1 Bit
		+0x000 FailedFirstNvmeCloudSSDCommand : Pos 3, 1 Bit
		+0x000 SmartPredictFailure : Pos 4, 1 Bit
		+0x000 Reserved         : Pos 5, 27 Bits
	 */
	int SmartMask;
} TELEMETRY_UNIT_EXTENSION, * PTELEMETRY_UNIT_EXTENSION;

// lkd> dt storport!_RAID_UNIT_EXTENSION -b
typedef struct _RAID_UNIT_EXTENSION
{
	union
	{
		struct
		{
			char Space[0x68]; // +0x068 Identity         : _STOR_SCSI_IDENTITY
			STOR_SCSI_IDENTITY Identity;
		} _Identity;

		struct
		{
			char Space[0x7c8]; // +0x7c8 TelemetryExtension : _TELEMETRY_UNIT_EXTENSION
			TELEMETRY_UNIT_EXTENSION Telemetry;
		} _Smart;
	};
} RAID_UNIT_EXTENSION, *PRAID_UNIT_EXTENSION;

typedef struct _RAID_UNIT_EXTENSION_2
{
	union
	{
		DEFINE_MEMBER_N(unsigned __int8, VendorId[9], 0xA0);
		DEFINE_MEMBER_N(unsigned __int8, ProductId[17], 0xA9);
		DEFINE_MEMBER_N(unsigned __int8, SerialNumber[256], 0xBA);
		DEFINE_MEMBER_N(_GUID, DeviceGuid, 0x7B8);

		//  _GUID DeviceGuid;
	};
} RAID_UNIT_EXTENSION_2, *PRAID_UNIT_EXTENSION_2;

typedef __int64(__fastcall* RaidUnitRegisterInterfaces)(PRAID_UNIT_EXTENSION a1);

// ------------------------------------------------
// disk.sys
// ------------------------------------------------

typedef NTSTATUS(__fastcall* DiskEnableDisableFailurePrediction)(void* a1, bool a2);

// ------------------------------------------------
// SMBIOS / gnu-efi
// ------------------------------------------------

typedef struct
{
	UINT8   Type;
	UINT8   Length;
	UINT8   Handle[2];
} SMBIOS_HEADER;

typedef UINT8   SMBIOS_STRING;

typedef struct
{
	SMBIOS_HEADER   Hdr;
	SMBIOS_STRING   Vendor;
	SMBIOS_STRING   BiosVersion;
	UINT8           BiosSegment[2];
	SMBIOS_STRING   BiosReleaseDate;
	UINT8           BiosSize;
	UINT8           BiosCharacteristics[8];
} SMBIOS_TYPE0;

typedef struct
{
	SMBIOS_HEADER   Hdr;
	SMBIOS_STRING   Manufacturer;
	SMBIOS_STRING   ProductName;
	SMBIOS_STRING   Version;
	SMBIOS_STRING   SerialNumber;

	//
	// always byte copy this data to prevent alignment faults!
	//
	GUID			Uuid; // EFI_GUID == GUID?

	UINT8           WakeUpType;
} SMBIOS_TYPE1;

typedef struct
{
	SMBIOS_HEADER   Hdr;
	SMBIOS_STRING   Manufacturer;
	SMBIOS_STRING   ProductName;
	SMBIOS_STRING   Version;
	SMBIOS_STRING   SerialNumber;
} SMBIOS_TYPE2;

typedef struct
{
	SMBIOS_HEADER   Hdr;
	SMBIOS_STRING   Manufacturer;
	UINT8           Type;
	SMBIOS_STRING   Version;
	SMBIOS_STRING   SerialNumber;
	SMBIOS_STRING   AssetTag;
	UINT8           BootupState;
	UINT8           PowerSupplyState;
	UINT8           ThermalState;
	UINT8           SecurityStatus;
	UINT8           OemDefined[4];
} SMBIOS_TYPE3;

typedef struct
{
	SMBIOS_HEADER   Hdr;
	UINT8           Socket;
	UINT8           ProcessorType;
	UINT8           ProcessorFamily;
	SMBIOS_STRING   ProcessorManufacture;
	UINT8           ProcessorId[8];
	SMBIOS_STRING   ProcessorVersion;
	UINT8           Voltage;
	UINT8           ExternalClock[2];
	UINT8           MaxSpeed[2];
	UINT8           CurrentSpeed[2];
	UINT8           Status;
	UINT8           ProcessorUpgrade;
	UINT8           L1CacheHandle[2];
	UINT8           L2CacheHandle[2];
	UINT8           L3CacheHandle[2];
} SMBIOS_TYPE4;



typedef union
{
	SMBIOS_HEADER* Hdr;
	SMBIOS_TYPE0* Type0;
	SMBIOS_TYPE1* Type1;
	SMBIOS_TYPE2* Type2;
	SMBIOS_TYPE3* Type3;
	SMBIOS_TYPE4* Type4;
	UINT8* Raw;
} SMBIOS_STRUCTURE_POINTER;

typedef struct
{
	UINT8   AnchorString[4];
	UINT8   EntryPointStructureChecksum;
	UINT8   EntryPointLength;
	UINT8   MajorVersion;
	UINT8   MinorVersion;
	UINT16  MaxStructureSize;
	UINT8   EntryPointRevision;
	UINT8   FormattedArea[5];
	UINT8   IntermediateAnchorString[5];
	UINT8   IntermediateChecksum;
	UINT16  TableLength;
	UINT32  TableAddress;
	UINT16  NumberOfSmbiosStructures;
	UINT8   SmbiosBcdRevision;
} SMBIOS_STRUCTURE_TABLE;

typedef struct {
	UINT16    Reserved : 1;
	UINT16    Other : 1;
	UINT16    Unknown : 1;
	UINT16    FastPaged : 1;
	UINT16    StaticColumn : 1;
	UINT16    PseudoStatic : 1;
	UINT16    Rambus : 1;
	UINT16    Synchronous : 1;
	UINT16    Cmos : 1;
	UINT16    Edo : 1;
	UINT16    WindowDram : 1;
	UINT16    CacheDram : 1;
	UINT16    Nonvolatile : 1;
	UINT16    Registered : 1;
	UINT16    Unbuffered : 1;
	UINT16    LrDimm : 1;
} MEMORY_DEVICE_TYPE_DETAIL;


typedef union {
	///
	/// Individual bit fields
	///
	struct {
		UINT16    Reserved : 1; ///< Set to 0.
		UINT16    Other : 1;
		UINT16    Unknown : 1;
		UINT16    VolatileMemory : 1;
		UINT16    ByteAccessiblePersistentMemory : 1;
		UINT16    BlockAccessiblePersistentMemory : 1;
		UINT16    Reserved2 : 10; ///< Set to 0.
	} Bits;
	///
	/// All bit fields as a 16-bit value
	///
	UINT16    Uint16;
} MEMORY_DEVICE_OPERATING_MODE_CAPABILITY;

typedef struct {
	SMBIOS_HEADER   Hdr;
	UINT16                                     MemoryArrayHandle;
	UINT16                                     MemoryErrorInformationHandle;
	UINT16                                     TotalWidth;
	UINT16                                     DataWidth;
	UINT16                                     Size;
	UINT8                                      FormFactor;        ///< The enumeration value from MEMORY_FORM_FACTOR.
	UINT8                                      DeviceSet;
	SMBIOS_STRING                        DeviceLocator;
	SMBIOS_STRING                        BankLocator;
	UINT8                                      MemoryType;        ///< The enumeration value from MEMORY_DEVICE_TYPE.
	MEMORY_DEVICE_TYPE_DETAIL                  TypeDetail;
	UINT16                                     Speed;
	SMBIOS_STRING                        Manufacturer;
	SMBIOS_STRING                        SerialNumber;
	SMBIOS_STRING                        AssetTag;
	SMBIOS_STRING                        PartNumber;
	//
	// Add for smbios 2.6
	//
	UINT8                                      Attributes;
	//
	// Add for smbios 2.7
	//
	UINT32                                     ExtendedSize;
	//
	// Keep using name "ConfiguredMemoryClockSpeed" for compatibility
	// although this field is renamed from "Configured Memory Clock Speed"
	// to "Configured Memory Speed" in smbios 3.2.0.
	//
	UINT16                                     ConfiguredMemoryClockSpeed;
	//
	// Add for smbios 2.8.0
	//
	UINT16                                     MinimumVoltage;
	UINT16                                     MaximumVoltage;
	UINT16                                     ConfiguredVoltage;
	//
	// Add for smbios 3.2.0
	//
	UINT8                                      MemoryTechnology;  ///< The enumeration value from MEMORY_DEVICE_TECHNOLOGY
	MEMORY_DEVICE_OPERATING_MODE_CAPABILITY    MemoryOperatingModeCapability;
	SMBIOS_STRING                        FirmwareVersion;
	UINT16                                     ModuleManufacturerID;
	UINT16                                     ModuleProductID;
	UINT16                                     MemorySubsystemControllerManufacturerID;
	UINT16                                     MemorySubsystemControllerProductID;
	UINT64                                     NonVolatileSize;
	UINT64                                     VolatileSize;
	UINT64                                     CacheSize;
	UINT64                                     LogicalSize;
	//
	// Add for smbios 3.3.0
	//
	UINT32                                     ExtendedSpeed;
	UINT32                                     ExtendedConfiguredMemorySpeed;
} SMBIOS_TABLE_TYPE17;

typedef struct _RAW_SMBIOS
{
	UINT8	Unknown;
	UINT8	MajorVersion;
	UINT8	MinorVersion;
	UINT8	DmiRevision;
	UINT32	Size;
	UINT8*	Entry;
} RAW_SMBIOS;

typedef struct _SMBIOS3_TABLE_HEADER
{
	unsigned __int8 Signature[5];
	unsigned __int8 Checksum;
	unsigned __int8 Length;
	unsigned __int8 MajorVersion;
	unsigned __int8 MinorVersion;
	unsigned __int8 Docrev;
	unsigned __int8 EntryPointRevision;
	unsigned __int8 Reserved;
	unsigned int StructureTableMaximumSize;
	unsigned __int64 StructureTableAddress;
} SMBIOS3_TABLE_HEADER;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;         // Not filled in
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[MAXIMUM_FILENAME_LENGTH];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;
typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;