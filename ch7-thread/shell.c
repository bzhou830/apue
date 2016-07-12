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
	char buf[1024];			//用户输入信息
	unsigned int pipeNum;	//统计管道的个数
}Info;

//记录主机名　默认localhost
static char hostname[1024] = "myshell";

//错误提示
int sys_err(const char* err)
{
	perror(err);
	exit(EXIT_FAILURE);
}


//消息提示
void display()
{
	//static unsigned int sum = 1;	//记录输入次数
	char buf[1024] = { 0 };
	//获取当前路径
	getcwd(buf, sizeof(buf));	
	printf("[%s@%s]>", hostname, buf);	
	//sum++;
}


/*
 * 用户输入的内容处理
 * @reminder 	info 传入的结构体
 * 	return:	
 * 		成功：EXIT_SUCCESS
 * 		失败：EXIT_FAILURE
*/
int handle(Info* info)
{
	if(NULL == info)
	{
		printf("func %s err: [NULL == info]\n", __FUNCTION__);
		exit(EXIT_FAILURE);
	}

	char* p = info->buf;

	//统计管道的个数
	while(NULL != (p = strchr(p, '|')))
	{
		info->pipeNum++;
		p++;
	}
	p = info->buf;

	//去除回车
	if(NULL != (p = strchr(p, '\n')))
	{
		*p = '\0';
	}
	return EXIT_SUCCESS;
}

//字符替换
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

//命令解析
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
	
	//判断是否有重定向
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
			//备份输出至1023描述符
			dup2(STDOUT_FILENO, 1023);
			dup2(fd, STDOUT_FILENO);
			argv[i] = NULL;
			close(fd);
			//找到第一个>
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
			
			//找到第一个>
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
			//向末尾输出一个结束符-1
			putc(-1, STDIN_FILENO);
			argv[i] = NULL;
			close(fd);

			//找到第一个>
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

//拆分命令
int seve(Info* info)
{
	if(NULL == info){
		printf("func %s err: [NULL == info]\n", __FUNCTION__);
		exit(EXIT_FAILURE);
	}
	//判断是否为空数据
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

	//复制原有数据
	memcpy(buf, info->buf, sizeof(buf));
	
	//处理tab键
	replate(buf, '\t', ' ');
	//处理'号
	replate(buf, '\'', ' ');
	//处理"号
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
		//初始化
		bzero(argv, sizeof(argv));
		//命令解析
		resolveRun(p, argv);
		
		//判断是否是内置命令
		if(0 == i && 0 == strcmp("cd", argv[0]))
		{
			if(0 > chdir(argv[1]))
			{
				//判断错误类型,提示用户信息
				if(ENOENT == errno){
					printf("-sea_bash: cd: %s: 没有那个文件或目录\n", argv[1]);	
				}
				if(EACCES == errno){
					printf("-sea_bash: cd: %s: 权限不够\n", argv[1]);
				}
				if(ENOTDIR == errno){
					printf("-sea_bash: cd: %s: 不是目录\n", argv[1]);
				}
			}
			return EXIT_SUCCESS;
		}
		else if(0 == i && 0 == strcmp("pwd", argv[0])){
			char buf[1024] = { 0 };		
			getcwd(buf, sizeof(buf));				//获取当前工作目录
			buf[strlen(buf)] = '\n';		 		//末尾添加换行
			write(STDOUT_FILENO, buf, strlen(buf));	//向屏幕打印当前路径
			return EXIT_SUCCESS;					//成功结束
		}
		else if(0 == i && 0 == strcmp("hostname", argv[0])){
			//清空		
			bzero(hostname, sizeof(hostname));			//将原来的hostname清空
			memcpy(hostname, argv[1], strlen(argv[1]));	//重新设置hostname
			return EXIT_SUCCESS;
		}
		else if(0 == i && 0 == strcmp("exit", argv[0])){
			//结束进程
			printf("--------------------goddbye!-------------------------\n");
			kill(getpid(), SIGINT);		//向本进程发送结束信号
			exit(EXIT_SUCCESS);			//直接进程退出
		}
	
		//创建管道
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
			close(fd[0]);		//子进程关闭读端
			dup2(1022, fd[0]);	//将上一个管道的读端重定向到fd[0]
			close(1022);		//关闭1022上一个信号的读端,避免多个读端存在
			break;				//跳出
		}
		
		//还原输出描述符
		dup2(1023, STDOUT_FILENO);	
		//保存读端给下一个进程使用
		dup2(fd[0], 1022);
		close(fd[1]);
		close(fd[0]);
	}

	//子进程处理
	if(i != info->pipeNum+1)
	{
		//没有管道命令
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
	
	//父进程等待子进程结束
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
		//获取用户输入　
		fgets(info.buf, sizeof(info.buf), stdin);
		//信息处理
		handle(&info);
		//拆分命令
		seve(&info);
		//清空初始化
		bzero(&info, sizeof(info));
	}
	return EXIT_SUCCESS;
}




