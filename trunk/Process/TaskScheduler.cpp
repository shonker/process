#include "pch.h"
#include "TaskScheduler.h"


#pragma warning (disable: 4100) //未引用的形参


//////////////////////////////////////////////////////////////////////////////////////////////////


// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// This sample enumerates all the tasks registered in the root task 
// folder on the local computer and displays their name and status.


EXTERN_C
__declspec(dllexport)
void WINAPI EnumTaskScheduler(void)
/*
\Windows-classic-samples\Samples\Win7Samples\sysmgmt\tasksched\registeredtaskenum\TaskEnumeration_Example.cpp
*/
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);//  Initialize COM.
    if (FAILED(hr)) {
        printf("\nCoInitializeEx failed: %x", hr);
        return;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, 0, nullptr);
    if (FAILED(hr)) {
        printf("\nCoInitializeSecurity failed: %x", hr);
        return;
    }

    //  Create an instance of the Task Service. 
    ITaskService * pService = nullptr;
    hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, (void **)&pService);
    if (FAILED(hr)) {
        printf("Failed to CoCreate an instance of the TaskService class: %x", hr);
        CoUninitialize();
        return;
    }

    //  Connect to the local task service.
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        printf("ITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return;
    }

    //  Get the pointer to the root task folder.
    ITaskFolder * pRootFolder = nullptr;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    pService->Release();
    if (FAILED(hr)) {
        printf("Cannot get Root Folder pointer: %x", hr);
        CoUninitialize();
        return;
    }

    //  Get the registered tasks in the folder.
    IRegisteredTaskCollection * pTaskCollection = nullptr;
    hr = pRootFolder->GetTasks(0, &pTaskCollection);
    pRootFolder->Release();
    if (FAILED(hr)) {
        printf("Cannot get the registered tasks.: %x", hr);
        CoUninitialize();
        return;
    }

    LONG numTasks = 0;
    hr = pTaskCollection->get_Count(&numTasks);
    if (FAILED(hr)) {
        printf("Cannot get the task collection.: %x", hr);
        CoUninitialize();
        return;
    }

    if (numTasks == 0) {
        printf("\nNo Tasks are currently running");
        pTaskCollection->Release();
        CoUninitialize();
        return;
    }

    printf("\nNumber of Tasks : %d", numTasks);

    //  Visit each task in the folder.
    for (LONG i = 0; i < numTasks; i++) {
        IRegisteredTask * pRegisteredTask = nullptr;
        _bstr_t taskName{};
        TASK_STATE taskState{};
        _bstr_t taskStateStr{};

        hr = pTaskCollection->get_Item(_variant_t(i + 1), &pRegisteredTask);
        if (FAILED(hr)) {
            printf("Cannot get the registered task: %x", hr);
            continue;
        }

        hr = pRegisteredTask->get_Name(taskName.GetAddress());
        if (FAILED(hr)) {
            printf("Cannot get the registered task name: %x", hr);
            pRegisteredTask->Release();
            continue;
        }

        hr = pRegisteredTask->get_State(&taskState);
        if (FAILED(hr)) {
            printf("Cannot get the registered task state: %x", hr);
        } else {
            printf("\n\nTask Name: %S", (LPCWSTR)taskName);

            switch (taskState) {
            case TASK_STATE_DISABLED:
                taskStateStr = "disabled";
                break;
            case TASK_STATE_QUEUED:
                taskStateStr = "queued";
                break;
            case TASK_STATE_READY:
                taskStateStr = "ready";
                break;
            case TASK_STATE_RUNNING:
                taskStateStr = "running";
                break;
            default:
                taskStateStr = "unknown";
                break;
            }
            printf("\n\tState: %s", (LPCSTR)taskStateStr);
        }

        pRegisteredTask->Release();
    }

    pTaskCollection->Release();
    CoUninitialize();
    return;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


/********************************************************************
 This sample schedules a task to start notepad.exe 1 minute from the
 time the task is registered.
********************************************************************/


EXTERN_C
__declspec(dllexport)
int WINAPI StartingExecutableAtSpecificTime()
/*
Time Trigger Example (C++)
05/31/2018

This C++ example shows how to create a task that is scheduled to execute Notepad at a specified time.
The task contains a time-based trigger that specifies a start boundary and an end boundary for the task.
The task also contains an action that specifies the task to execute Notepad.
The task is registered using an interactive logon type, which means the task runs under the security context of the user who runs the application.
The task also contains idle settings, which specifies how Task Scheduler performs tasks when the computer is in an idle condition.

The following procedure describes how to schedule a task to start an executable at a certain time.

To schedule Notepad to start at a specific time

Initialize COM and set general COM security.

Create the ITaskService object.

This object allows you to create tasks in a specified folder.

Get a task folder to create a task in.

Use the ITaskService::GetFolder method to get the folder, and the ITaskService::NewTask method to create the ITaskDefinition object.

Define information about the task using the ITaskDefinition object, such as the registration information for the task.

Use the RegistrationInfo property of ITaskDefinition and other properties of the ITaskDefinition interface to define the task information.

Create a time-based trigger using the Triggers property of ITaskDefinition to access the ITriggerCollection for the task.

Use the ITriggerCollection::Create method (specifying the type of trigger you want to create) to create a time-based trigger.
This allows you to set the start boundary and the end boundary for the trigger so that the task's actions will be scheduled to execute at a specified time.

Create an action for the task to execute by using the Actions property of ITaskDefinition to access the IActionCollection interface for the task.

Use the IActionCollection::Create method to specify the type of action that you want to create.
This example uses an IExecAction object, which represents an action that executes a command-line operation.

Register the task using the ITaskFolder::RegisterTaskDefinition method.

The following C++ example shows how to schedule a task to execute Notepad one minute after the task is registered.

https://docs.microsoft.com/en-us/windows/win32/taskschd/time-trigger-example--c---
*/
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);//  Initialize COM.
    if (FAILED(hr)) {
        printf("\nCoInitializeEx failed: %x", hr);
        return 1;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, 0, nullptr);
    if (FAILED(hr)) {
        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return 1;
    }

    LPCWSTR wszTaskName = L"Time Trigger Test Task";//  Create a name for the task.

    //  Get the windows directory and set the path to notepad.exe.
    wstring wstrExecutablePath = _wgetenv(L"WINDIR");
    wstrExecutablePath += L"\\SYSTEM32\\NOTEPAD.EXE";

    //  Create an instance of the Task Service. 
    ITaskService * pService = nullptr;
    hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, (void **)&pService);
    if (FAILED(hr)) {
        printf("Failed to create an instance of ITaskService: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        printf("ITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the pointer to the root task folder.  
    //  This folder will hold the new task that is registered.
    ITaskFolder * pRootFolder = nullptr;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        printf("Cannot get Root folder pointer: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }
    
    pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);//  If the same task exists, remove it.

    //  Create the task definition object to create the task.
    ITaskDefinition * pTask = nullptr;
    hr = pService->NewTask(0, &pTask);
    pService->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr)) {
        printf("Failed to CoCreate an instance of the TaskService class: %x", hr);
        pRootFolder->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the registration info for setting the identification.
    IRegistrationInfo * pRegInfo = nullptr;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        printf("\nCannot get identification pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRegInfo->put_Author((BSTR)L"Author Name");
    pRegInfo->Release();
    if (FAILED(hr)) {
        printf("\nCannot put identification info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the principal for the task - these credentials are overwritten with the credentials passed to RegisterTaskDefinition
    IPrincipal * pPrincipal = nullptr;
    hr = pTask->get_Principal(&pPrincipal);
    if (FAILED(hr)) {
        printf("\nCannot get principal pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set up principal logon type to interactive logon
    hr = pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
    pPrincipal->Release();
    if (FAILED(hr)) {
        printf("\nCannot put principal info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the settings for the task
    ITaskSettings * pSettings = nullptr;
    hr = pTask->get_Settings(&pSettings);
    if (FAILED(hr)) {
        printf("\nCannot get settings pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set setting values for the task.  
    hr = pSettings->put_StartWhenAvailable(VARIANT_TRUE);
    pSettings->Release();
    if (FAILED(hr)) {
        printf("\nCannot put setting information: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    // Set the idle settings for the task.
    IIdleSettings * pIdleSettings = nullptr;
    hr = pSettings->get_IdleSettings(&pIdleSettings);
    if (FAILED(hr)) {
        printf("\nCannot get idle setting information: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pIdleSettings->put_WaitTimeout((BSTR)L"PT5M");
    pIdleSettings->Release();
    if (FAILED(hr)) {
        printf("\nCannot put idle setting information: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the trigger collection to insert the time trigger.
    ITriggerCollection * pTriggerCollection = nullptr;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        printf("\nCannot get trigger collection: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add the time trigger to the task.
    ITrigger * pTrigger = nullptr;
    hr = pTriggerCollection->Create(TASK_TRIGGER_TIME, &pTrigger);
    pTriggerCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    ITimeTrigger * pTimeTrigger = nullptr;
    hr = pTrigger->QueryInterface(IID_ITimeTrigger, (void **)&pTimeTrigger);
    pTrigger->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call failed for ITimeTrigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pTimeTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr))
        printf("\nCannot put trigger ID: %x", hr);

    hr = pTimeTrigger->put_EndBoundary(_bstr_t(L"2015-05-02T08:00:00"));
    if (FAILED(hr))
        printf("\nCannot put end boundary on trigger: %x", hr);

    //  Set the task to start at a certain time. 
    //  The time format should be YYYY-MM-DDTHH:MM:SS(+-)(timezone).
    //  For example, the start boundary below is January 1st 2005 at 12:05
    hr = pTimeTrigger->put_StartBoundary(_bstr_t(L"2005-01-01T12:05:00"));
    pTimeTrigger->Release();
    if (FAILED(hr)) {
        printf("\nCannot add start boundary to trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add an action to the task. This task will execute notepad.exe.     
    IActionCollection * pActionCollection = nullptr;

    //  Get the task action collection pointer.
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        printf("\nCannot get Task collection pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the action, specifying that it is an executable action.
    IAction * pAction = nullptr;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    pActionCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create the action: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IExecAction * pExecAction = nullptr;
    //  QI for the executable task pointer.
    hr = pAction->QueryInterface(IID_IExecAction, (void **)&pExecAction);
    pAction->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call failed for IExecAction: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set the path of the executable to notepad.exe.
    hr = pExecAction->put_Path(_bstr_t(wstrExecutablePath.c_str()));
    pExecAction->Release();
    if (FAILED(hr)) {
        printf("\nCannot put action path: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Save the task in the root folder.
    IRegisteredTask * pRegisteredTask = nullptr;
    hr = pRootFolder->RegisterTaskDefinition(_bstr_t(wszTaskName),
                                             pTask,
                                             TASK_CREATE_OR_UPDATE,
                                             _variant_t(),
                                             _variant_t(),
                                             TASK_LOGON_INTERACTIVE_TOKEN,
                                             _variant_t(L""),
                                             &pRegisteredTask);
    if (FAILED(hr)) {
        printf("\nError saving the Task : %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    printf("\n Success! Task successfully registered. ");

    //  Clean up.
    pRootFolder->Release();
    pTask->Release();
    pRegisteredTask->Release();
    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


/********************************************************************
 This sample schedules a task to start on a daily basis.
********************************************************************/


EXTERN_C
__declspec(dllexport)
int WINAPI StartingExecutableDaily()
/*
Daily Trigger Example (C++)
05/31/2018
6 minutes to read

This C++ example shows how to create a task that is scheduled to execute Notepad on a daily basis.
The task contains a daily trigger that specifies a start boundary and a day interval for the task to start.
The example also shows how to set a repetition pattern for the trigger to repeat the task.
The task also contains an action that specifies the task to execute Notepad.

The following procedure describes how to schedule a task to start an executable on a daily basis.

To schedule Notepad to start on a daily basis

Initialize COM and set general COM security.

Create the ITaskService object.

This object allows you to create tasks in a specified folder.

Get a task folder to create a task in.

Use the ITaskService::GetFolder method to get the folder, and the ITaskService::NewTask method to create the ITaskDefinition object.

Define information about the task using the ITaskDefinition object, such as the registration information for the task.

Use the RegistrationInfo property of ITaskDefinition and other properties of the ITaskDefinition interface to define the task information.

Create a daily trigger using the Triggers property of ITaskDefinition to access the ITriggerCollection interface for the task.

Use the ITriggerCollection::Create method to specify that you want to create a daily trigger.
You can set the start boundary and the days interval for the trigger so that the task's actions will be scheduled to execute at a specified time on certain days.
The example also shows how to set a repetition pattern for the trigger to repeat the task.

Create an action for the task to execute by using the Actions property of ITaskDefinition to access the IActionCollection interface for the task.

Use the IActionCollection::Create method to specify the type of action that you want to create.
This example uses an IExecAction object, which represents an action that executes a command-line operation.

Register the task using the ITaskFolder::RegisterTaskDefinition method.

The following C++ example shows how to schedule a task to execute Notepad on a daily basis.

https://docs.microsoft.com/en-us/windows/win32/taskschd/daily-trigger-example--c---
*/
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);//  Initialize COM.
    if (FAILED(hr)) {
        printf("\nCoInitializeEx failed: %x", hr);
        return 1;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, 0, nullptr);
    if (FAILED(hr)) {
        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return 1;
    }

    LPCWSTR wszTaskName = L"Daily Trigger Test Task";//  Create a name for the task.

    //  Get the windows directory and set the path to notepad.exe.
    wstring wstrExecutablePath = _wgetenv(L"WINDIR");
    wstrExecutablePath += L"\\SYSTEM32\\NOTEPAD.EXE";

    //  Create an instance of the Task Service. 
    ITaskService * pService = nullptr;
    hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, (void **)&pService);
    if (FAILED(hr)) {
        printf("Failed to create an instance of ITaskService: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        printf("ITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the pointer to the root task folder.  This folder will hold the new task that is registered.
    ITaskFolder * pRootFolder = nullptr;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        printf("Cannot get Root Folder pointer: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }
    
    pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);// If the same task exists, remove it.

    //  Create the task builder object to create the task.
    ITaskDefinition * pTask = nullptr;
    hr = pService->NewTask(0, &pTask);
    pService->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr)) {
        printf("Failed to CoCreate an instance of the TaskService class: %x", hr);
        pRootFolder->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the registration info for setting the identification.
    IRegistrationInfo * pRegInfo = nullptr;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        printf("\nCannot get identification pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRegInfo->put_Author(_bstr_t(L"Author Name"));
    pRegInfo->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr)) {
        printf("\nCannot put identification info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the trigger collection to insert the daily trigger.
    ITriggerCollection * pTriggerCollection = nullptr;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        printf("\nCannot get trigger collection: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add the daily trigger to the task.
    ITrigger * pTrigger = nullptr;
    hr = pTriggerCollection->Create(TASK_TRIGGER_DAILY, &pTrigger);
    pTriggerCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create the trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IDailyTrigger * pDailyTrigger = nullptr;
    hr = pTrigger->QueryInterface(IID_IDailyTrigger, (void **)&pDailyTrigger);
    pTrigger->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call on IDailyTrigger failed: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pDailyTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr))
        printf("\nCannot put trigger ID: %x", hr);

    //  Set the task to start daily at a certain time. 
    //  The time format should be YYYY-MM-DDTHH:MM:SS(+-)(timezone).
    //  For example, the start boundary below is January 1st 2005 at 12:05
    hr = pDailyTrigger->put_StartBoundary(_bstr_t(L"2005-01-01T12:05:00"));
    if (FAILED(hr))
        printf("\nCannot put start boundary: %x", hr);

    //  Set the time when the trigger is deactivated.
    hr = pDailyTrigger->put_EndBoundary(_bstr_t(L"2007-05-02T12:05:00"));
    if (FAILED(hr))
        printf("\nCannot put the end boundary: %x", hr);

    //  Define the interval for the daily trigger. An interval of 2 produces an every other day schedule
    hr = pDailyTrigger->put_DaysInterval((short)2);
    if (FAILED(hr)) {
        printf("\nCannot put days interval: %x", hr);
        pRootFolder->Release();
        pDailyTrigger->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    // Add a repetition to the trigger so that it repeats five times.
    IRepetitionPattern * pRepetitionPattern = nullptr;
    hr = pDailyTrigger->get_Repetition(&pRepetitionPattern);
    pDailyTrigger->Release();
    if (FAILED(hr)) {
        printf("\nCannot get repetition pattern: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRepetitionPattern->put_Duration(_bstr_t(L"PT4M"));
    if (FAILED(hr)) {
        printf("\nCannot put repetition duration: %x", hr);
        pRootFolder->Release();
        pRepetitionPattern->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRepetitionPattern->put_Interval(_bstr_t(L"PT1M"));
    pRepetitionPattern->Release();
    if (FAILED(hr)) {
        printf("\nCannot put repetition interval: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add an action to the task. This task will execute notepad.exe.     
    IActionCollection * pActionCollection = nullptr;

    //  Get the task action collection pointer.
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        printf("\nCannot get task collection pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the action, specifying that it is an executable action.
    IAction * pAction = nullptr;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    pActionCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create action: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IExecAction * pExecAction = nullptr;
    hr = pAction->QueryInterface(IID_IExecAction, (void **)&pExecAction);
    pAction->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call failed for IExecAction: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set the path of the executable to notepad.exe.
    hr = pExecAction->put_Path(_bstr_t(wstrExecutablePath.c_str()));
    pExecAction->Release();
    if (FAILED(hr)) {
        printf("\nCannot put the executable path: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Securely get the user name and password. The task will
    //  be created to run with the credentials from the supplied user name and password.
    CREDUI_INFO cui{};
    TCHAR pszName[CREDUI_MAX_USERNAME_LENGTH] = TEXT("");
    TCHAR pszPwd[CREDUI_MAX_PASSWORD_LENGTH] = TEXT("");
    BOOL fSave{};
    DWORD dwErr{};

    cui.cbSize = sizeof(CREDUI_INFO);
    cui.hwndParent = nullptr;
    //  Ensure that MessageText and CaptionText identify
    //  what credentials to use and which application requires them.
    cui.pszMessageText = TEXT("Account information for task registration:");
    cui.pszCaptionText = TEXT("Enter Account Information for Task Registration");
    cui.hbmBanner = nullptr;
    fSave = FALSE;

    //  Create the UI asking for the credentials.
    dwErr = CredUIPromptForCredentials(
        &cui,                             //  CREDUI_INFO structure
        TEXT(""),                         //  Target for credentials
        nullptr,                             //  Reserved
        0,                                //  Reason
        pszName,                          //  User name
        CREDUI_MAX_USERNAME_LENGTH,       //  Max number for user name
        pszPwd,                           //  Password
        CREDUI_MAX_PASSWORD_LENGTH,       //  Max number for password
        &fSave,                           //  State of save check box
        CREDUI_FLAGS_GENERIC_CREDENTIALS |  //  Flags
        CREDUI_FLAGS_ALWAYS_SHOW_UI |
        CREDUI_FLAGS_DO_NOT_PERSIST);
    if (dwErr) {
        cout << "Did not get credentials." << endl;
        CoUninitialize();
        return 1;
    }

    //  Save the task in the root folder.
    IRegisteredTask * pRegisteredTask = nullptr;
    hr = pRootFolder->RegisterTaskDefinition(_bstr_t(wszTaskName),
                                             pTask,
                                             TASK_CREATE_OR_UPDATE,
                                             _variant_t(_bstr_t(pszName)),
                                             _variant_t(_bstr_t(pszPwd)),
                                             TASK_LOGON_PASSWORD,
                                             _variant_t(L""),
                                             &pRegisteredTask);
    if (FAILED(hr)) {
        printf("\nError saving the Task : %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        SecureZeroMemory(pszName, sizeof(pszName));
        SecureZeroMemory(pszPwd, sizeof(pszPwd));
        return 1;
    }

    printf("\n Success! Task successfully registered. ");

    //  Clean up
    pRootFolder->Release();
    pTask->Release();
    pRegisteredTask->Release();
    CoUninitialize();
    SecureZeroMemory(pszName, sizeof(pszName));
    SecureZeroMemory(pszPwd, sizeof(pszPwd));
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


/********************************************************************
 This sample schedules a task to start notepad.exe 30 seconds after
 the task is registered.
********************************************************************/


EXTERN_C
__declspec(dllexport)
int WINAPI StartingExecutableWhenTaskRegistered()
/*
Starting an Executable When a Task is Registered

Registration Trigger Example (C++)
05/31/2018

This C++ example shows how to create a task that is scheduled to execute Notepad when a task is registered.
The task contains a registration trigger that specifies a start boundary and an end boundary for the task and also a delay for the task.
The start boundary specifies when the trigger is activated, and the delay sets the amount of time between when the task is registered and when the task is started.
The task also contains an action that specifies the task to execute Notepad.

 Note
When a task with a registration trigger is updated, the task will execute after the update occurs.

The following procedure describes how to schedule a task to start an executable when the task is registered.

To schedule Notepad to start when a task is registered

Initialize COM and set general COM security.
Create the ITaskService object. This object allows you to create tasks in a specified folder.
Get a task folder to create a task in.
Use the ITaskService::GetFolder method to get the folder, and the ITaskService::NewTask method to create the ITaskDefinition object.
Define information about the task using the ITaskDefinition object, such as the registration information for the task.
Use the RegistrationInfo property of ITaskDefinition and other properties of the ITaskDefinition interface to define the task information.
Create a registration trigger using the Triggers property of ITaskDefinition to access the ITriggerCollection for the task.
Use the ITriggerCollection::Create method (specifying the type of trigger you want to create) to create a registration trigger.
Create an action for the task to execute by using the Actions property of ITaskDefinition to access the IActionCollection interface for the task.
Use the IActionCollection::Create method to specify the type of action that you want to create.
This example uses an IExecAction object, which represents an action that executes a command-line operation.
Register the task using the ITaskFolder::RegisterTaskDefinition method.
The following C++ example shows how to schedule a task to execute Notepad 30 seconds after the task is registered.

https://docs.microsoft.com/en-us/windows/win32/taskschd/registration-trigger-example--c---
*/
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);//  Initialize COM.
    if (FAILED(hr)) {
        printf("\nCoInitializeEx failed: %x", hr);
        return 1;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, 0, nullptr);
    if (FAILED(hr)) {
        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return 1;
    }

    LPCWSTR wszTaskName = L"Registration Trigger Test Task";//  Create a name for the task.

    //  Get the windows directory and set the path to notepad.exe.
    wstring wstrExecutablePath = _wgetenv(L"WINDIR");
    wstrExecutablePath += L"\\SYSTEM32\\NOTEPAD.EXE";

    //  Create an instance of the Task Service. 
    ITaskService * pService = nullptr;
    hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, (void **)&pService);
    if (FAILED(hr)) {
        printf("Failed to create an instance of ITaskService: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        printf("ITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the pointer to the root task folder.  This folder will hold the new task that is registered.
    ITaskFolder * pRootFolder = nullptr;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        printf("Cannot get Root Folder pointer: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);//  If the same task exists, remove it.

    //  Create the task builder object to create the task.
    ITaskDefinition * pTask = nullptr;
    hr = pService->NewTask(0, &pTask);

    pService->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr)) {
        printf("Failed to create a task definition: %x", hr);
        pRootFolder->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the registration info for setting the identification.
    IRegistrationInfo * pRegInfo = nullptr;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        printf("\nCannot get identification pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRegInfo->put_Author((BSTR)L"Author Name");
    pRegInfo->Release();
    if (FAILED(hr)) {
        printf("\nCannot put identification info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the principal for the task
    IPrincipal * pPrincipal = nullptr;
    hr = pTask->get_Principal(&pPrincipal);
    if (FAILED(hr)) {
        printf("\nCannot get principal pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set up principal information: 
    hr = pPrincipal->put_Id(_bstr_t(L"Principal1"));
    if (FAILED(hr))
        printf("\nCannot put the principal ID: %x", hr);

    hr = pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
    if (FAILED(hr))
        printf("\nCannot put principal logon type: %x", hr);

    //  Run the task with the least privileges (LUA) 
    hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_LUA);
    pPrincipal->Release();
    if (FAILED(hr)) {
        printf("\nCannot put principal run level: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the settings for the task
    ITaskSettings * pSettings = nullptr;
    hr = pTask->get_Settings(&pSettings);
    if (FAILED(hr)) {
        printf("\nCannot get settings pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set setting values for the task.
    hr = pSettings->put_StartWhenAvailable(VARIANT_TRUE);
    pSettings->Release();
    if (FAILED(hr)) {
        printf("\nCannot put setting info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the trigger collection to insert the registration trigger.
    ITriggerCollection * pTriggerCollection = nullptr;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        printf("\nCannot get trigger collection: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add the registration trigger to the task.
    ITrigger * pTrigger = nullptr;
    hr = pTriggerCollection->Create(TASK_TRIGGER_REGISTRATION, &pTrigger);
    pTriggerCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create a registration trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IRegistrationTrigger * pRegistrationTrigger = nullptr;
    hr = pTrigger->QueryInterface(IID_IRegistrationTrigger, (void **)&pRegistrationTrigger);
    pTrigger->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call failed on IRegistrationTrigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRegistrationTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr))
        printf("\nCannot put trigger ID: %x", hr);

    //  Define the delay for the registration trigger.
    hr = pRegistrationTrigger->put_Delay((BSTR)L"PT30S");
    pRegistrationTrigger->Release();
    if (FAILED(hr)) {
        printf("\nCannot put registration trigger delay: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add an Action to the task. This task will execute notepad.exe.     
    IActionCollection * pActionCollection = nullptr;

    //  Get the task action collection pointer.
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        printf("\nCannot get Task collection pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the action, specifying that it is an executable action.
    IAction * pAction = nullptr;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    pActionCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create action: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IExecAction * pExecAction = nullptr;

    //  QI for the executable task pointer.
    hr = pAction->QueryInterface(IID_IExecAction, (void **)&pExecAction);
    pAction->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call failed for IExecAction: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set the path of the executable to notepad.exe.
    hr = pExecAction->put_Path(_bstr_t(wstrExecutablePath.c_str()));
    pExecAction->Release();
    if (FAILED(hr)) {
        printf("\nCannot put the action executable path: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Save the task in the root folder.
    IRegisteredTask * pRegisteredTask = nullptr;
    hr = pRootFolder->RegisterTaskDefinition(_bstr_t(wszTaskName),
                                             pTask,
                                             TASK_CREATE_OR_UPDATE,
                                             _variant_t(),
                                             _variant_t(),
                                             TASK_LOGON_INTERACTIVE_TOKEN,
                                             _variant_t(L""),
                                             &pRegisteredTask);
    if (FAILED(hr)) {
        printf("\nError saving the Task : %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    printf("\n Success! Task successfully registered. ");

    //  Clean up.
    pRootFolder->Release();
    pTask->Release();
    pRegisteredTask->Release();
    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


/********************************************************************
 This sample schedules a task to start on a weekly basis.
********************************************************************/


EXTERN_C
__declspec(dllexport)
void WINAPI StartingExecutableWeekly(void)
/*
Starting an Executable Weekly

Weekly Trigger Example (C++)
05/31/2018

This C++ example shows how to create a task that is scheduled to execute Notepad on a weekly basis.
The task contains a weekly trigger that specifies a start boundary, a weeks interval, and a day of the week for the task to start on.
The task also contains an action that specifies the task to execute Notepad.

The following procedure describes how to schedule a task to start an executable on a weekly basis.

To schedule Notepad to start on a weekly basis

Initialize COM and set general COM security.

Create the ITaskService object.

This object allows you to create tasks in a specified folder.

Get a task folder to create a task in.

Use the ITaskService::GetFolder method to get the folder, and the ITaskService::NewTask method to create the ITaskDefinition object.

Define information about the task using the ITaskDefinition object, such as the registration information for the task.

Use the RegistrationInfo property of ITaskDefinition and other properties of the ITaskDefinition interface to define the task information.

Create a weekly trigger using the Triggers property of ITaskDefinition to access the ITriggerCollection interface for the task.

Use the ITriggerCollection::Create method to specify that you want to create a weekly trigger.
You can set the start boundary, the weeks interval,
and the day of the week for the IWeeklyTrigger trigger so that the task's actions will be scheduled to execute at a specified time on a certain day of the week.

Create an action for the task to execute by using the Actions property of ITaskDefinition to access the IActionCollection interface for the task.

Use the IActionCollection::Create method to specify the type of action that you want to create.
This example uses an IExecAction object, which represents an action that executes a command-line operation.

Register the task using the ITaskFolder::RegisterTaskDefinition method.

The following C++ example shows how to schedule a task to execute Notepad on a weekly basis.

https://docs.microsoft.com/en-us/windows/win32/taskschd/weekly-trigger-example--c---
*/
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);//  Initialize COM.
    if (FAILED(hr)) {
        printf("\nCoInitializeEx failed: %x", hr);
        return;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, 0, nullptr);
    if (FAILED(hr)) {
        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return;
    }

    LPCWSTR wszTaskName = L"Weekly Trigger Task";//  Create a name for the task.

    //  Get the windows directory and set the path to notepad.exe.
    wstring wstrExecutablePath = _wgetenv(L"WINDIR");
    wstrExecutablePath += L"\\SYSTEM32\\NOTEPAD.EXE";

    //  Create an instance of the Task Service. 
    ITaskService * pService = nullptr;
    hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, (void **)&pService);
    if (FAILED(hr)) {
        printf("Failed to create an instance of ITaskService: %x", hr);
        CoUninitialize();
        return;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        printf("ITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return;
    }

    //  Get the pointer to the root task folder.  
    //  This folder will hold the new task that is registered.
    ITaskFolder * pRootFolder = nullptr;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        printf("Cannot get Root Folder pointer: %x", hr);
        pService->Release();
        CoUninitialize();
        return;
    }
    
    pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);//  If the same task exists, remove it.

    //  Create the task builder object to create the task.
    ITaskDefinition * pTask = nullptr;
    hr = pService->NewTask(0, &pTask);
    pService->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr)) {
        printf("Failed to create a task definition: %x", hr);
        pRootFolder->Release();
        CoUninitialize();
        return;
    }

    //  Get the registration info for setting the identification.
    IRegistrationInfo * pRegInfo = nullptr;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        printf("\nCannot get identification pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return;
    }

    hr = pRegInfo->put_Author((BSTR)L"Author Name");
    pRegInfo->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr)) {
        printf("\nCannot put identification info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return;
    }

    //  Get the trigger collection to insert the weekly trigger.
    ITriggerCollection * pTriggerCollection = nullptr;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        printf("\nCannot get trigger collection: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return;
    }

    ITrigger * pTrigger = nullptr;
    hr = pTriggerCollection->Create(TASK_TRIGGER_WEEKLY, &pTrigger);
    pTriggerCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create the trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return;
    }

    IWeeklyTrigger * pWeeklyTrigger = nullptr;
    hr = pTrigger->QueryInterface(IID_IWeeklyTrigger, (void **)&pWeeklyTrigger);
    pTrigger->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call for IWeeklyTrigger failed: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return;
    }

    hr = pWeeklyTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr))
        printf("\nCannot put trigger ID: %x", hr);

    //  Set the task to start weekly at a certain time. 
    //  The time format should be YYYY-MM-DDTHH:MM:SS(+-)(timezone).
    //  For example, the start boundary below is January 1st 2005 at 12:05
    hr = pWeeklyTrigger->put_StartBoundary(_bstr_t(L"2005-01-01T12:05:00"));
    if (FAILED(hr))
        printf("\nCannot put the start boundary: %x", hr);

    //  Set the time when the trigger is deactivated.
    hr = pWeeklyTrigger->put_EndBoundary(_bstr_t(L"2007-01-01T12:05:00"));
    if (FAILED(hr))
        printf("\nCannot put the end boundary: %x", hr);

    //  Define the interval for the weekly trigger. 
    //  An interval of 2 produces an every other week schedule
    hr = pWeeklyTrigger->put_WeeksInterval((short)2);
    if (FAILED(hr)) {
        printf("\nCannot put weeks interval: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return;
    }

    hr = pWeeklyTrigger->put_DaysOfWeek((short)2);    // Runs on Monday
    pWeeklyTrigger->Release();
    if (FAILED(hr)) {
        printf("\nCannot put days of week interval: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return;
    }

    //  Add an Action to the task. This task will execute notepad.exe.     
    IActionCollection * pActionCollection = nullptr;

    //  Get the task action collection pointer.
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        printf("\nCannot get Task collection pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return;
    }

    //  Create the action, specifying that it is an executable action.
    IAction * pAction = nullptr;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    pActionCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create the action: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return;
    }

    IExecAction * pExecAction = nullptr;
    //  QI for the executable task pointer.
    hr = pAction->QueryInterface(IID_IExecAction, (void **)&pExecAction);
    pAction->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call failed on IExecAction: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return;
    }

    //  Set the path of the executable to notepad.exe.
    hr = pExecAction->put_Path(_bstr_t(wstrExecutablePath.c_str()));
    pExecAction->Release();
    if (FAILED(hr)) {
        printf("\nCannot add path for executable action: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return;
    }

    //  Securely get the user name and password.
    //  The task will be created to run with the credentials from the supplied user name and password.
    CREDUI_INFO cui{};
    TCHAR pszName[CREDUI_MAX_USERNAME_LENGTH] = L"";
    TCHAR pszPwd[CREDUI_MAX_PASSWORD_LENGTH] = L"";
    BOOL fSave{};
    DWORD dwErr{};

    cui.cbSize = sizeof(CREDUI_INFO);
    cui.hwndParent = nullptr;
    //  Ensure that MessageText and CaptionText identify
    //  what credentials to use and which application requires them.
    cui.pszMessageText = TEXT("Account information for task registration:");
    cui.pszCaptionText = TEXT("Enter Account Information for Task Registration");
    cui.hbmBanner = nullptr;
    fSave = FALSE;

    //  Create the UI asking for the credentials.
    dwErr = CredUIPromptForCredentials(
        &cui,                             //  CREDUI_INFO structure
        TEXT(""),                         //  Target for credentials
        nullptr,                             //  Reserved
        0,                                //  Reason
        pszName,                          //  User name
        CREDUI_MAX_USERNAME_LENGTH,       //  Max number for user name
        pszPwd,                           //  Password
        CREDUI_MAX_PASSWORD_LENGTH,       //  Max number for password
        &fSave,                           //  State of save check box
        CREDUI_FLAGS_GENERIC_CREDENTIALS |  //  Flags
        CREDUI_FLAGS_ALWAYS_SHOW_UI |
        CREDUI_FLAGS_DO_NOT_PERSIST);
    if (dwErr) {
        cout << "Did not get credentials." << endl;
        CoUninitialize();
        return;
    }

    //  Save the task in the root folder.
    IRegisteredTask * pRegisteredTask = nullptr;
    hr = pRootFolder->RegisterTaskDefinition(_bstr_t(wszTaskName),
                                             pTask,
                                             TASK_CREATE_OR_UPDATE,
                                             _variant_t(_bstr_t(pszName)),
                                             _variant_t(_bstr_t(pszPwd)),
                                             TASK_LOGON_PASSWORD,
                                             _variant_t(L""),
                                             &pRegisteredTask);
    if (FAILED(hr)) {
        printf("\nError saving the Task : %x", hr);
        pRootFolder->Release();
        pTask->Release();
        SecureZeroMemory(pszName, sizeof(pszName));
        SecureZeroMemory(pszPwd, sizeof(pszPwd));
        CoUninitialize();
        return;
    }

    printf("\n Success! Task succesfully registered. ");

    //  Clean up
    pRootFolder->Release();
    pTask->Release();
    pRegisteredTask->Release();
    SecureZeroMemory(pszName, sizeof(pszName));
    SecureZeroMemory(pszPwd, sizeof(pszPwd));
    CoUninitialize();
    return;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


/**********************************************************************
 This sample schedules a task to start notepad.exe when a user logs on.
**********************************************************************/


EXTERN_C
__declspec(dllexport)
int WINAPI StartingExecutableWhenUserLogsOn()
/*
Starting an Executable When a User Logs On

Logon Trigger Example (C++)
05/31/2018

This C++ example shows how to create a task that is scheduled to execute Notepad when a user logs on.
The task contains a logon trigger that specifies a start boundary for the task to start and a user identifier that specifies the user.
The task is registered using the Administrators group as a security context to run the task.

The following procedure describes how to schedule a task to start an executable when a user logs on.

To schedule Notepad to start when a user logs on

Initialize COM and set general COM security.

Create the ITaskService object.

This object allows you to create tasks in a specified folder.

Get a task folder to create a task in.

Use the ITaskService::GetFolder method to get the folder, and the ITaskService::NewTask method to create the ITaskDefinition object.

Define information about the task using the ITaskDefinition object, such as the registration information for the task.

Use the RegistrationInfo property of ITaskDefinition and other properties of the ITaskDefinition interface to define the task information.

Create a logon trigger using the Triggers property of ITaskDefinition to access the ITriggerCollection interface for the task.

Use the ITriggerCollection::Create method to specify that you want to create a logon trigger.
You can set the start boundary and the UserId property for the trigger so that the task's actions will be scheduled to execute when the user logs on after the start boundary.

Create an action for the task to execute by using the Actions property of ITaskDefinition to access the IActionCollection interface for the task.
Use the IActionCollection::Create method to specify the type of action that you want to create.
This example uses an IExecAction object, which represents an action that executes a command-line operation.

Register the task using the ITaskFolder::RegisterTaskDefinition method.

The following C++ example shows how to schedule a task to execute Notepad when a user logs on.

https://docs.microsoft.com/en-us/windows/win32/taskschd/logon-trigger-example--c---
*/
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);//  Initialize COM.
    if (FAILED(hr)) {
        printf("\nCoInitializeEx failed: %x", hr);
        return 1;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, 0, nullptr);
    if (FAILED(hr)) {
        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return 1;
    }

    LPCWSTR wszTaskName = L"Logon Trigger Test Task";//  Create a name for the task.

    //  Get the windows directory and set the path to notepad.exe.
    wstring wstrExecutablePath = _wgetenv(L"WINDIR");
    wstrExecutablePath += L"\\SYSTEM32\\NOTEPAD.EXE";

    //  Create an instance of the Task Service. 
    ITaskService * pService = nullptr;
    hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, (void **)&pService);
    if (FAILED(hr)) {
        printf("Failed to create an instance of ITaskService: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        printf("ITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the pointer to the root task folder.  This folder will hold the
    //  new task that is registered.
    ITaskFolder * pRootFolder = nullptr;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        printf("Cannot get Root Folder pointer: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  If the same task exists, remove it.
    pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);

    //  Create the task builder object to create the task.
    ITaskDefinition * pTask = nullptr;
    hr = pService->NewTask(0, &pTask);

    pService->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr)) {
        printf("Failed to create a task definition: %x", hr);
        pRootFolder->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the registration info for setting the identification.
    IRegistrationInfo * pRegInfo = nullptr;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        printf("\nCannot get identification pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRegInfo->put_Author((BSTR)L"Author Name");
    pRegInfo->Release();
    if (FAILED(hr)) {
        printf("\nCannot put identification info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the settings for the task
    ITaskSettings * pSettings = nullptr;
    hr = pTask->get_Settings(&pSettings);
    if (FAILED(hr)) {
        printf("\nCannot get settings pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set setting values for the task. 
    hr = pSettings->put_StartWhenAvailable(VARIANT_TRUE);
    pSettings->Release();
    if (FAILED(hr)) {
        printf("\nCannot put setting info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the trigger collection to insert the logon trigger.
    ITriggerCollection * pTriggerCollection = nullptr;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        printf("\nCannot get trigger collection: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add the logon trigger to the task.
    ITrigger * pTrigger = nullptr;
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    pTriggerCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create the trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    ILogonTrigger * pLogonTrigger = nullptr;
    hr = pTrigger->QueryInterface(IID_ILogonTrigger, (void **)&pLogonTrigger);
    pTrigger->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call failed for ILogonTrigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pLogonTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr))
        printf("\nCannot put the trigger ID: %x", hr);

    //  Set the task to start at a certain time. The time 
    //  format should be YYYY-MM-DDTHH:MM:SS(+-)(timezone).
    //  For example, the start boundary below is January 1st 2005 at 12:05
    hr = pLogonTrigger->put_StartBoundary(_bstr_t(L"2005-01-01T12:05:00"));
    if (FAILED(hr))
        printf("\nCannot put the start boundary: %x", hr);

    hr = pLogonTrigger->put_EndBoundary(_bstr_t(L"2015-05-02T08:00:00"));
    if (FAILED(hr))
        printf("\nCannot put the end boundary: %x", hr);

    //  Define the user.  The task will execute when the user logs on.
    //  The specified user must be a user on this computer.  
    hr = pLogonTrigger->put_UserId(_bstr_t(L"DOMAIN\\UserName"));
    pLogonTrigger->Release();
    if (FAILED(hr)) {
        printf("\nCannot add user ID to logon trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add an Action to the task. This task will execute notepad.exe.     
    IActionCollection * pActionCollection = nullptr;

    //  Get the task action collection pointer.
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        printf("\nCannot get Task collection pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the action, specifying that it is an executable action.
    IAction * pAction = nullptr;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    pActionCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create the action: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IExecAction * pExecAction = nullptr;
    //  QI for the executable task pointer.
    hr = pAction->QueryInterface(IID_IExecAction, (void **)&pExecAction);
    pAction->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call failed for IExecAction: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set the path of the executable to notepad.exe.
    hr = pExecAction->put_Path(_bstr_t(wstrExecutablePath.c_str()));
    pExecAction->Release();
    if (FAILED(hr)) {
        printf("\nCannot set path of executable: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Save the task in the root folder.
    IRegisteredTask * pRegisteredTask = nullptr;

    hr = pRootFolder->RegisterTaskDefinition(_bstr_t(wszTaskName),
                                             pTask,
                                             TASK_CREATE_OR_UPDATE,
                                             _variant_t(L"S-1-5-32-544"),
                                             _variant_t(),
                                             TASK_LOGON_GROUP,
                                             _variant_t(L""),
                                             &pRegisteredTask);
    if (FAILED(hr)) {
        printf("\nError saving the Task : %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    printf("\n Success! Task successfully registered. ");

    // Clean up
    pRootFolder->Release();
    pTask->Release();
    pRegisteredTask->Release();
    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


/********************************************************************
 This sample schedules a task to start Notepad.exe 30 seconds after
 the system is started.
********************************************************************/


EXTERN_C
__declspec(dllexport)
int WINAPI StartingExecutableOnSystemBoot()
/*
Starting an Executable on System Boot

Boot Trigger Example (C++)
05/31/2018
5 minutes to read

This topic contains a C++ code example that shows how to create a task that is scheduled to execute Notepad.exe when the system is started.
The task contains a boot trigger that specifies a start boundary and delay time for the task to start after the system is started.
The task also contains an action that specifies the task execute Notepad.exe.
The task is registered using the Local Service account as a security context to run the task.

The following procedure describes how to schedule a task to start an executable when the system is started.

To schedule Notepad to start when the system is started

Initialize COM and set general COM security.

Create the ITaskService object.

This object enables you to create tasks in a specified folder.

Get a task folder to create a task in.

Use the ITaskService::GetFolder method to get the folder, and the ITaskService::NewTask method to create the ITaskDefinition object.

Define information about the task using the ITaskDefinition object, such as the registration information for the task.

Use the RegistrationInfo property of ITaskDefinition and other properties of the ITaskDefinition interface to define the task information.

Create a boot trigger using the Triggers property of ITaskDefinition to access the ITriggerCollection for the task.

Use the ITriggerCollection::Create method to specify that you want to create a boot trigger.
You can set the start boundary and delay for the trigger so that the task actions will be scheduled to execute at a specified time when the system is started.

Create an action for the task to execute by using the Actions property of ITaskDefinition to access the IActionCollection collection for the task.

Use the IActionCollection::Create method to specify the type of action you want to create.
This example uses an IExecAction object, which represents an action that executes a command-line operation.

Register the task using the ITaskFolder::RegisterTaskDefinition method.

The following C++ code example shows how to schedule a task to execute Notepad.exe 30 seconds after the system is started.

https://docs.microsoft.com/en-us/windows/win32/taskschd/boot-trigger-example--c---
*/
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);//  Initialize COM.
    if (FAILED(hr)) {
        printf("\nCoInitializeEx failed: %x", hr);
        return 1;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, 0, nullptr);
    if (FAILED(hr)) {
        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return 1;
    }

    LPCWSTR wszTaskName = L"Boot Trigger Test Task";//  Create a name for the task.

    //  Get the Windows directory and set the path to Notepad.exe.
    wstring wstrExecutablePath = _wgetenv(L"WINDIR");
    wstrExecutablePath += L"\\SYSTEM32\\NOTEPAD.EXE";

    //  Create an instance of the Task Service. 
    ITaskService * pService = nullptr;
    hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, (void **)&pService);
    if (FAILED(hr)) {
        printf("Failed to create an instance of ITaskService: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        printf("ITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the pointer to the root task folder.  
    //  This folder will hold the new task that is registered.
    ITaskFolder * pRootFolder = nullptr;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        printf("Cannot get Root Folder pointer: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  If the same task exists, remove it.
    pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);

    //  Create the task builder object to create the task.
    ITaskDefinition * pTask = nullptr;
    hr = pService->NewTask(0, &pTask);

    pService->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr)) {
        printf("Failed to create a task definition: %x", hr);
        pRootFolder->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the registration info for setting the identification.
    IRegistrationInfo * pRegInfo = nullptr;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        printf("\nCannot get identification pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRegInfo->put_Author((BSTR)L"Author Name");
    pRegInfo->Release();
    if (FAILED(hr)) {
        printf("\nCannot put identification info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the settings for the task
    ITaskSettings * pSettings = nullptr;
    hr = pTask->get_Settings(&pSettings);
    if (FAILED(hr)) {
        printf("\nCannot get settings pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set setting values for the task. 
    hr = pSettings->put_StartWhenAvailable(VARIANT_TRUE);
    pSettings->Release();
    if (FAILED(hr)) {
        printf("\nCannot put setting info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the trigger collection to insert the boot trigger.
    ITriggerCollection * pTriggerCollection = nullptr;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        printf("\nCannot get trigger collection: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add the boot trigger to the task.
    ITrigger * pTrigger = nullptr;
    hr = pTriggerCollection->Create(TASK_TRIGGER_BOOT, &pTrigger);
    pTriggerCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create the trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IBootTrigger * pBootTrigger = nullptr;
    hr = pTrigger->QueryInterface(IID_IBootTrigger, (void **)&pBootTrigger);
    pTrigger->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call failed for IBootTrigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pBootTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr))
        printf("\nCannot put the trigger ID: %x", hr);

    //  Set the task to start at a certain time. The time 
    //  format should be YYYY-MM-DDTHH:MM:SS(+-)(timezone).
    //  For example, the start boundary below is January 1st 2005 at 12:05
    hr = pBootTrigger->put_StartBoundary(_bstr_t(L"2005-01-01T12:05:00"));
    if (FAILED(hr))
        printf("\nCannot put the start boundary: %x", hr);

    hr = pBootTrigger->put_EndBoundary(_bstr_t(L"2015-05-02T08:00:00"));
    if (FAILED(hr))
        printf("\nCannot put the end boundary: %x", hr);

    // Delay the task to start 30 seconds after system start. 
    hr = pBootTrigger->put_Delay((BSTR)L"PT30S");
    pBootTrigger->Release();
    if (FAILED(hr)) {
        printf("\nCannot put delay for boot trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add an Action to the task. This task will execute Notepad.exe.     
    IActionCollection * pActionCollection = nullptr;

    //  Get the task action collection pointer.
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        printf("\nCannot get Task collection pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the action, specifying it as an executable action.
    IAction * pAction = nullptr;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    pActionCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create the action: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IExecAction * pExecAction = nullptr;
    //  QI for the executable task pointer.
    hr = pAction->QueryInterface(IID_IExecAction, (void **)&pExecAction);
    pAction->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call failed for IExecAction: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set the path of the executable to Notepad.exe.
    hr = pExecAction->put_Path(_bstr_t(wstrExecutablePath.c_str()));
    pExecAction->Release();
    if (FAILED(hr)) {
        printf("\nCannot set path of executable: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Save the task in the root folder.
    IRegisteredTask * pRegisteredTask = nullptr;
    VARIANT varPassword;
    varPassword.vt = VT_EMPTY;
    hr = pRootFolder->RegisterTaskDefinition(_bstr_t(wszTaskName),
                                             pTask,
                                             TASK_CREATE_OR_UPDATE,
                                             _variant_t(L"Local Service"),
                                             varPassword,
                                             TASK_LOGON_SERVICE_ACCOUNT,
                                             _variant_t(L""),
                                             &pRegisteredTask);
    if (FAILED(hr)) {
        printf("\nError saving the Task : %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    printf("\n Success! Task successfully registered. ");

    //  Clean up.
    pRootFolder->Release();
    pTask->Release();
    pRegisteredTask->Release();
    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


/********************************************************************
 This sample schedules a task to send an email when an event occurs.
********************************************************************/


//#define _WIN32_DCOM
//
//#include <windows.h>
//#include <iostream>
//#include <stdio.h>
//#include <comdef.h>
//#include <wincred.h>
////  Include the task header file.
//#include <taskschd.h>
//# pragma comment(lib, \"taskschd.lib\")
//# pragma comment(lib, \"comsupp.lib\")
//# pragma comment(lib, \"credui.lib\")

//using namespace std;


EXTERN_C
__declspec(dllexport)
int WINAPI EventTriggerExample()
/*
Event Trigger Example (C++)
Article
10/17/2012
7 minutes to read
This C++ example shows how to create a task that is scheduled to send an email when an event occurs. 
The task contains an event trigger that specifies an event query that subscribes to an event from the Windows Event Log service. 
When the event occurs, the task is triggered. The task also contains an action that specifies the task to send an email message. 
The task is registered using a password and user name as the security context to run the task.

The following procedure describes how to schedule a task to send an email when an event occurs.

Aa446886.wedge(en-us,VS.85).gifTo schedule an E-mail to be sent when an event occurs

Initialize COM and set general COM security.

Create the ITaskService object.

This object allows you to create tasks in a specified folder.

Get a task folder to create a task in.

Use the ITaskService::GetFolder method to get the folder, and the ITaskService::NewTask method to create the ITaskDefinition object.

Define information about the task using the ITaskDefinition object, such as the registration information for the task.

Use the RegistrationInfo property of ITaskDefinition and other properties of the ITaskDefinition interface to define the task information.

Create an event trigger using the Triggers property of ITaskDefinition to access the ITriggerCollection for the task.

Use the ITriggerCollection::Create method (specifying the type of trigger you want to create) to create an event trigger. 
This allows you to set the event query to subscribe to events. For information about how to create an event query, see Event Selection.

Create an action for the task to execute by using the Actions property of ITaskDefinition to access the IActionCollection interface for the task.

Use the IActionCollection::Create method to specify the type of action that you want to create. 
This example uses an IEmailAction object, which represents an action that sends a specified email message.

Register the task using the ITaskFolder::RegisterTaskDefinition method.

The following C++ example shows how to schedule a task to send an email when an event occurs.

https://docs.microsoft.com/en-us/previous-versions/aa446886(v=vs.85)
*/
{
    //  ------------------------------------------------------
    //  Initialize COM.
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        printf("\nCoInitializeEx failed: %x", hr);
        return 1;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(
        nullptr,
        -1,
        nullptr,
        nullptr,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        0,
        nullptr);

    if (FAILED(hr)) {
        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Create a name for the task.
    LPCWSTR wszTaskName = L"Event Trigger Test Task";


    //  ------------------------------------------------------
    //  Create an instance of the Task Service. 
    ITaskService * pService = nullptr;
    hr = CoCreateInstance(CLSID_TaskScheduler,
                          nullptr,
                          CLSCTX_INPROC_SERVER,
                          IID_ITaskService,
                          (void **)&pService);
    if (FAILED(hr)) {
        printf("Failed to CoCreate an instance of the TaskService class: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(),
                           _variant_t(), _variant_t());
    if (FAILED(hr)) {
        printf("ITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Get the pointer to the root task folder.  This folder will hold the
    //  new task that is registered.
    ITaskFolder * pRootFolder = nullptr;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        printf("Cannot get Root Folder pointer: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    // If the same task exists, remove it.
    pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);

    //  Create the task builder object to create the task.
    ITaskDefinition * pTask = nullptr;
    hr = pService->NewTask(0, &pTask);

    pService->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr)) {
        printf("Failed to create an instance of the task: %x", hr);
        pRootFolder->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Get the registration info for setting the identification.
    IRegistrationInfo * pRegInfo = nullptr;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        printf("\nCannot get identification pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRegInfo->put_Author(_bstr_t(L"Author Name"));
    pRegInfo->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr)) {
        printf("\nCannot put identification info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Create the settings for the task
    ITaskSettings * pSettings = nullptr;
    hr = pTask->get_Settings(&pSettings);
    if (FAILED(hr)) {
        printf("\nCannot get settings pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set setting values for the task.  
    hr = pSettings->put_StartWhenAvailable(VARIANT_TRUE);
    pSettings->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr)) {
        printf("\nCannot put setting info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Get the trigger collection to insert the event trigger.
    ITriggerCollection * pTriggerCollection = nullptr;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        printf("\nCannot get trigger collection: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the event trigger for the task.
    ITrigger * pTrigger = nullptr;

    hr = pTriggerCollection->Create(TASK_TRIGGER_EVENT, &pTrigger);
    pTriggerCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create the trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IEventTrigger * pEventTrigger = nullptr;
    hr = pTrigger->QueryInterface(
        IID_IEventTrigger, (void **)&pEventTrigger);
    pTrigger->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call on IEventTrigger failed: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pEventTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr))
        printf("\nCannot put the trigger ID: %x", hr);

    //  Set the task to start at a certain time. The time 
    //  format should be YYYY-MM-DDTHH:MM:SS(+-)(timezone).
    //  For example, the start boundary below
    //  is January 1st 2005 at 12:05
    hr = pEventTrigger->put_StartBoundary(_bstr_t(L"2005-01-01T12:05:00"));
    if (FAILED(hr))
        printf("\nCannot put the trigger start boundary: %x", hr);

    hr = pEventTrigger->put_EndBoundary(_bstr_t(L"2015-05-02T08:00:00"));
    if (FAILED(hr))
        printf("\nCannot put the trigger end boundary: %x", hr);

    //  Define the delay for the event trigger (30 seconds).
    hr = pEventTrigger->put_Delay(_bstr_t(L"PT30S"));
    if (FAILED(hr))
        printf("\nCannot put the trigger delay: %x", hr);

    //  Define the event query for the event trigger.
    //  The following query string defines a subscription to all
    //  level 2 events in the System channel.
    hr = pEventTrigger->put_Subscription(_bstr_t(L"<QueryList> <Query Id='1'> <Select Path='System'>*[System/Level=2]</Select></Query></QueryList>"));
    pEventTrigger->Release();
    if (FAILED(hr)) {
        printf("\nCannot put the event query: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Add an action to the task. This task will send an email message.     
    IActionCollection * pActionCollection = nullptr;

    //  Get the task action collection pointer.
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        printf("\nCannot get action collection pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the action, specifying that it will send an email message.
    IAction * pAction = nullptr;
    hr = pActionCollection->Create(TASK_ACTION_SEND_EMAIL, &pAction);
    pActionCollection->Release();
    if (FAILED(hr)) {
        printf("\nCannot create an email action: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IEmailAction * pEmailAction = nullptr;
    //  QI for the email task pointer.
    hr = pAction->QueryInterface(IID_IEmailAction, (void **)&pEmailAction);
    pAction->Release();
    if (FAILED(hr)) {
        printf("\nQueryInterface call failed for IEmailAction: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set the properties of the email action.
    hr = pEmailAction->put_From(_bstr_t(L"SendersEmailAddress@domain.com"));
    if (FAILED(hr)) {
        printf("\nCannot put From information: %x", hr);
        pRootFolder->Release();
        pEmailAction->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pEmailAction->put_To(_bstr_t(L"RecipientsEmailAddress@domain.com"));
    if (FAILED(hr)) {
        printf("\nCannot put To information: %x", hr);
        pRootFolder->Release();
        pEmailAction->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pEmailAction->put_Server(_bstr_t(L"MyEmailServerName.domain.com"));
    if (FAILED(hr)) {
        printf("\nCannot put SMTP server information: %x", hr);
        pRootFolder->Release();
        pEmailAction->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pEmailAction->put_Subject(_bstr_t(L"An event has occurred"));
    if (FAILED(hr))
        printf("\nCannot put the subject information: %x", hr);

    hr = pEmailAction->put_Body(_bstr_t(L"A level 2 event occurred in the system channel."));
    if (FAILED(hr))
        printf("\nCannot put the email body information: %x", hr);

    pEmailAction->Release();

    //  ------------------------------------------------------
    //  Securely get the user name and password. The task will
    //  be created to run with the credentials from the supplied 
    //  user name and password.
    CREDUI_INFO cui{};
    TCHAR pszName[CREDUI_MAX_USERNAME_LENGTH] = L"";
    TCHAR pszPwd[CREDUI_MAX_PASSWORD_LENGTH] = L"";
    BOOL fSave{};
    DWORD dwErr{};

    cui.cbSize = sizeof(CREDUI_INFO);
    cui.hwndParent = nullptr;
    //  Ensure that MessageText and CaptionText identify
    //  what credentials to use and which application requires them.
    cui.pszMessageText = TEXT("Account information for task registration:");
    cui.pszCaptionText = TEXT("Enter Account Information for Task Registration");
    cui.hbmBanner = nullptr;
    fSave = FALSE;

    //  Create the UI asking for the credentials.
    dwErr = CredUIPromptForCredentials(
        &cui,                             //  CREDUI_INFO structure
        TEXT(""),                         //  Target for credentials
        nullptr,                             //  Reserved
        0,                                //  Reason
        pszName,                          //  User name
        CREDUI_MAX_USERNAME_LENGTH,       //  Max number for user name
        pszPwd,                           //  Password
        CREDUI_MAX_PASSWORD_LENGTH,       //  Max number for password
        &fSave,                           //  State of save check box
        CREDUI_FLAGS_GENERIC_CREDENTIALS |  //  Flags
        CREDUI_FLAGS_ALWAYS_SHOW_UI |
        CREDUI_FLAGS_DO_NOT_PERSIST);

    if (dwErr) {
        cout << "Did not get credentials." << endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Save the task in the root folder.
    IRegisteredTask * pRegisteredTask = nullptr;
    hr = pRootFolder->RegisterTaskDefinition(
        _bstr_t(wszTaskName),
        pTask,
        TASK_CREATE_OR_UPDATE,
        _variant_t(_bstr_t(pszName)),
        _variant_t(_bstr_t(pszPwd)),
        TASK_LOGON_PASSWORD,
        _variant_t(L""),
        &pRegisteredTask);
    if (FAILED(hr)) {
        printf("\nError saving the Task : %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        SecureZeroMemory(pszName, sizeof(pszName));
        SecureZeroMemory(pszPwd, sizeof(pszPwd));
        return 1;
    }

    printf("\n Success! Task successfully registered. ");

    //  Clean up.
    pRootFolder->Release();
    pTask->Release();
    pRegisteredTask->Release();
    CoUninitialize();

    // When you have finished using the credentials,
    // erase them from memory.
    SecureZeroMemory(pszName, sizeof(pszName));
    SecureZeroMemory(pszPwd, sizeof(pszPwd));

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


/********************************************************************
 This sample enumerates through the tasks on the local computer and
 displays their name and state.
********************************************************************/


EXTERN_C
__declspec(dllexport)
int WINAPI EnumeratingTasks()
/*
Enumerating Tasks and Displaying Task Information

Displaying Task Names and States (C++)
05/31/2018

These two C++ examples show how to enumerate tasks.
One example shows how to display information for tasks in a task folder,
and the other examples shows how to display information for all running tasks.

The following procedure describes how to display task names and state for all the tasks in a task folder.

To display task names and state for all the tasks in a task folder

Initialize COM and set general COM security.

Create the ITaskService object.

This object allows you to connect to the Task Scheduler service and access a specific task folder.

Get a task folder that holds the tasks you want information about.

Use the ITaskService::GetFolder method to get the folder.

Get the collection of tasks from the folder.

Use the ITaskFolder::GetTasks method to get the collection of tasks (IRegisteredTaskCollection).

Get the number of tasks in the collection, and enumerate through each task in the collection.

Use the Item Property of IRegisteredTaskCollection to get an IRegisteredTask instance.
Each instance will contain a task in the collection.
You can then display the information (property values) from each registered task.

The following C++ example shows how to display the name and state of all the tasks in the root task folder.

https://docs.microsoft.com/en-us/windows/win32/taskschd/displaying-task-names-and-state--c---
*/
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);//  Initialize COM.
    if (FAILED(hr)) {
        printf("\nCoInitializeEx failed: %x", hr);
        return 1;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, 0, nullptr);
    if (FAILED(hr)) {
        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Create an instance of the Task Service. 
    ITaskService * pService = nullptr;
    hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, (void **)&pService);
    if (FAILED(hr)) {
        printf("Failed to CoCreate an instance of the TaskService class: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        printf("ITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  Get the pointer to the root task folder.
    ITaskFolder * pRootFolder = nullptr;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    pService->Release();
    if (FAILED(hr)) {
        printf("Cannot get Root Folder pointer: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Get the registered tasks in the folder.
    IRegisteredTaskCollection * pTaskCollection = nullptr;
    hr = pRootFolder->GetTasks(0, &pTaskCollection);
    pRootFolder->Release();
    if (FAILED(hr)) {
        printf("Cannot get the registered tasks.: %x", hr);
        CoUninitialize();
        return 1;
    }

    LONG numTasks = 0;
    hr = pTaskCollection->get_Count(&numTasks);
    if (numTasks == 0) {
        printf("\nNo Tasks are currently running");
        pTaskCollection->Release();
        CoUninitialize();
        return 1;
    }

    printf("\nNumber of Tasks : %d", numTasks);

    TASK_STATE taskState{};

    for (LONG i = 0; i < numTasks; i++) {
        IRegisteredTask * pRegisteredTask = nullptr;
        hr = pTaskCollection->get_Item(_variant_t(i + 1), &pRegisteredTask);
        if (SUCCEEDED(hr)) {
            BSTR taskName = nullptr;
            hr = pRegisteredTask->get_Name(&taskName);
            if (SUCCEEDED(hr)) {
                printf("\nTask Name: %S", taskName);
                SysFreeString(taskName);

                hr = pRegisteredTask->get_State(&taskState);
                if (SUCCEEDED(hr))
                    printf("\n\tState: %d", taskState);
                else
                    printf("\n\tCannot get the registered task state: %x", hr);
            } else {
                printf("\nCannot get the registered task name: %x", hr);
            }
            pRegisteredTask->Release();
        } else {
            printf("\nCannot get the registered task item at index=%d: %x", i + 1, hr);
        }
    }

    pTaskCollection->Release();
    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


//�����ǣ�
//Task Scheduler 1.0 Examples


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI CreatingTaskUsingNewWorkItem(int argc, char ** argv)
/*
C/C++ Code Example: Creating a Task Using NewWorkItem
05/31/2018

This example creates a single task.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-creating-a-task-using-newworkitem
*/
{
    HRESULT hr = S_OK;
    ITaskScheduler * pITS{};

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object. 
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::NewWorkItem to create new task.
    LPCWSTR pwszTaskName{};
    ITask * pITask{};
    IPersistFile * pIPersistFile{};
    pwszTaskName = L"Test Task";

    hr = pITS->NewWorkItem(pwszTaskName,         // Name of task
                           CLSID_CTask,          // Class identifier 
                           IID_ITask,            // Interface identifier
                           (IUnknown **)&pITask); // Address of task interface
    pITS->Release();                               // Release object
    if (FAILED(hr)) {
        CoUninitialize();
        fprintf(stderr, "Failed calling NewWorkItem, error = 0x%x\n", hr);
        return 1;
    }

    // Call IUnknown::QueryInterface to get a pointer to 
    // IPersistFile and IPersistFile::Save to save the new task to disk.

    hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    pITask->Release();
    if (FAILED(hr)) {
        CoUninitialize();
        fprintf(stderr, "Failed calling QueryInterface, error = 0x%x\n", hr);
        return 1;
    }

    hr = pIPersistFile->Save(nullptr, TRUE);
    pIPersistFile->Release();
    if (FAILED(hr)) {
        CoUninitialize();
        fprintf(stderr, "Failed calling Save, error = 0x%x\n", hr);
        return 1;
    }

    CoUninitialize();
    printf("Created task.\n");
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI EnumeratingTasksOne(int argc, char ** argv)
/*
C/C++ Code Example: Enumerating Tasks
05/31/2018

This example enumerates all the tasks in the Scheduled Tasks folder of the local computer and prints the name of each task on the screen.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-enumerating-tasks
*/
{
    HRESULT hr = S_OK;
    ITaskScheduler * pITS{};

    // Call CoInitialize to initialize the COM library and 
    // then call CoCreateInstance to get the Task Scheduler object. 
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return hr;
        }
    } else {
        return hr;
    }

    // Call ITaskScheduler::Enum to get an enumeration object.
    IEnumWorkItems * pIEnum;
    hr = pITS->Enum(&pIEnum);
    pITS->Release();
    if (FAILED(hr)) {
        CoUninitialize();
        return hr;
    }

    // Call IEnumWorkItems::Next to retrieve tasks. Note that 
    // this example tries to retrieve five tasks for each call.
    LPWSTR * lpwszNames{};
    DWORD dwFetchedTasks = 0;
    while (SUCCEEDED(pIEnum->Next(TASKS_TO_RETRIEVE, &lpwszNames, &dwFetchedTasks)) && (dwFetchedTasks != 0)) {
        // Process each task. Note that this example prints the name of each task to the screen.
        while (dwFetchedTasks) {
            wprintf(L"%s\n", lpwszNames[--dwFetchedTasks]);
            CoTaskMemFree(lpwszNames[dwFetchedTasks]);
        }
        CoTaskMemFree(lpwszNames);
    }

    pIEnum->Release();
    CoUninitialize();
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI StartingTask(int argc, char ** argv)
/*
C/C++ Code Example: Starting a Task
05/31/2018

This example attempts to run an existing task.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-starting-a-task
*/
{
    HRESULT hr = S_OK;
    ITaskScheduler * pITS{};

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName{};
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate; error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::Run to start the execution of "Test Task".
    hr = pITask->Run();
    pITask->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::Run, error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI EditingWorkItem(int argc, char ** argv)
/*
C/C++ Code Example: Editing a Work Item
05/31/2018

This example displays the property pages for a known task and allows a user to edit the properties of the work item.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-editing-a-work-item
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.      
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName{};
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();//Release ITaskScheduler interface
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate, ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::EditWorkItem. Note that this method is inherited from IScheduledWorkItem.
    HWND hParent = nullptr;
    DWORD dwReserved = 0;
    hr = pITask->EditWorkItem(hParent, dwReserved);
    pITask->Release();// Release ITask interface
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::EditWorkItem, ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskAccountInformation(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving Task Account Information
05/31/2018

This code example retrieves the account information of a known task and displays the account name on the screen.
This example assumes that the task and the test task already exist on the local computer and that the Task Scheduler is running.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-task-account-information
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName{};
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();//Release ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetAccountInformation. Note that this method is inherited from IScheduledWorkItem.
    LPWSTR pwszAccountName;
    hr = pITask->GetAccountInformation(&pwszAccountName);
    pITask->Release();// Release the ITask interface.
    if (hr == SCHED_E_NO_SECURITY_SERVICES) {
        wprintf(L"Error: SCHED_E_NO_SECURITY_SERVICES");
        wprintf(L"Security services are available only on Windows Server 2003");
        wprintf(L", Windows XP, and Windows 2000.");
        CoUninitialize();
        return 1;
    }

    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetAccountInformation: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    wprintf(L"The account name for Test Task is: ");
    wprintf(L"  %s\n", pwszAccountName);

    CoTaskMemFree(pwszAccountName);
    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskComment(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving a Task Comment
05/31/2018

This example retrieves the comment string of a known task and displays the comment string on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-a-task-comment
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName{};
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();//Release ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetComment. Note that this method is inherited from IScheduledWorkItem.
    LPWSTR ppwszComment;
    hr = pITask->GetComment(&ppwszComment);
    pITask->Release();// Release the ITask interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetComment: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    wprintf(L"The comment for Test Task is: ");
    wprintf(L"  %s\n", ppwszComment);

    // Call CoTaskMemFree to free the string.
    CoTaskMemFree(ppwszComment);

    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskCreator(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving the Task Creator
05/31/2018

This example retrieves the name of the creator of the task and displays it on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-the-task-creator
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName{};
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();//Release ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetCreator. Note that this method is inherited from IScheduledWorkItem.
    LPWSTR ppwszCreator;
    hr = pITask->GetCreator(&ppwszCreator);
    pITask->Release();// Release the ITask interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetCreator: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    wprintf(L"The creator of Test Task is: ");
    wprintf(L"  %s\n", ppwszCreator);

    CoTaskMemFree(ppwszCreator);// Call CoTaskMemFree to free the string.
    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskExitCode(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving Task Exit Code
05/31/2018

This example retrieves the last exit code returned by a known task.
(A returned value of "0" indicates the task was never run.)
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-task-exit-code
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"TestTask";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetExitCode. Note that this method is inherited from IScheduledWorkItem.
    DWORD pdwExitCode;
    hr = pITask->GetExitCode(&pdwExitCode);
    pITask->Release();// Release ITask interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetExitCode: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    wprintf(L"The last exit code of Test Task is: %u\n", pdwExitCode);

    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskIdleWaitTime(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving Task Idle-wait Time
05/31/2018

This example retrieves the idle-wait time of the task and displays it on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-task-idle-wait-time
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetIdleWait. Note that this method is inherited from IScheduledWorkItem.
    WORD pwIdleMinutes;
    WORD pwDeadlineMinutes;
    hr = pITask->GetIdleWait(&pwIdleMinutes, &pwDeadlineMinutes);
    pITask->Release();// Release the ITask interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetIdleWait: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    wprintf(L"The idle wait of Test Task is: \n");
    wprintf(L" %u minutes\n", pwIdleMinutes);
    wprintf(L" %u minutes\n", pwDeadlineMinutes);

    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskMostRecentRunTime(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving the Task MostRecentRun Time
05/31/2018

This example retrieves the time the task was last run and displays it on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-the-task-mostrecentrun-time
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetMostRecentRunTime and display the most recent run time of this task.
    SYSTEMTIME pstLastRun;
    hr = pITask->GetMostRecentRunTime(&pstLastRun);
    pITask->Release();// Release the ITask interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling GetMostRecentRunTime: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    wprintf(L"The most recent run time for this task was: \n");
    wprintf(L"  %u/%u/%u \t %u:%02u\n", pstLastRun.wMonth, pstLastRun.wDay, pstLastRun.wYear, pstLastRun.wHour, pstLastRun.wMinute);

    CoUninitialize();

    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskNextRunTime(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving the Task NextRun Time
05/31/2018

This example retrieves the next time the task is scheduled to run and displays that time on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-the-task-nextrun-time
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetNextRunTime and display next run time of this task.
    SYSTEMTIME pstNextRun;
    hr = pITask->GetNextRunTime(&pstNextRun);
    pITask->Release();// Release the ITask interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling GetNextRunTime: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    wprintf(L"The next runtime for this task is: \n");
    wprintf(L"  %u/%u/%u \t %u:%02u\n", pstNextRun.wMonth, pstNextRun.wDay, pstNextRun.wYear, pstNextRun.wHour, pstNextRun.wMinute);

    CoUninitialize();

    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskRunTimes(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving Task Run Times
05/31/2018

This example retrieves the run times of the task and displays them on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-task-run-times
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release the ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetRunTimes and display the next five run times for this task.

    SYSTEMTIME stNow{};
    LPSYSTEMTIME pstListOfTimes{};
    LPSYSTEMTIME pstListBegin{};
    WORD wCountOfRuns = 5;

    GetSystemTime(&stNow);
    hr = pITask->GetRunTimes(&stNow, nullptr, &wCountOfRuns, &pstListBegin);
    pstListOfTimes = pstListBegin;
    pITask->Release();// Release the ITask interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling GetRunTimes: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    wprintf(L"The next %u runtimes for this task are: \n", wCountOfRuns);

    for (WORD i = 0; i < wCountOfRuns; i++) {
        wprintf(L"\t%d - %u/%u/%u \t %u:%u\n",
                i + 1,
                pstListOfTimes->wMonth,
                pstListOfTimes->wDay,
                pstListOfTimes->wYear,
                pstListOfTimes->wHour,
                pstListOfTimes->wMinute);
        pstListOfTimes++;
    }

    CoTaskMemFree(pstListBegin);
    CoTaskMemFree(pstListOfTimes);
    CoUninitialize();

    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskStatus(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving Task Status
05/31/2018

This example retrieves the current status of the task and displays it on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-task-status
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetStatus. Note that this method is inherited from IScheduledWorkItem.
    HRESULT phrStatus;
    hr = pITask->GetStatus(&phrStatus);
    pITask->Release();// Release the ITask interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetStatus: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    wprintf(L"The status of Test Task is: ");

    switch (phrStatus) {
    case SCHED_S_TASK_READY:
        wprintf(L"  SCHED_S_TASK_READY\n");
        break;
    case SCHED_S_TASK_RUNNING:
        wprintf(L"  SCHED_S_TASK_RUNNING\n");
        break;
    case SCHED_S_TASK_DISABLED:
        wprintf(L"  SCHED_S_TASK_DISABLED\n");
        break;
    case SCHED_S_TASK_HAS_NOT_RUN:
        wprintf(L"  SCHED_S_TASK_HAS_NOT_RUN\n");
        break;
    case SCHED_S_TASK_NOT_SCHEDULED:
        wprintf(L"  SCHED_S_TASK_NOT_SCHEDULED\n");
        break;
    case SCHED_S_TASK_NO_MORE_RUNS:
        wprintf(L"  SCHED_S_TASK_NO_MORE_RUNS\n");
        break;
    case SCHED_S_TASK_NO_VALID_TRIGGERS:
        wprintf(L"  SCHED_S_TASK_NO_VALID_TRIGGERS\n");
        break;
    default:
        wprintf(L"  unknown status flag!\n");
    }

    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI SettingTaskAccountInformation(int argc, char ** argv)
/*
C/C++ Code Example: Setting Task Account Information
05/31/2018

This example sets the account information for a known task.
This example assumes that the task "test task" already exists on the local computer and that the Task Scheduler service is running.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-setting-task-account-information
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};

    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            wprintf(L"Failed calling CoCreateInstance. ");
            CoUninitialize();
            return 1;
        }
    } else {
        wprintf(L"Failed calling CoInitializeEx. ");
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.

    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release the ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    //  Securely get the user name and password. 
    CREDUI_INFO cui{};
    TCHAR pszName[CREDUI_MAX_USERNAME_LENGTH] = L"";
    TCHAR pszPwd[CREDUI_MAX_PASSWORD_LENGTH] = L"";
    BOOL fSave{};
    DWORD dwErr{};

    cui.cbSize = sizeof(CREDUI_INFO);
    cui.hwndParent = nullptr;
    //  Ensure that MessageText and CaptionText identify
    //  what credentials to use and which application requires them.
    cui.pszMessageText = TEXT("Account information for task registration:");
    cui.pszCaptionText = TEXT("Enter Account Information for Task Registration");
    cui.hbmBanner = nullptr;
    fSave = FALSE;

    //  Create the UI asking for the credentials.
    dwErr = CredUIPromptForCredentials(
        &cui,                             //  CREDUI_INFO structure
        TEXT(""),                         //  Target for credentials
        nullptr,                             //  Reserved
        0,                                //  Reason
        pszName,                          //  User name
        CREDUI_MAX_USERNAME_LENGTH,       //  Max number for user name
        pszPwd,                           //  Password
        CREDUI_MAX_PASSWORD_LENGTH,       //  Max number for password
        &fSave,                           //  State of save check box
        CREDUI_FLAGS_GENERIC_CREDENTIALS |  //  Flags
        CREDUI_FLAGS_ALWAYS_SHOW_UI |
        CREDUI_FLAGS_DO_NOT_PERSIST);
    if (dwErr) {
        wprintf(L"Failed calling ITask::SetAccountInformation: ");
        wprintf(L"error = 0x%x\n", dwErr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Call ITask::SetAccountInformation to specify the account name and the account password for Test Task.
    hr = pITask->SetAccountInformation((LPCWSTR)pszName, (LPCWSTR)pszPwd);
    SecureZeroMemory(pszName, sizeof(pszName));
    SecureZeroMemory(pszPwd, sizeof(pszPwd));
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::SetAccountInformation: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Call IPersistFile::Save to save the modified task to disk.
    IPersistFile * pIPersistFile;
    hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    pITask->Release();// Release the ITask interface.
    hr = pIPersistFile->Save(nullptr, TRUE);
    if (FAILED(hr)) {
        wprintf(L"Failed calling IPersistFile::Save: ");
        wprintf(L"error = 0x%x\n", hr);
        pIPersistFile->Release();
        CoUninitialize();
        return 1;
    }

    pIPersistFile->Release();// Release the IPersistFile interface.

    wprintf(L"Set the account name and password.");
    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI SettingTaskComment(int argc, char ** argv)
/*
C/C++ Code Example: Setting Task Comment
05/31/2018

This example sets the comment for a known task.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-setting-task-comment
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};

    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release the ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::SetComment to specify the account name and the account password for Test Task.
    LPCWSTR pwszComment = L"This task is used to test the Task Scheduler APIs.";
    hr = pITask->SetComment(pwszComment);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::SetComment: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Call IPersistFile::Save to save the modified task to disk.
    IPersistFile * pIPersistFile;
    hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    pITask->Release();// Release the ITask interface.
    hr = pIPersistFile->Save(nullptr, TRUE);
    if (FAILED(hr)) {
        wprintf(L"Failed calling IPersistFile::Save: ");
        wprintf(L"error = 0x%x\n", hr);
        pIPersistFile->Release();
        CoUninitialize();
        return 1;
    }

    pIPersistFile->Release();// Release the IPersistFile interface.

    wprintf(L"Set the comment for Test Task.\n");
    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskApplicationName(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving the Task Application Name
05/31/2018

This example retrieves the name of the application associated with a given task and displays that name on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-the-task-application-name
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.

    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate; error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetApplicationName to display the name of the application associated with "Test Task".
    LPWSTR lpwszApplicationName;
    hr = pITask->GetApplicationName(&lpwszApplicationName);
    pITask->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetApplicationName\n");
        wprintf(L" error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Process the returned string. 
    wprintf(L"Test Task is associated with: %s\n", lpwszApplicationName);

    CoTaskMemFree(lpwszApplicationName);// Call CoTaskMemFree to free resources.
    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskMaxRunTime(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving the Task MaxRunTime
05/31/2018

This example retrieves the maximum amount of time the task can run (in milliseconds) and displays that number on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-the-task-maxruntime
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.

    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate;\n");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetMaxRunTime to display the maximum time Test Task is allowed to run.
    DWORD pdwRunTime;
    hr = pITask->GetMaxRunTime(&pdwRunTime);
    pITask->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetMaxRunTime: \n");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    wprintf(L"Test Task can run for %u milliseconds\n", pdwRunTime);
    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskParameters(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving Task Parameters
05/31/2018

This example retrieves the parameter string that is executed when the task is run and displays that string on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-task-parameters
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.

    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate; error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetParameters to display the parameters string associated with "Test Task".
    LPWSTR lpwszParameters;
    hr = pITask->GetParameters(&lpwszParameters);
    pITask->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetApplicationName\n");
        wprintf(L" error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Process returned parameter string. 
    wprintf(L"Test Task is associated with the following parameters:\n");
    wprintf(L"  %s\n", lpwszParameters);

    CoTaskMemFree(lpwszParameters);// Call CoTaskMemFree to free resources.
    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskPriority(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving Task Priority
05/31/2018

This example retrieves the priority level of a task and displays the path to the working directory on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-task-priority
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetPriority to retrieve the priority level of Test Task.
    DWORD pdwPriority;
    hr = pITask->GetPriority(&pdwPriority);
    pITask->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetPriority: error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }
    if (pdwPriority == HIGH_PRIORITY_CLASS) {
        wprintf(L"Test Task is a high priority task.\n");
    } else {
        wprintf(L"Test Task is not a high priority task.\n");
    }

    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskWorkingDirectory(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving the Task Working Directory
05/31/2018

This example retrieves the working directory associated with a task and displays the path to the working directory on the screen.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-the-task-working-directory
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.

    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetWorkingDirectory to display the working directory associated with "Test Task".
    LPWSTR lpwszWorkDir;
    hr = pITask->GetWorkingDirectory(&lpwszWorkDir);
    pITask->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetWorkingDirectory: \n");
        wprintf(L" error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    wprintf(L"Test Task is associated with : %s\n", lpwszWorkDir);// Process returned string. 

    CoTaskMemFree(lpwszWorkDir);// Call CoTaskMemFree to free resources.
    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI SettingApplicationName(int argc, char ** argv)
/*
C/C++ Code Example: Setting Application Name
05/31/2018

This example sets the name of the application associated with a known task.
This example assumes that the task "test task" already exists on the local computer and that the Task Scheduler service is running.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-setting-application-name
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};

    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release the ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::SetApplicationName to specify the Application name for Test Task.
    LPCWSTR pwszApplicationName = L"C:\\Windows\\System32\\notepad.exe";
    hr = pITask->SetApplicationName(pwszApplicationName);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::SetApplicationName: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Call IPersistFile::Save to save the modified task to disk.
    IPersistFile * pIPersistFile;
    hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    pITask->Release();// Release the ITask interface.
    hr = pIPersistFile->Save(nullptr, TRUE);
    if (FAILED(hr)) {
        wprintf(L"Failed calling IPersistFile::Save: ");
        wprintf(L"error = 0x%x\n", hr);
        pIPersistFile->Release();
        CoUninitialize();
        return 1;
    }

    pIPersistFile->Release();// Release the IPersistFile interface.

    wprintf(L"Set the application name for Test Task.\n");
    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI SettingMaxRunTime(int argc, char ** argv)
/*
C/C++ Code Example: Setting MaxRunTime
05/31/2018

This example sets the maximum run time (set in milliseconds) of a known task.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-setting-maxruntime
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};

    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release the ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::SetMaxRunTime to specify the maximum amount of time the task will run.
    DWORD dwMaxRunTime = (1000 * 60 * 5);
    hr = pITask->SetMaxRunTime(dwMaxRunTime);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::SetMaxRunTime: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Call IPersistFile::Save to save the modified task to disk.
    IPersistFile * pIPersistFile;
    hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    pITask->Release();// Release the ITask interface.
    hr = pIPersistFile->Save(nullptr, TRUE);
    if (FAILED(hr)) {
        wprintf(L"Failed calling IPersistFile::Save: ");
        wprintf(L"error = 0x%x\n", hr);
        pIPersistFile->Release();
        CoUninitialize();
        return 1;
    }

    pIPersistFile->Release();// Release the IPersistFile interface.

    wprintf(L"The maximum run time is set to 5 minutes.\n");

    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI SettingTaskParameters(int argc, char ** argv)
/*
C/C++ Code Example: Setting Task Parameters
05/31/2018

This example clears all command-line parameters associated with a known task.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-setting-task-parameters
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};

    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release the ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::SetParameters to L"" to clear the parameters for Test Task.
    LPCWSTR pwszParameters = L"";
    hr = pITask->SetParameters(pwszParameters);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::SetParameters: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Call IPersistFile::Save to save the modified task to disk.
    IPersistFile * pIPersistFile;
    hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    pITask->Release();// Release the ITask interface.
    hr = pIPersistFile->Save(nullptr, TRUE);
    if (FAILED(hr)) {
        wprintf(L"Failed calling IPersistFile::Save: ");
        wprintf(L"error = 0x%x\n", hr);
        pIPersistFile->Release();
        CoUninitialize();
        return 1;
    }

    pIPersistFile->Release();// Release the IPersistFile interface.

    wprintf(L"Cleared the parameters of TestTask.\n");

    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI SettingTaskPriority(int argc, char ** argv)
/*
C/C++ Code Example: Setting Task Priority
05/31/2018

This example sets the priority of a test task and then saves the task.
This example assumes that the test task already exists on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-setting-task-priority
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};

    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release the ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::SetPriority to specify the priority level of Test Task.
    DWORD dwPriority = HIGH_PRIORITY_CLASS;
    hr = pITask->SetPriority(dwPriority);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::SetPriority: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Call IPersistFile::Save to save the modified task to disk.
    IPersistFile * pIPersistFile;
    hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    pITask->Release();// Release the ITask interface.
    hr = pIPersistFile->Save(nullptr, TRUE);
    if (FAILED(hr)) {
        wprintf(L"Failed calling IPersistFile::Save: ");
        wprintf(L"error = 0x%x\n", hr);
        pIPersistFile->Release();
        CoUninitialize();
        return 1;
    }

    pIPersistFile->Release();// Release the IPersistFile interface.

    wprintf(L"Set the priority of TestTask to HIGH_PRIORITY_CLASS.\n");

    CoUninitialize();
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI SettingWorkingDirectory(int argc, char ** argv)
/*
C/C++ Code Example: Setting Working Directory
05/31/2018

This example sets the working directory of a known task.
This example assumes that the task and the test task already exist on the local computer.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-setting-working-directory
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};

    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release the ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::SetWorkingDirectory to specify the current working directory for Test Task.
    LPCWSTR pwszWorkingDirectory = L"C:\\Temp";
    hr = pITask->SetWorkingDirectory(pwszWorkingDirectory);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::SetWorkingDirectory: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Call IPersistFile::Save to save the modified task to disk.
    IPersistFile * pIPersistFile;
    hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    pITask->Release();// Release the ITask interface.
    hr = pIPersistFile->Save(nullptr, TRUE);
    if (FAILED(hr)) {
        wprintf(L"Failed calling IPersistFile::Save: ");
        wprintf(L"error = 0x%x\n", hr);
        pIPersistFile->Release();
        CoUninitialize();
        return 1;
    }

    pIPersistFile->Release();// Release the IPersistFile interface.

    wprintf(L"Set the working directory to C:\\Temp.\n");

    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTaskPage(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving a Task Page
05/31/2018

This example retrieves and displays the Task page of a known task.
Note that in this example, all interfaces are released when they are no longer needed.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-a-task-page
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();// Release the ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::QueryInterface to retrieve the IProvideTaskPage interface, and call IProvideTaskPage::GetPage to retrieve the task page.
    TASKPAGE tpType = TASKPAGE_TASK;
    BOOL fPersistChanges = TRUE;
    HPROPSHEETPAGE phPage{};
    IProvideTaskPage * pIProvTaskPage{};
    hr = pITask->QueryInterface(IID_IProvideTaskPage, (void **)&pIProvTaskPage);
    pITask->Release();// Release the ITask interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::QueryInterface: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    hr = pIProvTaskPage->GetPage(tpType, fPersistChanges, &phPage);
    pIProvTaskPage->Release();// Release the IProvideTaskPage interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling IProvideTaskPage::GetPage: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Display the page using additional code.
    PROPSHEETHEADER psh;
    ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_DEFAULT | PSH_NOAPPLYNOW;
    psh.phpage = &phPage;
    psh.nPages = 1;
    int psResult = (int)PropertySheet(&psh);
    if (psResult <= 0) {
        wprintf(L"Failed to create the property page: ");
        wprintf(L"0x%x\n", GetLastError());
    }

    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI CreatingTaskTrigger(int argc, char ** argv)
/*
C/C++ Code Example: Creating a Task Trigger
05/31/2018

This example creates a new trigger for an existing task named Test Task.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-creating-a-task-trigger
*/
{
    HRESULT hr = S_OK;
    ITaskScheduler * pITS{};

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::CreateTrigger to create new trigger.
    ITaskTrigger * pITaskTrigger;
    WORD piNewTrigger;
    hr = pITask->CreateTrigger(&piNewTrigger, &pITaskTrigger);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::CreatTrigger: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Define TASK_TRIGGER structure. Note that wBeginDay,
    // wBeginMonth, and wBeginYear must be set to a valid day, month, and year respectively.
    TASK_TRIGGER pTrigger;
    ZeroMemory(&pTrigger, sizeof(TASK_TRIGGER));

    // Add code to set trigger structure?
    pTrigger.wBeginDay = 1;                  // Required
    pTrigger.wBeginMonth = 1;                // Required
    pTrigger.wBeginYear = 1999;              // Required
    pTrigger.cbTriggerSize = sizeof(TASK_TRIGGER);
    pTrigger.wStartHour = 13;
    pTrigger.TriggerType = TASK_TIME_TRIGGER_DAILY;
    pTrigger.Type.Daily.DaysInterval = 1;

    // Call ITaskTrigger::SetTrigger to set trigger criteria.
    hr = pITaskTrigger->SetTrigger(&pTrigger);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskTrigger::SetTrigger: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        pITaskTrigger->Release();
        CoUninitialize();
        return 1;
    }

    // Call IPersistFile::Save to save trigger to disk.
    IPersistFile * pIPersistFile;
    hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    hr = pIPersistFile->Save(nullptr, TRUE);
    if (FAILED(hr)) {
        wprintf(L"Failed calling IPersistFile::Save: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        pITaskTrigger->Release();
        pIPersistFile->Release();
        CoUninitialize();
        return 1;
    }

    wprintf(L"The trigger was created and IPersistFile::Save was \n");
    wprintf(L"called to save the new trigger to disk.\n");

    // Release resources.
    pITask->Release();
    pITaskTrigger->Release();
    pIPersistFile->Release();
    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI CreatingIdleTrigger(int argc, char ** argv)
/*
C/C++ Code Example: Creating an Idle Trigger
05/31/2018

This example creates an idle trigger for an existing task.
For information about idle conditions, see Task Idle Conditions.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-creating-an-idle-trigger
*/
{
    HRESULT hr = S_OK;
    ITaskScheduler * pITS{};

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::SetIdleWait to specify idle time.
    WORD wIdleMinutes = 3;
    WORD wDeadlineMinutes = 5;
    hr = pITask->SetIdleWait(wIdleMinutes, wDeadlineMinutes);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::SetIdleWait: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Call ITask::CreateTrigger to create new trigger.
    ITaskTrigger * pITaskTrigger;
    WORD piNewTrigger;
    hr = pITask->CreateTrigger(&piNewTrigger, &pITaskTrigger);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::CreatTrigger: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Define TASK_TRIGGER structure and call ITask::CreateTrigger to create the idle trigger.
    TASK_TRIGGER pTrigger;
    ZeroMemory(&pTrigger, sizeof(TASK_TRIGGER));

    // Add code to set trigger structure.
    pTrigger.wBeginDay = 1;
    pTrigger.wBeginMonth = 1;
    pTrigger.wBeginYear = 1999;
    pTrigger.cbTriggerSize = sizeof(TASK_TRIGGER);
    pTrigger.TriggerType = TASK_EVENT_TRIGGER_ON_IDLE;

    hr = pITaskTrigger->SetTrigger(&pTrigger);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskTrigger::SetTrigger: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        pITaskTrigger->Release();
        CoUninitialize();
        return 1;
    }

    // Call IPersistFile::Save to save setting to disk.
    IPersistFile * pIPersistFile;
    hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::QueryInterface: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        pITaskTrigger->Release();
        CoUninitialize();
        return 1;
    }

    hr = pIPersistFile->Save(nullptr, TRUE);
    if (FAILED(hr)) {
        wprintf(L"Failed calling IPersistFile::Save: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        pITaskTrigger->Release();
        pIPersistFile->Release();
        CoUninitialize();
        return 1;
    }
    wprintf(L"The idle trigger was set and IPersistFile::Save \n");
    wprintf(L"was called to save the new trigger to disk.\n");

    // Release all resources.
    pITask->Release();
    pITaskTrigger->Release();
    pIPersistFile->Release();
    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI TerminatingTask(int argc, char ** argv)
/*
C/C++ Code Example: Terminating a Task
05/31/2018

This example verifies the status of a known task and terminates the task if it is running.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-terminating-a-task
*/
{
    HRESULT hr = S_OK;

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    ITaskScheduler * pITS{};
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName;
    lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();//Release ITaskScheduler interface.
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::GetStatus. Note that this method is inherited from IScheduledWorkItem.
    HRESULT phrStatus;
    hr = pITask->GetStatus(&phrStatus);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetStatus: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Call ITask::Terminate if the status is SCHED_S_TASK_RUNNING.
    if (phrStatus == SCHED_S_TASK_RUNNING) {
        hr = pITask->Terminate();
        if (FAILED(hr)) {
            wprintf(L"Failed calling ITask::Terminate: ");
            wprintf(L"error = 0x%x\n", hr);
            pITask->Release();
            CoUninitialize();
            return 1;
        }
        wprintf(L"Test Task is terminated.\n");
    } else {
        wprintf(L"Test Task is not running.\n");
    }

    pITask->Release();// Release the ITask interface.
    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI RetrievingTriggerStrings(int argc, char ** argv)
/*
C/C++ Code Example: Retrieving Trigger Strings
05/31/2018

This example retrieves a trigger string for all the triggers associated with a known task.

https://docs.microsoft.com/en-us/windows/win32/taskschd/c-c-code-example-retrieving-trigger-strings
*/
{
    HRESULT hr = S_OK;
    ITaskScheduler * pITS{};

    // Call CoInitialize to initialize the COM library and then call CoCreateInstance to get the Task Scheduler object.
    hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&pITS);
        if (FAILED(hr)) {
            CoUninitialize();
            return 1;
        }
    } else {
        return 1;
    }

    // Call ITaskScheduler::Activate to get the Task object.
    ITask * pITask{};
    LPCWSTR lpcwszTaskName = L"Test Task";
    hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown **)&pITask);
    pITS->Release();
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITaskScheduler::Activate: ");
        wprintf(L"error = 0x%x\n", hr);
        CoUninitialize();
        return 1;
    }

    // Call ITask::TriggerCount to retrieve the number of triggers associated with the task.
    WORD plTriggerCount;
    hr = pITask->GetTriggerCount(&plTriggerCount);
    if (FAILED(hr)) {
        wprintf(L"Failed calling ITask::GetTriggerCount: ");
        wprintf(L"error = 0x%x\n", hr);
        pITask->Release();
        CoUninitialize();
        return 1;
    }

    // Display the trigger stings, calling ITask::GetTriggerString for each trigger associated with the task.
    wprintf(L"There are %u triggers with Test Task.\n", plTriggerCount);
    wprintf(L"They are:\n");

    WORD CurrentTrigger = 0;
    LPWSTR ppwszTrigger = nullptr;
    for (CurrentTrigger = 0; CurrentTrigger < plTriggerCount; CurrentTrigger++) {
        pITask->GetTriggerString(CurrentTrigger, &ppwszTrigger);
        if (FAILED(hr)) {
            wprintf(L"Failed calling ITask::GetTriggerString: ");
            wprintf(L"error = 0x%x\n", hr);
            pITask->Release();
            CoUninitialize();
            return 1;
        }

        wprintf(L"%i) %s\n", CurrentTrigger + 1, ppwszTrigger);
    }

    // Release resources.
    CoTaskMemFree(ppwszTrigger);
    pITask->Release();
    CoUninitialize();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
