/*
功能：性能测试。

PDH的验证要结合性能监视器，不能是任务管理和资源监视器。

任务管理和资源监视器显示的是一致的，尽管资源监视器是基于性能监视器做的。

任务管理器显示的CPU的总是高于性能监视器的值。
但是任务管理器显示的CPU总是大于各个进程的CPU的和（尽管有些进程的CPU不到1%）。

还是有任务管理器（包括资源监视器）里的内存使用率总是大于性能监视器的值

如果要列出Memory下的计数器的所有的完整的名字，可以在ps里运行：
(Get-Counter -ListSet * | where {$_.CounterSetName -eq 'Memory'}).Paths
注意：这个需要等待几分钟。看来脚本真的慢啊！
https://wazuh.com/blog/monitoring-windows-resources-with-performance-counters/

关于PID的信息的思考所搜索的结果：
 Tip
Starting in Windows 10 20H2, you can avoid this issue by using the new Process V2 counterset. 
The Process V2 counterset includes the process ID in the instance name. 
This avoids the inconsistent results that appear with the original Process counterset.
https://learn.microsoft.com/en-us/windows/win32/perfctrs/collecting-performance-data
*/


#pragma once

#include "..\inc\Process.h"
#include "pch.h"

class Performance
{

};

void PerformanceTest();