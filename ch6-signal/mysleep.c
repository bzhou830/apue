/*************************************************************************
    > File Name: sigprocmask.c
    > Author: Robin
    > Mail: chou_robin@163.com 
    > Created Time: 2016年06月16日 星期四 09时58分58秒
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>


void mysleep(unsigned int sec)
{
	struct sigaction newact, oldact;
	unsigned int slept;
	newact.sa_siga
}


int main(int argc,char* argv[])
{
	sigset_t p, s;
	sigemptyset(&s);				//清空信号集
	sigaddset(&s, SIGINT);			//设置Ctrl + C 位 SIGINT
	sigaddset(&s, SIGTSTP);
	sigprocmask(SIG_BLOCK, &s, &p);	//修改进程信息集

	while(1)
	{
		sigpending(&p);				//获取未决信息集合
		printPend(&p);				//显示当前的未决信号集合
		sleep(1);
	}

	return 0;
}


