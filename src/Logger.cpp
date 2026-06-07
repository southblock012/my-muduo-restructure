#include "Logger.h"
#include "Timestamp.h"

#include <iostream>

//获取日志唯一的对象
Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

//设置日志级别
void Logger::setLogLevel(LogLevel level)
{
    logLevel_ = level;
}

//写日志 [级别信息] time : msg
void Logger::log(std::string msg)
{
    Timestamp timestamp = Timestamp::Now();
    std::string time_str = timestamp.ToString();

    switch (logLevel_)
    {
    case INFO:
        std::cout << "[INFO] " << time_str << " " << msg << std::endl;
        break;
    case ERROR:
        std::cerr << "[ERROR] " << time_str << " " << msg << std::endl;
        break;
    case FATAL:
        std::cerr << "[FATAL] " << time_str << " " << msg << std::endl;
        break;
    case DEBUG:
        std::cout << "[DEBUG] " << time_str << " " << msg << std::endl;
        break;
    default:
        break;
    }
}