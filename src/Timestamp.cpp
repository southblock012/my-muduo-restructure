#include "Timestamp.h"
#include <chrono>
#include <iostream>
#include <string>
#include <time.h>
#include <stdio.h>

Timestamp::Timestamp():microSecondsSinceEpoch_(0)
{
}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    :microSecondsSinceEpoch_(microSecondsSinceEpoch)
{
}

Timestamp Timestamp::Now()
{
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    // 转换为微秒
    auto duration = now.time_since_epoch();
    int64_t micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    return Timestamp(micros);
}

std::string Timestamp::ToString() const
{   
    char buf[128]={0};

    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / 1000000);
    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % 1000000);
    tm *tm_time = localtime(&seconds);

    snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d.%06d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec,
             microseconds);
    return std::string(buf);
}

// int main()
// {
//     Timestamp now = Timestamp::Now();
//     std::cout << now.ToString() << std::endl;
//     return 0;
// }
