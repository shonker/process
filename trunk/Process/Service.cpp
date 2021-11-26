#include "pch.h"
#include "Service.h"
#include "Process.h"

#pragma warning(disable:28159) //考虑使用“GetTickCount64”而不是“GetTickCount”。
#pragma warning(disable:6242)  //从此作用域跳转将强制执行局部展开。这会导致性能下降。请使用 __leave 在 try 的作用域之外返回。
#pragma warning(disable:6387)
#pragma warning(disable:28020)
#pragma warning(disable:6011)


#ifndef _DEBUG
#pragma warning(disable:4189) //局部变量已初始化但不引用。临时处理release下的_ASSERTE编译警告。
#endif // !_DEBUG


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
BOOL WINAPI DriverInstallInf(PWSTR InfFile)
/*
这里的路径即使有空格，也不要用双引号。
解决办法是切换目录，用相对路径。

参数：
InfFile：INF文件的全路径。

InstallHinfSection使用注意事项：
1.在wow64下有文件重定向。
2.需要一些权限。
3.有窗口。

测试发现，在管理员下可执行，在system账户下可执行，在服务下执行失败。
在服务启动的程序下执行也失败，没有专门写程序测试，但是出现的问题就是如此。
分析：因为服务是不准有窗口消息操作的。

所以，解决的思路是：更新不必删除服务，所以也无需用这个函数，只需复制文件即可。
另一个思路是：以登录用户启动这个程序。
*/
{
    WCHAR test[MAX_PATH] = {0};
    lstrcpy(test, L"DefaultInstall 132 ");
    lstrcat(test, InfFile);

    if (IsWow64()) {//在wow64下必须关闭文件重定向，否则复制驱动文件失败。
        BOOLEAN bRet = Wow64EnableWow64FsRedirection(FALSE);
        _ASSERTE(bRet);
    }

    InstallHinfSection(NULL, NULL, test, 0); //只是安装，没有启动。

    if (IsWow64()) {
        BOOLEAN bRet = Wow64EnableWow64FsRedirection(TRUE);//Enable WOW64 file system redirection. 
        _ASSERTE(bRet);
    }

    return true;
}


EXTERN_C
__declspec(dllexport)
BOOL WINAPI DriverInstall(_In_opt_ LPCWSTR BinaryPathName, _In_ LPCWSTR ServiceName)
/*
Purpose: Installs a Driver in the SCM database

http://msdn.microsoft.com/en-us/library/windows/desktop/ms683500(v=vs.85).aspx
*/
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return false;
    }

    if (IsWow64()) {
        BOOLEAN bRet = Wow64EnableWow64FsRedirection(FALSE);
        _ASSERTE(bRet);
    }
    schService = CreateService( // Create the service
                               schSCManager,              // SCM database 
                               ServiceName,                   // name of service 
                               ServiceName,                   // service name to display 
                               SERVICE_ALL_ACCESS,        // desired access 
                               SERVICE_KERNEL_DRIVER, //SERVICE_WIN32_OWN_PROCESS, // service type 不准是这个，否则失败：0x57 SERVICE_DRIVER,
                               SERVICE_SYSTEM_START, //SERVICE_DEMAND_START,      // start type  SERVICE_BOOT_START SERVICE_SYSTEM_START SERVICE_AUTO_START
                               SERVICE_ERROR_NORMAL,      // error control type 
                               BinaryPathName,            // path to service's binary 
                               NULL,                      // no load ordering group 
                               NULL,                      // no tag identifier 
                               NULL,                      // no dependencies 
                               NULL,                      // LocalSystem account 
                               NULL);                     // no password 
    if (IsWow64()) {
        Wow64EnableWow64FsRedirection(TRUE);//Enable WOW64 file system redirection. 
    }
    if (schService == NULL) {
        CloseServiceHandle(schSCManager);
        return false;
    } else {
        printf("Service installed successfully\n");
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return true;
}


EXTERN_C
__declspec(dllexport)
VOID WINAPI SvcInstall(_In_opt_ LPCWSTR BinaryPathName,
                       _In_ LPCWSTR ServiceName,
                       _In_opt_ LPCWSTR DisplayName
)
// Purpose: Installs a service in the SCM database
//https://docs.microsoft.com/zh-cn/windows/win32/services/installing-a-service
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    schSCManager = OpenSCManager(// Get a handle to the SCM database.  
                                 NULL,                    // local computer
                                 NULL,                    // ServicesActive database 
                                 SC_MANAGER_ALL_ACCESS);  // full access rights 
    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    schService = CreateService( // Create the service
                               schSCManager,              // SCM database 
                               ServiceName,               // name of service 
                               DisplayName,               // service name to display 
                               SERVICE_ALL_ACCESS,        // desired access 
                               SERVICE_WIN32_OWN_PROCESS, // service type.SERVICE_INTERACTIVE_PROCESS， Windows 10的某个版本开始不支持UI0Detect。
                               SERVICE_AUTO_START,        // start type 
                               SERVICE_ERROR_NORMAL,      // error control type 
                               BinaryPathName,            // path to service's binary 
                               NULL,                      // no load ordering group 
                               NULL,                      // no tag identifier 
                               NULL,                      // no dependencies 
                               NULL,                      // LocalSystem account 
                               NULL);                     // no password 
    if (schService == NULL) {
        DWORD LastError = GetLastError();
        if (ERROR_SERVICE_EXISTS != LastError) {
            printf("%ls CreateService failed, LastError:%d.\n", ServiceName, LastError);
            CloseServiceHandle(schSCManager);
            return;
        }
    } else {
        printf("Service installed successfully\n");
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


EXTERN_C
__declspec(dllexport)
VOID __stdcall DoStartSvc(_In_ LPCWSTR ServiceName)
//   Starts the service if possible.
//   https://msdn.microsoft.com/zh-cn/library/vs/alm/ms686315.ASPx
{
    SERVICE_STATUS_PROCESS ssStatus;
    DWORD dwOldCheckPoint;
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
    DWORD dwBytesNeeded;

    // Get a handle to the SCM database.  
    SC_HANDLE schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // servicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights  
    if (NULL == schSCManager) {
        DisplayError(L"OpenSCManager");
        return;
    }

    // Get a handle to the service.
    SC_HANDLE schService = OpenService(
        schSCManager,         // SCM database 
        ServiceName,              // name of service 
        SERVICE_ALL_ACCESS);  // full access  
    if (schService == NULL) {
        DisplayError(L"OpenService");
        CloseServiceHandle(schSCManager);
        return;
    }

    // Check the status in case the service is not stopped. 
    if (!QueryServiceStatusEx(
        schService,                     // handle to service 
        SC_STATUS_PROCESS_INFO,         // information level
        (LPBYTE)&ssStatus,             // address of structure
        sizeof(SERVICE_STATUS_PROCESS), // size of structure
        &dwBytesNeeded))              // size needed if buffer is too small
    {
        DisplayError(L"QueryServiceStatusEx");
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Check if the service is already running.
    // It would be possible to stop the service here, but for simplicity this example just returns. 
    if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING) {
        printf("Cannot start the service %ws, because it is already running\n", ServiceName);
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Save the tick count and initial checkpoint.
    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    // Wait for the service to stop before attempting to start it.
    while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
        // Do not wait longer than the wait hint.
        // A good interval is one-tenth of the wait hint but not less than 1 second and not more than 10 seconds.  
        dwWaitTime = ssStatus.dwWaitHint / 10;
        if (dwWaitTime < 1000) {
            dwWaitTime = 1000;
        } else if (dwWaitTime > 10000) {
            dwWaitTime = 10000;
        }

        Sleep(dwWaitTime);

        // Check the status until the service is no longer stop pending.  
        if (!QueryServiceStatusEx(
            schService,                     // handle to service 
            SC_STATUS_PROCESS_INFO,         // information level
            (LPBYTE)&ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded))              // size needed if buffer is too small
        {
            DisplayError(L"QueryServiceStatusEx");
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return;
        }

        if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
            // Continue to wait and check.
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        } else {
            if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint) {
                printf("Timeout waiting for service to stop\n");
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return;
            }
        }
    }

    // Attempt to start the service.
    if (!StartService(
        schService,  // handle to service 
        0,           // number of arguments 
        NULL))      // no arguments 
    {
        printf("%ws StartService failed\n", ServiceName);
        DisplayError(L"StartService");
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    } else {
        printf("%ws Service start pending...\n", ServiceName);
    }

    // Check the status until the service is no longer start pending.  
    if (!QueryServiceStatusEx(
        schService,                     // handle to service 
        SC_STATUS_PROCESS_INFO,         // info level
        (LPBYTE)&ssStatus,             // address of structure
        sizeof(SERVICE_STATUS_PROCESS), // size of structure
        &dwBytesNeeded))              // if buffer too small
    {
        DisplayError(L"QueryServiceStatusEx");
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Save the tick count and initial checkpoint.
    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    while (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
        // Do not wait longer than the wait hint. 
        // A good interval is one-tenth the wait hint, but no less than 1 second and no more than 10 seconds.  
        dwWaitTime = ssStatus.dwWaitHint / 10;
        if (dwWaitTime < 1000) {
            dwWaitTime = 1000;
        } else if (dwWaitTime > 10000) {
            dwWaitTime = 10000;
        }

        Sleep(dwWaitTime);

        // Check the status again.  
        if (!QueryServiceStatusEx(
            schService,             // handle to service 
            SC_STATUS_PROCESS_INFO, // info level
            (LPBYTE)&ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded))              // if buffer too small
        {
            DisplayError(L"QueryServiceStatusEx");
            break;
        }

        if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
            // Continue to wait and check.
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        } else {
            if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint) {
                break;// No progress made within the wait hint.
            }
        }
    }

    // Determine whether the service is running.
    if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
        printf("%ws Service started successfully.\n", ServiceName);
    } else {
        printf("Service not started. \n");
        printf("  Current State: %d\n", ssStatus.dwCurrentState);
        printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
        printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
        printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
        DisplayError(L"Service not started.");
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


EXTERN_C
__declspec(dllexport)
BOOL __stdcall StopDependentServices(_In_ LPCWSTR ServiceName)
{
    DWORD i;
    DWORD dwBytesNeeded;
    DWORD dwCount;

    LPENUM_SERVICE_STATUS   lpDependencies = NULL;
    ENUM_SERVICE_STATUS     ess;
    SC_HANDLE               hDepService;
    SERVICE_STATUS_PROCESS  ssp;

    DWORD dwStartTime = GetTickCount();
    DWORD dwTimeout = 30000; // 30-second time-out

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database.  
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights  
    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return FALSE;
    }

    // Get a handle to the service.
    schService = OpenService(
        schSCManager,         // SCM database 
        ServiceName,            // name of service 
        SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS);
    if (schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    // Pass a zero-length buffer to get the required buffer size.
    if (EnumDependentServices(schService, SERVICE_ACTIVE, lpDependencies, 0, &dwBytesNeeded, &dwCount)) {
        // If the Enum call succeeds, then there are no dependent services, so do nothing.
        return TRUE;
    } else {
        if (GetLastError() != ERROR_MORE_DATA)
            return FALSE; // Unexpected error

        // Allocate a buffer for the dependencies.
        lpDependencies = (LPENUM_SERVICE_STATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);
        if (!lpDependencies)
            return FALSE;

        __try {
            // Enumerate the dependencies.
            if (!EnumDependentServices(schService,
                                       SERVICE_ACTIVE,
                                       lpDependencies,
                                       dwBytesNeeded,
                                       &dwBytesNeeded,
                                       &dwCount))
                return FALSE;

            for (i = 0; i < dwCount; i++) {
                ess = *(lpDependencies + i);
                // Open the service.
                hDepService = OpenService(schSCManager, ess.lpServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS);
                if (!hDepService)
                    return FALSE;

                __try {
                    // Send a stop code.
                    if (!ControlService(hDepService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp))
                        return FALSE;

                    // Wait for the service to stop.
                    while (ssp.dwCurrentState != SERVICE_STOPPED) {
                        Sleep(ssp.dwWaitHint);
                        if (!QueryServiceStatusEx(hDepService,
                                                  SC_STATUS_PROCESS_INFO,
                                                  (LPBYTE)&ssp,
                                                  sizeof(SERVICE_STATUS_PROCESS),
                                                  &dwBytesNeeded))
                            return FALSE;

                        if (ssp.dwCurrentState == SERVICE_STOPPED)
                            break;

                        if (GetTickCount() - dwStartTime > dwTimeout)
                            return FALSE;
                    }
                } __finally {
                    CloseServiceHandle(hDepService);// Always release the service handle.
                }
            }
        } __finally {
            HeapFree(GetProcessHeap(), 0, lpDependencies);// Always free the enumeration buffer.
        }
    }

    return TRUE;
}


EXTERN_C
__declspec(dllexport)
VOID __stdcall DoStopSvc(_In_ LPCWSTR ServiceName)
/*
Stops the service.

https://msdn.microsoft.com/en-us/library/windows/desktop/bb540474(v=vs.85).aspx
*/
{
    SERVICE_STATUS_PROCESS ssp;
    DWORD dwStartTime = GetTickCount();
    DWORD dwBytesNeeded;
    DWORD dwTimeout = 30000; // 30-second time-out
    DWORD dwWaitTime;
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database.  
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights  
    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the service.
    schService = OpenService(
        schSCManager,         // SCM database 
        ServiceName,            // name of service 
        SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS);
    if (schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Make sure the service is not already stopped.
    if (!QueryServiceStatusEx(schService,
                              SC_STATUS_PROCESS_INFO,
                              (LPBYTE)&ssp,
                              sizeof(SERVICE_STATUS_PROCESS),
                              &dwBytesNeeded)) {
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        goto stop_cleanup;
    }

    if (ssp.dwCurrentState == SERVICE_STOPPED) {
        printf("%ws Service is already stopped.\n", ServiceName);
        goto stop_cleanup;
    }

    // If a stop is pending, wait for it.
    while (ssp.dwCurrentState == SERVICE_STOP_PENDING) {
        printf("Service stop pending...\n");

        // Do not wait longer than the wait hint. 
        // A good interval is one-tenth of the wait hint but not less than 1 second and not more than 10 seconds. 
        dwWaitTime = ssp.dwWaitHint / 10;
        if (dwWaitTime < 1000) {
            dwWaitTime = 1000;
        } else if (dwWaitTime > 10000) {
            dwWaitTime = 10000;
        }

        Sleep(dwWaitTime);

        if (!QueryServiceStatusEx(schService,
                                  SC_STATUS_PROCESS_INFO,
                                  (LPBYTE)&ssp,
                                  sizeof(SERVICE_STATUS_PROCESS),
                                  &dwBytesNeeded)) {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            goto stop_cleanup;
        }

        if (ssp.dwCurrentState == SERVICE_STOPPED) {
            printf("Service stopped successfully.\n");
            goto stop_cleanup;
        }

        if (GetTickCount() - dwStartTime > dwTimeout) {
            printf("Service stop timed out.\n");
            goto stop_cleanup;
        }
    }

    // If the service is running, dependencies must be stopped first.
    StopDependentServices(ServiceName);

    // Send a stop code to the service.
    if (!ControlService(schService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp)) {
        printf("ControlService failed (%d)\n", GetLastError());
        goto stop_cleanup;
    }

    // Wait for the service to stop.
    while (ssp.dwCurrentState != SERVICE_STOPPED) {
        Sleep(ssp.dwWaitHint);
        if (!QueryServiceStatusEx(schService,
                                  SC_STATUS_PROCESS_INFO,
                                  (LPBYTE)&ssp,
                                  sizeof(SERVICE_STATUS_PROCESS),
                                  &dwBytesNeeded)) {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            goto stop_cleanup;
        }

        if (ssp.dwCurrentState == SERVICE_STOPPED) {
            break;
        }

        if (GetTickCount() - dwStartTime > dwTimeout) {
            printf("Wait timed out\n");
            goto stop_cleanup;
        }
    }

    printf("%ws Service stopped successfully\n", ServiceName);

stop_cleanup:
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


EXTERN_C
__declspec(dllexport)
VOID __stdcall DoDeleteSvc(_In_ LPCWSTR ServiceName)
/*
Deletes a service from the SCM database

https://msdn.microsoft.com/en-us/library/windows/desktop/bb540473%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
*/
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    //SERVICE_STATUS ssStatus; 

    // Get a handle to the SCM database.  
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights  
    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the service.
    schService = OpenService(
        schSCManager,       // SCM database 
        ServiceName,          // name of service 
        DELETE);            // need delete access  
    if (schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Delete the service. 
    if (!DeleteService(schService)) {
        printf("DeleteService failed (%d)\n", GetLastError());
    } else printf("Service deleted successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


EXTERN_C
__declspec(dllexport)
VOID __stdcall DoQuerySvc(_In_ LPCWSTR ServiceName)
/*
Retrieves and displays the current service configuration.

https://msdn.microsoft.com/en-us/library/windows/desktop/bb540473%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
*/
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    LPQUERY_SERVICE_CONFIG lpsc = NULL;
    LPSERVICE_DESCRIPTION lpsd = NULL;
    DWORD dwBytesNeeded, cbBufSize = 0, dwError;

    // Get a handle to the SCM database.  
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights  
    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the service.
    schService = OpenService(
        schSCManager,          // SCM database 
        ServiceName,             // name of service 
        SERVICE_QUERY_CONFIG); // need query config access  
    if (schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Get the configuration information. 
    if (!QueryServiceConfig(schService, NULL, 0, &dwBytesNeeded)) {
        dwError = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == dwError) {
            cbBufSize = dwBytesNeeded;
            lpsc = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, cbBufSize);
        } else {
            printf("QueryServiceConfig failed (%d)", dwError);
            goto cleanup;
        }
    }

    if (!QueryServiceConfig(schService, lpsc, cbBufSize, &dwBytesNeeded)) {
        printf("QueryServiceConfig failed (%d)", GetLastError());
        goto cleanup;
    }

    if (!QueryServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, NULL, 0, &dwBytesNeeded)) {
        dwError = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == dwError) {
            cbBufSize = dwBytesNeeded;
            lpsd = (LPSERVICE_DESCRIPTION)LocalAlloc(LMEM_FIXED, cbBufSize);
        } else {
            printf("QueryServiceConfig2 failed (%d)", dwError);
            goto cleanup;
        }
    }

    if (!QueryServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, (LPBYTE)lpsd, cbBufSize, &dwBytesNeeded)) {
        printf("QueryServiceConfig2 failed (%d)", GetLastError());
        goto cleanup;
    }

    // Print the configuration information. 
    _tprintf(TEXT("%s configuration: \n"), ServiceName);
    _tprintf(TEXT("  Type: 0x%x\n"), lpsc->dwServiceType);
    _tprintf(TEXT("  Start Type: 0x%x\n"), lpsc->dwStartType);
    _tprintf(TEXT("  Error Control: 0x%x\n"), lpsc->dwErrorControl);
    _tprintf(TEXT("  Binary path: %s\n"), lpsc->lpBinaryPathName);
    _tprintf(TEXT("  Account: %s\n"), lpsc->lpServiceStartName);

    if (lpsd->lpDescription != NULL && lstrcmp(lpsd->lpDescription, TEXT("")) != 0)
        _tprintf(TEXT("  Description: %s\n"), lpsd->lpDescription);
    if (lpsc->lpLoadOrderGroup != NULL && lstrcmp(lpsc->lpLoadOrderGroup, TEXT("")) != 0)
        _tprintf(TEXT("  Load order group: %s\n"), lpsc->lpLoadOrderGroup);
    if (lpsc->dwTagId != 0)
        _tprintf(TEXT("  Tag ID: %d\n"), lpsc->dwTagId);
    if (lpsc->lpDependencies != NULL && lstrcmp(lpsc->lpDependencies, TEXT("")) != 0)
        _tprintf(TEXT("  Dependencies: %s\n"), lpsc->lpDependencies);

    LocalFree(lpsc);
    LocalFree(lpsd);

cleanup:
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


EXTERN_C
__declspec(dllexport)
VOID __stdcall DoDisableSvc(_In_ LPCWSTR szSvcName)
// Purpose: 
//   Disables the service.
//https://docs.microsoft.com/zh-cn/windows/win32/services/changing-a-service-configuration
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the service.
    schService = OpenService(
        schSCManager,            // SCM database 
        szSvcName,               // name of service 
        SERVICE_CHANGE_CONFIG);  // need change config access 
    if (schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Change the service start type.
    if (!ChangeServiceConfig(
        schService,        // handle of service 
        SERVICE_NO_CHANGE, // service type: no change 
        SERVICE_DISABLED,  // service start type 
        SERVICE_NO_CHANGE, // error control: no change 
        NULL,              // binary path: no change 
        NULL,              // load order group: no change 
        NULL,              // tag ID: no change 
        NULL,              // dependencies: no change 
        NULL,              // account name: no change 
        NULL,              // password: no change 
        NULL))            // display name: no change
    {
        printf("ChangeServiceConfig failed (%d)\n", GetLastError());
    } else printf("Service disabled successfully.\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


EXTERN_C
__declspec(dllexport)
VOID __stdcall DoEnableSvc(_In_ LPCWSTR szSvcName)
// Purpose: 
//   Enables the service.
//https://docs.microsoft.com/zh-cn/windows/win32/services/changing-a-service-configuration
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the service.
    schService = OpenService(
        schSCManager,            // SCM database 
        szSvcName,               // name of service 
        SERVICE_CHANGE_CONFIG);  // need change config access 
    if (schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Change the service start type.
    if (!ChangeServiceConfig(
        schService,            // handle of service 
        SERVICE_NO_CHANGE,     // service type: no change 
        SERVICE_DEMAND_START,  // service start type 
        SERVICE_NO_CHANGE,     // error control: no change 
        NULL,                  // binary path: no change 
        NULL,                  // load order group: no change 
        NULL,                  // tag ID: no change 
        NULL,                  // dependencies: no change 
        NULL,                  // account name: no change 
        NULL,                  // password: no change 
        NULL))                // display name: no change
    {
        printf("ChangeServiceConfig failed (%d)\n", GetLastError());
    } else printf("Service enabled successfully.\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


EXTERN_C
__declspec(dllexport)
VOID __stdcall DoUpdateSvcDesc(_In_ LPCWSTR szSvcName)
// Purpose: 
//   Updates the service description to "This is a test description".
//https://docs.microsoft.com/zh-cn/windows/win32/services/changing-a-service-configuration
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_DESCRIPTION sd;
    LPCTSTR szDesc = TEXT("This is a test description");

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the service.
    schService = OpenService(
        schSCManager,            // SCM database 
        szSvcName,               // name of service 
        SERVICE_CHANGE_CONFIG);  // need change config access 
    if (schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Change the service description.
    sd.lpDescription = (LPWSTR)szDesc;
    if (!ChangeServiceConfig2(
        schService,                 // handle to service
        SERVICE_CONFIG_DESCRIPTION, // change: description
        &sd))                      // new description
    {
        printf("ChangeServiceConfig2 failed\n");
    } else printf("Service description updated successfully.\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


EXTERN_C
__declspec(dllexport)
VOID __stdcall DoUpdateSvcDacl(_In_ LPCWSTR szSvcName)
// Purpose: 
//   Updates the service DACL to grant start, stop, delete, and read
//   control access to the Guest account.
//   https://docs.microsoft.com/zh-cn/windows/win32/services/modifying-the-dacl-for-a-service
{
    EXPLICIT_ACCESS      ea = {0};
    SECURITY_DESCRIPTOR  sd;
    PSECURITY_DESCRIPTOR psd = NULL;
    PACL                 pacl = NULL;
    PACL                 pNewAcl = NULL;
    BOOL                 bDaclPresent = FALSE;
    BOOL                 bDaclDefaulted = FALSE;
    DWORD                dwError = 0;
    DWORD                dwSize = 0;
    DWORD                dwBytesNeeded = 0;
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the service
    schService = OpenService(
        schSCManager,              // SCManager database 
        szSvcName,                 // name of service 
        READ_CONTROL | WRITE_DAC); // access
    if (schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Get the current security descriptor.
    if (!QueryServiceObjectSecurity(schService,
                                    DACL_SECURITY_INFORMATION,
                                    &psd,           // using NULL does not work on all versions
                                    0,
                                    &dwBytesNeeded)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            dwSize = dwBytesNeeded;
            psd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
            if (psd == NULL) {
                // Note: HeapAlloc does not support GetLastError.
                printf("HeapAlloc failed\n");
                goto dacl_cleanup;
            }

            if (!QueryServiceObjectSecurity(schService,
                                            DACL_SECURITY_INFORMATION, psd, dwSize, &dwBytesNeeded)) {
                printf("QueryServiceObjectSecurity failed (%d)\n", GetLastError());
                goto dacl_cleanup;
            }
        } else {
            printf("QueryServiceObjectSecurity failed (%d)\n", GetLastError());
            goto dacl_cleanup;
        }
    }

    // Get the DACL.
    if (!GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl, &bDaclDefaulted)) {
        printf("GetSecurityDescriptorDacl failed(%d)\n", GetLastError());
        goto dacl_cleanup;
    }

    // Build the ACE.
    BuildExplicitAccessWithName(&ea, (LPWSTR)TEXT("GUEST"),
                                SERVICE_START | SERVICE_STOP | READ_CONTROL | DELETE,
                                SET_ACCESS, NO_INHERITANCE);
    dwError = SetEntriesInAcl(1, &ea, pacl, &pNewAcl);
    if (dwError != ERROR_SUCCESS) {
        printf("SetEntriesInAcl failed(%d)\n", dwError);
        goto dacl_cleanup;
    }

    // Initialize a new security descriptor.
    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        printf("InitializeSecurityDescriptor failed(%d)\n", GetLastError());
        goto dacl_cleanup;
    }

    // Set the new DACL in the security descriptor.
    if (!SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE)) {
        printf("SetSecurityDescriptorDacl failed(%d)\n", GetLastError());
        goto dacl_cleanup;
    }

    // Set the new DACL for the service object.
    if (!SetServiceObjectSecurity(schService, DACL_SECURITY_INFORMATION, &sd)) {
        printf("SetServiceObjectSecurity failed(%d)\n", GetLastError());
        goto dacl_cleanup;
    } else printf("Service DACL updated successfully\n");

dacl_cleanup:
    CloseServiceHandle(schSCManager);
    CloseServiceHandle(schService);

    if (NULL != pNewAcl)
        LocalFree((HLOCAL)pNewAcl);
    if (NULL != psd)
        HeapFree(GetProcessHeap(), 0, (LPVOID)psd);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
