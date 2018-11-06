#include "unpipc.h"

int main(int argc, char **argv) {
    if(argc != 2)
        err_quit("usage: pipeconf <pathname>");

    // 打印IPC的管道/FIFO的最大读写buf长度,一个进程任意时刻最大打开描述符
	// ubuntu pipe_buf:4096, open_max: 1024
    printf("PIPE_BUF = %ld, OPEN_MAX = %ld\n",
           Pathconf(argv[1], _PC_PIPE_BUF), Sysconf(_SC_OPEN_MAX));
    exit(0);
}
