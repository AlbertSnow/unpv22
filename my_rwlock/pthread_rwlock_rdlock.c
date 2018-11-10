/* include rdlock */
#include "pthread_rwlock.h"
#include "unpipc.h"

int pthread_rwlock_rdlock(pthread_rwlock_t *rw) {
    int result;

    if(rw->rw_magic != RW_MAGIC)
        return (EINVAL);

    if((result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
        return (result);

    /* 4give preference to waiting writers */
	// 读者拿锁的等待条件: 写者拿锁 || 有写者在等待
    while(rw->rw_refcount < 0 || rw->rw_nwaitwriters > 0) {
        rw->rw_nwaitreaders++;
		// pthread_cond_wait成功返回0,失败为非0
        result = pthread_cond_wait(&rw->rw_condreaders, &rw->rw_mutex);
        rw->rw_nwaitreaders--;
        if(result != 0)
            break;
    }
    if(result == 0)
        rw->rw_refcount++; /* another reader has a read lock */

    pthread_mutex_unlock(&rw->rw_mutex);
    return (result);
}
/* end rdlock */

void Pthread_rwlock_rdlock(pthread_rwlock_t *rw) {
    int n;

    if((n = pthread_rwlock_rdlock(rw)) == 0)
        return;
    errno = n;
    err_sys("pthread_rwlock_rdlock error");
}
