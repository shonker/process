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


int WINAPI ExecInExplorer(HINSTANCE, HINSTANCE, PWSTR, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        ShellExecInExplorerProcess(L"http://www.msn.com");
        CoUninitialize();
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
