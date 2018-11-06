#include "unpipc.h"

#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"

void client(int, int), server(int, int);

int main(int argc, char **argv) {
    int readfd, writefd;
    pid_t childpid;

    /* 4create two FIFOs; OK if they already exist */
    // 创建fifo,如果创建失败则判断是否原来存在,存在就继续运行
    if((mkfifo(FIFO1, FILE_MODE) < 0) && (errno != EEXIST))
        err_sys("can't create %s", FIFO1);
    if((mkfifo(FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) {
        // 删除创建的fifo(不删除则在文件系统可见)
        /** unlink(FIFO1); */
        err_sys("can't create %s", FIFO2);
    }

    if((childpid = Fork()) == 0) { /* child */
                                   // 父进程先读后写打开,子进程先写后读打开FIFO,这样的顺序不会造成死锁
                                   // 因为对于同一个FIFO,读方式打开若没有写则阻塞,写方式打开若没有读则阻塞
                                   // 详情见p45图4-21的表格
        readfd = Open(FIFO1, O_RDONLY, 0);
        writefd = Open(FIFO2, O_WRONLY, 0);

        server(readfd, writefd);
        exit(0);
    }
    /* 4parent */
    writefd = Open(FIFO1, O_WRONLY, 0);
    readfd = Open(FIFO2, O_RDONLY, 0);

    client(readfd, writefd);

    Waitpid(childpid, NULL, 0); /* wait for child to terminate */

    //手动关闭描述符. ps: 描述符和FIFO不是同一回事,一个FIFO可以打开多个描述符
    Close(readfd);
    Close(writefd);

    // 删除创建的fifo(不删除则在文件系统可见),注释后在/tmp/fifo.1 /tmp/fifo.2 可找到
    Unlink(FIFO1);
    Unlink(FIFO2);
    exit(0);
}
