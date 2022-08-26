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
    _ASSERTE(wsz_sid);

    PSID * ppSid = (PSID *)HeapAlloc(GetProcessHeap(), 0, 0x200);
    _ASSERTE(ppSid);

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

    hToken = NULL;
    if (GetCurrentToken(&hToken) == false) {
        return 0;
    }

    PSID pSid = NULL;
    if (!GetLogonSID(hToken, &pSid)) {
        return 0;
    }

    LPTSTR StringSid;
    if (ConvertSidToStringSid(pSid, &StringSid) == 0) {
        //继续走下面的释放函数。
    }

    if (LocalFree(StringSid) != NULL) {
        int x = GetLastError();
    }

    if (pSid) {
        FreeLogonSID(&pSid);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////

    return 0;
}
