/*
 Performance Data Helper (PDH) library

 显示和查询基于PID/TID的性能计数器。
 HKLM\System\CurrentControlSet\Services\Perfproc\Performance
 ProcessNameFormat 或 ThreadNameFormat
 谨记：无需重启操作系统即可生效。应该适用所有的版本。
 https://learn.microsoft.com/zh-cn/windows/win32/perfctrs/handling-duplicate-instance-names
*/

/*
关于PdhGetFormattedCounterValue返回PDH_CALC_NEGATIVE_DENOMINATOR的处理。

https://learn.microsoft.com/zh-cn/windows/win32/perfctrs/pdh-error-codes
https://learn.microsoft.com/zh-cn/windows/win32/perfctrs/checking-pdh-interface-return-values
https://github.com/DerellLicht/derbar/blob/master/system.cpp
https://support.microfocus.com/kb/doc.php?id=7010545

 In these situations, you have an option to ignore this return value and retry a bit later like 1 second to get the new values.
*/

#pragma once

#include <pdh.h> //不能与LMErrlog.h同时被包含。
#pragma comment (lib,"pdh.lib")
