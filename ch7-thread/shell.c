#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

typedef struct __Info
{
	char buf[1024];	
typedef struct __Info
{
	char buf[1024];			//�û�������Ϣ
	unsigned int pipeNum;	//ͳ�ƹܵ��ĸ���
}Info;

//��¼��������Ĭ��localhost
static char hostname[1024] = "myshell";

//������ʾ
int sys_err(const char* err)
{
	perror(err);
	exit(EXIT_FAILURE);
}


//��Ϣ��ʾ
void display()
{
	//static unsigned int sum = 1;	//��¼�������
	char buf[1024] = { 0 };
	//��ȡ��ǰ·��
	getcwd(buf, sizeof(buf));	
	printf("[%s@%s]>", hostname, buf);	
	//sum++;
}


/*
 * �û���������ݴ���
 * @reminder 	info ����Ľṹ��
 * 	return:	
 * 		�ɹ���EXIT_SUCCESS
 * 		ʧ�ܣ�EXIT_FAILURE
*/
int handle(Info* info)
{
	if(NULL == info)
	{
		printf("func %s err: [NULL == info]\n", __FUNCTION__);
		exit(EXIT_FAILURE);
	}

	char* p = info->buf;

	//ͳ�ƹܵ��ĸ���
	while(NULL != (p = strchr(p, '|')))
	{
		info->pipeNum++;
		p++;
	}
	p = info->buf;

	//ȥ���س�
	if(NULL != (p = strchr(p, '\n')))
	{
		*p = '\0';
	}
	return EXIT_SUCCESS;
}

//�ַ��滻
int replate(char* str, const char src, const char des)
{
	if(NULL == str)
	{
		printf("func %s error: [NULL == str]\n", __FUNCTION__);
		exit(EXIT_FAILURE);
	}
	char* p =str;
	while(*p)
	{
		if(src == *p)
		{
			*p = des;
		}
		p++;
	}
	return EXIT_SUCCESS;
}

//�������
int resolveRun(char* ptr, char** argv)
{
	if(NULL == ptr || NULL == argv)
	{
		printf("func %s error:[NULL == ptr || NULL == argv]\n", __FUNCTION__);
		exit(EXIT_FAILURE);
	}
	int i = 0;
	int fd;
	char* inptr = NULL;
	char* p = strtok_r(ptr, " ", &inptr);
	argv[i++] = p;
	while(NULL != (p = strtok_r(NULL, " ", &inptr)))
	{
		argv[i++] = p;
	}
	
	//�ж��Ƿ����ض���
	i--;
	while(i)
	{
		if(0 == strcmp(argv[i], ">"))
		{
			fd = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if(0 > fd)
			{
				sys_err("func resolveRun() open error: ");
			}
			//���������1023������
			dup2(STDOUT_FILENO, 1023);
			dup2(fd, STDOUT_FILENO);
			argv[i] = NULL;
			close(fd);
			//�ҵ���һ��>
			int n = 1;
			while(n < i)
			{
				if(0 == strcmp(argv[n], ">"))
				{
					argv[n] = NULL;
					break;
				}
				n++;
			}
			break;
		}

		if(0 == strcmp(argv[i], ">>"))
		{
			fd = open(argv[i+1], O_APPEND|O_CREAT|O_WRONLY, 0644);
			if(0 > fd)
			{
				sys_err("func resolveRun() open error: ");
			}
			dup2(STDOUT_FILENO, 1023);
			dup2(fd, STDOUT_FILENO);
			argv[i] = NULL;
			close(fd);
			
			//�ҵ���һ��>
			int n = 1;
			while(n < i){
				if(0 == strcmp(argv[n], ">>")){
					argv[n] = NULL;
					break;
				}
				n++;
			}
			break;
		}
		
		if(0 == strcmp(argv[i], "<")){
			fd = open(argv[i+1], O_RDONLY);
			if(0 > fd){
				sys_err("func resolveRun() open error: ");
			}
			
			char buf[1024] = { 0 };
			int len = 0;
			while(0 != (len = read(fd, buf, sizeof(buf)))){
				if(0 > len){
					sys_err("func resolveRun() read error: ");
				}
				write(STDIN_FILENO, buf, len);
				bzero(buf, sizeof(buf));
			}
			//��ĩβ���һ��������-1
			putc(-1, STDIN_FILENO);
			argv[i] = NULL;
			close(fd);

			//�ҵ���һ��>
			int n = 1;
			while(n < i){
				if(0 == strcmp(argv[n], "<")){
					argv[n] = NULL;
					break;
				}
				n++;
			}	
			break;
		}
		i--;
	}
	
	return EXIT_SUCCESS;
}

//�������
int seve(Info* info)
{
	if(NULL == info){
		printf("func %s err: [NULL == info]\n", __FUNCTION__);
		exit(EXIT_FAILURE);
	}
	//�ж��Ƿ�Ϊ������
	if(0 == *(info->buf)){
		return EXIT_SUCCESS;
	}
	
	pid_t 		pid;
	pid_t		wpid;
	char 		buf[1024] 	= { 0 };
	char* 		p 		= buf;
	char* 		inptr 		= NULL;
	int 		i 		= 0;
	int 		fd[2];
	char* 		argv[256] 	= {NULL};

	//����ԭ������
	memcpy(buf, info->buf, sizeof(buf));
	
	//����tab��
	replate(buf, '\t', ' ');
	//����'��
	replate(buf, '\'', ' ');
	//����"��
	replate(buf, '\"', ' ');

	for(i = 0; i <= info->pipeNum; i++)
	{
		if(0 == i)
		{
			p = strtok_r(p, "|", &inptr);
		}
		else
		{
			p = strtok_r(NULL, "|", &inptr);
		}		
		//��ʼ��
		bzero(argv, sizeof(argv));
		//�������
		resolveRun(p, argv);
		
		//�ж��Ƿ�����������
		if(0 == i && 0 == strcmp("cd", argv[0]))
		{
			if(0 > chdir(argv[1]))
			{
				//�жϴ�������,��ʾ�û���Ϣ
				if(ENOENT == errno){
					printf("-sea_bash: cd: %s: û���Ǹ��ļ���Ŀ¼\n", argv[1]);	
				}
				if(EACCES == errno){
					printf("-sea_bash: cd: %s: Ȩ�޲���\n", argv[1]);
				}
				if(ENOTDIR == errno){
					printf("-sea_bash: cd: %s: ����Ŀ¼\n", argv[1]);
				}
			}
			return EXIT_SUCCESS;
		}
		else if(0 == i && 0 == strcmp("pwd", argv[0])){
			char buf[1024] = { 0 };		
			getcwd(buf, sizeof(buf));				//��ȡ��ǰ����Ŀ¼
			buf[strlen(buf)] = '\n';		 		//ĩβ��ӻ���
			write(STDOUT_FILENO, buf, strlen(buf));	//����Ļ��ӡ��ǰ·��
			return EXIT_SUCCESS;					//�ɹ�����
		}
		else if(0 == i && 0 == strcmp("hostname", argv[0])){
			//���		
			bzero(hostname, sizeof(hostname));			//��ԭ����hostname���
			memcpy(hostname, argv[1], strlen(argv[1]));	//��������hostname
			return EXIT_SUCCESS;
		}
		else if(0 == i && 0 == strcmp("exit", argv[0])){
			//��������
			printf("--------------------goddbye!-------------------------\n");
			kill(getpid(), SIGINT);		//�򱾽��̷��ͽ����ź�
			exit(EXIT_SUCCESS);			//ֱ�ӽ����˳�
		}
	
		//�����ܵ�
		if(0 > pipe(fd))
		{
			sys_err("func seve() pipe error: ");
		}
		pid = fork();
		if(0 > pid)
		{
			sys_err("func seve() fork error:");
		}
		else if(0 == pid)
		{
			close(fd[0]);		//�ӽ��̹رն���
			dup2(1022, fd[0]);	//����һ���ܵ��Ķ����ض���fd[0]
			close(1022);		//�ر�1022��һ���źŵĶ���,���������˴���
			break;				//����
		}
		
		//��ԭ���������
		dup2(1023, STDOUT_FILENO);	
		//������˸���һ������ʹ��
		dup2(fd[0], 1022);
		close(fd[1]);
		close(fd[0]);
	}

	//�ӽ��̴���
	if(i != info->pipeNum+1)
	{
		//û�йܵ�����
		if(0 != info->pipeNum)
		{
			if(i == info->pipeNum)
			{
				close(fd[1]);
				dup2(fd[0], STDIN_FILENO);
			}
			if(0 == i)
			{
				dup2(fd[1], STDOUT_FILENO);
			}
			else
			{
				dup2(fd[0], STDIN_FILENO);
				dup2(fd[1], STDOUT_FILENO);
			}
		}
		execvp(argv[0], argv);
		printf("-sea_bash: %s: command not found\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	//�����̵ȴ��ӽ��̽���
	if(i == info->pipeNum+1)
	{
		do
		{
			wpid = waitpid(-1, NULL, WNOHANG);
			if(0 < wpid)
				i--;
		}
		while(0 < i);
	}
	return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
	Info info = {{0}, 0};
	for(;;)
	{
		display();
		//��ȡ�û����롡
		fgets(info.buf, sizeof(info.buf), stdin);
		//��Ϣ����
		handle(&info);
		//�������
		seve(&info);
		//��ճ�ʼ��
		bzero(&info, sizeof(info));
	}
	return EXIT_SUCCESS;
}




