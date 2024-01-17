#pragma once


#define ARRAY_SIZE 1024


EXTERN_C
__declspec(dllexport) 
BOOL WINAPI ListProcessModules(DWORD dwPID);


EXTERN_C
__declspec(dllexport)
int WINAPI EnumeratingAllDeviceDrivers();
