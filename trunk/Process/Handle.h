#pragma once


#include "pch.h"


//#include <SubAuth.h>
#include <winternl.h>


//////////////////////////////////////////////////////////////////////////////////////////////////


//摘自：\wrk\WindowsResearchKernel-WRK\WRK-v1.2\public\sdk\inc\ntexapi.h
// System Information Classes.
//\Windows Kits\10\Include\10.0.19041.0\um\winternl.h已经有这个定义。
//typedef enum _SYSTEM_INFORMATION_CLASS {
//    SystemBasicInformation,
//    SystemProcessorInformation,             // obsolete...delete
//    SystemPerformanceInformation,
//    SystemTimeOfDayInformation,
//    SystemPathInformation,
//    SystemProcessInformation,
//    SystemCallCountInformation,
//    SystemDeviceInformation,
//    SystemProcessorPerformanceInformation,
//    SystemFlagsInformation,
//    SystemCallTimeInformation,
//    SystemModuleInformation,
//    SystemLocksInformation,
//    SystemStackTraceInformation,
//    SystemPagedPoolInformation,
//    SystemNonPagedPoolInformation,
//    SystemHandleInformation,//ExpGetHandleInformation处理的。
//    SystemObjectInformation,
//    SystemPageFileInformation,
//    SystemVdmInstemulInformation,
//    SystemVdmBopInformation,
//    SystemFileCacheInformation,
//    SystemPoolTagInformation,
//    SystemInterruptInformation,
//    SystemDpcBehaviorInformation,
//    SystemFullMemoryInformation,
//    SystemLoadGdiDriverInformation,
//    SystemUnloadGdiDriverInformation,
//    SystemTimeAdjustmentInformation,
//    SystemSummaryMemoryInformation,
//    SystemMirrorMemoryInformation,
//    SystemPerformanceTraceInformation,
//    SystemObsolete0,
//    SystemExceptionInformation,
//    SystemCrashDumpStateInformation,
//    SystemKernelDebuggerInformation,
//    SystemContextSwitchInformation,
//    SystemRegistryQuotaInformation,
//    SystemExtendServiceTableInformation,
//    SystemPrioritySeperation,
//    SystemVerifierAddDriverInformation,
//    SystemVerifierRemoveDriverInformation,
//    SystemProcessorIdleInformation,
//    SystemLegacyDriverInformation,
//    SystemCurrentTimeZoneInformation,
//    SystemLookasideInformation,
//    SystemTimeSlipNotification,
//    SystemSessionCreate,
//    SystemSessionDetach,
//    SystemSessionInformation,
//    SystemRangeStartInformation,
//    SystemVerifierInformation,
//    SystemVerifierThunkExtend,
//    SystemSessionProcessInformation,
//    SystemLoadGdiDriverInSystemSpace,
//    SystemNumaProcessorMap,
//    SystemPrefetcherInformation,
//    SystemExtendedProcessInformation,
//    SystemRecommendedSharedDataAlignment,
//    SystemComPlusPackage,
//    SystemNumaAvailableMemory,
//    SystemProcessorPowerInformation,
//    SystemEmulationBasicInformation,
//    SystemEmulationProcessorInformation,
//    SystemExtendedHandleInformation,
//    SystemLostDelayedWriteInformation,
//    SystemBigPoolInformation,
//    SystemSessionPoolTagInformation,
//    SystemSessionMappedViewInformation,
//    SystemHotpatchInformation,
//    SystemObjectSecurityMode,
//    SystemWatchdogTimerHandler,
//    SystemWatchdogTimerInformation,
//    SystemLogicalProcessorInformation,
//    SystemWow64SharedInformation,
//    SystemRegisterFirmwareTableInformationHandler,
//    SystemFirmwareTableInformation,
//    SystemModuleInformationEx,
//    SystemVerifierTriageInformation,
//    SystemSuperfetchInformation,
//    SystemMemoryListInformation,
//    SystemFileCacheInformationEx,
//    MaxSystemInfoClass  // MaxSystemInfoClass should always be the last enum
//} SYSTEM_INFORMATION_CLASS;

#define STATUS_SUCCESS					((NTSTATUS)0x00000000L)
#define STATUS_INFO_LENGTH_MISMATCH		((NTSTATUS)0xc0000004L)

// The NtQuerySystemInformation function and the structures that it returns are internal to the operating system and subject to change from one  release of Windows to another.
// To maintain the compatibility of your application, it is better not to use the function.
typedef NTSTATUS(WINAPI * QUERYSYSTEMINFORMATION)(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

//\Windows Kits\10\Include\10.0.18362.0\km\ntifs.h
//\Windows Kits\10\Include\10.0.19041.0\um\winternl.h
//typedef enum _OBJECT_INFORMATION_CLASS {
//    ObjectBasicInformation = 0,
//    ObjectTypeInformation = 2
//} OBJECT_INFORMATION_CLASS;

typedef NTSTATUS(WINAPI * QueryObject)(
    __in_opt HANDLE  Handle,
    __in OBJECT_INFORMATION_CLASS  ObjectInformationClass,
    __out_bcount_opt(ObjectInformationLength) PVOID  ObjectInformation,
    __in ULONG  ObjectInformationLength,
    __out_opt PULONG  ReturnLength
    );


// Undocumented structure: SYSTEM_HANDLE_INFORMATION
typedef struct _SYSTEM_HANDLE
{
    ULONG ProcessId;
    UCHAR ObjectTypeNumber;
    UCHAR Flags;
    USHORT Handle;
    PVOID Object;
    ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, * PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
    ULONG NumberOfHandles;
    SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;


////摘自：\wrk\WindowsResearchKernel-WRK\WRK-v1.2\public\sdk\inc\ntobapi.h
//typedef enum _OBJECT_INFORMATION_CLASS {
//    ObjectBasicInformation,
//    ObjectNameInformation,
//    ObjectTypeInformation,
//    ObjectTypesInformation,
//    ObjectHandleFlagInformation,
//    ObjectSessionInformation,
//    MaxObjectInfoClass  // MaxObjectInfoClass should always be the last enum
//} OBJECT_INFORMATION_CLASS;

//摘自：\wrk\WindowsResearchKernel-WRK\WRK-v1.2\public\sdk\inc\ntobapi.h
typedef struct _OBJECT_NAME_INFORMATION {               // ntddk wdm nthal
    UNICODE_STRING Name;                                // ntddk wdm nthal
} OBJECT_NAME_INFORMATION, * POBJECT_NAME_INFORMATION;   // ntddk wdm nthal


//////////////////////////////////////////////////////////////////////////////////////////////////


#define BUFFER_SIZE			512

#define DIRECTORY_QUERY 0x0001

#define STATUS_SUCCESS					((NTSTATUS)0x00000000L)
#define STATUS_UNSUCCESSFUL             ((NTSTATUS)0xC0000001L)
#define STATUS_INFO_LENGTH_MISMATCH		((NTSTATUS)0xc0000004L)
#define STATUS_NO_MORE_FILES            ((NTSTATUS)0x80000006L)
#define STATUS_INSUFFICIENT_RESOURCES   ((NTSTATUS)0xC000009AL) 

// Undocumented SYSTEM_INFORMATION_CLASS: SystemHandleInformation
const SYSTEM_INFORMATION_CLASS SystemHandleInformation = (SYSTEM_INFORMATION_CLASS)16;

// The NtQuerySystemInformation function and the structures that it returns are internal to the operating system and subject to change from one release of Windows to another. 
// To maintain the compatibility of your application, it is better not to use the function.
typedef NTSTATUS(WINAPI * QUERYSYSTEMINFORMATION)(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

#define HANDLE_TYPE_FILE				28

// Undocumented structure: SYSTEM_HANDLE_INFORMATION
//typedef struct _SYSTEM_HANDLE
//{
//    ULONG ProcessId;
//    UCHAR ObjectTypeNumber;
//    UCHAR Flags;
//    USHORT Handle;
//    PVOID Object;
//    ACCESS_MASK GrantedAccess;
//} SYSTEM_HANDLE, * PSYSTEM_HANDLE;

//typedef struct _SYSTEM_HANDLE_INFORMATION
//{
//    ULONG NumberOfHandles;
//    SYSTEM_HANDLE Handles[1];
//} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

// Undocumented FILE_INFORMATION_CLASS: FileNameInformation
const FILE_INFORMATION_CLASS FileNameInformation = (FILE_INFORMATION_CLASS)9;

// The NtQueryInformationFile function and the structures that it returns are internal to the operating system and subject to change from one release of Windows to another. 
// To maintain the compatibility of your application, it is better not to use the function.
typedef NTSTATUS(WINAPI * QUERYINFORMATIONFILE)(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

// FILE_NAME_INFORMATION contains name of queried file object.
typedef struct _FILE_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, * PFILE_NAME_INFORMATION;

BOOL GetFileNameFromHandle(HANDLE hFile);


//////////////////////////////////////////////////////////////////////////////////////////////////


//C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\km\ntifs.h
typedef struct _FILE_DIRECTORY_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_DIRECTORY_INFORMATION, * PFILE_DIRECTORY_INFORMATION;


//https://docs.microsoft.com/en-us/windows/win32/devnotes/ntopendirectoryobject
typedef
NTSTATUS (WINAPI * NtOpenDirectoryObject_PFN)(
    _Out_ PHANDLE            DirectoryHandle,
    _In_  ACCESS_MASK        DesiredAccess,
    _In_  POBJECT_ATTRIBUTES ObjectAttributes
);


//https://docs.microsoft.com/en-us/windows/win32/devnotes/ntquerydirectoryobject
typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, * POBJECT_DIRECTORY_INFORMATION;


//https://docs.microsoft.com/en-us/windows/win32/devnotes/ntquerydirectoryobject
typedef
NTSTATUS (WINAPI *QueryDirectoryObject)(
    _In_      HANDLE  DirectoryHandle,
    _Out_opt_ PVOID   Buffer,
    _In_      ULONG   Length,
    _In_      BOOLEAN ReturnSingleEntry,
    _In_      BOOLEAN RestartScan,
    _Inout_   PULONG  Context,
    _Out_opt_ PULONG  ReturnLength
);


//https://docs.microsoft.com/en-us/windows/win32/devnotes/ntopensymboliclinkobject
typedef
NTSTATUS (WINAPI *OpenSymbolicLinkObject)(
    _Out_ PHANDLE            LinkHandle,
    _In_  ACCESS_MASK        DesiredAccess,
    _In_  POBJECT_ATTRIBUTES ObjectAttributes
);


//https://docs.microsoft.com/en-us/windows/win32/devnotes/ntquerysymboliclinkobject
typedef
NTSTATUS (WINAPI *QuerySymbolicLinkObject)(
    _In_      HANDLE          LinkHandle,
    _Inout_   PUNICODE_STRING LinkTarget,
    _Out_opt_ PULONG          ReturnedLength
);


//////////////////////////////////////////////////////////////////////////////////////////////////


extern OpenSymbolicLinkObject NtOpenSymbolicLinkObject;
extern QuerySymbolicLinkObject NtQuerySymbolicLinkObject;
