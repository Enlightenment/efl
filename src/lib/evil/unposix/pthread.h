#ifndef PTHREAD_H
#define PTHREAD_H

struct pthread_key_t { int x; };

typedef struct pthread_key_t pthread_key_t;

typedef struct { int x; } sem_t;

struct pthread_mutex_t { int x; };

typedef struct pthread_mutex_t pthread_mutex_t;

struct pthread_cond_t { int x; };

typedef struct pthread_cond_t pthread_cond_t;

struct pthread_rwlock_t { int x; };

typedef struct pthread_rwlock_t pthread_rwlock_t;

/* struct pthread_t { int x; }; */

/* typedef struct pthread_t pthread_t; */

typedef unsigned long long pthread_t;

#endif
