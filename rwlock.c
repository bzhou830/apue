/*************************************************************************
	> File Name: nosy.c
	> Author: 
	> Mail: 
	> Created Time: 2016年07月12日 星期二 10时03分47秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define NLOOP 50

int counter = 0;
pthread_rwlock_t rwlock;

void *th_write(void *arg)
{
    int t = 0;
    while(++t < NLOOP)
    {
    //    printf("%x: %d\n", (unsigned int)pthread_self(), t);
        pthread_rwlock_wrlock(&rwlock);
        t = counter;
        usleep(100);
        counter += 1;
        printf("write %x : counter %d, ++counter %d \n", (unsigned int)pthread_self(), t, counter);
       
        pthread_rwlock_unlock(&rwlock);
        usleep(200);
    }
}

void *th_read(void *arg)
{
    int t = 0;
    while(++t < NLOOP)
    {
        pthread_rwlock_rdlock(&rwlock);
        printf("read %x, counter: %d\n", (unsigned int)pthread_self(), counter);
        pthread_rwlock_unlock(&rwlock);
        usleep(200);
    }
}

int main()
{
    int i = 0;
    pthread_t tid[8];
    pthread_rwlock_init(&rwlock, NULL);

    for(i = 0; i < 3; ++i)
    {
        pthread_create(&tid[i], NULL, th_write, NULL);
    }
    
    for(i = 3; i < 8; i++)
    {
        pthread_create(&tid[i], NULL, th_read, NULL);
    }


    for(i=0; i<8; ++i)
    {
        pthread_join(tid[i], NULL);
    }

    pthread_rwlock_destroy(&rwlock);
    
    return 0;
}


