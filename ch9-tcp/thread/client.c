/*************************************************************************
	> File Name: client.c
	> Author: 
	> Mail: 
	> Created Time: 2016年07月13日 星期三 15时05分22秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLINE 80
#define SERV_PORT 8000

int main(int argc, char *argv[])
{
    //sockaddr_in IPv4
    struct sockaddr_in servaddr;
    char buf[MAXLINE];
    int sockfd, n;
    
    //1. 创建一个套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    //2. 链接到本机8000端口服务器
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(SERV_PORT);
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
    while(fgets(buf, MAXLINE, stdin) != NULL)
    {
        //3. 向服务器发送数据
        write(sockfd, buf, strlen(buf));
        //4. 读取服务器的回传数据
        n = read(sockfd, buf, MAXLINE);
        printf("Response from server:\n");
        write(STDOUT_FILENO, buf, n);
    }

    //5. 关闭套接字描述符
    close(sockfd);
    return 0;
}



