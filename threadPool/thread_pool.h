#ifndef _MY_THREAD_POOL_H_
#define _MY_THREAD_POOL_H_

#include <cstdio>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <vector>
#include <list>
#include <functional>

class ThreadPool{
public:
    using Task = std::function<void(void)>;
    ThreadPool(int num);
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator= (const ThreadPool& rhs) = delete;

    void start();
    void stop();
    void append(const Task& task);
    void append(Task&& task);
private:
    void work();
    bool isrunning;
    int thread_num;
    std::mutex m;
    std::condition_variable cond;
    std::vector<std::shared_ptr<std::thread>> threads;
    std::list<Task> tasks;
};

#endif
