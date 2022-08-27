#pragma once


/////////////////////////////////////////////////////////////////////////////////////////////////
//一些系统的头文件和库的包含。


//#define _WIN32_WINNT 0x0501

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#ifndef UNICODE
#define UNICODE
#endif

#include <Winsock2.h>
#include <windows.h>
#include <strsafe.h>
#include <assert.h>
#include <crtdbg.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <winioctl.h>
#include <string.h>
#include <fltuser.h>
#include <locale.h>
#include <Lmserver.h>
#include <stdarg.h>
#include <wincrypt.h>
#include <intrin.h>
#include <TlHelp32.h>
#include <aclapi.h>
#include <VersionHelpers.h>
#include <ShlDisp.h>
#include <Shlobj.h>
#include <Softpub.h>
#include <mscat.h>
//#include <SubAuth.h>
//#include <LsaLookup.h>
#include <WinUser.h>
#include <direct.h>
#include <sddl.h>
#include <ws2tcpip.h>
#include <fwpsu.h>
#include <atlbase.h>
#include <mbnapi.h>
#include <iostream>
#include <netfw.h>
#include <atlcomcli.h>
#include <objbase.h>
#include <oleauto.h>
#include <atlconv.h>
#define _WS2DEF_
#include <mstcpip.h>
#include <Intshcut.h>
//#include <winternl.h>
#include <SubAuth.h>
//#include <NTSecAPI.h>
//#include <ntdef.h>
//#include <netioapi.h>
#include <atlstr.h>
#include <comutil.h>
#include <wbemidl.h>
#include <dbt.h>
#include <lm.h>
#include <winnetwk.h>
#include <ws2spi.h>
#include <comdef.h>

#include <initguid.h> //注意前后顺序。静态定义UUID用的，否则：error LNK2001。
#include <usbioctl.h>
#include <usbiodef.h>
//#include <usbctypes.h>
#include <intsafe.h>
#include <specstrings.h>
#include <usb.h>
#include <usbuser.h>

#include <wincon.h> 
#include <time.h> 
#include <fwpmu.h>
#include <conio.h>
#include <nb30.h>

#pragma comment(lib, "fwpuclnt.lib") 
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "mpr.lib")
#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Version.lib") 
//#pragma comment (lib,"Url.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib,"Netapi32.lib")

#include <taskschd.h>
#pragma comment(lib, "taskschd.lib")

#include <bcrypt.h>
#pragma comment (lib, "Bcrypt.lib")

#include <shellapi.h>
#pragma comment (lib, "Shell32.lib")

#include <ncrypt.h>
#pragma comment (lib, "Ncrypt.lib")

#include <wintrust.h>
#pragma comment (lib, "wintrust.lib")

#include <Setupapi.h>
#pragma comment (lib,"Setupapi.lib")

#include <Shlwapi.h>
#pragma comment (lib,"Shlwapi.lib")

#include <DbgHelp.h>
#pragma comment (lib,"DbgHelp.lib")

#include <psapi.h>
#pragma comment(lib, "Psapi.lib")

#include <Sfc.h>
#pragma comment(lib, "Sfc.lib")

//#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

#include <Wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")

#include <Userenv.h>
#pragma comment(lib, "Userenv.lib")

#include <Sensapi.h>
#pragma comment (lib,"Sensapi.lib")

#include <string>
#include <list>
#include <regex>
#include <map>
#include <set>

using namespace std;


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C_START


//////////////////////////////////////////////////////////////////////////////////////////////////


__declspec(dllimport) int WINAPI GetEnvironment();

__declspec(dllimport) int WINAPI GetProcessHeapsInfo();
__declspec(dllimport) int WINAPI TraversingHeapList(void);
__declspec(dllimport) int WINAPI EnumeratingHeap(void);

__declspec(dllimport) int WINAPI CreatingViewWithinFile(void);
__declspec(dllimport) int WINAPI CreatingNamedSharedMemory(void);
__declspec(dllimport) int WINAPI CreatingFileMappingUsingLargePages(void);
__declspec(dllimport) int WINAPI ObtainingFileNameFromFileHandle(int argc, TCHAR * argv[]);

__declspec(dllimport) void WINAPI AWE();
__declspec(dllimport) void WINAPI NUMA(int argc, const wchar_t * argv[]);
__declspec(dllimport) int WINAPI CreatingGuardPages();
__declspec(dllimport) VOID WINAPI ReservingCommittingMemory(VOID);

__declspec(dllimport) int WINAPI CollectingMemoryUsageInformationForProcess();

__declspec(dllimport) int WINAPI EnumeratingAllProcesses();

__declspec(dllimport) int WINAPI ShellExecInExplorerProcess(PCWSTR pszFile);

__declspec(dllimport) int WINAPI EnumeratingAllModulesForProcess();

__declspec(dllimport) int WINAPI EnumeratingAllDeviceDrivers();

__declspec(dllimport) BOOL WINAPI IsWow64();

__declspec(dllimport) BOOL WINAPI IsWow64Process2Ex(_In_ DWORD Pid);


//////////////////////////////////////////////////////////////////////////////////////////////////
//服务相关的函数。


__declspec(dllimport) BOOL WINAPI DriverInstallInf(PWSTR InfFile);
__declspec(dllimport) BOOL WINAPI DriverInstall(_In_opt_ LPCWSTR BinaryPathName, _In_ LPCWSTR ServiceName);
__declspec(dllimport) VOID WINAPI SvcInstall(_In_opt_ LPCWSTR BinaryPathName, _In_ LPCWSTR ServiceName, _In_opt_ LPCWSTR DisplayName);
__declspec(dllimport) VOID __stdcall StartSvc(_In_ LPCWSTR ServiceName);
__declspec(dllimport) BOOL __stdcall StopDependentServices(_In_ LPCWSTR ServiceName);
__declspec(dllimport) VOID __stdcall StopSvc(_In_ LPCWSTR ServiceName);
__declspec(dllimport) VOID __stdcall DeleteSvc(_In_ LPCWSTR ServiceName);
__declspec(dllimport) VOID __stdcall QuerySvc(_In_ LPCWSTR ServiceName);
__declspec(dllimport) VOID __stdcall DisableSvc(_In_ LPCWSTR szSvcName);
__declspec(dllimport) VOID __stdcall EnableSvc(_In_ LPCWSTR szSvcName);
__declspec(dllimport) VOID __stdcall UpdateSvcDesc(_In_ LPCWSTR szSvcName);
__declspec(dllimport) VOID __stdcall UpdateSvcDacl(_In_ LPCWSTR szSvcName);


//////////////////////////////////////////////////////////////////////////////////////////////////


__declspec(dllimport) BOOL WINAPI GetProcessList();

__declspec(dllimport) DWORD WINAPI GetProcessName(_In_ DWORD ProcessID, PWCHAR ProcessName);

__declspec(dllimport) BOOL WINAPI ListProcessThreads(DWORD dwOwnerPID);

__declspec(dllimport) BOOL WINAPI ListProcessModules(DWORD dwPID);

__declspec(dllimport) void CALLBACK RunDllApi(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllimport) int WINAPI TraversingHeapList2(void);

__declspec(dllimport) void WINAPI CreatingProcesses(int argc, TCHAR * argv[]);

__declspec(dllimport) int WINAPI CreatingThreads();

__declspec(dllimport) BOOL WINAPI SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege   // to enable or disable privilege
);

__declspec(dllimport) BOOL WINAPI AdjustCurrentProcessPrivilege(PCTSTR szPrivilege, BOOL fEnable);

__declspec(dllimport) DWORD WINAPI EnumerateProcessHandles(ULONG pid);

__declspec(dllimport) void WINAPI SetLowLabelToFile();

__declspec(dllimport) void WINAPI CreateLowProcess();

__declspec(dllimport) int WINAPI RemoveShutdownPrivilegeStartProcess();

__declspec(dllimport) void WINAPI ShowProcessIntegrityLevel();

__declspec(dllimport) BOOL WINAPI GetCurrentUserAndDomain(PTSTR szUser, PDWORD pcchUser, PTSTR szDomain, PDWORD pcchDomain);

__declspec(dllimport) BOOL WINAPI GetLogonSID(HANDLE hToken, PSID * ppsid);

__declspec(dllimport) bool WINAPI GetCurrentToken(OUT PHANDLE hToken);

__declspec(dllimport) VOID WINAPI FreeLogonSID(PSID * ppsid);

__declspec(dllimport) BOOL WINAPI IsCurrentSessionRemoteable();

__declspec(dllimport) void WINAPI EnumerateSessions();

__declspec(dllimport) bool WINAPI IsRemoteSession(DWORD SessionId);

__declspec(dllimport) void WINAPI EnumerateSessionsEx();

__declspec(dllimport) int WINAPI EnumerateLogonSessions();

__declspec(dllimport) BOOL WINAPI StartInteractiveClientProcess(
    LPTSTR lpszUsername,    // client to log on
    LPTSTR lpszDomain,      // domain of client's account
    LPTSTR lpszPassword,    // client's password
    LPTSTR lpCommandLine    // command line to execute
);

__declspec(dllimport) HRESULT WINAPI GetSid(LPCWSTR wszAccName, PSID * ppSid);

__declspec(dllimport) int WINAPI GetCurrentSid();

__declspec(dllimport) BOOL WINAPI SearchTokenGroupsForSID(VOID);

__declspec(dllimport) int WINAPI EnumerateAccountRights(int argc, char * argv[]);

__declspec(dllimport) BOOL WINAPI MySystemShutdown();

__declspec(dllimport) BOOL WINAPI MySystemShutdownWithDialogbox(LPCTSTR lpMsg);

__declspec(dllimport) BOOL WINAPI PreventSystemShutdown();

__declspec(dllimport) int WINAPI EnumCred();

__declspec(dllimport) BOOL WINAPI IsUserAdmin(VOID);

__declspec(dllimport) BOOL WINAPI IsUserAnSystem(VOID);

__declspec(dllimport) BOOL WINAPI IsCurrentUserLocalAdministrator(VOID);

__declspec(dllimport) int WINAPI ManageUserPrivileges(int argc, char * argv[]);


//////////////////////////////////////////////////////////////////////////////////////////////////


__declspec(dllimport) void WINAPI CreateProcessWithLogon(int argc, WCHAR * argv[]);

__declspec(dllimport) int WINAPI ShellExecuteExApp();

__declspec(dllimport) HANDLE WINAPI GetParentsPid(_In_ HANDLE UniqueProcessId);

__declspec(dllimport) bool WINAPI IsWow64ProcessEx(_In_ HANDLE ProcessHandle);

__declspec(dllimport) DWORD WINAPI GetCommandLineEx(_In_ HANDLE UniqueProcessId);

__declspec(dllimport) 
NTSTATUS WINAPI ZwEnumerateDirectoryObject(_In_ PCWSTR ObjectDirectory, _In_ PCWSTR TypeNameFilter);

__declspec(dllimport) void WINAPI QuerySymbolicLinkName(_In_ PCWSTR SymbolicLink);


//////////////////////////////////////////////////////////////////////////////////////////////////
//计划任务相关。


__declspec(dllimport)
int WINAPI EnumTaskScheduler();

__declspec(dllimport)
int WINAPI StartingExecutableAtSpecificTime();

__declspec(dllimport)
int WINAPI StartingExecutableDaily();

__declspec(dllimport)
int WINAPI StartingExecutableWhenTaskRegistered();

__declspec(dllimport)
int WINAPI StartingExecutableWeekly();

__declspec(dllimport)
int WINAPI StartingExecutableWhenUserLogsOn();

__declspec(dllimport)
int WINAPI StartingExecutableOnSystemBoot();

__declspec(dllimport)
int WINAPI EventTriggerExample();

__declspec(dllimport)
int WINAPI EnumeratingTasks();

__declspec(dllimport)
int WINAPI CreatingTaskUsingNewWorkItem(int argc, char ** argv);

__declspec(dllimport)
int WINAPI EnumeratingTasksOne(int argc, char ** argv);

__declspec(dllimport)
int WINAPI StartingTask(int argc, char ** argv);

__declspec(dllimport)
int WINAPI EditingWorkItem(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskAccountInformation(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskComment(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskCreator(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskExitCode(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskIdleWaitTime(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskMostRecentRunTime(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskNextRunTime(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskRunTimes(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskStatus(int argc, char ** argv);

__declspec(dllimport)
int WINAPI SettingTaskAccountInformation(int argc, char ** argv);

__declspec(dllimport)
int WINAPI SettingTaskComment(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskApplicationName(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskMaxRunTime(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskParameters(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskPriority(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskWorkingDirectory(int argc, char ** argv);

__declspec(dllimport)
int WINAPI SettingApplicationName(int argc, char ** argv);

__declspec(dllimport)
int WINAPI SettingMaxRunTime(int argc, char ** argv);

__declspec(dllimport)
int WINAPI SettingTaskParameters(int argc, char ** argv);

__declspec(dllimport)
int WINAPI SettingTaskPriority(int argc, char ** argv);

__declspec(dllimport)
int WINAPI SettingWorkingDirectory(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTaskPage(int argc, char ** argv);

__declspec(dllimport)
int WINAPI CreatingTaskTrigger(int argc, char ** argv);

__declspec(dllimport)
int WINAPI CreatingIdleTrigger(int argc, char ** argv);

__declspec(dllimport)
int WINAPI TerminatingTask(int argc, char ** argv);

__declspec(dllimport)
int WINAPI RetrievingTriggerStrings(int argc, char ** argv);



//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C_END


//////////////////////////////////////////////////////////////////////////////////////////////////
