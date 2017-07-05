#include "ThreadPool.h"
#include <assert.h>

ThreadPool::ThreadPool(const std::string& name)
  : _m(),
    _name(name),
    _maxQueueSize(0),
    _running(false)
{
}

ThreadPool::~ThreadPool(){
    if(_running)
        stop();
}

void ThreadPool::start(int num){
    assert(_threads.empty());
    _running = true;
    _threads.reserve(num);
    for(int i = 0; i < num; ++i){
        _threads.push_back(std::make_shared<std::thread>(std::bind(&ThreadPool::work, this)));
    }
}

void ThreadPool::stop(){
    {
        std::unique_lock<std::mutex> locker(_m);
        _running = false;
        _notEmpty.notify_all();
    }
    for(int i = 0 ;i < _threads.size(); ++i){
        auto t = _threads[i];
        if(t->joinable()){
            t->join();
        }
    }
}

size_t ThreadPool::queueSize() const{
    std::lock_guard<std::mutex> locker(_m);
    return _queue.size();
}

void ThreadPool::append(const Task& task){
    if(_threads.empty()){
        task();
    }
    else{
        std::unique_lock<std::mutex> locker(_m);
        while(isFull()){
            _notFull.wait(locker);
        }
        assert(!isFull());
        _queue.push_back(task);
        _notEmpty.notify_one();
    }
    
}

void ThreadPool::append(Task&& task){
    std::unique_lock<std::mutex> locker(_m);
    while(isFull()){
        _notFull.wait(locker);
    }
    assert(!isFull());
    _queue.push_back(std::move(task));
    _notEmpty.notify_one();
}

ThreadPool::Task ThreadPool::takeOne(){
    std::unique_lock<std::mutex> locker(_m);
    //while or if, maybe all are ok
    if(_queue.empty() && _running){
        //printf("is empty, waiting..\n");
        _notEmpty.wait(locker);
    }
    Task task;
    //if queue is not empty, wheather the pool is running or not, run the task
    if(!_queue.empty()){
        //printf("not empty, take one\n");
        task = _queue.front();
        _queue.pop_front();
        if(_maxQueueSize > 0){
            _notFull.notify_one();
        }
    }
    return task;
}

bool ThreadPool::isFull() const{
    //lock has been kept before this func is invoked, just check it
    assert(!_m.try_lock());
    return _maxQueueSize > 0 && _queue.size() >= _maxQueueSize;
}

void ThreadPool::work(){
    while(_running){
        Task task = takeOne();
        if(task)
            task();
    }
}
