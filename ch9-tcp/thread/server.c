/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 2016年07月13日 星期三 14时21分11秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERV_PORT 8000
#define MAXLINE 1024

struct s_info{
    struct sockaddr_in addr;
    int connfd;
};


void *do_work(void *arg)
{
    int n, i;
    struct s_info *ts = (struct s_info*)arg;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    
    pthread_detach(pthread_self());

    while(1)
    {
        n = read(ts->connfd, buf, MAXLINE);
        if (n == 0)
        {
            printf("peer closed. \n");
            break;
        }

        printf("client ip: %s, port: %d \n",
              inet_ntop(AF_INET, &(*ts).addr.sin_addr, str, sizeof(str)),
              ntohs(ts->addr.sin_port));

        for(i=0; i<n; ++i)
            buf[i] = toupper(buf[i]);

        write(ts->connfd, buf, n);
    }
    
    close(ts->connfd);
    return NULL;
}


int main()
{
    int fd, confd, clientaddrlen, revlen, i = 0;
    struct sockaddr_in serveraddr, clientaddr;
    char str[INET_ADDRSTRLEN];
    struct s_info ts[383];
    pthread_t tid;
    //1. 创建一个socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    //2. 绑定一个端口
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    //3. 设置监听
    listen(fd, 20);
    printf("Accepting Connecting...\n");

    while(1)
    {
        //4.接受链接
        clientaddrlen = sizeof(clientaddr);
        confd = accept(fd, (struct sockaddr*)&clientaddr, &clientaddrlen);
        ts[i].addr = clientaddr;
        ts[i].connfd = confd;
    
        pthread_create(&tid, NULL, do_work, (void *)&ts[i]);

        i++;
        if(i >= 382)
            i = 0;
    }
    close(fd);
    return 0;
}




