#pragma once


class Thread
{

};


EXTERN_C
__declspec(dllexport)
BOOL WINAPI ListProcessThreads(DWORD dwOwnerPID);
