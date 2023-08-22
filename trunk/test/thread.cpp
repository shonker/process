#include "thread.h"

void TestThread()
{
    //LoadLibraryInProcess(L"d:\\test.dll", 8744);

    BYTE ShellCode[1] = {0xcc};// int 3
    LoadShellCodeInProcess(&ShellCode, sizeof(ShellCode), 8744);
}
