/* include main */
#include "unpipc.h"

#define MAXNITEMS 1000000
#define MAXNTHREADS 100

int nitems; /* read-only by producer and consumer */
// 同一个结构里代表数据是共享的,需要同步
struct {
    pthread_mutex_t mutex;
    int buff[MAXNITEMS];
    int nput;
    int nval;
} shared = {PTHREAD_MUTEX_INITIALIZER}; // 用PTHREAD_MUTEX_INITIALIZER初始化mutex

void *produce(void *), *consume(void *);

int main(int argc, char **argv) {
    int i, nthreads, count[MAXNTHREADS];
    pthread_t tid_produce[MAXNTHREADS], tid_consume;

    if(argc != 3)
        err_quit("usage: prodcons2 <#items> <#threads>");
    nitems = min(atoi(argv[1]), MAXNITEMS);
    nthreads = min(atoi(argv[2]), MAXNTHREADS);

    Set_concurrency(nthreads);
    /* 4start all the producer threads */
    // 创建生产者
    for(i = 0; i < nthreads; i++) {
        count[i] = 0;
        // count[i]用来记录每个线程究竟生产了多少次
        Pthread_create(&tid_produce[i], NULL, produce, &count[i]);
    }

    /* 4wait for all the producer threads */
    // 等待生产者线程都结束任务
    for(i = 0; i < nthreads; i++) {
        Pthread_join(tid_produce[i], NULL);
        printf("count[%d] = %d\n", i, count[i]);
    }

    /* 4start, then wait for the consumer thread */
    Pthread_create(&tid_consume, NULL, consume, NULL);
    Pthread_join(tid_consume, NULL);

    exit(0);
}
/* end main */

/* include producer */
void *produce(void *arg) {
    for(;;) {
        Pthread_mutex_lock(&shared.mutex);
        if(shared.nput >= nitems) {
            Pthread_mutex_unlock(&shared.mutex);
            return (NULL); /* array is full, we're done */
        }
        shared.buff[shared.nput] = shared.nval;
        shared.nput++;
        shared.nval++;
        Pthread_mutex_unlock(&shared.mutex);
        *((int *)arg) += 1;
    }
}

void *consume(void *arg) {
    int i;
    printf("consume.......");

    // 这里不会输出,因为生产者 shared.nput 和 shared.nval 同时增加,所以他们一直相同
    for(i = 0; i < nitems; i++) {
        if(shared.buff[i] != i)
            printf("buff[%d] = %d\n", i, shared.buff[i]);
    }
    return (NULL);
}
/* end producer */
