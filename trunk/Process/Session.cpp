#include "pch.h"
#include "Session.h"


#include <NTSecAPI.h>


#pragma warning(disable:6011)


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI EnumerateLogonSessions()
/*
文件名：LsaEnumerateLogonSessions.Cpp
功能：枚举系统的登录的会话及其信息。
说明：看名字是枚举会话的，其实还有很多的用户名，会话是会重复的。
      感觉还不如枚举进程，然后收集进程的会话信息方便呢？

会话，好多人都听说过。
如网络上的，网页的链接，TCP/Http的链接等。
这里说的是系统的登录的会话。

这好像用处不大，只是在远程登录/多用户的情况下会考虑到。
但是用到了，就必须会，解决掉。

再说：会话的ID也不一定是连续的。
如登录一个会话，然后再注销，再登录看看：注销的那个会话的ID就不再了，但是后面登录的是还存在的。

http://msdn.microsoft.com/en-us/library/windows/desktop/aa378290(v=vs.85).aspx
这个页面的LSAFreeReturnBuffer也写错了，严格的说是大小写错误。

made by correy
made at 2014.06.15
*/
{
    NTSTATUS Status;
    int iRetVal = RTN_ERROR;
    ULONG LogonSessionCount;
    PLUID LogonSessionList;

    Status = LsaEnumerateLogonSessions(&LogonSessionCount, &LogonSessionList);
    if (Status != STATUS_SUCCESS) {
        DisplayNtStatus("LsaEnumerateLogonSessions", Status);
        return RTN_ERROR;
    }

    /*
    登录的会话还挺多的。
    几个用户同时登录还会重复。
    */

    ULONG i = 0;
    for (PLUID p = LogonSessionList; i < LogonSessionCount; i++) {
        //To retrieve information about a logon session, the caller must be the owner of the session or a local system administrator.
        PSECURITY_LOGON_SESSION_DATA ppLogonSessionData;
        Status = LsaGetLogonSessionData(p, &ppLogonSessionData);
        if (Status != STATUS_SUCCESS) {
            DisplayNtStatus("LsaGetLogonSessionData", Status);
            return RTN_ERROR;//break;
        }

        if (ppLogonSessionData->UserName.Length) {
            fprintf(stderr, "UserName:%wZ!\n", &ppLogonSessionData->UserName);
        }
        if (ppLogonSessionData->LogonDomain.Length) {
            fprintf(stderr, "LogonDomain:%wZ!\n", &ppLogonSessionData->LogonDomain);
        }
        if (ppLogonSessionData->AuthenticationPackage.Length) {
            fprintf(stderr, "AuthenticationPackage:%wZ!\n", &ppLogonSessionData->AuthenticationPackage);
        }

        if (ppLogonSessionData->LogonServer.Length) {
            fprintf(stderr, "LogonServer:%wZ!\n", &ppLogonSessionData->LogonServer);
        }
        if (ppLogonSessionData->DnsDomainName.Length) {
            fprintf(stderr, "DnsDomainName:%wZ!\n", &ppLogonSessionData->DnsDomainName);
        }
        if (ppLogonSessionData->Upn.Length) {
            fprintf(stderr, "Upn:%wZ!\n", &ppLogonSessionData->Upn);
        }

        if (ppLogonSessionData->LogonScript.Length) {
            fprintf(stderr, "LogonScript:%wZ!\n", &ppLogonSessionData->LogonScript);
        }
        if (ppLogonSessionData->ProfilePath.Length) {
            fprintf(stderr, "ProfilePath:%wZ!\n", &ppLogonSessionData->ProfilePath);
        }
        if (ppLogonSessionData->HomeDirectory.Length) {
            fprintf(stderr, "HomeDirectory:%wZ!\n", &ppLogonSessionData->HomeDirectory);
        }
        if (ppLogonSessionData->HomeDirectoryDrive.Length) {
            fprintf(stderr, "HomeDirectoryDrive:%wZ!\n", &ppLogonSessionData->HomeDirectoryDrive);
        }

        if (ppLogonSessionData->LogonType == Interactive) {
            fprintf(stderr, "LogonType:Interactive!\n");//有几个普通的帐户登录，这里就会显示几次。就是说可以显示多个。
        } else if (ppLogonSessionData->LogonType == Network) {
            fprintf(stderr, "LogonType:Network!\n");
        } else if (ppLogonSessionData->LogonType == Service) {
            fprintf(stderr, "LogonType:Service!\n");
        } else {
            fprintf(stderr, "LogonType:%d!\n", ppLogonSessionData->LogonType);
        }

        /*摘自：\Microsoft SDKs\Windows\v7.1A\Include\NTSecAPI.h
        //
        // Values for UserFlags.
        //

        #define LOGON_GUEST                 0x01
        #define LOGON_NOENCRYPTION          0x02
        #define LOGON_CACHED_ACCOUNT        0x04
        #define LOGON_USED_LM_PASSWORD      0x08
        #define LOGON_EXTRA_SIDS            0x20
        #define LOGON_SUBAUTH_SESSION_KEY   0x40
        #define LOGON_SERVER_TRUST_ACCOUNT  0x80
        #define LOGON_NTLMV2_ENABLED        0x100       // says DC understands NTLMv2
        #define LOGON_RESOURCE_GROUPS       0x200
        #define LOGON_PROFILE_PATH_RETURNED 0x400
        // Defined in Windows Server 2008 and above
        #define LOGON_NT_V2                 0x800   // NT response was used for validation
        #define LOGON_LM_V2                 0x1000  // LM response was used for validation
        #define LOGON_NTLM_V2               0x2000  // LM response was used to authenticate but NT response was used to derive the session key

        #if (_WIN32_WINNT >= 0x0600)

        #define LOGON_OPTIMIZED             0x4000  // this is an optimized logon
        #define LOGON_WINLOGON              0x8000  // the logon session was created for winlogon
        #define LOGON_PKINIT               0x10000  // Kerberos PKINIT extension was used to authenticate the user
        #define LOGON_NO_OPTIMIZED         0x20000  // optimized logon has been disabled for this account

        #endif
        */
        if (ppLogonSessionData->UserFlags & LOGON_WINLOGON) {
            fprintf(stderr, "UserFlags:LOGON_WINLOGON!\n");//经测试，感觉这个就是普通用户的登录。远程登录的没有试验。
        }
        //else if (ppLogonSessionData->UserFlags & LOGON_NTLMV2_ENABLED)
        //{
        //    fprintf(stderr,"UserFlags:LOGON_NTLMV2_ENABLED!\n");
        //}
        //else if (ppLogonSessionData->UserFlags == LOGON_EXTRA_SIDS)
        //{
        //    fprintf(stderr,"UserFlags:LOGON_EXTRA_SIDS!\n");
        //}
        else if (ppLogonSessionData->UserFlags == 0) {
            fprintf(stderr, "UserFlags:%d!\n", ppLogonSessionData->UserFlags);
        }

        fprintf(stderr, "Session:%d!\n", ppLogonSessionData->Session);

        /*
        还有更多新信息就不打印了，如SID，时间的转换等。
        */

        fprintf(stderr, "\n");
        fprintf(stderr, "\n");

        Status = LsaFreeReturnBuffer(ppLogonSessionData);
        if (Status != STATUS_SUCCESS) {
            DisplayNtStatus(" LSAFreeReturnBuffer", Status);
            return RTN_ERROR;//break;
        }

        p++;
    }

    Status = LsaFreeReturnBuffer(LogonSessionList);
    if (Status != STATUS_SUCCESS) {
        DisplayNtStatus(" LSAFreeReturnBuffer", Status);
        return RTN_ERROR;
    }

    iRetVal = RTN_OK;

    return iRetVal;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
void WINAPI CreateProcessWithLogon(int argc, WCHAR * argv[])
/*
runas是一个常用的命令，你会用吗？
我不会，但我要探究它的进一步的原理。
其实很简单，参见：http://msdn.microsoft.com/en-us/library/windows/desktop/ms682431(v=vs.85).aspx
注意事项：见注释。

不过要在不同的会话中运行，那就麻烦了。
普通的程序需要提权。
而进程的权限是继承自用户的，所以还要给用户分派权限。
具体的设置可以参考本地安全策略：http://www.microsoft.com/technet/prodtechnol/WindowsServer2003/Library/IIS/08bc7712-548c-4308-a49c-d551a4b5e245.mspx?mfr=true。
注意：这些设置还要重启后生效。
所以常用的办法是用服务，服务默认是具有这些权限的。再次郑重说明：不用服务也是可以的。
正好这有解决了服务在Vista及以后系统的编程交互（如：弹出对话框）的问题。

提示：命令行输入空的参数，可以使用双引号，也就是这两个引号之间没有内容，包括空格。

made by correy
made at 2014.05.17

https://docs.microsoft.com/zh-cn/windows/win32/api/winbase/nf-winbase-createprocesswithlogonw?redirectedfrom=MSDN
*/
{
    DWORD     dwSize;
    HANDLE    hToken;
    LPVOID    lpvEnv;
    PROCESS_INFORMATION pi = {0};
    STARTUPINFO         si = {0};
    WCHAR               szUserProfile[256] = L"";

    si.cb = sizeof(STARTUPINFO);

    if (argc != 4) {
        wprintf(L"Usage: %s [user@domain] [password] [cmd]", argv[0]);
        wprintf(L"\n\n");
        return;
    }

    // TO DO: change NULL to '.' to use local account database
    ////必须有密码，不然失败。
    if (!LogonUser(argv[1], NULL, argv[2], LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken))
        DisplayError(L"LogonUser");

    ////得到的内容其实就是：The environment block is an array of null-terminated Unicode strings. The list ends with two nulls (\0\0).
    if (!CreateEnvironmentBlock(&lpvEnv, hToken, TRUE))
        DisplayError(L"CreateEnvironmentBlock");

    dwSize = sizeof(szUserProfile) / sizeof(WCHAR);

    //获取用户的文件夹。
    //用户必须登录运行过一次，不必此时这个用户运行着。
    if (!GetUserProfileDirectory(hToken, szUserProfile, &dwSize))
        DisplayError(L"GetUserProfileDirectory");

    // TO DO: change NULL to '.' to use local account database
    if (!CreateProcessWithLogonW(argv[1], NULL, argv[2],
                                 LOGON_WITH_PROFILE, NULL, argv[3],
                                 CREATE_UNICODE_ENVIRONMENT, lpvEnv, szUserProfile,
                                 &si, &pi))
        DisplayError(L"CreateProcessWithLogonW");

    if (!DestroyEnvironmentBlock(lpvEnv))
        DisplayError(L"DestroyEnvironmentBlock");

    CloseHandle(hToken);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


VOID FreeLogonSID(PSID * ppsid)
//Free the buffer for the logon SID.
{
    HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
}


EXTERN_C
__declspec(dllexport)
BOOL WINAPI GetLogonSID(HANDLE hToken, PSID * ppsid)
/*
Getting the Logon SID in C++
(Get the SID for the client's logon session.)
11/17/2013

A logon security identifier (SID) identifies the logon session associated with an access token.
A typical use of a logon SID is in an ACE that allows access for the duration of a client's logon session.
For example, a Windows service can use the LogonUser function to start a new logon session.
The LogonUser function returns an access token from which the service can extract the logon SID.
The service can then use the SID in an ACE that allows the client's logon session to access the interactive window station and desktop.

The following example gets the logon SID from an access token.
It uses the GetTokenInformation function to fill a TOKEN_GROUPS buffer with an array of the group SIDs from an access token.
This array includes the logon SID, which is identified by the SE_GROUP_LOGON_ID attribute.
The example function allocates a buffer for the logon SID;
it is the caller's responsibility to free the buffer.

The following function frees the buffer allocated by the GetLogonSID example function.

VOID FreeLogonSID (PSID *ppsid)
{
    HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
}

https://docs.microsoft.com/en-us/previous-versions//aa446670(v=vs.85)
https://msdn.microsoft.com/en-us/library/aa446670(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/aa446670(v=vs.85).aspx
*/
{
    BOOL bSuccess = FALSE;
    DWORD dwIndex;
    DWORD dwLength = 0;
    PTOKEN_GROUPS ptg = NULL;

    // Verify the parameter passed in is not NULL.
    if (NULL == ppsid)
        goto Cleanup;

    // Get required buffer size and allocate the TOKEN_GROUPS buffer.
    if (!GetTokenInformation(
        hToken,         // handle to the access token
        TokenGroups,    // get information about the token's groups 
        (LPVOID)ptg,   // pointer to TOKEN_GROUPS buffer
        0,              // size of buffer
        &dwLength       // receives required buffer size
    )) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            goto Cleanup;

        ptg = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength);
        if (ptg == NULL)
            goto Cleanup;
    }

    // Get the token group information from the access token.
    if (!GetTokenInformation(
        hToken,         // handle to the access token
        TokenGroups,    // get information about the token's groups 
        (LPVOID)ptg,   // pointer to TOKEN_GROUPS buffer
        dwLength,       // size of buffer
        &dwLength       // receives required buffer size
    )) {
        goto Cleanup;
    }

    // Loop through the groups to find the logon SID.
    for (dwIndex = 0; dwIndex < ptg->GroupCount; dwIndex++)
        if ((ptg->Groups[dwIndex].Attributes & SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID) {
            // Found the logon SID; make a copy of it.
            dwLength = GetLengthSid(ptg->Groups[dwIndex].Sid);
            *ppsid = (PSID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength);
            if (*ppsid == NULL)
                goto Cleanup;
            if (!CopySid(dwLength, *ppsid, ptg->Groups[dwIndex].Sid)) {
                HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
                goto Cleanup;
            }
            break;
        }

    bSuccess = TRUE;

Cleanup:
    // Free the buffer for the token groups.
    if (ptg != NULL)
        HeapFree(GetProcessHeap(), 0, (LPVOID)ptg);

    return bSuccess;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
BOOL WINAPI IsRemoteSession(void)
/*
The following example shows a function that 
returns TRUE if the application is running in a remote session and 
FALSE if the application is running on the console.

You should not use GetSystemMetrics(SM_REMOTESESSION) to determine if your application is running in a remote session in Windows 8 and later or 
Windows Server 2012 and later if the remote session may also be using the RemoteFX vGPU improvements to the Microsoft Remote Display Protocol (RDP). 
In this case, GetSystemMetrics(SM_REMOTESESSION) will identify the remote session as a local session.

https://docs.microsoft.com/en-us/windows/win32/termserv/detecting-the-terminal-services-environment
*/
{
    return GetSystemMetrics(SM_REMOTESESSION);
}


#define TERMINAL_SERVER_KEY _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\")
#define GLASS_SESSION_ID    _T("GlassSessionId")


EXTERN_C
__declspec(dllexport)
BOOL WINAPI IsCurrentSessionRemoteable()
/*
Your application can check the following registry key to determine whether the session is a remote session that uses RemoteFX vGPU. 
If a local session exists, this registry key provides the ID of the local session.

HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Terminal Server\GlassSessionId

If the ID of the current session in which the application is running is the same as in the registry key, 
the application is running in a local session. Sessions identified as remote session in this way include remote sessions that use RemoteFX vGPU. 
The following sample code demonstrates this.

https://docs.microsoft.com/en-us/windows/win32/termserv/detecting-the-terminal-services-environment
*/
{
    BOOL fIsRemoteable = FALSE;

    if (GetSystemMetrics(SM_REMOTESESSION)) {
        fIsRemoteable = TRUE;
    } else {
        HKEY hRegKey = NULL;
        LONG lResult;

        lResult = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TERMINAL_SERVER_KEY,
            0, // ulOptions
            KEY_READ,
            &hRegKey);
        if (lResult == ERROR_SUCCESS) {
            DWORD dwGlassSessionId;
            DWORD cbGlassSessionId = sizeof(dwGlassSessionId);
            DWORD dwType;

            lResult = RegQueryValueEx(
                hRegKey,
                GLASS_SESSION_ID,
                NULL, // lpReserved
                &dwType,
                (BYTE *)&dwGlassSessionId,
                &cbGlassSessionId);
            if (lResult == ERROR_SUCCESS) {
                DWORD dwCurrentSessionId;

                if (ProcessIdToSessionId(GetCurrentProcessId(), &dwCurrentSessionId)) {
                    fIsRemoteable = (dwCurrentSessionId != dwGlassSessionId);
                }
            }
        }

        if (hRegKey) {
            RegCloseKey(hRegKey);
        }
    }

    return fIsRemoteable;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
