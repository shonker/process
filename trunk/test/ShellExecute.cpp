#include "ShellExecute.h"


//////////////////////////////////////////////////////////////////////////////////////////////////


int WINAPI ExecInExplorerTest()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        ShellExecInExplorerProcess(L"https://correy.webs.com/");
        ShellExecInExplorerProcess(L"notepad.exe");
        CoUninitialize();
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
