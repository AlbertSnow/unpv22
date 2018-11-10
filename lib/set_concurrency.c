#include "unpipc.h"

int set_concurrency(int level) {
#ifdef HAVE_THR_SETCONCURRENCY_PROTO
	int thr_setconcurrency(int);

	return (thr_setconcurrency(level));
#else
	// Ubuntu16.04
	return (pthread_setconcurrency(level));
    /** return (0); */
#endif
}

void Set_concurrency(int level) {
	/** printf(level); */
    if(set_concurrency(level) != 0)
        err_sys("set_concurrency error");
}
