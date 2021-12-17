#include "pch.h"
#include "PEB.h"
#include "Process.h"


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
DWORD WINAPI GetCommandLineEx(_In_ HANDLE UniqueProcessId)
/*
功能：获取一个进程的命令行。

实现思路：读取另外一个进程的PEB信息。

注意：
1.32位操作系统的32位程序。
2.64位操作系统的64位程序。
3.64位操作系统的32位程序。

参数，其实都是DWORD。

https://wj32.org/wp/2009/01/24/howto-get-the-command-line-of-processes/
*/
{
    HANDLE ParentsPid = INVALID_HANDLE_VALUE;
    bool IsWow64 = false;
    int Offset = 0;

    if (NULL == ZwQueryInformationProcess) {
        return GetLastError();
    }

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                  FALSE,
                                  HandleToULong(UniqueProcessId));
    if (NULL == hProcess) {
        printf("LastError:%d\n", GetLastError());
        return GetLastError();
    }

    PROCESS_BASIC_INFORMATION ProcessBasicInfo = {0};
    ULONG ReturnLength = 0;
    NTSTATUS status = ZwQueryInformationProcess(hProcess,
                                                ProcessBasicInformation,
                                                &ProcessBasicInfo,
                                                sizeof(PROCESS_BASIC_INFORMATION),
                                                &ReturnLength);
    if (!NT_SUCCESS(status)) {
        printf("LastError:%d\n", GetLastError());
        CloseHandle(hProcess);
        return GetLastError();
    }

    if (IsWow64) {
        Offset = FIELD_OFFSET(PEB_WOW64, ProcessParameters);
    } else {
        Offset = FIELD_OFFSET(PEB, ProcessParameters);
    }

    PRTL_USER_PROCESS_PARAMETERS ProcessParameters = NULL;
    if (!ReadProcessMemory(hProcess,
                           (PCHAR)ProcessBasicInfo.PebBaseAddress + Offset,
                           &ProcessParameters,
                           sizeof(PVOID),
                           NULL)) {
        printf("LastError:%d\n", GetLastError());
        CloseHandle(hProcess);
        return GetLastError();
    }

    RTL_USER_PROCESS_PARAMETERS ProcessParameter = {0};
    if (!ReadProcessMemory(hProcess,
                           ProcessParameters,
                           &ProcessParameter,
                           sizeof(RTL_USER_PROCESS_PARAMETERS),
                           NULL)) {
        printf("LastError:%d\n", GetLastError());
        CloseHandle(hProcess);
        return GetLastError();
    }

    PWCHAR CommandLine = (PWCHAR)HeapAlloc(GetProcessHeap(),
                                           HEAP_ZERO_MEMORY,
                                           ProcessParameter.CommandLine.Length + sizeof(WCHAR));
    _ASSERTE(CommandLine);

    if (!ReadProcessMemory(hProcess,
                           ProcessParameter.CommandLine.Buffer,
                           CommandLine,
                           ProcessParameter.CommandLine.Length,
                           NULL)) {
        printf("LastError:%d\n", GetLastError());
    }

    printf("Pid:%d, CommandLine:%ls\n", HandleToULong(UniqueProcessId), CommandLine);

    HeapFree(GetProcessHeap(), 0, CommandLine);
    CloseHandle(hProcess);
    return GetLastError();
}


//////////////////////////////////////////////////////////////////////////////////////////////////
