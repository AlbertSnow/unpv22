/* include destroy */
#include "pthread_rwlock.h"
#include "unpipc.h"

int pthread_rwlock_destroy(pthread_rwlock_t *rw) {
    if(rw->rw_magic != RW_MAGIC)
        return (EINVAL);
    // 如果有正在读或写的线程或者正在等待读或写的线程
    if(rw->rw_refcount != 0 ||
       rw->rw_nwaitreaders != 0 || rw->rw_nwaitwriters != 0)
        return (EBUSY);

    pthread_mutex_destroy(&rw->rw_mutex);
    pthread_cond_destroy(&rw->rw_condreaders);
    pthread_cond_destroy(&rw->rw_condwriters);
    rw->rw_magic = 0;

    return (0);
}
/* end destroy */

void Pthread_rwlock_destroy(pthread_rwlock_t *rw) {
    int n;

    if((n = pthread_rwlock_destroy(rw)) == 0)
        return;
    errno = n;
    err_sys("pthread_rwlock_destroy error");
}
