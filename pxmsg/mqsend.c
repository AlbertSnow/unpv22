#include "unpipc.h"

int main(int argc, char **argv) {
    mqd_t mqd;
    void *ptr;
    size_t len;
    uint_t prio;

	// 用户指定消息队列名,写入字节大小(写入内容处理为空),优先级
    if(argc != 4)
        err_quit("usage: mqsend <name> <#bytes> <priority>");
    len = atoi(argv[2]);
    prio = atoi(argv[3]);

    mqd = Mq_open(argv[1], O_WRONLY);

    // 用calloc给ptr分配指定长度的字节(用0填充),以便写入到消息队列
    ptr = Calloc(len, sizeof(char));
    Mq_send(mqd, ptr, len, prio);

    exit(0);
}
