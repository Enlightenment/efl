#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(HAVE_SYS_EPOLL_H) && defined(HAVE_SYS_TIMERFD_H)
# define HAVE_EPOLL   1
# include <sys/epoll.h>
# include <sys/timerfd.h>
#endif

#ifdef _WIN32

# include <winsock2.h>

# define pipe_write(fd, buffer, size) send((fd), (char *)(buffer), size, 0)
# define pipe_read(fd, buffer, size)  recv((fd), (char *)(buffer), size, 0)
# define pipe_close(fd)               closesocket(fd)
# define PIPE_FD_ERROR   SOCKET_ERROR

#else

# include <sys/select.h>
# include <fcntl.h>

# define pipe_write(fd, buffer, size) write((fd), buffer, size)
# define pipe_read(fd, buffer, size)  read((fd), buffer, size)
# define pipe_close(fd)               close(fd)
# define PIPE_FD_ERROR   -1

#endif /* ! _WIN32 */

#ifdef HAVE_PRCTL
# include <sys/prctl.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

struct _Ecore_Animator
{
   EINA_INLIST;

   Ecore_Task_Cb     func;
   void             *data;

   double            start, run;
   Ecore_Timeline_Cb run_func;
   void             *run_data;

   Eina_Bool         delete_me : 1;
   Eina_Bool         suspended : 1;
   Eina_Bool         just_added : 1;
};

static int _ecore_anim_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_anim_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_anim_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ecore_anim_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_anim_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_ecore_anim_log_dom, __VA_ARGS__)

static Eina_Bool _do_tick(void);
static Eina_Bool _ecore_animator_run(void *data);

static int animators_delete_me = 0;
static Ecore_Animator *animators = NULL;
static volatile double animators_frametime = 1.0 / 60.0;
static unsigned int animators_suspended = 0;

static Ecore_Animator_Source src = ECORE_ANIMATOR_SOURCE_TIMER;
static int ticking = 0;
static Ecore_Cb begin_tick_cb = NULL;
static const void *begin_tick_data = NULL;
static Ecore_Cb end_tick_cb = NULL;
static const void *end_tick_data = NULL;
static Eina_Bool animator_ran = EINA_FALSE;

static volatile int timer_fd_read = -1;
static volatile int timer_fd_write = -1;
static Ecore_Thread *timer_thread = NULL;
static volatile int timer_event_is_busy = 0;
static Eina_Spinlock tick_queue_lock;
static int           tick_queue_count = 0;
static Eina_Bool     tick_skip = EINA_FALSE;

static void
_tick_send(signed char val)
{
   DBG("_tick_send(%i)", val);
   if (pipe_write(timer_fd_write, &val, 1) != 1)
     {
        ERR("Cannot write to animator control fd");
     }
}

static void
_timer_send_time(double t, Ecore_Thread *thread)
{
   double *tim = malloc(sizeof(*tim));
   if (tim)
     {
        *tim = t;
        DBG("   ... send %1.8f", t);
        eina_spinlock_take(&tick_queue_lock);
        tick_queue_count++;
        eina_spinlock_release(&tick_queue_lock);
        ecore_thread_feedback(thread, tim);
     }
}

static void
_timer_tick_core(void *data EINA_UNUSED, Ecore_Thread *thread)
{
#ifdef HAVE_EPOLL
   int pollfd = -1, timerfd = -1;
   struct epoll_event pollev = { 0 };
   struct epoll_event pollincoming[2];
   uint64_t timerfdbuf;
   int i;
   unsigned int t_ft;
   double pframetime = -1.0;
   struct itimerspec tspec_new;
   struct itimerspec tspec_old;
#endif
   fd_set rfds, wfds, exfds;
   struct timeval tv;
   Eina_Bool data_control;
   Eina_Bool data_timeout;
   unsigned int t;
   signed char tick = 0;
   double t0, d, ft;
   int ret;

   eina_thread_name_set(eina_thread_self(), "Eanimator-timer");
#ifdef HAVE_PRCTL
   prctl(PR_SET_TIMERSLACK, 1, 0, 0, 0);
#endif

#ifdef HAVE_EPOLL
   pollfd = epoll_create(1);
   if (pollfd >= 0) eina_file_close_on_exec(pollfd, EINA_TRUE);

#if defined(TFD_NONBLOCK) && defined(TFD_CLOEXEC)
   timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
#endif
   if (timerfd < 0)
     {
        timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
        if (timerfd >= 0) eina_file_close_on_exec(timerfd, EINA_TRUE);
     }
   if (timerfd < 0)
     {
        close(pollfd);
        pollfd = -1;
     }

#define INPUT_TIMER_CONTROL ((void *) ((unsigned long) 0x11))
#define INPUT_TIMER_TIMERFD ((void *) ((unsigned long) 0x22))

   if (pollfd >= 0)
     {
        pollev.data.ptr = INPUT_TIMER_CONTROL;
        pollev.events = EPOLLIN;
        if (epoll_ctl(pollfd, EPOLL_CTL_ADD, timer_fd_read, &pollev) != 0)
          {
             close(timerfd);
             timerfd = -1;
             close(pollfd);
             pollfd = -1;
          }
        if (pollfd >= 0)
          {
             pollev.data.ptr = INPUT_TIMER_TIMERFD;
             pollev.events = EPOLLIN;
             if (epoll_ctl(pollfd, EPOLL_CTL_ADD, timerfd, &pollev) != 0)
               {
                  close(timerfd);
                  timerfd = -1;
                  close(pollfd);
                  pollfd = -1;
               }
          }
     }

   if (pollfd >= 0)
     {
        while (!ecore_thread_check(thread))
          {
             data_control = EINA_FALSE;
             data_timeout = EINA_FALSE;
             ft = animators_frametime;

             DBG("------- timer_event_is_busy=%i", timer_event_is_busy);

             t0 = ecore_time_get();
             d = fmod(t0, ft);
             if (tick)
               {
                  if (!EINA_DBL_EQ(pframetime, ft))
                    {
                       t = (ft - d) * 1000000000.0;
                       t_ft = ft * 1000000000.0;
                       tspec_new.it_value.tv_sec = t / 1000000000;
                       tspec_new.it_value.tv_nsec = t % 1000000000;
                       tspec_new.it_interval.tv_sec = t_ft / 1000000000;
                       tspec_new.it_interval.tv_nsec = t_ft % 1000000000;
                       timerfd_settime(timerfd, 0, &tspec_new, &tspec_old);
                       pframetime = ft;
                    }
                  DBG("sleep...");
                  ret = epoll_wait(pollfd, pollincoming, 2, -1);
               }
             else
               {
                  tspec_new.it_value.tv_sec = 0;
                  tspec_new.it_value.tv_nsec = 0;
                  tspec_new.it_interval.tv_sec = 0;
                  tspec_new.it_interval.tv_nsec = 0;
                  pframetime = -1.0;
                  timerfd_settime(timerfd, 0, &tspec_new, &tspec_old);
                  DBG("wait...");
                  ret = epoll_wait(pollfd, pollincoming, 2, -1);
               }

             for (i = 0; i < ret; i++)
               {
                  if (pollincoming[i].events & EPOLLIN)
                    {
                       if (pollincoming[i].data.ptr == INPUT_TIMER_TIMERFD)
                         {
                            if (read(timerfd, &timerfdbuf, sizeof(timerfdbuf)) == -1)
                              {
                                 ERR("Cannot read from timer descriptor. %m.");
                              }
                            data_timeout = EINA_TRUE;
                         }
                       else if (pollincoming[i].data.ptr == INPUT_TIMER_CONTROL)
                         data_control = EINA_TRUE;
                    }
               }
             if (data_control)
               {
                  if (pipe_read(timer_fd_read, &tick, sizeof(tick)) != 1)
                    {
                       ERR("Cannot read from animator control fd");
                    }
                  DBG("tick = %i", tick);
                  if (tick == -1) goto done;
               }
             else if (data_timeout)
               {
                  if (tick) _timer_send_time(t0 - d + ft, thread);
               }
          }
     }
   else
#endif
     {
        while (!ecore_thread_check(thread))
          {
             data_control = EINA_FALSE;
             data_timeout = EINA_FALSE;
             ft = animators_frametime;

             DBG("------- timer_event_is_busy=%i", timer_event_is_busy);
             FD_ZERO(&rfds);
             FD_ZERO(&wfds);
             FD_ZERO(&exfds);
             FD_SET(timer_fd_read, &rfds);

             t0 = ecore_time_get();
             d = fmod(t0, ft);
             if (tick)
               {
                  DBG("sleep...");
                  t = (animators_frametime - d) * 1000000.0;
                  tv.tv_sec = t / 1000000;
                  tv.tv_usec = t % 1000000;
                  ret = select(timer_fd_read + 1, &rfds, &wfds, &exfds, &tv);
               }
             else
               {
                  DBG("wait...");
                  ret = select(timer_fd_read + 1, &rfds, &wfds, &exfds, NULL);
               }
             if ((ret == 1) && (FD_ISSET(timer_fd_read, &rfds)))
               data_control = EINA_TRUE;
             else if (ret == 0)
               data_timeout = EINA_TRUE;
             if (data_control)
               {
                  if (pipe_read(timer_fd_read, &tick, sizeof(tick)) != 1)
                    {
                       ERR("Cannot read from animator control fd");
                    }
                  DBG("tick = %i", tick);
                  if (tick == -1) goto done;
               }
             else if (data_timeout)
               {
                  if (tick) _timer_send_time(t0 - d + ft, thread);
               }
          }
     }
done:
#ifdef HAVE_EPOLL
   if (pollfd >= 0)
     {
        close(pollfd);
        pollfd = -1;
     }
   if (timerfd >= 0)
     {
        close(timerfd);
        timerfd = -1;
     }
#endif
   pipe_close(timer_fd_read);
   timer_fd_read = -1;
   pipe_close(timer_fd_write);
   timer_fd_write = -1;
}

static void
_timer_tick_notify(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED, void *msg)
{
   int tick_queued;

   eina_spinlock_take(&tick_queue_lock);
   tick_queued = tick_queue_count;
   tick_queue_count--;
   eina_spinlock_release(&tick_queue_lock);
   DBG("notify.... %3.3f %i", *((double *)msg), timer_event_is_busy);
   if (timer_event_is_busy)
     {
        double *t = msg;
        static double pt = 0.0;

        DBG("VSYNC %1.8f = delt %1.8f", *t, *t - pt);
        if ((!tick_skip) || (tick_queued == 1))
          {
             ecore_loop_time_set(*t);
             _do_tick();
          }
        pt = *t;
     }
   free(msg);
}

static void
_timer_tick_finished(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED)
{
   eina_spinlock_free(&tick_queue_lock);
   timer_thread = NULL;
   tick_queue_count = 0;
   if (timer_fd_read >= 0)
     {
        pipe_close(timer_fd_read);
        timer_fd_read = -1;
     }
   if (timer_fd_write >= 0)
     {
        pipe_close(timer_fd_write);
        timer_fd_write = -1;
     }
}

static void
_timer_tick_begin(void)
{
   if (timer_fd_read < 0)
     {
        int fds[2];

        if (pipe(fds) != 0) return;
        eina_file_close_on_exec(fds[0], EINA_TRUE);
        eina_file_close_on_exec(fds[1], EINA_TRUE);
        timer_fd_read = fds[0];
        timer_fd_write = fds[1];
        if (getenv("ECORE_ANIMATOR_SKIP")) tick_skip = EINA_TRUE;
        tick_queue_count = 0;
        eina_spinlock_new(&tick_queue_lock);
        timer_thread = ecore_thread_feedback_run(_timer_tick_core,
                                                 _timer_tick_notify,
                                                 _timer_tick_finished,
                                                 _timer_tick_finished,
                                                 NULL, EINA_TRUE);
     }
   timer_event_is_busy = 1;
   _tick_send(1);
}

static void
_timer_tick_end(void)
{
   if (timer_fd_read < 0) return;
   timer_event_is_busy = 0;
   _tick_send(0);
}

static void
_timer_tick_quit(void)
{
   if (timer_fd_read < 0) return;
   _tick_send(-1);
   if (timer_thread) ecore_thread_wait(timer_thread, 0.5);
}

static Eina_Bool
_have_animators(void)
{
   if ((animators) &&
       (animators_suspended < eina_inlist_count(EINA_INLIST_GET(animators)))
      )
     return EINA_TRUE;
   return EINA_FALSE;
}

static void
_begin_tick(void)
{
   if (ticking) return;
   eina_evlog(">animator", NULL, 0.0, NULL);
   ticking = 1;
   switch (src)
     {
      case ECORE_ANIMATOR_SOURCE_TIMER:
        DBG("General animator registered with timer source.");
        _timer_tick_begin();
        break;

      case ECORE_ANIMATOR_SOURCE_CUSTOM:
        DBG("General animator registered with custom source.");
        if (begin_tick_cb) begin_tick_cb((void *)begin_tick_data);
        break;

      default:
        break;
     }
}

static void
_end_tick(void)
{
   if (!ticking) return;
   eina_evlog("<animator", NULL, 0.0, NULL);
   ticking = 0;

   DBG("General animator unregistered.");

   _timer_tick_end();

   if ((src == ECORE_ANIMATOR_SOURCE_CUSTOM) && end_tick_cb)
     end_tick_cb((void *)end_tick_data);
}

static Eina_Bool
_do_tick(void)
{
   Ecore_Animator *animator;
   Eina_Inlist *tmp;

   DBG("General animator tick.");
   EINA_INLIST_FOREACH(animators, animator)
     {
        animator->just_added = EINA_FALSE;
     }
   if (animators) eina_evlog("!FRAME", NULL, ecore_loop_time_get(), NULL);
   EINA_INLIST_FOREACH_SAFE(animators, tmp, animator)
     {
        if ((!animator->delete_me) &&
            (!animator->suspended) &&
            (!animator->just_added))
          {
             animator_ran = EINA_TRUE;
             eina_evlog("+animator", animator, 0.0, NULL);
             if (!_ecore_call_task_cb(animator->func, animator->data))
               {
                  animator->delete_me = EINA_TRUE;
                  animators_delete_me++;
               }
             eina_evlog("-animator", animator, 0.0, NULL);
          }
        else animator->just_added = EINA_FALSE;
     }
   if (!_ecore_animator_flush())
     return ECORE_CALLBACK_CANCEL;
   return ECORE_CALLBACK_RENEW;
}

static Ecore_Animator *
_ecore_animator_add(Ecore_Task_Cb func,
                    const void   *data)
{
   Ecore_Animator *animator;

   if (EINA_UNLIKELY(!eina_main_loop_is()))
     {
        EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
     }

   if (!func)
     {
        ERR("callback function must be set up for an Ecore_Animator object.");
        return NULL;
     }

   animator = calloc(1, sizeof (Ecore_Animator));
   if (!animator) return NULL;

   animator->func = func;
   animator->data = (void *)data;
   animator->just_added = EINA_TRUE;
   animators = (Ecore_Animator *)eina_inlist_append(EINA_INLIST_GET(animators), EINA_INLIST_GET(animator));
   _begin_tick();

   return animator;
}

EAPI Ecore_Animator *
ecore_animator_add(Ecore_Task_Cb func,
                   const void   *data)
{
   return _ecore_animator_add(func, data);
}

EAPI Ecore_Animator *
ecore_animator_timeline_add(double            runtime,
                            Ecore_Timeline_Cb func,
                            const void       *data)
{
   Ecore_Animator *animator;

   if (runtime <= 0.0) runtime = 0.0;

   animator = _ecore_animator_add(_ecore_animator_run, NULL);
   if (!animator)
     return NULL;

   animator->data = animator;
   animator->run_func = func;
   animator->run_data = (void *)data;
   animator->start = ecore_loop_time_get();
   animator->run = runtime;

   return animator;
}

static double
_pos_map_sin(double in)
{
   return eina_f32p32_double_to(eina_f32p32_sin(eina_f32p32_double_from(in)));
}

#if 0
static double
_pos_map_cos(double in)
{
   return eina_f32p32_double_to(eina_f32p32_cos(eina_f32p32_double_from(in)));
}
#endif

static double
_pos_map_accel_factor(double pos,
                      double v1)
{
   int i, fact = (int)v1;
   double p, o1 = pos, o2, v;
   p = 1.0 - _pos_map_sin((M_PI / 2.0) + ((pos * M_PI) / 2.0));
   o2 = p;
   for (i = 0; i < fact; i++)
     {
        o1 = o2;
        o2 = o2 * p;
     }
   v = v1 - (double)fact;
   pos = (v * o2) + ((1.0 - v) * o1);
   return pos;
}

static double
_pos_map_pow(double pos,
             double divis,
             int    p)
{
   double v = 1.0;
   int i;
   for (i = 0; i < p; i++) v *= pos;
   return ((pos * divis) * (1.0 - v)) + (pos * v);
}

static double
_pos_map_spring(double pos,
                int    bounces,
                double decfac)
{
   int segnum, segpos, b1, b2;
   double len, decay, decpos, p2;
   if (bounces < 0) bounces = 0;
   p2 = _pos_map_pow(pos, 0.5, 3);
   len = (M_PI / 2.0) + ((double)bounces * M_PI);
   segnum = (bounces * 2) + 1;
   segpos = 2 * (((int)(p2 * segnum) + 1) / 2);
   b1 = segpos;
   b2 = segnum + 1;
   if (b1 < 0) b1 = 0;
   decpos = (double)b1 / (double)b2;
   decay = _pos_map_accel_factor(1.0 - decpos, decfac);
   return _pos_map_sin((M_PI / 2.0) + (p2 * len)) * decay;
}

static double
_cubic_bezier_a (double a1, double a2)
{
    return 1.0 - 3.0 * a2 + 3.0 * a1;
}

static double
_cubic_bezier_b (double a1, double a2)
{
    return 3.0 * a2 - 6.0 * a1;
}

static double
_cubic_bezier_c(double a1)
{
    return 3.0 * a1;
}

static double
_cubic_bezier_calc(double t,
                   double a1,
                   double a2)
{
    return ((_cubic_bezier_a(a1, a2) * t +
             _cubic_bezier_b(a1, a2)) * t +
            _cubic_bezier_c(a1)) * t;
}

static double
_cubic_bezier_slope_get(double t,
                        double a1,
                        double a2)
{
    return 3.0 * _cubic_bezier_a(a1, a2) * t * t +
           2.0 * _cubic_bezier_b(a1, a2) * t +
           _cubic_bezier_c(a1);
}

static double
_cubic_bezier_t_get(double a,
                        double x1,
                        double x2)
{
#define APPROXIMATE_RANGE(val) \
  ((((val) < 0.01) && ((val) > -0.01)) ? EINA_TRUE : EINA_FALSE)

    const int LIMIT = 100;
    double current_slope;
    double change;
    double current_x;
    double guess_t = a;

    for (int i = 0; i < LIMIT; i++)
      {
         current_slope = _cubic_bezier_slope_get(guess_t, x1, x2);
         if (EINA_DBL_EQ(current_slope, 0.0)) return guess_t;
         current_x = _cubic_bezier_calc(guess_t, x1, x2) - a;
         change = current_x / current_slope;
         guess_t -= change;
         if (APPROXIMATE_RANGE(change)) break;
      }
    return guess_t;
}

static double
_pos_map_cubic_bezier(double pos,
                      double x1,
                      double y1,
                      double x2,
                      double y2)
{
   if (EINA_DBL_EQ(x1, y1) &&
       EINA_DBL_EQ(x2, y2))
     return pos;
   return _cubic_bezier_calc(_cubic_bezier_t_get(pos, x1, x2), y1, y2);
}

#define DBL_TO(Fp) eina_f32p32_double_to(Fp)
#define DBL_FROM(D) eina_f32p32_double_from(D)
#define INT_FROM(I) eina_f32p32_int_from(I)
#define SIN(Fp) eina_f32p32_sin(Fp)
#define COS(Fp) eina_f32p32_cos(Fp)
#define ADD(A, B) eina_f32p32_add(A, B)
#define SUB(A, B) eina_f32p32_sub(A, B)
#define MUL(A, B) eina_f32p32_mul(A, B)

EAPI double
ecore_animator_pos_map_n(double        pos,
                         Ecore_Pos_Map map,
                         int           v_size,
                         double       *v)
{
    double v0 = 0, v1 = 0, v2 = 0, v3 = 0;

   /* purely functional - locking not required */
    if (pos >= 1.0) return 1.0;
    else if (pos <= 0.0)
      return 0.0;
    switch (map)
      {
       case ECORE_POS_MAP_LINEAR:
         return pos;

       case ECORE_POS_MAP_ACCELERATE:
	 /* pos = 1 - sin(Pi / 2 + pos * Pi / 2); */
	 pos = DBL_TO(SUB(INT_FROM(1), SIN(ADD((EINA_F32P32_PI >> 1), MUL(DBL_FROM(pos), (EINA_F32P32_PI >> 1))))));
         return pos;

       case ECORE_POS_MAP_DECELERATE:
	 /* pos = sin(pos * Pi / 2); */
	 pos = DBL_TO(SIN(MUL(DBL_FROM(pos), (EINA_F32P32_PI >> 1))));
         return pos;

       case ECORE_POS_MAP_SINUSOIDAL:
	 /* pos = (1 - cos(pos * Pi)) / 2 */
	 pos = DBL_TO((SUB(INT_FROM(1), COS(MUL(DBL_FROM(pos), EINA_F32P32_PI)))) >> 1);
         return pos;

       case ECORE_POS_MAP_ACCELERATE_FACTOR:
         if (v_size > 0) v0 = v[0];
         pos = _pos_map_accel_factor(pos, v0);
         return pos;

       case ECORE_POS_MAP_DECELERATE_FACTOR:
         if (v_size > 0) v0 = v[0];
         pos = 1.0 - _pos_map_accel_factor(1.0 - pos, v0);
         return pos;

       case ECORE_POS_MAP_SINUSOIDAL_FACTOR:
         if (v_size > 0) v0 = v[0];
         if (pos < 0.5) pos = _pos_map_accel_factor(pos * 2.0, v0) / 2.0;
         else pos = 1.0 - (_pos_map_accel_factor((1.0 - pos) * 2.0, v0) / 2.0);
         return pos;

       case ECORE_POS_MAP_DIVISOR_INTERP:
         if (v_size > 0) v0 = v[0];
         if (v_size > 1) v1 = v[1];
         pos = _pos_map_pow(pos, v0, (int)v1);
         return pos;

       case ECORE_POS_MAP_BOUNCE:
         if (v_size > 0) v0 = v[0];
         if (v_size > 1) v1 = v[1];
         pos = _pos_map_spring(pos, (int)v1, v0);
         if (pos < 0.0) pos = -pos;
         pos = 1.0 - pos;
         return pos;

       case ECORE_POS_MAP_SPRING:
         if (v_size > 0) v0 = v[0];
         if (v_size > 1) v1 = v[1];
         pos = 1.0 - _pos_map_spring(pos, (int)v1, v0);
         return pos;

       case ECORE_POS_MAP_CUBIC_BEZIER:
         if (v_size > 0) v0 = v[0];
         if (v_size > 1) v1 = v[1];
         if (v_size > 2) v2 = v[2];
         if (v_size > 3) v3 = v[3];
         pos = _pos_map_cubic_bezier(pos, v0, v1, v2, v3);
         return pos;

       default:
         return pos;
      }

    return pos;
}

EAPI double
ecore_animator_pos_map(double        pos,
                       Ecore_Pos_Map map,
                       double        v1,
                       double        v2)
{
    double v[2];

    v[0] = v1;
    v[1] = v2;
    return ecore_animator_pos_map_n(pos, map, 2, v);
}

EAPI void *
ecore_animator_del(Ecore_Animator *animator)
{
   void *data = NULL;

   if (!animator) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   if (animator->delete_me)
     {
        data = animator->data;
        goto end;
     }

   animator->delete_me = EINA_TRUE;
   animators_delete_me++;
   if (animator->run_func)
     data = animator->run_data;
   else
     data = animator->data;

 end:
   if (!in_main_loop) _ecore_animator_flush();
   return data;
}

EAPI void
ecore_animator_frametime_set(double frametime)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (frametime < 0.0) frametime = 0.0;
   if (EINA_DBL_EQ(animators_frametime, frametime)) return ;
   animators_frametime = frametime;
   _end_tick();
   if (_have_animators()) _begin_tick();
}

EAPI double
ecore_animator_frametime_get(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0.0);
   return animators_frametime;
}

EAPI void
ecore_animator_freeze(Ecore_Animator *animator)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (!animator) return ;
   if (animator->delete_me) return ;
   if (!animator->suspended)
     {
        animator->suspended = EINA_TRUE;
        animators_suspended++;
        if (!_have_animators()) _end_tick();
     }
}

EAPI void
ecore_animator_thaw(Ecore_Animator *animator)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (!animator) return ;
   if (animator->delete_me) return;
   if (animator->suspended)
     {
        animator->suspended = EINA_FALSE;
        animators_suspended--;
        if (_have_animators()) _begin_tick();
     }
}

EAPI void
ecore_animator_source_set(Ecore_Animator_Source source)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   _end_tick();
   src = source;
   DBG("New source set to %s.",
       source == ECORE_ANIMATOR_SOURCE_TIMER ? "TIMER" :
       source == ECORE_ANIMATOR_SOURCE_CUSTOM ? "CUSTOM" :
       "UNKNOWN");
   if (_have_animators()) _begin_tick();
}

EAPI Ecore_Animator_Source
ecore_animator_source_get(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   return src;
}

EAPI void
ecore_animator_custom_source_tick_begin_callback_set(Ecore_Cb    func,
                                                     const void *data)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   _end_tick();
   begin_tick_cb = func;
   begin_tick_data = data;
   if (_have_animators()) _begin_tick();
}

EAPI void
ecore_animator_custom_source_tick_end_callback_set(Ecore_Cb    func,
                                                   const void *data)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   _end_tick();
   end_tick_cb = func;
   end_tick_data = data;
   if (_have_animators()) _begin_tick();
}

EAPI void
ecore_animator_custom_tick(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (src == ECORE_ANIMATOR_SOURCE_CUSTOM) _do_tick();
}

void
_ecore_animator_shutdown(void)
{
   Ecore_Animator *animator;

   _timer_tick_quit();
   _end_tick();

   EINA_INLIST_FREE(animators, animator)
     {
        if (animator->suspended) animators_suspended--;
        if (animator->delete_me) animators_delete_me--;

        animators = (Ecore_Animator *) eina_inlist_remove
              (EINA_INLIST_GET(animators), EINA_INLIST_GET(animator));
        free(animator);
     }

   eina_log_domain_unregister(_ecore_anim_log_dom);
   _ecore_anim_log_dom = -1;
}

void
_ecore_animator_run_reset(void)
{
   animator_ran = EINA_FALSE;
}

Eina_Bool
_ecore_animator_run_get(void)
{
   return animator_ran;
}

static Eina_Bool
_ecore_animator_run(void *data)
{
   Ecore_Animator *animator = data;
   double pos = 0.0, t;
   Eina_Bool run_ret;

   t = ecore_loop_time_get();
   if (animator->run > 0.0)
     {
        pos = (t - animator->start) / animator->run;
        if (pos > 1.0) pos = 1.0;
        else if (pos < 0.0)
          pos = 0.0;
     }
   run_ret = animator->run_func(animator->run_data, pos);
   if (t >= (animator->start + animator->run) && (pos >= 1.0)) run_ret = EINA_FALSE;
   return run_ret;
}

Eina_Bool
_ecore_animator_flush(void)
{
   Ecore_Animator *animator;

   if (animators_delete_me)
     {
        Ecore_Animator *l;
        for (l = animators; l; )
          {
             animator = l;
             l = (Ecore_Animator  *)EINA_INLIST_GET(l)->next;
             if (animator->delete_me)
               {
                  if (animator->suspended) animators_suspended--;
                  animators = (Ecore_Animator *)
                    eina_inlist_remove(EINA_INLIST_GET(animators),
                                       EINA_INLIST_GET(animator));

                  free(animator);

                  animators_delete_me--;
                  if (animators_delete_me == 0) break;
               }
          }
     }
   if (!_have_animators())
     {
        _end_tick();
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

void
_ecore_animator_init(void)
{
   _ecore_anim_log_dom = eina_log_domain_register("ecore_animator", ECORE_DEFAULT_LOG_COLOR);
   if (_ecore_anim_log_dom < 0)
     {
        EINA_LOG_ERR("Ecore was unable to create a log domain.");
     }
}
