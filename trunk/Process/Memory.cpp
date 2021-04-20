#include "pch.h"
#include "Memory.h"


#pragma warning(disable:6386) 
#pragma warning(disable:6385) 
#pragma warning(disable:6001)
#pragma warning(disable:28182) 
#pragma warning(disable:6387) 
#pragma warning(disable:26451)
#pragma warning(disable:4477)
#pragma warning(disable:6328)


//////////////////////////////////////////////////////////////////////////////////////////////////


#define MEMORY_REQUESTED 1024*1024 // request a megabyte


BOOL LoggedSetLockPagesPrivilege(HANDLE hProcess, BOOL bEnable)
/*****************************************************************
   LoggedSetLockPagesPrivilege: a function to obtain or release the privilege of locking physical pages.

   Inputs:
       HANDLE hProcess: Handle for the process for which the privilege is needed
       BOOL bEnable: Enable (TRUE) or disable?

   Return value: TRUE indicates success, FALSE failure.
*****************************************************************/
{
    struct {
        DWORD Count;
        LUID_AND_ATTRIBUTES Privilege[1];
    } Info;

    HANDLE Token;
    BOOL Result;

    // Open the token.
    Result = OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &Token);
    if (Result != TRUE) {
        _tprintf(_T("Cannot open process token.\n"));
        return FALSE;
    }

    // Enable or disable?
    Info.Count = 1;
    if (bEnable) {
        Info.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
    } else {
        Info.Privilege[0].Attributes = 0;
    }

    // Get the LUID.
    Result = LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &(Info.Privilege[0].Luid));
    if (Result != TRUE) {
        _tprintf(_T("Cannot get privilege for %s.\n"), SE_LOCK_MEMORY_NAME);
        return FALSE;
    }

    // Adjust the privilege.
    Result = AdjustTokenPrivileges(Token, FALSE, (PTOKEN_PRIVILEGES)&Info, 0, NULL, NULL);
    if (Result != TRUE) {// Check the result.
        _tprintf(_T("Cannot adjust token privileges (%u)\n"), GetLastError());
        return FALSE;
    } else {
        if (GetLastError() != ERROR_SUCCESS) {
            _tprintf(_T("Cannot enable the SE_LOCK_MEMORY_NAME privilege; "));
            _tprintf(_T("please check the local policy.\n"));
            return FALSE;
        }
    }

    CloseHandle(Token);

    return TRUE;
}


EXTERN_C
__declspec(dllexport)
void WINAPI AWE()
/*
AWE Example

The following sample program illustrates the Address Windowing Extensions.

https://docs.microsoft.com/en-us/windows/win32/memory/awe-example
*/
{
    BOOL bResult;                   // generic Boolean value
    ULONG_PTR NumberOfPages;        // number of pages to request
    ULONG_PTR NumberOfPagesInitial; // initial number of pages requested
    ULONG_PTR * aPFNs;               // page info; holds opaque data
    PVOID lpMemReserved;            // AWE window
    SYSTEM_INFO sSysInfo;           // useful system information
    SIZE_T PFNArraySize;               // memory to request for PFN array

    GetSystemInfo(&sSysInfo);  // fill the system information structure

    _tprintf(_T("This computer has page size %d.\n"), sSysInfo.dwPageSize);

    // Calculate the number of pages of memory to request.
    NumberOfPages = MEMORY_REQUESTED / sSysInfo.dwPageSize;
    _tprintf(_T("Requesting %Id pages of memory.\n"), NumberOfPages);

    // Calculate the size of the user PFN array.
    PFNArraySize = NumberOfPages * sizeof(ULONG_PTR);

    _tprintf(_T("Requesting a PFN array of %Id bytes.\n"), PFNArraySize);

    aPFNs = (ULONG_PTR *)HeapAlloc(GetProcessHeap(), 0, PFNArraySize);
    if (aPFNs == NULL) {
        _tprintf(_T("Failed to allocate on heap.\n"));
        return;
    }

    // Enable the privilege.
    if (!LoggedSetLockPagesPrivilege(GetCurrentProcess(), TRUE)) {
        return;
    }

    // Allocate the physical memory.
    NumberOfPagesInitial = NumberOfPages;
    bResult = AllocateUserPhysicalPages(GetCurrentProcess(), &NumberOfPages, aPFNs);
    if (bResult != TRUE) {
        _tprintf(_T("Cannot allocate physical pages (%u)\n"), GetLastError());
        return;
    }

    if (NumberOfPagesInitial != NumberOfPages) {
        _tprintf(_T("Allocated only %p pages.\n"), (void *)NumberOfPages);
        return;
    }

    // Reserve the virtual memory.
    lpMemReserved = VirtualAlloc(NULL,
                                 MEMORY_REQUESTED,
                                 MEM_RESERVE | MEM_PHYSICAL,
                                 PAGE_READWRITE);
    if (lpMemReserved == NULL) {
        _tprintf(_T("Cannot reserve memory.\n"));
        return;
    }

    // Map the physical memory into the window.
    bResult = MapUserPhysicalPages(lpMemReserved, NumberOfPages, aPFNs);
    if (bResult != TRUE) {
        _tprintf(_T("MapUserPhysicalPages failed (%u)\n"), GetLastError());
        return;
    }

    // unmap
    bResult = MapUserPhysicalPages(lpMemReserved, NumberOfPages, NULL);
    if (bResult != TRUE) {
        _tprintf(_T("MapUserPhysicalPages failed (%u)\n"), GetLastError());
        return;
    }

    // Free the physical pages.
    bResult = FreeUserPhysicalPages(GetCurrentProcess(), &NumberOfPages, aPFNs);
    if (bResult != TRUE) {
        _tprintf(_T("Cannot free physical pages, error %u.\n"), GetLastError());
        return;
    }

    // Free virtual memory.
    bResult = VirtualFree(lpMemReserved, 0, MEM_RELEASE);

    // Release the aPFNs array.
    bResult = HeapFree(GetProcessHeap(), 0, aPFNs);
    if (bResult != TRUE) {
        _tprintf(_T("Call to HeapFree has failed (%u)\n"), GetLastError());
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////


//#define _WIN32_WINNT 0x0600

SIZE_T AllocationSize;
DWORD  PageSize;

void DumpNumaNodeInfo(PVOID Buffer, SIZE_T Size);


EXTERN_C
__declspec(dllexport)
void WINAPI NUMA(int argc, const wchar_t * argv[])
/*
Allocating Memory from a NUMA Node
05/31/2018

The following sample code demonstrates the use of the NUMA functions GetNumaHighestNodeNumber, GetNumaProcessorNode, and VirtualAllocExNuma.
It also demonstrates the use of the QueryWorkingSetEx function to retrieve the NUMA node on which pages are allocated.

https://docs.microsoft.com/en-us/windows/win32/memory/allocating-memory-from-a-numa-node
*/
{
    ULONG HighestNodeNumber;
    ULONG NumberOfProcessors;
    PVOID * Buffers = NULL;

    if (argc > 1) {
        (void)swscanf_s(argv[1], L"%Ix", &AllocationSize);
    }

    if (AllocationSize == 0) {
        AllocationSize = 16 * 1024 * 1024;
    }

    // Get the number of processors and system page size.
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    NumberOfProcessors = SystemInfo.dwNumberOfProcessors;
    PageSize = SystemInfo.dwPageSize;

    // Get the highest node number.
    if (!GetNumaHighestNodeNumber(&HighestNodeNumber)) {
        _tprintf(_T("GetNumaHighestNodeNumber failed: %d\n"), GetLastError());
        goto Exit;
    }

    if (HighestNodeNumber == 0) {
        _putts(_T("Not a NUMA system - exiting"));
        goto Exit;
    }

    // Allocate array of pointers to memory blocks.
    Buffers = (PVOID *)malloc(sizeof(PVOID) * NumberOfProcessors);
    if (Buffers == NULL) {
        _putts(_T("Allocating array of buffers failed"));
        goto Exit;
    }

    ZeroMemory(Buffers, sizeof(PVOID) * NumberOfProcessors);

    // For each processor, get its associated NUMA node and allocate some memory from it.
    for (UCHAR i = 0; i < NumberOfProcessors; i++) {
        UCHAR NodeNumber;

        if (!GetNumaProcessorNode(i, &NodeNumber)) {
            _tprintf(_T("GetNumaProcessorNode failed: %d\n"), GetLastError());
            goto Exit;
        }

        _tprintf(_T("CPU %u: node %u\n"), (ULONG)i, NodeNumber);

        PCHAR Buffer = (PCHAR)VirtualAllocExNuma(
            GetCurrentProcess(),
            NULL,
            AllocationSize,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_READWRITE,
            NodeNumber
        );
        if (Buffer == NULL) {
            _tprintf(_T("VirtualAllocExNuma failed: %d, node %u\n"), GetLastError(), NodeNumber);
            goto Exit;
        }

        PCHAR BufferEnd = Buffer + AllocationSize - 1;
        SIZE_T NumPages = ((SIZE_T)BufferEnd) / PageSize - ((SIZE_T)Buffer) / PageSize + 1;

        _putts(_T("Allocated virtual memory:"));
        _tprintf(_T("%p - %p (%6Iu pages), preferred node %u\n"), Buffer, BufferEnd, NumPages, NodeNumber);

        Buffers[i] = Buffer;

        // At this point, virtual pages are allocated but no valid physical pages are associated with them yet.
        //
        // The FillMemory call below will touch every page in the buffer, faulting
        // them into our working set. When this happens physical pages will be allocated
        // from the preferred node we specified in VirtualAllocExNuma, or any node
        // if the preferred one is out of pages.

        FillMemory(Buffer, AllocationSize, 'x');

        // Check the actual node number for the physical pages that are still valid
        // (if system is low on physical memory, some pages could have been trimmed already).

        DumpNumaNodeInfo(Buffer, AllocationSize);

        _putts(_T(""));
    }

Exit:
    if (Buffers != NULL) {
        for (UINT i = 0; i < NumberOfProcessors; i++) {
            if (Buffers[i] != NULL) {
                VirtualFree(Buffers[i], 0, MEM_RELEASE);
            }
        }

        free(Buffers);
    }
}


void DumpRegion(PVOID StartPtr, PVOID EndPtr, BOOL Valid, DWORD Node)
{
    DWORD_PTR StartPage = ((DWORD_PTR)StartPtr) / PageSize;
    DWORD_PTR EndPage = ((DWORD_PTR)EndPtr) / PageSize;
    DWORD_PTR NumPages = (EndPage - StartPage) + 1;

    if (!Valid) {
        _tprintf(_T("%p - %p (%6Iu pages): no valid pages\n"), StartPtr, EndPtr, NumPages);
    } else {
        _tprintf(_T("%p - %p (%6Iu pages): node %u\n"), StartPtr, EndPtr, NumPages, Node);
    }
}


void DumpNumaNodeInfo(PVOID Buffer, SIZE_T Size)
{
    DWORD_PTR StartPage = ((DWORD_PTR)Buffer) / PageSize;
    DWORD_PTR EndPage = ((DWORD_PTR)Buffer + Size - 1) / PageSize;
    DWORD_PTR NumPages = (EndPage - StartPage) + 1;

    PCHAR StartPtr = (PCHAR)(PageSize * StartPage);

    _putts(_T("Checking NUMA node:"));

    PPSAPI_WORKING_SET_EX_INFORMATION WsInfo = (PPSAPI_WORKING_SET_EX_INFORMATION)
        malloc(NumPages * sizeof(PSAPI_WORKING_SET_EX_INFORMATION));
    if (WsInfo == NULL) {
        _putts(_T("Could not allocate array of PSAPI_WORKING_SET_EX_INFORMATION structures"));
        return;
    }

    for (DWORD_PTR i = 0; i < NumPages; i++) {
        WsInfo[i].VirtualAddress = StartPtr + i * PageSize;
    }

    BOOL bResult = QueryWorkingSetEx(
        GetCurrentProcess(),
        WsInfo,
        (DWORD)NumPages * sizeof(PSAPI_WORKING_SET_EX_INFORMATION));
    if (!bResult) {
        _tprintf(_T("QueryWorkingSetEx failed: %d\n"), GetLastError());
        free(WsInfo);
        return;
    }

    PCHAR RegionStart = NULL;
    BOOL  RegionIsValid = false;
    DWORD RegionNode = 0;

    for (DWORD_PTR i = 0; i < NumPages; i++) {
        PCHAR Address = (PCHAR)WsInfo[i].VirtualAddress;
        BOOL  IsValid = WsInfo[i].VirtualAttributes.Valid;
        DWORD Node = WsInfo[i].VirtualAttributes.Node;

        if (i == 0) {
            RegionStart = Address;
            RegionIsValid = IsValid;
            RegionNode = Node;
        }

        if (IsValid != RegionIsValid || Node != RegionNode) {
            DumpRegion(RegionStart, Address - 1, RegionIsValid, RegionNode);

            RegionStart = Address;
            RegionIsValid = IsValid;
            RegionNode = Node;
        }

        if (i == (NumPages - 1)) {
            DumpRegion(RegionStart, Address + PageSize - 1, IsValid, Node);
        }
    }

    free(WsInfo);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI CreatingGuardPages()
/*
Creating Guard Pages
05/31/2018

https://docs.microsoft.com/en-us/windows/win32/memory/creating-guard-pages

A guard page provides a one-shot alarm for memory page access.
This can be useful for an application that needs to monitor the growth of large dynamic data structures.
For example, there are operating systems that use guard pages to implement automatic stack checking.

To create a guard page, set the PAGE_GUARD page protection modifier for the page.
This value can be specified, along with other page protection modifiers,
in the VirtualAlloc, VirtualAllocEx, VirtualProtect, and VirtualProtectEx functions.
The PAGE_GUARD modifier can be used with any other page protection modifiers, except PAGE_NOACCESS.

If a program attempts to access an address within a guard page, the system raises a STATUS_GUARD_PAGE_VIOLATION (0x80000001) exception.
The system also clears the PAGE_GUARD modifier, removing the memory page's guard page status.
The system will not stop the next attempt to access the memory page with a STATUS_GUARD_PAGE_VIOLATION exception.

If a guard page exception occurs during a system service, the service fails and typically returns some failure status indicator.
Since the system also removes the relevant memory page's guard page status,
the next invocation of the same system service won't fail due to a STATUS_GUARD_PAGE_VIOLATION exception (unless, of course, someone reestablishes the guard page).

The following short program illustrates the behavior of guard page protection.

   A program to demonstrate the use of guard pages of memory. Allocate
   a page of memory as a guard page, then try to access the page. That
   will fail, but doing so releases the lock on the guard page, so the next access works correctly.

   The output will look like this. The actual address may vary.

   This computer has a page size of 4096.
   Committed 4096 bytes at address 0x00520000
   Cannot lock at 00520000, error = 0x80000001
   2nd Lock Achieved at 00520000

   This sample does not show how to use the guard page fault to "grow" a dynamic array, such as a stack.

   The first attempt to lock the memory block fails, raising a STATUS_GUARD_PAGE_VIOLATION exception.
   The second attempt succeeds, because the memory block's guard page protection has been toggled off by the first attempt.
*/
{
    LPVOID lpvAddr;               // address of the test memory
    DWORD dwPageSize;             // amount of memory to allocate.
    BOOL bLocked;                 // address of the guarded memory
    SYSTEM_INFO sSysInfo;         // useful information about the system

    GetSystemInfo(&sSysInfo);     // initialize the structure

    _tprintf(TEXT("This computer has page size %d.\n"), sSysInfo.dwPageSize);

    dwPageSize = sSysInfo.dwPageSize;

    // Try to allocate the memory.
    lpvAddr = VirtualAlloc(NULL, dwPageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READONLY | PAGE_GUARD);
    if (lpvAddr == NULL) {
        _tprintf(TEXT("VirtualAlloc failed. Error: %ld\n"), GetLastError());
        return 1;
    } else {
        _ftprintf(stderr, TEXT("Committed %lu bytes at address 0x%p\n"), dwPageSize, lpvAddr);
    }

    // Try to lock the committed memory. This fails the first time because of the guard page.
    bLocked = VirtualLock(lpvAddr, dwPageSize);
    if (!bLocked) {
        _ftprintf(stderr, TEXT("Cannot lock at %p, error = 0x%lx\n"), lpvAddr, GetLastError());
    } else {
        _ftprintf(stderr, TEXT("Lock Achieved at %p\n"), lpvAddr);
    }

    // Try to lock the committed memory again. This succeeds the second
    // time because the guard page status was removed by the first access attempt.
    bLocked = VirtualLock(lpvAddr, dwPageSize);
    if (!bLocked) {
        _ftprintf(stderr, TEXT("Cannot get 2nd lock at %p, error = %lx\n"), lpvAddr, GetLastError());
    } else {
        _ftprintf(stderr, TEXT("2nd Lock Achieved at %p\n"), lpvAddr);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


// A short program to demonstrate dynamic memory allocation using a structured exception handler.

#define PAGELIMIT 80            // Number of pages to ask for

LPTSTR lpNxtPage;               // Address of the next page to ask for
DWORD dwPages = 0;              // Count of pages gotten so far
DWORD dwPageSize;               // Page size on this computer


INT PageFaultExceptionFilter(DWORD dwCode)
{
    LPVOID lpvResult;

    // If the exception is not a page fault, exit.
    if (dwCode != EXCEPTION_ACCESS_VIOLATION) {
        _tprintf(TEXT("Exception code = %d.\n"), dwCode);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    _tprintf(TEXT("Exception is a page fault.\n"));

    // If the reserved pages are used up, exit.
    if (dwPages >= PAGELIMIT) {
        _tprintf(TEXT("Exception: out of pages.\n"));
        return EXCEPTION_EXECUTE_HANDLER;
    }

    // Otherwise, commit another page.
    lpvResult = VirtualAlloc(
        (LPVOID)lpNxtPage, // Next page to commit
        dwPageSize,         // Page size, in bytes
        MEM_COMMIT,         // Allocate a committed page
        PAGE_READWRITE);    // Read/write access
    if (lpvResult == NULL) {
        _tprintf(TEXT("VirtualAlloc failed.\n"));
        return EXCEPTION_EXECUTE_HANDLER;
    } else {
        _tprintf(TEXT("Allocating another page.\n"));
    }

    // Increment the page count, and advance lpNxtPage to the next page.
    dwPages++;
    lpNxtPage = (LPTSTR)((PCHAR)lpNxtPage + dwPageSize);

    // Continue execution where the page fault occurred.
    return EXCEPTION_CONTINUE_EXECUTION;
}


VOID ErrorExit(LPCTSTR lpMsg)
{
    _tprintf(TEXT("Error! %s with error code of %ld.\n"), lpMsg, GetLastError());
    exit(0);
}


EXTERN_C
__declspec(dllexport)
VOID WINAPI ReservingCommittingMemory(VOID)
/*
Reserving and Committing Memory

The following example illustrates the use of the VirtualAlloc and VirtualFree functions in reserving and committing memory as needed for a dynamic array.
First, VirtualAlloc is called to reserve a block of pages with NULL specified as the base address parameter, forcing the system to determine the location of the block.
Later, VirtualAlloc is called whenever it is necessary to commit a page from this reserved region, and the base address of the next page to be committed is specified.

The example uses structured exception-handling syntax to commit pages from the reserved region.
Whenever a page fault exception occurs during the execution of the __try block, the filter function in the expression preceding the __except block is executed.
If the filter function can allocate another page, execution continues in the __try block at the point where the exception occurred.
Otherwise, the exception handler in the __except block is executed. For more information, see Structured Exception Handling.

As an alternative to dynamic allocation, the process can simply commit the entire region instead of only reserving it.
Both methods result in the same physical memory usage because committed pages do not consume any physical storage until they are first accessed.
The advantage of dynamic allocation is that it minimizes the total number of committed pages on the system.
For very large allocations, pre-committing an entire allocation can cause the system to run out of committable pages, resulting in virtual memory allocation failures.

The ExitProcess function in the __except block automatically releases virtual memory allocations,
so it is not necessary to explicitly free the pages when the program terminates through this execution path.
The VirtualFree function frees the reserved and committed pages if the program is built with exception handling disabled.
This function uses MEM_RELEASE to decommit and release the entire region of reserved and committed pages.

The following C++ example demonstrates dynamic memory allocation using a structured exception handler.

https://docs.microsoft.com/en-us/windows/win32/memory/reserving-and-committing-memory
*/
{
    LPVOID lpvBase;               // Base address of the test memory
    LPTSTR lpPtr;                 // Generic character pointer
    BOOL bSuccess;                // Flag
    DWORD i;                      // Generic counter
    SYSTEM_INFO sSysInfo;         // Useful information about the system

    GetSystemInfo(&sSysInfo);     // Initialize the structure.

    _tprintf(TEXT("This computer has page size %d.\n"), sSysInfo.dwPageSize);

    dwPageSize = sSysInfo.dwPageSize;

    // Reserve pages in the virtual address space of the process.
    lpvBase = VirtualAlloc(
        NULL,                 // System selects address
        PAGELIMIT * dwPageSize, // Size of allocation
        MEM_RESERVE,          // Allocate reserved pages
        PAGE_NOACCESS);       // Protection = no access
    if (lpvBase == NULL)
        ErrorExit(TEXT("VirtualAlloc reserve failed."));

    lpPtr = lpNxtPage = (LPTSTR)lpvBase;

    // Use structured exception handling when accessing the pages.
    // If a page fault occurs, the exception filter is executed to
    // commit another page from the reserved block of pages.

    for (i = 0; i < PAGELIMIT * dwPageSize; i++) {
        __try {
            // Write to memory.
            lpPtr[i] = 'a';
        }
        // If there's a page fault, commit another page and try again.
        __except (PageFaultExceptionFilter(GetExceptionCode())) {
            // This code is executed only if the filter function
            // is unsuccessful in committing the next page.

            _tprintf(TEXT("Exiting process.\n"));

            ExitProcess(GetLastError());
        }
    }

    // Release the block of pages when you are finished using them.
    bSuccess = VirtualFree(
        lpvBase,       // Base address of block
        0,             // Bytes of committed pages
        MEM_RELEASE);  // Decommit the pages

    _tprintf(TEXT("Release %s.\n"), bSuccess ? TEXT("succeeded") : TEXT("failed"));
}


//////////////////////////////////////////////////////////////////////////////////////////////////


// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS and compile with -DPSAPI_VERSION=1
void PrintMemoryInfo(DWORD processID)
/*
Collecting Memory Usage Information For a Process
05/31/2018

To determine the efficiency of your application, you may want to examine its memory usage.
The following sample code uses the GetProcessMemoryInfo function to obtain information about the memory usage of a process.

https://docs.microsoft.com/en-us/windows/win32/psapi/collecting-memory-usage-information-for-a-process
*/
{
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    // Print the process identifier.
    printf("\nProcess ID: %u\n", processID);

    // Print information about the memory usage of the process.
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL == hProcess)
        return;

    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        printf("\tPageFaultCount: 0x%08X\n", pmc.PageFaultCount);
        printf("\tPeakWorkingSetSize: 0x%08X\n", pmc.PeakWorkingSetSize);
        printf("\tWorkingSetSize: 0x%08X\n", pmc.WorkingSetSize);
        printf("\tQuotaPeakPagedPoolUsage: 0x%08X\n", pmc.QuotaPeakPagedPoolUsage);
        printf("\tQuotaPagedPoolUsage: 0x%08X\n", pmc.QuotaPagedPoolUsage);
        printf("\tQuotaPeakNonPagedPoolUsage: 0x%08X\n", pmc.QuotaPeakNonPagedPoolUsage);
        printf("\tQuotaNonPagedPoolUsage: 0x%08X\n", pmc.QuotaNonPagedPoolUsage);
        printf("\tPagefileUsage: 0x%08X\n", pmc.PagefileUsage);
        printf("\tPeakPagefileUsage: 0x%08X\n", pmc.PeakPagefileUsage);
    }

    CloseHandle(hProcess);
}


EXTERN_C
__declspec(dllexport)
int WINAPI CollectingMemoryUsageInformationForProcess(void)
/*
Collecting Memory Usage Information For a Process
2018/05/31

To determine the efficiency of your application, you may want to examine its memory usage.
The following sample code uses the GetProcessMemoryInfo function to obtain information about the memory usage of a process.

https://docs.microsoft.com/zh-cn/windows/win32/psapi/collecting-memory-usage-information-for-a-process
*/
{
    // Get the list of process identifiers.
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return 1;
    }

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the memory usage for each process
    for (i = 0; i < cProcesses; i++) {
        PrintMemoryInfo(aProcesses[i]);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


void GetDumpFileName(TCHAR * dumpFileName)
{
    TCHAR ImageFilePath[MAX_PATH] = {0};
    GetImageFilePath(ImageFilePath, ARRAYSIZE(ImageFilePath));

    lstrcpy(dumpFileName, ImageFilePath);
    BOOL B = PathAppend(dumpFileName, L"Dump");
    _ASSERTE(B);

    if (FALSE == PathFileExists(dumpFileName)) {
        B = CreateDirectory(dumpFileName, NULL);
        _ASSERTE(B);
    }

    SYSTEMTIME st;
    GetLocalTime(&st);

    wchar_t file_name[MAX_PATH] = {0};
    wsprintf(file_name, L"%d-%d-%d-%d-%d-%d-%d.dmp",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    B = PathAppend(dumpFileName, file_name);
    _ASSERTE(B);
}


EXTERN_C
__declspec(dllexport)
LONG WINAPI TopLevelExceptionFilter(_In_  struct _EXCEPTION_POINTERS * ExceptionInfo)
/*
SetUnhandledExceptionFilterµÄ²ÎÊý¡£
*/
{
    TCHAR dumpFileName[MAX_PATH] = {0};
    GetDumpFileName(dumpFileName);

    HANDLE hFile = CreateFile(dumpFileName,
                              GENERIC_ALL,
                              FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        //EVENTLOGW(EVENTLOG_ERROR_TYPE, "LastError:%#x", GetLastError());
        return  EXCEPTION_EXECUTE_HANDLER;
    }

    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ExceptionPointers = ExceptionInfo;
    mei.ThreadId = GetCurrentThreadId();
    mei.ClientPointers = 0;
    BOOL b = MiniDumpWriteDump(GetCurrentProcess(),
                               GetCurrentProcessId(),
                               hFile,
                               MiniDumpWithFullMemory,
                               &mei,
                               NULL,
                               NULL);
    if (!b) {
        //EVENTLOGW(EVENTLOG_ERROR_TYPE, "LastError:%#x", GetLastError());
    }

    CloseHandle(hFile);

    return EXCEPTION_EXECUTE_HANDLER;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
