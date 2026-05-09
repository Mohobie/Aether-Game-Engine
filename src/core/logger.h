#pragma once
#include <string>

namespace vge {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class Logger {
private:
    LogLevel logLevel;
    bool logToFile;
    
    Logger();
    
public:
    ~Logger();
    
    static Logger& GetInstance();
    
    void SetLogLevel(LogLevel level);
    void Log(LogLevel level, const std::string& message);
    
    static void Info(const std::string& msg);
    static void Error(const std::string& msg);
    static void Debug(const std::string& msg);
};

} // namespace vge
