#pragma once

//#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN 1             // 从 Windows 头文件中排除极少使用的内容
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_DCOM
//#define SECURITY_WIN32
#define _CRT_SECURE_NO_WARNINGS

//#pragma warning(disable:4005) //宏重定义

// Windows 头文件
#include <Winsock2.h>
#include <windows.h>
#include <strsafe.h>
#include <assert.h>
#include <crtdbg.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <winioctl.h>
#include <string.h>
#include <fltuser.h>
#include <locale.h>
#include <Lmserver.h>
#include <stdarg.h>
#include <wincrypt.h>
#include <intrin.h>
#include <TlHelp32.h>
#include <aclapi.h>
#include <Softpub.h>
#include <mscat.h>
#include <WinUser.h>
#include <direct.h>
#include <sddl.h>
#include <ws2tcpip.h>
#include <fwpsu.h>
#include <atlbase.h>
#include <mbnapi.h>
#include <iostream>
#include <netfw.h>
#include <objbase.h>
#include <oleauto.h>
#include <atlconv.h>
#define _WS2DEF_
#include <mstcpip.h>
#include <Intshcut.h>
#include <atlstr.h>
#include <wbemidl.h>
#include <dbt.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <conio.h>
#include <AccCtrl.h>
#include <Mq.h>
#include <wincred.h>
#include <lmcons.h>
#include <netioapi.h>
#include <ole2.h>
//#include <urlmon.h>
//#include <winsafer.h>
#include <pdhmsg.h>
//#include <ShlDisp.h>
#include <Shlobj.h>
//#include <ShlGuid.h>

//#include <shobjidl_core.h>
//#include <shlobj_core.h>
#include <exdisp.h>
#include <comutil.h>

//几个内核相关的头文件。
/*
因为这几个文件有冲突，所以要做到不冲突，要做到：
1.不冲突。
2.不在同一个文件中包含他们。
3.也不再递归的，隐含的包含中包含他们。
4.只能做到不冲突，所以它们的功能不能合并一个文件，要分开在多个文件中。
5.所以，这里不能包含他们中的任意一个。
6.尽量在头文件中包含它们。
7.最后的绝杀是在实现文件中包含他们。

注意以下5个可同时包含，但不能与下面的任何一个同时包含。
*/
//#include <winioctl.h>
//#include <ntstatus.h> //和winnt.h定义的有重复，注意开关：WIN32_NO_STATUS
//#include <winnt.h>
//#include <NTSecAPI.h>
//#include <LsaLookup.h>
//#include <SubAuth.h> //SubAuth.h和NTSecAPI.h有重复定义的结构。注意开关：_NTDEF_
//#include <winternl.h>
//#include <ntdef.h>
//#include <SubAuth.h>

#include <wincon.h> 
#include <time.h> 
#include <fwpmu.h>
#include <nb30.h>

//几个USB相关的头文件。
#include <initguid.h> //注意前后顺序。
#include <usbioctl.h>
#include <usbiodef.h>
//#include <usbctypes.h>
#include <intsafe.h>
#include <specstrings.h>
#include <usb.h>
#include <usbuser.h>

//  Include the task header file.
#include <rpcsal.h>
#include <mstask.h>
#include <msterr.h>
#include <objidl.h>

# pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "credui.lib")

#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )
#pragma comment(lib, "crypt32.lib")
//#pragma comment (lib,"Url.lib")
#pragma comment(lib, "wbemuuid.lib")
//#pragma comment(lib, "cmcfg32.lib")
#pragma comment(lib,"Mpr.lib")
#pragma comment(lib,"Comctl32.lib")
#pragma comment(lib, "fwpuclnt.lib") 

#include <taskschd.h>
#pragma comment(lib, "taskschd.lib")

#include <VersionHelpers.h>
#pragma comment(lib, "Version.lib") 

#pragma comment(lib,"Netapi32.lib")
#pragma comment (lib,"Secur32.lib")

#include <Rpc.h>
#pragma comment (lib,"Rpcrt4.lib")

#include <bcrypt.h>
#pragma comment (lib, "Bcrypt.lib")

#include <ncrypt.h>
#pragma comment (lib, "Ncrypt.lib")

#include <wintrust.h>
#pragma comment (lib, "wintrust.lib")

#include <Setupapi.h>
#pragma comment (lib,"Setupapi.lib")

//#include <shellapi.h>
#pragma comment (lib,"Shell32.lib")

#include <Shlwapi.h>
#pragma comment (lib,"Shlwapi.lib")

#include <DbgHelp.h>
#pragma comment (lib,"DbgHelp.lib")

#include <psapi.h>
#pragma comment(lib, "Psapi.lib")

#include <Sfc.h>
#pragma comment(lib, "Sfc.lib")

//#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

#include <Wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")

#include <Userenv.h>
#pragma comment(lib, "Userenv.lib")

#include <Sensapi.h>
#pragma comment (lib,"Sensapi.lib")

#include <Wininet.h>
#pragma comment (lib,"Wininet.lib")

#include <pdh.h>
#pragma comment (lib,"pdh.lib")

#include <string>
#include <list>
#include <regex>
#include <map>
#include <set>
#include <iostream>

using namespace std;


//////////////////////////////////////////////////////////////////////////////////////////////////


#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __FILENAMEW__ (wcsrchr(_CRT_WIDE(__FILE__), L'\\') ? wcsrchr(_CRT_WIDE(__FILE__), L'\\') + 1 : _CRT_WIDE(__FILE__))


//////////////////////////////////////////////////////////////////////////////////////////////////
