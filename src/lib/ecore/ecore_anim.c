#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

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

#include <Eo.h>

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS ECORE_ANIMATOR_CLASS

#define MY_CLASS_NAME "Ecore_Animator"

#define ECORE_ANIMATOR_CHECK(obj)                       \
  if (!eo_isa((obj), ECORE_ANIMATOR_CLASS)) \
    return

struct _Ecore_Animator_Data
{
   EINA_INLIST;
   Ecore_Animator   *obj;

   Ecore_Task_Cb     func;
   void             *data;

   double            start, run;
   Ecore_Timeline_Cb run_func;
   void             *run_data;

   Eina_Bool         delete_me : 1;
   Eina_Bool         suspended : 1;
   Eina_Bool         just_added : 1;
};

typedef struct _Ecore_Animator_Data Ecore_Animator_Data;

static Eina_Bool _do_tick(void);
static Eina_Bool _ecore_animator_run(void *data);

static int animators_delete_me = 0;
static Ecore_Animator_Data *animators = NULL;
static double animators_frametime = 1.0 / 30.0;
static unsigned int animators_suspended = 0;

static Ecore_Animator_Source src = ECORE_ANIMATOR_SOURCE_TIMER;
static int ticking = 0;
static Ecore_Cb begin_tick_cb = NULL;
static const void *begin_tick_data = NULL;
static Ecore_Cb end_tick_cb = NULL;
static const void *end_tick_data = NULL;
static Eina_Bool animator_ran = EINA_FALSE;

static int timer_fd_read = -1;
static int timer_fd_write = -1;
static Ecore_Thread *timer_thread = NULL;
static volatile int timer_event_is_busy = 0;

static void
_tick_send(char val)
{
   DBG("_tick_send(%i)", val);
   if (pipe_write(timer_fd_write, &val, 1) != 1)
     {
        ERR("Cannot write to animator control fd");
     }
}

static void
_timer_send_time(double t)
{
   double *tim = malloc(sizeof(*tim));
   if (tim)
     {
        *tim = t;
        DBG("   ... send %1.8f", t);
        ecore_thread_feedback(timer_thread, tim);
     }
}

static void
_timer_tick_core(void *data EINA_UNUSED, Ecore_Thread *thread)
{
   fd_set rfds, wfds, exfds;
   struct timeval tv;
   unsigned int t;
   char tick = 0;
   double t0, d;
   int ret;

   eina_thread_name_set(eina_thread_self(), "Eanimator-timer");
   while (!ecore_thread_check(thread))
     {
        DBG("------- timer_event_is_busy=%i", timer_event_is_busy);
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&exfds);
        FD_SET(timer_fd_read, &rfds);

        t0 = ecore_time_get();
        d = fmod(t0, animators_frametime);
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
          {
             if (pipe_read(timer_fd_read, &tick, sizeof(tick)) != 1)
               {
                  ERR("Cannot read from animator control fd");
               }
             DBG("tick = %i", tick);
             if (tick == -1) goto done;
          }
        else
          {
             if (tick) _timer_send_time(t0 - d + animators_frametime);
          }
     }
done:
   pipe_close(timer_fd_read);
   timer_fd_read = -1;
   pipe_close(timer_fd_write);
   timer_fd_write = -1;
}

static void
_timer_tick_notify(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED, void *msg)
{
   DBG("notify.... %3.3f %i", *((double *)msg), timer_event_is_busy);
   if (timer_event_is_busy)
     {
        double *t = msg;
        static double pt = 0.0;

        DBG("VSYNC %1.8f = delt %1.8f", *t, *t - pt);
        ecore_loop_time_set(*t);
        _do_tick();
        pt = *t;
     }
   free(msg);
}

static void
_timer_tick_finished(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED)
{
   timer_thread = NULL;
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
        timer_fd_read = fds[0];
        timer_fd_write = fds[1];
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
   int i;

   if (timer_fd_read < 0) return;
   _tick_send(-1);
   for (i = 0; (i < 500) && (timer_thread); i++)
     {
        usleep(1000);
     }
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
        _timer_tick_begin();
        break;

      case ECORE_ANIMATOR_SOURCE_CUSTOM:
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

   _timer_tick_end();

   if ((src == ECORE_ANIMATOR_SOURCE_CUSTOM) && end_tick_cb)
     end_tick_cb((void *)end_tick_data);
}

static Eina_Bool
_do_tick(void)
{
   Ecore_Animator_Data *animator;

   EINA_INLIST_FOREACH(animators, animator)
     {
        animator->just_added = EINA_FALSE;
     }
   if (animators) eina_evlog("!FRAME", NULL, ecore_loop_time_get(), NULL);
   EINA_INLIST_FOREACH(animators, animator)
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
   if (animators_delete_me)
     {
        Ecore_Animator_Data *l;
        for (l = animators; l; )
          {
             animator = l;
             l = (Ecore_Animator_Data *)EINA_INLIST_GET(l)->next;
             if (animator->delete_me)
               {
                  if (animator->suspended) animators_suspended--;
                  animators = (Ecore_Animator_Data *)
                    eina_inlist_remove(EINA_INLIST_GET(animators),
                                       EINA_INLIST_GET(animator));

                  eo_do(animator->obj, eo_parent_set(NULL));
                  if (eo_destructed_is(animator->obj))
                     eo_manual_free(animator->obj);
                  else
                     eo_manual_free_set(animator->obj, EINA_FALSE);

                  animators_delete_me--;
                  if (animators_delete_me == 0) break;
               }
          }
     }
   if (!_have_animators())
     {
        _end_tick();
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_animator_add(Ecore_Animator *obj,
                    Ecore_Animator_Data *animator,
                    Ecore_Task_Cb func,
                    const void   *data)
{
    if (EINA_UNLIKELY(!eina_main_loop_is()))
      {
         EINA_MAIN_LOOP_CHECK_RETURN_VAL(EINA_FALSE);
      }

   animator->obj = obj;
   eo_manual_free_set(obj, EINA_TRUE);

   if (!func)
     {
        ERR("callback function must be set up for an object of class: '%s'", MY_CLASS_NAME);
        return EINA_FALSE;
     }

   animator->func = func;
   animator->data = (void *)data;
   animator->just_added = EINA_TRUE;
   animators = (Ecore_Animator_Data *)eina_inlist_append(EINA_INLIST_GET(animators), EINA_INLIST_GET(animator));
   _begin_tick();
   return EINA_TRUE;
}

EAPI Ecore_Animator *
ecore_animator_add(Ecore_Task_Cb func,
                   const void   *data)
{
   Ecore_Animator *animator = NULL;

   animator = eo_add(MY_CLASS, _ecore_parent,
                            ecore_animator_constructor(func, data));
   return animator;
}

EOLIAN static void
_ecore_animator_constructor(Eo *obj, Ecore_Animator_Data *animator, Ecore_Task_Cb func, const void *data)
{
   _ecore_animator_add(obj, animator, func, data);
}

EAPI Ecore_Animator *
ecore_animator_timeline_add(double            runtime,
                            Ecore_Timeline_Cb func,
                            const void       *data)
{
   Ecore_Animator *animator;
   animator = eo_add(MY_CLASS, _ecore_parent,
                            ecore_animator_timeline_constructor(runtime, func, data));
   return animator;
}

EOLIAN static void
_ecore_animator_timeline_constructor(Eo *obj, Ecore_Animator_Data *animator, double runtime, Ecore_Timeline_Cb func, const void *data)
{
   if (runtime <= 0.0) runtime = 0.0;

   if (!_ecore_animator_add(obj, animator, _ecore_animator_run, NULL)) return;

   animator->data = obj;
   animator->run_func = func;
   animator->run_data = (void *)data;
   animator->start = ecore_loop_time_get();
   animator->run = runtime;
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
         if (current_slope == 0.0) return guess_t;
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
    if (x1 == y1 && x2 == y2) return pos;
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
ecore_animator_del(Ecore_Animator *obj)
{
   void *data = NULL;

   if (!obj) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   Ecore_Animator_Data *animator = eo_data_scope_get(obj, MY_CLASS);

   if (!animator) return NULL;
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
   return data;
}

EOLIAN static void
_ecore_animator_eo_base_destructor(Eo *obj, Ecore_Animator_Data *pd)
{
   pd->delete_me = EINA_TRUE;
   animators_delete_me++;

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

EOLIAN static Eo *
_ecore_animator_eo_base_finalize(Eo *obj, Ecore_Animator_Data *pd)
{
   if (!pd->func)
     {
        return NULL;
     }

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_finalize());
}

EAPI void
ecore_animator_frametime_set(double frametime)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (frametime < 0.0) frametime = 0.0;
   if (animators_frametime == frametime) return ;
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
   ECORE_ANIMATOR_CHECK(animator);
   eo_do(animator, eo_event_freeze());
}

EOLIAN static void
_ecore_animator_eo_base_event_freeze(Eo *obj EINA_UNUSED, Ecore_Animator_Data *animator)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
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
   ECORE_ANIMATOR_CHECK(animator);
   eo_do(animator, eo_event_thaw());
}

EOLIAN static void
_ecore_animator_eo_base_event_thaw(Eo *obj EINA_UNUSED, Ecore_Animator_Data *animator)
{
   EINA_MAIN_LOOP_CHECK_RETURN;

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
   _timer_tick_quit();
   _end_tick();
   while (animators)
     {
        Ecore_Animator_Data *animator;

        animator = animators;
        if (animator->suspended) animators_suspended--;
        animators = (Ecore_Animator_Data *)eina_inlist_remove(EINA_INLIST_GET(animators), EINA_INLIST_GET(animators));

        eo_do(animator->obj, eo_parent_set(NULL));
        if (eo_destructed_is(animator->obj))
           eo_manual_free(animator->obj);
        else
           eo_manual_free_set(animator->obj, EINA_FALSE);
     }
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
   Ecore_Animator *obj = data;
   Ecore_Animator_Data *animator = eo_data_scope_get(obj, MY_CLASS);

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
   if (t >= (animator->start + animator->run)) run_ret = EINA_FALSE;
   return run_ret;
}

#include "ecore_animator.eo.c"
