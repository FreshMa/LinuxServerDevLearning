#include <thread>
#include <cassert>
#include <fstream>
#include <cstring>

#include <sys/inotify.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>

#include "InotifyReload.h"

const int MAXBUFSIZE=16383;

int InotifyReload::Open() {
    int fd = inotify_init();
    if (fd == -1) {
        return -1;
    }
    inotify_fd = fd;
    m_isrunning = true;

    std::thread t(&InotifyReload::RunWatch, this);
    t.detach();
    return 0;
}

int InotifyReload::Close() {
    m_isrunning = false;
    if (inotify_fd != -1) {
        close(inotify_fd);
    }
    return 0;
}

std::string InotifyReload::GetContent(const std::string& file_name) {
    if (content_map.find(file_name) == content_map.end()) {
        return "";
    }
    std::unique_lock<std::mutex> locker(con_mutex);
    std::string con = content_map.at(file_name);
    return con;
}

//register
int InotifyReload::Add(const std::string& file_name, const reloadFn& fn) {
    if (!m_isrunning) return 1;
    assert(inotify_fd != -1);

    int watch_id = -1;
    if ((watch_id = inotify_add_watch(inotify_fd, file_name.c_str(), IN_MODIFY)) == -1) {
        perror("add failed, ");
        return -1;
    }
    printf("add file:%s, watch_id:%d\n", file_name.c_str(), watch_id);

    {
        ReloadData data = {file_name, fn};
        std::unique_lock<std::mutex> locker(m_mutex);
        m_map[watch_id] = data;
    }
    //first time reload 
    std::string file_content = loadFile(file_name);
    if (fn) {
        fn(file_content);
    } else {
        std::unique_lock<std::mutex> locker(con_mutex);
        content_map[file_name] = file_content;
    }

    return 0;
}

void InotifyReload::RunWatch() {
    FD_ZERO(&fds);
    FD_SET(inotify_fd, &fds);

    std::vector<struct inotify_event> events;
    while(m_isrunning) {
        int ret = select(1024, &fds, NULL, NULL, NULL);
        if (ret == -1) {
            perror("select return with -1 ");
            return;
        }
        readEvents(events);
        handleEvents(events);

        //reset
        events.clear();
        FD_ZERO(&fds);
        FD_SET(inotify_fd, &fds);
    }
}

int InotifyReload::readEvents(std::vector<struct inotify_event>& events) {
    assert(inotify_fd != -1);
    if (!m_isrunning) return 1;

    char buf[MAXBUFSIZE] = {0};

    int r = read(inotify_fd, buf, sizeof(buf));
    if (r == -1 && errno != EAGAIN) {
        perror("read failed");
        return -1;
    }

    if (r <= 0) {
        return -1;
    }

    size_t idx = 0, event_size = 0;
    struct inotify_event *pevent;
    while(idx < r) {
       pevent = (struct inotify_event*)&buf[idx]; 
       event_size = offsetof(struct inotify_event, name) + pevent->len;

       char event_buf[MAXBUFSIZE];
       memmove(event_buf, pevent, event_size);
       struct inotify_event *event = (inotify_event*)event_buf;

       events.push_back(*event);
       idx+=event_size;
    }
    return 0;
}

//when select return, mean some files were modified
int InotifyReload::handleEvents(const std::vector<struct inotify_event>& events) {
    if (events.size() == 0) {
        return -1;
    }

    std::string content;
    for (int i = 0; i < events.size(); ++i) {
        struct inotify_event event = events[i];

        if (m_map.find(event.wd) == m_map.end()) {
            printf("[handleEvents] m_map find watch_id:%d failed\n", event.wd);
            continue;
        }

        std::string file_name = m_map.at(event.wd).file_name;

        reloadFn fn = m_map.at(event.wd).fn;
        switch (event.mask) {
            case IN_MODIFY:
                content = loadFile(file_name);
                if (fn != nullptr) {
                    fn(content);
                } else {
                    std::unique_lock<std::mutex> locker(con_mutex);
                    content_map[file_name] = content;
                }
                break;
            default:
                printf("unsupported mask:%x\n", event.mask);
                break;
        }
    }
    return 0;
}

std::string InotifyReload::loadFile(const std::string& file_name) {
    std::ifstream ifs(file_name, std::ifstream::in);
    std::string con, line;
    while(getline(ifs, line, '\n')) {
        con += line;
        con += "\n";
    }
    con.pop_back();
    return con;
}
