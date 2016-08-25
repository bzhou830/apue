#include<stdio.h>
#include<pthread.h>
#include<unistd.h>

void *th_func(void *arg)
{
	printf("this is a child thread \n");
	printf("func tid: %x \n", (unsigned int)pthread_self());
	return NULL;
}

int main()
{
	pthread_t tid;                                  
	pthread_create(&tid, NULL, th_func, NULL);      
	printf("main tid: %x \n", (unsigned int)tid);   
	pthread_join(tid, NULL);                        //等待线程终止
	return 0;                                       
}

