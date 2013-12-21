#include <stdio.h>
#include <tchar.h>
#include <Windows.h>

#include "..\source\bdktime.h"
using BDK::timestamp;

#include "..\source\log.h"

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\bin\\BDK_d.lib")
#else
#pragma comment(lib, "..\\..\\bin\\BDK.lib")
#endif

int _tmain(int argc, _TCHAR* argv[])
{
    timestamp time1 = timestamp::now();
    printf("time1: %s\n", time1.toFormattedString().c_str());
    printf("%I64d, %I64d s, %I64d ms, %s\n", time1.microSecondsSinceEpoch(), 
        time1.microSecondsSinceEpoch() / timestamp::kMicroSecondsPerSecond, 
        time1.microSecondsSinceEpoch() % timestamp::kMicroSecondsPerSecond, 
        time1.toString().c_str());

    Sleep(100);

    timestamp time2 = timestamp::now();
    printf("time2: %s\n", time2.toFormattedString().c_str());

    printf("diff: time2 - time1 : %f\n", BDK::timeDifference(time2, time1));

    printf("time1 < time2  = %d\n", time1 < time2);
    printf("time1 == time2 = %d\n", time1 == time2);

    timestamp time3 = BDK::addTime(time2, 10.001001);
    printf("time3: %s\n", time3.toFormattedString().c_str());
    printf("diff: time3 - time2 : %f\n", BDK::timeDifference(time3, time2));

    SetFileDirectory("C:\\BDKLog");
    SetFileName("BDK");
    LogAssert(1, "test assert success");
    LogAssert(0, "test assert failed");
    while (1) {
        LogTrace("test log\n");
        Sleep(10);
    }

	return 0;
}

