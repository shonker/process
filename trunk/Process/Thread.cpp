#include "pch.h"
#include "Thread.h"


#pragma warning(disable:6001)


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
BOOL WINAPI ListProcessThreads(DWORD dwOwnerPID)
/*
Traversing the Thread List
05/31/2018

The following example function lists running threads for a specified process.
First, the ListProcessThreads function takes a snapshot of the currently executing threads in the system using CreateToolhelp32Snapshot,
and then it walks through the list recorded in the snapshot using the Thread32First and Thread32Next functions.
The parameter for ListProcessThreads is the process identifier of the process whose threads are to be listed.

用法：ListProcessThreads(GetCurrentProcessId());

https://docs.microsoft.com/en-us/windows/win32/toolhelp/traversing-the-thread-list
*/
{
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    // Take a snapshot of all running threads  
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return(FALSE);

    // Fill in the size of the structure before using it. 
    te32.dwSize = sizeof(THREADENTRY32);

    // Retrieve information about the first thread,
    // and exit if unsuccessful
    if (!Thread32First(hThreadSnap, &te32)) {
        printError(TEXT("Thread32First"));  // Show cause of failure
        CloseHandle(hThreadSnap);     // Must clean up the snapshot object!
        return(FALSE);
    }

    // Now walk the thread list of the system,
    // and display information about each thread
    // associated with the specified process
    do {
        if (te32.th32OwnerProcessID == dwOwnerPID) {
            _tprintf(TEXT("\n     THREAD ID      = 0x%08X"), te32.th32ThreadID);
            _tprintf(TEXT("\n     base priority  = %d"), te32.tpBasePri);
            _tprintf(TEXT("\n     delta priority = %d"), te32.tpDeltaPri);
        }
    } while (Thread32Next(hThreadSnap, &te32));

    _tprintf(TEXT("\n"));

    //  Don't forget to clean up the snapshot object.
    CloseHandle(hThreadSnap);
    return(TRUE);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


#define MAX_THREADS 3
#define BUF_SIZE 255

DWORD WINAPI MyThreadFunction(LPVOID lpParam);

// Sample custom data structure for threads to use.
// This is passed by void pointer so it can be any data type
// that can be passed using a single void pointer (LPVOID).
typedef struct MyData {
    int val1;
    int val2;
} MYDATA, * PMYDATA;


EXTERN_C
__declspec(dllexport)
int WINAPI CreatingThreads()
/*
Creating Threads
2018/05/31

The CreateThread function creates a new thread for a process.
The creating thread must specify the starting address of the code that the new thread is to execute.
Typically, the starting address is the name of a function defined in the program code (for more information, see ThreadProc).
This function takes a single parameter and returns a DWORD value.
A process can have multiple threads simultaneously executing the same function.

The following is a simple example that demonstrates how to create a new thread that executes the locally defined function, MyThreadFunction.

The calling thread uses the WaitForMultipleObjects function to persist until all worker threads have terminated.
The calling thread blocks while it is waiting;
to continue processing, a calling thread would use WaitForSingleObject and wait for each worker thread to signal its wait object.
Note that if you were to close the handle to a worker thread before it terminated, this does not terminate the worker thread.
However, the handle will be unavailable for use in subsequent function calls.

https://docs.microsoft.com/zh-cn/windows/win32/procthread/creating-threads
*/
{
    PMYDATA pDataArray[MAX_THREADS];
    DWORD   dwThreadIdArray[MAX_THREADS];
    HANDLE  hThreadArray[MAX_THREADS] = { 0 };

    // Create MAX_THREADS worker threads.
    for (int i = 0; i < MAX_THREADS; i++) {
        // Allocate memory for thread data.
        pDataArray[i] = (PMYDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MYDATA));
        if (pDataArray[i] == NULL) {
            // If the array allocation fails, the system is out of memory
            // so there is no point in trying to print an error message.
            // Just terminate execution.
            ExitProcess(2);
        }

        // Generate unique data for each thread to work with.
        pDataArray[i]->val1 = i;
        pDataArray[i]->val2 = i + 100;

        // Create the thread to begin execution on its own.
        hThreadArray[i] = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            MyThreadFunction,       // thread function name
            pDataArray[i],          // argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray[i]);   // returns the thread identifier 

        // Check the return value for success.
        // If CreateThread fails, terminate execution. 
        // This will automatically clean up threads and memory. 
        if (hThreadArray[i] == NULL) {
            ErrorHandler(TEXT("CreateThread"));
            ExitProcess(3);
        }
    } // End of main thread creation loop.

    // Wait until all threads have terminated.
    WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

    // Close all thread handles and free memory allocations.
    for (int i = 0; i < MAX_THREADS; i++) {
        CloseHandle(hThreadArray[i]);
        if (pDataArray[i] != NULL) {
            HeapFree(GetProcessHeap(), 0, pDataArray[i]);
            pDataArray[i] = NULL;    // Ensure address is not reused.
        }
    }

    return 0;
}


DWORD WINAPI MyThreadFunction(LPVOID lpParam)
{
    HANDLE hStdout;
    PMYDATA pDataArray;
    TCHAR msgBuf[BUF_SIZE];
    size_t cchStringSize;
    DWORD dwChars;

    // Make sure there is a console to receive output results. 
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout == INVALID_HANDLE_VALUE)
        return 1;

    // Cast the parameter to the correct data type.
    // The pointer is known to be valid because 
    // it was checked for NULL before the thread was created.
    pDataArray = (PMYDATA)lpParam;

    // Print the parameter values using thread-safe functions.
    StringCchPrintf(msgBuf, BUF_SIZE, TEXT("Parameters = %d, %d\n"),
        pDataArray->val1, pDataArray->val2);
    (void)StringCchLength(msgBuf, BUF_SIZE, &cchStringSize);
    WriteConsole(hStdout, msgBuf, (DWORD)cchStringSize, &dwChars, NULL);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


#define INFO_BUFFER_SIZE	260


void ReportError(LPCWSTR pszFunction, DWORD dwError = GetLastError())
//   FUNCTION: ReportError(LPWSTR, DWORD)
//
//   PURPOSE: Display an error message for the failure of a certain function.
//
//   PARAMETERS:
//   * pszFunction - the name of the function that failed.
//   * dwError - the Win32 error code. Its default value is the calling 
//   thread's last-error code value.
//
//   NOTE: The failing function must be immediately followed by the call of 
//   ReportError if you do not explicitly specify the dwError parameter of 
//   ReportError. This is to ensure that the calling thread's last-error code 
//   value is not overwritten by any calls of API between the failing function and ReportError.
{
    wprintf(L"%s failed w/err 0x%08lx\n", pszFunction, dwError);
}


int ImpersonateUser(int argc, wchar_t* argv[])
/***************************** Module Header *******************************\
* Module Name:  CppImpersonateUser.cpp
* Project:      CppImpersonateUser
* Copyright (c) Microsoft Corporation.
*
* 代码出处：User impersonation demo (CppImpersonateUser)
*
* Windows Impersonation is a powerful feature Windows uses frequently in its
* security model. In general Windows also uses impersonation in its client/
* server programming model.Impersonation lets a server to temporarily adopt
* the security profile of a client making a resource request. The server can
* then access resources on behalf of the client, and the OS carries out the
* access validations.
* A server impersonates a client only within the thread that makes the
* impersonation request. Thread-control data structures contain an optional
* entry for an impersonation token. However, a thread's primary token, which
* represents the thread's real security credentials, is always accessible in
* the process's control structure.
*
* After the server thread finishes its task, it reverts to its primary
* security profile. These forms of impersonation are convenient for carrying
* out specific actions at the request of a client and for ensuring that object
* accesses are audited correctly.
*
* In this code sample we use the LogonUser API and the ImpersonateLoggedOnUser
* API to impersonate the user represented by the specified user token. Then
* display the current user via the GetUserName API to show user impersonation.
* LogonUser can only be used to log onto the local machine; it cannot log you
* onto a remote computer. The account that you use in the LogonUser() call
* must also be known to the local machine, either as a local account or as a
* domain account that is visible to the local computer. If LogonUser is
* successful, then it will give you an access token that specifies the
* credentials of the user account you chose.
*
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/licenses.aspx#MPL
* All other rights reserved.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/
{
    wchar_t szCurrentUserName[INFO_BUFFER_SIZE] = {};
    wchar_t szUserName[INFO_BUFFER_SIZE] = {};
    wchar_t szDomain[INFO_BUFFER_SIZE] = {};
    wchar_t szPassword[INFO_BUFFER_SIZE] = {};
    wchar_t* pc = NULL;
    HANDLE hToken = NULL;
    BOOL fSucceeded = FALSE;

    // Print the name of the user associated with the current thread.
    wprintf(L"Before the impersonation ...\n");
    DWORD nSize = ARRAYSIZE(szCurrentUserName);
    if (!GetUserName(szCurrentUserName, &nSize)) {
        ReportError(L"GetUserName");
        goto Cleanup;
    }
    wprintf(L"The current user is %s\n\n", szCurrentUserName);

    // Gather the credential information of the impersonated user.

    wprintf(L"Enter the name of the impersonated user: ");
    fgetws(szUserName, ARRAYSIZE(szUserName), stdin);
    pc = wcschr(szUserName, L'\n');
    if (pc != NULL) *pc = L'\0';  // Remove the trailing L'\n'

    wprintf(L"Enter the domain name: ");
    fgetws(szDomain, ARRAYSIZE(szDomain), stdin);
    pc = wcschr(szDomain, L'\n');
    if (pc != NULL) *pc = L'\0';  // Remove the trailing L'\n'

    wprintf(L"Enter the password: ");
    fgetws(szPassword, ARRAYSIZE(szPassword), stdin);
    pc = wcschr(szPassword, L'\n');
    if (pc != NULL) *pc = L'\0';  // Remove the trailing L'\n'

    // Attempt to log on the user.
    if (!LogonUser(szUserName, szDomain, szPassword, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken)) {
        ReportError(L"LogonUser");
        goto Cleanup;
    }

    // Impersonate the logged on user.
    if (!ImpersonateLoggedOnUser(hToken)) {
        ReportError(L"ImpersonateLoggedOnUser");
        goto Cleanup;
    }

    // The impersonation is successful.
    fSucceeded = TRUE;
    wprintf(L"\nThe impersonation is successful\n");

    // Print the name of the user associated with the current thread.
    ZeroMemory(szCurrentUserName, sizeof(szCurrentUserName));
    nSize = ARRAYSIZE(szCurrentUserName);
    if (!GetUserName(szCurrentUserName, &nSize)) {
        ReportError(L"GetUserName");
        goto Cleanup;
    }
    wprintf(L"The current user is %s\n\n", szCurrentUserName);

    // Work as the impersonated user.
    // ...

Cleanup:

    // Clean up the buffer containing sensitive password.
    SecureZeroMemory(szPassword, sizeof(szPassword));

    // If the impersonation was successful, undo the impersonation.
    if (fSucceeded) {
        wprintf(L"Undo the impersonation ...\n");
        if (!RevertToSelf()) {
            ReportError(L"RevertToSelf");
        }

        // Print the name of the user associated with the current thread.
        ZeroMemory(szCurrentUserName, sizeof(szCurrentUserName));
        nSize = ARRAYSIZE(szCurrentUserName);
        if (!GetUserName(szCurrentUserName, &nSize)) {
            ReportError(L"GetUserName");
        }
        wprintf(L"The current user is %s\n\n", szCurrentUserName);
    }

    if (hToken) {
        CloseHandle(hToken);
        hToken = NULL;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
