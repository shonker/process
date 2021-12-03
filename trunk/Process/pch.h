// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。


#pragma once


// 添加要在此处预编译的标头
#include "framework.h"


//////////////////////////////////////////////////////////////////////////////////////////////////


// If you have the ddk, include ntstatus.h.
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#endif


#define RTN_OK 0
#define RTN_USAGE 1
#define RTN_ERROR 13


#define LOW_INTEGRITY_SDDL_SACL_W L"S:(ML;;NW;;;LW)"

#define MAX_NAME 256

#define MY_BUFSIZE 256 // all allocations should be dynamic


//////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef __cplusplus
extern "C++"
{
    char _RTL_CONSTANT_STRING_type_check(const char * s);
    char _RTL_CONSTANT_STRING_type_check(const WCHAR * s);
    // __typeof would be desirable here instead of sizeof.
    template <size_t N> class _RTL_CONSTANT_STRING_remove_const_template_class;
template <> class _RTL_CONSTANT_STRING_remove_const_template_class<sizeof(char)> { public: typedef  char T; };
template <> class _RTL_CONSTANT_STRING_remove_const_template_class<sizeof(WCHAR)> { public: typedef WCHAR T; };
#define _RTL_CONSTANT_STRING_remove_const_macro(s) \
    (const_cast<_RTL_CONSTANT_STRING_remove_const_template_class<sizeof((s)[0])>::T*>(s))
}
#else
char _RTL_CONSTANT_STRING_type_check(const void * s);
#define _RTL_CONSTANT_STRING_remove_const_macro(s) (s)
#endif
#define RTL_CONSTANT_STRING(s) \
{ \
    sizeof( s ) - sizeof( (s)[0] ), \
    sizeof( s ) / sizeof(_RTL_CONSTANT_STRING_type_check(s)), \
    _RTL_CONSTANT_STRING_remove_const_macro(s) \
}


//////////////////////////////////////////////////////////////////////////////////////////////////


void DisplayError(const wchar_t * pszAPI);
void printError(CONST TCHAR * msg);
void ErrorHandler(LPCTSTR lpszFunction);
void DisplayNtStatus(LPCSTR szAPI, NTSTATUS Status);
void DisplayWinError(LPCSTR szAPI, DWORD dwError);

void DebugPrintA(PCSTR format, ...);
void DebugPrintW(PCWSTR format, ...);

bool GetCurrentToken(OUT PHANDLE hToken);

VOID FreeLogonSID(PSID * ppsid);

EXTERN_C
__declspec(dllexport)
BOOL WINAPI GetLogonSID(HANDLE hToken, PSID * ppsid);

EXTERN_C
__declspec(dllexport)
BOOL WINAPI IsCurrentSessionRemoteable();

EXTERN_C
__declspec(dllexport)
void WINAPI EnumerateSessions();

EXTERN_C
__declspec(dllexport)
bool WINAPI IsRemoteSession(DWORD SessionId);

EXTERN_C
__declspec(dllexport)
void WINAPI EnumerateSessionsEx();

EXTERN_C
__declspec(dllexport)
BOOL WINAPI SearchTokenGroupsForSID(VOID);

void TimeStampToFileTime(INT64 timeStamp, FILETIME & fileTime);
void FileTimeToTimeStamp(const FILETIME & fileTime, INT64 & timeStamp);

void GetImageFilePath(_Out_ LPWSTR ImageFilePath, _In_ DWORD nSize);

void Nt2Dos(IN  OUT TCHAR * szFileName);

BOOL WINAPI EnablePrivilege(PCTSTR szPrivilege, BOOL fEnable);


//////////////////////////////////////////////////////////////////////////////////////////////////
