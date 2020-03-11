#ifndef PTHREAD_H
#define PTHREAD_H

#include "unimplemented.h"

UNIMPLEMENTED typedef unsigned long long pthread_t;

UNIMPLEMENTED_STRUCT_T(pthread_mutexattr)
UNIMPLEMENTED_STRUCT_T(pthread_condattr)
UNIMPLEMENTED_STRUCT_T(pthread_attr)
UNIMPLEMENTED_STRUCT_T(pthread_rwlock)
UNIMPLEMENTED_STRUCT_T(pthread_cond)
UNIMPLEMENTED_STRUCT_T(pthread_mutex)
UNIMPLEMENTED_STRUCT_T(sem)

UNIMPLEMENTED struct sched_param {
    int sched_priority;
};

UNIMPLEMENTED typedef int pthread_key_t;

#define SCHED_RR 5
#define SCHED_FIFO 6

#define PTHREAD_BARRIER_SERIAL_THREAD 1
#define PTHREAD_CANCEL_ASYNCHRONOUS 2
#define PTHREAD_CANCEL_ENABLE 3
#define PTHREAD_CANCEL_DEFERRED 4
#define PTHREAD_CANCEL_DISABLE 5
#define PTHREAD_CANCELED 6
#define PTHREAD_CREATE_DETACHED 7
#define PTHREAD_CREATE_JOINABLE 8
#define PTHREAD_EXPLICIT_SCHED 9
#define PTHREAD_INHERIT_SCHED 10
#define PTHREAD_MUTEX_DEFAULT 11
#define PTHREAD_MUTEX_ERRORCHECK 12
#define PTHREAD_MUTEX_NORMAL 13
#define PTHREAD_MUTEX_RECURSIVE 14
#define PTHREAD_MUTEX_ROBUST 15
#define PTHREAD_MUTEX_STALLED 16
#define PTHREAD_ONCE_INIT 17
#define PTHREAD_PRIO_INHERIT 18
#define PTHREAD_PRIO_NONE 19
#define PTHREAD_PRIO_PROTECT 20
#define PTHREAD_PROCESS_SHARED 21
#define PTHREAD_PROCESS_PRIVATE 22
#define PTHREAD_SCOPE_PROCESS 23
#define PTHREAD_SCOPE_SYSTEM 24

UNIMPLEMENTED inline int pthread_mutex_trylock(void* m)
{
    #warning pthread_mutex_trylock is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_mutex_lock(void* m)
{
    #warning pthread_mutex_lock is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_mutex_unlock(void* m)
{
    #warning pthread_mutex_unlock is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_cond_wait(void* c, void* m)
{
    #warning pthread_cond_wait is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_cond_timedwait(void* c, void* m, int t)
{
    #warning pthread_cond_timedwait is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_cond_broadcast(void* c)
{
    #warning pthread_cond_broadcast is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_cond_signal(void* c)
{
    #warning pthread_cond_signal is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_rwlock_rdlock(void* c)
{
    #warning pthread_rwlock_rdlock is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_rwlock_wrlock(void* c)
{
    #warning pthread_rwlock_wrlock is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_rwlock_unlock(void* c)
{
    #warning pthread_rwlock_unlock is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_key_create(void* c, void * d)
{
    #warning pthread_key_create is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_key_delete(void* c)
{
    #warning pthread_key_delete is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_getspecific(void* c)
{
    #warning pthread_getspecific is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_setspecific(void* c, const void* d)
{
    #warning pthread_setspecific is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_mutexattr_init(void* c, ...)
{
    #warning pthread_mutexattr_init is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_getschedparam(void* a, ...)
{
    #warning pthread_getschedparam is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_self()
{
    #warning pthread_self is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_setschedparam(void* c, void* d, void* e)
{
    #warning pthread_ is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_rwlock_init(void* a, ...)
{
    #warning pthread_rwlock_init is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_mutexattr_settype(void* a, ...)
{
    #warning pthread_mutexattr_settype is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_mutex_init(void* a, ...)
{
    #warning pthread_mutex_init is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_mutex_destroy(void* a, ...)
{
    #warning pthread_mutex_destroy is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_condattr_init(void* a, ...)
{
    #warning pthread_condattr_init is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_cond_init(void* a, ...)
{
    #warning pthread_cond_init is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_condattr_destroy(void* a, ...)
{
    #warning pthread_condattr_destroy is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_cond_destroy(void* a, ...)
{
    #warning pthread_cond_destroy is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_rwlock_destroy(void* a, ...)
{
    #warning pthread_rwlock_destroy is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_cancel(void* a, ...)
{
    #warning pthread_cancel is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_setcancelstate(void* a, ...)
{
    #warning pthread_setcancelstate is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_testcancel()
{
    #warning pthread_testcancel is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_cleanup_pop(void* a, ...)
{
    #warning pthread_cleanup_pop is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_cleanup_push(void* a, ...)
{
    #warning pthread_cleanup_push is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_attr_init(void* a, ...)
{
    #warning pthread_attr_init is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_join(void* a, ...)
{
    #warning pthread_join is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_create(void* a, ...)
{
    #warning pthread_create is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_equal(void* a, ...)
{
    #warning pthread_equal is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_setcanceltype(void* a, ...)
{
    #warning pthread_setcanceltype is not implemented.
    return 0;
}

UNIMPLEMENTED inline int pthread_mutexattr_destroy(void* a, ...)
{
    #warning pthread_mutexattr_destroy is not implemented.
    return 0;
}

#endif
