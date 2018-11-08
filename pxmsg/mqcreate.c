#include "unpipc.h"
// 报错:
// pen error for /test: Function not implemented
// 原因:Ubuntu16.04 把Posix消息队列功能禁用了,使用不了,需要重新编译内核(通过参数开启0)

struct mq_attr attr; /* mq_maxmsg and mq_msgsize both init to 0 */

int main(int argc, char **argv) {
    int c, flags;
    mqd_t mqd;

    flags = O_RDWR | O_CREAT;
    while((c = Getopt(argc, argv, "em:z:")) != -1) {
        switch(c) {
        case 'e':
            flags |= O_EXCL;
            break;

        case 'm':
            attr.mq_maxmsg = atol(optarg);
            break;

        case 'z':
            attr.mq_msgsize = atol(optarg);
            break;
        }
    }
    if(optind != argc - 1)
        err_quit("usage: mqcreate [ -e ] [ -m maxmsg -z msgsize ] <name>");

    // 要求要么同时设置消息队列消息长度最大值,队列长度最大值,要么都不设置
    if((attr.mq_maxmsg != 0 && attr.mq_msgsize == 0) ||
       (attr.mq_maxmsg == 0 && attr.mq_msgsize != 0))
        err_quit("must specify both -m maxmsg and -z msgsize");
    printf("optind: %d", optind);
	// 如果attr没有设置,第四个参数就为空,使用默认属性
    mqd = Mq_open(argv[optind], flags, FILE_MODE,
                  (attr.mq_maxmsg != 0) ? &attr : NULL);

    Mq_close(mqd);
    exit(0);
}
