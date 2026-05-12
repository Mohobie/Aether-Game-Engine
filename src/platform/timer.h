#pragma once
#include <chrono>
namespace vge {
class Timer {
public:
    void start();
    float elapsedSeconds() const;
private:
    std::chrono::high_resolution_clock::time_point startTime;
};
} // namespace vge
