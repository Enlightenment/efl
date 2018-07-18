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
   _tick_send(1);
}

static void
_drm_tick_end(void *data EINA_UNUSED)
{
   _drm_fail_count = 0;
   drm_event_is_busy = 0;
   _tick_send(0);
}

static void
_drm_send_time(double t)
{
   double *tim = malloc(sizeof(*tim));
   if (tim)
     {
        *tim = t;
        DBG("   ... send %1.8f", t);
        D("    @%1.5f   ... send %1.8f\n", ecore_time_get(), t);
        eina_spinlock_take(&tick_queue_lock);
        tick_queue_count++;
        eina_spinlock_release(&tick_queue_lock);
        ecore_thread_feedback(drm_thread, tim);
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
#define DELTA_COUNT 10
             double t = (double)sec + ((double)usec / 1000000);
             double tnow = ecore_time_get();
             static double tdelta[DELTA_COUNT];
             static double tdelta_avg = 0.0;
             static int tdelta_n = 0;

             if (t > tnow)
               {
                  if (tdelta_n > DELTA_COUNT)
                    {
                       t = t + tdelta_avg;
                    }
                  else if (tdelta_n < DELTA_COUNT)
                    {
                       tdelta[tdelta_n] = tnow - t;
                       tdelta_n++;
                       t = tnow;
                    }
                  else if (tdelta_n == DELTA_COUNT)
                    {
                       int i;

                       for (i = 0; i < DELTA_COUNT; i++)
                         tdelta_avg += tdelta[i];
                       tdelta_avg /= (double)(DELTA_COUNT);
                       tdelta_n++;
                    }
               }
             else
               {
                  tdelta_avg = 0.0;
                  tdelta_n = 0;
               }
             _drm_send_time(t);
             pframe = frame;
          }
     }
   else
     {
        D("    @%1.5f vblank drm event when not busy!\n", ecore_time_get());
     }
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
             D("    @1.5f wait...\n", ecore_time_get());
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
        double *t = msg;
        static double pt = 0.0;

        DBG("VSYNC %1.8f = delt %1.8f", *t, *t - pt);
        D("VSYNC %1.8f = delt %1.8f\n", *t, *t - pt);
        if ((!tick_skip) || (tick_queued == 1))
          {
             ecore_loop_time_set(*t);
             ecore_animator_custom_tick();
          }
        pt = *t;
     }
   free(msg);
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

static int
_drm_init(int *flags)
{
   struct stat st;
   char buf[512];
   Eina_Bool ok = EINA_FALSE;
   int vmaj = 0, vmin = 0;

   // vboxvideo 4.3.14 is crashing when calls drmWaitVBlank()
   // https://www.virtualbox.org/ticket/13265
   // also affects 4.3.12
   if (stat("/sys/module/vboxvideo", &st) == 0)
     {
/*
        FILE *fp = fopen("/sys/module/vboxvideo/version", "rb");
        if (fp)
          {
             if (fgets(buf, sizeof(buf), fp))
               {
                  if (eina_str_has_prefix(buf, "4.3.14"))
                    {
                       fclose(fp);
                       return 0;
                    }
               }
             fclose(fp);
          }
 */
        return 0;
     }
   // only do this on new kernels = let's say 3.14 and up. 3.16 definitely
   // works
     {
        FILE *fp = fopen("/proc/sys/kernel/osrelease", "rb");
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
     }
   ok = EINA_FALSE;

   snprintf(buf, sizeof(buf), "/dev/dri/card1");
   if (stat(buf, &st) == 0)
     {
        // XXX: 2 dri cards - ambiguous. unknown device for screen
        return 0;
     }
   snprintf(buf, sizeof(buf), "/dev/dri/card0");
   if (stat(buf, &st) != 0) return 0;
   drm_fd = open(buf, O_RDWR | O_CLOEXEC);
   if (drm_fd < 0) return 0;

   if (!getenv("ECORE_VSYNC_DRM_ALL"))
     {
        drmVersion *drmver;
        drmVersionBroken *drmverbroken;

        drmver = sym_drmGetVersion(drm_fd);
        drmverbroken = (drmVersionBroken *)drmver;
        if (!drmver)
          {
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
              (drmver->version_minor >= 6)) ||
             (drmver->version_major > 1)) &&
            (drmver->name > (char *)4000L) &&
            (drmver->date_len < 200))
          {
             // whitelist of known-to-work drivers
             if ((!strcmp(drmver->name, "exynos")) &&
                 (strstr(drmver->desc, "Samsung")))
               {
                  if (((vmaj >= 3) && (vmin >= 0)) || (vmaj >= 4))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "Whitelisted exynos OK\n");
                       ok = EINA_TRUE;
                       goto checkdone;
                    }
               }
             if ((!strcmp(drmver->name, "i915")) &&
                 (strstr(drmver->desc, "Intel Graphics")))
               {
                  if (((vmaj >= 3) && (vmin >= 14)) || (vmaj >= 4))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "Whitelisted intel OK\n");
                       ok = EINA_TRUE;
                       goto checkdone;
                    }
               }
             if ((!strcmp(drmver->name, "radeon")) &&
                 (strstr(drmver->desc, "Radeon")) &&
                 (((drmver->version_major == 2) &&
                   (drmver->version_minor >= 39)) ||
                  (drmver->version_major > 2)))
               {
                  if (((vmaj >= 3) && (vmin >= 14)) || (vmaj >= 4))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "Whitelisted radeon OK\n");
                       ok = EINA_TRUE;
                       goto checkdone;
                    }
               }
             if ((!strcmp(drmver->name, "amdgpu")) &&
                 (strstr(drmver->desc, "AMD")) &&
                 ((drmver->version_major > 2)))
               {
                  if (((vmaj >= 3) && (vmin >= 14)) || (vmaj >= 4))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "Whitelisted amdgpu OK\n");
                       ok = EINA_TRUE;
                       goto checkdone;
                    }
               }
          }
        if ((((drmverbroken->version_major == 1) &&
              (drmverbroken->version_minor >= 6)) ||
             (drmverbroken->version_major > 1)) &&
            (drmverbroken->name > (char *)4000L) &&
            (drmverbroken->date_len < 200))
          {
             // whitelist of known-to-work drivers
             if ((!strcmp(drmverbroken->name, "exynos")) &&
                 (strstr(drmverbroken->desc, "Samsung")))
               {
                  if (((vmaj >= 3) && (vmin >= 0)) || (vmaj >= 4))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "Whitelisted exynos OK\n");
                       ok = EINA_TRUE;
                       goto checkdone;
                    }
               }
             if ((!strcmp(drmverbroken->name, "i915")) &&
                 (strstr(drmverbroken->desc, "Intel Graphics")))
               {
                  if (((vmaj >= 3) && (vmin >= 14)) || (vmaj >= 4))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "Whitelisted intel OK\n");
                       ok = EINA_TRUE;
                       goto checkdone;
                    }
               }
             if ((!strcmp(drmverbroken->name, "radeon")) &&
                 (strstr(drmverbroken->desc, "Radeon")) &&
                 (((drmver->version_major == 2) &&
                   (drmver->version_minor >= 39)) ||
                  (drmver->version_major > 2)))
               {
                  if (((vmaj >= 3) && (vmin >= 14)) || (vmaj >= 4))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "Whitelisted radeon OK\n");
                       ok = EINA_TRUE;
                       goto checkdone;
                    }
               }
             if ((!strcmp(drmverbroken->name, "amdgpu")) &&
                 (strstr(drmverbroken->desc, "AMD")) &&
                 ((drmverbroken->version_major > 2)))
               {
                  if (((vmaj >= 3) && (vmin >= 14)) || (vmaj >= 4))
                    {
                       if (getenv("ECORE_VSYNC_DRM_VERSION_DEBUG"))
                         fprintf(stderr, "Whitelisted amdgpu OK\n");
                       ok = EINA_TRUE;
                       goto checkdone;
                    }
               }
          }
        if ((drmver->version_major >= 0) &&
            (drmver->version_minor >= 0) &&
            (drmver->name > (char *)4000L) &&
            (drmver->date_len < 200))
          {
             if ((!strcmp(drmver->name, "nvidia-drm")) &&
                 (strstr(drmver->desc, "NVIDIA DRM driver")))
               {
                  if (((vmaj >= 3) && (vmin >= 14)) || (vmaj >= 4))
                    {
                       *flags |= DRM_HAVE_NVIDIA;
                       goto checkdone;
                    }
               }
          }
        if ((drmverbroken->version_major >= 0) &&
            (drmverbroken->version_minor >= 0) &&
            (drmverbroken->name > (char *)4000L) &&
            (drmverbroken->date_len < 200))
          {
             if ((!strcmp(drmverbroken->name, "nvidia-drm")) &&
                 (strstr(drmverbroken->desc, "NVIDIA DRM driver")))
               {
                  if (((vmaj >= 3) && (vmin >= 14)) || (vmaj >= 4))
                    {
                       *flags |= DRM_HAVE_NVIDIA;
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
        close(drm_fd);
        drm_fd = -1;
        return 0;
     }

   if (getenv("ECORE_ANIMATOR_SKIP")) tick_skip = EINA_TRUE;
   tick_queue_count = 0;
   eina_spinlock_new(&tick_queue_lock);
   thq = eina_thread_queue_new();
   drm_thread = ecore_thread_feedback_run(_drm_tick_core, _drm_tick_notify,
                                          NULL, NULL, NULL, EINA_TRUE);
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
