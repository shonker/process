#include "Session.h"


void QuerySessionInformation(IN DWORD SessionId, IN WTS_INFO_CLASS WTSInfoClass)
/*


https://learn.microsoft.com/en-us/windows/win32/api/wtsapi32/nf-wtsapi32-wtsquerysessioninformationa
https://learn.microsoft.com/en-us/windows/win32/api/wtsapi32/ne-wtsapi32-wts_info_class
https://learn.microsoft.com/en-us/windows/win32/api/wtsapi32/ne-wtsapi32-wts_connectstate_class
https://learn.microsoft.com/en-us/windows/win32/api/wtsapi32/ns-wtsapi32-wts_client_address
https://learn.microsoft.com/en-us/windows/win32/api/wtsapi32/ns-wtsapi32-wts_client_display
*/
{
    LPWSTR ppBuffer = nullptr;
    DWORD pBytesReturned = 0;
    HANDLE hServer = WTS_CURRENT_SERVER_HANDLE;

    BOOL b = WTSQuerySessionInformationW(hServer, SessionId, WTSInfoClass, &ppBuffer, &pBytesReturned);
    if (!b) {
        printf("WTSQuerySessionInformationW, WTSInfoClass:%d, LastError:%d.\n", WTSInfoClass, GetLastError());
        return;
    }

    switch (WTSInfoClass) {
    case WTSInitialProgram:
        printf("InitialProgram:%ls.\n", ppBuffer);
        break;
    case WTSApplicationName:
        printf("ApplicationName:%ls.\n", ppBuffer);
        break;
    case WTSWorkingDirectory:
        printf("WorkingDirectory:%ls.\n", ppBuffer);
        break;
    case WTSOEMId:
        printf("This value is not used.\n");
        break;
    case WTSSessionId:
        printf("SessionId:%d.\n", *(PULONG)ppBuffer);
        break;
    case WTSUserName:
        printf("UserName:%ls.\n", ppBuffer);
        break;
    case WTSWinStationName:
        printf("WinStationName:%ls.\n", ppBuffer);
        break;
    case WTSDomainName:
        printf("DomainName:%ls.\n", ppBuffer);
        break;
    case WTSConnectState:
    {
        WTS_CONNECTSTATE_CLASS ConnectState = *(WTS_CONNECTSTATE_CLASS *)ppBuffer;
        switch (ConnectState) {
        case WTSActive:
            printf("ConnectState:User logged on to WinStation.\n");
            break;
        case WTSConnected:
            printf("ConnectState:WinStation connected to client.\n");
            break;
        case WTSConnectQuery:
            printf("ConnectState:In the process of connecting to client.\n");
            break;
        case WTSShadow:
            printf("ConnectState:Shadowing another WinStation.\n");
            break;
        case WTSDisconnected:
            printf("ConnectState:WinStation logged on without client.\n");
            break;
        case WTSIdle:
            printf("ConnectState:Waiting for client to connect.\n");
            break;
        case WTSListen:
            printf("ConnectState:WinStation is listening for connection.\n");
            break;
        case WTSReset:
            printf("ConnectState:WinStation is being reset.\n");
            break;
        case WTSDown:
            printf("ConnectState:WinStation is down due to error.\n");
            break;
        case WTSInit:
            printf("ConnectState:WinStation in initialization.\n");
            break;
        default:
            printf("ConnectState:%d.\n", ConnectState);
            break;
        }

        break;
    }
    case WTSClientBuildNumber:
        printf("ClientBuildNumber:%d.\n", *(PULONG)ppBuffer);
        break;
    case WTSClientName:
        printf("ClientName:%ls.\n", ppBuffer);
        break;
    case WTSClientDirectory:
        printf("ClientDirectory:%ls.\n", ppBuffer);
        break;
    case WTSClientProductId:
        printf("ClientProductId:%d.\n", *(PUSHORT)ppBuffer);
        break;
    case WTSClientHardwareId:
        //This option is reserved for future use. 
        //WTSQuerySessionInformation will always return a value of 0.
        printf("ClientHardwareId:%d.\n", *(PULONG)ppBuffer);
        break;
    case WTSClientAddress://断开时，这里的数据无效。
    {
        PWTS_CLIENT_ADDRESS ClientAddress = (PWTS_CLIENT_ADDRESS)ppBuffer;

        switch (ClientAddress->AddressFamily) {
        case AF_UNSPEC:
        {
            printf("ClientAddress:AF_UNSPEC.\n");
            break;
        }
        case AF_INET:
        {
            char ipv4[32] = {0};
            InetNtopA(AF_INET, &ClientAddress->Address[2], ipv4, sizeof(ipv4));
            printf("ClientAddress:%s.\n", ipv4);
            break;
        }
        case AF_INET6:
        {
            char ipv6[32] = {0};
            InetNtopA(AF_INET6, &ClientAddress->Address, ipv6, sizeof(ipv6));
            printf("ClientAddress:%s.\n", ipv6);
            break;
        }
        case AF_IPX:
            printf("AddressFamily:AF_IPX.\n");
            break;
        case AF_NETBIOS:
            printf("AddressFamily:AF_NETBIOS.\n");
            break;
        default:
            printf("AddressFamily:%d.\n", ClientAddress->AddressFamily);
            break;
        }

        break;
    }
    case WTSClientDisplay:
    {
        PWTS_CLIENT_DISPLAY ClientDisplay = (PWTS_CLIENT_DISPLAY)ppBuffer;

        printf("ClientDisplay:HorizontalResolution:%d, VerticalResolution:%d, ColorDepth:%d.\n",
               ClientDisplay->HorizontalResolution,
               ClientDisplay->VerticalResolution,
               ClientDisplay->ColorDepth);
        break;
    }
    case WTSClientProtocolType:
    {
        USHORT ClientProtocolType = *(PUSHORT)ppBuffer;

        switch (ClientProtocolType) {
        case 0:
            printf("ClientProtocolType:The console session.\n");
            break;
        case 1:
            printf("ClientProtocolType:retained for legacy purposes.\n");
            break;
        case 2:
            printf("ClientProtocolType:The RDP protocol.\n");
            break;
        default:
            printf("ClientProtocolType:%d.\n", ClientProtocolType);
            break;
        }

        break;
    }
    case WTSIdleTime:
    {
        //PWTSINFOA
        printf("IdleTime.\n");
        break;
    }
    case WTSLogonTime:
        printf("LogonTime.\n");
        break;
    case WTSIncomingBytes:
        printf("IncomingBytes.\n");
        break;
    case WTSOutgoingBytes:
        printf("OutgoingBytes.\n");
        break;
    case WTSIncomingFrames:
        printf("IncomingFrames.\n");
        break;
    case WTSOutgoingFrames:
        printf("OutgoingFrames.\n");
        break;
    case WTSClientInfo:
        printf("ClientInfo.\n");
        break;
    case WTSSessionInfo:
        printf("SessionInfo.\n");
        break;
    case WTSSessionInfoEx:
        printf("SessionInfoEx.\n");
        break;
    case WTSConfigInfo:
        printf("ConfigInfo.\n");
        break;
    case WTSValidationInfo:
        printf("This value is not supported.\n");
        break;
    case WTSSessionAddressV4:
    {
        PWTS_SESSION_ADDRESS SessionAddressV4 = (PWTS_SESSION_ADDRESS)ppBuffer;

        switch (SessionAddressV4->AddressFamily) {
        case AF_UNSPEC:
        {
            printf("SessionAddressV4:AF_UNSPEC.\n");
            break;
        }
        case AF_INET:
        {
            char ipv4[32] = {0};
            InetNtopA(AF_INET, &SessionAddressV4->Address[2], ipv4, sizeof(ipv4));
            printf("SessionAddressV4:%s.\n", ipv4);
            break;
        }
        case AF_INET6:
        {
            char ipv6[32] = {0};
            InetNtopA(AF_INET6, &SessionAddressV4->Address, ipv6, sizeof(ipv6));
            printf("SessionAddressV4:%s.\n", ipv6);
            break;
        }
        case AF_IPX:
            printf("SessionAddressV4:AF_IPX.\n");
            break;
        case AF_NETBIOS:
            printf("SessionAddressV4:AF_NETBIOS.\n");
            break;
        default:
            printf("SessionAddressV4:%d.\n", SessionAddressV4->AddressFamily);
            break;
        }

        break;
    }
    case WTSIsRemoteSession:
    {
        /*
        Determines whether the current session is a remote session.
        The WTSQuerySessionInformation function returns a value of TRUE to indicate that the current session is a remote session,
        and FALSE to indicate that the current session is a local session. This value can only be used for the local machine,
        so the hServer parameter of the WTSQuerySessionInformation function must contain WTS_CURRENT_SERVER_HANDLE.
        */

        BOOL IsRemoteSession = *(PBOOL)ppBuffer;

        printf("IsRemoteSession:%d.\n", IsRemoteSession);

        break;
    }
    default:
        printf("WTSInfoClass:%d.\n", WTSInfoClass);
        break;
    }

    WTSFreeMemory(ppBuffer);
}


void QuerySessionInformations(IN DWORD SessionId)
/*

*/
{
    for (int i = WTSInitialProgram; i <= WTSIsRemoteSession; i++) {
        QuerySessionInformation(SessionId, (WTS_INFO_CLASS)i);
    }
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
/*

https://learn.microsoft.com/zh-cn/windows/win32/termserv/wm-wtssession-change
*/
{
    switch (uMsg) {    
    case WM_WTSSESSION_CHANGE:
    {
        DWORD SessionId = (DWORD)lParam;
        bool IsQuerySessionInformations = false;

        switch (wParam) {
        case WTS_CONSOLE_CONNECT:
            printf("会话%d：已连接到控制台终端或RemoteFX会话.\n", SessionId);
            break;
        case WTS_CONSOLE_DISCONNECT:
            printf("会话%d：与控制台终端或RemoteFX会话断开连接.\n", SessionId);
            break;
        case WTS_REMOTE_CONNECT:
            printf("会话%d：已连接到远程终端.\n", SessionId);
            IsQuerySessionInformations = true;
            break;
        case WTS_REMOTE_DISCONNECT:
            printf("会话%d：已与远程终端断开连接.\n", SessionId);
            IsQuerySessionInformations = true;
            break;
        case WTS_SESSION_LOGON:
            printf("会话%d：已登录.\n", SessionId);
            break;
        case WTS_SESSION_LOGOFF:
            printf("会话%d：已注销.\n", SessionId);
            break;
        case WTS_SESSION_LOCK:
            printf("会话%d：已被锁定.\n", SessionId);
            break;
        case WTS_SESSION_UNLOCK:
            printf("会话%d：已解锁.\n", SessionId);
            break;
        case WTS_SESSION_REMOTE_CONTROL://若要确定状态，请调用 GetSystemMetrics 并检查 SM_REMOTECONTROL 指标。
            printf("会话%d：已更改其远程控制状态.\n", SessionId);
            IsQuerySessionInformations = true;
            break;
        case WTS_SESSION_CREATE:
            printf("会话%d：保留供将来使用.\n", SessionId);
            break;
        case WTS_SESSION_TERMINATE:
            printf("会话%d：保留供将来使用.\n", SessionId);
            break;
        default:
            printf("会话%d：Param:%d.\n", SessionId, (DWORD)wParam);
            break;
        }

        if (IsQuerySessionInformations) {
            QuerySessionInformations(SessionId);
        }

        printf("\n\n\n");

        break;
    }
    case WM_DESTROY:
        WTSUnRegisterSessionNotification(hWnd);
        PostQuitMessage(0);
        break;
    default:
        return(DefWindowProc(hWnd, uMsg, wParam, lParam));
    }

    return(0);
}


void RegisterSessionNotification()
/*

https://learn.microsoft.com/zh-cn/windows/win32/api/wtsapi32/nf-wtsapi32-wtsregistersessionnotification
*/
{
    WNDCLASSEX sWndClassEx = {
        sizeof(WNDCLASSEX),
        CS_HREDRAW | CS_VREDRAW,
        WindowProc,
        0,
        0,
        GetModuleHandle(0),
        0,
        LoadCursor(0,IDC_ARROW),
      (HBRUSH)GetStockObject(WHITE_BRUSH),// (HBRUSH)6,
        0,
        L"correy",
        0
    };

    ATOM a = RegisterClassEx(&sWndClassEx);
    if (!a) {
        printf("LastError:%d.\n", GetLastError());
        return;
    }

    HWND hWnd = CreateWindowEx(0,
                               L"correy",
                               L"correy",
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               0,
                               0,
                               GetModuleHandle(0),
                               0);
    ShowWindow(hWnd, 1);

    BOOL b = WTSRegisterSessionNotification(hWnd, NOTIFY_FOR_ALL_SESSIONS);

    MSG sMsg;
    while (GetMessage(&sMsg, nullptr, 0, 0)) {
        DispatchMessage(&sMsg);
    }

    ExitProcess(0);
}
