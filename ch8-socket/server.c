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

#define SERV_PORT 8000
#define MAXLINE 1024

int main()
{
    int fd, confd, clientaddrlen, revlen, i;
    struct sockaddr_in serveraddr, clientaddr;
    char str[INET_ADDRSTRLEN];
    char buf[MAXLINE];

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
       
        //5. 读取数据，处理数据
        
        revlen = read(confd, buf, MAXLINE);
        printf("client: %s\t port: %d\n",
              inet_ntop(AF_INET, &clientaddr.sin_addr, str, sizeof(str)),
              ntohs(clientaddr.sin_port));

        i = 0;
        while(i < revlen)
        {
            buf[i] = toupper(buf[i]);
            ++i;
        }
        write(confd, buf, revlen);
        //6. 关闭连接
        close(confd);
    }
    close(fd);
    return 0;
}




