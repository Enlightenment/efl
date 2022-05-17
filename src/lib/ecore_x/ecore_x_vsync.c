#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>

#ifdef HAVE_PRCTL
# include <sys/prctl.h>
#endif

#define ECORE_X_VSYNC_DRM 1

static Ecore_X_Window vsync_root = 0;

int _ecore_x_image_shm_check(void);

static int _vsync_log_dom = -1;

static double _ecore_x_vsync_animator_tick_delay = 0.0;

#undef ERR
#define ERR(...) EINA_LOG_DOM_ERR(_vsync_log_dom, __VA_ARGS__)

#undef DBG
#define DBG(...) EINA_LOG_DOM_DBG(_vsync_log_dom, __VA_ARGS__)

#undef INF
#define INF(...) EINA_LOG_DOM_INFO(_vsync_log_dom, __VA_ARGS__)

#undef WRN
#define WRN(...) EINA_LOG_DOM_WARN(_vsync_log_dom, __VA_ARGS__)

#undef CRI
#define CRI(...) EINA_LOG_DOM_CRIT(_vsync_log_dom, __VA_ARGS__)



#ifdef ECORE_X_VSYNC_DRM
// relevant header bits of dri/drm inlined here to avoid needing external
// headers to build
/// drm
typedef enum
{
   DRM_VBLANK_ABSOLUTE = 0x00000000,
   DRM_VBLANK_RELATIVE = 0x00000001,
   DRM_VBLANK_EVENT = 0x04000000,
   DRM_VBLANK_FLIP = 0x08000000,
   DRM_VBLANK_NEXTONMISS = 0x10000000,
   DRM_VBLANK_SECONDARY = 0x20000000,
   DRM_VBLANK_SIGNAL = 0x40000000
}
drmVBlankSeqType;

typedef struct _drmVBlankReq
{
   drmVBlankSeqType type;
   unsigned int     sequence;
   unsigned long    signal;
} drmVBlankReq;

typedef struct _drmVBlankReply
{
   drmVBlankSeqType type;
   unsigned int     sequence;
   long             tval_sec;
   long             tval_usec;
} drmVBlankReply;

typedef union _drmVBlank
{
   drmVBlankReq   request;
   drmVBlankReply reply;
} drmVBlank;

#define DRM_EVENT_CONTEXT_VERSION 2

typedef struct _drmEventContext
{
   int version;
   void (*vblank_handler)(int fd,
                          unsigned int sequence,
                          unsigned int tv_sec,
                          unsigned int tv_usec,
                          void *user_data);
   void (*page_flip_handler)(int fd,
                             unsigned int sequence,
                             unsigned int tv_sec,
                             unsigned int tv_usec,
                             void *user_data);
} drmEventContext;

typedef struct _drmVersionBroken
{
   int version_major;
   int version_minor;
//   int version_patchlevel;
   size_t name_len;
   // WARNING! this does NOT match the system drm.h headers because
   // literally drm.h is wrong. the below is correct. drm hapily
   // broke its ABI at some point.
   char *name;
   size_t date_len;
   char *date;
   size_t desc_len;
   char *desc;
} drmVersionBroken;

typedef struct _drmVersion
{
   int version_major;
   int version_minor;
   int version_patchlevel;
   size_t name_len;
   // WARNING! this does NOT match the system drm.h headers because
   // literally drm.h is wrong. the below is correct. drm hapily
   // broke its ABI at some point.
   char *name;
   size_t date_len;
   char *date;
   size_t desc_len;
   char *desc;
} drmVersion;

static int (*sym_drmClose)(int fd) = NULL;
static int (*sym_drmWaitVBlank)(int fd,
                                drmVBlank *vbl) = NULL;
static int (*sym_drmHandleEvent)(int fd,
                                 drmEventContext *evctx) = NULL;
static void *(*sym_drmGetVersion)(int fd) = NULL;
static void (*sym_drmFreeVersion)(void *drmver) = NULL;
static int drm_fd = -1;
static volatile int drm_event_is_busy = 0;
static int drm_animators_interval = 1;
static drmEventContext drm_evctx;
static double _drm_fail_time = 0.1;
static double _drm_fail_time2 = 1.0 / 60.0;
static int _drm_fail_count = 0;

static void *drm_lib = NULL;

static Eina_Thread_Queue *thq = NULL;
static Ecore_Thread *drm_thread = NULL;
static Eina_Spinlock tick_queue_lock;
static int           tick_queue_count = 0;
static Eina_Bool     tick_skip = EINA_FALSE;
static Eina_Bool     threaded_vsync = EINA_TRUE;
static Ecore_Timer  *fail_timer = NULL;
static Ecore_Timer  *fallback_timer = NULL;

typedef struct
{
   Eina_Thread_Queue_Msg head;
   char val;
} Msg;

#if 0
# define D(args...) fprintf(stderr, ##args)
#else
# define D(args...)
#endif

static void _drm_send_time(double t);

static Eina_Bool
_fallback_timeout(void *data EINA_UNUSED)
{
   if (drm_event_is_busy)
     {
        _drm_send_time(ecore_time_get());
        return EINA_TRUE;
     }
   fallback_timer = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_fail_timeout(void *data EINA_UNUSED)
{
   fail_timer = NULL;
   _drm_fail_count++;
   if (_drm_fail_count >= 10)
     {
        _drm_fail_count = 10;
        if (!fallback_timer)
          fallback_timer = ecore_timer_add
            (1.0 / 60.0, _fallback_timeout, NULL);
     }
   if (drm_event_is_busy)
     _drm_send_time(ecore_time_get());
   return EINA_FALSE;
}

static Eina_Bool
_drm_tick_schedule(void)
{
   drmVBlank vbl;

   DBG("sched...");
   vbl.request.type = DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT;
   vbl.request.sequence = drm_animators_interval;
   vbl.request.signal = 0;
   if (sym_drmWaitVBlank(drm_fd, &vbl) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

static void
_tick_send(char val)
{
   Msg *msg;
   void *ref;
   DBG("_tick_send(%i)", val);
   msg = eina_thread_queue_send(thq, sizeof(Msg), &ref);
   msg->val = val;
   eina_thread_queue_send_done(thq, ref);
}

static void
_drm_tick_begin(void *data EINA_UNUSED)
{
   _drm_fail_count = 0;
   drm_event_is_busy = 1;
   if (threaded_vsync)
     {
        _tick_send(1);
     }
   else
     {
        if (fail_timer) ecore_timer_reset(fail_timer);
        else fail_timer = ecore_timer_add(1.0 / 15.0, _fail_timeout, NULL);
        if (_drm_fail_count < 10)
          {
             if (!_drm_tick_schedule())
               {
                  _drm_fail_count = 999999;
                  if (!fallback_timer)
                    fallback_timer = ecore_timer_add
                      (1.0 / 60.0, _fallback_timeout, NULL);
               }
          }
        else
          {
             if (!_drm_tick_schedule())
               _drm_fail_count = 999999;
             if (!fallback_timer)
               fallback_timer = ecore_timer_add
                 (1.0 / 60.0, _fallback_timeout, NULL);
          }
     }
}

static void
_drm_tick_end(void *data EINA_UNUSED)
{
   _drm_fail_count = 0;
   drm_event_is_busy = 0;
   if (threaded_vsync)
     {
        _tick_send(0);
     }
   else
     {
        if (fail_timer)
          {
             ecore_timer_del(fail_timer);
             fail_timer = NULL;
          }
        if (fallback_timer)
          {
             ecore_timer_del(fallback_timer);
             fallback_timer = NULL;
          }
     }
}

static void
_drm_send_time(double t)
{
   if (threaded_vsync)
     {
        static double t_last = 0.0;
        double *tim = malloc(sizeof(*tim));

        // you won't believe this
        if (t <= t_last)
          {
             fprintf(stderr, "EEEEEEK! time went backwards! %1.5f -> %1.5f\n", t_last, t);
             t = ecore_time_get();
             if (t <= t_last) t = t_last + 0.001;
          }
        if (tim)
          {
             *tim = t;
             DBG("   ... send %1.8f", t);
             // if we are the wm/compositor we need to offset out vsync by 1/2
             // a frame ... we should never offset by more than
             // frame_time - render_time though ... but we don't know what
             // this is and this varies... so for now this will do.a
             if (_ecore_x_vsync_animator_tick_delay > 0.0)
               {
                  static double t_delta_hist[10] = { 0.0 };
                  double t_delta = t - t_last;
                  double t_delta_min = 0.0;
                  double t_sleep = 0.0;

                  // if time delta is sane like 1/20th of a sec or less..
                  if (t_delta < (1.0 / 20.0))
                    {
                       int i;

                       for (i = 0; i < 9; i++)
                         t_delta_hist[i] = t_delta_hist[i + 1];
                       t_delta_hist[9] = t_delta;
                       t_delta_min = t_delta_hist[0];
                       for (i = 1; i < 10; i++)
                         {
                            if (t_delta_hist[i] < t_delta_min)
                              t_delta_min = t_delta_hist[i];
                         }
                       t_sleep = t_delta_min * _ecore_x_vsync_animator_tick_delay;
                       // if w'ere sleeping too long - don't sleep at all.
                       if (t_sleep > (1.0 / 20.0)) t_sleep = 0.0;
                    }
                  if (t_sleep > 0.0) usleep(t_sleep * 1000000.0);
               }
             D("    @%1.5f   ... send %1.8f\n", ecore_time_get(), t);
             eina_spinlock_take(&tick_queue_lock);
             tick_queue_count++;
             eina_spinlock_release(&tick_queue_lock);
             ecore_thread_feedback(drm_thread, tim);
          }
        t_last = t;
     }
   else
     {
        if (drm_event_is_busy)
          {
             if (_drm_fail_count == 0)
               {
                  if (fallback_timer)
                    {
                       ecore_timer_del(fallback_timer);
                       fallback_timer = NULL;
                    }
               }
             ecore_loop_time_set(t);
             ecore_animator_custom_tick();
             if (drm_event_is_busy)
               {
                  if (fail_timer) ecore_timer_reset(fail_timer);
                  else fail_timer = ecore_timer_add(1.0 / 15.0, _fail_timeout, NULL);
                  if (!_drm_tick_schedule())
                    _drm_fail_count = 999999;
               }
          }
     }
}

static void
_drm_vblank_handler(int fd EINA_UNUSED,
                    unsigned int frame,
                    unsigned int sec,
                    unsigned int usec,
                    void *data EINA_UNUSED)
{
   if (drm_event_is_busy)
     {
        static unsigned int pframe = 0;

        DBG("vblank %i", frame);
        D("    @%1.5f vblank %i\n", ecore_time_get(), frame);
        if (pframe != frame)
          {
#if 0 // disable timestamp from vblank and use time event arrived
             double t = (double)sec + ((double)usec / 1000000);
             unsigned long long tusec, ptusec, tdelt = 0;
             static unsigned int psec = 0, pusec = 0;

             tusec = ((unsigned long long)sec) * 1000000 + usec;
             ptusec = ((unsigned long long)psec) * 1000000 + pusec;
             if (tusec <= ptusec)
               {
                  fprintf(stderr,
                          "EEEEEEK! drm time went backwards! %u.%06u -> %u.%06u\n",
                          psec, pusec, sec, usec);
               }
             else
               {
                  if (frame > pframe)
                    {
                       tdelt = (tusec - ptusec) / (frame - pframe);
                       // go back in time 1/8th of a frame to account for
                       // vlnbak gap - this should be enough for now.
                       // probably need to be a bit more accurate.
                       // 
                       // why do this? because the timestamp is the time
                       // the top-left pixel is first displayed which is
                       // after the vlbank gap time
                       t -= (double)(tdelt / 8) / 1000000.0;
                    }
               }
             _drm_fail_count = 0;
             pusec = usec;
             psec = sec;
#else
             double t = ecore_time_get();
             _drm_send_time(t);
             sec = 0;
             usec = 0;
#endif
             pframe = frame;
          }
     }
   else
     {
        D("    @%1.5f vblank drm event when not busy!\n", ecore_time_get());
     }
}

static double _ecore_x_vsync_wakeup_time = 0.0;

EAPI double _ecore_x_vsync_wakeup_time_get(void)
{
   return _ecore_x_vsync_wakeup_time;
}

static void
_drm_tick_core(void *data EINA_UNUSED, Ecore_Thread *thread)
{
   Msg *msg;
   void *ref;
   int tick = 0;

   eina_thread_name_set(eina_thread_self(), "Eanimator-vsync");
#ifdef HAVE_PRCTL
   prctl(PR_SET_TIMERSLACK, 1, 0, 0, 0);
#endif
   while (!ecore_thread_check(thread))
     {
        DBG("------- drm_event_is_busy=%i", drm_event_is_busy);
        D("    @%1.5f ------- drm_event_is_busy=%i\n", ecore_time_get(), drm_event_is_busy);
        if (!drm_event_is_busy)
          {
             DBG("wait...");
             D("    @%1.5f wait...\n", ecore_time_get());
             msg = eina_thread_queue_wait(thq, &ref);
             if (msg)
               {
                  tick = msg->val;
                  eina_thread_queue_wait_done(thq, ref);
               }
          }
        else
          {
             do
               {
                  DBG("poll...");
                  D("    @%1.5f poll...\n", ecore_time_get());
                  msg = eina_thread_queue_poll(thq, &ref);
                  if (msg)
                    {
                       tick = msg->val;
                       eina_thread_queue_wait_done(thq, ref);
                    }
               }
             while (msg);
          }
        DBG("tick = %i", tick);
        D("    @%1.5f tick = %i\n", ecore_time_get(), tick);
        if (tick == -1)
          {
             drm_thread = NULL;
             eina_thread_queue_free(thq);
             thq = NULL;
             return;
          }
        else if (tick)
          {
             fd_set rfds, wfds, exfds;
             int max_fd;
             int ret;
             struct timeval tv;

             if (!_drm_tick_schedule())
               {
                  D("    @%1.5f schedule fail\n", ecore_time_get());
                  _drm_fail_count = 999999;
               }
             max_fd = 0;
             FD_ZERO(&rfds);
             FD_ZERO(&wfds);
             FD_ZERO(&exfds);
             FD_SET(drm_fd, &rfds);
             max_fd = drm_fd;
             tv.tv_sec = 0;
             if (_drm_fail_count >= 10)
               tv.tv_usec = _drm_fail_time2 * 1000000;
             else
               tv.tv_usec = _drm_fail_time * 1000000;
             D("    @%1.5f wait %ims\n", ecore_time_get(), (int)(tv.tv_usec /1000));
             ret = select(max_fd + 1, &rfds, &wfds, &exfds, &tv);
             _ecore_x_vsync_wakeup_time = ecore_time_get();
#if 0
             static double pt = 0.0;
             double t = ecore_time_get();
             double f = 1.0 / (t - pt);
             char buf[1024];
             int i, fps;
             fps = 30 + ((f - 60.0) * 10.0);
             if (fps > 1000) fps = 1000;
             for (i = 0; i < fps; i++) buf[i] = '#';
             buf[i] = 0;
             printf("WAKE %1.5f [%s>\n", 1.0 / (t - pt), buf);
             pt = t;
#endif
             if ((ret == 1) && (FD_ISSET(drm_fd, &rfds)))
               {
                  D("    @%1.5f have event\n", ecore_time_get());
                  sym_drmHandleEvent(drm_fd, &drm_evctx);
                  _drm_fail_count = 0;
               }
             else if (ret == 0)
               {
                  // timeout
                  _drm_send_time(ecore_time_get());
                  _drm_fail_count++;
                  D("    @%1.5f fail count %i\n", ecore_time_get(), _drm_fail_count);
               }
          }
     }
}

static void
_drm_tick_notify(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED, void *msg)
{
   int tick_queued;

   eina_spinlock_take(&tick_queue_lock);
   tick_queued = tick_queue_count;
   tick_queue_count--;
   eina_spinlock_release(&tick_queue_lock);
   DBG("notify.... %3.3f %i", *((double *)msg), drm_event_is_busy);
   D("notify.... %3.3f %i\n", *((double *)msg), drm_event_is_busy);
   if (drm_event_is_busy)
     {
        double *t = msg, rt, lt;
        static double pt = 0.0, prt = 0.0, plt = 0.0;

        rt = ecore_time_get();
        lt = ecore_loop_time_get();
        DBG("VSYNC %1.8f = delt %1.8f | real = %1.8f | loop = %1.8f", *t, *t - pt, rt - prt, lt - plt);
        D("VSYNC %1.8f = delt %1.8f | real = %1.8f | loop = %1.8f", *t, *t - pt, rt - prt, lt - plt);
//        printf("VSYNC %1.8f = delt %1.5f | real = %1.5f | loop = %1.5f\n", *t, 1.0 / (*t - pt), 1.0 / (rt - prt), 1.0 / (lt - plt));
        if ((!tick_skip) || (tick_queued == 1))
          {
             ecore_loop_time_set(*t);
             ecore_animator_custom_tick();
          }
        pt = *t;
        prt = rt;
        plt = lt;
     }
   free(msg);
}

static Eina_Bool
_ecore_vsync_fd_handler(void *data EINA_UNUSED,
                        Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   _ecore_x_vsync_wakeup_time = ecore_time_get();
   sym_drmHandleEvent(drm_fd, &drm_evctx);
   return ECORE_CALLBACK_RENEW;
}

// yes. most evil. we dlopen libdrm and libGL etc. to manually find smbols
// so we can be as compatible as possible given the whole mess of the
// gl/dri/drm etc. world. and handle graceful failure at runtime not
// compile time
static int
_drm_link(void)
{
   const char *drm_libs[] =
   {
      "libdrm.so.2",
      "libdrm.so.1",
      "libdrm.so.0",
      "libdrm.so",
      NULL,
   };
   int i, fail;
#define SYM(lib, xx)                         \
   do {                                      \
      sym_ ## xx = dlsym(lib, #xx);          \
      if (!(sym_ ## xx)) {                   \
         fail = 1;                           \
      }                                      \
   } while (0)

   if (drm_lib) return 1;
   for (i = 0; drm_libs[i]; i++)
     {
        drm_lib = dlopen(drm_libs[i], RTLD_LOCAL | RTLD_LAZY);
        if (drm_lib)
          {
             fail = 0;
             SYM(drm_lib, drmClose);
             SYM(drm_lib, drmWaitVBlank);
             SYM(drm_lib, drmHandleEvent);
             SYM(drm_lib, drmGetVersion);
             SYM(drm_lib, drmFreeVersion);
             if (fail)
               {
                  dlclose(drm_lib);
                  drm_lib = NULL;
               }
             else break;
          }
     }
   if (!drm_lib) return 0;
   return 1;
}

#define DRM_HAVE_NVIDIA 1

static Eina_Bool
glob_match(const char *glob, const char *str)
{
   if (!glob) return EINA_TRUE;
   if (!str) return EINA_FALSE;
   if (eina_fnmatch(glob, str, 0)) return EINA_TRUE;
   return EINA_FALSE;
}

static int
_drm_init(int *flags)
{
   // whitelist of known-to-work drivers
   struct whitelist_card
     {
        const char *name_glob;
        const char *desc_glob;
        const char *date_glob;
        int drm_ver_min_maj;
        int drm_ver_min_min;
        int kernel_ver_min_maj;
        int kernel_ver_min_min;
     };
   static const struct whitelist_card whitelist[] = {
      { "exynos",  "*Samsung*", NULL,                 1,  6,    3,  0 },
      { "i915",    "*Intel*",   NULL,                 1,  6,    3, 14 },
      { "radeon",  "*Radeon*",  NULL,                 2, 39,    3, 14 },
      { "amdgpu",  "*AMD*",     NULL,                 3,  0,    4,  9 },
      { "nouveau", "*nVidia*",  "201[23456789]*",     1,  3,    4,  9 },
      { "nouveau", "*nVidia*",  "202[0123456789]*",   1,  3,    4,  9 },
      { NULL, NULL, NULL, 0, 0, 0, 0 }
   };
   int i;
   struct stat st;
   char buf[512];
   Eina_Bool ok = EINA_FALSE;
   int vmaj = 0, vmin = 0;
   FILE *fp;

   // vboxvideo 4.3.14 is crashing when calls drmWaitVBlank()
   // https://www.virtualbox.org/ticket/13265
   // also affects 4.3.12
   if (stat("/sys/module/vboxvideo", &st) == 0) return 0;

   // only do this on new kernels = let's say 3.14 and up. 3.16 definitely
   // works
   fp = fopen("/proc/sys/kernel/osrelease", "rb");
   if (fp)
     {
        if (fgets(buf, sizeof(buf), fp))
          {
             if (sscanf(buf, "%i.%i.%*s", &vmaj, &vmin) == 2)
               {
                  if (vmaj >= 3) ok = EINA_TRUE;
               }
          }
        fclose(fp);
     }
   if (!ok) return 0;
   ok = EINA_FALSE;

   snprintf(buf, sizeof(buf), "/dev/dri/card1");
   if (stat(buf, &st) == 0)
     {
        // XXX: 2 dri cards - ambiguous. unknown device for screen
        if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
          fprintf(stderr, "You have 2 DRI cards. Don't know which to use for vsync\n");
        return 0;
     }
   snprintf(buf, sizeof(buf), "/dev/dri/card0");
   if (stat(buf, &st) != 0)
     {
        if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
          fprintf(stderr, "Cannot find device card 0 (/de/dri/card0)\n");
        return 0;
     }
   drm_fd = open(buf, O_RDWR | O_CLOEXEC);
   if (drm_fd < 0)
     {
        if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
          fprintf(stderr, "Cannot open device card 0 (/de/dri/card0)\n");
        return 0;
     }

   if (!getenv("ECORE_VSYNC_DRM_ALL"))
     {
        drmVersion *drmver;
        drmVersionBroken *drmverbroken;

        drmver = sym_drmGetVersion(drm_fd);
        drmverbroken = (drmVersionBroken *)drmver;
        if (!drmver)
          {
             if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
               fprintf(stderr, "Cannot get dri version info from drmGetVersion()\n");
             close(drm_fd);
             return 0;
          }
        // sanity check the drm version structure due to public versions
        // not matching the real memory layout, check drm version
        // is recent (1.6+) and name and sec ptrs exist AND their lengths are
        // not garbage (within a sensible range)
        if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
          {
             if ((drmverbroken->name > (char *)4000L) &&
                 (drmverbroken->date_len < 200))
              fprintf(stderr,
                      "!BROKEN DRM! Do FIXUP of ABI\n"
                      "DRM Version: %i.%i\n"
                      "Name:        '%s'\n"
                      "Date:        '%s'\n"
                      "Desc:        '%s'\n",
                      drmverbroken->version_major, drmverbroken->version_minor,
                      drmverbroken->name, drmverbroken->date, drmverbroken->desc);
             else
               fprintf(stderr,
                       "OK DRM\n"
                       "DRM Version: %i.%i\n"
                       "Name:        '%s'\n"
                       "Date:        '%s'\n"
                       "Desc:        '%s'\n",
                       drmver->version_major, drmver->version_minor,
                       drmver->name, drmver->date, drmver->desc);
          }

        if ((((drmver->version_major == 1) &&
              (drmver->version_minor >= 3)) ||
             (drmver->version_major > 1)) &&
            (drmver->name > (char *)4000L) &&
            (drmver->date_len < 200))
          {
             if ((!strcmp(drmver->name, "nvidia-drm")) &&
                 (strstr(drmver->desc, "NVIDIA DRM driver")))
               {
                  if (((vmaj >= 3) && (vmin >= 14)) || (vmaj >= 4))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "You have nVidia binary drivers - no vsync\n");
                       *flags |= DRM_HAVE_NVIDIA;
                       goto checkdone;
                    }
               }
             for (i = 0; whitelist[i].name_glob; i++)
               {
                  if ((glob_match(whitelist[i].name_glob, drmver->name)) &&
                      (glob_match(whitelist[i].desc_glob, drmver->desc)) &&
                      (glob_match(whitelist[i].date_glob, drmver->date)) &&
                      ((drmver->version_major > whitelist[i].drm_ver_min_maj) ||
                       ((drmver->version_major == whitelist[i].drm_ver_min_maj) &&
                        (drmver->version_minor >= whitelist[i].drm_ver_min_min))) &&
                      ((vmaj > whitelist[i].kernel_ver_min_maj) ||
                       ((vmaj == whitelist[i].kernel_ver_min_maj) &&
                        (vmin >= whitelist[i].kernel_ver_min_min))))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "Whitelisted %s OK\n",
                                 whitelist[i].name_glob);
                       ok = EINA_TRUE;
                       goto checkdone;
                    }
               }
          }
        else if ((((drmverbroken->version_major == 1) &&
                   (drmverbroken->version_minor >= 3)) ||
                  (drmverbroken->version_major > 1)) &&
                 (drmverbroken->name > (char *)4000L) &&
                 (drmverbroken->date_len < 200))
          {
             if ((!strcmp(drmverbroken->name, "nvidia-drm")) &&
                 (strstr(drmverbroken->desc, "NVIDIA DRM driver")))
               {
                  if (((vmaj >= 3) && (vmin >= 14)) || (vmaj >= 4))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "You have nVidia binary drivers - no vsync\n");
                       *flags |= DRM_HAVE_NVIDIA;
                       goto checkdone;
                    }
               }
             for (i = 0; whitelist[i].name_glob; i++)
               {
                  if ((glob_match(whitelist[i].name_glob, drmverbroken->name)) &&
                      (glob_match(whitelist[i].desc_glob, drmverbroken->desc)) &&
                      (glob_match(whitelist[i].date_glob, drmverbroken->date)) &&
                      ((drmverbroken->version_major > whitelist[i].drm_ver_min_maj) ||
                       ((drmverbroken->version_major == whitelist[i].drm_ver_min_maj) &&
                        (drmverbroken->version_minor >= whitelist[i].drm_ver_min_min))) &&
                      ((vmaj > whitelist[i].kernel_ver_min_maj) ||
                       ((vmaj == whitelist[i].kernel_ver_min_maj) &&
                        (vmin >= whitelist[i].kernel_ver_min_min))))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "Whitelisted %s OK\n",
                                 whitelist[i].name_glob);
                       ok = EINA_TRUE;
                       goto checkdone;
                    }
               }
          }
checkdone:
        sym_drmFreeVersion(drmver);
        if (!ok)
          {
             close(drm_fd);
             return 0;
          }
     }

   memset(&drm_evctx, 0, sizeof(drm_evctx));
   drm_evctx.version = DRM_EVENT_CONTEXT_VERSION;
   drm_evctx.vblank_handler = _drm_vblank_handler;
   drm_evctx.page_flip_handler = NULL;

   if (!_drm_tick_schedule())
     {
        if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
          fprintf(stderr, "Cannot schedule vblank tick.event...\n");
        close(drm_fd);
        drm_fd = -1;
        return 0;
     }

   if (getenv("ECORE_ANIMATOR_SKIP")) tick_skip = EINA_TRUE;
   if (threaded_vsync)
     {
        tick_queue_count = 0;
        eina_spinlock_new(&tick_queue_lock);
        thq = eina_thread_queue_new();
        drm_thread = ecore_thread_feedback_run(_drm_tick_core, _drm_tick_notify,
                                               NULL, NULL, NULL, EINA_TRUE);
     }
   else
     {
        ecore_main_fd_handler_add(drm_fd, ECORE_FD_READ,
                                  _ecore_vsync_fd_handler, NULL,
                                  NULL, NULL);
     }
   return 1;
}

static Eina_Bool
_drm_animator_tick_source_set(void)
{
   if (vsync_root)
     {
        ecore_animator_custom_source_tick_begin_callback_set
          (_drm_tick_begin, NULL);
        ecore_animator_custom_source_tick_end_callback_set
          (_drm_tick_end, NULL);
        ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_CUSTOM);
     }
   else
     {
        if (drm_fd >= 0)
          {
             _drm_tick_end(NULL);
             ecore_animator_custom_source_tick_begin_callback_set
               (NULL, NULL);
             ecore_animator_custom_source_tick_end_callback_set
               (NULL, NULL);
             ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_TIMER);
          }
     }
   return EINA_TRUE;
}
#endif







// XXX: missing mode 3 == separate x connection with compiled in dri2 proto
// handling ala mesa (taken from mesa likely)

static int mode = 0;

static void
_vsync_init(void)
{
   static int done = 0;
   struct stat stb;
   int flags = 0;

   if (done) return;

   _vsync_log_dom = eina_log_domain_register("ecore_x_vsync", EINA_COLOR_LIGHTRED);
   if (_ecore_x_image_shm_check())
     {
#ifdef ECORE_X_VSYNC_DRM
        // preferred inline drm if possible
        if (!stat("/dev/dri/card0", &stb))
          {
             if (_drm_link())
               {
                  if (_drm_init(&flags)) mode = 1;
               }
          }
#endif
     }
   done = 1;
}

EAPI Eina_Bool
ecore_x_vsync_animator_tick_source_set(Ecore_X_Window win)
{
   Ecore_X_Window root;
   static int vsync_veto = -1;

   if (vsync_veto == -1)
     {
        char buf[4096];
        const char *home;
        struct stat st;

        if (getenv("ECORE_VSYNC_THREAD")) threaded_vsync = EINA_TRUE;
        if (getenv("ECORE_VSYNC_NO_THREAD")) threaded_vsync = EINA_FALSE;
        home = eina_environment_home_get();
        if (!home) eina_environment_tmp_get();
        snprintf(buf, sizeof(buf), "%s/.ecore-no-vsync", home);
        if (getenv("ECORE_NO_VSYNC")) vsync_veto = 1;
        else if (stat(buf, &st) == 0) vsync_veto = 1;
        else if (stat("/etc/.ecore-no-vsync", &st) == 0) vsync_veto = 1;
        else vsync_veto = 0;
     }
   if (vsync_veto == 1) return EINA_FALSE;

   root = ecore_x_window_root_get(win);
   if (root != vsync_root)
     {
        _vsync_init();
        vsync_root = root;
#ifdef ECORE_X_VSYNC_DRM
        if (mode == 1) return _drm_animator_tick_source_set();
#endif
     }
   return EINA_TRUE;
}

EAPI void
ecore_x_vsync_animator_tick_delay_set(double delay)
{
   _ecore_x_vsync_animator_tick_delay = delay;
}
