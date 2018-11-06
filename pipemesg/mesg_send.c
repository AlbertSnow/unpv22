/* include mesg_send */
#include "mesg.h"
// 把整个消息内容发过去: 消息头长度+消息内容长度
ssize_t mesg_send(int fd, struct mymesg *mptr) {
    return (write(fd, mptr, MESGHDRSIZE + mptr->mesg_len));
}
/* end mesg_send */

void Mesg_send(int fd, struct mymesg *mptr) {
    ssize_t n;

    if((n = mesg_send(fd, mptr)) != mptr->mesg_len)
        err_quit("mesg_send error");
}
