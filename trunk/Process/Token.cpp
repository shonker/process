#include "pch.h"
#include "Token.h"


#pragma warning(disable:6011)
#pragma warning(disable:6387)
#pragma warning(disable:26451)
#pragma warning(disable:28182)
#pragma warning(disable:6386)


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
BOOL WINAPI SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege   // to enable or disable privilege
)
/*
Enabling and Disabling Privileges in C++

https://docs.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
*/
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(
        NULL,            // lookup privilege on local system
        lpszPrivilege,   // privilege to lookup 
        &luid))        // receives LUID of privilege
    {
        printf("LookupPrivilegeValue error: %u\n", GetLastError());
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
        printf("AdjustTokenPrivileges error: %u\n", GetLastError());
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
        printf("The token does not have the specified privilege. \n");
        return FALSE;
    }

    return TRUE;
}


EXTERN_C
__declspec(dllexport)
BOOL WINAPI AdjustCurrentProcessPrivilege(PCTSTR szPrivilege, BOOL fEnable)
/*
功能：本进程的特权开启的开关。

细节：
叫Set不如叫Get，叫Get不如叫Adjust（调整）。

如：
EnablePrivilege(SE_DEBUG_NAME, TRUE);
EnablePrivilege(SE_DEBUG_NAME, FALSE);
*/
{
    // Enabling the debug privilege allows the application to see information about service applications
    BOOL fOk = FALSE;    // Assume function fails
    HANDLE hToken;

    // Try to open this process's access token
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        // Attempt to modify the given privilege
        TOKEN_PRIVILEGES tp;

        tp.PrivilegeCount = 1;
        LookupPrivilegeValue(NULL, szPrivilege, &tp.Privileges[0].Luid);

        tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
        AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
        fOk = (GetLastError() == ERROR_SUCCESS);

        CloseHandle(hToken);// Don't forget to close the token handle
    }

    return(fOk);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
void WINAPI SetLowLabelToFile()
/*
Lowering a resource mandatory label

Designing Applications to Run at a Low Integrity Level

https://msdn.microsoft.com/en-us/library/bb625960.aspx
https://docs.microsoft.com/en-us/previous-versions/dotnet/articles/bb625960(v=msdn.10)?redirectedfrom=MSDN
*/
{
    // The LABEL_SECURITY_INFORMATION SDDL SACL to be set for low integrity 
    DWORD dwErr = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PACL pSacl = NULL; // not allocated
    BOOL fSaclPresent = FALSE;
    BOOL fSaclDefaulted = FALSE;
    LPCWSTR pwszFileName = L"Sample.txt";

    if (ConvertStringSecurityDescriptorToSecurityDescriptorW(LOW_INTEGRITY_SDDL_SACL_W, SDDL_REVISION_1, &pSD, NULL)) {
        if (GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl, &fSaclDefaulted)) {
            // Note that psidOwner, psidGroup, and pDacl are all NULL and set the new LABEL_SECURITY_INFORMATION
            dwErr = SetNamedSecurityInfoW((LPWSTR)pwszFileName, SE_FILE_OBJECT, LABEL_SECURITY_INFORMATION, NULL, NULL, NULL, pSacl);
        }
        LocalFree(pSD);
    }
}


EXTERN_C
__declspec(dllexport)
void WINAPI CreateLowProcess()
/*
低完整性级别的进程的设置和启动

Starting a process at low integrity

Designing Applications to Run at a Low Integrity Level

https://msdn.microsoft.com/en-us/library/bb625960.aspx
https://docs.microsoft.com/en-us/previous-versions/dotnet/articles/bb625960(v=msdn.10)?redirectedfrom=MSDN
*/
{
    BOOL                  fRet;
    HANDLE                hToken = NULL;
    HANDLE                hNewToken = NULL;
    PSID                  pIntegritySid = NULL;
    TOKEN_MANDATORY_LABEL TIL = {0};
    PROCESS_INFORMATION   ProcInfo = {0};
    STARTUPINFO           StartupInfo = {0};
    WCHAR wszProcessName[MAX_PATH] = L"C:\\Windows\\System32\\Notepad.exe";// Notepad is used as an example    
    WCHAR wszIntegritySid[20] = L"S-1-16-1024";// Low integrity SID

    fRet = OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY, &hToken);
    if (!fRet) {
        goto CleanExit;
    }

    fRet = DuplicateTokenEx(hToken, 0, NULL, SecurityImpersonation, TokenPrimary, &hNewToken);
    if (!fRet) {
        goto CleanExit;
    }

    fRet = ConvertStringSidToSid(wszIntegritySid, &pIntegritySid);
    if (!fRet) {
        goto CleanExit;
    }

    TIL.Label.Attributes = SE_GROUP_INTEGRITY;
    TIL.Label.Sid = pIntegritySid;

    // Set the process integrity level
    fRet = SetTokenInformation(hNewToken, 
                               TokenIntegrityLevel, 
                               &TIL,
                               sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid));
    if (!fRet) {
        goto CleanExit;
    }

    // Create the new process at Low integrity
    fRet = CreateProcessAsUser(hNewToken, NULL, wszProcessName, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcInfo);
    /*
    win10,这行出错。0xc0000022.
    */

CleanExit:

    if (ProcInfo.hProcess != NULL) {
        CloseHandle(ProcInfo.hProcess);
    }

    if (ProcInfo.hThread != NULL) {
        CloseHandle(ProcInfo.hThread);
    }

    LocalFree(pIntegritySid);

    if (hNewToken != NULL) {
        CloseHandle(hNewToken);
    }

    if (hToken != NULL) {
        CloseHandle(hToken);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////


BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid)
{
    ACCESS_ALLOWED_ACE * pace = NULL;
    ACL_SIZE_INFORMATION aclSizeInfo;
    BOOL                 bDaclExist;
    BOOL                 bDaclPresent;
    BOOL                 bSuccess = FALSE;
    DWORD                dwNewAclSize;
    DWORD                dwSidSize = 0;
    DWORD                dwSdSizeNeeded;
    PACL                 pacl;
    PACL                 pNewAcl = NULL;
    PSECURITY_DESCRIPTOR psd = NULL;
    PSECURITY_DESCRIPTOR psdNew = NULL;
    PVOID                pTempAce;
    SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
    unsigned int         i;

    __try {
        // Obtain the DACL for the window station.
        if (!GetUserObjectSecurity(hwinsta, &si, psd, dwSidSize, &dwSdSizeNeeded))
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                psd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSdSizeNeeded);
                if (psd == NULL)
                    __leave;

                psdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSdSizeNeeded);
                if (psdNew == NULL)
                    __leave;

                dwSidSize = dwSdSizeNeeded;
                if (!GetUserObjectSecurity(hwinsta, &si, psd, dwSidSize, &dwSdSizeNeeded))
                    __leave;
            } else
                __leave;

            // Create a new DACL.
            if (!InitializeSecurityDescriptor(psdNew, SECURITY_DESCRIPTOR_REVISION))
                __leave;

            // Get the DACL from the security descriptor.
            if (!GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl, &bDaclExist))
                __leave;

            // Initialize the ACL.
            ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
            aclSizeInfo.AclBytesInUse = sizeof(ACL);

            // Call only if the DACL is not NULL.
            if (pacl != NULL) {
                // get the file ACL size info
                if (!GetAclInformation(pacl, (LPVOID)&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
                    __leave;
            }

            // Compute the size of the new ACL.
            dwNewAclSize = aclSizeInfo.AclBytesInUse +
                (2 * sizeof(ACCESS_ALLOWED_ACE)) + (2 * GetLengthSid(psid)) -
                (2 * sizeof(DWORD));

            // Allocate memory for the new ACL.
            pNewAcl = (PACL)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNewAclSize);
            if (pNewAcl == NULL)
                __leave;

            // Initialize the new DACL.
            if (!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION))
                __leave;

            // If DACL is present, copy it to a new DACL.
            if (bDaclPresent) {
                // Copy the ACEs to the new ACL.
                if (aclSizeInfo.AceCount) {
                    for (i = 0; i < aclSizeInfo.AceCount; i++) {
                        // Get an ACE.
                        if (!GetAce(pacl, i, &pTempAce))
                            __leave;

                        // Add the ACE to the new ACL.
                        if (!AddAce(pNewAcl, ACL_REVISION, MAXDWORD, pTempAce, ((PACE_HEADER)pTempAce)->AceSize))
                            __leave;
                    }
                }
            }

            // Add the first ACE to the window station.
            pace = (ACCESS_ALLOWED_ACE *)HeapAlloc(
                GetProcessHeap(),
                HEAP_ZERO_MEMORY,
                sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psid) - sizeof(DWORD));
            if (pace == NULL)
                __leave;

            pace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
            pace->Header.AceFlags = CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
            pace->Header.AceSize = LOWORD(sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psid) - sizeof(DWORD));
            pace->Mask = GENERIC_ACCESS;
            if (!CopySid(GetLengthSid(psid), &pace->SidStart, psid))
                __leave;

            if (!AddAce(pNewAcl, ACL_REVISION, MAXDWORD, (LPVOID)pace, pace->Header.AceSize))
                __leave;

            // Add the second ACE to the window station.
            pace->Header.AceFlags = NO_PROPAGATE_INHERIT_ACE;
            pace->Mask = WINSTA_ALL;
            if (!AddAce(pNewAcl, ACL_REVISION, MAXDWORD, (LPVOID)pace, pace->Header.AceSize))
                __leave;

            // Set a new DACL for the security descriptor.
            if (!SetSecurityDescriptorDacl(psdNew, TRUE, pNewAcl, FALSE))
                __leave;

            // Set the new security descriptor for the window station.
            if (!SetUserObjectSecurity(hwinsta, &si, psdNew))
                __leave;

            // Indicate success.
            bSuccess = TRUE;
    } __finally {
        // Free the allocated buffers.

        if (pace != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)pace);

        if (pNewAcl != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);

        if (psd != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)psd);

        if (psdNew != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)psdNew);
    }

    return bSuccess;

}


BOOL AddAceToDesktop(HDESK hdesk, PSID psid)
{
    ACL_SIZE_INFORMATION aclSizeInfo;
    BOOL                 bDaclExist;
    BOOL                 bDaclPresent;
    BOOL                 bSuccess = FALSE;
    DWORD                dwNewAclSize;
    DWORD                dwSidSize = 0;
    DWORD                dwSdSizeNeeded;
    PACL                 pacl;
    PACL                 pNewAcl = NULL;
    PSECURITY_DESCRIPTOR psd = NULL;
    PSECURITY_DESCRIPTOR psdNew = NULL;
    PVOID                pTempAce;
    SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
    unsigned int         i;

    __try {
        // Obtain the security descriptor for the desktop object.
        if (!GetUserObjectSecurity(hdesk, &si, psd, dwSidSize, &dwSdSizeNeeded)) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                psd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSdSizeNeeded);
                if (psd == NULL)
                    __leave;

                psdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSdSizeNeeded);
                if (psdNew == NULL)
                    __leave;

                dwSidSize = dwSdSizeNeeded;
                if (!GetUserObjectSecurity(hdesk, &si, psd, dwSidSize, &dwSdSizeNeeded))
                    __leave;
            } else
                __leave;
        }

        // Create a new security descriptor.
        if (!InitializeSecurityDescriptor(psdNew, SECURITY_DESCRIPTOR_REVISION))
            __leave;

        // Obtain the DACL from the security descriptor.
        if (!GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl, &bDaclExist))
            __leave;

        // Initialize.
        ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
        aclSizeInfo.AclBytesInUse = sizeof(ACL);

        // Call only if NULL DACL.
        if (pacl != NULL) {
            // Determine the size of the ACL information.
            if (!GetAclInformation(pacl, (LPVOID)&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
                __leave;
        }

        // Compute the size of the new ACL.
        dwNewAclSize = aclSizeInfo.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psid) - sizeof(DWORD);

        // Allocate buffer for the new ACL.
        pNewAcl = (PACL)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNewAclSize);
        if (pNewAcl == NULL)
            __leave;

        // Initialize the new ACL.
        if (!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION))
            __leave;

        // If DACL is present, copy it to a new DACL.
        if (bDaclPresent) {
            // Copy the ACEs to the new ACL.
            if (aclSizeInfo.AceCount) {
                for (i = 0; i < aclSizeInfo.AceCount; i++) {
                    // Get an ACE.
                    if (!GetAce(pacl, i, &pTempAce))
                        __leave;

                    // Add the ACE to the new ACL.
                    if (!AddAce(pNewAcl, ACL_REVISION, MAXDWORD, pTempAce, ((PACE_HEADER)pTempAce)->AceSize))
                        __leave;
                }
            }
        }

        // Add ACE to the DACL.
        if (!AddAccessAllowedAce(pNewAcl, ACL_REVISION, DESKTOP_ALL, psid))
            __leave;

        // Set new DACL to the new security descriptor.
        if (!SetSecurityDescriptorDacl(psdNew, TRUE, pNewAcl, FALSE))
            __leave;

        // Set the new security descriptor for the desktop object.
        if (!SetUserObjectSecurity(hdesk, &si, psdNew))
            __leave;

        // Indicate success.
        bSuccess = TRUE;
    } __finally {
        // Free buffers.

        if (pNewAcl != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);

        if (psd != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)psd);

        if (psdNew != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)psdNew);
    }

    return bSuccess;
}


EXTERN_C
__declspec(dllexport)
BOOL WINAPI StartInteractiveClientProcess(
    LPTSTR lpszUsername,    // client to log on
    LPTSTR lpszDomain,      // domain of client's account
    LPTSTR lpszPassword,    // client's password
    LPTSTR lpCommandLine    // command line to execute
)
/*
Starting an Interactive Client Process in C++
11/17/2013

The following example uses the LogonUser function to start a new logon session for a client.
The example gets the logon SID from the client's access token,
and uses it to add access control entries (ACEs) to the discretionary access control list (DACL) of the interactive window station and desktop.
The ACEs allow the client access to the interactive desktop for the duration of the logon session.
Next, the example calls the ImpersonateLoggedOnUser function to ensure that it has access to the client's executable file.
A call to the CreateProcessAsUser function creates the client's process, specifying that it run in the interactive desktop.
Note that your process must have the SE_ASSIGNPRIMARYTOKEN_NAME and SE_INCREASE_QUOTA_NAME privileges for successful execution of CreateProcessAsUser.
Before the function returns, it calls the RevertToSelf function to end the caller's impersonation of the client.

This example calls the GetLogonSID and FreeLogonSID functions described in Getting the Logon SID in C++.

https://docs.microsoft.com/en-us/previous-versions//aa379608(v=vs.85)?redirectedfrom=MSDN
*/
{
    HANDLE      hToken;
    HDESK       hdesk = NULL;
    HWINSTA     hwinsta = NULL, hwinstaSave = NULL;
    PROCESS_INFORMATION pi;
    PSID pSid = NULL;
    STARTUPINFO si;
    BOOL bResult = FALSE;

    // Log the client on to the local computer.
    if (!LogonUser(lpszUsername, lpszDomain, lpszPassword, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken)) {
        goto Cleanup;
    }

    // Save a handle to the caller's current window station.
    if ((hwinstaSave = GetProcessWindowStation()) == NULL)
        goto Cleanup;

    // Get a handle to the interactive window station.
    hwinsta = OpenWindowStation(
        _T("winsta0"),                   // the interactive window station 
        FALSE,                       // handle is not inheritable
        READ_CONTROL | WRITE_DAC);   // rights to read/write the DACL
    if (hwinsta == NULL)
        goto Cleanup;

    // To get the correct default desktop, set the caller's 
    // window station to the interactive window station.
    if (!SetProcessWindowStation(hwinsta))
        goto Cleanup;

    // Get a handle to the interactive desktop.
    hdesk = OpenDesktop(
        _T("default"),     // the interactive window station 
        0,             // no interaction with other desktop processes
        FALSE,         // handle is not inheritable
        READ_CONTROL | // request the rights to read and write the DACL
        WRITE_DAC | DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS);

    // Restore the caller's window station.
    if (!SetProcessWindowStation(hwinstaSave))
        goto Cleanup;

    if (hdesk == NULL)
        goto Cleanup;

    // Get the SID for the client's logon session.
    if (!GetLogonSID(hToken, &pSid))
        goto Cleanup;

    // Allow logon SID full access to interactive window station.
    if (!AddAceToWindowStation(hwinsta, pSid))
        goto Cleanup;

    // Allow logon SID full access to interactive desktop.
    if (!AddAceToDesktop(hdesk, pSid))
        goto Cleanup;

    // Impersonate client to ensure access to executable file.
    if (!ImpersonateLoggedOnUser(hToken))
        goto Cleanup;

    // Initialize the STARTUPINFO structure.
    // Specify that the process runs in the interactive desktop.
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = (LPWSTR)TEXT("winsta0\\default");

    // Launch the process in the client's logon session.
    bResult = CreateProcessAsUser(
        hToken,            // client's access token
        NULL,              // file to execute
        lpCommandLine,     // command line
        NULL,              // pointer to process SECURITY_ATTRIBUTES
        NULL,              // pointer to thread SECURITY_ATTRIBUTES
        FALSE,             // handles are not inheritable
        NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,   // creation flags
        NULL,              // pointer to new environment block 
        NULL,              // name of current directory 
        &si,               // pointer to STARTUPINFO structure
        &pi                // receives information about new process
    );

    // End impersonation of client.
    RevertToSelf();

    if (bResult && pi.hProcess != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
    }

    if (pi.hThread != INVALID_HANDLE_VALUE)
        CloseHandle(pi.hThread);

Cleanup:

    if (hwinstaSave != NULL)
        SetProcessWindowStation(hwinstaSave);

    // Free the buffer for the logon SID.
    if (pSid)
        FreeLogonSID(&pSid);

    // Close the handles to the interactive window station and desktop.

    if (hwinsta)
        CloseWindowStation(hwinsta);

    if (hdesk)
        CloseDesktop(hdesk);

    // Close the handle to the client's access token.
    if (hToken != INVALID_HANDLE_VALUE)
        CloseHandle(hToken);

    return bResult;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
BOOL WINAPI SearchTokenGroupsForSID(VOID)
/*
Searching for a SID in an Access Token in C++
2018/05/31

The following example uses the OpenProcessToken and GetTokenInformation functions to get the group memberships in an access token.
Then it uses the AllocateAndInitializeSid function to create a SID that identifies the well-known SID of the administrator group for the local computer.
Next, it uses the EqualSid function to compare the well-known SID with the group SIDs from the access token.
If the SID is present in the token, the function checks the attributes of the SID to determine whether it is enabled.

The CheckTokenMembership function should be used to determine whether a specified SID is present and enabled in an access token.
This function eliminates potential misinterpretations of the active group membership.

http://msdn.microsoft.com/en-us/library/windows/desktop/aa379554(v=vs.85).aspx
https://docs.microsoft.com/zh-cn/windows/win32/secauthz/searching-for-a-sid-in-an-access-token-in-c--?redirectedfrom=MSDN
*/
{
    DWORD i, dwSize = 0, dwResult = 0;
    HANDLE hToken;
    PTOKEN_GROUPS pGroupInfo;
    SID_NAME_USE SidType;
    char lpName[MAX_NAME];
    char lpDomain[MAX_NAME];
    PSID pSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;

    // Open a handle to the access token for the calling process.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        printf("OpenProcessToken Error %u\n", GetLastError());
        return FALSE;
    }

    // Call GetTokenInformation to get the buffer size.
    if (!GetTokenInformation(hToken, TokenGroups, NULL, dwSize, &dwSize)) {
        dwResult = GetLastError();
        if (dwResult != ERROR_INSUFFICIENT_BUFFER) {
            printf("GetTokenInformation Error %u\n", dwResult);
            return FALSE;
        }
    }

    // Allocate the buffer.
    pGroupInfo = (PTOKEN_GROUPS)GlobalAlloc(GPTR, dwSize);

    // Call GetTokenInformation again to get the group information.
    if (!GetTokenInformation(hToken, TokenGroups, pGroupInfo, dwSize, &dwSize)) {
        printf("GetTokenInformation Error %u\n", GetLastError());
        return FALSE;
    }

    // Create a SID for the BUILTIN\Administrators group.
    if (!AllocateAndInitializeSid(&SIDAuth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pSID)) {
        printf("AllocateAndInitializeSid Error %u\n", GetLastError());
        return FALSE;
    }

    // Loop through the group SIDs looking for the administrator SID.
    for (i = 0; i < pGroupInfo->GroupCount; i++) {
        if (EqualSid(pSID, pGroupInfo->Groups[i].Sid)) {   // Lookup the account name and print it.
            dwSize = MAX_NAME;
            if (!LookupAccountSid(NULL,
                                  pGroupInfo->Groups[i].Sid,
                                  (LPWSTR)lpName,
                                  &dwSize,
                                  (LPWSTR)lpDomain,
                                  &dwSize,
                                  &SidType))  //此函数能实现根据sid获取用户名的功能,进而可以想办法利用此函数进行枚举.
            {
                dwResult = GetLastError();
                if (dwResult == ERROR_NONE_MAPPED)
                    strcpy_s(lpName, dwSize, "NONE_MAPPED");
                else {
                    printf("LookupAccountSid Error %u\n", GetLastError());
                    return FALSE;
                }
            }
            printf("Current user is a member of the %s\\%s group\n", lpDomain, lpName);

            // Find out whether the SID is enabled in the token.
            if (pGroupInfo->Groups[i].Attributes & SE_GROUP_ENABLED)
                printf("The group SID is enabled.\n");
            else if (pGroupInfo->Groups[i].Attributes & SE_GROUP_USE_FOR_DENY_ONLY)
                printf("The group SID is a deny-only SID.\n");
            else
                printf("The group SID is not enabled.\n");
        }
    }

    if (pSID)
        FreeSid(pSID);
    if (pGroupInfo)
        GlobalFree(pGroupInfo);

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


/*
Dealing with Administrator and standard user’s context
2010/05/31
7 分钟可看完
With introduction of UAC, I often get two questions for Windows Vista and later.

1) How to launch an application in the Administrative context from an application which is running in standard user’s context?
2) How to launch application in standard user’s context from an application which is running in administrative context?

The first question has been discussed in details on multiple places.
The following description clearly talks about the methodologies which can be used and their consequences.

Spawning Interactive Child Processes

There is no published API for applications to spawn an arbitrary program as an adminstrator like Windows Explorer’s “Run as Administrator” context menu.
The overall UAC design philosophy is that admin and installation applications need to be detected by the program launch mechanism rather than individually started as elevated or not by the calling program.
This frees the caller from having to know which processes to run elevated and which to run as standard users.

CreateProcess() and CreateProcessWithLogonW do not have new flags to launch the child process as elevated.
Internally, CreateProcess() checks whether the target application requires elevation by looking for a manifest, determining if it is an installer, or if it has an app compat shim.
If CreateProcess() determines the target application requires elevation, it simply fails with ERROR_ELEVATION_REQUIRED(740).
It will not contact the AIS to perform the elevation prompt or run the app.
If CreateProcess() determines the application doesn’t require elevation, it will spawn it as a new process.

Likewise, it would seem reasonable to attempt this with the following code, but it doesn’t work:

OpenProcessToken (GetCurrentProcess, …)
GetTokenInformation (TokenLinkedToken)
CreateProcessWithTokenW(hLinkedToken, …)

To programmatically launch a child process as an elevated process, two things must occur: first,
the executable of the child process needs to be identified as needing elevation, and second,
the parent process needs to use ShellExecute() or ShellExecuteEx().
Applications that are designed to perform admin tasks should be marked with a signed application manifest that sets the requestedExecutionLevel to “requireAdministrator”.
See page 60-71 of “Windows Vista Application Development Requirements for User Account Control Compatibility” in the reading list below.
Then, when an application uses ShellExecute(), the following sequence of events occurs:

Parent Process is Standard User

The Windows Vista standard user launch path is similar to the Windows XP launch path, but includes some modifications.

1) Application calls ShellExecute() which calls CreateProcess().

2) CreateProcess() calls AppCompat, Fusion, and Installer Detection to assess if the application requires elevation.
The executable is then inspected to determine its requestedExecutionLevel, which is stored in the executable's application manifest.
The AppCompat database stores information for an application's application compatibility fix entries. Installer Detection detects setup executables.

3) CreateProcess() returns a Win32 error code stating ERROR_ELEVATION_REQUIRED.

4) ShellExecute() looks specifically for this new error and, upon receiving it,
calls across to the Application Information service (AIS) to attempt the elevated launch.

Parent Process is an Elevated User

The Windows Vista elevated launch path is a new Windows launch path.

5) Application calls ShellExecute() which calls the AIS.

6) AIS receives the call from ShellExecute() and re-evaluates the requested execution level and Group Policy to determine if the elevation is allowed and to define the elevation user experience.

7) If the requested execution level requires elevation, the service launches the elevation prompt on the caller’s interactive desktop (based on Group Policy),
using the HWND passed in from ShellExecute().

8) After the user has given consent or valid credentials, AIS will retrieve the corresponding access token associated with the appropriate user, if necessary.
For example, an application requesting a requestedExecutionLevel of highestAvailable will retrieve different access tokens for a user that is only a member of the Backup Operators group than for a member of the local Administrators group.

9) AIS re-issues a CreateProcessAsUser() call, supplying the administrator access token and specifying the caller’s interactive desktop.

Spawning Non-interactive Child Processes

One cannot launch non-interactive child processes as another user with FULL access token from a parent process that is started with LUA access token.
It may seem reasonable to attempt the following code to launch a process with FULL access token from a process started with LUA access token, but, it doesn't work.

LogonUser(member of local Administrators), ...

          LOGON32_LOGON_BATCH, .., &hToken)

ImpersonateLoggedOnUser(&hToken)

CreateProcessWithTokenW(hToken, ...)

If one needs to launch non-interactive processes,
they can do only from parent processes that are started with FULL access token such as LocalSystem service or service running under an account that is member of Administrators group.
Non-interactive software installations as Administrator may use methods like SMS deployment.

For question 2, it becomes difficulty to decrease the privileges of the child process.
In earlier case we have found that an application must use manifest to let operating system know the level of execution required.
Remember there are three levels which can be defined in an application manifest.


Requested Execution Levels

Possible requested execution level values

表 1
Value

Description

Comment

asInvoker

The application runs with the same access token as the parent process.

Recommended for standard user applications. Do refractoring with internal elevation points, as per the guidance provided earlier in this document.

highestAvailable

The application runs with the highest privileges the current user can obtain.

Recommended for mixed-mode applications. Plan to refractor the application in a future release.

requireAdministrator

The application runs only for administrators and requires that the application be launched with the full access token of an administrator.

Recommended for administrator only applications. Internal elevation points are not needed. The application is already running elevated.

If the caller is an Administrator you will not be able to use any of the execution level to ensure that the child application launched uses standard user token.

In this case one reasonable approach would be with following code and it should work.

OpenProcessToken (GetCurrentProcess, …)
GetTokenInformation (TokenLinkedToken)
CreateProcessWithTokenW(hLinkedToken, …)

If you are running as in interactive administrative context you can use following approach.

See the following sample, this sample if launched as an administrator will launch Nodepad.exe in a standard user’ context.
To paunch this sample as an administrator you would need to manifest it with requireAdministrator execution level:

https://docs.microsoft.com/zh-cn/archive/blogs/winsdk/dealing-with-administrator-and-standard-users-context
*/


void CreateLowProcess(HANDLE hLowPrivToken, WCHAR * wszProcessName);


HANDLE GetLinkedToken(HANDLE hToken);


int CreateLowProcessTest(int argc, _TCHAR * argv[])
{
    BOOL fRet;
    HANDLE hToken = NULL;
    HANDLE hNewToken = NULL;
    TOKEN_LINKED_TOKEN tlt = {0};

    // Notepad is used as an example
    fRet = OpenProcessToken(GetCurrentProcess(), 
                            TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY,
                            &hToken);
    if (!fRet) {
        return 0;;
    }

    WCHAR wszProcessName[MAX_PATH] = L"C:\\Windows\\System32\\Notepad.exe";
    HANDLE hLowPrivilage = GetLinkedToken(hToken);
    if (NULL != hLowPrivilage) {
        CreateLowProcess(hLowPrivilage, wszProcessName);
    }

    return 0;
}


HANDLE GetLinkedToken(HANDLE hToken)
{
    HANDLE hProcessToken = NULL;
    HANDLE hLinkedToken = NULL;
    DWORD et = (0);
    DWORD dwReturnLength;

    if (0 == GetTokenInformation(hToken, TokenElevationType, &et, sizeof(et), &dwReturnLength)) { // return of zero indicates an error
        return NULL;
    } else { // success
        switch (et) {
        case TokenElevationTypeDefault:
            hLinkedToken = NULL;
            break;
        case TokenElevationTypeLimited:
        {
            hLinkedToken = NULL;
            break;
        }
        case TokenElevationTypeFull:
        {
            TOKEN_LINKED_TOKEN linkedToken;
            DWORD dwLength;

            linkedToken.LinkedToken = 0;

            int iRet = GetTokenInformation(hToken, TokenLinkedToken, &linkedToken, sizeof(linkedToken), &dwLength);
            if (0 == iRet) {
                hLinkedToken = NULL;
            } else {
                hLinkedToken = linkedToken.LinkedToken;
            }
        }
        } // switch (et)
    }

    if (hProcessToken) {
        CloseHandle(hProcessToken);
    }

    return hLinkedToken;
}


void CreateLowProcess(HANDLE hLowPrivToken, WCHAR * wszProcessName)
{
    PROCESS_INFORMATION ProcInfo = {0};
    STARTUPINFO StartupInfo = {0};
    DWORD dwError = 0;
    DWORD dwLength = 0;
    BOOL fRet = 0;

    fRet = CreateProcessAsUser(hLowPrivToken, NULL, wszProcessName, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcInfo);
    if (0 == fRet) {
        return;
    }

    WaitForSingleObject(ProcInfo.hProcess, INFINITE);

    if (ProcInfo.hProcess != NULL) {
        CloseHandle(ProcInfo.hProcess);
    }

    if (ProcInfo.hThread != NULL) {
        CloseHandle(ProcInfo.hThread);
    }

    return;
}


/*
If you are not able to get TokenLinkedToken token, most likely the process is not interactive.
Perhaps it has got token from following code sequence

LogonUser(member of local Administrators), ...
          LOGON32_LOGON_BATCH, .., &hToken)
ImpersonateLoggedOnUser(&hToken)
CreateProcessWithTokenW(hToken, ...)

In this case you would need to use CreateRestrictedToken to create a low privileged token from the administrative token of the parent.
You would need to specifically strip the privileges you don’t need.
*/


//////////////////////////////////////////////////////////////////////////////////////////////////
