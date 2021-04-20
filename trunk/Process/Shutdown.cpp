#include "pch.h"
#include "Shutdown.h"


//////////////////////////////////////////////////////////////////////////////////////////////////


bool GetCurrentToken(OUT PHANDLE hToken)
/*
功能：获取当前（进程/线程）的TOKEN。
注意：请求的权限。

必须加：TOKEN_ASSIGN_PRIMARY
见：http://blogs.msdn.com/b/s4cd/archive/2007/04/16/guest-blog-user-account-control-for-developers.aspx
*/
{
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, TRUE, hToken)) {
        if (GetLastError() == ERROR_NO_TOKEN) {
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, hToken)) {
                return FALSE;
            }
        } else {
            return FALSE;
        }
    }

    return TRUE;
}


bool GetComSpec(OUT TCHAR * cmd)
/*
功能：获取 命令提示符的完整路径。

这个方法应该比较标准。
https://technet.microsoft.com/en-us/library/cc976142.aspx
HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment
*/
{
    if (cmd == NULL) {
        return false;
    }

    TCHAR  infoBuf[MAX_PATH] = {0};
    DWORD  bufCharCount = ExpandEnvironmentStrings(L"%ComSpec%", infoBuf, MAX_PATH);//其实也可以动态获取需要的内存的大小.
    if (bufCharCount > MAX_PATH || bufCharCount == 0) {
        int x = GetLastError();
        return false;
    }

    RtlCopyMemory(cmd, infoBuf, bufCharCount * sizeof(TCHAR));
    return true;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RemoveShutdownPrivilegeStartProcess()
/*
功能：去掉关机的权限，然后启动进程。
      即：以没有关机的权限启动进程。
      通俗的说是：禁止启动的那个进程关机。
*/
{
    HANDLE hToken = 0;
    GetCurrentToken(&hToken);

    DWORD Flags = 0;
    DWORD DisableSidCount = 0;
    PSID_AND_ATTRIBUTES SidsToDisable = NULL;
    DWORD DeletePrivilegeCount = 0;
    LUID_AND_ATTRIBUTES PrivilegesToDelete = {0};
    DWORD RestrictedSidCount = 0;
    PSID_AND_ATTRIBUTES SidsToRestrict = NULL;
    HANDLE NewTokenHandle = NULL;

    LUID shutdownPrivilege = {0};

    if (!LookupPrivilegeValue(
        NULL,            // lookup privilege on local system
        SE_SHUTDOWN_NAME,// privilege to lookup 
        &shutdownPrivilege))        // receives LUID of privilege
    {
        printf("LookupPrivilegeValue error: %u\n", GetLastError());
        return FALSE;
    }

    DeletePrivilegeCount = 1;
    PrivilegesToDelete.Luid = shutdownPrivilege;
    PrivilegesToDelete.Attributes = 0;

    BOOL B = CreateRestrictedToken(
        hToken,
        Flags,
        DisableSidCount,
        SidsToDisable,
        DeletePrivilegeCount,
        &PrivilegesToDelete,
        RestrictedSidCount,
        SidsToRestrict,
        &NewTokenHandle
    );
    if (B == 0) {
        int x = GetLastError();
        return x;
    }

    TCHAR ComSpec[MAX_PATH] = {0};
    GetComSpec(ComSpec);

    LPCTSTR lpApplicationName = NULL;
    LPSECURITY_ATTRIBUTES lpProcessAttributes = NULL;
    LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL;
    BOOL bInheritHandles = false;
    DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS;
    LPVOID lpEnvironment = NULL;
    LPCTSTR lpCurrentDirectory = NULL;
    STARTUPINFO StartupInfo = {0};
    PROCESS_INFORMATION ProcessInformation = {0};
    StartupInfo.cb = sizeof(STARTUPINFO);
    B = CreateProcessAsUser(
        NewTokenHandle,
        lpApplicationName,
        ComSpec,
        lpProcessAttributes,
        lpThreadAttributes,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        &StartupInfo,
        &ProcessInformation
    );
    if (B == 0) {
        int x = GetLastError();//5 拒绝访问。 
        return x;
    }

    if (B && ProcessInformation.hProcess != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
        CloseHandle(ProcessInformation.hProcess);
    }

    if (ProcessInformation.hThread != INVALID_HANDLE_VALUE) {
        CloseHandle(ProcessInformation.hThread);
    }

    if (hToken != INVALID_HANDLE_VALUE) {
        CloseHandle(hToken);
    }

    if (NewTokenHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(NewTokenHandle);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
BOOL WINAPI MySystemShutdown()
//关机的方法一：
{
    HANDLE hToken; // Get a token for this process. 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) return(FALSE);

    TOKEN_PRIVILEGES tkp; // Get the LUID for the shutdown privilege. 
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;  // one privilege to set    
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get the shutdown privilege for this process. 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
    if (GetLastError() != ERROR_SUCCESS) return FALSE;

#pragma prefast(push)
#pragma prefast(disable: 28159, "考虑使用“InitiateSystemShutdownEx”")
    // Shut down the system and force all applications to close. 
    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_UPGRADE | SHTDN_REASON_FLAG_PLANNED))
        return FALSE;
#pragma prefast(pop)    

    return TRUE;//shutdown was successful
}


EXTERN_C
__declspec(dllexport)
BOOL WINAPI MySystemShutdownWithDialogbox(LPCTSTR lpMsg)
//带提示的关机。
{
    HANDLE hToken;              // handle to process token 
    // Get the current process token handle so we can get shutdown privilege. 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return FALSE;

    TOKEN_PRIVILEGES tkp;       // pointer to token structure 
    // Get the LUID for shutdown privilege. 
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;  // one privilege to set    
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get shutdown privilege for this process. 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    // Cannot test the return value of AdjustTokenPrivileges. 
    if (GetLastError() != ERROR_SUCCESS) return FALSE;

#pragma prefast(push)
#pragma prefast(disable: 28159, "考虑使用“InitiateSystemShutdownEx”")
    BOOL fResult;               // system shutdown flag 
    // Display the shutdown dialog box and start the countdown. 
    fResult = InitiateSystemShutdown(NULL,    // shut down local computer 
                                     (LPWSTR)lpMsg,   // message for user
                                     30,      // time-out period, in seconds 
                                     FALSE,   // ask user to close apps 
                                     TRUE);   // reboot after shutdown 
    if (!fResult)
        return FALSE;
#pragma prefast(pop)      

    // Disable shutdown privilege. 
    tkp.Privileges[0].Attributes = 0;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    return TRUE;
}


EXTERN_C
__declspec(dllexport)
BOOL WINAPI PreventSystemShutdown()
//阻止关机。
{
    HANDLE hToken;              // handle to process token 
    // Get the current process token handle  so we can get shutdown privilege. 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return FALSE;

    TOKEN_PRIVILEGES tkp;       // pointer to token structure 
    // Get the LUID for shutdown privilege. 
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;  // one privilege to set    
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get shutdown privilege for this process. 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
    if (GetLastError() != ERROR_SUCCESS) return FALSE;

    if (!AbortSystemShutdown(NULL)) return FALSE; // Prevent the system from shutting down. 

    tkp.Privileges[0].Attributes = 0; // Disable shutdown privilege. 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    return TRUE;
}


int __cdecl ShutdownTest()
{
    //锁定计算机
    int r = MessageBox(NULL, (LPCWSTR)L"要锁定计算机吗？", (LPCWSTR)L"锁定计算机测试", MB_YESNO);
    if (r == IDYES) LockWorkStation();

    //注销当前用户的方法一：
    r = MessageBox(NULL, (LPCWSTR)L"要注销当前用户吗？", (LPCWSTR)L"注销当前用户测试一", MB_YESNO);
    if (r == IDYES) ExitWindows(0, 0);

    //注销当前用户的方法二：
    r = MessageBox(NULL, (LPCWSTR)L"要注销当前用户吗？", (LPCWSTR)L"注销当前用户测试二", MB_YESNO);
    if (r == IDYES) ExitWindowsEx(EWX_LOGOFF, 0);

    /*在应用（界面）程序中阻止注销的办法：就是返回失败。
    case WM_QUERYENDSESSION:
    {
    int r = MessageBox(NULL,(LPCWSTR)L"End the session?",(LPCWSTR)L"WM_QUERYENDSESSION",MB_YESNO);
    return r == IDYES; // Return TRUE to continue, FALSE to stop.
    break;
    }
    */

    //关机测试一：
    r = MessageBox(NULL, (LPCWSTR)L"要关机吗？", (LPCWSTR)L"关机测试一", MB_YESNO);
    if (r == IDYES) MySystemShutdown();//已经开始关机，后面用sleep，本人想也没有用。

    //经测试，上面的关机，很难取消，不知用消息能否拦截。hook ExitWindowsEx是可以的。
    //即使能运行下面的取消关机函数，也无济于事。
    /*取消关机
    r = MessageBox(NULL,(LPCWSTR)L"要取消关机吗？",(LPCWSTR)L"取消关机",MB_YESNO);
    if (r == IDYES) PreventSystemShutdown();
    */

    //关机测试二：
    r = MessageBox(NULL, (LPCWSTR)L"要关机吗？", (LPCWSTR)L"关机测试二", MB_YESNO);
    if (r == IDYES)MySystemShutdownWithDialogbox(L"made by correy");//在xp下就是弹出那个经典的消息框，Win 7下也弹出消息。

    //取消关机
    r = MessageBox(NULL, (LPCWSTR)L"要取消关机吗？", (LPCWSTR)L"取消关机", MB_YESNO);
    if (r == IDYES) PreventSystemShutdown();//相当于shutdon -a

    //重启的就不说了，别的函数也能实现。

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
