#include "Performance.h"


void PerformanceTest()
{
    //CollectPerformanceData(L"\\Processor(*)\\% Processor Time");
    //CollectPerformanceData(L"\\Processor(*)\\% User Time");

    //CollectPerformanceData(L"\\Processor Information(*)\\% Processor Time");//类似：L"\\Processor(*)\\% Processor Time"

    //CollectPerformanceDatas(L"\\Process(*)\\% Processor Time");
    //CollectPerformanceDatas(L"\\Process(notepad)\\% Processor Time");
    CollectPerformanceDatas(L"\\Process(svchost#1)\\% Processor Time");//是索引不是PID，从1开始。
    //CollectPerformanceDatas(L"\\Process(*)\\Working Set");

    //CollectPerformanceData(L"\\Memory\\% Committed Bytes In Use");

    //CollectPerformanceData(L"\\PhysicalDisk(*)\\% Disk Time");
    //CollectPerformanceData(L"\\PhysicalDisk(*)\\% Idle Time");

    //EnumeratingProcessObjects();
    //EnumObjectItems(L"Process");

    //EnumCountersObjects();
    //EnumCountersMachines();
}
