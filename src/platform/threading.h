#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
namespace vge {
class ThreadPool {
public:
    ThreadPool(size_t numThreads = 4);
    ~ThreadPool();
    void enqueue(std::function<void()> task);
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop = false;
};
} // namespace vge
