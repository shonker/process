#include "pch.h"
#include "PDH.h"


#pragma warning(disable:4706) //条件表达式内的赋值
#pragma warning(disable:6385) //正在从 "pItems" 读取无效数据。
#pragma warning(disable:4456)


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


EXTERN_C
__declspec(dllexport)
void WINAPI EnumObjectItems(_In_ LPCWSTR ObjectName)
/*

参数
取值，如：L"Process"

功能：类似于PS的命令 (Get-Counter -ListSet * | where {$_.CounterSetName -eq 'Memory'}).Paths

https://learn.microsoft.com/en-us/windows/win32/perfctrs/enumerating-process-objects
*/
{
    PDH_STATUS status = ERROR_SUCCESS;
    LPWSTR pwsCounterListBuffer = NULL;
    DWORD dwCounterListSize = 0;
    LPWSTR pwsInstanceListBuffer = NULL;
    DWORD dwInstanceListSize = 0;
    LPWSTR pTemp = NULL;

    // Determine the required buffer size for the data. 
    status = PdhEnumObjectItems(
        NULL,                   // real-time source
        NULL,                   // local machine
        ObjectName,         // object to enumerate
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
                ObjectName,         // object to enumerate
                pwsCounterListBuffer,
                &dwCounterListSize,
                pwsInstanceListBuffer,
                &dwInstanceListSize,
                PERF_DETAIL_WIZARD,     // counter detail level
                0);
            if (status == ERROR_SUCCESS) {
                wprintf(L"Counters that the %ls objects defines:\n\n", ObjectName);

                // Walk the counters list.
                // The list can contain one or more null-terminated strings.
                // The list is terminated using two null-terminator characters.
                for (pTemp = pwsCounterListBuffer; *pTemp != 0; pTemp += wcslen(pTemp) + 1) {
                    wprintf(L"\\%ls\\%s\n", ObjectName, pTemp);
                }

                wprintf(L"\nInstances of the %ls object:\n\n", ObjectName);

                // Walk the instance list.
                // The list can contain one or more null-terminated strings.
                // The list is terminated using two null-terminator characters.
                for (pTemp = pwsInstanceListBuffer; *pTemp != 0; pTemp += wcslen(pTemp) + 1) {
                    wprintf(L"\\%ls\\%s\n", ObjectName, pTemp);//如果设置了ProcessNameFormat 或 ThreadNameFormat这里的显示会：_ID。
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


EXTERN_C
__declspec(dllexport)
void WINAPI EnumCountersObjects()
/*

https://learn.microsoft.com/en-us/windows/win32/api/pdh/nf-pdh-pdhenumobjectsa
*/
{
    //PDH_STATUS status = ERROR_SUCCESS;
    //LPCWSTR DataSource = NULL;
    //LPCWSTR MachineName = NULL;
    //PZZWSTR ObjectList = NULL;    
    //DWORD BufferSize = 0;
    //DWORD   DetailLevel = PERF_DETAIL_NOVICE;//PERF_DETAIL_WIZARD
    //BOOL    Refresh = FALSE;//TRUE

    ////总是异常
    //status = PdhEnumObjects(DataSource, MachineName, ObjectList, &BufferSize, DetailLevel, Refresh);

    //ObjectList = (PZZWSTR)HeapAlloc(GetProcessHeap(), 0, MAX_PATH * sizeof(WCHAR) * MAX_PATH);
    //_ASSERTE(ObjectList);
}


EXTERN_C
__declspec(dllexport)
void WINAPI EnumCountersMachines()
/*

有数据的条件：
The computer names were either specified when adding counters to the query or when calling the PdhConnectMachine function.

https://learn.microsoft.com/en-us/windows/win32/api/pdh/nf-pdh-pdhenummachinesa
*/
{
    PDH_STATUS status = ERROR_SUCCESS;
    LPCWSTR DataSource = NULL;
    PZZWSTR MachineList = NULL;
    DWORD BufferSize = 0;

    status = PdhEnumMachines(DataSource, MachineList, &BufferSize);
    _ASSERTE(PDH_MORE_DATA == status);

    MachineList = (PZZWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BufferSize + sizeof(WCHAR));
    _ASSERTE(MachineList);

    status = PdhEnumMachines(DataSource, MachineList, &BufferSize);
    if (status == ERROR_SUCCESS) {


    }

    HeapFree(GetProcessHeap(), 0, MachineList);
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
    PDH_STATUS Status{};
    HQUERY Query = NULL;
    HCOUNTER Counter{};
    PDH_FMT_COUNTERVALUE DisplayValue{};
    DWORD CounterType{};
    SYSTEMTIME SampleTime{};
    PDH_BROWSE_DLG_CONFIG BrowseDlgData{};
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
    // PDH stores the current sample value and the previously collected sample value.
    // This call retrieves the first value that will be used
    // by PdhGetFormattedCounterValue in the first iteration of the loop
    // Note that this value is lost if the counter does not require two values to compute a displayable value.
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

        wprintf(L"\n\"%2.2u/%2.2u/%4.4u %2.2u:%2.2u:%2.2u.%3.3u\"",
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


void DisplayCommandLineHelp(void)
{
    wprintf(L"The command line must include a valid log file name.\n");
}


EXTERN_C
__declspec(dllexport)
void WINAPI WritingPerformanceDataToLogFile(int argc, WCHAR ** argv)
/*
Writing Performance Data to a Log File
Article
01/08/2021
3 contributors
The following example writes real time performance data to a log file.
The example calls the PdhOpenQuery and PdhAddCounter functions to create a query to collect Processor Time counter data.
The example then calls the PdhOpenLog function to create the log file to write the data to.
The example calls the PdhUpdateLog function to collect a sample and update the log file once a second for 20 seconds.

For an example that reads the generated log file, see Reading Performance Data from a Log File.

https://learn.microsoft.com/en-us/windows/win32/perfctrs/writing-performance-data-to-a-log-file
*/
{
    HQUERY hQuery = NULL;
    HLOG hLog = NULL;
    PDH_STATUS pdhStatus{};
    DWORD dwLogType = PDH_LOG_TYPE_CSV;
    HCOUNTER hCounter{};
    DWORD dwCount{};
    CONST PWSTR COUNTER_PATH = (CONST PWSTR)L"\\Processor(0)\\% Processor Time";
    CONST ULONG SAMPLE_INTERVAL_MS = 1000;

    if (argc != 2) {
        DisplayCommandLineHelp();
        goto cleanup;
    }

    // Open a query object.
    pdhStatus = PdhOpenQuery(NULL, 0, &hQuery);
    if (pdhStatus != ERROR_SUCCESS) {
        wprintf(L"PdhOpenQuery failed with 0x%x\n", pdhStatus);
        goto cleanup;
    }

    // Add one counter that will provide the data.
    pdhStatus = PdhAddCounter(hQuery, COUNTER_PATH, 0, &hCounter);
    if (pdhStatus != ERROR_SUCCESS) {
        wprintf(L"PdhAddCounter failed with 0x%x\n", pdhStatus);
        goto cleanup;
    }

    // Open the log file for write access.
    pdhStatus = PdhOpenLog(argv[1],
                           PDH_LOG_WRITE_ACCESS | PDH_LOG_CREATE_ALWAYS,
                           &dwLogType,
                           hQuery,
                           0,
                           NULL,
                           &hLog);
    if (pdhStatus != ERROR_SUCCESS) {
        wprintf(L"PdhOpenLog failed with 0x%x\n", pdhStatus);
        goto cleanup;
    }

    // Write 10 records to the log file.
    for (dwCount = 0; dwCount < 10; dwCount++) {
        wprintf(L"Writing record %u\n", dwCount);

        pdhStatus = PdhUpdateLog(hLog, NULL);
        if (ERROR_SUCCESS != pdhStatus) {
            wprintf(L"PdhUpdateLog failed with 0x%x\n", pdhStatus);
            goto cleanup;
        }

        Sleep(SAMPLE_INTERVAL_MS);// Wait one second between samples for a counter update.
    }

cleanup:
    if (hLog)
        PdhCloseLog(hLog, 0);// Close the log file.    
    if (hQuery)
        PdhCloseQuery(hQuery);// Close the query object.
}


//////////////////////////////////////////////////////////////////////////////////////////////////


//void DisplayCommandLineHelp(void)
//{
//    wprintf(L"The command line must contain a valid log file name.\n");
//}


EXTERN_C
__declspec(dllexport)
void WINAPI ReadingPerformanceDataFromLogFile(int argc, WCHAR ** argv)
/*
Reading Performance Data from a Log File
Article
01/08/2021
3 contributors
The following example reads data written to a log file in the Writing Performance Data to a Log File example.
It uses the PdhCollectQueryData function to retrieve the data from the log file and 
the PdhGetFormattedCounterValue function to format the data for display.

https://learn.microsoft.com/en-us/windows/win32/perfctrs/reading-performance-data-from-a-log-file
*/
{
    HQUERY hQuery = NULL;
    HCOUNTER hCounter = NULL;
    PDH_STATUS status = ERROR_SUCCESS;
    DWORD dwFormat = PDH_FMT_DOUBLE;
    PDH_FMT_COUNTERVALUE ItemBuffer{};
    CONST PWSTR COUNTER_PATH = (CONST PWSTR)L"\\Processor(0)\\% Processor Time";

    if (argc != 2) {
        DisplayCommandLineHelp();
        goto cleanup;
    }

    // Opens the log file to write performance data
    status = PdhOpenQuery(argv[1], 0, &hQuery);
    if (ERROR_SUCCESS != status) {
        wprintf(L"PdhOpenQuery failed with 0x%x\n", status);
        goto cleanup;
    }

    // Add the same counter used when writing the log file.
    status = PdhAddCounter(hQuery, COUNTER_PATH, 0, &hCounter);
    if (ERROR_SUCCESS != status) {
        wprintf(L"PdhAddCounter failed with 0x%x\n", status);
        goto cleanup;
    }

    status = PdhCollectQueryData(hQuery);// Read a performance data record.
    if (ERROR_SUCCESS != status) {
        wprintf(L"PdhCollectQueryData failed with 0x%x\n", status);//0x800007d5 == PDH_NO_DATA
        goto cleanup;
    }

    while (ERROR_SUCCESS == status) {
        status = PdhCollectQueryData(hQuery);// Read the next record
        if (ERROR_SUCCESS == status) {
            // Format the performance data record.
            status = PdhGetFormattedCounterValue(hCounter, dwFormat, (LPDWORD)NULL, &ItemBuffer);
            if (ERROR_SUCCESS != status) {
                wprintf(L"PdhGetFormattedCounterValue failed with 0x%x.\n", status);
                goto cleanup;
            }

            wprintf(L"Formatted counter value = %.20g\n", ItemBuffer.doubleValue);
        } else {
            if (PDH_NO_MORE_DATA != status) {
                wprintf(L"PdhCollectQueryData failed with 0x%x\n", status);
            }
        }
    }

cleanup:
    if (hQuery)
        PdhCloseQuery(hQuery);// Close the query.
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
void WINAPI ConvertingLogFile(int argc, WCHAR ** argv)
/*
Converting Data from a Binary-format Log File to a CSV-format Log File
Article
08/26/2021
4 contributors
The following example transfers data from a counter log file created by the Performance tool to a comma separated format (.csv).
The example transfers Processor Time counter data collected from the local computer.
To specify another type of counter data, change the szCounterPath variable.
If the collected counter data is from a specific computer,
add the computer name to the path (for example, "\\\\<computername>\\Processor(0)\\% Processor Time").

https://learn.microsoft.com/en-us/windows/win32/perfctrs/transferring-data-from-a-perfmon-format-log-file-to-a-csv-format-log-file
*/
{
    HQUERY hQuery = NULL;
    HLOG hOutputLog = NULL;
    HCOUNTER hCounter = NULL;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    DWORD dwOutputLogType = PDH_LOG_TYPE_CSV;
    CONST PWSTR COUNTER_PATH = (CONST PWSTR)L"\\Processor(0)\\% Processor Time";

    if (3 != argc) {
        wprintf(L"Syntax: convertlog <input file name> <output file name>\n"
                L"\nThe input log file must be in the Perfmon format. The output\n"
                L"log file will written in the CSV file format, so specify a .csv extension.");
        goto cleanup;
    }

    // Create the query object using the input log file.
    pdhStatus = PdhOpenQuery(argv[1], 0, &hQuery);
    if (ERROR_SUCCESS != pdhStatus) {
        wprintf(L"PdhOpenQuery failed with 0x%x\n", pdhStatus);
        goto cleanup;
    }

    // Add the counter to the query object; identifies the counter
    // records from the log file that you are going to relog to the new log file.
    pdhStatus = PdhAddCounter(hQuery, COUNTER_PATH, 0, &hCounter);
    if (ERROR_SUCCESS != pdhStatus) {
        wprintf(L"PdhAddCounter failed with 0x%x\n", pdhStatus);
        goto cleanup;
    }

    // Create and open the output log file.
    pdhStatus = PdhOpenLog(argv[2],
                           PDH_LOG_WRITE_ACCESS | PDH_LOG_CREATE_ALWAYS,
                           &dwOutputLogType,
                           hQuery,
                           0,
                           NULL,
                           &hOutputLog);
    if (ERROR_SUCCESS != pdhStatus) {
        wprintf(L"PdhOpenLog failed with 0x%x\n", pdhStatus);
        goto cleanup;
    }

    // Transfer the log records from the input file to the output file.
    while (ERROR_SUCCESS == pdhStatus) {
        pdhStatus = PdhUpdateLog(hOutputLog, NULL);
    }

    if (PDH_NO_MORE_DATA != pdhStatus) {
        wprintf(L"PdhUpdateLog failed with 0x%x\n", pdhStatus);
    }

cleanup:
    if (hOutputLog)
        PdhCloseLog(hOutputLog, 0);// Close the output log file.     
    if (hQuery)
        PdhCloseQuery(hQuery);// Close the query object and input log file.
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
void WINAPI CollectQueryDataEx(void)
/*

https://learn.microsoft.com/zh-cn/windows/win32/api/pdh/nf-pdh-pdhcollectquerydataex
*/
{
    PDH_STATUS Status{};
    HANDLE Event = NULL;
    PDH_HQUERY Query = NULL;
    PDH_HCOUNTER Counter{};
    ULONG WaitResult{};
    ULONG CounterType{};
    PDH_FMT_COUNTERVALUE DisplayValue{};
    //CONST PWSTR COUNTER_NAME = (CONST PWSTR)L"\\Processor(0)\\% Processor Time";
    CONST PWSTR COUNTER_NAME = (CONST PWSTR)L"\\Processor(_Total)\\% Processor Time";
    CONST ULONG SAMPLE_COUNT = 10;
    CONST ULONG SAMPLE_INTERVAL = 2;

    Status = PdhOpenQuery(NULL, 0, &Query);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nPdhOpenQuery failed with status 0x%x.", Status);
        goto Cleanup;
    }

    Status = PdhAddCounter(Query, COUNTER_NAME, 0, &Counter);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nPdhAddCounter failed with 0x%x.", Status);
        goto Cleanup;
    }

    // Calculating the formatted value of some counters requires access to the
    // value of a previous sample. Make this call to get the first sample value
    // populated, to be used later for calculating the next sample.
    Status = PdhCollectQueryData(Query);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nPdhCollectQueryData failed with status 0x%x.", Status);
        goto Cleanup;
    }

    // This will create a separate thread that will collect raw counter data 
    // every 2 seconds and set the supplied Event.
    Event = CreateEvent(NULL, FALSE, FALSE, L"MyEvent");
    if (Event == NULL) {
        wprintf(L"\nCreateEvent failed with status 0x%x.", GetLastError());
        goto Cleanup;
    }

    Status = PdhCollectQueryDataEx(Query, SAMPLE_INTERVAL, Event);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nPdhCollectQueryDataEx failed with status 0x%x.", Status);
        goto Cleanup;
    }

    // Collect and format 10 samples, 2 seconds apart.
    for (ULONG i = 0; i < SAMPLE_COUNT; i++) {
        WaitResult = WaitForSingleObject(Event, INFINITE);
        if (WaitResult == WAIT_OBJECT_0) {
            Status = PdhGetFormattedCounterValue(Counter, PDH_FMT_DOUBLE, &CounterType, &DisplayValue);
            if (Status == ERROR_SUCCESS) {
                wprintf(L"\nCounter Value: %.20g", DisplayValue.doubleValue);
            } else {
                wprintf(L"\nPdhGetFormattedCounterValue failed with status 0x%x.", Status);
                goto Cleanup;
            }
        } else if (WaitResult == WAIT_FAILED) {
            wprintf(L"\nWaitForSingleObject failed with status 0x%x.", GetLastError());
            goto Cleanup;
        }
    }

Cleanup:

    if (Event) {
        CloseHandle(Event);
    }

    // This will close both the Query handle and all associated Counter handles returned by PdhAddCounter.

    if (Query) {
        PdhCloseQuery(Query);
    }
}


EXTERN_C
__declspec(dllexport)
void WINAPI CollectPerformanceData(_In_ LPCWSTR FullCounterPath, _In_ DWORD Format)
/*

参数：
FullCounterPath，形如：L"\\Processor(_Total)\\% Processor Time"


https://learn.microsoft.com/zh-cn/windows/win32/api/pdh/nf-pdh-pdhcollectquerydataex
*/
{
    PDH_STATUS Status{};
    HANDLE Event = NULL;
    PDH_HQUERY Query = NULL;
    PDH_HCOUNTER Counter{};
    ULONG WaitResult{};
    ULONG CounterType{};
    PDH_FMT_COUNTERVALUE DisplayValue{};

    Status = PdhOpenQuery(NULL, 0, &Query);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nPdhOpenQuery failed with status 0x%x.", Status);
        goto Cleanup;
    }

    Status = PdhAddCounter(Query, FullCounterPath, 0, &Counter);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nPdhAddCounter failed with 0x%x.", Status);
        goto Cleanup;
    }

    // Calculating the formatted value of some counters requires access to the value of a previous sample.
    // Make this call to get the first sample value populated, to be used later for calculating the next sample.
    Status = PdhCollectQueryData(Query);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nPdhCollectQueryData failed with status 0x%x.", Status);
        goto Cleanup;
    }

    // This will create a separate thread that will collect raw counter data 
    // every 2 seconds and set the supplied Event.
    Event = CreateEvent(NULL, FALSE, FALSE, L"PerformanceEvent");
    if (Event == NULL) {
        wprintf(L"\nCreateEvent failed with status 0x%x.", GetLastError());
        goto Cleanup;
    }

    Status = PdhCollectQueryDataEx(Query, 1, Event);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nPdhCollectQueryDataEx failed with status 0x%x.", Status);
        goto Cleanup;
    }

    for (;;) {
        WaitResult = WaitForSingleObject(Event, INFINITE);
        if (WaitResult == WAIT_OBJECT_0) {
            Status = PdhGetFormattedCounterValue(Counter, Format, &CounterType, &DisplayValue);
            if (Status == ERROR_SUCCESS) {
                switch (Format) {
                case PDH_FMT_LONG:
                    wprintf(L"\nCounter Value: %d", DisplayValue.longValue);
                    break;
                case PDH_FMT_ANSI:
                    wprintf(L"\nCounter Value: %hs", DisplayValue.AnsiStringValue);
                    break;
                case PDH_FMT_UNICODE:
                    wprintf(L"\nCounter Value: %ls", DisplayValue.WideStringValue);
                    break;
                case PDH_FMT_DOUBLE:
                    wprintf(L"\nCounter Value: %.3g", DisplayValue.doubleValue);
                    break;
                case PDH_FMT_LARGE:
                    wprintf(L"\nCounter Value: %lld", DisplayValue.largeValue);
                    break;
                default:
                    break;
                }
            } else {
                wprintf(L"\nPdhGetFormattedCounterValue failed with status 0x%x.", Status);
                goto Cleanup;
            }
        } else if (WaitResult == WAIT_FAILED) {
            wprintf(L"\nWaitForSingleObject failed with status 0x%x.", GetLastError());
            goto Cleanup;
        }
    }

Cleanup:

    if (Event) {
        CloseHandle(Event);
    }

    // This will close both the Query handle and all associated Counter handles returned by PdhAddCounter.

    if (Query) {
        PdhCloseQuery(Query);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
void WINAPI GetFormattedCounterArray()
/*

https://learn.microsoft.com/zh-cn/windows/win32/api/pdh/nf-pdh-pdhgetformattedcounterarraya
*/
{
    PDH_HQUERY hQuery = NULL;
    PDH_STATUS status = ERROR_SUCCESS;
    PDH_HCOUNTER hCounter = NULL;
    DWORD dwBufferSize = 0;         // Size of the pItems buffer
    DWORD dwItemCount = 0;          // Number of items in the pItems buffer
    PDH_FMT_COUNTERVALUE_ITEM * pItems = NULL;  // Array of PDH_FMT_COUNTERVALUE_ITEM structures
    CONST PWSTR COUNTER_PATH = (CONST PWSTR)L"\\Processor(*)\\% Processor Time";
    CONST ULONG SAMPLE_INTERVAL_MS = 1000;

    if (status = PdhOpenQuery(NULL, 0, &hQuery)) {
        wprintf(L"PdhOpenQuery failed with 0x%x.\n", status);
        goto cleanup;
    }

    // Specify a counter object with a wildcard for the instance.
    if (status = PdhAddCounter(hQuery, COUNTER_PATH, 0, &hCounter)) {
        wprintf(L"PdhAddCounter failed with 0x%x.\n", status);
        goto cleanup;
    }

    // Some counters need two sample in order to format a value, so
    // make this call to get the first value before entering the loop.
    if (status = PdhCollectQueryData(hQuery)) {
        wprintf(L"PdhCollectQueryData failed with 0x%x.\n", status);
        goto cleanup;
    }

    for (int i = 0; i < 10; i++) {
        Sleep(SAMPLE_INTERVAL_MS);

        if (status = PdhCollectQueryData(hQuery)) {
            wprintf(L"PdhCollectQueryData failed with 0x%x.\n", status);
            goto cleanup;
        }

        // Get the required size of the pItems buffer.
        status = PdhGetFormattedCounterArray(hCounter, PDH_FMT_DOUBLE, &dwBufferSize, &dwItemCount, pItems);
        if (PDH_MORE_DATA == status) {
            pItems = (PDH_FMT_COUNTERVALUE_ITEM *)malloc(dwBufferSize);
            if (pItems) {
                status = PdhGetFormattedCounterArray(hCounter, PDH_FMT_DOUBLE, &dwBufferSize, &dwItemCount, pItems);
                if (ERROR_SUCCESS == status) {
                    // Loop through the array and print the instance name and counter value.
                    for (DWORD i = 0; i < dwItemCount; i++) {
                        wprintf(L"counter: %s, value %.20g\n", pItems[i].szName, pItems[i].FmtValue.doubleValue);
                    }
                } else {
                    wprintf(L"Second PdhGetFormattedCounterArray call failed with 0x%x.\n", status);
                    goto cleanup;
                }

                free(pItems);
                pItems = NULL;
                dwBufferSize = dwItemCount = 0;
            } else {
                wprintf(L"malloc for PdhGetFormattedCounterArray failed.\n");
                goto cleanup;
            }
        } else {
            wprintf(L"PdhGetFormattedCounterArray failed with 0x%x.\n", status);
            goto cleanup;
        }
    }

cleanup:

    if (pItems)
        free(pItems);

    if (hQuery)
        PdhCloseQuery(hQuery); // Closes all counter handles and the query handle
}


EXTERN_C
__declspec(dllexport)
void WINAPI CollectPerformanceDatas(_In_  LPCWSTR FullCounterPath, _In_ DWORD Format)
/*

参数：
FullCounterPath的取值，如下：
L"\\Processor(*)\\% Processor Time"
L"\\Process(*)\\% Processor Time"
L"\\Process(*)\\Working Set"

https://learn.microsoft.com/zh-cn/windows/win32/api/pdh/nf-pdh-pdhgetformattedcounterarraya
*/
{
    PDH_HQUERY hQuery = NULL;
    PDH_STATUS status = ERROR_SUCCESS;
    PDH_HCOUNTER hCounter = NULL;
    DWORD dwBufferSize = 0;         // Size of the pItems buffer
    DWORD dwItemCount = 0;          // Number of items in the pItems buffer
    PDH_FMT_COUNTERVALUE_ITEM * pItems = NULL;  // Array of PDH_FMT_COUNTERVALUE_ITEM structures
    //CONST PWSTR COUNTER_PATH = (CONST PWSTR)L"\\Processor(*)\\% Processor Time";

    if (status = PdhOpenQuery(NULL, 0, &hQuery)) {
        wprintf(L"PdhOpenQuery failed with 0x%x.\n", status);
        goto cleanup;
    }

    // Specify a counter object with a wildcard for the instance.
    if (status = PdhAddCounter(hQuery, FullCounterPath, 0, &hCounter)) {//这个消耗点时间。
        wprintf(L"PdhAddCounter failed with 0x%x.\n", status);
        goto cleanup;
    }

    // Some counters need two sample in order to format a value, so
    // make this call to get the first value before entering the loop.
    if (status = PdhCollectQueryData(hQuery)) {
        wprintf(L"PdhCollectQueryData failed with 0x%x.\n", status);
        goto cleanup;
    }

    for (;;) {
        Sleep(1000);

        if (status = PdhCollectQueryData(hQuery)) {
            wprintf(L"PdhCollectQueryData failed with 0x%x.\n", status);
            goto cleanup;
        }

        // Get the required size of the pItems buffer.
        status = PdhGetFormattedCounterArray(hCounter, Format, &dwBufferSize, &dwItemCount, pItems);
        if (PDH_MORE_DATA == status) {
            pItems = (PDH_FMT_COUNTERVALUE_ITEM *)malloc(dwBufferSize);
            if (pItems) {
                status = PdhGetFormattedCounterArray(hCounter, Format, &dwBufferSize, &dwItemCount, pItems);
                if (ERROR_SUCCESS == status) {
                    // Loop through the array and print the instance name and counter value.
                    /*
                    谨记：
                    如果设置了
                     HKLM\System\CurrentControlSet\Services\Perfproc\Performance
                     ProcessNameFormat 或 ThreadNameFormat 为 2
                    这里的szName显示有所改变，多了_ID。
                    */
                    for (DWORD i = 0; i < dwItemCount; i++) {
                        switch (Format) {
                        case PDH_FMT_LONG:
                            wprintf(L"counter: %-32s, value %d\n", pItems[i].szName, pItems[i].FmtValue.longValue);
                            break;
                        case PDH_FMT_ANSI:
                            wprintf(L"counter: %-32s, value %hs\n", pItems[i].szName, pItems[i].FmtValue.AnsiStringValue);
                            break;
                        case PDH_FMT_UNICODE:
                            wprintf(L"counter: %-32s, value %ls\n", pItems[i].szName, pItems[i].FmtValue.WideStringValue);
                            break;
                        case PDH_FMT_DOUBLE:
                            wprintf(L"counter: %-32s, value %.3g\n", pItems[i].szName, pItems[i].FmtValue.doubleValue);
                            break;
                        case PDH_FMT_LARGE:
                            wprintf(L"counter: %-32s, value %lld\n", pItems[i].szName, pItems[i].FmtValue.largeValue);
                            break;
                        default:
                            break;
                        }
                    }
                } else {
                    wprintf(L"Second PdhGetFormattedCounterArray call failed with 0x%x.\n", status);
                    goto cleanup;
                }

                free(pItems);
                pItems = NULL;
                dwBufferSize = dwItemCount = 0;
            } else {
                wprintf(L"malloc for PdhGetFormattedCounterArray failed.\n");
                goto cleanup;
            }
        } else {
            wprintf(L"PdhGetFormattedCounterArray failed with 0x%x.\n", status);
            goto cleanup;
        }
    }

cleanup:
    if (pItems)
        free(pItems);
    if (hQuery)
        PdhCloseQuery(hQuery); // Closes all counter handles and the query handle
}


//////////////////////////////////////////////////////////////////////////////////////////////////