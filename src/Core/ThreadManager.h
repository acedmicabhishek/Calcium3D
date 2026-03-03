#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <iostream>

class ThreadManager {
public:
    static void Init();
    static void Shutdown();

    
    static void ParallelFor(int start, int end, std::function<void(int)> func);

    static bool IsEnabled() { return s_Enabled; }
    static void SetEnabled(bool enabled) { s_Enabled = enabled; }

private:
    static void WorkerThread();

    struct Task {
        std::function<void()> func;
    };

    static std::vector<std::thread> s_Workers;
    static std::queue<Task> s_Tasks;
    static std::mutex s_QueueMutex;
    static std::condition_variable s_Condition;
    static std::atomic<bool> s_Stop;
    static std::atomic<bool> s_Enabled;
    static int s_ThreadCount;
};

#endif 
