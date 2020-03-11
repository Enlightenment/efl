#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "unimplemented.h"

UNIMPLEMENTED_STRUCT_T(sem)

UNIMPLEMENTED inline int sem_close(sem_t* sem)
{
    #warning sem_close is not implemented
    return 0;
}
UNIMPLEMENTED inline int sem_destroy(sem_t* sem)
{
    #warning sem_destroy is not implemented
    return 0;
}
UNIMPLEMENTED inline int sem_getvalue(sem_t* restrict sem, int* restrict x)
{
    #warning sem_getvalue is not implemented
    return 0;
}
UNIMPLEMENTED inline int sem_init(sem_t* sem, int x, unsigned y)
{
    #warning sem_init is not implemented
    return 0;
}
UNIMPLEMENTED inline sem_t* sem_open(const char* name, int x, ...)
{
    #warning sem_open is not implemented
    return 0;
}
UNIMPLEMENTED inline int sem_post(sem_t* sem)
{
    #warning sem_post is not implemented
    return 0;
}
UNIMPLEMENTED inline int sem_timedwait(sem_t* restrict sem, const struct timespec* restrict timeout)
{
    #warning sem_timedwait is not implemented
    return 0;
}
UNIMPLEMENTED inline int sem_trywait(sem_t* sem)
{
    #warning sem_trywait is not implemented
    return 0;
}
UNIMPLEMENTED inline int sem_unlink(const char* name)
{
    #warning sem_unlink is not implemented
    return 0;
}
UNIMPLEMENTED inline int sem_wait(sem_t* sem)
{
    #warning sem_wait is not implemented
    return 0;
}

#endif
