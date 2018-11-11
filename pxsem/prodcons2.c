// comment:使用基于内存的信号量实现生产者-消费者模式
#include "unpipc.h"

#define NBUFF 10

int nitems; /* read-only by producer and consumer */
struct {    /* data shared by producer and consumer */
    int buff[NBUFF];
    // 注意,是对象,非指针(对象供线程间共享)
    sem_t mutex, nempty, nstored; /* semaphores, not pointers */
} shared;

void *produce(void *), *consume(void *);

int main(int argc, char **argv) {
    pthread_t tid_produce, tid_consume;

    if(argc != 2)
        err_quit("usage: prodcons2 <#items>");
    nitems = atoi(argv[1]);

    /* 4initialize three semaphores */
    // 传入对象指针,初始化对象,不同线程可直接使用sem_t对象来同步
    Sem_init(&shared.mutex, 0, 1);
    Sem_init(&shared.nempty, 0, NBUFF);
    Sem_init(&shared.nstored, 0, 0);

    Set_concurrency(2);
    Pthread_create(&tid_produce, NULL, produce, NULL);
    Pthread_create(&tid_consume, NULL, consume, NULL);

    Pthread_join(tid_produce, NULL);
    Pthread_join(tid_consume, NULL);

    Sem_destroy(&shared.mutex);
    Sem_destroy(&shared.nempty);
    Sem_destroy(&shared.nstored);
    exit(0);
}

void *produce(void *arg) {
    int i;

    for(i = 0; i < nitems; i++) {
        // 传入对象指针,不同线程共享内存空间
        Sem_wait(&shared.nempty); /* wait for at least 1 empty slot */
        Sem_wait(&shared.mutex);
        shared.buff[i % NBUFF] = i; /* store i into circular buffer */
        Sem_post(&shared.mutex);
        Sem_post(&shared.nstored); /* 1 more stored item */
    }
    return (NULL);
}

void *consume(void *arg) {
    int i;

    for(i = 0; i < nitems; i++) {
        // 传入对象指针,不同线程共享内存空间
        Sem_wait(&shared.nstored); /* wait for at least 1 stored item */
        Sem_wait(&shared.mutex);
        if(shared.buff[i % NBUFF] != i)
            printf("buff[%d] = %d\n", i, shared.buff[i % NBUFF]);
        Sem_post(&shared.mutex);
        Sem_post(&shared.nempty); /* 1 more empty slot */
    }
    return (NULL);
}
