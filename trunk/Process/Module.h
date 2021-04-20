#pragma once


class Module
{

};


EXTERN_C
__declspec(dllexport) 
BOOL WINAPI ListProcessModules(DWORD dwPID);
