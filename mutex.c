/*************************************************************************
	> File Name: nosy.c
	> Author: 
	> Mail: 
	> Created Time: 2016年07月12日 星期二 10时03分47秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define NLOOP 500

int counter;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;


void *doit(void *arg)
{

    int i = 0, val;
    for(i = 0; i < NLOOP; ++i)
    {
        pthread_mutex_lock(&counter_mutex);
        val = counter;
        printf("%x: %d\n", (unsigned int)pthread_self(), val + 1);
        counter = val + 1;
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}


int main()
{
    pthread_t tidA, tidB;
    
    pthread_create(&tidA, NULL, doit, NULL);
    pthread_create(&tidB, NULL, doit, NULL);

    pthread_join(tidA, NULL);
    pthread_join(tidB, NULL);

    return 0;
}




