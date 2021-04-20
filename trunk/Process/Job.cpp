#include "pch.h"
#include "Job.h"


#pragma warning (disable: 4996)
#pragma warning (disable: 6387)


//////////////////////////////////////////////////////////////////////////////////////////////////


HANDLE CreateJob()
{
    UUID Uuid = {0};

    RPC_STATUS STATUS = UuidCreate(&Uuid);
    if (RPC_S_OK != STATUS) {
        return INVALID_HANDLE_VALUE;
    }

    RPC_WSTR StringUuid = NULL;
    STATUS = UuidToString(&Uuid, &StringUuid);
    if (RPC_S_OK != STATUS) {
        return INVALID_HANDLE_VALUE;
    }

    /*
    如果是这样：L"Global\\Job\\test"，返回的错误码的三，代表：系统找不到指定的路径。
    */
    WCHAR JobName[MAX_PATH] = {0};

    wsprintf(JobName, L"Global\\TestJob_{%s}", StringUuid);

    /*
    SECURITY_ATTRIBUTES的设置参考：Creating a Child Process with Redirected Input and Output
    https://msdn.microsoft.com/zh-cn/library/windows/desktop/ms682499%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
    */
    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    /*
    1.只有这个进程有这个句柄，也就是创建它的进程。
    2.If lpJobAttributes is NULL, the job object gets a default security descriptor and the handle cannot be inherited.
      这样：子进程是不会继承JOB句柄的。
    3.即使设置了SECURITY_ATTRIBUTES或者SetHandleInformation + HANDLE_FLAG_INHERIT都是无效的。
    看来只能使用：IsProcessInJob了。
    */
    HANDLE hjob = CreateJobObject(&sa, JobName);//
    if (NULL == hjob) {
        int x = GetLastError();
    }

    /*
    函数的用法参见：
    https://msdn.microsoft.com/en-us/library/windows/desktop/ms724935(v=vs.85).aspx
    https://support.microsoft.com/de-ch/kb/315939/zh-cn
    */
    BOOL B = SetHandleInformation(hjob, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    if (0 == B) {
        int x = GetLastError();
    }

    /*
    运行成功了，但是子进程孙进程等都没有继承这个句柄，也就是说没有看到这个对象。
    */

    STATUS = RpcStringFree(&StringUuid);
    if (RPC_S_OK != STATUS) {
        return INVALID_HANDLE_VALUE;
    }

    return hjob;
}


int TestJob(int argc, char * argv[])
/*
目的：测试JOB的相关功能和用法。

made by correy
made at 2015.07.10
*/
{
    //DebugBreak();

    HANDLE hjob = CreateJob();
    if (INVALID_HANDLE_VALUE == hjob) {
        return 0;
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process. 
    if (!CreateProcess(L"c:\\windows\\system32\\calc.exe" /*这个应该有标准的获取的办法，这里是测试。*/, 
                       NULL,        // Command line
                       NULL,           // Process handle not inheritable
                       NULL,           // Thread handle not inheritable
                       FALSE,          // Set handle inheritance to FALSE
                       0,              // No creation flags
                       NULL,           // Use parent's environment block
                       NULL,           // Use parent's starting directory 
                       &si,            // Pointer to STARTUPINFO structure
                       &pi)           // Pointer to PROCESS_INFORMATION structure
        ) {
        printf("CreateProcess failed (%d)\n", GetLastError());
        return 0;
    }

    BOOL B = AssignProcessToJobObject(hjob, pi.hProcess);//这个进程没有看到JOB对象。
    if (0 == B) {
        int x = GetLastError();
    }

    BOOL Result = false;
    B = IsProcessInJob(pi.hProcess, hjob, &Result);
    if (0 == B) {
        int x = GetLastError();
        printf("failed (%d)\n", GetLastError());
    }

    // Start the child process. 
    if (!CreateProcess(L"c:\\windows\\system32\\cmd.exe" /*这个应该有标准的获取的办法，这里是测试。*/,
                       NULL,        // Command line
                       NULL,           // Process handle not inheritable
                       NULL,           // Thread handle not inheritable
                       FALSE,          // Set handle inheritance to FALSE
                       0,              // No creation flags
                       NULL,           // Use parent's environment block
                       NULL,           // Use parent's starting directory 
                       &si,            // Pointer to STARTUPINFO structure
                       &pi)           // Pointer to PROCESS_INFORMATION structure
        ) {
        printf("CreateProcess failed (%d)\n", GetLastError());
        return 0;
    }

    B = AssignProcessToJobObject(hjob, pi.hProcess);//这个进程没有看到JOB对象。
    if (0 == B) {
        int x = GetLastError();
    }

    Result = false;
    B = IsProcessInJob(pi.hProcess, hjob, &Result);
    if (0 == B) {
        int x = GetLastError();
        printf("failed (%d)\n", GetLastError());
    }

    /*
    在这里可以做一些操作，如创建子进程，孙进程，父子进程，断链（结束有子进程的进程）。
    经观察，JOB的句柄没有继承/传递。或许是设置的问题（ SECURITY_ATTRIBUTES ）。
    看样子只能自己手动设置了（AssignProcessToJobObject）。
    */

    /*
    这个函数还真是：能杀死所有的属于某个Job的进程。
    尽管SetHandleInformation设置为可继承，子孙进程没有看到这个句柄，
    但是还是把所有的（属于JOB）进程给结束了，不论断链没有。
    */
    B = TerminateJobObject(hjob, 0);//STILL_ACTIVE
    if (0 == B) {
        int x = GetLastError();
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hjob);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
