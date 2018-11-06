#include "fifo.h"

void server(int, int);

int main(int argc, char **argv) {
    int readfifo, writefifo, dummyfd, fd;
    char *ptr, buff[MAXLINE], fifoname[MAXLINE];
    pid_t pid;
    ssize_t n;

    /* 4create server's well-known FIFO; OK if already exists */
    if((mkfifo(SERV_FIFO, FILE_MODE) < 0) && (errno != EEXIST))
        err_sys("can't create %s", SERV_FIFO);

    /* 4open server's well-known FIFO for reading and writing */
    // 阻塞! 直到有第一个客户端连接
    readfifo = Open(SERV_FIFO, O_RDONLY, 0);
    // 读打开FIFO,但是不使用,这样可以防止第一个客户端断开连接时,
    // readfifo因为FIFO没有任何读会返回0
    dummyfd = Open(SERV_FIFO, O_WRONLY, 0); /* never used */

    // 如果没有上面的读打开,这里的Readline会在第一次客户端断开连接时
    // 直接返回0, 造成退出. 现在有闲置的读打开,FIFO不为空,Readline即使
    // 第一个连接断开,但是会阻塞到有数据到来(后续的连接并传输数据),
    // 实现迭代而不会退出程序
    while((n = Readline(readfifo, buff, MAXLINE)) > 0) {
        if(buff[n - 1] == '\n')
            n--;        /* delete newline from readline() */
        buff[n] = '\0'; /* null terminate pathname */

        if((ptr = strchr(buff, ' ')) == NULL) {
            err_msg("bogus request: %s", buff);
            continue;
        }

        *ptr++ = 0; /* null terminate PID, ptr = pathname */
        pid = atol(buff);
        snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long)pid);
        if((writefifo = open(fifoname, O_WRONLY, 0)) < 0) {
            err_msg("cannot open: %s", fifoname);
            continue;
        }

        if((fd = open(ptr, O_RDONLY)) < 0) {
            /* 4error: must tell client */
            snprintf(buff + n, sizeof(buff) - n, ": can't open, %s\n",
                     strerror(errno));
            n = strlen(ptr);
            Write(writefifo, ptr, n);
            Close(writefifo);

        } else {
            /* 4open succeeded: copy file to FIFO */
			// 本例子的一个问题: 写入客户端的FIFO会阻塞,直到客户端开始读其FIFO
			// FIFO写时若无读则阻塞,读时若无写则阻塞
			// 如果,有个客户端发送请求,但不读,这里则一直阻塞,其它客户端即使连接
			// 也不会有回应.
            while((n = Read(fd, buff, MAXLINE)) > 0)
                Write(writefifo, buff, n);
            Close(fd);
            Close(writefifo);
        }
    }
}
