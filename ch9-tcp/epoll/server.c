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
    int listenfd, connfd, client[OPEN_MAX], efd;
    struct sockaddr_in server_addr, client_addr;
    struct epoll_event tep, ep[OPEN_MAX];
    int i = 0, maxi, nready, clientlen, sockfd, n, j=0, res;
    char buf[MAXLINE];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));    
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(listenfd, 20);
    
    maxi = -1;
    for(i=1; i<OPEN_MAX; ++i)
        client[i] = -1;

    efd = epoll_create(OPEN_MAX);
    if(efd == -1)
    {
        printf("epoll_create error!\n");
        exit(-1);
    }
    tep.events = EPOLLIN;
    tep.data.fd = listenfd;
    res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
    if(res == -1)
    {
        printf("epoll_ccl error!\n");
        exit(-1);
    }

    while(1)
    {
        nready = epoll_wait(efd, ep, OPEN_MAX, -1);
        if(nready == -1)
        {
            printf("epoll_wait error!\n");
            exit(-1);
        }
        for(i=0; i<nready; ++i)
        {
            if(!(ep[i].events & EPOLLIN))
                continue;
            
            if(ep[i].data.fd == listenfd)
            {
                clientlen = sizeof(client_addr);
                connfd = accept(listenfd, (struct sockaddr*)&client_addr, &clientlen);
                
                for(j=0;j<OPEN_MAX;++j)
                {
                    if(client[j] < 0)
                    {
                        client[j] = connfd;
                        break;
                    }
                    if(j == OPEN_MAX)
                        printf("too many clients\n");
                    if(j > maxi)
                        maxi = j;
                    tep.events = EPOLLIN;
                    tep.data.fd = connfd;
                    res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
                    if(res == -1)
                    {
                        printf("epoll_ctl error!\n");
                        exit(-1);
                    }
                }
            }
            else
            {
                sockfd = ep[i].data.fd;
                n = read(sockfd, buf, MAXLINE);
                if(0 == n)
                {
                    for(j=0; j <= maxi; ++j)
                    {
                        if(client[j] == sockfd)
                        {
                            client[i] = -1;
                            break;
                        }
                    }
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                    if(res == -1)
                    {
                        printf("epoll_ctl error\n");
                        exit(-1);
                    }
                    close(sockfd);
                }
                else
                    write(sockfd, buf, n);
            }
        }
    }
    close(listenfd);
    close(efd);
    return 0;
}



