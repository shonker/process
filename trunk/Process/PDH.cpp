#include "pch.h"
#include "PDH.h"


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
void WINAPI EnumeratingProcessObjects(void)
/*
Enumerating Process Objects
Article
06/17/2022
3 contributors
The following example calls the PdhEnumObjectItems function to enumerate the instances and counters of the process objects on the local computer.

https://learn.microsoft.com/en-us/windows/win32/perfctrs/enumerating-process-objects
*/
{
    PDH_STATUS status = ERROR_SUCCESS;
    LPWSTR pwsCounterListBuffer = NULL;
    DWORD dwCounterListSize = 0;
    LPWSTR pwsInstanceListBuffer = NULL;
    DWORD dwInstanceListSize = 0;
    LPWSTR pTemp = NULL;
    CONST PWSTR COUNTER_OBJECT = (CONST PWSTR)L"Process";

    // Determine the required buffer size for the data. 
    status = PdhEnumObjectItems(
        NULL,                   // real-time source
        NULL,                   // local machine
        COUNTER_OBJECT,         // object to enumerate
        pwsCounterListBuffer,   // pass NULL and 0
        &dwCounterListSize,     // to get required buffer size
        pwsInstanceListBuffer,
        &dwInstanceListSize,
        PERF_DETAIL_WIZARD,     // counter detail level
        0);
    if (status == PDH_MORE_DATA) {
        // Allocate the buffers and try the call again.
        pwsCounterListBuffer = (LPWSTR)malloc(dwCounterListSize * sizeof(WCHAR));
        pwsInstanceListBuffer = (LPWSTR)malloc(dwInstanceListSize * sizeof(WCHAR));
        if (NULL != pwsCounterListBuffer && NULL != pwsInstanceListBuffer) {
            status = PdhEnumObjectItems(
                NULL,                   // real-time source
                NULL,                   // local machine
                COUNTER_OBJECT,         // object to enumerate
                pwsCounterListBuffer,
                &dwCounterListSize,
                pwsInstanceListBuffer,
                &dwInstanceListSize,
                PERF_DETAIL_WIZARD,     // counter detail level
                0);
            if (status == ERROR_SUCCESS) {
                wprintf(L"Counters that the Process objects defines:\n\n");

                // Walk the counters list.
                // The list can contain one or more null-terminated strings.
                // The list is terminated using two null-terminator characters.
                for (pTemp = pwsCounterListBuffer; *pTemp != 0; pTemp += wcslen(pTemp) + 1) {
                    wprintf(L"%s\n", pTemp);
                }

                wprintf(L"\nInstances of the Process object:\n\n");

                // Walk the instance list.
                // The list can contain one or more null-terminated strings.
                // The list is terminated using two null-terminator characters.
                for (pTemp = pwsInstanceListBuffer; *pTemp != 0; pTemp += wcslen(pTemp) + 1) {
                    wprintf(L"%s\n", pTemp);
                }
            } else {
                wprintf(L"Second PdhEnumObjectItems failed with 0x%x.\n", status);
            }
        } else {
            wprintf(L"Unable to allocate buffers.\n");
            status = ERROR_OUTOFMEMORY;
        }
    } else {
        wprintf(L"\nPdhEnumObjectItems failed with 0x%x.\n", status);
    }

    if (pwsCounterListBuffer != NULL)
        free(pwsCounterListBuffer);

    if (pwsInstanceListBuffer != NULL)
        free(pwsInstanceListBuffer);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
void WINAPI BrowsingPerformanceCounters(void)
/*
Browsing Performance Counters
Article
01/08/2021
3 contributors
The following example shows how to call PdhBrowseCounters to browse performance counters.
The example also shows how to collect and format raw counter data for display.

https://learn.microsoft.com/en-us/windows/win32/perfctrs/browsing-performance-counters
*/
{
    PDH_STATUS Status;
    HQUERY Query = NULL;
    HCOUNTER Counter;
    PDH_FMT_COUNTERVALUE DisplayValue;
    DWORD CounterType;
    SYSTEMTIME SampleTime;
    PDH_BROWSE_DLG_CONFIG BrowseDlgData;
    WCHAR CounterPathBuffer[PDH_MAX_COUNTER_PATH]{};
    CONST ULONG SAMPLE_INTERVAL_MS = 1000;
    CONST PWSTR BROWSE_DIALOG_CAPTION = (CONST PWSTR)L"Select a counter to monitor.";

    Status = PdhOpenQuery(NULL, NULL, &Query);// Create a query.
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nPdhOpenQuery failed with status 0x%x.", Status);
        goto Cleanup;
    }

    // Initialize the browser dialog window settings.
    ZeroMemory(&CounterPathBuffer, sizeof(CounterPathBuffer));
    ZeroMemory(&BrowseDlgData, sizeof(PDH_BROWSE_DLG_CONFIG));
    BrowseDlgData.bIncludeInstanceIndex = FALSE;
    BrowseDlgData.bSingleCounterPerAdd = TRUE;
    BrowseDlgData.bSingleCounterPerDialog = TRUE;
    BrowseDlgData.bLocalCountersOnly = FALSE;
    BrowseDlgData.bWildCardInstances = TRUE;
    BrowseDlgData.bHideDetailBox = TRUE;
    BrowseDlgData.bInitializePath = FALSE;
    BrowseDlgData.bDisableMachineSelection = FALSE;
    BrowseDlgData.bIncludeCostlyObjects = FALSE;
    BrowseDlgData.bShowObjectBrowser = FALSE;
    BrowseDlgData.hWndOwner = NULL;
    BrowseDlgData.szReturnPathBuffer = CounterPathBuffer;
    BrowseDlgData.cchReturnPathLength = PDH_MAX_COUNTER_PATH;
    BrowseDlgData.pCallBack = NULL;
    BrowseDlgData.dwCallBackArg = 0;
    BrowseDlgData.CallBackStatus = ERROR_SUCCESS;
    BrowseDlgData.dwDefaultDetailLevel = PERF_DETAIL_WIZARD;
    BrowseDlgData.szDialogBoxCaption = BROWSE_DIALOG_CAPTION;

    // Display the counter browser window.
    // The dialog is configured to return a single selection from the counter list.
    Status = PdhBrowseCounters(&BrowseDlgData);//这里弹出窗口后崩溃。
    if (Status != ERROR_SUCCESS) {
        if (Status == PDH_DIALOG_CANCELLED) {
            wprintf(L"\nDialog canceled by user.");
        } else {
            wprintf(L"\nPdhBrowseCounters failed with status 0x%x.", Status);
        }
        goto Cleanup;
    } else if (wcslen(CounterPathBuffer) == 0) {
        wprintf(L"\nUser did not select any counter.");
        goto Cleanup;
    } else {
        wprintf(L"\nCounter selected: %s\n", CounterPathBuffer);
    }

    // Add the selected counter to the query.
    Status = PdhAddCounter(Query, CounterPathBuffer, 0, &Counter);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nPdhAddCounter failed with status 0x%x.", Status);
        goto Cleanup;
    }

    // Most counters require two sample values to display a formatted value.
    // PDH stores the current sample value and the previously collected
    // sample value. This call retrieves the first value that will be used
    // by PdhGetFormattedCounterValue in the first iteration of the loop
    // Note that this value is lost if the counter does not require two
    // values to compute a displayable value.
    Status = PdhCollectQueryData(Query);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nPdhCollectQueryData failed with 0x%x.\n", Status);
        goto Cleanup;
    }

    // Print counter values until a key is pressed.
    while (!_kbhit()) {
        Sleep(SAMPLE_INTERVAL_MS);

        GetLocalTime(&SampleTime);

        Status = PdhCollectQueryData(Query);
        if (Status != ERROR_SUCCESS) {
            wprintf(L"\nPdhCollectQueryData failed with status 0x%x.", Status);
        }

        wprintf(L"\n\"%2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d\"",
                SampleTime.wMonth,
                SampleTime.wDay,
                SampleTime.wYear,
                SampleTime.wHour,
                SampleTime.wMinute,
                SampleTime.wSecond,
                SampleTime.wMilliseconds);

        // Compute a displayable value for the counter.
        Status = PdhGetFormattedCounterValue(Counter, PDH_FMT_DOUBLE, &CounterType, &DisplayValue);
        if (Status != ERROR_SUCCESS) {
            wprintf(L"\nPdhGetFormattedCounterValue failed with status 0x%x.", Status);
            goto Cleanup;
        }

        wprintf(L",\"%.20g\"", DisplayValue.doubleValue);
    }

Cleanup:
    if (Query) {
        PdhCloseQuery(Query);// Close the query.
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////