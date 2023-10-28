#include "Memory.h"


BOOL WINAPI  EmptyProcessWorkingSet(_In_ PVOID lppe, _In_opt_ PVOID Context)
/*
这家伙的效果还是明显的，一家伙内存降低好几个GB。

The handle must have the PROCESS_QUERY_INFORMATION or PROCESS_QUERY_LIMITED_INFORMATION access right and the PROCESS_SET_QUOTA access right

https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-emptyworkingset
https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-setprocessworkingsetsize
https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-setprocessworkingsetsizeex
*/
{
    LPPROCESSENTRY32W pe32 = (LPPROCESSENTRY32W)lppe;

    DWORD DesiredAccess = PROCESS_QUERY_INFORMATION | PROCESS_SET_QUOTA | PROCESS_QUERY_LIMITED_INFORMATION;
    HANDLE hProcess = OpenProcess(DesiredAccess, FALSE, pe32->th32ProcessID);
    if (nullptr == hProcess) {
        printf("LastError:%d\n", GetLastError());
        return TRUE;
    }

    //SetProcessWorkingSetSize(hProcess, (SIZE_T)–1, (SIZE_T)–1);
    //SetProcessWorkingSetSizeEx(hProcess, (SIZE_T)–1, (SIZE_T)–1, 0);
    if (!EmptyWorkingSet(hProcess)) {
        printf("LastError:%d\n", GetLastError());
    }

    CloseHandle(hProcess);

    return TRUE;
}


void EmptyAllProcessWorkingSet()
{
    EnumerateProcess(EmptyProcessWorkingSet, nullptr);
}


void TestMemory()
{
    EmptyAllProcessWorkingSet();
}
