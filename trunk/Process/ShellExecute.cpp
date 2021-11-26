#include "pch.h"
#include "ShellExecute.h"


//////////////////////////////////////////////////////////////////////////////////////////////////


// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


HRESULT GetShellViewForDesktop(REFIID riid, void ** ppv)
// use the shell view for the desktop using the shell windows automation to find the
// desktop web browser and then grabs its view
//
// returns:
//      IShellView, IFolderView and related interfaces
{
    *ppv = NULL;

    IShellWindows * psw;
    HRESULT hr = CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&psw));
    if (SUCCEEDED(hr)) {
        HWND hwnd;
        IDispatch * pdisp;
        VARIANT vEmpty = {}; // VT_EMPTY
        if (S_OK == psw->FindWindowSW(&vEmpty, &vEmpty, SWC_DESKTOP, (long *)&hwnd, SWFO_NEEDDISPATCH, &pdisp)) {
            IShellBrowser * psb;
            hr = IUnknown_QueryService(pdisp, SID_STopLevelBrowser, IID_PPV_ARGS(&psb));
            if (SUCCEEDED(hr)) {
                IShellView * psv;
                hr = psb->QueryActiveShellView(&psv);
                if (SUCCEEDED(hr)) {
                    hr = psv->QueryInterface(riid, ppv);
                    psv->Release();
                }
                psb->Release();
            }
            pdisp->Release();
        } else {
            hr = E_FAIL;
        }
        psw->Release();
    }
    return hr;
}


HRESULT GetShellDispatchFromView(IShellView * psv, REFIID riid, void ** ppv)
// From a shell view object gets its automation interface and from that gets the shell
// application object that implements IShellDispatch2 and related interfaces.
{
    *ppv = NULL;

    IDispatch * pdispBackground;
    HRESULT hr = psv->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARGS(&pdispBackground));
    if (SUCCEEDED(hr)) {
        IShellFolderViewDual * psfvd;
        hr = pdispBackground->QueryInterface(IID_PPV_ARGS(&psfvd));
        if (SUCCEEDED(hr)) {
            IDispatch * pdisp;
            hr = psfvd->get_Application(&pdisp);
            if (SUCCEEDED(hr)) {
                hr = pdisp->QueryInterface(riid, ppv);
                pdisp->Release();
            }
            psfvd->Release();
        }
        pdispBackground->Release();
    }
    return hr;
}


EXTERN_C
__declspec(dllexport)
HRESULT WINAPI ShellExecInExplorerProcess(PCWSTR pszFile)
/*
进程启动后，父进程是Explorer。

摘自：
\Windows-classic-samples\Samples\Win7Samples\winui\shell\appplatform\ExecInExplorer\ExecInExplorer.cpp
*/
{
    IShellView * psv;
    HRESULT hr = GetShellViewForDesktop(IID_PPV_ARGS(&psv));
    if (SUCCEEDED(hr)) {
        IShellDispatch2 * psd;
        hr = GetShellDispatchFromView(psv, IID_PPV_ARGS(&psd));
        if (SUCCEEDED(hr)) {
            BSTR bstrFile = SysAllocString(pszFile);
            hr = bstrFile ? S_OK : E_OUTOFMEMORY;
            if (SUCCEEDED(hr)) {
                VARIANT vtEmpty = {}; // VT_EMPTY
                hr = psd->ShellExecute(bstrFile, vtEmpty, vtEmpty, vtEmpty, vtEmpty);//error C2039: "ShellExecuteW": 不是 "IShellDispatch2" 的成员
                SysFreeString(bstrFile);
            }

            psd->Release();
        }

        psv->Release();
    }

    return hr;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


void FindDesktopFolderView(REFIID riid, void ** ppv)
//https://devblogs.microsoft.com/oldnewthing/20130318-00/?p=4933
{
    CComPtr<IShellWindows> spShellWindows;
    (void)spShellWindows.CoCreateInstance(CLSID_ShellWindows);

    CComVariant vtLoc(CSIDL_DESKTOP);
    CComVariant vtEmpty;
    long lhwnd;
    CComPtr<IDispatch> spdisp;
    spShellWindows->FindWindowSW(&vtLoc, &vtEmpty, SWC_DESKTOP, &lhwnd, SWFO_NEEDDISPATCH, &spdisp);

    CComPtr<IShellBrowser> spBrowser;
    CComQIPtr<IServiceProvider>(spdisp)->QueryService(SID_STopLevelBrowser, IID_PPV_ARGS(&spBrowser));

    CComPtr<IShellView> spView;
    spBrowser->QueryActiveShellView(&spView);

    spView->QueryInterface(riid, ppv);
}


void GetDesktopAutomationObject(REFIID riid, void ** ppv)
//http://blogs.msdn.com/b/oldnewthing/archive/2013/11/18/10468726.aspx
{
    CComPtr<IShellView> spsv;
    FindDesktopFolderView(IID_PPV_ARGS(&spsv));

    CComPtr<IDispatch> spdispView;
    spsv->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARGS(&spdispView));
    spdispView->QueryInterface(riid, ppv);
}


void ShellExecuteFromExplorer(PCWSTR pszFile,
                              PCWSTR pszParameters = nullptr,
                              PCWSTR pszDirectory = nullptr,
                              PCWSTR pszOperation = nullptr,
                              int nShowCmd = SW_SHOWNORMAL
)
//http://blogs.msdn.com/b/oldnewthing/archive/2013/11/18/10468726.aspx
{
    CComPtr<IShellFolderViewDual> spFolderView;
    GetDesktopAutomationObject(IID_PPV_ARGS(&spFolderView));
    CComPtr<IDispatch> spdispShell;
    spFolderView->get_Application(&spdispShell);

    //error C2039: "ShellExecuteW": 不是 "IShellDispatch2" 的成员
    CComQIPtr<IShellDispatch2>(spdispShell)->ShellExecute(CComBSTR(pszFile),
                                                          CComVariant(pszParameters ? pszParameters : L""),
                                                          CComVariant(pszDirectory ? pszDirectory : L""),
                                                          CComVariant(pszOperation ? pszOperation : L""),
                                                          CComVariant(nShowCmd));
}


int __cdecl ShellExecuteFromExplorerTest(int argc, wchar_t ** argv)
{
    if (argc < 2) return 0;

    (void)CoInitialize(0);
    ShellExecuteFromExplorer(
        argv[1],
        argc >= 3 ? argv[2] : L"",
        argc >= 4 ? argv[3] : L"",
        argc >= 5 ? argv[4] : L"",
        argc >= 6 ? _wtoi(argv[5]) : SW_SHOWNORMAL);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
