#include "pch.h"
#include "User.h"

#include <LsaLookup.h>
#include <NTSecAPI.h>

#include <ntsecapi.h>

#pragma warning(disable:6011)
#pragma warning(disable:6387)
#pragma warning(disable:4996)
#pragma warning(disable:28159)
#pragma warning(disable:26451)


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
BOOL WINAPI IsUserAdmin(VOID)
/*++
Routine Description: This routine returns TRUE if the caller's
process is a member of the Administrators local group. Caller is NOT
expected to be impersonating anyone and is expected to be able to
open its own process and process token.
Arguments: None.
Return Value:
   TRUE - Caller has Administrators local group.
   FALSE - Caller does not have Administrators local group. --

Examples
The following example shows checking a token for membership in the Administrators local group.

注意还有个函数叫：IsUserAnAdmin

看到一些判断是不是管理员的小代码，没有细看。方法很多。
原来一个函数就能实现，关键在于知识，知识面，技术？
下面这两个是取自msdn，觉得最正宗的还是用微软的。
再详细的注释下：
是判断当前用户是不是管理员的组的成员，并不一定是administrator用户.
在win 7下非administrator的组的用户，以administrator组的成员的权限运行，需要输入密码的情况，别当另论。

https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership
https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-isuseranadmin
*/
{
    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    b = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
    if (b) {
        if (!CheckTokenMembership(NULL, AdministratorsGroup, &b)) {
            b = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    return(b);
}


EXTERN_C
__declspec(dllexport)
BOOL WINAPI IsUserAnSystem()
//BOOL WINAPI CurrentUserIsLocalSystem()
/*
功能：判断进程是否运行在NT AUTHORITY\SYSTEM用户下。

https://www.cnblogs.com/idebug/p/11124664.html
*/
{
    BOOL bIsLocalSystem = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID psidLocalSystem;

    BOOL fSuccess = ::AllocateAndInitializeSid(&ntAuthority, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &psidLocalSystem);
    if (fSuccess) {
        fSuccess = ::CheckTokenMembership(0, psidLocalSystem, &bIsLocalSystem);
        ::FreeSid(psidLocalSystem);
    }

    return bIsLocalSystem;
}


EXTERN_C
__declspec(dllexport)
BOOL WINAPI IsCurrentUserLocalAdministrator(void)
/*
IsCurrentUserLocalAdministrator ()

This function checks the token of the calling thread to see if the caller
belongs to the Administrators group.

Return Value:
   TRUE if the caller is an administrator on the local machine.
   Otherwise, FALSE.

概要
若要确定是否在本地管理员帐户下运行一个线程，您必须检查与线程相关联的访问令牌。本文介绍如何执行此操作。

Windows 2000 及更高版本，您可以使用CheckTokenMembership() API 而不是在这篇文章中介绍的步骤。
有关其他信息，请参阅 Microsoft 平台 SDK 文档。
详细信息
默认情况下，与线程相关联的标记是进程的其包含。"用户上下文"被取代任何直接连接到该线程的标识。
因此，要确定线程的用户上下文，您应首先尝试获取具有OpenThreadToken函数的线程令牌。
如果此方法失败并且时出错函数报告 ERROR_NO_TOKEN，然后您可以使用OpenProcessToken函数的进程令牌。

获取当前用户的令牌之后，可以使用访问权限检查功能来检测用户是否为管理员。若要执行此操作，请按照下列步骤操作：
通过使用AllocateAndInitializeSid函数创建本地管理员组的安全标识符(SID)。
构建新的安全描述符(SD) 的自由访问控制列表(DACL)，其中包含访问控制项(ACE) 的管理员组的 SID。
调用与当前用户和新构造的 SD，来检测用户是否为管理员令牌访问权限检查。
下面的代码示例使用来测试是否当前线程运行本地计算机上的管理员的用户作为本文中前面提到的函数。
示例代码

https://support.microsoft.com/zh-cn/kb/118626
*/
{
    BOOL   fReturn = FALSE;
    DWORD  dwStatus;
    DWORD  dwAccessMask;
    DWORD  dwAccessDesired;
    DWORD  dwACLSize;
    DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
    PACL   pACL = NULL;
    PSID   psidAdmin = NULL;

    HANDLE hToken = NULL;
    HANDLE hImpersonationToken = NULL;

    PRIVILEGE_SET   ps;
    GENERIC_MAPPING GenericMapping;

    PSECURITY_DESCRIPTOR     psdAdmin = NULL;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

    /*
       Determine if the current thread is running as a user that is a member of
       the local admins group.  To do this, create a security descriptor that
       has a DACL which has an ACE that allows only local aministrators access.
       Then, call AccessCheck with the current thread's token and the security
       descriptor.  It will say whether the user could access an object if it
       had that security descriptor.  Note: you do not need to actually create
       the object.  Just checking access against the security descriptor alone
       will be sufficient.
    */
    const DWORD ACCESS_READ = 1;
    const DWORD ACCESS_WRITE = 2;

    __try {
        /*
           AccessCheck() requires an impersonation token.  We first get a primary
             token and then create a duplicate impersonation token.  The
             impersonation token is not actually assigned to the thread, but is
             used in the call to AccessCheck.  Thus, this function itself never
             impersonates, but does use the identity of the thread.  If the thread
             was impersonating already, this function uses that impersonation context.
          */
        if (!OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE | TOKEN_QUERY, TRUE, &hToken)) {
            if (GetLastError() != ERROR_NO_TOKEN)
                __leave;

            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_QUERY, &hToken))
                __leave;
        }

        if (!DuplicateToken(hToken, SecurityImpersonation, &hImpersonationToken))
            __leave;

        /*
          Create the binary representation of the well-known SID that
          represents the local administrators group.  Then create the security
            descriptor and DACL with an ACE that allows only local admins access.
            After that, perform the access check.  This will determine whether
            the current user is a local admin.
          */
        if (!AllocateAndInitializeSid(&SystemSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdmin))
            __leave;

        psdAdmin = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (psdAdmin == NULL)
            __leave;

        if (!InitializeSecurityDescriptor(psdAdmin, SECURITY_DESCRIPTOR_REVISION))
            __leave;

        // Compute size needed for the ACL.
        dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psidAdmin) - sizeof(DWORD);
        pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
        if (pACL == NULL)
            __leave;

        if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
            __leave;

        dwAccessMask = ACCESS_READ | ACCESS_WRITE;
        if (!AddAccessAllowedAce(pACL, ACL_REVISION2, dwAccessMask, psidAdmin))
            __leave;

        if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
            __leave;

        /*
           AccessCheck validates a security descriptor somewhat; set the group
             and owner so that enough of the security descriptor is filled out to
             make AccessCheck happy.
          */
        SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
        SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);

        if (!IsValidSecurityDescriptor(psdAdmin))
            __leave;

        dwAccessDesired = ACCESS_READ;

        /*
           Initialize GenericMapping structure even though you do not use generic rights.
        */
        GenericMapping.GenericRead = ACCESS_READ;
        GenericMapping.GenericWrite = ACCESS_WRITE;
        GenericMapping.GenericExecute = 0;
        GenericMapping.GenericAll = ACCESS_READ | ACCESS_WRITE;

        if (!AccessCheck(psdAdmin, hImpersonationToken, dwAccessDesired, &GenericMapping, &ps, &dwStructureSize, &dwStatus, &fReturn)) {
            fReturn = FALSE;
            __leave;
        }
    } __finally {
        // Clean up.
        if (pACL) LocalFree(pACL);
        if (psdAdmin) LocalFree(psdAdmin);
        if (psidAdmin) FreeSid(psidAdmin);
        if (hImpersonationToken) CloseHandle(hImpersonationToken);
        if (hToken) CloseHandle(hToken);
    }

    return fReturn;
}


void TestIsCurrentUserLocalAdministrator()
{
    if (IsCurrentUserLocalAdministrator())
        printf("You are an administrator\n");
    else
        printf("You are not an administrator\n");
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI EnumCred()
/*
功能：获取本机的所有的凭据（包括用户名和密码），而且不需要输入计算机用户的密码。
*/
{
    DWORD Count = 0;
    PCREDENTIALW * Credential;
    BOOL ret = CredEnumerateW(NULL, 0, &Count, &Credential);
    _ASSERTE(ret);

    printf("Count:%d.\n", Count);
    printf("\n");

    for (DWORD x = 0; x < Count; x++) {
        printf("第%d的信息：\n", x + 1);

        printf("TargetName:%ls.\n", Credential[x]->TargetName);

        if (lstrlen(Credential[x]->TargetAlias)) {
            printf("TargetAlias:%ls.\n", Credential[x]->TargetAlias);
        }

        if (lstrlen(Credential[x]->Comment)) {
            printf("Comment:%ls.\n", Credential[x]->Comment);
        }

        if (Credential[x]->Attributes) {
            printf("Keyword:%ls.\n", Credential[x]->Attributes->Keyword);
            printf("Value:%ls.\n", (LPWSTR)Credential[x]->Attributes->Value);
        }

        printf("UserName:%ls.\n", Credential[x]->UserName);
        printf("CredentialBlobSize(字节，换字符请除以二):%d, CredentialBlob(密码):%ls.\n",
               Credential[x]->CredentialBlobSize,
               (LPWSTR)Credential[x]->CredentialBlob);
        printf("\n");
    }

    CredFree(Credential);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


BOOL GetAccountSid(LPTSTR SystemName, LPTSTR AccountName, PSID * Sid)
/*++
This function attempts to obtain a SID representing the supplied account on the supplied system.

If the function succeeds, the return value is TRUE.
A buffer is allocated which contains the SID representing the supplied account.
This buffer should be freed when it is no longer needed by calling HeapFree(GetProcessHeap(), 0, buffer)

If the function fails, the return value is FALSE. Call GetLastError() to obtain extended error information.
--*/
{
    LPTSTR ReferencedDomain = NULL;
    LPTSTR TempReferencedDomain = NULL;
    LPTSTR TempSid = NULL;
    DWORD cbSid = 128;    // initial allocation attempt
    DWORD cchReferencedDomain = 16; // initial allocation size
    SID_NAME_USE peUse;
    BOOL bSuccess = FALSE; // assume this function will fail

    __try {
        *Sid = (PSID)HeapAlloc(GetProcessHeap(), 0, cbSid);// initial memory allocations
        if (*Sid == NULL) __leave;

        ReferencedDomain = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, cchReferencedDomain * sizeof(TCHAR));
        if (ReferencedDomain == NULL) __leave;

        // Obtain the SID of the specified account on the specified system.
        while (!LookupAccountName(
            SystemName,         // machine to lookup account on
            AccountName,        // account to lookup
            *Sid,               // SID of interest
            &cbSid,             // size of SID
            ReferencedDomain,   // domain account was found on
            &cchReferencedDomain, &peUse)) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                TempSid = (LPTSTR)HeapReAlloc(GetProcessHeap(), 0, *Sid, cbSid);// reallocate memory
                if (TempSid == NULL) __leave;
                *Sid = TempSid;

                TempReferencedDomain = (LPTSTR)HeapReAlloc(GetProcessHeap(), 0, ReferencedDomain, cchReferencedDomain * sizeof(TCHAR));

                if (TempReferencedDomain == NULL) __leave;
                ReferencedDomain = TempReferencedDomain;
            } else __leave;
        }

        bSuccess = TRUE;// Indicate success.
    } // try
    __finally {// Cleanup and indicate failure, if appropriate.
        HeapFree(GetProcessHeap(), 0, ReferencedDomain);
        if (!bSuccess) {
            if (*Sid != NULL) {
                HeapFree(GetProcessHeap(), 0, *Sid);
                *Sid = NULL;
            }
        }
    } // finally

    return bSuccess;
}


void InitLsaString(PLSA_UNICODE_STRING LsaString, LPWSTR String)
{
    DWORD StringLength;

    if (String == NULL) {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }

    StringLength = lstrlenW(String);
    LsaString->Buffer = String;
    LsaString->Length = (USHORT)StringLength * sizeof(WCHAR);
    LsaString->MaximumLength = (USHORT)(StringLength + 1) * sizeof(WCHAR);
}


NTSTATUS OpenPolicy(LPWSTR ServerName, DWORD DesiredAccess, PLSA_HANDLE PolicyHandle)
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server;

    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));// Always initialize the object attributes to all zeroes.
    if (ServerName != NULL) {// Make a LSA_UNICODE_STRING out of the LPWSTR passed in
        InitLsaString(&ServerString, ServerName);
        Server = &ServerString;
    } else {
        Server = NULL;
    }

    return LsaOpenPolicy(Server, &ObjectAttributes, DesiredAccess, PolicyHandle);// Attempt to open the policy.
}


EXTERN_C
__declspec(dllexport)
int WINAPI EnumerateAccountRights(int argc, char * argv[])
/*
功能：枚举用户或者用户组的特权信息，如：administrator或者administrators.

进程继承用户的权限。
程序员大多知道给进程提权。
但是有的进程的某些权限是不易搞到的。
如：Note that your process must have the SE_ASSIGNPRIMARYTOKEN_NAME and
SE_INCREASE_QUOTA_NAME privileges for successful execution of CreateProcessAsUser.
解决办法是用LsaAddAccountRights/LsaRemoveAccountRights给用户添加/删除权限。
然后再用常用的办法提权。
可是这要求注销/重启系统。

记得有个办法是不用重启的，但是忘了方法和链接了。
所以谨记此文。
此文的功能是枚举用户或者用户组的权限的。
此文修改自：Microsoft SDKs\Windows\v6.0\Samples\Security\LSAPolicy\LsaPrivs\LsaPrivs.c。

注意：Unlike privileges, however, account rights are not supported by the LookupPrivilegeValue and LookupPrivilegeName functions.
用户的特权很重要，如：
修改固件环境值。
加载驱动。
启动服务，服务一般不是普通用户运行的。
以操作系统方式执行。
更多信息，请看：
http://msdn.microsoft.com/en-us/library/windows/desktop/bb545671(v=vs.85).aspx  Account Rights Constants
http://msdn.microsoft.com/en-us/library/windows/desktop/bb530716(v=vs.85).aspx  Privilege Constants
http://www.microsoft.com/technet/prodtechnol/WindowsServer2003/Library/IIS/08bc7712-548c-4308-a49c-d551a4b5e245.mspx?mfr=true
等等。

更多的功能请看：
LsaEnumerateAccountsWithUserRight
LsaEnumerateTrustedDomains
LsaEnumerateTrustedDomainsEx
NetEnumerateServiceAccounts （Windows 7/Windows Server 2008 R2）

made by correy
made at 2014.06.14
*/
{
    LSA_HANDLE PolicyHandle;
    WCHAR wComputerName[256] = L"";   // static machine name buffer
    TCHAR AccountName[256];         // static account name buffer
    PSID pSid;
    NTSTATUS Status;
    int iRetVal = RTN_ERROR;          // assume error from main

    if (argc == 1) {
        fprintf(stderr, "Usage: %s <Account> [TargetMachine]\n", argv[0]);
        return RTN_USAGE;
    }

    // Pick up account name on argv[1].
    // Assumes source is ANSI. Resultant string is ANSI or Unicode
    //可以是用户也可以是用户组，如：administrator或者administrators.
    _snwprintf_s(AccountName, 256, 255, TEXT("%hS"), argv[1]);

    // Pick up machine name on argv[2], if appropriate
    // assumes source is ANSI. Resultant string is Unicode.
    if (argc == 3) _snwprintf_s(wComputerName, 256, 255, L"%hS", argv[2]);//这个参数可以无。

    // Open the policy on the target machine. 
    Status = OpenPolicy(
        wComputerName,      // target machine
        POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES,
        &PolicyHandle       // resultant policy handle
    );
    if (Status != STATUS_SUCCESS) {
        DisplayNtStatus("OpenPolicy", Status);
        return RTN_ERROR;
    }

    // Obtain the SID of the user/group.
    // Note that we could target a specific machine, but we don't.
    // Specifying NULL for target machine searches for the SID in the following order: 
    // well-known, Built-in and local, primary domain,trusted domains.
    if (GetAccountSid(
        NULL,       // default lookup logic
        AccountName,// account to obtain SID
        &pSid       // buffer to allocate to contain resultant SID
    )) {
        //这几行代码是自己的。
        PLSA_UNICODE_STRING UserRights;
        ULONG CountOfRights;
        Status = LsaEnumerateAccountRights(PolicyHandle, pSid, &UserRights, &CountOfRights);
        if (Status != STATUS_SUCCESS) {
            DisplayNtStatus("LsaEnumerateAccountRights", Status);
            return RTN_ERROR;
        }

        ULONG i = 0;
        for (PLSA_UNICODE_STRING plun = UserRights; i < CountOfRights; i++) {
            fprintf(stderr, "%ws!\n", plun->Buffer);
            plun++;
        }

        Status = LsaFreeMemory(&UserRights);
        if (Status != STATUS_SUCCESS) {
            DisplayNtStatus("LsaFreeMemory", Status);
            return RTN_ERROR;
        }

        iRetVal = RTN_OK;
    } else {
        DisplayWinError("GetAccountSid", GetLastError());// Error obtaining SID.
    }

    LsaClose(PolicyHandle);// Close the policy handle.
    if (pSid != NULL) HeapFree(GetProcessHeap(), 0, pSid);// Free memory allocated for SID.
    return iRetVal;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
BOOL WINAPI GetCurrentUserAndDomain(PTSTR szUser, PDWORD pcchUser, PTSTR szDomain, PDWORD pcchDomain)
/*
//  FUNCTION:     GetCurrentUserAndDomain - This function looks up the user name and
                  domain name for the user account associated with the calling thread.
//  PARAMETERS:   szUser - a buffer that receives the user name
//                pcchUser - the size, in characters, of szUser
//                szDomain - a buffer that receives the domain name
//                pcchDomain - the size, in characters, of szDomain
//  RETURN VALUE: TRUE if the function succeeds.
                  Otherwise, FALSE and GetLastError() will return the failure reason.
                  If either of the supplied buffers are too small,
                  GetLastError() will return ERROR_INSUFFICIENT_BUFFER and pcchUser and
                  pcchDomain will be adjusted to reflect the required buffer sizes.

用法示例：
    wchar_t szUser[MAX_PATH] = {0};//估计最大值不是这个。
    wchar_t szDomain[MAX_PATH] = {0};//估计最大值不是这个。这个好像和计算机名一样。
    DWORD d = MAX_PATH;
    bool b = GetCurrentUserAndDomain(szUser, &d, szDomain, &d);

made by correy
made at 2013.05.03

本文摘自：// http://support.microsoft.com/kb/111544/zh-cn
*/
{
    BOOL         fSuccess = FALSE;
    HANDLE       hToken = NULL;
    PTOKEN_USER  ptiUser = NULL;
    DWORD        cbti = 0;
    SID_NAME_USE snu;

    __try {
        // Get the calling thread's access token.
        if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken)) {
            if (GetLastError() != ERROR_NO_TOKEN) {
                __leave;
            }

            // Retry against process token if no thread token exists.
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
                __leave;
            }
        }

        // Obtain the size of the user information in the token.    
        if (GetTokenInformation(hToken, TokenUser, NULL, 0, &cbti)) {
            __leave;// Call should have failed due to zero-length buffer.   
        } else {// Call should have failed due to zero-length buffer.
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                __leave;
            }
        }

        // Allocate buffer for user information in the token.
        ptiUser = (PTOKEN_USER)HeapAlloc(GetProcessHeap(), 0, cbti);
        if (!ptiUser) {
            __leave;
        }

        // Retrieve the user information from the token.
        if (!GetTokenInformation(hToken, TokenUser, ptiUser, cbti, &cbti)) {
            __leave;
        }

        // Retrieve user name and domain name based on user's SID.
        if (!LookupAccountSid(NULL, ptiUser->User.Sid, szUser, pcchUser, szDomain, pcchDomain, &snu)) {
            __leave;
        }

        fSuccess = TRUE;
    } __finally {// Free resources.        
        if (hToken) {
            CloseHandle(hToken);
        }

        if (ptiUser) {
            HeapFree(GetProcessHeap(), 0, ptiUser);
        }
    }

    return fSuccess;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
HRESULT WINAPI GetSid(LPCWSTR wszAccName, PSID * ppSid)
/*
http://msdn.microsoft.com/en-us/library/windows/desktop/ms707085(v=vs.85).aspx
https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms707085(v=vs.85)?redirectedfrom=MSDN
*/
{
    if (wszAccName == NULL || ppSid == NULL) {// Validate the input parameters.
        return MQ_ERROR_INVALID_PARAMETER;
    }

    // Create buffers that may be large enough.If a buffer is too small, the count parameter will be set to the size needed.
    const DWORD INITIAL_SIZE = 32;
    DWORD cbSid = 0;
    DWORD dwSidBufferSize = INITIAL_SIZE;
    DWORD cchDomainName = 0;
    DWORD dwDomainBufferSize = INITIAL_SIZE;
    WCHAR * wszDomainName = NULL;
    SID_NAME_USE eSidType;
    DWORD dwErrorCode = 0;
    HRESULT hr = MQ_OK;

    *ppSid = (PSID) new BYTE[dwSidBufferSize];// Create buffers for the SID and the domain name.
    if (*ppSid == NULL) {
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
    memset(*ppSid, 0, dwSidBufferSize);
    wszDomainName = new WCHAR[dwDomainBufferSize];
    if (wszDomainName == NULL) {
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
    memset(wszDomainName, 0, dwDomainBufferSize * sizeof(WCHAR));

    for (; ; )// Obtain the SID for the account name passed.
    {   // Set the count variables to the buffer sizes and retrieve the SID.
        cbSid = dwSidBufferSize;
        cchDomainName = dwDomainBufferSize;
        if (LookupAccountNameW(
            NULL,            // Computer name. NULL for the local computer
            wszAccName,
            *ppSid,          // Pointer to the SID buffer. Use NULL to get the size needed,
            &cbSid,          // Size of the SID buffer needed.
            wszDomainName,   // wszDomainName,//这个还能获取域名.
            &cchDomainName,
            &eSidType)) //其实这个函数就是返回sid和域名用的别的没啥,不要多想,下面的是垃圾,加上更完美.
        {
            if (IsValidSid(*ppSid) == FALSE) {
                wprintf(L"The SID for %s is invalid.\n", wszAccName);
                dwErrorCode = MQ_ERROR;
            }
            break;
        }
        dwErrorCode = GetLastError();

        if (dwErrorCode == ERROR_INSUFFICIENT_BUFFER) // Check if one of the buffers was too small.
        {
            if (cbSid > dwSidBufferSize) {   // Reallocate memory for the SID buffer.
                wprintf(L"The SID buffer was too small. It will be reallocated.\n");
                FreeSid(*ppSid);
                *ppSid = (PSID) new BYTE[cbSid];
                if (*ppSid == NULL) {
                    return MQ_ERROR_INSUFFICIENT_RESOURCES;
                }
                memset(*ppSid, 0, cbSid);
                dwSidBufferSize = cbSid;
            }
            if (cchDomainName > dwDomainBufferSize) {   // Reallocate memory for the domain name buffer.
                wprintf(L"The domain name buffer was too small. It will be reallocated.\n");
                delete[] wszDomainName;
                wszDomainName = new WCHAR[cchDomainName];
                if (wszDomainName == NULL) {
                    return MQ_ERROR_INSUFFICIENT_RESOURCES;
                }
                memset(wszDomainName, 0, cchDomainName * sizeof(WCHAR));
                dwDomainBufferSize = cchDomainName;
            }
        } else {
            wprintf(L"LookupAccountNameW failed. GetLastError returned: %d\n", dwErrorCode);
            hr = HRESULT_FROM_WIN32(dwErrorCode);
            break;
        }
    }

    delete[] wszDomainName;
    return hr;
}


EXTERN_C
__declspec(dllexport)
int WINAPI GetCurrentSid()
/*
参考:Microsoft SDKs\Windows\v7.1\Samples\security\authorization\textsid这个工程.
获取当前用户(进程的)SID更简单.其实也就这么简单.
made at 2013.10.10
*/
{
    HANDLE hToken;
    BYTE buf[MY_BUFSIZE];
    PTOKEN_USER ptgUser = (PTOKEN_USER)buf;
    DWORD cbBuffer = MY_BUFSIZE;
    BOOL bSuccess;

    // obtain current process token
    if (!OpenProcessToken(
        GetCurrentProcess(), // target current process
        TOKEN_QUERY,         // TOKEN_QUERY access
        &hToken              // resultant hToken
    )) {
        return 1;
    }

    // obtain user identified by current process' access token
    bSuccess = GetTokenInformation(
        hToken,    // identifies access token
        TokenUser, // TokenUser info type
        ptgUser,   // retrieved info buffer
        cbBuffer,  // size of buffer passed-in
        &cbBuffer  // required buffer size
    );
    CloseHandle(hToken);
    if (!bSuccess) {
        return 1;
    }

    LPWSTR lpSid = NULL;
    ConvertSidToStringSid(ptgUser->User.Sid, &lpSid);

    //这时已经获取到了,可以查看了.

    LocalFree(lpSid);

    return 1;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


BOOL IsRunAsAdmin()
//摘自：UAC self-elevation (CppUACSelfElevation)
//   PURPOSE: The function checks whether the current process is run as administrator. 
//   In other words, it dictates whether the primary access token of the process belongs to user account that is a member of the local Administrators group and it is elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process belongs to user account that is a member of the local Administrators group and it is elevated. 
//   Returns FALSE if the token does not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception which contains the Win32 error code of the failure.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsRunAsAdmin())
//             wprintf (L"Process is run as administrator\n");
//         else
//             wprintf (L"Process is not run as administrator\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsRunAsAdmin failed w/err %lu\n", dwError);
//     }
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Determine whether the SID of administrators group is enabled in the primary access token of the process.
    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin)) {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (pAdministratorsGroup) {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError) {
        throw dwError;
    }

    return fIsRunAsAdmin;
}


BOOL IsUserInAdminGroup()
//摘自：UAC self-elevation (CppUACSelfElevation)
//   PURPOSE: The function checks whether the primary access token of the process belongs to user account that is a member of the local Administrators group,
//            even if it currently is not elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process belongs to user account that is a member of the local Administrators group. 
//   Returns FALSE if the token does not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception which contains the Win32 error code of the failure.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsUserInAdminGroup())
//             wprintf (L"User is a member of the Administrators group\n");
//         else
//             wprintf (L"User is not a member of the Administrators group\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsUserInAdminGroup failed w/err %lu\n", dwError);
//     }
{
    BOOL fInAdminGroup = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    HANDLE hTokenToCheck = NULL;
    DWORD cbSize = 0;
    OSVERSIONINFO osver = {sizeof(osver)};

    // Open the primary access token of the process for query and duplicate.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &hToken)) {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Determine whether system is running Windows Vista or later operating systems (major version >= 6) because they support linked tokens,
    // but previous versions (major version < 6) do not.
    if (!GetVersionEx(&osver)) {
        dwError = GetLastError();
        goto Cleanup;
    }

    if (osver.dwMajorVersion >= 6) {
        // Running Windows Vista or later (major version >= 6). 
        // Determine token type: limited, elevated, or default. 
        TOKEN_ELEVATION_TYPE elevType;
        if (!GetTokenInformation(hToken, TokenElevationType, &elevType, sizeof(elevType), &cbSize)) {
            dwError = GetLastError();
            goto Cleanup;
        }

        // If limited, get the linked elevated token for further check.
        if (TokenElevationTypeLimited == elevType) {
            if (!GetTokenInformation(hToken, TokenLinkedToken, &hTokenToCheck, sizeof(hTokenToCheck), &cbSize)) {
                dwError = GetLastError();
                goto Cleanup;
            }
        }
    }

    // CheckTokenMembership requires an impersonation token. 
    // If we just got a linked token, it already is an impersonation token.  
    // If we did not get a linked token, duplicate the original into an impersonation token for CheckTokenMembership.
    if (!hTokenToCheck) {
        if (!DuplicateToken(hToken, SecurityIdentification, &hTokenToCheck)) {
            dwError = GetLastError();
            goto Cleanup;
        }
    }

    // Create the SID corresponding to the Administrators group.
    BYTE adminSID[SECURITY_MAX_SID_SIZE];
    cbSize = sizeof(adminSID);
    if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID, &cbSize)) {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Check if the token to be checked contains admin SID.
    // http://msdn.microsoft.com/en-us/library/aa379596(VS.85).aspx:
    // To determine whether a SID is enabled in a token, that is, 
    // whether it has the SE_GROUP_ENABLED attribute, call CheckTokenMembership.
    if (!CheckTokenMembership(hTokenToCheck, &adminSID, &fInAdminGroup)) {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken) {
        CloseHandle(hToken);
        hToken = NULL;
    }

    if (hTokenToCheck) {
        CloseHandle(hTokenToCheck);
        hTokenToCheck = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError) {
        throw dwError;
    }

    return fInAdminGroup;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


BOOL IsEnableUAC()
/*
Determine whether the current user has opened UAC (Vista or Win7 OS)

https://titanwolf.org/Network/Articles/Article?AID=c55a10c3-265e-42cd-b520-118ca46c2c60
*/
{
    BOOL isEnableUAC = FALSE;
    OSVERSIONINFO osversioninfo;
    ZeroMemory(&osversioninfo, sizeof(osversioninfo));
    osversioninfo.dwOSVersionInfoSize = sizeof(osversioninfo);
    BOOL bSuccess = GetVersionEx(&osversioninfo);
    if (bSuccess) {
        //window vista or windows server 2008 or later operating system
        if (osversioninfo.dwMajorVersion > 5) {
            HKEY hKEY = NULL;
            DWORD dwType = REG_DWORD;
            DWORD dwEnableLUA = 0;
            DWORD dwSize = sizeof(DWORD);
            LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                       TEXT("SOFTWARE//Microsoft//Windows//CurrentVersion//Policies//System//"),
                                       0,
                                       KEY_READ,
                                       &hKEY);
            if (ERROR_SUCCESS == status) {
                status = RegQueryValueEx(hKEY, TEXT("EnableLUA"), NULL, &dwType, (BYTE *)&dwEnableLUA, &dwSize);
                if (ERROR_SUCCESS == status) {
                    isEnableUAC = (dwEnableLUA == 1) ? TRUE : FALSE;
                }
                RegCloseKey(hKEY);
            }
        }
    }

    return isEnableUAC;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


#define TARGET_SYSTEM_NAME L"mysystem"


LSA_HANDLE GetPolicyHandle()
/*
打开策略对象句柄

大多数 LSA 策略函数要求使用 策略 对象的句柄来查询或修改系统。
若要获取 策略 对象的句柄，请调用 LsaOpenPolicy 并指定要访问的系统的名称和所需的访问权限集。

应用程序所需的访问权限取决于它执行的操作。
有关每个函数所需权限的详细信息，请参阅 LSA 策略函数中该函数的说明。

如果对 LsaOpenPolicy 的调用成功，则它将为指定系统返回 策略 对象的句柄。
然后，应用程序在后续 LSA 策略函数调用中传递此句柄。 当应用程序不再需要句柄时，它应调用 LsaClose 来释放它。

下面的示例演示如何打开 策略 对象句柄。

在前面的示例中，应用程序请求策略 _ 所有 _ 访问 权限。
有关调用 LsaOpenPolicy时应用程序应该请求的权限的详细信息，请参阅应用程序将 策略 对象句柄传递到的函数的说明。

若要打开受信任域的 策略 对象的句柄，请调用 LsaCreateTrustedDomainEx (以与域) 创建新的信任关系，或调用 LsaOpenTrustedDomainByName (访问现有的受信任域) 。
这两个函数都设置一个指向 LSA _ 句柄的指针，然后您可以在后续 LSA 策略函数调用中指定该句柄。
与 LsaOpenPolicy一样，应用程序在不再需要受信任域的 策略 对象的句柄时，应调用 LsaClose 。

https://docs.microsoft.com/zh-cn/windows/win32/secmgmt/opening-a-policy-object-handle
*/
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR SystemName[] = TARGET_SYSTEM_NAME;
    USHORT SystemNameLength;
    LSA_UNICODE_STRING lusSystemName;
    NTSTATUS ntsResult;
    LSA_HANDLE lsahPolicyHandle;

    // Object attributes are reserved, so initialize to zeros.
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

    //Initialize an LSA_UNICODE_STRING to the server name.
    SystemNameLength = (USHORT)wcslen(SystemName);
    lusSystemName.Buffer = SystemName;
    lusSystemName.Length = SystemNameLength * sizeof(WCHAR);
    lusSystemName.MaximumLength = (SystemNameLength + 1) * sizeof(WCHAR);

    // Get a handle to the Policy object.
    ntsResult = LsaOpenPolicy(
        &lusSystemName,    //Name of the target system.
        &ObjectAttributes, //Object attributes.
        POLICY_ALL_ACCESS, //Desired access permissions.
        &lsahPolicyHandle  //Receives the policy handle.
    );

    if (ntsResult != STATUS_SUCCESS) {
        // An error occurred. Display it as a win32 error code.
        wprintf(L"OpenPolicy returned %lu\n", LsaNtStatusToWinError(ntsResult));
        return NULL;
    }
    return lsahPolicyHandle;
}


void AddPrivileges(PSID AccountSID, LSA_HANDLE PolicyHandle)
/*
管理帐户权限

LSA 提供一些函数，应用程序可调用这些函数来枚举或设置用户、组和本地组帐户的 特权 。

你的应用程序必须获得本地策略对象的句柄，如 打开策略对象句柄中所述，你的应用程序必须获得本地 策略对象的句柄。
此外，若要枚举或编辑帐户的权限，则必须具有该帐户的 安全标识符 (SID) 。
应用程序可以根据 名称和 Sid 间的转换中所述，查找给定了帐户名的 SID。

若要访问具有特定权限的所有帐户，请调用 LsaEnumerateAccountsWithUserRight。
此函数使用具有指定权限的所有帐户的 Sid 填充数组。

获取帐户的 SID 后，可以修改其权限。 调用 LsaAddAccountRights ，将权限添加到帐户。
如果指定的帐户不存在， LsaAddAccountRights 将创建该帐户。
若要从帐户中删除权限，请调用 LsaRemoveAccountRights。
如果从帐户中删除所有权限，则 LsaRemoveAccountRights 也会删除该帐户。

应用程序可以通过调用 LsaEnumerateAccountRights来检查当前分配给帐户的权限。
此函数填充 LSA _ UNICODE _ 字符串 结构的数组。
每个结构都包含指定帐户持有的特权的名称。

下面的示例将 SeServiceLogonRight 权限添加到帐户。
在此示例中，AccountSID 变量指定了帐户的 SID。
有关如何查找帐户 SID 的详细信息，请参阅 名称和 Sid 间的转换。

https://docs.microsoft.com/zh-cn/windows/win32/secmgmt/managing-account-permissions
*/
{
    LSA_UNICODE_STRING lucPrivilege;
    NTSTATUS ntsResult;

    // Create an LSA_UNICODE_STRING for the privilege names.
    InitLsaString(&lucPrivilege, (LPWSTR)L"SeServiceLogonRight");

    ntsResult = LsaAddAccountRights(
        PolicyHandle,  // An open policy handle.
        AccountSID,    // The target SID.
        &lucPrivilege, // The privileges.
        1              // Number of privileges.
    );
    if (ntsResult == STATUS_SUCCESS) {
        wprintf(L"Privilege added.\n");
    } else {
        wprintf(L"Privilege was not added - %lu \n", LsaNtStatusToWinError(ntsResult));
    }
}


void GetSIDInformation(LPWSTR AccountName, LSA_HANDLE PolicyHandle)
/*
名称和 Sid 间的转换

本地安全机构 (LSA) 提供在用户、组和本地组名称之间进行转换的功能，以及 (SID) 值的相应 安全标识符。
若要查找帐户名称，请调用 LsaLookupNames 函数。 此函数以 RID/域索引对的形式返回 SID。
若要以单个元素的形式获取 SID，请调用 LsaLookupNames2 函数。
若要查找 Sid，请调用 LsaLookupSids。

这些函数可以从本地系统信任的任何域转换名称和 SID 信息。

你的应用程序必须获得本地策略对象的句柄，如 打开策略对象句柄中所述，你的应用程序必须获得本地 策略对象的句柄。

下面的示例在给定帐户名称的情况下查找帐户的 SID。

在前面的示例中，函数 InitLsaString 将 unicode 字符串转换为 LSA _ unicode _ 字符串 结构。
此函数的代码在 使用 LSA Unicode 字符串中显示。

 备注

这些转换函数主要由权限编辑器用来显示 访问控制列表 (ACL) 信息。
权限编辑器应始终使用 name 或 security identifier SID 所在系统的 Policy对象调用这些函数。
这可确保在转换过程中引用正确的受信任域集。

Windows Access Control 还提供了在 Sid 和帐户名之间执行转换的函数： LookupAccountName 和 LookupAccountSid。
如果你的应用程序需要查找帐户名称或 SID，但不使用其他 LSA 策略功能，请使用 Windows Access Control 功能，而不是 LSA 策略函数。
有关这些函数的详细信息，请参阅 访问控制。

https://docs.microsoft.com/zh-cn/windows/win32/secmgmt/translating-between-names-and-sids
*/
{
    LSA_UNICODE_STRING lucName;
    PLSA_TRANSLATED_SID ltsTranslatedSID;
    PLSA_REFERENCED_DOMAIN_LIST lrdlDomainList;
    LSA_TRUST_INFORMATION myDomain;
    NTSTATUS ntsResult;
    PWCHAR DomainString = NULL;

    // Initialize an LSA_UNICODE_STRING with the name.
    InitLsaString(&lucName, AccountName);

    ntsResult = LsaLookupNames(
        PolicyHandle,     // handle to a Policy object
        1,                // number of names to look up
        &lucName,         // pointer to an array of names
        &lrdlDomainList,  // receives domain information
        &ltsTranslatedSID // receives relative SIDs
    );
    if (STATUS_SUCCESS != ntsResult) {
        wprintf(L"Failed LsaLookupNames - %lu \n", LsaNtStatusToWinError(ntsResult));
        return;
    }

    // Get the domain the account resides in.
    myDomain = lrdlDomainList->Domains[ltsTranslatedSID->DomainIndex];
    DomainString = (PWCHAR)LocalAlloc(LPTR, myDomain.Name.Length + 1);
    _ASSERTE(DomainString);
    wcsncpy_s(DomainString,
              myDomain.Name.Length + 1,
              myDomain.Name.Buffer,
              myDomain.Name.Length);

    // Display the relative Id. 
    wprintf(L"Relative Id is %lu in domain %ws.\n", ltsTranslatedSID->RelativeId, DomainString);

    LocalFree(DomainString);
    LsaFreeMemory(ltsTranslatedSID);
    LsaFreeMemory(lrdlDomainList);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef UNICODE
#define UNICODE
#endif // UNICODE

#include "ntsecapi.h"

NTSTATUS
OpenPolicy(
    LPWSTR ServerName,          // machine to open policy on (Unicode)
    DWORD DesiredAccess,        // desired access to policy
    PLSA_HANDLE PolicyHandle    // resultant policy handle
);

BOOL
GetAccountSid(
    LPTSTR SystemName,          // where to lookup account
    LPTSTR AccountName,         // account of interest
    PSID * Sid                   // resultant buffer containing SID
);

NTSTATUS
SetPrivilegeOnAccount(
    LSA_HANDLE PolicyHandle,    // open policy handle
    PSID AccountSid,            // SID to grant privilege to
    LPWSTR PrivilegeName,       // privilege to grant (Unicode)
    BOOL bEnable                // enable or disable
);

void
InitLsaString(
    PLSA_UNICODE_STRING LsaString, // destination
    LPWSTR String                  // source (Unicode)
);


#define RTN_OK 0
#define RTN_USAGE 1
#define RTN_ERROR 13


// If you have the ddk, include ntstatus.h.
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#endif


EXTERN_C
__declspec(dllexport)
int WINAPI ManageUserPrivileges(int argc, char * argv[])
/*
\Windows-classic-samples\Samples\Win7Samples\security\lsapolicy\lsaprivs\LsaPrivs.c
*/
/*
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

Copyright (C) 1998 - 2000.  Microsoft Corporation.  All rights reserved.

LsaSamp.c

This sample demonstrates the use of the Lsa APIs to manage User Privileges.
This sample will work properly compiled ANSI or Unicode.
History:    12-Jul-95 Scott Field (sfield)    Created
            08-Jan-99 David Mowers (davemo)   Updated
*/
{
    LSA_HANDLE PolicyHandle;
    WCHAR wComputerName[256] = L"";   // static machine name buffer
    TCHAR AccountName[256];         // static account name buffer
    PSID pSid;
    NTSTATUS Status;
    int iRetVal = RTN_ERROR;          // assume error from main

    if (argc == 1) {
        fprintf(stderr, "Usage: %s <Account> [TargetMachine]\n", argv[0]);
        return RTN_USAGE;
    }

    // Pick up account name on argv[1].
    // Assumes source is ANSI. Resultant string is ANSI or Unicode
    _snwprintf_s(AccountName, 256, 255, TEXT("%hS"), argv[1]);

    // Pick up machine name on argv[2], if appropriate
    // assumes source is ANSI. Resultant string is Unicode.
    if (argc == 3) _snwprintf_s(wComputerName, 256, 255, L"%hS", argv[2]);

    // Open the policy on the target machine. 
    Status = OpenPolicy(
        wComputerName,      // target machine
        POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES,
        &PolicyHandle       // resultant policy handle
    );

    if (Status != STATUS_SUCCESS) {
        DisplayNtStatus("OpenPolicy", Status);
        return RTN_ERROR;
    }

    // Obtain the SID of the user/group.
    // Note that we could target a specific machine, but we don't.
    // Specifying NULL for target machine searches for the SID in the
    // following order: well-known, Built-in and local, primary domain, trusted domains.
    if (GetAccountSid(
        NULL,       // default lookup logic
        AccountName,// account to obtain SID
        &pSid       // buffer to allocate to contain resultant SID
    )) {
        // We only grant the privilege if we succeeded in obtaining the
        // SID. We can actually add SIDs which cannot be looked up, but
        // looking up the SID is a good sanity check which is suitable for most cases.

        // Grant the SeServiceLogonRight to users represented by pSid.
        Status = SetPrivilegeOnAccount(
            PolicyHandle,           // policy handle
            pSid,                   // SID to grant privilege
            (LPWSTR)L"SeServiceLogonRight", // Unicode privilege
            TRUE                    // enable the privilege
        );
        if (Status == STATUS_SUCCESS)
            iRetVal = RTN_OK;
        else
            DisplayNtStatus("SetPrivilegeOnAccount", Status);
    } else {
        DisplayWinError("GetAccountSid", GetLastError());// Error obtaining SID.
    }

    LsaClose(PolicyHandle);// Close the policy handle.

    // Free memory allocated for SID.
    if (pSid != NULL) HeapFree(GetProcessHeap(), 0, pSid);

    return iRetVal;
}


//BOOL GetAccountSid(LPTSTR SystemName, LPTSTR AccountName, PSID * Sid)
///*++
//This function attempts to obtain a SID representing the supplied
//account on the supplied system.
//
//If the function succeeds, the return value is TRUE. A buffer is
//allocated which contains the SID representing the supplied account.
//This buffer should be freed when it is no longer needed by calling
//HeapFree(GetProcessHeap(), 0, buffer)
//
//If the function fails, the return value is FALSE. Call GetLastError()
//to obtain extended error information.
//
//--*/
//{
//    LPTSTR ReferencedDomain = NULL;
//    LPTSTR TempReferencedDomain = NULL;
//    LPTSTR TempSid = NULL;
//    DWORD cbSid = 128;    // initial allocation attempt
//    DWORD cchReferencedDomain = 16; // initial allocation size
//    SID_NAME_USE peUse;
//    BOOL bSuccess = FALSE; // assume this function will fail
//
//    __try {
//
//        //
//        // initial memory allocations
//        //
//        *Sid = (PSID)HeapAlloc(GetProcessHeap(), 0, cbSid);
//
//        if (*Sid == NULL) __leave;
//
//        ReferencedDomain = (LPTSTR)HeapAlloc(
//            GetProcessHeap(),
//            0,
//            cchReferencedDomain * sizeof(TCHAR)
//        );
//
//        if (ReferencedDomain == NULL) __leave;
//
//        //
//        // Obtain the SID of the specified account on the specified system.
//        //
//        while (!LookupAccountName(
//            SystemName,         // machine to lookup account on
//            AccountName,        // account to lookup
//            *Sid,               // SID of interest
//            &cbSid,             // size of SID
//            ReferencedDomain,   // domain account was found on
//            &cchReferencedDomain,
//            &peUse
//        )) {
//            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
//                //
//                // reallocate memory
//                //
//                TempSid = (PSID)HeapReAlloc(
//                    GetProcessHeap(),
//                    0,
//                    *Sid,
//                    cbSid
//                );
//
//                if (TempSid == NULL) __leave;
//                *Sid = TempSid;
//
//                TempReferencedDomain = (LPTSTR)HeapReAlloc(
//                    GetProcessHeap(),
//                    0,
//                    ReferencedDomain,
//                    cchReferencedDomain * sizeof(TCHAR)
//                );
//
//                if (TempReferencedDomain == NULL) __leave;
//                ReferencedDomain = TempReferencedDomain;
//
//            } else __leave;
//        }
//
//        //
//        // Indicate success.
//        //
//        bSuccess = TRUE;
//
//    } // try
//    __finally {
//
//        //
//        // Cleanup and indicate failure, if appropriate.
//        //
//
//        HeapFree(GetProcessHeap(), 0, ReferencedDomain);
//
//        if (!bSuccess) {
//            if (*Sid != NULL) {
//                HeapFree(GetProcessHeap(), 0, *Sid);
//                *Sid = NULL;
//            }
//        }
//
//    } // finally
//
//    return bSuccess;
//}


NTSTATUS
SetPrivilegeOnAccount(
    LSA_HANDLE PolicyHandle,    // open policy handle
    PSID AccountSid,            // SID to grant privilege to
    LPWSTR PrivilegeName,       // privilege to grant (Unicode)
    BOOL bEnable                // enable or disable
)
{
    LSA_UNICODE_STRING PrivilegeString;

    // Create a LSA_UNICODE_STRING for the privilege name.
    InitLsaString(&PrivilegeString, PrivilegeName);

    // grant or revoke the privilege, accordingly
    if (bEnable) {
        return LsaAddAccountRights(
            PolicyHandle,       // open policy handle
            AccountSid,         // target SID
            &PrivilegeString,   // privileges
            1                   // privilege count
        );
    } else {
        return LsaRemoveAccountRights(
            PolicyHandle,       // open policy handle
            AccountSid,         // target SID
            FALSE,              // do not disable all rights
            &PrivilegeString,   // privileges
            1                   // privilege count
        );
    }
}


//void InitLsaString(PLSA_UNICODE_STRING LsaString,LPWSTR String)
//{
//    DWORD StringLength;
//
//    if (String == NULL) {
//        LsaString->Buffer = NULL;
//        LsaString->Length = 0;
//        LsaString->MaximumLength = 0;
//        return;
//    }
//
//    StringLength = lstrlenW(String);
//    LsaString->Buffer = String;
//    LsaString->Length = (USHORT)StringLength * sizeof(WCHAR);
//    LsaString->MaximumLength = (USHORT)(StringLength + 1) * sizeof(WCHAR);
//}


//NTSTATUS OpenPolicy(LPWSTR ServerName, DWORD DesiredAccess, PLSA_HANDLE PolicyHandle)
//{
//    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
//    LSA_UNICODE_STRING ServerString;
//    PLSA_UNICODE_STRING Server;
//
//    //
//    // Always initialize the object attributes to all zeroes.
//    //
//    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
//
//    if (ServerName != NULL) {
//        //
//        // Make a LSA_UNICODE_STRING out of the LPWSTR passed in
//        //
//        InitLsaString(&ServerString, ServerName);
//        Server = &ServerString;
//    } else {
//        Server = NULL;
//    }
//
//    //
//    // Attempt to open the policy.
//    //
//    return LsaOpenPolicy(
//        Server,
//        &ObjectAttributes,
//        DesiredAccess,
//        PolicyHandle
//    );
//}


//////////////////////////////////////////////////////////////////////////////////////////////////
