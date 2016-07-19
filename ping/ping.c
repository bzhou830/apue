#include <stdio.h>					//perror()
#include <stdlib.h>					//
#include <netdb.h>					//getprotobynumber  gethostbyname
#include <unistd.h>					//setuid() getuid() sleep() alarm()
#include <sys/types.h>				//getuid()
#include <string.h>					//bzero(s, n) ��s��ǰn���ֽ���Ϊ0
#include <signal.h>					//signal()
#include <time.h>					//gettimeofday()
#include <netinet/ip_icmp.h>		//
#include <errno.h>					//EINTR
#include <arpa/inet.h>				//inet_ntoa()

#define MAX_NO_PACKETS	10			//
#define PACKET_SIZE		4096		//
#define MAX_WAIT_TIME	5			//

int sockfd;
int datalen = 56;
struct sockaddr_in dest_addr;
struct sockaddr_in from;
pid_t pid;							//��ȡ���̺�

int nsend = 0, nreceived = 0;

char sendpacket[PACKET_SIZE];
char recvpacket[PACKET_SIZE];

struct timeval tvrecv;

void statistics();					//��ʾ�������ݰ����ܽ���Ϣ
void send_packet(int count);		//����ICMP����
void recv_packet(int count);		//��������ICMP����

int main(int argc, char** argv)
{
	struct protoent* protocol;		//
	int size = 1024*50;
	unsigned int inaddr = 0l;		//0L
	struct hostent *host;

	if (argc < 3)
	{
		printf("usage:%s hostname/IP address or usage:%s hostname/IP address -r\n", argv[0], argv[0]);
		exit(1);
	}

	//getprotobyname()�᷵��һ��protoent�ṹ������protoΪ����ѯ������Э������
	//�˺������ /etc/protocols�в��ҷ������������ݲ��ɽṹprotoent����
	if ((protocol=getprotobyname("icmp")) == NULL)
	{
		perror("getprotobyname");
		exit(1);
	}

	//����ʹ��ICMP��ԭʼ�׽���,�����׽���ֻ��root�û���������
	if ((sockfd=socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0)
	{
		perror("socket error");
		exit(1);
	}

	//setuid(getuid()); //����rootȨ�ޣ����õ�ǰ�û�Ȩ��
	//�����׽��ֽ��ջ�������50K��������ҪΪ�˼�С���ջ���������ĵĿ�����,��������pingһ���㲥��ַ��ಥ��ַ,������������Ӧ��
	//SO_RCVBUF int Ϊ����ȷ����������С
	//ѡ���Ĳ��,Ŀǰ��֧��SOL_SOCKET��IPPROTO_TCP��Ρ�
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

	bzero(&dest_addr, sizeof(dest_addr));
	//����sockaddr_inΪ��Ա
	//sa_family�ǵ�ַ���壬һ�㶼�ǡ�AF_xxx������ʽ��ͨ������õ��Ƕ���AF_INET,����TCP/IPЭ���塣
	//sin_port�洢�˿ںţ�ʹ�������ֽ�˳��
	//sin_addr�洢IP��ַ��ʹ��in_addr������ݽṹ
	dest_addr.sin_family = AF_INET;	

	inaddr = inet_addr(argv[2]);//��һ�����ʮ���Ƶ�IPת����һ������������

	//�ж�������������ip��ַ
	if (inaddr == INADDR_NONE)
	{
		if ((host=gethostbyname(argv[2])) == NULL)
		{
			perror("gethostbyname error");
			exit(1);
		}
		//��������ַ
		memcpy((char*)&dest_addr.sin_addr, host->h_addr, sizeof(dest_addr.sin_addr));//û��h_addr�����Ա������h_addr��ʾh_addr_list�ĵ�һ����ַ,��Ϊ#define h_addr h_addr_list[0]
	}
	else//��ip��ַ
	{
		memcpy((char *)&dest_addr.sin_addr, (char *)&inaddr, sizeof(inaddr)); 
	}

	//��ȡmain�Ľ���id����������icmp�ı�־��
	pid = getpid();

	printf("PING %s(%s): %d bytes data in ICMP packets.\n", argv[2], inet_ntoa(dest_addr.sin_addr), datalen);

	//����ĳһ�źŵĶ�Ӧ����
	//SIGINT:��Interrupt Key������ͨ����CTRL+C����DELETE�����͸�����ForeGround Group�Ľ���
	signal(SIGINT, statistics);

	if (argc == 3)
	{
		send_packet(5);
		recv_packet(5);
		statistics();
	}
	else
	{
		printf("input error, pls check it...\n");
	}
	return 0;
}


//FUNCTION:У����㷨
unsigned short cal_chksum(unsigned short *addr, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;

	//��ICMP��ͷ������������2�ֽ�Ϊ��λ�ۼ�����
	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	//��ICMP��ͷΪ�������ֽڣ���ʣ�����һ�ֽڡ������һ���ֽ���Ϊһ��2�ֽ����ݵĸ��ֽڣ�
	//���2�ֽ����ݵĵ��ֽ�Ϊ0�������ۼ�
	if (nleft == 1)
	{
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}

	sum = (sum>>16) + (sum&0xffff);
	sum += (sum>>16);
	answer = ~sum;

	return answer;
}


//FUNCTION:����timeval�ṹ���
void tv_sub(struct timeval *out,struct timeval *in)
{
	if ((out->tv_usec-=in->tv_usec) < 0)
	{
		--out->tv_sec;
		out->tv_usec += 1000000;
	}

	out->tv_sec -= in->tv_sec;
}


//FUNCTION:����ICMP��ͷ
int pack(int pack_no)
{
	int i, packsize;
	struct icmp *icmp;
	struct timeval *tval;

	icmp = (struct icmp *)sendpacket;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_seq = pack_no;
	icmp->icmp_id = pid;

	packsize = datalen + 8;
	tval = (struct timeval *)icmp->icmp_data;
	gettimeofday(tval, NULL);	//��¼����ʱ��

	icmp->icmp_cksum = cal_chksum((unsigned short *)icmp, packsize);

	return packsize;
}


//FUNCTION:��ȥICMP��ͷ����ʾ
int unpack(char *buf, int len)
{
	int i, iphdrlen;
	struct ip *ip;
	struct icmp *icmp;
	struct timeval *tvsend;
	double rtt;

	ip = (struct ip *)buf;
	iphdrlen = ip->ip_hl<<2;//��ip��ͷ����,��ip��ͷ�ĳ��ȱ�־��4
	icmp = (struct icmp *)(buf+iphdrlen);//Խ��ip��ͷ,ָ��ICMP��ͷ
	
	len -= iphdrlen;	//ICMP��ͷ��ICMP���ݱ����ܳ���
	if (len < 8)		//С��ICMP��ͷ�����򲻺���
	{
		printf("ICMP packets\'s length is less than 8\n");
		return -1;
	}

	//ȷ�������յ����������ĵ�ICMP�Ļ�Ӧ
	if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid))
	{
		tvsend = (struct timeval *)icmp->icmp_data;
		tv_sub(&tvrecv, tvsend);//���պͷ��͵�ʱ���

		rtt = tvrecv.tv_sec*1000 + tvrecv.tv_usec/1000;//�Ժ��뵥λ����rtt
		
		printf("%d byte from %s: icmp_seq=%u ttl=%d rtt=%.3f ms\n", len, inet_ntoa(from.sin_addr), icmp->icmp_seq, ip->ip_ttl, rtt);
	}
	else
		return -1;
}


//FUNCTION:
void statistics()
{
	printf("\n------------------------ping statistics------------------------\n");
	printf("%d packets transmitted, %d received, %.2f%% packets lost\n", nsend, nreceived, (float)(nsend-nreceived)/(float)nsend*100);
	printf("---------------------------------------------------------------\n");
	close(sockfd);
	exit(1);
}


//FUNCTION:
void send_packet(int count)
{
	int packetsize;

	while (nsend < MAX_NO_PACKETS)//����MAX_NO_PACKETS������
	{
		nsend++;
		packetsize = pack(nsend);

		//sendpacketΪҪ���͵����ݣ���pack()�����趨��dest_addr��Ŀ�ĵ�ַ
		if (sendto(sockfd, sendpacket, packetsize, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
		{
			perror("sendto error");
			continue;
		}
		//sleep(1);//ÿ��һ�뷢��һ��ICMP����
	}
}


//FUNCTION:��������ICMP����
void recv_packet(int count)
{
	int n, fromlen;
	extern int errno;

	//��alarm�������õ�timer��ʱ��setitimer�������õ�interval timer��ʱ
	signal(SIGALRM, statistics);

	fromlen = sizeof(from);
	while (nreceived < nsend)
	{
		//alarm()���������ź�SIGALRM�ھ�������secondsָ�����������͸�Ŀǰ�Ľ���
		alarm(MAX_WAIT_TIME);

		//(��socket��������
		if ((n=recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&from, &fromlen)) < 0)
		{
			if (errno == EINTR)
				continue;
			perror("recvfrom error");
			continue;
		}

		gettimeofday(&tvrecv, NULL);//��¼����ʱ��

		if (unpack(recvpacket, n) == -1)
			continue;

		nreceived++;
	}
}
