#include "thread_pool.h"
#include <cstdio>
using namespace std;

void fun(int num){
    printf("hello from %d\n", num);
}

void test(){
    //task queue size is 20, and 10 threads in the pool
    ThreadPool pool;
    pool.setMaxQueueSize(20);
    pool.start(10);

    for(int i = 0; i < 50; ++i){
        pool.append(bind(fun,i));
    }

}

int main(){
    test();
}
