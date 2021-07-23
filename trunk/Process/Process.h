#pragma once


typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C 
__declspec(dllexport)
BOOL WINAPI IsWow64();
