#include "pch.h"
#include "Process.h"
#include "Module.h"
#include "Thread.h"
#include <shellapi.h>


#pragma warning(disable:6387)
#pragma warning(disable:6328)
#pragma warning(disable:4311)
#pragma warning(disable:4302)
#pragma warning(disable:6230)
#pragma warning(disable:6216)


//////////////////////////////////////////////////////////////////////////////////////////////////


// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS and compile with -DPSAPI_VERSION=1


void PrintProcessNameAndID(DWORD processID)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL != hProcess) {// Get a handle to the process.
        HMODULE hMod;
        DWORD cbNeeded;
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {// Get the process name.
            GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
        }
    }

    // Print the process name and identifier.
    _tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);

    CloseHandle(hProcess);// Release the handle to the process.
}


EXTERN_C
__declspec(dllexport)
int WINAPI EnumeratingAllProcesses(void)
/*
Enumerating All Processes
2018/05/31

The following sample code uses the EnumProcesses function to enumerate the current processes in the system.

https://docs.microsoft.com/zh-cn/windows/win32/psapi/enumerating-all-processes
*/
{
    // Get the list of process identifiers.
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return 1;
    }

    cProcesses = cbNeeded / sizeof(DWORD);// Calculate how many process identifiers were returned.

    // Print the name and process identifier for each process.
    for (unsigned int i = 0; i < cProcesses; i++) {
        if (aProcesses[i] != 0) {
            PrintProcessNameAndID(aProcesses[i]);
        }
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS 
// and compile with -DPSAPI_VERSION=1

#define ARRAY_SIZE 1024


EXTERN_C
__declspec(dllexport)
int WINAPI EnumeratingAllDeviceDrivers(void)
/*
Enumerating All Device Drivers in the System
2018/05/31

The following sample code uses the EnumDeviceDrivers function to enumerate the current device drivers in the system.
It passes the load addresses retrieved from this function call to the GetDeviceDriverBaseName function to retrieve a name that can be displayed.

https://docs.microsoft.com/zh-cn/windows/win32/psapi/enumerating-all-device-drivers-in-the-system
*/
{
    LPVOID drivers[ARRAY_SIZE];
    DWORD cbNeeded;
    int cDrivers, i;

    if (EnumDeviceDrivers(drivers, sizeof(drivers), &cbNeeded) && cbNeeded < sizeof(drivers)) {
        TCHAR szDriver[ARRAY_SIZE];

        cDrivers = cbNeeded / sizeof(drivers[0]);

        _tprintf(TEXT("There are %d drivers:\n"), cDrivers);
        for (i = 0; i < cDrivers; i++) {
            if (GetDeviceDriverBaseName(drivers[i], szDriver, sizeof(szDriver) / sizeof(szDriver[0]))) {
                _tprintf(TEXT("%d: %s\n"), i + 1, szDriver);
            }
        }
    } else {
        _tprintf(TEXT("EnumDeviceDrivers failed; array size needed is %zd\n"), cbNeeded / sizeof(LPVOID));
        return 1;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
BOOL WINAPI IsWow64()
{
    BOOL bIsWow64 = FALSE;

#ifdef _WIN64    
    return false;// 64-bit code, obviously not running in a 32-bit process
#endif

#pragma warning(push)
#pragma warning(disable:4702)
    HMODULE ModuleHandle = GetModuleHandle(TEXT("kernel32"));
    if (NULL != ModuleHandle) {
        LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(ModuleHandle,
                                                                                   "IsWow64Process");
        if (NULL != fnIsWow64Process) {
            if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) {
                // handle error
            }
        }
    }

    return bIsWow64;
#pragma warning(pop)
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
BOOL WINAPI GetProcessList()
/*
Taking a Snapshot and Viewing Processes
05/31/2018

The following simple console application obtains a list of running processes.
First, the GetProcessList function takes a snapshot of currently executing processes in the system using CreateToolhelp32Snapshot,
and then it walks through the list recorded in the snapshot using Process32First and Process32Next.
For each process in turn, GetProcessList calls the ListProcessModules function which is described in Traversing the Module List,
and the ListProcessThreads function which is described in Traversing the Thread List.

A simple error-reporting function, printError, displays the reason for any failures,
which usually result from security restrictions.
For example, OpenProcess fails for the Idle and CSRSS processes because their access restrictions prevent user-level code from opening them.

https://docs.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes
*/
{
    // Take a snapshot of all processes in the system.
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
        return(FALSE);
    }

    // Retrieve information about the first process, and exit if unsuccessful
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);// Set the size of the structure before using it.
    if (!Process32First(hProcessSnap, &pe32)) {
        printError(TEXT("Process32First")); // show cause of failure
        CloseHandle(hProcessSnap);          // clean the snapshot object
        return(FALSE);
    }

    // Now walk the snapshot of processes, and display information about each process in turn
    do {
        _tprintf(TEXT("\n\n====================================================="));
        _tprintf(TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile);
        _tprintf(TEXT("\n-------------------------------------------------------"));

        // Retrieve the priority class.
        DWORD dwPriorityClass = 0;
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
        if (hProcess == NULL) {
            printError(TEXT("OpenProcess"));
        } else {
            dwPriorityClass = GetPriorityClass(hProcess);
            if (!dwPriorityClass)
                printError(TEXT("GetPriorityClass"));
            CloseHandle(hProcess);
        }

        _tprintf(TEXT("\n  Process ID        = 0x%08X"), pe32.th32ProcessID);
        _tprintf(TEXT("\n  Thread count      = %d"), pe32.cntThreads);
        _tprintf(TEXT("\n  Parent process ID = 0x%08X"), pe32.th32ParentProcessID);
        _tprintf(TEXT("\n  Priority base     = %d"), pe32.pcPriClassBase);
        if (dwPriorityClass)
            _tprintf(TEXT("\n  Priority class    = %d"), dwPriorityClass);

        // List the modules and threads associated with this process
        ListProcessModules(pe32.th32ProcessID);
        ListProcessThreads(pe32.th32ProcessID);
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return(TRUE);
}


EXTERN_C
__declspec(dllexport)
DWORD WINAPI GetProcessName(_In_ DWORD ProcessID, PWCHAR ProcessName)
/*
功能：获取一个（在线的，非退出的）进程的进程名（非进程的完整路径）。

参数：
ProcessName的元素大小 >= pe32.szExeFile的MAX_PATH。

注释：这个总是能获取到的，只要它没有退出。
      但是获取到的仅仅是进程名，非完整，不能据此获取进程的IMAGE，主要作用是显示。
*/
{
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    // Retrieve information about the first process
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return GetLastError();
    }

    // Now walk the snapshot of processes
    do {
        if (ProcessID == pe32.th32ProcessID) {
            lstrcpy(ProcessName, pe32.szExeFile);
            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return GetLastError();
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
void WINAPI CreatingProcesses(int argc, TCHAR * argv[])
/*
Creating Processes
05/31/2018

The CreateProcess function creates a new process, which runs independently of the creating process.
However, for simplicity, the relationship is referred to as a parent-child relationship.

The following code demonstrates how to create a process.

https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes
http://msdn.microsoft.com/en-us/library/ms682512(v=vs.85).aspx
*/
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (argc != 2) {
        printf("Usage: %ls [cmdline]\n", argv[0]);
        return;
    }

    // Start the child process. 
    if (!CreateProcess(NULL,   // No module name (use command line)
                       argv[1],        // Command line
                       NULL,           // Process handle not inheritable
                       NULL,           // Thread handle not inheritable
                       FALSE,          // Set handle inheritance to FALSE
                       0,              // No creation flags
                       NULL,           // Use parent's environment block
                       NULL,           // Use parent's starting directory 
                       &si,            // Pointer to STARTUPINFO structure
                       &pi)           // Pointer to PROCESS_INFORMATION structure
        ) {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);// Wait until child process exits.

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
void WINAPI ShowProcessIntegrityLevel()
/*
Appendix D: Getting the Integrity Level for an Access Token
07/05/2007

Use the GetTokenInformation API to retrieve the access token integrity level from the access token.
GetTokenInformation has a parameter to indicate what access token information class to retrieve.
The TOKEN_INFORMATION_CLASS parameter has a defined value for the integrity level, TokenIntegrityLevel.
GetTokenInformation returns a TOKEN_MANDATORY_LABEL data structure.

To determine the integrity level of a process
1.Open a handle to the access token of the current process.
2.Get the integrity level of the access token.
3.Compare the integrity level SID to the system-defined integrity level RIDs.

The following code sample shows how to do this.

https://msdn.microsoft.com/en-us/library/bb250462(v=vs.85).aspx
https://docs.microsoft.com/en-us/previous-versions/dotnet/articles/bb625966(v=msdn.10)
*/
{
    HANDLE hToken;
    HANDLE hProcess;
    DWORD dwLengthNeeded;
    DWORD dwError = ERROR_SUCCESS;
    PTOKEN_MANDATORY_LABEL pTIL = NULL;
    //LPWSTR pStringSid;
    DWORD dwIntegrityLevel;

    hProcess = GetCurrentProcess();
    if (OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_QUERY_SOURCE, &hToken)) {
        // Get the Integrity level.
        if (!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwLengthNeeded)) {
            dwError = GetLastError();
            if (dwError == ERROR_INSUFFICIENT_BUFFER) {
                pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc(0, dwLengthNeeded);
                if (pTIL != NULL) {
                    if (GetTokenInformation(hToken, TokenIntegrityLevel,
                                            pTIL, dwLengthNeeded, &dwLengthNeeded)) {
                        dwIntegrityLevel = *GetSidSubAuthority(pTIL->Label.Sid,
                                                               (DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1));
                        if (dwIntegrityLevel < SECURITY_MANDATORY_MEDIUM_RID) {
                            wprintf(L"Low Process");// Low Integrity
                        } else if (dwIntegrityLevel >= SECURITY_MANDATORY_MEDIUM_RID &&
                                   dwIntegrityLevel < SECURITY_MANDATORY_HIGH_RID) {
                            wprintf(L"Medium Process");// Medium Integrity
                        } else if (dwIntegrityLevel >= SECURITY_MANDATORY_HIGH_RID) {
                            wprintf(L"High Integrity Process");// High Integrity
                        }
                    }

                    LocalFree(pTIL);
                }
            }
        }

        CloseHandle(hToken);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI ShellExecuteExApp()
/*
A Simple Example of How to Use ShellExecuteEx
The following sample console application illustrates the use of ShellExecuteEx.
Most error checking code has been omitted for clarity.

https://docs.microsoft.com/en-us/windows/win32/shell/launch
*/
{
    LPITEMIDLIST pidlWinFiles = NULL;
    LPITEMIDLIST pidlItems = NULL;
    IShellFolder * psfWinFiles = NULL;
    IShellFolder * psfDeskTop = NULL;
    LPENUMIDLIST ppenum = NULL;
    STRRET strDispName;
    TCHAR pszParseName[MAX_PATH];
    ULONG celtFetched;
    SHELLEXECUTEINFO ShExecInfo;
    HRESULT hr;
    BOOL fBitmap = FALSE;

    hr = SHGetFolderLocation(NULL, CSIDL_WINDOWS, NULL, NULL, &pidlWinFiles);

    hr = SHGetDesktopFolder(&psfDeskTop);

    hr = psfDeskTop->BindToObject(pidlWinFiles, NULL, IID_IShellFolder, (LPVOID *)&psfWinFiles);
    hr = psfDeskTop->Release();

    hr = psfWinFiles->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &ppenum);

    while (hr = ppenum->Next(1, &pidlItems, &celtFetched) == S_OK && (celtFetched) == 1) {
        psfWinFiles->GetDisplayNameOf(pidlItems, SHGDN_FORPARSING, &strDispName);
        StrRetToBuf(&strDispName, pidlItems, pszParseName, MAX_PATH);
        CoTaskMemFree(pidlItems);
        if (StrCmpI(PathFindExtension(pszParseName), TEXT(".bmp")) == 0) {
            fBitmap = TRUE;
            break;
        }
    }

    ppenum->Release();

    if (fBitmap) {
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = NULL;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = NULL;
        ShExecInfo.lpFile = pszParseName;
        ShExecInfo.lpParameters = NULL;
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_MAXIMIZE;
        ShExecInfo.hInstApp = NULL;

        ShellExecuteEx(&ShExecInfo);
    }

    CoTaskMemFree(pidlWinFiles);
    psfWinFiles->Release();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


void GetSystemStartTime(char * SystemStartTime)
/*
功能：获取操作系统启动时间。

注意：存储的格式是unix时间戳。
*/
{
    time_t nowtime;
    time(&nowtime);

    /*
    注意：这两个操作之前不可有任何操作，否者影响计算的结果，包括调试也不行。
    */

    time_t StartTime = nowtime - (GetTickCount64() / 1000);

    /*
    时间戳转换为本地时间。
    */
    FILETIME fileTime;
    TimeStampToFileTime(StartTime, fileTime);

    FILETIME ftLocal;
    FileTimeToLocalFileTime(&fileTime, &ftLocal);

    FileTimeToTimeStamp(ftLocal, StartTime);

    //满足本工程的需求。
    wsprintfA(SystemStartTime, "%I64u", StartTime);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


BOOL GetProcessIntegrityLevel(PDWORD pdwIntegrityLevel)
//代码原始出处：Create low-integrity process in C++ (CppCreateLowIntegrityProcess)
//   PURPOSE: The function gets the integrity level of the current process. 
//   Integrity level is only available on Windows Vista and newer operating systems, 
//   thus GetProcessIntegrityLevel returns FALSE if it is called on on systems prior to Windows Vista.
//
//   PARAMETERS: 
//   * pdwIntegrityLevel - Outputs the integrity level of the current process. It is usually one of these values:
//
//     SECURITY_MANDATORY_UNTRUSTED_RID (SID: S-1-16-0x0)
//     Means untrusted level. It is used by processes started by the Anonymous group. Blocks most write access. 
//
//     SECURITY_MANDATORY_LOW_RID (SID: S-1-16-0x1000)
//     Means low integrity level. It is used by Protected Mode Internet Explorer. 
//     Blocks write acess to most objects (such as files and registry keys) on the system. 
//
//     SECURITY_MANDATORY_MEDIUM_RID (SID: S-1-16-0x2000)
//     Means medium integrity level. It is used by normal applications being launched while UAC is enabled. 
//
//     SECURITY_MANDATORY_HIGH_RID (SID: S-1-16-0x3000)
//     Means high integrity level. It is used by administrative applications launched through elevation when UAC is enabled, 
//     or normal applications if UAC is disabled and the user is an administrator. 
//
//     SECURITY_MANDATORY_SYSTEM_RID (SID: S-1-16-0x4000)
//     Means system integrity level. It is used by services and other system-level applications (such as Wininit, Winlogon, Smss, etc.)  
//
//   RETURN VALUE: If the function succeeds, the return value is TRUE. 
//   If the function fails, the return value is FALSE. To get extended error information, call GetLastError. 
//   For example, ERROR_INVALID_PARAMETER is the last error if GetProcessIntegrityLevel is called on systems prior to Windows Vista or pdwIntegrityLevel is NULL.
//
//   EXAMPLE CALL:
//     DWORD dwIntegrityLevel;
//     if (!GetProcessIntegrityLevel(&dwIntegrityLevel))
//     {
//         wprintf(L"GetProcessIntegrityLevel failed w/err %lu\n", 
//             GetLastError());
//     }
{
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    DWORD cbTokenIL = 0;
    PTOKEN_MANDATORY_LABEL pTokenIL = NULL;

    if (pdwIntegrityLevel == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    // Open the primary access token of the process with TOKEN_QUERY.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Query the size of the token integrity level information. 
    // Note that we expect a FALSE result and the last error ERROR_INSUFFICIENT_BUFFER from GetTokenInformation because we have given it a NULL buffer. 
    // On exit cbTokenIL will tell the size of the integrity level information.
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &cbTokenIL)) {
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
            // When the process is run on operating systems prior to Windows Vista, GetTokenInformation returns FALSE with the 
            // ERROR_INVALID_PARAMETER error code because TokenElevation is not supported on those operating systems.
            dwError = GetLastError();
            goto Cleanup;
        }
    }

    // Now we allocate a buffer for the integrity level information.
    pTokenIL = (TOKEN_MANDATORY_LABEL *)LocalAlloc(LPTR, cbTokenIL);
    if (pTokenIL == NULL) {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Retrieve token integrity level information.
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, pTokenIL, cbTokenIL, &cbTokenIL)) {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Integrity Level SIDs are in the form of S-1-16-0xXXXX. (e.g. S-1-16-0x1000 stands for low integrity level SID). 
    // There is one and only one subauthority.
    *pdwIntegrityLevel = *GetSidSubAuthority(pTokenIL->Label.Sid, 0);

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken) {
        CloseHandle(hToken);
        hToken = NULL;
    }
    if (pTokenIL) {
        LocalFree(pTokenIL);
        pTokenIL = NULL;
        cbTokenIL = 0;
    }

    if (ERROR_SUCCESS != dwError) {
        SetLastError(dwError);// Make sure that the error code is set for failure.
        return FALSE;
    } else {
        return TRUE;
    }
}


BOOL CreateLowIntegrityProcess(PWSTR pszCommandLine)
//代码原始出处：Create low-integrity process in C++ (CppCreateLowIntegrityProcess)
//   PURPOSE: The function launches an application at low integrity level. 
//
//   PARAMETERS:
//   * pszCommandLine - The command line to be executed. 
//     The maximum length of this string is 32K characters. 
//     This parameter cannot be a pointer to read-only memory (such as a const variable or a literal string). 
//     If this parameter is a constant string, the function may cause an access violation.
//
//   RETURN VALUE: If the function succeeds, the return value is TRUE. 
//   If the function fails, the return value is zero. 
//   To get extended error information, call GetLastError.
//
//   COMMENT:
//   To start a low-integrity process, 
//   1) Duplicate the handle of the current process, which is at medium integrity level.
//   2) Use SetTokenInformation to set the integrity level in the access token to Low.
//   3) Use CreateProcessAsUser to create a new process using the handle to the low integrity access token.
{
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    HANDLE hNewToken = NULL;
    SID_IDENTIFIER_AUTHORITY MLAuthority = SECURITY_MANDATORY_LABEL_AUTHORITY;
    PSID pIntegritySid = NULL;
    TOKEN_MANDATORY_LABEL tml = {0};
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};

    // Open the primary access token of the process.
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_DUPLICATE | TOKEN_QUERY | TOKEN_ADJUST_DEFAULT | TOKEN_ASSIGN_PRIMARY,
                          &hToken)) {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Duplicate the primary token of the current process.
    if (!DuplicateTokenEx(hToken, 0, NULL, SecurityImpersonation, TokenPrimary, &hNewToken)) {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Create the low integrity SID.
    if (!AllocateAndInitializeSid(&MLAuthority, 1, SECURITY_MANDATORY_LOW_RID, 0, 0, 0, 0, 0, 0, 0, &pIntegritySid)) {
        dwError = GetLastError();
        goto Cleanup;
    }

    tml.Label.Attributes = SE_GROUP_INTEGRITY;
    tml.Label.Sid = pIntegritySid;

    // Set the integrity level in the access token to low.
    if (!SetTokenInformation(hNewToken, TokenIntegrityLevel, &tml, (sizeof(tml) + GetLengthSid(pIntegritySid)))) {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Create the new process at the Low integrity level.
    if (!CreateProcessAsUser(hNewToken, NULL, pszCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken) {
        CloseHandle(hToken);
        hToken = NULL;
    }
    if (hNewToken) {
        CloseHandle(hNewToken);
        hNewToken = NULL;
    }
    if (pIntegritySid) {
        FreeSid(pIntegritySid);
        pIntegritySid = NULL;
    }
    if (pi.hProcess) {
        CloseHandle(pi.hProcess);
        pi.hProcess = NULL;
    }
    if (pi.hThread) {
        CloseHandle(pi.hThread);
        pi.hThread = NULL;
    }

    if (ERROR_SUCCESS != dwError) {
        SetLastError(dwError);// Make sure that the error code is set for failure.
        return FALSE;
    } else {
        return TRUE;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////


BOOL IsProcessElevated()
//摘自：UAC self-elevation (CppUACSelfElevation)
//   PURPOSE: The function gets the elevation information of the current process. 
//   It dictates whether the process is elevated or not. 
//   Token elevation is only available on Windows Vista and newer operating systems, 
//   thus IsProcessElevated throws a C++ exception if it is called on systems prior to Windows Vista. 
//   It is not appropriate to use this function to determine whether a process is run as administartor.
//
//   RETURN VALUE: Returns TRUE if the process is elevated. Returns FALSE if it is not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception which contains the Win32 error code of the failure. 
//   For example, if IsProcessElevated is called on systems prior to Windows Vista, the error code will be ERROR_INVALID_PARAMETER.
//
//   NOTE: TOKEN_INFORMATION_CLASS provides TokenElevationType to check the elevation type (TokenElevationTypeDefault / TokenElevationTypeLimited / TokenElevationTypeFull) of the process. 
//   It is different from TokenElevation in that, when UAC is turned off, 
//   elevation type always returns TokenElevationTypeDefault even though the process is elevated (Integrity Level == High). 
//   In other words, it is not safe to say if the process is elevated based on elevation type. 
//   Instead, we should use TokenElevation.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsProcessElevated())
//             wprintf (L"Process is elevated\n");
//         else
//             wprintf (L"Process is not elevated\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsProcessElevated failed w/err %lu\n", dwError);
//     }
{
    BOOL fIsElevated = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;

    // Open the primary access token of the process with TOKEN_QUERY.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Retrieve token elevation information.
    TOKEN_ELEVATION elevation;
    DWORD dwSize;
    if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
        // When the process is run on operating systems prior to Windows Vista,
        // GetTokenInformation returns FALSE with the ERROR_INVALID_PARAMETER error code because TokenElevation is not supported on those operating systems.
        dwError = GetLastError();
        goto Cleanup;
    }

    fIsElevated = elevation.TokenIsElevated;

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken) {
        CloseHandle(hToken);
        hToken = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError) {
        throw dwError;
    }

    return fIsElevated;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
