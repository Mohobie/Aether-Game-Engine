#include "logger.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace vge {

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : logLevel(LogLevel::Debug), logToFile(false) {}

Logger::~Logger() {}

void Logger::SetLogLevel(LogLevel level) {
    logLevel = level;
}

void Logger::Log(LogLevel level, const std::string& message) {
    if (level < logLevel) return;
    
    std::string prefix;
    switch (level) {
        case LogLevel::Debug: prefix = "[DEBUG]"; break;
        case LogLevel::Info: prefix = "[INFO]"; break;
        case LogLevel::Warning: prefix = "[WARN]"; break;
        case LogLevel::Error: prefix = "[ERROR]"; break;
        case LogLevel::Fatal: prefix = "[FATAL]"; break;
    }
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] "
       << prefix << " " << message;
    
    std::cout << ss.str() << std::endl;
}

void Logger::Info(const std::string& msg) {
    GetInstance().Log(LogLevel::Info, msg);
}

void Logger::Error(const std::string& msg) {
    GetInstance().Log(LogLevel::Error, msg);
}

void Logger::Debug(const std::string& msg) {
    GetInstance().Log(LogLevel::Debug, msg);
}

} // namespace vge
