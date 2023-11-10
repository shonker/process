#include "User.h"


int SidTest(int argc, _TCHAR * argv[])
/*
sid一个神秘的东西,本想是获取或者枚举用户和它的关系.
这里有两个从微软搬过来的函数,
一个是从句柄获得sid,这个好像有问题,难道是我使用的问题.
一个是从(用户)名字获取sid.这个经试验是好的.
这里主要用了两个函数:GetTokenInformation,LookupAccountNameW
因为用GetTokenInformation的函数获取的东西好像有点问题,所以此文就命名为:LookupAccountName.Cpp.
*/
{
    wchar_t sz_UserNamew[260] = {0};
    int len = ARRAYSIZE(sz_UserNamew);
    GetUserName(sz_UserNamew, (LPDWORD)&len);

    LPWSTR * wsz_sid = (LPWSTR *)HeapAlloc(GetProcessHeap(), 0, 0x200);
    if (!wsz_sid) {

        return 1;
    }

    PSID * ppSid = (PSID *)HeapAlloc(GetProcessHeap(), 0, 0x200);
    if (!ppSid) {
        HeapFree(GetProcessHeap(), 0, wsz_sid);
        return 1;
    }

    GetSid(sz_UserNamew, ppSid);//Administrator,Defaultapppool应该有枚举的办法.NetUserEnum,但不全.特殊的没有.
    bool  b = ConvertSidToStringSid(*ppSid, (LPWSTR *)wsz_sid);
    int x = GetLastError();
    //MessageBox(0, (LPCWSTR)(*(int *)wsz_sid), 0, 0);

    RtlZeroMemory(wsz_sid, 0x200);
    RtlZeroMemory(ppSid, 0x200);

    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY_SOURCE | TOKEN_QUERY, &hToken))
        return(FALSE);

    GetLogonSID(hToken, ppSid);//字面意思是登录的sid,用的是当前进程或者线程的句柄.
    b = ConvertSidToStringSid(*ppSid, (LPWSTR *)wsz_sid);
    x = GetLastError();

    //得到的这个值在注册表中找不到.HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList
    //MessageBox(0, (LPCWSTR)(*(int *)wsz_sid), 0, 0);

    HeapFree(GetProcessHeap(), 0, wsz_sid);
    HeapFree(GetProcessHeap(), 0, ppSid);

    SearchTokenGroupsForSID();

    //////////////////////////////////////////////////////////////////////////////////////////////////

    hToken = nullptr;
    if (GetCurrentToken(&hToken) == false) {
        return 0;
    }

    PSID pSid = nullptr;
    if (!GetLogonSID(hToken, &pSid)) {
        return 0;
    }

    LPTSTR StringSid;
    if (ConvertSidToStringSid(pSid, &StringSid) == 0) {
        //继续走下面的释放函数。
    }

    if (LocalFree(StringSid) != nullptr) {
        int x = GetLastError();
    }

    if (pSid) {
        FreeLogonSID(&pSid);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////

    return 0;
}


void TestIsCurrentUserLocalAdministrator()
{
    if (IsCurrentUserLocalAdministrator())
        printf("You are an administrator\n");
    else
        printf("You are not an administrator\n");
}


void TestEnumerateAccountRights()
{
    wchar_t szUser[MAX_PATH] = {0};//估计最大值不是这个。
    wchar_t szDomain[MAX_PATH] = {0};//估计最大值不是这个。这个好像和计算机名一样。
    DWORD d = MAX_PATH;
    bool b = GetCurrentUserAndDomain(szUser, &d, szDomain, &d);

    USES_CONVERSION;

#pragma prefast(push)
#pragma prefast(disable: 6255, "`_alloca` 通过引发堆栈溢出异常来指示失败。应考虑改用 _malloca")
    LPSTR Stack = W2A(szUser);
#pragma prefast(pop)      

    char * argv[2];
    argv[1] = Stack;

    int ret = EnumerateAccountRights(2, argv);
}


void TestGetCurrentSid()
{
    GetCurrentSid();
}


void TestConvertStringSidToSid()
{
    BOOL                  fRet;
    PSID                  pIntegritySid = nullptr;
    WCHAR wszIntegritySid[20] = L"S-1-5-18";// Local System

    fRet = ConvertStringSidToSid(wszIntegritySid, &pIntegritySid);

    PISID tmp = (PISID)pIntegritySid;
    int x = sizeof(SID);

    LocalFree(pIntegritySid);
}


void BruteForceCrack()
/*
获取操作系统的用户的密码的正规方式是编译一个DLL，然后修改注册表，等登录的时候自动的拦截到，不过这个需要权限的限制。

在防御的方面说，这是弱密码检测。
在攻击的方面说，这是暴力破解。
*/
{
    LPCWSTR password[] = {//弱密码字典，可以有成千上万条。
        L"",
        L"123456",
        L"1qaz2wsx",
        L"!QAZ@WSX",
        L"qwer",
        L"password",
        L"123456789",
        L"123123",
        L"correy" //正确的密码。
    };

    WCHAR UserName[UNLEN + 1]{};
    DWORD Size{_ARRAYSIZE(UserName)};
    GetUserName(UserName, &Size);
    printf("正在尝试获取当前用户(%ls)的密码。\n", UserName);

    LPUSER_INFO_2 UserInfo2 = NULL;
    USER_INFO_2 Tmp = {0};
    DWORD parm_err = 0;
    NET_API_STATUS nStatus = NERR_Success;
    DWORD dwRet = NERR_Success;

    __try {//警告：看看编译器的奇特：如果没有__try及__leave，在中间声明变量也会编译出错。
        nStatus = NetUserGetInfo(NULL, UserName, 2, (LPBYTE *)&UserInfo2);
        if (nStatus != NERR_Success) {

            __leave;
        }

        /*账户设置为永不锁定*/
        memcpy_s(&Tmp, sizeof(USER_INFO_2), UserInfo2, sizeof(USER_INFO_2));
        Tmp.usri2_bad_pw_count = 0;
        nStatus = NetUserSetInfo(NULL, UserName, 2, (LPBYTE)&Tmp, &parm_err);
        if (nStatus != NERR_Success) {

            __leave;
        }

        dwRet = NetUserChangePassword(NULL, UserName, L"", L"");
        if (NERR_Success == dwRet || NERR_PasswordTooShort == dwRet) {
            printf("密码为空\n");
            __leave;
        }

        /* 检测账号密码相同账户 *///日她娘的，这个注释如果删除了，编译会出现：error C2059: 语法错误:“}”
        dwRet = NetUserChangePassword(NULL, UserName, UserName, UserName);
        if (NERR_Success == dwRet || NERR_PasswordTooShort == dwRet) {
            printf("密码和用户相同\n");
            __leave;
        }

        for (int i = 0; i < _ARRAYSIZE(password); i++) {
            dwRet = NetUserChangePassword(NULL, UserName, password[i], password[i]);
            if (NERR_Success == dwRet || NERR_PasswordTooShort == dwRet) {
                printf("UserName:%ls, Password:%ls\n", UserName, password[i]);
                break;
            } else if (ERROR_ACCOUNT_LOCKED_OUT == dwRet) {
                break;
            }
        }
    } __finally {
        /*还原用户配置*/
        memcpy_s(&Tmp, sizeof(USER_INFO_2), UserInfo2, sizeof(USER_INFO_2));
        Tmp.usri2_bad_pw_count = UserInfo2->usri2_bad_pw_count;
        nStatus = NetUserSetInfo(NULL, UserName, 2, (LPBYTE)&Tmp, &parm_err);
        if (nStatus != NERR_Success) {

        }

        if (UserInfo2) {
            NetApiBufferFree(UserInfo2);
        }
    }
}