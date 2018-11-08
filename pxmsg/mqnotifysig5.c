// 使用select的Posix消息队列
// 由于mq_open返回的不是"普通"的描述符,不能用于select/poll中.
// 故可以采用管道来间接实现select
#include "unpipc.h"

int pipefd[2];
static void sig_usr1(int);
int main(int argc, char **argv) {
    int nfds;
    char c;
    fd_set rset;
    mqd_t mqd;
    void *buff;
    ssize_t n;
    struct mq_attr attr;
    struct sigevent sigev;

    if(argc != 2)
        err_quit("usage: mqnotifysig5 <name>");

    /* 4open queue, get attributes, allocate read buffer */
    mqd = Mq_open(argv[1], O_RDONLY | O_NONBLOCK);
    Mq_getattr(mqd, &attr);
    buff = Malloc(attr.mq_msgsize);

    Pipe(pipefd);

    /* 4establish signal handler, enable notification */
    Signal(SIGUSR1, sig_usr1);
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;
    Mq_notify(mqd, &sigev);

    FD_ZERO(&rset);
    for(;;) {
        FD_SET(pipefd[0], &rset);
        nfds = Select(pipefd[0] + 1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(pipefd[0], &rset)) {
            Read(pipefd[0], &c, 1);
            Mq_notify(mqd, &sigev); /* reregister first */
            while((n = mq_receive(mqd, buff, attr.mq_msgsize, NULL)) >= 0) {
                printf("read %ld bytes\n", (long)n);
            }
            if(errno != EAGAIN)
                err_sys("mq_receive error");
        }
    }
    exit(0);
}


static void sig_usr1(int signo) {
	// 往管道写数据
    Write(pipefd[1], "", 1); /* one byte of 0 */
    return;
}
