#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define NLOOP 500

int counter;

void *doit(void *arg)
{

    int i = 0, val;
    for(i = 0; i < NLOOP; ++i)
    {
        val = counter;
        printf("%x: %d\n", (unsigned int)pthread_self(), val + 1);
        counter = val + 1;
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

