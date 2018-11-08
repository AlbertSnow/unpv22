/* include mq_close */
#include "mqueue.h"
#include "unpipc.h"

int mymq_close(mymqd_t mqd) {
    long msgsize, filesize;
    struct mymq_hdr *mqhdr;
    struct mymq_attr *attr;
    struct mymq_info *mqinfo;

    mqinfo = mqd;
	// 检查是否已经关闭了
    if(mqinfo->mqi_magic != MQI_MAGIC) {
        errno = EBADF;
        return (-1);
    }
    mqhdr = mqinfo->mqi_hdr;
    attr = &mqhdr->mqh_attr;
	
	// 如果当前进程有注册事件则清空事件
    if(mymq_notify(mqd, NULL) != 0) /* unregister calling process */
        return (-1);

    msgsize = MSGSIZE(attr->mq_msgsize);
    filesize = sizeof(struct mymq_hdr) + (attr->mq_maxmsg *
                                          (sizeof(struct mymsg_hdr) + msgsize));
    if(munmap(mqinfo->mqi_hdr, filesize) == -1)
        return (-1);

    mqinfo->mqi_magic = 0; /* just in case */
	// 释放指针
    free(mqinfo);
    return (0);
}
/* end mq_close */

void Mymq_close(mymqd_t mqd) {
    if(mymq_close(mqd) == -1)
        err_sys("mymq_close error");
}
