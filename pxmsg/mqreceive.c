#include "unpipc.h"
typedef unsigned int uint_t;

int main(int argc, char **argv) {
    int c, flags;
    mqd_t mqd;
    ssize_t n;
    uint_t prio;
    void *buff;
    struct mq_attr attr;

    flags = O_RDONLY;
    while((c = Getopt(argc, argv, "n")) != -1) {
        switch(c) {
        case 'n':
            // 如果命令行加了'-n'参数,则表明不阻塞
            // 问题:哪里不阻塞?
            // 下面的mq_receive如果队列为空,会阻塞到不为空,不阻塞则返回错误
            flags |= O_NONBLOCK;
            break;
        }
    }
    if(optind != argc - 1)
        err_quit("usage: mqreceive [ -n ] <name>");

    mqd = Mq_open(argv[optind], flags);
    Mq_getattr(mqd, &attr);

    buff = Malloc(attr.mq_msgsize);

    n = Mq_receive(mqd, buff, attr.mq_msgsize, &prio);
    printf("read %ld bytes, priority = %u\n", (long)n, prio);

    exit(0);
}
