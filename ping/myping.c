#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>					//getprotobynumber  gethostbyname
#include <unistd.h>					//setuid() getuid() sleep() alarm()
#include <sys/types.h>				//getuid()
#include <signal.h>					//signal()
#include <time.h>					//gettimeofday()
#include <netinet/ip_icmp.h>		//
#include <arpa/inet.h>				//inet_ntoa()
#include <errno.h>					//EINTR

#define PACKET_SIZE		4096
#define MAX_WAIT_TIME	20

struct sockaddr_in dest_addr;
struct sockaddr_in from;
struct timeval tvrecv;

pid_t pid;
int sockfd = 0;
int datalen = 56;
int nsend = 0, nreceived = 0;
char sendpacket[PACKET_SIZE];
char recvpacket[PACKET_SIZE];

void sys_error(char *msg)
{
    printf("error: %s\n", msg);
    exit(-1);
}

//16位检验和
//bsd ping中使用的相同方法
unsigned short cal_chksum(unsigned short* addr, int len)
{
    int nleft = len;            //剩下字节数
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while(nleft > 1){           //按照两个字节为一个单位，叠加
        sum += *w++;
        nleft -=2;
    }

    if(nleft == 1){             //若ICMP首部为奇数个字节，则最后一个字节当作高位字节相加
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum>>16) + (sum&0xffff); //sum的高位和低位相加
    sum += (sum>>16);
    answer = ~sum;
    return answer;
}

void tv_sub(struct timeval *out, struct timeval *in)
{
    if((out->tv_usec -= in->tv_usec) < 0){
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

int pack(int pack_no)
{
    int packsize;
    struct icmp *s_icmp;
    struct timeval *tval;
    s_icmp = (struct icmp*)sendpacket;
    s_icmp->icmp_type = ICMP_ECHO;
    s_icmp->icmp_code = 0;
    s_icmp->icmp_cksum = 0;
    s_icmp->icmp_seq = pack_no;
    s_icmp->icmp_id = pid;
    packsize = datalen + 8;
    tval = (struct timeval *)s_icmp->icmp_data;
    gettimeofday(tval, NULL);
    s_icmp->icmp_cksum = cal_chksum((unsigned short*)s_icmp, packsize);
    return packsize;
}

int unpack(char *buf, int len)
{
    int iphdrlen;
    struct ip *ip;
    struct icmp *s_icmp;
    struct timeval *tvsend;
    double rtt;
    ip = (struct ip*)buf;
    iphdrlen = ip->ip_hl<<2;
    s_icmp = (struct icmp*)(buf + iphdrlen);
    len -= iphdrlen;
    if (len < 8)
        sys_error("ICMP packets length is less than 8");

    if ((s_icmp->icmp_type == ICMP_ECHOREPLY) && (s_icmp->icmp_id == pid))
    {
        tvsend = (struct timeval *)s_icmp->icmp_data;
        tv_sub(&tvrecv, tvsend);
        rtt = tvrecv.tv_sec*1000 + tvrecv.tv_usec/1000;
        printf("%d byte from %s: icmp_seq=%u ttl=%d rtt=%.3f ms\n",
               len, inet_ntoa(from.sin_addr), s_icmp->icmp_seq, ip->ip_ttl, rtt);
        return 0;
    }
    return -1;
}

void statistics()
{
    printf("\n------------------------ping statistics------------------------\n");
    printf("%d packets transmitted, %d received, %.2f%% packets lost\n",
           nsend, nreceived, (float)(nsend-nreceived)/(float)nsend*100);
    printf("---------------------------------------------------------------\n");
    close(sockfd);
    exit(1);
}

void send_packet(int count)
{
    int packetsize;
    while (nsend < count)
    {
        nsend++;
        packetsize = pack(nsend);
        if (sendto(sockfd, sendpacket, packetsize, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
        {
            perror("sendto error");
            continue;
        }
    }
}

void recv_packet(int count)
{
    int n, fromlen;
    extern int errno;
    signal(SIGALRM, statistics);
    fromlen = sizeof(from);
    while (nreceived < nsend)
    {
        alarm(MAX_WAIT_TIME);

        if ((n = recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&from, &fromlen)) < 0)
        {
            if (errno == EINTR)
                continue;
            perror("recvfrom error");
            continue;
        }
        gettimeofday(&tvrecv, NULL);
        if (unpack(recvpacket, n) == -1)
            continue;
        nreceived++;
    }
}

//主函数
int main(int argc, char *argv[])
{
    int size = 1024 * 50;
    unsigned int inaddr = 0L;
    
    struct protoent* protocol;
    struct hostent* host;

    //setuid(getuid());

    if(argc < 2)//调用命令行参数检查
    {
        printf("usage:%s hostname/IP address or usage: %s hostname/IP address -r\n",argv[0], argv[0]);
        exit(0);
    }

    if((protocol = getprotobyname("icmp")) == NULL)
        sys_error("getprotobyname");

    if((sockfd=socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0 )
        sys_error("socket error");
    
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    inaddr = inet_addr(argv[1]);

    if(inaddr == INADDR_NONE)
    {
        if((host=(struct hostent*)getprotobyname(argv[1])) == NULL)
            sys_error("gethostbyname");
        memcpy((char*)&dest_addr.sin_addr, host->h_addr, sizeof(dest_addr.sin_addr));
    }
    else
        memcpy((char*)&dest_addr.sin_addr, (char *)&inaddr, sizeof(inaddr));



    pid = getpid();
    
    printf("ping %s(%s): %d bytes data in ICMP packets. \n",
           argv[1], inet_ntoa(dest_addr.sin_addr), datalen);
    
    signal(SIGINT, statistics);

    send_packet(5);
    recv_packet(5);
    statistics();

    return 0;
}

