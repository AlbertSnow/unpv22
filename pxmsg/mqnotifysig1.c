#include "unpipc.h"

mqd_t mqd;
void *buff;
struct mq_attr attr;
struct sigevent sigev;

static void sig_usr1(int);

int main(int argc, char **argv) {
    if(argc != 2)
        err_quit("usage: mqnotifysig1 <name>");

    /* 4open queue, get attributes, allocate read buffer */
    // 通过参数名来打开一个消息队列(这里是通过mqcreate先创建的)
    mqd = Mq_open(argv[1], O_RDONLY);
    Mq_getattr(mqd, &attr);
    buff = Malloc(attr.mq_msgsize);

    /* 4establish signal handler, enable notification */
    // 先注册SIGUSR1的信号处理函数
    Signal(SIGUSR1, sig_usr1);

    // 把信号值注册到消息队列中:
    // 即当消息队列由空变为非空时,会产生一个信号(该信号本质还是数字),
    // 信号产生后,系统会自动帮你调用对应的信号处理函数(如果没有手动注册,会使用默认处理函数?)
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;
    Mq_notify(mqd, &sigev);

	// 无限循环,等待队列由空变非空,产生信号,调用信号处理函数
    for(;;)
        pause(); /* signal handler does everything */
    exit(0);
}

// 信号处理函数第一个参数是信号值
static void sig_usr1(int signo) {
    ssize_t n;

	// 以下三个函数都不是异步信号安全函数.存在危险,见P71
    Mq_notify(mqd, &sigev); /* reregister first */
    n = Mq_receive(mqd, buff, attr.mq_msgsize, NULL);
    printf("SIGUSR1 received, read %ld bytes\n", (long)n);
    return;
}
