// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。


#include <NTSecAPI.h>


#pragma warning(disable:6067)
#pragma warning(disable:28183)
#pragma warning(disable:26451)


//////////////////////////////////////////////////////////////////////////////////////////////////


void
DisplayWinError(
    LPCSTR szAPI,    // pointer to Ansi function name
    DWORD dwError   // DWORD WinError
)
/*
摘自：Windows-classic-samples\Samples\Win7Samples\security\authorization\audit\Audit.c
*/
{
    LPSTR MessageBuffer;
    DWORD dwBufferLength;

    // TODO get this fprintf out of here!
    fprintf(stderr, "%s error!\n", szAPI);

    if (dwBufferLength = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&MessageBuffer,
        0,
        NULL)) {
        DWORD dwBytesWritten; // unused

        // Output message string on stderr
        WriteFile(GetStdHandle(STD_ERROR_HANDLE), MessageBuffer, dwBufferLength, &dwBytesWritten, NULL);

        // free the buffer allocated by the system
        LocalFree(MessageBuffer);
    }
}


void DisplayNtStatus(LPCSTR szAPI, NTSTATUS Status)
/*
摘自：Windows-classic-samples\Samples\Win7Samples\security\authorization\audit\Audit.c
*/
{
    // convert the NTSTATUS to Winerror and DisplayWinError()
    DisplayWinError(szAPI, LsaNtStatusToWinError(Status));
}


void DisplayError(const wchar_t * pszAPI)
{
    LPWSTR lpvMessageBuffer;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPWSTR)&lpvMessageBuffer,
                  0,
                  NULL);

    //去掉回车换行
    int x = lstrlenW((LPWSTR)lpvMessageBuffer);
    lpvMessageBuffer[x - 1] = 0;
    lpvMessageBuffer[x - 2] = 0;

    //... now display this string
    wprintf(L"ERROR: API        = %s.\n", pszAPI);
    wprintf(L"       error code = %d.\n", GetLastError());
    wprintf(L"       message    = %s.\n", (LPWSTR)lpvMessageBuffer);//lpvMessageBuffer后有回车换行，而且还有垃圾数据。

    LocalFree(lpvMessageBuffer);// Free the buffer allocated by the system
    //ExitProcess(GetLastError());
}


void printError(CONST TCHAR * msg)
{
    DWORD eNum;
    TCHAR sysMsg[256];
    TCHAR * p;

    eNum = GetLastError();
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, eNum,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                  sysMsg, 256, NULL);

    // Trim the end of the line and terminate it with a null
    p = sysMsg;
    while ((*p > 31) || (*p == 9))
        ++p;
    do { *p-- = 0; } while ((p >= sysMsg) &&
                            ((*p == '.') || (*p < 33)));

    // Display the message
    _tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}


void ErrorHandler(LPCTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code.

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message.

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
                                      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
                    LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                    TEXT("%s failed with error %d: %s"),
                    lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    // Free error-handling buffer allocations.

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef _DEBUG
void DebugPrintA(PCSTR format, ...)
//OutputDebugStringA 最长支持 65534（MAXUINT16 - 1） 个字符的输出(包括结尾的 L'\0').
{
    size_t len = MAXUINT16;

    char * out = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len);
    _ASSERTE(NULL != out);

    va_list marker;

    va_start(marker, format);
    StringCbVPrintfA(out, len, format, marker);//STRSAFE_MAX_CCH
    va_end(marker);

    OutputDebugStringA(out);
    HeapFree(GetProcessHeap(), 0, out);
}
#else
void DebugPrintA(PCSTR format, ...)
{
    UNREFERENCED_PARAMETER(format);
}
#endif


#ifdef _DEBUG
void DebugPrintW(PCWSTR format, ...)
//OutputDebugStringW 最长支持 32766（MAXINT16 - 1） 个字符的输出(包括结尾的 L'\0').
{
    size_t len = MAXINT16 * sizeof(WCHAR);
    wchar_t * out = (wchar_t *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len);
    _ASSERTE(NULL != out);

    va_list marker;

    va_start(marker, format);
    StringCbVPrintfW(out, len, format, marker); //STRSAFE_MAX_CCH
    va_end(marker);

    OutputDebugStringW(out);

    HeapFree(GetProcessHeap(), 0, out);
}
#else
void DebugPrintW(PCWSTR format, ...)
{
    UNREFERENCED_PARAMETER(format);
}
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////


void TimeStampToFileTime(INT64 timeStamp, FILETIME & fileTime)
{
    INT64 nll = timeStamp * 10000000 + 116444736000000000;
    fileTime.dwLowDateTime = (DWORD)nll;
    fileTime.dwHighDateTime = nll >> 32;
}


void FileTimeToTimeStamp(const FILETIME & fileTime, INT64 & timeStamp)
{
    timeStamp = ((INT64)fileTime.dwHighDateTime << 32) + fileTime.dwLowDateTime;
    timeStamp = (timeStamp - 116444736000000000) / 10000000;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


void GetImageFilePath(_Out_ LPWSTR ImageFilePath, _In_ DWORD nSize)
/*
功能：获取进程所在的目录。

此目录区别于GetCurrentDirectory.
*/
{
    GetModuleFileName(NULL, ImageFilePath, nSize);

    ImageFilePath[lstrlen(ImageFilePath) - lstrlen(PathFindFileName(ImageFilePath))] = 0;

    PathRemoveBackslash(ImageFilePath);
}


//////////////////////////////////////////////////////////////////////////////////////////////////


void Nt2Dos(IN  OUT TCHAR * szFileName)
/*
功能：把形如：L"\\Device\\HarddiskVolume1\\test.txt"的文件路径转换为：L"C:\\test.txt"。
本代码摘自：微软的CppFileHandle工程。
*/
{
    if (szFileName == NULL) {
        return;
    }

    if (lstrlenW(szFileName) == 0) {
        return;
    }

    TCHAR szTemp[MAX_PATH] = {0};// Translate path with device name to drive letters.

    // Get a series of null-terminated strings, one for each valid drive in the system, plus with an additional null character.
    // Each string is a drive name. e.g. C:\\0D:\\0\0
    if (GetLogicalDriveStrings(MAX_PATH - 1, szTemp)) {
        TCHAR szName[MAX_PATH];
        TCHAR szDrive[3] = _T(" :");
        BOOL bFound = FALSE;
        TCHAR * p = szTemp;

        do {
            *szDrive = *p;// Copy the drive letter to the template string

            // Look up each device name. 
            // For example, given szDrive is C:, the output szName may be \Device\HarddiskVolume2.
            if (QueryDosDevice(szDrive, szName, MAX_PATH)) {
                size_t uNameLen = _tcslen(szName);

                if (uNameLen < MAX_PATH) {
                    // Match the device name e.g. \Device\HarddiskVolume2
                    bFound = _tcsnicmp(szFileName, szName, uNameLen) == 0;
                    if (bFound) {
                        // Reconstruct szFileName using szTempFile
                        // Replace device path with DOS path
                        TCHAR szTempFile[MAX_PATH];
                        StringCchPrintf(szTempFile, MAX_PATH, _T("%s%s"), szDrive, szFileName + uNameLen);
                        StringCchCopyN(szFileName, MAX_PATH + 1, szTempFile, _tcslen(szTempFile));
                    }
                }
            }

            while (*p++);// Go to the next NULL character, i.e. the next drive name.

        } while (!bFound && *p); // End of string
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////
