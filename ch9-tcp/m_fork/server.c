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
    pid_t pid;

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
        
        pid = fork();
        if(pid == 0)
        {
            close(fd);
            while(1)
            {
                revlen = read(confd, buf, MAXLINE);
                if(revlen == 0)
                {
                    printf("closed\n");
                    break;
                }
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
            }
            close(confd);
            return 0;
        }
        else if(pid > 0)
            close(confd);
        else
            printf("fork err\n");
    }
    close(fd);
    return 0;
}

