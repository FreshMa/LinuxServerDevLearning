#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>

int MyRecv(int fd, char* buf, size_t ncount){
    size_t nread = 0;
    int oneread;
    //如果没接收完，一直接收
    while(nread < ncount){
        do{
            oneread = read(fd, buf, ncount - nread);
        }while((oneread < 0)&&(errno == EINTR));
        if(oneread < 0){
            return oneread;
        }
        else if(oneread == 0){
            return nread;
        }
        nread += oneread;
        buf += oneread;
    }
}

#define DEFAULT_PORT 6666

int main(){
    int sockfd, listenfd;
    struct sockaddr_in seraddr;
    struct sockaddr_in cliaddr;

    unsigned int sin_size, listenNum = 10;
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket error\n");
        return -1;
    }
    printf("socket ok:%d\n",listenfd);

    bzero(&seraddr, sizeof(seraddr));
    //htons和thonl将数字转为网络字节序，字符串不需要转换
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(DEFAULT_PORT);
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listenfd, (struct sockaddr*)&seraddr, sizeof(seraddr)) == -1){
        perror("bind failed\n");
        return -1;
    }

    if(listen(listenfd, listenNum) == -1){
        perror("listen failed\n");
        return -1;
    }
    char msg[10];
    sin_size = sizeof(cliaddr);
    
    sockfd = accept(listenfd, (struct sockaddr*)&cliaddr, &sin_size);
    if(sockfd < 0){
        close(listenfd);
        printf("accept faild\n");
        return -1;
    }
    //先接收4B大小的数据，并将其转换为主机字节序的数字
    ssize_t readLen = MyRecv(sockfd, msg, sizeof(int));
    if(readLen < 0){
        perror("read failed\n");
        return -1;
    }
    int len = (int)ntohl(*(int*)msg); 
    printf("msg len:%d\n",len);
    
    //接收指定长度的字符串
    readLen = MyRecv(sockfd, msg, len);
    if(readLen < 0){
        perror("read failed\n");
        return -1;
    }
    msg[readLen] = '\0';
    printf("msg is:%s\n",msg);
    //close(listenfd);
    close(sockfd);
    return 0;
}
