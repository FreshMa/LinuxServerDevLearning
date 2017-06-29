/*
*后台开发一书上的demo
*
*编译跑起来之后可以使用nc命令来测试：
*nc 127.0.0.1 6666
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>

#define IPADDR       "127.0.0.1"
#define PORT         6666
#define MAXSIZE      1024
#define LISTENQ      5
#define FDSIZE       1000
#define EPOLLEVENTS  100

int socket_bind(const char *ip, int port);
void do_epoll(int listenfd);
void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf);
void handle_accept(int epollfd, int listenfd);

//负责读和写
void do_read(int epollfd, int fd, char *buf);
void do_write(int epollfd, int fd, char *buf);

//封装epoll的epoll_ctl,其原型为:
//void epoll_ctl(int epollfd, int OP, int fd, struct epoll_event *event);
void add_event(int epollfd, int fd, int state);
void modify_event(int epollfd, int fd, int state);
void delete_event(int epollfd, int fd, int state);

int main(){
    int listenfd;
    listenfd = socket_bind(IPADDR, PORT);
    listen(listenfd, LISTENQ);
    do_epoll(listenfd);
    return 0;
}

//socket, bind
int socket_bind(const char *ip, int port){
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    //inet_pton converts cstring to struct in_addr* type
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    return listen_fd;
}

void do_epoll(int listenfd){
    int epollfd;
    //用户空间的epoll_event数组，包含event和data成员,data中存放有该事件所属的fd
    struct epoll_event events[EPOLLEVENTS];
    int ret;
    char buf[MAXSIZE];
    memset(buf, 0, MAXSIZE);
    
    //创建fd
    epollfd = epoll_create(FDSIZE);
    //监听listenfd的读事件
    add_event(epollfd, listenfd, EPOLLIN);
    while(1){
        //events将接受从内核返回的事件，ret是事件个数。-1代表阻塞
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        //处理事件
        handle_events(epollfd, events, ret, listenfd, buf);
    }
    close(epollfd);
}

void handle_events(int epollfd, struct epoll_event* events, int num, int listenfd, char *buf){
    int i,fd;
    for(i = 0;i<num; ++i){
        //获取该事件所对应的fd
        fd = events[i].data.fd;
        //如果是listenfd可读，说明有新连接
        if((fd == listenfd)&&(events[i].events & EPOLLIN)){
            handle_accept(epollfd, listenfd);
        }
        //如果是老连接可读
        else if(events[i].events & EPOLLIN){
            do_read(epollfd, fd, buf);
        }
        //如果连接可写
        else if(events[i].events & EPOLLOUT){
            do_write(epollfd, fd, buf);
        }
    }
}

void handle_accept(int epollfd, int listenfd){
    int clifd;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    //与客户端对应的一个socket
    clifd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
    if(clifd == -1){
        perror("accept error\n");
    }
    else{
        //新连接可读
        printf("accept a new client:%s:%d\n",inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
        add_event(epollfd, clifd, EPOLLIN);
    }
}

void do_read(int epollfd, int fd, char *buf){
    int nread;
    nread = read(fd, buf, MAXSIZE);
    //连接失败
    if(nread == -1){
        perror("read error\n");
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    }
    //客户端主动关闭
    else if(nread == 0){
        fprintf(stderr, "client closed\n");
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    }
    //有数据可读，读取数据，并将该fd设置为可写状态，写的数据为它发过来的数据
    else{
        printf("read msg is:%s",buf);
        modify_event(epollfd, fd, EPOLLOUT);
    }
}

void do_write(int epollfd, int fd, char *buf){
    int nwrite;
    nwrite = write(fd, buf, strlen(buf));
    //连接失败
    if(nwrite == -1){
        perror("write error\n");
        close(fd);
        delete_event(epollfd, fd, EPOLLOUT);
    }
    //写完之后监听fd的可读事件
    else
        modify_event(epollfd, fd, EPOLLIN);
    memset(buf, 0, MAXSIZE);
}
//wrapper of epoll_ctl
void add_event(int epollfd, int fd, int state){
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void modify_event(int epollfd, int fd, int state){
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

void delete_event(int epollfd, int fd, int state){
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}
