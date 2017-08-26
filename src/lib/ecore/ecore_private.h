#ifndef _ECORE_PRIVATE_H
#define _ECORE_PRIVATE_H

#include <assert.h>

#include "ecore_internal.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

extern int _ecore_log_dom;
#ifdef  _ECORE_DEFAULT_LOG_DOM
# undef _ECORE_DEFAULT_LOG_DOM
#endif
#define _ECORE_DEFAULT_LOG_DOM _ecore_log_dom

#ifdef ECORE_DEFAULT_LOG_COLOR
# undef ECORE_DEFAULT_LOG_COLOR
#endif
#define ECORE_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ECORE_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ECORE_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ECORE_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ECORE_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_ECORE_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif

#ifndef ABS
# define ABS(x)             ((x) < 0 ? -(x) : (x))
#endif

#ifndef CLAMP
# define CLAMP(x, min, max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#endif

typedef struct _Ecore_Factorized_Idle Ecore_Factorized_Idle;

typedef struct _Efl_Loop_Promise_Simple_Data Efl_Loop_Promise_Simple_Data;

typedef struct _Efl_Loop_Data Efl_Loop_Data;
struct _Efl_Loop_Data
{
   Eina_Hash *providers;

   Ecore_Timer *poll_high;
   Ecore_Timer *poll_medium;
   Ecore_Timer *poll_low;

   int idlers;

   struct {
      int high;
      int medium;
      int low;
   } pollers;
};

#define EVAS_FRAME_QUEUING        1 /* for test */

#define READBUFSIZ                65536

#define ECORE_MAGIC_NONE          0x1234fedc
#define ECORE_MAGIC_EXE           0xf7e812f5
#define ECORE_MAGIC_TIMER         0xf7d713f4
#define ECORE_MAGIC_IDLER         0xf7c614f3
#define ECORE_MAGIC_IDLE_ENTERER  0xf7b515f2
#define ECORE_MAGIC_IDLE_EXITER   0xf7601afd
#define ECORE_MAGIC_FD_HANDLER    0xf7a416f1
#define ECORE_MAGIC_EVENT_HANDLER 0xf79317f0
#define ECORE_MAGIC_EVENT_FILTER  0xf78218ff
#define ECORE_MAGIC_EVENT         0xf77119fe
#define ECORE_MAGIC_ANIMATOR      0xf7643ea5
#define ECORE_MAGIC_POLLER        0xf7568127
#define ECORE_MAGIC_PIPE          0xf7458226
#define ECORE_MAGIC_WIN32_HANDLER 0xf7e8f1a3
#define ECORE_MAGIC_JOB           0x76543210

typedef unsigned int Ecore_Magic;
#define ECORE_MAGIC               Ecore_Magic __magic

#define ECORE_MAGIC_SET(d, m)      (d)->__magic = (m)
#define ECORE_MAGIC_CHECK(d, m)    ((d) && ((d)->__magic == (m)))
#define ECORE_MAGIC_FAIL(d, m, fn) _ecore_magic_fail((d), (d) ? (d)->__magic : 0, (m), (fn));

/* undef the following, we want our version */
#undef FREE
#define FREE(ptr)                  free(ptr); ptr = NULL;

#undef IF_FREE
#define IF_FREE(ptr)               if (ptr) free(ptr); ptr = NULL;

#undef IF_FN_DEL
#define IF_FN_DEL(_fn, ptr)        if (ptr) { _fn(ptr); ptr = NULL; }

EAPI void
ecore_print_warning(const char *function,
                    const char *sparam);

/* convenience macros for checking pointer parameters for non-NULL */
#undef CHECK_PARAM_POINTER_RETURN
#define CHECK_PARAM_POINTER_RETURN(sparam, param, ret) \
  if (!(param))                                        \
    {                                                  \
       ecore_print_warning(__FUNCTION__, sparam);      \
       return ret;                                     \
    }

#undef CHECK_PARAM_POINTER
#define CHECK_PARAM_POINTER(sparam, param)        \
  if (!(param))                                   \
    {                                             \
       ecore_print_warning(__FUNCTION__, sparam); \
       return;                                    \
    }

EAPI void _ecore_magic_fail(const void *d,
                            Ecore_Magic m,
                            Ecore_Magic req_m,
                            const char *fname);

void         _ecore_time_init(void);

void        *_efl_loop_timer_del(Ecore_Timer *timer);
void         _efl_loop_timer_shutdown(void);
void         _efl_loop_timer_enable_new(void);
double       _efl_loop_timer_next_get(void);
void         _efl_loop_timer_expired_timers_call(double when);
int          _efl_loop_timers_exists(void);

int          _efl_loop_timer_expired_call(double when);

Ecore_Factorized_Idle *_ecore_factorized_idle_add(const Efl_Callback_Array_Item*desc,
                                                  Ecore_Task_Cb func,
                                                  const void   *data);
void        *_ecore_factorized_idle_del(Ecore_Idler *idler);
void    _ecore_factorized_idle_process(void *data, const Efl_Event *event);
void    _ecore_factorized_idle_event_del(void *data, const Efl_Event *event);

void         _ecore_idler_all_call(Eo *loop);
int          _ecore_idler_exist(Eo *loop);

void         _ecore_idle_enterer_call(Eo *loop);

void         _ecore_idle_exiter_call(Eo *loop);


Eina_Future_Scheduler *_ecore_event_future_scheduler_get(void);

Eina_Bool    _ecore_event_init(void);
void         _ecore_event_shutdown(void);
int          _ecore_event_exist(void);
Ecore_Event *_ecore_event_add(int type,
                              void *ev,
                              Ecore_End_Cb func_free,
                              void *data);
void         _ecore_event_call(void);
void        *_ecore_event_handler_del(Ecore_Event_Handler *event_handler);

Ecore_Timer *_ecore_exe_doomsday_clock_get(Ecore_Exe *exe);
void         _ecore_exe_doomsday_clock_set(Ecore_Exe *exe,
                                           Ecore_Timer *dc);

void      *_ecore_event_signal_user_new(void);
void      *_ecore_event_signal_hup_new(void);
void      *_ecore_event_signal_exit_new(void);
void      *_ecore_event_signal_power_new(void);
void      *_ecore_event_signal_realtime_new(void);

Ecore_Pipe *_ecore_pipe_add(Ecore_Pipe_Cb handler,
                            const void   *data);
int         _ecore_pipe_wait(Ecore_Pipe *p,
                             int message_count,
                             double wait);
void       *_ecore_pipe_del(Ecore_Pipe *p);

Ecore_Fd_Handler *
           _ecore_main_fd_handler_add(int fd,
                                      Ecore_Fd_Handler_Flags flags,
                                      Ecore_Fd_Cb func,
                                      const void *data,
                                      Ecore_Fd_Cb buf_func,
                                      const void *buf_data,
                                      Eina_Bool is_file);
void      *_ecore_main_fd_handler_del(Ecore_Fd_Handler *fd_handler);

void       _ecore_main_shutdown(void);

#if defined (_WIN32) || defined (__lv2ppu__) || defined (HAVE_EXOTIC)
static inline void _ecore_signal_shutdown(void) { }

static inline void _ecore_signal_init(void) { }

static inline void _ecore_signal_received_process(void) { }

static inline int _ecore_signal_count_get(void) { return 0; }

static inline void _ecore_signal_call(void) { }

#else
void _ecore_signal_shutdown(void);
void _ecore_signal_init(void);
void _ecore_signal_received_process(void);
int  _ecore_signal_count_get(void);
void _ecore_signal_call(void);
#endif

void       _ecore_exe_init(void);
void       _ecore_exe_shutdown(void);
#ifndef _WIN32
Ecore_Exe *_ecore_exe_find(pid_t pid);
void      *_ecore_exe_event_del_new(void);
void       _ecore_exe_event_del_free(void *data,
                                     void *ev);
#endif

void _ecore_animator_init(void);
void _ecore_animator_shutdown(void);
void _ecore_animator_run_reset(void);
Eina_Bool _ecore_animator_run_get(void);
Eina_Bool _ecore_animator_flush(void);

void _ecore_poller_shutdown(void);

void _ecore_fps_debug_init(void);
void _ecore_fps_debug_shutdown(void);
void _ecore_fps_debug_runtime_add(double t);

void _ecore_thread_init(void);
void _ecore_thread_shutdown(void);

void _ecore_glib_init(void);
void _ecore_glib_shutdown(void);

void _ecore_job_init(void);
void _ecore_job_shutdown(void);

void _ecore_main_loop_init(void);
void _ecore_main_loop_shutdown(void);

void _ecore_coroutine_init(void);
void _ecore_coroutine_shutdown(void);

void _ecore_throttle(void);

void _ecore_main_call_flush(void);

static inline Eina_Bool
_ecore_call_task_cb(Ecore_Task_Cb func,
                    void *data)
{
   return func(data);
}

static inline void *
_ecore_call_data_cb(Ecore_Data_Cb func,
                    void *data)
{
   return func(data);
}

static inline void
_ecore_call_end_cb(Ecore_End_Cb func,
                   void *user_data,
                   void *func_data)
{
   func(user_data, func_data);
}

static inline Eina_Bool
_ecore_call_filter_cb(Ecore_Filter_Cb func,
                      void *data,
                      void *loop_data,
                      int type,
                      void *event)
{
   return func(data, loop_data, type, event);
}

static inline Eina_Bool
_ecore_call_handler_cb(Ecore_Event_Handler_Cb func,
                       void *data,
                       int type,
                       void *event)
{
   return func(data, type, event);
}

static inline void
_ecore_call_prep_cb(Ecore_Fd_Prep_Cb func,
                    void *data,
                    Ecore_Fd_Handler *fd_handler)
{
   func(data, fd_handler);
}

static inline Eina_Bool
_ecore_call_fd_cb(Ecore_Fd_Cb func,
                  void *data,
                  Ecore_Fd_Handler *fd_handler)
{
   return func(data, fd_handler);
}

extern int _ecore_fps_debug;
extern double _ecore_time_loop_time;
extern Eina_Bool _ecore_glib_always_integrate;
extern Ecore_Select_Function main_loop_select;
extern int in_main_loop;

Eina_Bool ecore_mempool_init(void);
void ecore_mempool_shutdown(void);
#define GENERIC_ALLOC_FREE_HEADER(TYPE, Type) \
  TYPE *Type##_calloc(unsigned int);          \
  void Type##_mp_free(TYPE *e);
#define GENERIC_ALLOC_SIZE_DECLARE(TYPE)      \
  size_t _ecore_sizeof_##TYPE = sizeof (TYPE);

//GENERIC_ALLOC_FREE_HEADER(Ecore_Animator, ecore_animator);
GENERIC_ALLOC_FREE_HEADER(Ecore_Event_Handler, ecore_event_handler);
GENERIC_ALLOC_FREE_HEADER(Ecore_Event_Filter, ecore_event_filter);
GENERIC_ALLOC_FREE_HEADER(Ecore_Event, ecore_event);
//GENERIC_ALLOC_FREE_HEADER(Ecore_Idle_Exiter, ecore_idle_exiter);
//GENERIC_ALLOC_FREE_HEADER(Ecore_Idle_Enterer, ecore_idle_enterer);
//GENERIC_ALLOC_FREE_HEADER(Ecore_Idler, ecore_idler);
//GENERIC_ALLOC_FREE_HEADER(Ecore_Job, ecore_job);
//GENERIC_ALLOC_FREE_HEADER(Ecore_Timer, ecore_timer);
//GENERIC_ALLOC_FREE_HEADER(Ecore_Poller, ecore_poller);
GENERIC_ALLOC_FREE_HEADER(Ecore_Pipe, ecore_pipe);
GENERIC_ALLOC_FREE_HEADER(Ecore_Fd_Handler, ecore_fd_handler);
GENERIC_ALLOC_FREE_HEADER(Efl_Loop_Promise_Simple_Data, efl_loop_promise_simple_data);
#ifdef _WIN32
GENERIC_ALLOC_FREE_HEADER(Ecore_Win32_Handler, ecore_win32_handler);
#endif

#undef GENERIC_ALLOC_FREE_HEADER

extern Eo *_mainloop_singleton;
extern Efl_Version _app_efl_version;

void ecore_loop_future_register(Efl_Loop *l, Efl_Future *f);
void ecore_loop_future_unregister(Efl_Loop *l, Efl_Future *f);
void ecore_loop_promise_register(Efl_Loop *l, Efl_Promise *p);
void ecore_loop_promise_unregister(Efl_Loop *l, Efl_Promise *p);
void ecore_loop_promise_fulfill(Efl_Promise *p);

// access to direct input cb
#define ECORE_EVAS_INTERNAL

#undef EAPI
#define EAPI

#endif
