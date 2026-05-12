#include "timer.h"
namespace vge {
void Timer::start() { startTime = std::chrono::high_resolution_clock::now(); }
float Timer::elapsedSeconds() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float>(now - startTime).count();
}
} // namespace vge
