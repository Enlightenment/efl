//////////////////////////////////////////////////////////////////////////
#if defined(HAVE_SYS_EPOLL_H)
# define HAVE_EPOLL    1
# include <sys/epoll.h>
#else

# define HAVE_EPOLL    0
# define EPOLLIN       1
# define EPOLLPRI      2
# define EPOLLOUT      4
# define EPOLLERR      8

# define EPOLL_CTL_ADD 1
# define EPOLL_CTL_DEL 2
# define EPOLL_CTL_MOD 3

typedef union epoll_data
{
   void    *ptr;
   int      fd;
   uint32_t u32;
   uint64_t u64;
} epoll_data_t;

struct epoll_event
{
   uint32_t     events;
   epoll_data_t data;
};

static inline int
epoll_create(int size EINA_UNUSED)
{
   return -1;
}

static inline int
epoll_wait(int                 epfd EINA_UNUSED,
           struct epoll_event *events EINA_UNUSED,
           int                 maxevents EINA_UNUSED,
           int                 timeout EINA_UNUSED)
{
   return -1;
}

static inline int
epoll_ctl(int                 epfd EINA_UNUSED,
          int                 op EINA_UNUSED,
          int                 fd EINA_UNUSED,
          struct epoll_event *event EINA_UNUSED)
{
   return -1;
}
#endif

//////////////////////////////////////////////////////////////////////////
#ifdef HAVE_SYS_TIMERFD_H
# include <sys/timerfd.h>
#else
/* fallback code if we don't have real timerfd - reduces number of ifdefs  */
# ifndef CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC 0 /* bogus value */
# endif
# ifndef TFD_NONBLOCK
#  define TFD_NONBLOCK    0 /* bogus value */
# endif
#endif /* HAVE_SYS_TIMERFD_H */

#ifndef TFD_TIMER_ABSTIME
# define TFD_TIMER_ABSTIME (1 << 0)
#endif
#ifndef TFD_TIMER_CANCEL_ON_SET
# define TFD_TIMER_CANCEL_ON_SET (1 << 1)
#endif

#ifndef TIME_T_MAX
# define TIME_T_MAX (time_t)((1UL << ((sizeof(time_t) << 3) - 1)) - 1)
#endif

#ifndef HAVE_TIMERFD_CREATE
static inline int
timerfd_create(int clockid EINA_UNUSED,
               int flags EINA_UNUSED)
{
   return -1;
}

static inline int
timerfd_settime(int                      fd EINA_UNUSED,
                int                      flags EINA_UNUSED,
                const struct itimerspec *new_value EINA_UNUSED,
                struct itimerspec       *old_value EINA_UNUSED)
{
   return -1;
}
#endif /* HAVE_TIMERFD_CREATE */
