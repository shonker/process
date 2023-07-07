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

如：
index:0 RetAddr:00007FF8C3D191A4        FunctionAddress:00007FF8C3F6B450.
index:1 RetAddr:00007FF6F1BFE711        FunctionAddress:00007FF6F1D445D8.
index:2 RetAddr:00007FF6F1C063DA        FunctionAddress:0000000000000000.
index:3 RetAddr:00007FF6F1C080B9        FunctionAddress:00007FF6F1BED535.
index:4 RetAddr:00007FF6F1C07F9E        FunctionAddress:00007FF6F1C08080.
index:5 RetAddr:00007FF6F1C07E5E        FunctionAddress:00007FF6F1C07E70.
index:6 RetAddr:00007FF6F1C0814E        FunctionAddress:00007FF6F1C07E50.
index:7 RetAddr:00007FF99AD37614        FunctionAddress:00007FF99ADA4238.
index:8 RetAddr:00007FF99AF226A1        FunctionAddress:00007FF99B055000.
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
    if (!BackTrace) {

        return;
    }

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
生动事例：
 # Child-SP         RetAddr          : Args to Child                                                       : Call Site
 0 000000BC3CCFDB80 00007FF6F1BFE717 : 00007FF6F1D467F7 00007FF99A66EB30 00000216F175B790 CCCCCCCCCCCCCCCC : Process!DumpStackByWalk(00007FF8C3D19320)
 1 000000BC3CCFF780 00007FF6F1C063DA : 00000216F1755A30 000000BC3CCFF8A4 0000000000000000 000000BC3CCFF8A0 : test!TestStack(00007FF6F1BFE6F0)
 2 000000BC3CCFF880 00007FF6F1C080B9 : 0000000000000001 00000216F1761C00 00007FF6F1CED200 00007FF6F1C0709D : test!main(00007FF6F1C06330)
 3 000000BC3CCFF9C0 00007FF6F1C07F9E : 00007FF6F1CEF000 00007FF6F1CEF720 0000000000000000 0000000000000000 : test!invoke_main(00007FF6F1C08080)
 4 000000BC3CCFFA10 00007FF6F1C07E5E : 0000000000000000 0000000000000000 0000000000000000 0000000000000000 : test!__scrt_common_main_seh(00007FF6F1C07E70)
 5 000000BC3CCFFA80 00007FF6F1C0814E : 0000000000000000 0000000000000000 0000000000000000 0000000000000000 : test!__scrt_common_main(00007FF6F1C07E50)
 6 000000BC3CCFFAB0 00007FF99AD37614 : 000000BC3CAD3000 0000000000000000 0000000000000000 0000000000000000 : test!mainCRTStartup(00007FF6F1C08140)
 7 000000BC3CCFFAE0 00007FF99AF226A1 : 0000000000000000 0000000000000000 0000000000000000 0000000000000000 : KERNEL32!BaseThreadInitThunk(00007FF99AD37600)
 8 000000BC3CCFFB10 0000000000000000 : 0000000000000000 0000000000000000 0000000000000000 0000000000000000 : ntdll!RtlUserThreadStart(00007FF99AF22680)

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

    /*
    windbg的kv命令格式：
    0:000> KV
     # Child-SP          RetAddr           : Args to Child                                                           : Call Site
    00 000000dc`0394f878 00007ffc`830ada88 : 00000000`00000000 00000000`00000000 00000000`00000000 00000000`00000000 : ntdll!NtTerminateProcess+0x14
    */
    switch (MachineType) {
    case IMAGE_FILE_MACHINE_AMD64:
        printf(" # Child-SP         RetAddr          : Args to Child                                                       : Call Site\r\n");
        break;
    case IMAGE_FILE_MACHINE_I386:
        printf(" # Child-SP RetAddr  : Args to Child                       : Call Site\r\n");
        break;
    default:
        break;
    }    

    for (int i = 0;; i++) {
        BOOL ret = StackWalk(MachineType,
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

        printf("% 2d %p %p : %p %p %p %p : %s!%s(%p)\r\n",
               i,
               (PVOID)(size_t)stackFrame.AddrStack.Offset,
               (PVOID)(size_t)stackFrame.AddrReturn.Offset,
               (PVOID)(size_t)stackFrame.Params[0],
               (PVOID)(size_t)stackFrame.Params[1],
               (PVOID)(size_t)stackFrame.Params[2],
               (PVOID)(size_t)stackFrame.Params[3],
               moduleInfo.ModuleName,
               Package.sym.Name,
               (PVOID)(size_t)Package.sym.Address);

        //printf("ip(AddrPC):%llx, AddrReturn:%llx, AddrFrame(bp):%llx, AddrStack(sp):%llx, "
        //       "Params0:%llx, Params1:%llx, Params2:%llx, Params3:%llx.\r\n",
        //       (DWORD64)stackFrame.AddrPC.Offset,
        //       (DWORD64)stackFrame.AddrReturn.Offset,
        //       (DWORD64)stackFrame.AddrFrame.Offset,
        //       (DWORD64)stackFrame.AddrStack.Offset,
        //       (DWORD64)stackFrame.Params[0],
        //       (DWORD64)stackFrame.Params[1],
        //       (DWORD64)stackFrame.Params[2],
        //       (DWORD64)stackFrame.Params[3]);

        //printf("Address:%llx, Name:%s!%s.\r\n",
        //       (DWORD64)Package.sym.Address,
        //       moduleInfo.ModuleName,
        //       Package.sym.Name);

        //printf("\r\n\r\n\r\n");
    }

    SymCleanup(GetCurrentProcess());
}


EXTERN_C
__declspec(dllexport)
void WINAPI DumpStackByTrace()
{



}


//////////////////////////////////////////////////////////////////////////////////////////////////
