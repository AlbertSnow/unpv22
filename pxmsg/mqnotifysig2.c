#include "unpipc.h"

volatile sig_atomic_t mqflag; /* set nonzero by signal handler */
static void sig_usr1(int);

int main(int argc, char **argv) {
    mqd_t mqd;
    void *buff;
    ssize_t n;
    sigset_t zeromask, newmask, oldmask;
    struct mq_attr attr;
    struct sigevent sigev;

    if(argc != 2)
        err_quit("usage: mqnotifysig2 <name>");

    /* 4open queue, get attributes, allocate read buffer */
    mqd = Mq_open(argv[1], O_RDONLY);
    Mq_getattr(mqd, &attr);
    buff = Malloc(attr.mq_msgsize);

    Sigemptyset(&zeromask); /* no signals blocked */
    Sigemptyset(&newmask);
    Sigemptyset(&oldmask);
    Sigaddset(&newmask, SIGUSR1);

    /* 4establish signal handler, enable notification */
    Signal(SIGUSR1, sig_usr1);
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;
    Mq_notify(mqd, &sigev);

    for(;;) {
        // sigprocmask 会把第二个参数&newmask里的信号(newmask包括SIGUSR1信号)阻塞(第一个参数SIG_BLOCK决定)
        // 即如果SIGUSR1信号产生,也不会直接调用其处理函数,而是阻塞它,直到下面的sigsuspend()调用
        Sigprocmask(SIG_BLOCK, &newmask, &oldmask); /* block SIGUSR1 */
        while(mqflag == 0)
            // sigsuspend的整个原子操作过程为：
            // (1) 设置新的mask阻塞当前进程；
            // (2) 收到信号，恢复原先mask；
            // (3) 调用该进程设置的信号处理函数；
            // (4) 待信号处理函数返回后，sigsuspend返回。
            sigsuspend(&zeromask);
        mqflag = 0; /* reset flag */

        // 再一次注册信号,因为消息队列的注册只有一次性
        Mq_notify(mqd, &sigev);
        // 这里存在一个问题,如果阻塞期间有两个以上消息到达队列,而这里只调用了一次
        // 造成队列里还有数据(不为空),这样SIGUSR1就不会再次触发了
        n = Mq_receive(mqd, buff, attr.mq_msgsize, NULL);
        printf("read %ld bytes\n", (long)n);
        Sigprocmask(SIG_UNBLOCK, &newmask, NULL); /* unblock SIGUSR1 */
    }
    exit(0);
}

static void
sig_usr1(int signo) {
    mqflag = 1;
    return;
}
