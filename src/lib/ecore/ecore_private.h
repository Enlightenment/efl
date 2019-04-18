#ifndef _ECORE_PRIVATE_H
#define _ECORE_PRIVATE_H

#include <assert.h>

#include "ecore_internal.h"

#include "ecore_exe_eo.h"
#include "ecore_event_message.eo.h"
#include "ecore_event_message_handler.eo.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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
#endif

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

typedef struct _Efl_Loop_Timer_Data Efl_Loop_Timer_Data;
typedef struct _Efl_Loop_Future_Scheduler Efl_Loop_Future_Scheduler;
typedef struct _Efl_Loop_Data Efl_Loop_Data;

typedef struct _Efl_Task_Data Efl_Task_Data;
typedef struct _Efl_Appthread_Data Efl_Appthread_Data;

typedef struct _Message Message;

struct _Message
{
   EINA_INLIST;
   Eo *handler;
   Eo *message;
   Eina_Bool delete_me;
};

struct _Efl_Loop_Future_Scheduler
{
   Eina_Future_Scheduler  eina_future_scheduler;
   const Eo              *loop;
   Efl_Loop_Data         *loop_data;
};

struct _Efl_Loop_Data
{
   double               loop_time;
   Eina_Hash           *providers;

   Efl_Loop_Future_Scheduler future_scheduler;

   Efl_Loop_Message_Handler *future_message_handler;

   Efl_Loop_Timer      *poll_high;
   Efl_Loop_Timer      *poll_medium;
   Efl_Loop_Timer      *poll_low;

   Eina_List           *exes; // only used in main loop (for now?)

   Eina_List           *fd_handlers_obj;

   Ecore_Fd_Handler    *fd_handlers;
   Eina_List           *fd_handlers_with_prep;
   Eina_List           *file_fd_handlers;
   Eina_List           *always_fd_handlers;
   Eina_List           *fd_handlers_with_buffer;
   Eina_List           *fd_handlers_to_delete;
   Ecore_Fd_Handler    *fd_handlers_to_call;
   Ecore_Fd_Handler    *fd_handlers_to_call_current;

# ifdef _WIN32
   Ecore_Win32_Handler *win32_handlers;
   Ecore_Win32_Handler *win32_handler_current;
   Eina_List           *win32_handlers_to_delete;
# endif

   Eina_Inlist         *message_queue;
   unsigned int         message_walking;

   unsigned int         throttle;

   int                  epoll_fd;
   pid_t                epoll_pid;
   int                  timer_fd;

   double               last_check;
   Eina_Inlist         *timers;
   Eina_Inlist         *suspended;
   Efl_Loop_Timer_Data *timer_current;
   int                  timers_added;

   Eina_Value           exit_code;

   int                  idlers;
   int                  in_loop;

   struct {
      int               high;
      int               medium;
      int               low;
   } pollers;

   struct {
      char      **environ_ptr;
      char      **environ_copy;
   } env;

   Eina_Bool            do_quit;
};

struct _Efl_Task_Data
{
   Eina_Stringshare  *command;
   Eina_Array        *args;
   Efl_Task_Priority  priority;
   int                exit_code;
   Efl_Task_Flags     flags;
   Eina_Bool          command_dirty : 1;
   Eina_Bool          exited : 1;
};

struct _Efl_Appthread_Data
{
   int read_listeners;
   struct {
      int in, out;
      Eo *in_handler, *out_handler;
      Eina_Bool can_read : 1;
      Eina_Bool eos_read : 1;
      Eina_Bool can_write : 1;
   } fd, ctrl;
   void *thdat;
};

struct _Ecore_Animator
{
   EINA_INLIST;

   Ecore_Task_Cb     func;
   void             *data;

   double            start, run;
   Ecore_Timeline_Cb run_func;
   void             *run_data;

   void             *ee;

   Eina_Bool         delete_me : 1;
   Eina_Bool         suspended : 1;
   Eina_Bool         just_added : 1;
};

typedef struct _Ecore_Evas_Object_Animator_Interface
{
   Ecore_Animator *(*timeline_add)(void *obj, double runtime, Ecore_Timeline_Cb func, const void *data);
   Ecore_Animator *(*add)(void *obj, Ecore_Task_Cb func, const void *data);
   void (*freeze)(Ecore_Animator *animator);
   void (*thaw)(Ecore_Animator *animator);
   void *(*del)(Ecore_Animator *animator);
} Ecore_Evas_Object_Animator_Interface;

EAPI void ecore_evas_object_animator_init(Ecore_Evas_Object_Animator_Interface *iface);

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
void         _efl_loop_timer_enable_new(Eo *obj, Efl_Loop_Data *pd);
double       _efl_loop_timer_next_get(Eo *obj, Efl_Loop_Data *pd);
void         _efl_loop_timer_expired_timers_call(Eo *obj, Efl_Loop_Data *pd, double when);
int          _efl_loop_timers_exists(Eo *obj, Efl_Loop_Data *pd);
int          _efl_loop_timer_expired_call(Eo *obj, Efl_Loop_Data *pd, double when);

Ecore_Factorized_Idle *_ecore_factorized_idle_add(const Efl_Callback_Array_Item*desc,
                                                  Ecore_Task_Cb func,
                                                  const void   *data);
void        *_ecore_factorized_idle_del(Ecore_Idler *idler);
void    _ecore_factorized_idle_process(void *data, const Efl_Event *event);
void    _ecore_factorized_idle_event_del(void *data, const Efl_Event *event);

Eina_Future_Scheduler *_ecore_event_future_scheduler_get(void);

Eina_Bool    _ecore_event_init(void);
void         _ecore_event_shutdown(void);
int          _ecore_event_exist(void);
Ecore_Event *_ecore_event_add(int type,
                              void *ev,
                              Ecore_End_Cb func_free,
                              void *data);
void         _ecore_event_call(void);

Efl_Loop_Timer *_ecore_exe_doomsday_clock_get(Ecore_Exe *exe);
void            _ecore_exe_doomsday_clock_set(Ecore_Exe *exe, Efl_Loop_Timer *dc);

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

Ecore_Fd_Handler *_ecore_main_fd_handler_add(Eo *obj,
                                             Efl_Loop_Data *pd,
                                             Eo *handler,
                                             int fd,
                                             Ecore_Fd_Handler_Flags flags,
                                             Ecore_Fd_Cb func,
                                             const void *data,
                                             Ecore_Fd_Cb buf_func,
                                             const void *buf_data,
                                             Eina_Bool is_file);
void      *_ecore_main_fd_handler_del(Eo *obj,
                                      Efl_Loop_Data *pd,
                                      Ecore_Fd_Handler *fd_handler);
Ecore_Win32_Handler *
_ecore_main_win32_handler_add(Eo                    *obj,
                              Efl_Loop_Data         *pd,
                              Eo                    *handler,
                              void                  *h,
                              Ecore_Win32_Handle_Cb  func,
                              const void            *data);
void *
_ecore_main_win32_handler_del(Eo *obj,
                              Efl_Loop_Data *pd,
                              Ecore_Win32_Handler *win32_handler);

void       _ecore_main_content_clear(Eo *obj, Efl_Loop_Data *pd);
void       _ecore_main_shutdown(void);

#if defined (_WIN32) || defined (__lv2ppu__) || defined (HAVE_EXOTIC)
static inline void _ecore_signal_shutdown(void) { }

static inline void _ecore_signal_init(void) { }

static inline void _ecore_signal_received_process(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd EINA_UNUSED) { }

static inline int _ecore_signal_count_get(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd EINA_UNUSED) { return 0; }

static inline void _ecore_signal_call(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd EINA_UNUSED) { }

#else
#define ECORE_SIGNALS 1
typedef struct _Ecore_Signal_Pid_Info Ecore_Signal_Pid_Info;

struct _Ecore_Signal_Pid_Info
{
   pid_t pid;
   int exit_code;
   int exit_signal;
   siginfo_t info;
};

void _ecore_signal_shutdown(void);
void _ecore_signal_init(void);
void _ecore_signal_received_process(Eo *obj, Efl_Loop_Data *pd);
int  _ecore_signal_count_get(Eo *obj, Efl_Loop_Data *pd);
void _ecore_signal_call(Eo *obj, Efl_Loop_Data *pd);
void _ecore_signal_pid_lock(void);
void _ecore_signal_pid_unlock(void);
void _ecore_signal_pid_register(pid_t pid, int fd);
void _ecore_signal_pid_unregister(pid_t pid, int fd);

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

void _ecore_main_loop_iterate(Eo *obj, Efl_Loop_Data *pd);
int  _ecore_main_loop_iterate_may_block(Eo *obj, Efl_Loop_Data *pd, int may_block);
void _ecore_main_loop_begin(Eo *obj, Efl_Loop_Data *pd);
void _ecore_main_loop_quit(Eo *obj, Efl_Loop_Data *pd);

void _ecore_coroutine_init(void);
void _ecore_coroutine_shutdown(void);

void _ecore_throttle(void);

void _ecore_main_call_flush(void);

void _ecore_main_timechanges_start(Eo *obj);
void _ecore_main_timechanges_stop(Eo *obj);

Eina_Bool _ecore_event_do_filter(void *handler_pd, Eo *msg_handler, Eo *msg);
void _ecore_event_filters_call(Eo *obj, Efl_Loop_Data *pd);
void _efl_loop_messages_filter(Eo *obj, Efl_Loop_Data *pd, void *handler_pd);
void _efl_loop_messages_call(Eo *obj, Efl_Loop_Data *pd, void *func, void *data);

void _efl_loop_message_send_info_set(Eo *obj, Eina_Inlist *node, Eo *loop, Efl_Loop_Data *loop_data);
void _efl_loop_message_unsend(Eo *obj);

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
//GENERIC_ALLOC_FREE_HEADER(Ecore_Event_Handler, ecore_event_handler);
//GENERIC_ALLOC_FREE_HEADER(Ecore_Event_Filter, ecore_event_filter);
//GENERIC_ALLOC_FREE_HEADER(Ecore_Event, ecore_event);
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
extern Efl_Loop_Data *_mainloop_singleton_data;
#define ML_OBJ _mainloop_singleton
#define ML_DAT _mainloop_singleton_data
//#define ML_DAT efl_data_scope_get(ML_OBJ, EFL_LOOP_CLASS)

extern Efl_Version _app_efl_version;

// access to direct input cb
#define ECORE_EVAS_INTERNAL

#define EFL_LOOP_DATA efl_data_scope_get(efl_main_loop_get(), EFL_LOOP_CLASS)

#undef EAPI
#define EAPI

#endif
