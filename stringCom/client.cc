#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int MySend(int fd, char *buf, size_t len){
    int onesend = 0;
    unsigned int nsent = 0;
    if(len == 0){
        return 0;
    }
    while(nsent < len){
        do{
            onesend = send(fd, buf, len - nsent, 0);
        }while((onesend<0) && errno==EINTR);
        if(onesend < 0)
            return nsent;
        nsent += onesend;
        buf += onesend;
    }
    return len;
}

#define DEFAULT_PORT 6666
int main(){
    int connfd;
    int clen = 0;
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(DEFAULT_PORT);
    //servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    if((connfd = socket(AF_INET, SOCK_STREAM, 0))==-1){
        perror("socket failed\n");
        return -1;
    }
    if(connect(connfd, (struct sockaddr*)&servaddr, sizeof(servaddr))==-1){
        perror("connect failed\n");
        return -1;
    }

    ssize_t wlen;
    const char *sendmsg = "01234555";
    int tlen = strlen(sendmsg);
    printf("len of msg:%d", tlen);
    //构造发送字符串，前四个字节代表消息长度
    int ilen = 0;
    char *buf = new char[100];
    *(int *)(buf) = htonl(tlen);
    ilen += sizeof(int);
    //复制消息
    memcpy(buf + ilen, sendmsg, tlen);
    ilen += tlen;
    //发送字符串
    wlen = MySend(connfd, buf, ilen);
    if(wlen < 0){
        printf("write failed\n");
        close(connfd);
        return -1;
    }
    else{
        printf("write success, length:%d,msg is:%s",tlen, sendmsg);
    }
    close(connfd);
    return 0;

}
