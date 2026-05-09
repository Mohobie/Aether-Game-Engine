#include "profiler.h"
#include <fstream>
#include <mutex>
namespace aether {
Profiler& Profiler::getInstance() {
    static Profiler instance;
    return instance;
}
void Profiler::beginSession(const std::string& name) {
    std::lock_guard<std::mutex> guard(lock);
    sessionName = name;
    outputStream.open("profile.json");
    outputStream << "{"otherData": {},"traceEvents":[";
    outputStream.flush();
    profileCount = 0;
    active = true;
}
void Profiler::endSession() {
    std::lock_guard<std::mutex> guard(lock);
    active = false;
    outputStream << "]}";
    outputStream.flush();
    outputStream.close();
    profileCount = 0;
}
void Profiler::writeProfile(const ProfileResult& result) {
    std::lock_guard<std::mutex> guard(lock);
    if (profileCount++ > 0) outputStream << ",";
    outputStream << "{"cat":"function","dur":" << (result.end - result.start)
              << ","name":"" << result.name << "","ph":"X","pid":0,"tid":"
              << result.threadID << ","ts":" << result.start << "}";
    outputStream.flush();
}
Timer::Timer(const std::string& name) : name(name), startTime(std::chrono::high_resolution_clock::now()) {}
Timer::~Timer() { if (!stopped) stop(); }
void Timer::stop() {
    auto endTime = std::chrono::high_resolution_clock::now();
    long long start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
    long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
    Profiler::getInstance().writeProfile({name, start, end, 0});
    stopped = true;
}
} // namespace aether
