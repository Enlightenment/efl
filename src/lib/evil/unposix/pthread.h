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

#endif
