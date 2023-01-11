/*
�������ݣ�ջ��֡����������صġ�

�������ݣ�ջ���ݡ�
*/


/*
����ջ����
��Ŀ
2022/10/18
1 ��������

���� ��ջ �����߳̽��еĺ������õ����ݡ� 
ÿ���������õ����ݳ�Ϊ��ջ ֡ ���������ص�ַ�����ݸ������Ĳ����Լ������ľֲ������� 
ÿ�ε��ú���ʱ���µĶ�ջ֡�����͵���ջ������ 
���ú�������ʱ����ջ֡�Ӷ�ջ�е�����

ÿ���̶߳����Լ��ĵ��ö�ջ����ʾ���߳��е��á�

��Ҫ��ȡ��ջ���٣���ʹ�� GetStackTrace �� GetContextStackTrace ������ 
����ʹ�� OutputStackTrace �� OutputContextStackTrace ��ӡ��ջ���١�

https://learn.microsoft.com/zh-cn/windows-hardware/drivers/debugger/examining-the-stack-trace
*/


/*
StackWalk function
StackWalk64 function
StackWalkEx function

�ĵã�
StackWalk�ĵ��������������GetCurrentThread()��ֻ��ѭ��һ���Ρ�
�������SuspendThread�Ǹ��̡߳�

https://learn.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-stackwalk
https://learn.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-stackwalk64
https://learn.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-stackwalkex
*/


/*
If you want to print out a stack trace without having an exception, 
you��ll have to get the local context with the RtlCaptureContext() function.

https://spin.atomicobject.com/2013/01/13/exceptions-stack-traces-c/
*/


#pragma once


class Stack
{

};
