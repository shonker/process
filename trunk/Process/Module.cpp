#include "pch.h"
#include "Module.h"
#include <shellapi.h>


#pragma warning(disable:6273)
#pragma warning(disable:4477)
#pragma warning(disable:4313)
#pragma warning(disable:6328)
#pragma warning(disable:4311)
#pragma warning(disable:4302)


//////////////////////////////////////////////////////////////////////////////////////////////////


// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS 
// and compile with -DPSAPI_VERSION=1


int PrintModules(DWORD processID)
/*
缺点：枚举不到WOW64的DLL。
*/
{
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;

    // Print the process identifier.
    printf("\nProcess ID: %u\n", processID);

    // Get a handle to the process.
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL == hProcess)
        return 1;

    // Get a list of all the modules in this process.
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            TCHAR szModName[MAX_PATH];

            // Get the full path to the module's file.
            if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
                // Print the module name and handle value.
                _tprintf(TEXT("\t%s (0x%08X)\n"), szModName, hMods[i]);
            }
        }
    }

    // Release the handle to the process.
    CloseHandle(hProcess);

    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI EnumeratingAllModulesForProcess(void)
/*
Enumerating All Modules For a Process
2018/05/31

To determine which processes have loaded a particular DLL, you must enumerate the modules for each process. 
The following sample code uses the EnumProcessModules function to enumerate the modules of current processes in the system.

https://docs.microsoft.com/zh-cn/windows/win32/psapi/enumerating-all-modules-for-a-process
*/
{
    DWORD aProcesses[1024];
    DWORD cbNeeded;
    DWORD cProcesses;
    unsigned int i;

    // Get the list of process identifiers.
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
        return 1;

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the names of the modules for each process.
    for (i = 0; i < cProcesses; i++) {
        PrintModules(aProcesses[i]);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
BOOL WINAPI ListProcessModules(DWORD dwPID)
/*
Traversing the Module List
05/31/2018

The following example obtains a list of modules for the specified process. 
The ListProcessModules function takes a snapshot of the modules associated with a given process using the CreateToolhelp32Snapshot function, 
and then walks through the list using the Module32First and Module32Next functions. 
The dwPID parameter of ListProcessModules identifies the process for which modules are to be enumerated, 
and is usually obtained by calling CreateToolhelp32Snapshot to enumerate the processes running on the system. 
See Taking a Snapshot and Viewing Processes for a simple console application that uses this function.

A simple error-reporting function, printError, displays the reason for any failures, 
which usually result from security restrictions.

用法示例：ListProcessModules(GetCurrentProcessId());

禁止WOW64下运行。
WOW64访问64会出现：WARNING: CreateToolhelp32Snapshot (of modules) failed with error 299 (仅完成部分的 ReadProcessMemory 或 WriteProcessMemory 请求。)
导致WOW64访问64失败。

对于WOW64程序获取不到WOW64（如：C:\Windows\SysWOW64）的DLL。

https://docs.microsoft.com/en-us/windows/win32/toolhelp/traversing-the-module-list
*/
{
    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;

    //  Take a snapshot of all modules in the specified process. 
    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (hModuleSnap == INVALID_HANDLE_VALUE)
    {
        printError(TEXT("CreateToolhelp32Snapshot (of modules)"));
        return(FALSE);
    }

    //  Set the size of the structure before using it. 
    me32.dwSize = sizeof(MODULEENTRY32);

    //  Retrieve information about the first module, and exit if unsuccessful 
    if (!Module32First(hModuleSnap, &me32))
    {
        printError(TEXT("Module32First"));  // Show cause of failure 
        CloseHandle(hModuleSnap);     // Must clean up the snapshot object! 
        return(FALSE);
    }

    //  Now walk the module list of the process, and display information about each module 
    do
    {
        _tprintf(TEXT("\n\n     MODULE NAME:     %s"), me32.szModule);
        _tprintf(TEXT("\n     executable     = %s"), me32.szExePath);
        _tprintf(TEXT("\n     process ID     = 0x%08X"), me32.th32ProcessID);
        _tprintf(TEXT("\n     ref count (g)  =     0x%04X"), me32.GlblcntUsage);
        _tprintf(TEXT("\n     ref count (p)  =     0x%04X"), me32.ProccntUsage);
        _tprintf(TEXT("\n     base address   = 0x%08X"), (DWORD)me32.modBaseAddr);
        _tprintf(TEXT("\n     base size      = %d"), me32.modBaseSize);

    } while (Module32Next(hModuleSnap, &me32));

    _tprintf(TEXT("\n"));

    //  Do not forget to clean up the snapshot object. 
    CloseHandle(hModuleSnap);
    return(TRUE);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
void CALLBACK RunDllApi(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
/*
此函数专门用于被rundll32.exe调用，所以此函数的原型固定。

本函的特色：后面可根的参数的个数不受限制（系统/内存等的除外）。

用法示例：
rundll32.exe Process.dll,RunDllApi notepad.exe d:\test.txt
当然后面可以跟任意的参数，如：
rundll32.exe Process.dll,RunDllApi notepad.exe d:\test.txt x y z
注意：引号的作用。

注意：rundll32默认是没有控制台的。
*/
{
    DWORD NumberOfCharsWritten = 0;

    //__debugbreak();    

    setlocale(LC_CTYPE, ".936");  

    //HANDLE Output = GetStdHandle(STD_OUTPUT_HANDLE);
    //char buffer[MAX_PATH] = {0};
    //wsprintfA(buffer, "第三个参数:%s.\n", lpszCmdLine);
    //WriteConsole(Output, buffer, sizeof(buffer), &NumberOfCharsWritten, NULL);
    MessageBoxA(0, lpszCmdLine, "第三个参数", 0);

    WCHAR Buffer[MAX_PATH] = {0};
    MessageBox(0, GetCommandLineW(), L"命令行", 0);

    WCHAR pwszDst[MAX_PATH];
    SHAnsiToUnicode(lpszCmdLine, pwszDst, MAX_PATH);
    PathRemoveBlanksW(pwszDst);

    int Args;
    LPWSTR * Arglist = CommandLineToArgvW(pwszDst, &Args);
    if (NULL == Arglist) {
        wsprintf(Buffer, L"LastError：%d.\n", GetLastError());
        MessageBox(0, Buffer, L"CommandLineToArgvW运行错误：", 0);
        return;
    }

    const WCHAR * temp = L"第三个参数的解析：\n";
    MessageBoxA(0, lpszCmdLine, "开始解析第三个参数：", 0);

    wsprintf(Buffer, L"%d.\n", Args);
    MessageBox(0, Buffer, L"第三个参数的个数：", 0);

    for (int i = 0; i < Args; i++) {
        wsprintf(Buffer, L"参数：%d.\n", i + 1);
        MessageBox(0, Arglist[i], Buffer, 0);
    }

    ShellExecute(hwnd, 0i64, Arglist[0], Arglist[1], 0i64, nCmdShow);

    LocalFree(Arglist);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


#define _WINSOCK_DEPRECATED_NO_WARNINGS 


set<wstring> g_Module;
CRITICAL_SECTION g_ModuleLock;


void GetAllModuleInfo(HANDLE hProcess)
/*
功能：获取一个进程的所有的模块信息。

To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS and compile with -DPSAPI_VERSION=1

此办法可显示WOW64的dll,但显示的路径不对，是64的路径。
这是GetModuleFileNameEx导致的。
改进思路：用GetMappedFileName，但是得到的是NT路径。

此函数不可用于WOW64,否则，有太多的进程获取为空，即使提权也不行。

参考：
https://docs.microsoft.com/zh-cn/windows/win32/psapi/enumerating-all-modules-for-a-process
https://docs.microsoft.com/zh-cn/windows/win32/memory/obtaining-a-file-name-from-a-file-handle
*/
{
    DWORD cbNeeded = 0;
    EnumProcessModulesEx(hProcess, NULL, 0, &cbNeeded, LIST_MODULES_ALL);
    if (0 == cbNeeded) {
        //LOGW(IMPORTANT_INFO_LEVEL, "警告：LastError:%d, pid:%d", GetLastError(), GetProcessId(hProcess));
        return;
    }

    HMODULE * hMods = (HMODULE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbNeeded);
    if (NULL == hMods) {
        //LOGA(ERROR_LEVEL, "申请内存失败");
        return;
    }

    if (EnumProcessModulesEx(hProcess, hMods, cbNeeded, &cbNeeded, LIST_MODULES_ALL)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            TCHAR szModName[MAX_PATH] = {0};

#pragma warning(push)
#pragma warning(disable:6011)
#pragma warning(disable:6385)
            if (GetMappedFileName(hProcess, hMods[i], szModName, MAX_PATH)) {
#pragma warning(pop)
                Nt2Dos(szModName);//这里是NT名，以\Device\HarddiskVolume6\开头。
                lstrcat(szModName, L"\n");
                g_Module.insert(szModName);
            } else {
                //LOGA(ERROR_LEVEL, "pid:%d, LastError:%#x", GetProcessId(hProcess), GetLastError());
            }
        }
    }

    HeapFree(GetProcessHeap(), 0, hMods);
}


void GetProcessListInfo()
/*
注意：
对于没有对应文件的一些进程，这里是OpenProcess失败的。
对于受保护的进程，这里是OpenProcess失败的。
*/
{
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return;
    }

    do {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_QUERY_LIMITED_INFORMATION,
                                      FALSE,
                                      pe32.th32ProcessID);
        if (hProcess) {
            GetAllModuleInfo(hProcess);
            CloseHandle(hProcess);
        } else {
            switch (GetLastError()) {
            case ERROR_ACCESS_DENIED://5（拒绝访问，即权限不足）。
            case ERROR_INVALID_PARAMETER://87（参数错误，即进程已经退出）。
                break;
            default:
                //LOGW(IMPORTANT_INFO_LEVEL, "pid:%#d", pe32.th32ProcessID);
                //DbgPrintW("警告：打开进程失败, pid:%#d, LastError:%d", pe32.th32ProcessID, GetLastError());
                break;
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}


void PrintAllModule()
{
    wstring all;
    int n = 0;

    //一个一个的打印有点慢。所以收集下，一下打印。
    if (!g_Module.empty()) {
        for (wstring temp : g_Module) {
            //printf("%ls.\n", temp.c_str());
            all += temp;
            n++;
        }
    }

    //printf("%d.\n", all.size());
    //printf("%d.\n", all.length());
    //printf("%d.\n", all.max_size());//-2.
    //printf("%d.\n", all._Mysize);
    printf("模块总个数：%d.\n", n);

    printf("模块信息如下：\n");
    printf("\n");

    printf("%ls\n", all.c_str());
}


void PrintAllModuleTest()
/*
功能：枚举系统的所有进程（排除没有对应文件的一些进程和受保护的进程）的所有模块（去掉重复）。

感觉这个函数是不是很有用。
1.一些系统的工具，也有类似的功能，如：tasklist /m.
2.procexp.exe可以搜索。
3.Listdlls.exe的有重复。
*/
{
    GetProcessListInfo();

    PrintAllModule();
}


//////////////////////////////////////////////////////////////////////////////////////////////////


int PrintModulesEx(DWORD processID)
/*
To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS and compile with -DPSAPI_VERSION=1

此办法可显示WOW64的dll,但显示的路径不对，是64的路径。
这是GetModuleFileNameEx导致的。
改进思路：用GetMappedFileName，但是得到的是NT路径。

此函数不可用于WOW64,否则，有太多的进程获取为空，即使提权也不行。

参考：
https://docs.microsoft.com/zh-cn/windows/win32/psapi/enumerating-all-modules-for-a-process
https://docs.microsoft.com/zh-cn/windows/win32/memory/obtaining-a-file-name-from-a-file-handle
*/
{
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;

    // Print the process identifier.
    printf("\nProcess ID: %u\n", processID);

    // Get a handle to the process.
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL == hProcess) {
        printf("\nLastError: %u\n", GetLastError());
        return 1;
    }

    // Get a list of all the modules in this process.
    if (EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)) {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            TCHAR szModName[MAX_PATH];

            // Get the full path to the module's file.
            if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
                // Print the module name and handle value.
                //_tprintf(TEXT("\t%s (0x%p)\n"), szModName, hMods[i]);
            }

            if (GetMappedFileName(hProcess, hMods[i], szModName, MAX_PATH)) {
                // Translate path with device name to drive letters.
                TCHAR szTemp[MAX_PATH];
                szTemp[0] = '\0';

                if (GetLogicalDriveStrings(MAX_PATH - 1, szTemp)) {
                    TCHAR szName[MAX_PATH];
                    TCHAR szDrive[3] = TEXT(" :");
                    BOOL bFound = FALSE;
                    TCHAR * p = szTemp;

                    do {
                        // Copy the drive letter to the template string
                        *szDrive = *p;

                        // Look up each device name
                        if (QueryDosDevice(szDrive, szName, MAX_PATH)) {
                            size_t uNameLen = _tcslen(szName);

                            if (uNameLen < MAX_PATH) {
                                bFound = _tcsnicmp(szModName, szName, uNameLen) == 0 && *(szModName + uNameLen) == _T('\\');

                                if (bFound) {
                                    // Reconstruct pszFilename using szTempFile
                                    // Replace device path with DOS path
                                    TCHAR szTempFile[MAX_PATH];
                                    StringCchPrintf(szTempFile,
                                                    MAX_PATH,
                                                    TEXT("%s%s"),
                                                    szDrive,
                                                    szModName + uNameLen);
                                    StringCchCopyN(szModName, MAX_PATH, szTempFile, _tcslen(szTempFile));
                                }
                            }
                        }

                        // Go to the next NULL character.
                        while (*p++);
                    } while (!bFound && *p); // end of string
                }

                _tprintf(TEXT("\t%s (0x%p)\n"), szModName, hMods[i]);//这里是NT名，以\Device\HarddiskVolume6\开头。
            }
        }
    }

    CloseHandle(hProcess);// Release the handle to the process.
    return 0;
}


int TestPrintModulesEx(void)
{
    DWORD aProcesses[1024];
    DWORD cbNeeded;
    DWORD cProcesses;
    unsigned int i;

    EnablePrivilege(SE_DEBUG_NAME, TRUE);

    // Get the list of process identifiers.
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        _ASSERTE(FALSE);
        return 1;
    }

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the names of the modules for each process.
    for (i = 0; i < cProcesses; i++) {
        PrintModulesEx(aProcesses[i]);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
