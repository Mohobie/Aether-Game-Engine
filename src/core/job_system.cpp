#include "job_system.h"
namespace aether {
JobSystem::JobSystem(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    condition.wait(lock, [this] { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
                --pendingTasks;
            }
        });
    }
}
JobSystem::~JobSystem() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (auto& worker : workers) worker.join();
}
void JobSystem::waitForAll() {
    while (pendingTasks > 0) std::this_thread::yield();
}
size_t JobSystem::getPendingTasks() const {
    return pendingTasks.load();
}
} // namespace aether
