#include "pch.h"
#include "FileMap.h"
#include "Handle.h"


#pragma warning(disable:6387) 
#pragma warning(disable:6011) 
#pragma warning(disable:26451) 


//////////////////////////////////////////////////////////////////////////////////////////////////


#define BUFFSIZE 1024 // size of the memory to examine at any one time
#define FILE_MAP_START 138240 // starting point within the file of the data to examine (135K)

/* 
The test file. 
The code below creates the file and populates it, so there is no need to supply it in advance. 
*/
CONST TCHAR * lpcTheFile = TEXT("fmtest.txt"); // the file to be manipulated


EXTERN_C
__declspec(dllexport)
int WINAPI CreatingViewWithinFile(void)
/*
* Creating a View Within a File
*
   This program demonstrates file mapping, especially how to align a view with the system file allocation granularity.

   https://docs.microsoft.com/en-us/windows/win32/memory/creating-a-view-within-a-file
*/
{
    HANDLE hMapFile{};      // handle for the file's memory-mapped region
    HANDLE hFile{};         // the file handle
    BOOL bFlag{};           // a result holder
    DWORD dBytesWritten{};  // number of bytes written
    DWORD dwFileSize{};     // temporary storage for file sizes
    DWORD dwFileMapSize{};  // size of the file mapping
    DWORD dwMapViewSize{};  // the size of the view
    DWORD dwFileMapStart{}; // where to start the file map view
    DWORD dwSysGran{};      // system allocation granularity
    SYSTEM_INFO SysInfo{};  // system information; used to get granularity
    LPVOID lpMapAddress{};  // pointer to the base address of the memory-mapped region
    char * pData{};         // pointer to the data
    int i{};                // loop counter
    int iData{};            // on success contains the first int of data
    int iViewDelta{};       // the offset into the view where the data shows up

    // Create the test file.
    // Open it "Create Always" to overwrite any existing file. The data is re-created below
    hFile = CreateFile(lpcTheFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT("hFile is NULL\n"));
        _tprintf(TEXT("Target file is %s\n"), lpcTheFile);
        return 4;
    }

    // Get the system allocation granularity.
    GetSystemInfo(&SysInfo);
    dwSysGran = SysInfo.dwAllocationGranularity;

    // Now calculate a few variables. Calculate the file offsets as
    // 64-bit values, and then get the low-order 32 bits for the function calls.

    // To calculate where to start the file mapping, round down the
    // offset of the data into the file to the nearest multiple of the system allocation granularity.
    dwFileMapStart = (FILE_MAP_START / dwSysGran) * dwSysGran;
    _tprintf(TEXT("The file map view starts at %lu bytes into the file.\n"), dwFileMapStart);

    // Calculate the size of the file mapping view.
    dwMapViewSize = (FILE_MAP_START % dwSysGran) + BUFFSIZE;
    _tprintf(TEXT("The file map view is %lu bytes large.\n"), dwMapViewSize);

    // How large will the file mapping object be?
    dwFileMapSize = FILE_MAP_START + BUFFSIZE;
    _tprintf(TEXT("The file mapping object is %lu bytes large.\n"), dwFileMapSize);

    // The data of interest isn't at the beginning of the view, so determine how far into the view to set the pointer.
    iViewDelta = FILE_MAP_START - dwFileMapStart;
    _tprintf(TEXT("The data is %d bytes into the view.\n"), iViewDelta);

    // Now write a file with data suitable for experimentation. This
    // provides unique int (4-byte) offsets in the file for easy visual
    // inspection. Note that this code does not check for storage
    // medium overflow or other errors, which production code should
    // do. Because an int is 4 bytes, the value at the pointer to the
    // data should be one quarter of the desired offset into the file

    for (i = 0; i < (int)dwSysGran; i++) {
        WriteFile(hFile, &i, sizeof(i), &dBytesWritten, NULL);
    }

    // Verify that the correct file size was written.
    dwFileSize = GetFileSize(hFile, NULL);
    _tprintf(TEXT("hFile size: %10u\n"), dwFileSize);

    // Create a file mapping object for the file
    // Note that it is a good idea to ensure the file size is not zero
    hMapFile = CreateFileMapping(hFile,          // current file handle
                                 NULL,           // default security
                                 PAGE_READWRITE, // read/write permission
                                 0,              // size of mapping object, high
                                 dwFileMapSize,  // size of mapping object, low
                                 NULL);          // name of mapping object
    if (hMapFile == NULL) {
        _tprintf(TEXT("hMapFile is NULL: last error: %u\n"), GetLastError());
        return (2);
    }

    // Map the view and test the results.
    lpMapAddress = MapViewOfFile(hMapFile,            // handle to mapping object
                                 FILE_MAP_ALL_ACCESS, // read/write
                                 0,                   // high-order 32 bits of file offset
                                 dwFileMapStart,      // low-order 32 bits of file offset
                                 dwMapViewSize);      // number of bytes to map
    if (lpMapAddress == NULL) {
        _tprintf(TEXT("lpMapAddress is NULL: last error: %u\n"), GetLastError());
        return 3;
    }

    // Calculate the pointer to the data.
    pData = (char *)lpMapAddress + iViewDelta;

    // Extract the data, an int. Cast the pointer pData from a "pointer
    // to char" to a "pointer to int" to get the whole thing
    iData = *(int *)pData;

    _tprintf(TEXT("The value at the pointer is %d,\nwhich %s one quarter of the desired file offset.\n"),
             iData,
             iData * 4 == FILE_MAP_START ? TEXT("is") : TEXT("is not"));

    // Close the file mapping object and the open file
    bFlag = UnmapViewOfFile(lpMapAddress);
    bFlag = CloseHandle(hMapFile); // close the file mapping object
    if (!bFlag) {
        _tprintf(TEXT("\nError %lu occurred closing the mapping object!"), GetLastError());
    }

    bFlag = CloseHandle(hFile);   // close the file itself
    if (!bFlag) {
        _tprintf(TEXT("\nError %lu occurred closing the file!"), GetLastError());
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


#define BUF_SIZE 256
TCHAR szName[] = TEXT("Global\\MyFileMappingObject");
TCHAR szMsg[] = TEXT("Message from first process.");


EXTERN_C
__declspec(dllexport)
int WINAPI CreatingNamedSharedMemory()
/*
Creating Named Shared Memory

https://docs.microsoft.com/en-us/windows/win32/memory/creating-named-shared-memory
*/
{
    HANDLE hMapFile{};
    LPCTSTR pBuf{};

    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,    // use paging file
        NULL,                    // default security
        PAGE_READWRITE,          // read/write access
        0,                       // maximum object size (high-order DWORD)
        BUF_SIZE,                // maximum object size (low-order DWORD)
        szName);                 // name of mapping object
    if (hMapFile == NULL) {
        _tprintf(TEXT("Could not create file mapping object (%u).\n"), GetLastError());
        return 1;
    }
    pBuf = (LPTSTR)MapViewOfFile(hMapFile,   // handle to map object
                                 FILE_MAP_ALL_ACCESS, // read/write permission
                                 0,
                                 0,
                                 BUF_SIZE);
    if (pBuf == NULL) {
        _tprintf(TEXT("Could not map view of file (%u).\n"), GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    CopyMemory((PVOID)pBuf, szMsg, (_tcslen(szMsg) * sizeof(TCHAR)));
    (void)_getch();

    UnmapViewOfFile(pBuf);

    CloseHandle(hMapFile);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef BUF_SIZE
#undef BUF_SIZE
#define BUF_SIZE 65536
#endif // BUF_SIZE


TCHAR szName2[] = TEXT("LARGEPAGE");
typedef int (*GETLARGEPAGEMINIMUM)(void);


void DisplayError(const wchar_t * pszAPI, DWORD dwError)
{
    LPVOID lpvMessageBuffer;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dwError,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&lpvMessageBuffer, 0, NULL);

    //... now display this string
    _tprintf(TEXT("ERROR: API        = %s\n"), pszAPI);
    _tprintf(TEXT("       error code = %u\n"), dwError);
    _tprintf(TEXT("       message    = %p\n"), lpvMessageBuffer);

    // Free the buffer allocated by the system
    LocalFree(lpvMessageBuffer);

    ExitProcess(GetLastError());
}


void Privilege(const wchar_t * pszPrivilege, BOOL bEnable)
{
    HANDLE           hToken{};
    TOKEN_PRIVILEGES tp{};
    BOOL             status{};
    DWORD            error{};

    // open process token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        DisplayError(TEXT("OpenProcessToken"), GetLastError());

    // get the luid
    if (!LookupPrivilegeValue(NULL, pszPrivilege, &tp.Privileges[0].Luid))
        DisplayError(TEXT("LookupPrivilegeValue"), GetLastError());

    tp.PrivilegeCount = 1;

    // enable or disable privilege
    if (bEnable)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // enable or disable privilege
    status = AdjustTokenPrivileges(hToken, FALSE, &tp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    // It is possible for AdjustTokenPrivileges to return TRUE and still not succeed.
    // So always check for the last error value.
    error = GetLastError();
    if (!status || (error != ERROR_SUCCESS))
        DisplayError(TEXT("AdjustTokenPrivileges"), GetLastError());

    // close the handle
    if (!CloseHandle(hToken))
        DisplayError(TEXT("CloseHandle"), GetLastError());
}


EXTERN_C
__declspec(dllexport)
int WINAPI CreatingFileMappingUsingLargePages(void)
/*
Creating a File Mapping Using Large Pages

The following example uses the CreateFileMapping function with the SEC_LARGE_PAGES flag to use large pages.
The buffer must be large enough to contain the minimum size of a large page.
This value is obtained using the GetLargePageMinimum function.
This feature also requires the "SeLockMemoryPrivilege" privilege.

 Note
Starting in Windows 10, version 1703, the MapViewOfFile function maps a view using small pages by default,
even for file mapping objects created with the SEC_LARGE_PAGES flag.
In this and later OS versions, you must specify the FILE_MAP_LARGE_PAGES flag with the MapViewOfFile function to map large pages.
This flag is ignored on OS versions before Windows 10, version 1703.

https://docs.microsoft.com/en-us/windows/win32/memory/creating-a-file-mapping-using-large-pages
*/
{
    HANDLE hMapFile{};
    LPCTSTR pBuf{};
    DWORD size{};
    GETLARGEPAGEMINIMUM pGetLargePageMinimum{};
    HINSTANCE  hDll{};

    // call succeeds only on Windows Server 2003 SP1 or later
    hDll = LoadLibrary(TEXT("kernel32.dll"));
    if (hDll == NULL)
        DisplayError(TEXT("LoadLibrary"), GetLastError());

    pGetLargePageMinimum = (GETLARGEPAGEMINIMUM)GetProcAddress(hDll, "GetLargePageMinimum");
    if (pGetLargePageMinimum == NULL)
        DisplayError(TEXT("GetProcAddress"), GetLastError());

    size = (*pGetLargePageMinimum)();

    FreeLibrary(hDll);

    _tprintf(TEXT("Page Size: %u\n"), size);

    Privilege(TEXT("SeLockMemoryPrivilege"), TRUE);

    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,    // use paging file
        NULL,                    // default security
        PAGE_READWRITE | SEC_COMMIT | SEC_LARGE_PAGES,
        0,                       // max. object size
        size,                    // buffer size
        szName2);                 // name of mapping object
    if (hMapFile == NULL)
        DisplayError(TEXT("CreateFileMapping"), GetLastError());
    else
        _tprintf(TEXT("File mapping object successfully created.\n"));

    Privilege(TEXT("SeLockMemoryPrivilege"), FALSE);

    pBuf = (LPTSTR)MapViewOfFile(hMapFile,          // handle to map object
                                 FILE_MAP_ALL_ACCESS | FILE_MAP_LARGE_PAGES, // read/write permission
                                 0,
                                 0,
                                 BUF_SIZE);
    if (pBuf == NULL)
        DisplayError(TEXT("MapViewOfFile"), GetLastError());
    else
        _tprintf(TEXT("View of file successfully mapped.\n"));

    // do nothing, clean up an exit
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


#define BUFSIZE 512


//BOOL GetFileNameFromHandle(HANDLE hFile)
//{
//    BOOL bSuccess = FALSE;
//    TCHAR pszFilename[MAX_PATH + 1];
//    HANDLE hFileMap;
//
//    // Get the file size.
//    DWORD dwFileSizeHi = 0;
//    DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);
//
//    if (dwFileSizeLo == 0 && dwFileSizeHi == 0) {
//        _tprintf(TEXT("Cannot map a file with a length of zero.\n"));
//        return FALSE;
//    }
//
//    // Create a file mapping object.
//    hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 1, NULL);
//    if (hFileMap) {
//        // Create a file mapping to get the file name.
//        void * pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);
//        if (pMem) {
//            if (GetMappedFileName(GetCurrentProcess(), pMem, pszFilename, MAX_PATH)) {
//                // Translate path with device name to drive letters.
//                TCHAR szTemp[BUFSIZE];
//                szTemp[0] = '\0';
//
//                if (GetLogicalDriveStrings(BUFSIZE - 1, szTemp)) {
//                    TCHAR szName[MAX_PATH];
//                    TCHAR szDrive[3] = TEXT(" :");
//                    BOOL bFound = FALSE;
//                    TCHAR * p = szTemp;
//
//                    do {
//                        // Copy the drive letter to the template string
//                        *szDrive = *p;
//
//                        // Look up each device name
//                        if (QueryDosDevice(szDrive, szName, MAX_PATH)) {
//                            size_t uNameLen = _tcslen(szName);
//
//                            if (uNameLen < MAX_PATH) {
//                                bFound = _tcsnicmp(pszFilename, szName, uNameLen) == 0
//                                    && *(pszFilename + uNameLen) == _T('\\');
//
//                                if (bFound) {
//                                    // Reconstruct pszFilename using szTempFile
//                                    // Replace device path with DOS path
//                                    TCHAR szTempFile[MAX_PATH];
//                                    StringCchPrintf(szTempFile,
//                                                    MAX_PATH,
//                                                    TEXT("%s%s"),
//                                                    szDrive,
//                                                    pszFilename + uNameLen);
//                                    StringCchCopyN(pszFilename, MAX_PATH + 1, szTempFile, _tcslen(szTempFile));
//                                }
//                            }
//                        }
//
//                        // Go to the next NULL character.
//                        while (*p++);
//                    } while (!bFound && *p); // end of string
//                }
//            }
//
//            bSuccess = TRUE;
//            UnmapViewOfFile(pMem);
//        }
//
//        CloseHandle(hFileMap);
//    }
//
//    _tprintf(TEXT("File name is %s\n"), pszFilename);
//    return(bSuccess);
//}


EXTERN_C
__declspec(dllexport)
int WINAPI ObtainingFileNameFromFileHandle(int argc, TCHAR * argv[])
/*
Obtaining a File Name From a File Handle

GetFinalPathNameByHandle, introduced in Windows Vista and Windows Server 2008, will return a path from a handle.
If you need to do this on earlier releases of Windows,
the following example obtains a file name from a handle to a file object using a file mapping object.
It uses the CreateFileMapping and MapViewOfFile functions to create the mapping.
Next, it uses the GetMappedFileName function to obtain the file name.
For remote files, it prints the device path received from this function.
For local files, it converts the path to use a drive letter and prints this path.
To test this code, create a main function that opens a file using CreateFile and passes the resulting handle to GetFileNameFromHandle.

https://docs.microsoft.com/en-us/windows/win32/memory/obtaining-a-file-name-from-a-file-handle
*/
{
    HANDLE hFile{};

    if (argc != 2) {
        _tprintf(TEXT("This sample takes a file name as a parameter.\n"));
        return 0;
    }

    hFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT("CreateFile failed with %u\n"), GetLastError());
        return 0;
    }

    GetFileNameFromHandle(hFile);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


// This class makes it easy to work with memory-mapped sparse files
class CGMMF
{
    HANDLE m_hfilemap;      // File-mapping object
    PVOID  m_pvFile;        // Address to start of mapped file

public:
    // Creates a GMMF and maps it in the process's address space.
    CGMMF(HANDLE hFile, UINT_PTR cbFileSizeMax);

    // Closes a GMMF
    ~CGMMF() { ForceClose(); }

    // GMMF to BYTE cast operator returns address of first byte 
    // in the memory-mapped sparse file. 
    operator PBYTE() const { return((PBYTE)m_pvFile); }

    // Allows you to explicitly close the GMMF without having
    // to wait for the destructor to be called.
    VOID ForceClose();

public:
    // Static method that resets a portion of a file back to
    // zeroes (frees disk clusters)
    static BOOL SetToZero(HANDLE hfile, UINT_PTR cbOffsetStart, UINT_PTR cbOffsetEnd);
};


CGMMF::CGMMF(HANDLE hfile, UINT_PTR cbFileSizeMax)
{
    m_hfilemap = m_pvFile = NULL;// Initialize to NULL in case something goes wrong

    // Make the file sparse
    DWORD dw;
    BOOL fOk = ::DeviceIoControl(hfile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dw, NULL);
    if (fOk) {
        // Create a file-mapping object
        m_hfilemap = ::CreateFileMapping(hfile, NULL, PAGE_READWRITE, 0, (DWORD)cbFileSizeMax, NULL);
        if (m_hfilemap != NULL) {// Map the file into the process's address space            
            m_pvFile = ::MapViewOfFile(m_hfilemap, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
        } else {
            ForceClose();// Failed to map the file, cleanup
        }
    }
}


VOID CGMMF::ForceClose() 
{
    // Cleanup everything that was done sucessfully
    if (m_pvFile != NULL) {
        ::UnmapViewOfFile(m_pvFile);
        m_pvFile = NULL;
    }
    if (m_hfilemap != NULL) {
        ::CloseHandle(m_hfilemap);
        m_hfilemap = NULL;
    }
}


BOOL CGMMF::SetToZero(HANDLE hfile, UINT_PTR cbOffsetStart, UINT_PTR cbOffsetEnd)
{
    // NOTE: This function does not work if this file is memory-mapped.
    DWORD dw{};
    FILE_ZERO_DATA_INFORMATION fzdi{};
    fzdi.FileOffset.QuadPart = cbOffsetStart;
    fzdi.BeyondFinalZero.QuadPart = cbOffsetEnd + 1;
    return(::DeviceIoControl(hfile, FSCTL_SET_ZERO_DATA, (LPVOID)&fzdi, sizeof(fzdi), NULL, 0, &dw, NULL));
}


int WINAPI GMMF(HINSTANCE, HINSTANCE, LPSTR, int)
/*
Module name: GMMF.cpp
Written by: Jeffrey Richter
Purpose:    Function prototypes for using growable memory-mapped files.
*/
{
    char szPathname[] = "C:\\GMMF.";

    // Create the file
    HANDLE hfile = CreateFileA(szPathname, GENERIC_READ | GENERIC_WRITE,
                               0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Create a GMMF using the file (set the maximum size here too)
    CGMMF gmmf(hfile, 10 * 1024 * 1024);

    // Read bytes from the file (0s are returned)
    for (int x = 0; x < 10 * 1024 * 1024; x += 1024) {
        if (gmmf[x] != 0) DebugBreak();
    }

    // Write bytes to the file (clusters are allocated as necessary).
    for (int x = 0; x < 100; x++) {
        gmmf[8 * 1024 * 1024 + x] = (BYTE)x;
    }

    // These lines just prove to us what's going on
    DWORD dw = GetFileSize(hfile, NULL);
    // This returns the logical size of the file.

    // Get the actual number of bytes allocated in the file
    dw = GetCompressedFileSizeA(szPathname, NULL);
    // This returns 0 because the data has not been written to the file yet.

    // Force the data to be written to the file
    FlushViewOfFile(gmmf, 10 * 1024 * 1024);

    // Get the actual number of bytes allocated in the file
    dw = GetCompressedFileSizeA(szPathname, NULL);
    // This returns the size of a cluster now

    // Normally the destructor causes the file-mapping to close.
    // But, in this case, we wish to force it so that we can reset 
    // a portion of the file back to all zeroes.
    gmmf.ForceClose();

    // We call ForceClose above because attempting to zero a portion of the 
    // file while it is mapped, causes DeviceIoControl to fail with error 
    // code 0x4C8 (ERROR_USER_MAPPED_FILE: "The requested operation cannot 
    // be performed on a file with a user-mapped section open.")
    CGMMF::SetToZero(hfile, 0, 2 * 1024 * 1024);

    // We no longer need access to the file, close it.
    CloseHandle(hfile);

    return(0);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
