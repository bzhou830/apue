/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 2016年07月13日 星期三 14时21分11秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>


#define SERV_PORT 8000
#define MAXLINE 1024

int main()
{

    int maxfd, listenfd, confd, clientaddrlen,sockfd, n, i = 0;
    struct sockaddr_in serveraddr, clientaddr;
    char str[INET_ADDRSTRLEN];                      //存放IP地址，点分十进制表示
    fd_set rset, allset;                            //select 使用
    char buf[MAXLINE];                              //传输数据
    int nReady, client[FD_SETSIZE],maxi;


    //1. 创建一个socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    //2. 绑定一个端口
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    //3. 设置监听
    listen(listenfd, 20);
    //printf("Accepting Connecting...\n");

    maxfd = listenfd;
    maxi = 0;

    for(i=0; i<FD_SETSIZE; ++i);                   //初始化clinet[]
        client[i] = -1;

    FD_ZERO(&allset);                               //select监控文件描述符集
    FD_SET(listenfd, &allset);

    for(;;)
    {
        rset = allset;
        nReady = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if(nReady < 0)                              //select出错
        {
            perror("select err\n");
            break;
        }

        if(FD_ISSET(listenfd, &rset))               //新链接的客户端
        {
            clientaddrlen = sizeof(clientaddr);
            confd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);

            printf("ip: %s, port: %d \n", 
                  inet_ntop(AF_INET, &clientaddr.sin_addr, str, sizeof(str)),
                  ntohs(clientaddr.sin_port));
            
            for(i=0; i<FD_SETSIZE; ++i)
            {
                if(client[i] < 0)
                {
                    client[i] = confd;
                    break;
                }
            }

            if(i == FD_SETSIZE)
            {
                fputs("limited\n", stderr);
                exit(1);
            }

            FD_SET(confd, &allset);
            
            if(confd > maxfd) maxfd = confd;
            if(i > maxi) maxi = i;
            if(--nReady == 0) continue;
        }

        for(i=0; i<=maxi; ++i)                          //遍历看哪个客户端有数据就绪
        {
            if((sockfd = client[i]) < 0) continue;
            if(FD_ISSET(sockfd, &rset))
            {
                if((n = read(sockfd, buf, MAXLINE)) == 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i]=0;
                }
                else
                {
                    int j;
                    for(j=0; j<n; ++j)
                        buf[j] = toupper(buf[j]);
                    write(sockfd, buf, n);
                }
                if(--nReady == 0)
                    break;
            }
        }
    }
    close(listenfd);
    return 0;
}




