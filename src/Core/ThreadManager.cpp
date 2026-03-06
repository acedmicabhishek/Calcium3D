#include "ThreadManager.h"
#include <algorithm>

std::vector<std::thread> ThreadManager::s_Workers;
std::queue<ThreadManager::Task> ThreadManager::s_Tasks;
std::mutex ThreadManager::s_QueueMutex;
std::condition_variable ThreadManager::s_Condition;
std::atomic<bool> ThreadManager::s_Stop(false);
std::atomic<bool> ThreadManager::s_Enabled(true);
int ThreadManager::s_ThreadCount = 0;

void ThreadManager::Init() {
    if (!s_Workers.empty()) return;

    unsigned int cores = std::thread::hardware_concurrency();
    
    s_ThreadCount = std::max(1, (int)(cores * 0.8f));
    s_Stop = false;

    for (int i = 0; i < s_ThreadCount; ++i) {
        s_Workers.emplace_back(WorkerThread);
    }
    
    std::cout << "[ThreadManager] Initialized with " << s_ThreadCount << " worker threads (80% of " << cores << " cores)" << std::endl;
}

void ThreadManager::Shutdown() {
    {
        std::unique_lock<std::mutex> lock(s_QueueMutex);
        s_Stop = true;
    }
    s_Condition.notify_all();
    for (std::thread& worker : s_Workers) {
        if (worker.joinable()) worker.join();
    }
    s_Workers.clear();
}

void ThreadManager::ParallelFor(int start, int end, std::function<void(int)> func) {
    if (start >= end) return;
    
    if (!s_Enabled || s_Workers.empty() || (end - start) < 2) {
        for (int i = start; i < end; ++i) func(i);
        return;
    }

    struct SharedState {
        std::atomic<int> remaining;
        std::mutex mutex;
        std::condition_variable cv;
        SharedState(int count) : remaining(count) {}
    };
    auto state = std::make_shared<SharedState>(end - start);

    {
        std::unique_lock<std::mutex> lock(s_QueueMutex);
        for (int i = start; i < end; ++i) {
            s_Tasks.push({[i, &func, state]() {
                func(i);
                if (state->remaining.fetch_sub(1) == 1) { 
                    std::lock_guard<std::mutex> lock(state->mutex);
                    state->cv.notify_all();
                }
            }});
        }
    }
    s_Condition.notify_all();

    std::unique_lock<std::mutex> lock(state->mutex);
    state->cv.wait(lock, [&state]() { return state->remaining == 0; });
}

void ThreadManager::WorkerThread() {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(s_QueueMutex);
            s_Condition.wait(lock, [] { return s_Stop || !s_Tasks.empty(); });
            if (s_Stop && s_Tasks.empty()) return;
            task = std::move(s_Tasks.front());
            s_Tasks.pop();
        }
        task.func();
    }
}
