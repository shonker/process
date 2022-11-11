#include "pch.h"
#include "Stack.h"
#include "Process.h"


//////////////////////////////////////////////////////////////////////////////////////////////////


PVOID GetFunctionAddressByReturnAddress(PVOID ReturnAddress)
/*
功能：根据返回地址（call之后的第一个指令的地址）获取要call的函数的地址。
*/
{
    PVOID FunctionAddress = nullptr;
    int Offset = 0;

    if (!ReturnAddress) {
        return FunctionAddress;
    }

    PVOID NearCallAddress = (PVOID)((PBYTE)ReturnAddress - 5);
    PVOID FarCallAddress = (PVOID)((PBYTE)ReturnAddress - 6);
    PVOID EsiCallAddress = (PVOID)((PBYTE)ReturnAddress - 2);//ffd6            call    esi

    if (htons(0xffd6) == *(PWORD)EsiCallAddress) {

        return FunctionAddress;//这个暂时没想好处理的办法。
    }

    if (0xe8 != *(PBYTE)NearCallAddress && htons(0xff15) != *(PWORD)FarCallAddress) {

        return FunctionAddress;
    }

    if (0xe8 == *(PBYTE)NearCallAddress) {
        Offset = *(PULONG)((PBYTE)NearCallAddress + 1);
    }

    if (htons(0xff15) == *(PWORD)FarCallAddress) {
        Offset = *(PULONG)((PBYTE)FarCallAddress + 2);
    }

    if (!Offset) {

        return FunctionAddress;
    }

    if (Offset) {
        FunctionAddress = (PVOID)((SIZE_T)ReturnAddress + Offset);
    } else {
        Offset = !Offset;
        Offset++;
        FunctionAddress = (PVOID)((SIZE_T)ReturnAddress - Offset);
    }

    return FunctionAddress;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
void WINAPI DumpStackByCapture()
/*
功能：打印类似于windbg的kv命令。

*/
{
    //In Windows XP and Windows Server 2003, the sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
    ULONG FramesToSkip = 0;//The number of frames to skip from the start of the back trace. 
    ULONG FramesToCapture = 62;//The number of frames to be captured.     

    //http://msdn.microsoft.com/zh-cn/library/windows/hardware/Dn613940(v=vs.85).aspx
    int stacklength = 32 * 1024;//取X86/X64/IA64的栈的最大值。

    PVOID CallersAddress = _ReturnAddress();

    PVOID * BackTrace = 0;//An array of pointers captured from the current stack trace. 
    BackTrace = (PVOID *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, stacklength);
    _ASSERTE(BackTrace);

    USHORT ncf = 0;//The number of captured frames
    ULONG BackTraceHash = 0;
    ncf = RtlCaptureStackBackTrace(FramesToSkip, FramesToCapture, BackTrace, &BackTraceHash);

    USHORT i = 0;
    for (PVOID * pBackTrace = BackTrace; i < ncf; pBackTrace++, i++) {
        PVOID FunctionAddress = GetFunctionAddressByReturnAddress(*pBackTrace);
        printf("index:%d\tRetAddr:%p\tFunctionAddress:%p.\r\n", i, *pBackTrace, FunctionAddress);
    }

    HeapFree(GetProcessHeap(), 0, BackTrace);
}


EXTERN_C
__declspec(dllexport)
void WINAPI DumpStackByWalk()
/*

参考：\nt5src\Source\Win2K3\NT\admin\activec\base\mmcdebug.cpp的CTraceTag::DumpStack()
*/
{
    SymInitialize(GetCurrentProcess(), NULL, TRUE);

    DWORD MachineType = IMAGE_FILE_MACHINE_AMD64;

#ifdef _WIN64    
    if (IsWow64()) {
        MachineType = IMAGE_FILE_MACHINE_I386;
    }
#else
    MachineType = IMAGE_FILE_MACHINE_I386;
#endif

    CONTEXT threadContext{};
    threadContext.ContextFlags = CONTEXT_FULL;
    //GetThreadContext(GetCurrentThread(), &threadContext);//这个只会返回一两个循环（帧）。
    RtlCaptureContext(&threadContext);

    STACKFRAME stackFrame{};

#if defined(_M_IX86)
    //dwMachType = IMAGE_FILE_MACHINE_I386;

    stackFrame.AddrPC.Offset = threadContext.Eip;
    stackFrame.AddrPC.Mode = AddrModeFlat;

    stackFrame.AddrStack.Offset = threadContext.Esp;
    stackFrame.AddrStack.Mode = AddrModeFlat;

    stackFrame.AddrFrame.Offset = threadContext.Ebp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
#elif defined(_M_AMD64)
    //dwMachType = IMAGE_FILE_MACHINE_AMD64;

    stackFrame.AddrPC.Offset = threadContext.Rip;
    stackFrame.AddrPC.Mode = AddrModeFlat;

    stackFrame.AddrFrame.Offset = threadContext.Rbp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;

    stackFrame.AddrStack.Offset = threadContext.Rsp;
    stackFrame.AddrStack.Mode = AddrModeFlat;
#elif defined(_M_IA64)
    //dwMachType = IMAGE_FILE_MACHINE_IA64;
    stackFrame.AddrPC.Offset = threadContext.StIIP;
    stackFrame.AddrPC.Mode = AddrModeFlat;

    stackFrame.AddrStack.Offset = threadContext.IntSp;
    stackFrame.AddrStack.Mode = AddrModeFlat;
#else
#error("Unknown Target Machine");
#endif

    for (;;) {
        BOOL ret = FALSE;

        ret = StackWalk(MachineType,
                        GetCurrentProcess(),
                        GetCurrentThread(),
                        &stackFrame,
                        &threadContext,
                        NULL, // Use ReadProcessMemory
                        SymFunctionTableAccess,
                        SymGetModuleBase,
                        NULL);
        if (!ret) {
            break;
        }

        IMAGEHLP_MODULE  moduleInfo{sizeof(IMAGEHLP_MODULE)};
        ret = SymGetModuleInfo(GetCurrentProcess(), stackFrame.AddrPC.Offset, &moduleInfo);

        IMAGEHLP_SYMBOL_PACKAGE Package{};
        Package.sym.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        Package.sym.MaxNameLength = MAX_SYM_NAME;

        DWORD_PTR Displacement = 0;
        ret = SymGetSymFromAddr(GetCurrentProcess(), stackFrame.AddrPC.Offset, &Displacement, &Package.sym);

        printf("ip(AddrPC):%llx, AddrReturn:%llx, AddrFrame(bp):%llx, AddrStack(sp):%llx, "
               "Params0:%llx, Params1:%llx, Params2:%llx, Params3:%llx.\r\n",
               (DWORD64)stackFrame.AddrPC.Offset,
               (DWORD64)stackFrame.AddrReturn.Offset,
               (DWORD64)stackFrame.AddrFrame.Offset,
               (DWORD64)stackFrame.AddrStack.Offset,
               (DWORD64)stackFrame.Params[0],
               (DWORD64)stackFrame.Params[1],
               (DWORD64)stackFrame.Params[2],
               (DWORD64)stackFrame.Params[3]);

        printf("Address:%llx, Name:%s!%s.\r\n",
               (DWORD64)Package.sym.Address,
               moduleInfo.ModuleName,
               Package.sym.Name);

        /*
        windbg的kv命令格式：
        0:000> KV
         # Child-SP          RetAddr           : Args to Child                                                           : Call Site
        00 000000dc`0394f878 00007ffc`830ada88 : 00000000`00000000 00000000`00000000 00000000`00000000 00000000`00000000 : ntdll!NtTerminateProcess+0x14
        */

        printf("\r\n\r\n\r\n");
}

    SymCleanup(GetCurrentProcess());
}


EXTERN_C
__declspec(dllexport)
void WINAPI DumpStackByTrace()
{



}


//////////////////////////////////////////////////////////////////////////////////////////////////
