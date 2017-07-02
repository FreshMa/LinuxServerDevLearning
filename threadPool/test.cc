#include "thread_pool.h"

void fun(int num){
    printf("hello from %d\n", num);
    return;
}

int main(){
    ThreadPool pool(10);
    pool.start();
    for(size_t i = 0; i < 20; ++i){
        pool.append(std::bind(fun,i));
    }
    return 0;
}
