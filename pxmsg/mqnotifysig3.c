// 使用了非阻塞mq_receive的信号通知
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
        err_quit("usage: mqnotifysig3 <name>");

    /* 4open queue, get attributes, allocate read buffer */
    // 这里增加了标志符O_NONBLOCK,即不阻塞,每次读(mq_reveive)如果队列为空
    // 不阻塞,直接返回0,设置errno
    mqd = Mq_open(argv[1], O_RDONLY | O_NONBLOCK);
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
        Sigprocmask(SIG_BLOCK, &newmask, &oldmask);
        // 消息低下:不断轮询,直到mqflag被设置为0退出循环
        // 解决方案: 后面的例子使用sigwait()
        while(mqflag == 0)
            sigsuspend(&zeromask);
        mqflag = 0; /* reset flag */

        Mq_notify(mqd, &sigev);
        // 前面的逻辑保证队列不为空,但消息数量可能有1+个
        // 多次读取,直到队列为空,mq_receive返回0,并设置errno
        while((n = mq_receive(mqd, buff, attr.mq_msgsize, NULL)) >= 0) {
            printf("read %ld bytes\n", (long)n);
        }
        // 读取到为空错误是EAGAIN
        if(errno != EAGAIN)
            err_sys("mq_receive error");
        Sigprocmask(SIG_UNBLOCK, &newmask, NULL); /* unblock SIGUSR1 */
    }
    exit(0);
}

static void
sig_usr1(int signo) {
    mqflag = 1;
    return;
}
