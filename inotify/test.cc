#include "InotifyReload.h"
#include <stdio.h>
#include <string>
#include <unistd.h>

using namespace std;

struct A {
public:
    static A* Instance() {
        static A a;
        return &a;
    }
    void procReload(const string& content) {
        if (content.length() > 0) {
            printf("[A] first char:%c\n", content[0]);
        } else {
            printf("[A] empty content\n");
        }
    }
private:
    A() {}
};

void testAdd(const string&, reloadFn = nullptr);
void test(int secs) {
    if (InotifyReload::instance()->Open() != 0) {
        printf("open faield\n");
        return;
    }

    //test not exist
    testAdd("./xxx.cc");

    testAdd("./1.txt");

    testAdd("./2.txt", std::bind(&A::procReload, A::Instance(), std::placeholders::_1));
    sleep(secs);
}

void testAdd(const string& file_name, reloadFn fn) {
    if (InotifyReload::instance()->Add(file_name, fn) != 0) {
        printf("[testAdd] add %s failed\n", file_name.c_str());
        return;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("need an arg to speicify the block secs\n");
        return -1;
    }
    test(atoi(argv[1]));
}
