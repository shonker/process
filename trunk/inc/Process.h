#pragma once


/////////////////////////////////////////////////////////////////////////////////////////////////
//һЩϵͳ��ͷ�ļ��Ϳ�İ�����


//#define _WIN32_WINNT 0x0501

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>


//////////////////////////////////////////////////////////////////////////////////////////////////


//C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\wincred.h
#ifndef _NTDEF_
typedef _Return_type_success_(return >= 0) LONG NTSTATUS, * PNTSTATUS;
#endif


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
//������صĺ�����


__declspec(dllimport) BOOL WINAPI DriverInstallInf(PWSTR InfFile);
__declspec(dllimport) BOOL WINAPI DriverInstall(_In_opt_ LPCWSTR BinaryPathName, _In_ LPCWSTR ServiceName);
__declspec(dllimport) VOID WINAPI SvcInstall(_In_opt_ LPCWSTR BinaryPathName,
                                             _In_ LPCWSTR ServiceName, 
                                             _In_opt_ LPCWSTR DisplayName);
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

__declspec(dllimport) void WINAPI CreatingProcesses(int argc, WCHAR * argv[]);

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

__declspec(dllimport) BOOL WINAPI GetCurrentUserAndDomain(PTSTR szUser,
                                                          PDWORD pcchUser,
                                                          PTSTR szDomain,
                                                          PDWORD pcchDomain);

__declspec(dllimport) BOOL WINAPI GetLogonSID(HANDLE hToken, PSID * ppsid);

__declspec(dllimport) BOOL WINAPI GetCurrentToken(OUT PHANDLE hToken);

__declspec(dllimport) VOID WINAPI FreeLogonSID(PSID * ppsid);

__declspec(dllimport) BOOL WINAPI IsCurrentSessionRemoteable();

__declspec(dllimport) void WINAPI EnumerateSessions();

__declspec(dllimport) BOOL WINAPI IsRemoteSession(DWORD SessionId);

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

__declspec(dllimport) BOOL WINAPI IsWow64ProcessEx(_In_ HANDLE ProcessHandle);

__declspec(dllimport) DWORD WINAPI GetCommandLineEx(_In_ HANDLE UniqueProcessId);

__declspec(dllimport) 
NTSTATUS WINAPI ZwEnumerateDirectoryObject(_In_ PCWSTR ObjectDirectory, _In_ PCWSTR TypeNameFilter);

__declspec(dllimport) void WINAPI QuerySymbolicLinkName(_In_ PCWSTR SymbolicLink);


//////////////////////////////////////////////////////////////////////////////////////////////////
//�ƻ�������ء�


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


__declspec(dllimport) 
void WINAPI DumpStackByCapture();

__declspec(dllimport)
void WINAPI DumpStackByWalk();

__declspec(dllimport)
void WINAPI DumpStackByTrace();


//////////////////////////////////////////////////////////////////////////////////////////////////


__declspec(dllimport)
void WINAPI EnumeratingProcessObjects(void);

__declspec(dllimport)
void WINAPI BrowsingPerformanceCounters(void);

__declspec(dllimport)
void WINAPI WritingPerformanceDataToLogFile(int argc, WCHAR ** argv);

__declspec(dllimport)
void WINAPI ReadingPerformanceDataFromLogFile(int argc, WCHAR ** argv);

__declspec(dllimport)
void WINAPI ConvertingLogFile (int argc, WCHAR ** argv);

__declspec(dllimport)
void WINAPI CollectQueryDataEx(void);


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C_END


//////////////////////////////////////////////////////////////////////////////////////////////////
