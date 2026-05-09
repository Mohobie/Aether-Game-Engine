#pragma once
#include <string>
#include <map>
#include <chrono>
namespace aether {
struct ProfileResult {
    std::string name;
    long long start, end;
    size_t threadID;
};
class Profiler {
public:
    void beginSession(const std::string& name);
    void endSession();
    void writeProfile(const ProfileResult& result);
    static Profiler& getInstance();
private:
    std::string sessionName;
    std::ofstream outputStream;
    int profileCount = 0;
    std::mutex lock;
    bool active = false;
};
class Timer {
public:
    Timer(const std::string& name);
    ~Timer();
    void stop();
private:
    std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    bool stopped = false;
};
} // namespace aether
