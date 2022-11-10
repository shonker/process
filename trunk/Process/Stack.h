/*
本文内容：栈，帧（函数）相关的。

核心内容：栈回溯。
*/


/*
检查堆栈跟踪
项目
2022/10/18
1 个参与者

调用 堆栈 包含线程进行的函数调用的数据。 
每个函数调用的数据称为堆栈 帧 ，包括返回地址、传递给函数的参数以及函数的局部变量。 
每次调用函数时，新的堆栈帧将推送到堆栈顶部。 
当该函数返回时，堆栈帧从堆栈中弹出。

每个线程都有自己的调用堆栈，表示该线程中调用。

若要获取堆栈跟踪，请使用 GetStackTrace 和 GetContextStackTrace 方法。 
可以使用 OutputStackTrace 和 OutputContextStackTrace 打印堆栈跟踪。

https://learn.microsoft.com/zh-cn/windows-hardware/drivers/debugger/examining-the-stack-trace
*/


/*
StackWalk function
StackWalk64 function
StackWalkEx function

https://learn.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-stackwalk
*/


#pragma once

class Stack
{

};

