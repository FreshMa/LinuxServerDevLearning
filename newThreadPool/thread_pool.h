#ifndef MY_THREAD_POOL_H
#define MY_THREAD_POOL_H

#include <condition_variable>
#include <mutex>
#include <deque>
#include <functional>
#include <thread>
#include <vector>
#include <memory>
#include <deque>
#include <string>

class ThreadPool{
public:
    using Task = std::function<void(void)>;
    explicit ThreadPool(const std::string& name = std::string("default"));
    ~ThreadPool();

    void setMaxQueueSize(size_t size){
        _maxQueueSize = size;
    }

    void start(int numThreads);
    void stop();

    std::string& name(){
        return _name;
    }

    size_t queueSize() const;
    void append(const Task& f);
    void append(Task&& f);

private:
    bool isFull() const;
    void work();
    Task takeOne();

    mutable std::mutex _m;
    std::condition_variable _notEmpty;
    std::condition_variable _notFull;
    std::string _name;
    Task _threadInitCallback;
    std::vector<std::shared_ptr<std::thread>> _threads;
    std::deque<Task> _queue;
    size_t _maxQueueSize;
    bool _running;
};

#endif
