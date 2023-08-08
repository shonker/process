#include "ProcessesTest.h"


BOOL WINAPI EnumeratingProcessesCallBackTest(_In_ DWORD Pid, _In_opt_ PVOID Context)
{
    printf("pid:%d\n", Pid);

    return TRUE;
}


BOOL WINAPI  EnumerateProcessCallBackTest(_In_ PVOID lppe, /*实际类型是LPPROCESSENTRY32W*/
                                          _In_opt_ PVOID Context)
{
    LPPROCESSENTRY32W pe32 = (LPPROCESSENTRY32W)lppe;

    printf("Pid:%08d\tExeFile:%-50ls\n", pe32->th32ProcessID, pe32->szExeFile);

    return TRUE;
}


void TestProcess()
{
    //EnumeratingAllProcesses(EnumeratingProcessesCallBackTest, NULL);

    EnumerateProcess(EnumerateProcessCallBackTest, NULL);


}
