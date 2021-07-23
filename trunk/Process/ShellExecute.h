#pragma once


/*
切记这两个文件不可以包含在一个文件中，包括隐含的包含。
因为shellapi.h会把ShellExecute变成ShellExecuteW,
从而导致IShellDispatch2找不到ShellExecute成员。
16:45 2021/7/23
*/
#include <ShlDisp.h>
//#include <shellapi.h>


//////////////////////////////////////////////////////////////////////////////////////////////////
