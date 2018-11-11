// comment: 使用基于内存信号实现'多生产者-单消费者'
/* include main */
#include "unpipc.h"

#define NBUFF 10
#define MAXNTHREADS 100

int nitems, nproducers; /* read-only by producer and consumer */

struct { /* data shared by producers and consumer */
    int buff[NBUFF];
    int nput;
    int nputval;
    sem_t mutex, nempty, nstored; /* semaphores, not pointers */
} shared;

void *produce(void *), *consume(void *);

int main(int argc, char **argv) {
    int i, count[MAXNTHREADS];
    pthread_t tid_produce[MAXNTHREADS], tid_consume;

    if(argc != 3)
        err_quit("usage: prodcons3 <#items> <#producers>");
    nitems = atoi(argv[1]);
    nproducers = min(atoi(argv[2]), MAXNTHREADS);

    /* 4initialize three semaphores */
    Sem_init(&shared.mutex, 0, 1);
    Sem_init(&shared.nempty, 0, NBUFF);
    Sem_init(&shared.nstored, 0, 0);

    /* 4create all producers and one consumer */
    // 创建生产者线程
    Set_concurrency(nproducers + 1);
    for(i = 0; i < nproducers; i++) {
        count[i] = 0;
        Pthread_create(&tid_produce[i], NULL, produce, &count[i]);
    }
    // 创建消费者线程
    Pthread_create(&tid_consume, NULL, consume, NULL);

    /* 4wait for all producers and the consumer */
    // 等待所有线程结束
    for(i = 0; i < nproducers; i++) {
        Pthread_join(tid_produce[i], NULL);
        printf("count[%d] = %d\n", i, count[i]);
    }
    Pthread_join(tid_consume, NULL);

    // 需要手动销毁基于内存的信号量
    Sem_destroy(&shared.mutex);
    Sem_destroy(&shared.nempty);
    Sem_destroy(&shared.nstored);
    exit(0);
}
/* end main */

/* include produce */
void *produce(void *arg) {
    for(;;) {
        // 等待空槽数量不为0
        Sem_wait(&shared.nempty); /* wait for at least 1 empty slot */
                                  // 加锁
        Sem_wait(&shared.mutex);

        // 如果已经全部生产完, 空槽数量+1(上面sem_wait-1), 解锁
        // 加锁后还要判断是否nput大于等于items的原因:
        // Sem_wait(&share.nempty)可能不阻塞了,但是切换到另外一个线程,也刚好
        // 不阻塞,于是另外一个线程执行生产操作,并刚好最后一个. 此时切换到原来
        // 线程这里,执行生产操作,但是已经生产完了,所以得判断.
        // 这里主要还是sem_wait(&share.nempty)和sem_wait(&shared.mutex)不是原子
        // 操作的问题
        if(shared.nput >= nitems) {
            Sem_post(&shared.nempty);
            Sem_post(&shared.mutex);
            return (NULL); /* all done */
        }

        shared.buff[shared.nput % NBUFF] = shared.nputval;
        shared.nput++;
        shared.nputval++;

        Sem_post(&shared.mutex);
        Sem_post(&shared.nstored); /* 1 more stored item */
                                   // 当前线程生产数量(数组里的一个元素)+1
        *((int *)arg) += 1;
    }
}
/* end produce */

/* include consume */
void *consume(void *arg) {
    int i;

    for(i = 0; i < nitems; i++) {
        Sem_wait(&shared.nstored); /* wait for at least 1 stored item */
        Sem_wait(&shared.mutex);

		// 正常这里是不会有任何输出,因为在生产者中,nputval和nput是同步增加的(原子操作)
        if(shared.buff[i % NBUFF] != i)
            printf("error: buff[%d] = %d\n", i, shared.buff[i % NBUFF]);

        Sem_post(&shared.mutex);
        Sem_post(&shared.nempty); /* 1 more empty slot */
    }
    return (NULL);
}
/* end consume */
