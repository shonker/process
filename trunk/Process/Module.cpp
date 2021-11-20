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
