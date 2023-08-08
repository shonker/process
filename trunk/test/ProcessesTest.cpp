#include "ProcessesTest.h"


double GetCpuUsage(int PID)
{
    static SIZE_T dwLastProcessTime = 0;
    static SIZE_T dwLastSystemTime = 0;
    static double dCpuUsage = 0;

    if (!PID) {
        return 0;
    }

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, PID);
    if (hProcess) {
        FILETIME ftCreationTime, ftExitTime, ftKernelTime, ftUserTime;
        GetProcessTimes(hProcess, &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUserTime);

        FILETIME IdleTime;
        FILETIME KernelTime;
        FILETIME UserTime;
        bool b = GetSystemTimes(&IdleTime, &KernelTime, &UserTime);

        ULARGE_INTEGER uiKernelTime, uiUserTime;
        uiKernelTime.HighPart = ftKernelTime.dwHighDateTime;
        uiKernelTime.LowPart = ftKernelTime.dwLowDateTime;
        uiUserTime.HighPart = ftUserTime.dwHighDateTime;
        uiUserTime.LowPart = ftUserTime.dwLowDateTime;

        SIZE_T dwActualProcessTime = (SIZE_T)((uiKernelTime.QuadPart + uiUserTime.QuadPart) / 100);
        SIZE_T dwActualSystemTime = (SIZE_T)GetTickCount64();

        if (0 == dwActualProcessTime) {
            return 0.0;
        }

        if (dwActualProcessTime <= dwLastProcessTime) {
            return 0.0;
        }

        if (dwLastSystemTime) {
            dCpuUsage = (double)(dwActualProcessTime - dwLastProcessTime) / (dwActualSystemTime - dwLastSystemTime);
        }

        dwLastProcessTime = dwActualProcessTime;
        dwLastSystemTime = dwActualSystemTime;
    } else {
        printf("LastError:%#x\n", GetLastError());
        dCpuUsage = 0;
    }

    return dCpuUsage;
}


BOOL WINAPI EnumeratingProcessesCallBackTest(_In_ DWORD Pid, _In_opt_ PVOID Context)
{
    printf("pid:%d\n", Pid);

    return TRUE;
}


BOOL WINAPI  EnumerateProcessCallBackTest(_In_ PVOID lppe, /*实际类型是LPPROCESSENTRY32W*/
                                          _In_opt_ PVOID Context)
{
    LPPROCESSENTRY32W pe32 = (LPPROCESSENTRY32W)lppe;

    printf("Pid:%08d\tExeFile:%-50ls\t%.3g\n", pe32->th32ProcessID, pe32->szExeFile, GetCpuUsage(pe32->th32ProcessID));

    return TRUE;
}


void TestProcess()
{
    //EnumeratingAllProcesses(EnumeratingProcessesCallBackTest, NULL);

    EnumerateProcess(EnumerateProcessCallBackTest, NULL);


}
