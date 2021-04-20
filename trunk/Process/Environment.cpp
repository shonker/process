#include "pch.h"
#include "Environment.h"


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI GetEnvironment()
/*
https://docs.microsoft.com/en-us/windows/win32/procthread/changing-environment-variables

The following example retrieves the process's environment block using GetEnvironmentStrings and prints the contents to the console.
*/
{
    // Get a pointer to the environment block. 
    LPTCH lpvEnv = GetEnvironmentStrings();
    if (lpvEnv == NULL) {// If the returned pointer is NULL, exit.
        printf("GetEnvironmentStrings failed (%d)\n", GetLastError());
        return 0;
    }

    // Variable strings are separated by NULL byte, and the block is terminated by a NULL byte. 
    LPTSTR lpszVariable = (LPTSTR)lpvEnv;

    while (*lpszVariable) {
        _tprintf(TEXT("%s\n"), lpszVariable);

        //经测试，这里由无效的环境变量。
        //建议用scanf或者正则表达式提取出名字，然后用ExpandEnvironmentStrings验证。

        lpszVariable += (size_t)lstrlen(lpszVariable) + 1;
    }

    FreeEnvironmentStrings(lpvEnv);
    return 1;
}
