/*************************************************************************
	> File Name: consumer.c
	> Author: 
	> Mail: 
	> Created Time: 2016年07月12日 星期二 10时49分59秒
 ************************************************************************/

#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<semaphore.h>

#define NUM 5
int queue[NUM];
sem_t blank_number, product_number;


void *consumer(void *arg)
{
    int c = 0;
    for(;;)
    {
        sem_wait(&product_number);
        printf("consum %d \n", queue[c]);
        queue[c] = 0;
        sem_post(&blank_number);
        c = (c+1) % NUM;
        sleep(rand() % 5);
    }

    return NULL;
}

void *producer(void *arg)
{
    int p = 0;
    for(;;)
    {
        sem_wait(&blank_number);
        queue[p] = rand() % 1000 + 1;
        printf("Produce %d \n", queue[p]);
        sem_post(&product_number);
        p = (p+1) % NUM;
        sleep(rand()%5);
    }
    return NULL;
}


int main()
{
    pthread_t pid, cid;
    srand(time(NULL));

    sem_init(&blank_number, 0, NUM);
    sem_init(&product_number, 0, 0);

    pthread_create(&pid, NULL, producer, NULL);
    pthread_create(&cid, NULL, consumer, NULL);

    pthread_join(pid, NULL);
    pthread_join(cid, NULL);

    return 0;
}


