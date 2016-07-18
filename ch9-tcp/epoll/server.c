#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/epoll.h>

#define SERV_PORT 8000
#define MAXLINE 1024
#define OPEN_MAX 1024

int main()
{
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;    
    int i = 0, maxi, nready, clientlen, sockfd, n;
    char buf[MAXLINE];
    


    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));    
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(listenfd, 20);
    
    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for(i = 1; i < OPEN_MAX; ++i)
        client[i].fd  = -1;
    maxi = 0;

    for(;;)
    {
        nready = poll(client, maxi + 1, -1);
        if(client[0].revents & POLLRDNORM)              //新的客户端链接请求
        {
            clientlen = sizeof(client_addr);
            connfd = accept(listenfd, (struct sockaddr*)&client_addr, &clientlen);
            for(i=1; i<OPEN_MAX; ++i)
            {
                if(client[i].fd < 0)
                {
                    client[i].fd = connfd;
                    break;
                }
            }
            if(i == OPEN_MAX)
                printf("too many clients! \n ");
            client[i].events = POLLRDNORM;
            if(i > maxi)
                maxi = i;
            if(--nready <= 0)
                continue;
        }

        for(i=0; i<=maxi; ++i)
        {
            if((sockfd = client[i].fd) < 0)
                continue;
            if(client[i].revents & (POLLRDNORM | POLLERR))
            {
                if((n=read(client[i].fd, buf, sizeof(buf))) <= 0)
                {
                    close(client[i].fd);
                    client[i].fd = -1;
                }
                else
                    write(client[i].fd, buf, n);

                if(--nready <= 0)
                    break;
            }
        }
    }
    return 0;
}




