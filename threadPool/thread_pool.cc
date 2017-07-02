#include "thread_pool.h"

ThreadPool::ThreadPool(int num):thread_num(num), isrunning(true){}
ThreadPool::~ThreadPool(){
    if(isrunning)
        stop();
}

bool ThreadPool::start(){
    for(size_t i = 0; i < thread_num; ++i){
        //绑定一个对象成员函数时，需要显式指明，并传入this指针
        threads.push_back(std::make_shared<std::thread>(std::bind(&ThreadPool::work, this));
    }
    return true;
}
bool ThreadPool::stop(){
    if(isrunning){
        isrunning = false;
        for(size_t i = 0; i < thread_num; ++i){
            auto t = threads[i];
            if(t->joinable())
                t->join();
        }
    }
    return true;
}

bool ThreadPool::append(Task task){
    if(isrunning){
        std::lock_guard<std::mutex> locker(m);
        tasks.push_front(task);
        cond.notify_one();
    }
    return true;
}

void ThreadPool::work(){
    while(isrunning){
        Task task = nullptr;
        {
            std::unique_lock<std::mutex> locker(m);
            if(tasks.empty())
                cond.wait(locker, [&]{return !tasks.empty();});
            if(!tasks.empty()){
                task = tasks.front();
                tasks.pop_front();
            }
        }
        task();
    }
}
