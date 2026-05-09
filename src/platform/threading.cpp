#include "threading.h"
namespace aether {
ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                { std::unique_lock<std::mutex> lock(queueMutex);
                  condition.wait(lock, [this] { return stop || !tasks.empty(); });
                  if (stop && tasks.empty()) return;
                  task = std::move(tasks.front()); tasks.pop(); }
                task();
            }
        });
    }
}
ThreadPool::~ThreadPool() {
    { std::unique_lock<std::mutex> lock(queueMutex); stop = true; }
    condition.notify_all();
    for (auto& worker : workers) worker.join();
}
void ThreadPool::enqueue(std::function<void()> task) {
    { std::unique_lock<std::mutex> lock(queueMutex); tasks.push(std::move(task)); }
    condition.notify_one();
}
} // namespace aether
