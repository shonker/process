#include "ProcessesTest.h"


BOOL WINAPI EnumeratingProcessesCallBackTest(_In_ DWORD Pid, _In_opt_ PVOID Context)
{
    printf("pid:%d\n", Pid);

    return TRUE;
}


BOOL WINAPI  EnumerateProcessCallBackTest(_In_ PVOID lppe, /*实际类型是LPPROCESSENTRY32W*/
                                          _In_opt_ PVOID Context)
{
    LPPROCESSENTRY32W pe32 = (LPPROCESSENTRY32W)lppe;

    printf("Pid:%08d\tExeFile:%-50ls\n", pe32->th32ProcessID, pe32->szExeFile);

    return TRUE;
}


void StartProtectProcess()
/*
启动受保护的子进程
新的安全模型还允许受反恶意软件保护的服务启动受保护的子进程。 这些子进程将在与父服务相同的保护级别上运行，
并且其二进制文件必须使用通过 ELAM 资源部分注册的相同证书进行签名。

为了允许反恶意软件保护服务启动受保护的子进程，已公开新的扩展属性密钥 （PROC_THREAD_ATTRIBUTE_PROTECTION_LEVEL），
并且必须与 UpdateProcThreadAttribute API 一起使用。
指向 PROTECTION_LEVEL_SAME 属性值的指针必须传递到 UpdateProcThreadAttribute API 中。

注意：

为了使用此新属性，服务还必须在 CreateProcess 调用的进程创建标志参数中指定CREATE_PROTECTED_PROCESS。
需要使用 /ac 开关对服务二进制文件进行签名，以包含交叉证书，以将其链接到已知 CA。
未正确链接到已知根 CA 的自签名证书将不起作用。
代码示例：

https://learn.microsoft.com/zh-cn/windows/win32/services/protecting-anti-malware-services-
*/
{
    DWORD ProtectionLevel = PROTECTION_LEVEL_SAME;//启动成功了，但不是保护的进程，看来还得签名啊！
    //DWORD ProtectionLevel = PROTECTION_LEVEL_WINTCB;//启动失败，返回值是0x00000241。
    SIZE_T AttributeListSize = 0;
    DWORD Result;
    PROCESS_INFORMATION ProcessInformation = {0};
    STARTUPINFOEXW StartupInfoEx = {0};

    StartupInfoEx.StartupInfo.cb = sizeof(StartupInfoEx);

    InitializeProcThreadAttributeList(nullptr, 1, 0, &AttributeListSize);
    StartupInfoEx.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, AttributeListSize);
    _ASSERTE(StartupInfoEx.lpAttributeList);
    if (InitializeProcThreadAttributeList(StartupInfoEx.lpAttributeList, 1, 0, &AttributeListSize) == FALSE) {
        Result = GetLastError();
        goto exitFunc;
    }

    if (UpdateProcThreadAttribute(StartupInfoEx.lpAttributeList,
                                  0,
                                  PROC_THREAD_ATTRIBUTE_PROTECTION_LEVEL,
                                  &ProtectionLevel,
                                  sizeof(ProtectionLevel),
                                  nullptr,
                                  nullptr) == FALSE) {
        Result = GetLastError();
        goto exitFunc;
    }

    if (CreateProcessW(L"c:\\windows\\system32\\cmd.exe",
                       nullptr,
                       nullptr,
                       nullptr,
                       FALSE,
                       EXTENDED_STARTUPINFO_PRESENT | CREATE_PROTECTED_PROCESS,
                       nullptr,
                       nullptr,
                       (LPSTARTUPINFOW)&StartupInfoEx,
                       &ProcessInformation) == FALSE) {
        Result = GetLastError();
        goto exitFunc;
    }

exitFunc:
    if (ProcessInformation.hProcess){
        CloseHandle(ProcessInformation.hProcess);
    }
    
    if (ProcessInformation.hThread){
        CloseHandle(ProcessInformation.hThread);
    }    
}


void TestProcess()
{
    //EnumeratingAllProcesses(EnumeratingProcessesCallBackTest, nullptr);

    EnumerateProcess(EnumerateProcessCallBackTest, nullptr);


}
