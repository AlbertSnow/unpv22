/* include wrlock */
#include "pthread_rwlock.h"
#include "unpipc.h"

int pthread_rwlock_wrlock(pthread_rwlock_t *rw) {
    int result;

    if(rw->rw_magic != RW_MAGIC)
        return (EINVAL);

    if((result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
        return (result);

    // 写者拿锁的等待条件: 已有有写者拿锁(refcount=-1)或有读者拿锁了(refcount>0)
    while(rw->rw_refcount != 0) {
        rw->rw_nwaitwriters++;
        // 这里在等待,但如果其线程被取消了,&rw->rw_mutex仍然被取消线程锁定
		//  导致其它想获取锁(比如pthread_rwlock_unlock())阻塞
        result = pthread_cond_wait(&rw->rw_condwriters, &rw->rw_mutex);
        rw->rw_nwaitwriters--;
        if(result != 0)
            break;
    }
    if(result == 0)
        rw->rw_refcount = -1;

    pthread_mutex_unlock(&rw->rw_mutex);
    return (result);
}
/* end wrlock */

void Pthread_rwlock_wrlock(pthread_rwlock_t *rw) {
    int n;

    if((n = pthread_rwlock_wrlock(rw)) == 0)
        return;
    errno = n;
    err_sys("pthread_rwlock_wrlock error");
}
