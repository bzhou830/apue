/*************************************************************************
    > File Name: access.c
    > Author: Robin
    > Mail: chou_robin@163.com 
    > Created Time: 2016年06月01日 星期三 15时52分50秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>


int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr,"args num: %d error\n", argc);
		exit(1);
	}

	int rt;

	int mode = R_OK;
	switch(atoi(argv[2]))
	{
		case 0: mode = R_OK; break;
		case 1: mode = W_OK; break;
		case 2: mode = X_OK; break;
		case 3: mode = F_OK; break;
	}

	if((rt = access(argv[1], mode)) == -1)
	{
		fprintf(stderr,"Error: \n", perror);
		exit(1);
	}
    
	
	printf("%d\n", rt);
	return 0;
}

