#pragma once
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
namespace aether {
class JobSystem {
public:
    JobSystem(size_t numThreads = 4);
    ~JobSystem();
    template<typename Func>
    auto submit(Func&& func) -> std::future<decltype(func())> {
        using ReturnType = decltype(func());
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<Func>(func));
        std::future<ReturnType> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return result;
    }
    void waitForAll();
    size_t getPendingTasks() const;
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop = false;
    std::atomic<size_t> pendingTasks{0};
};
} // namespace aether
