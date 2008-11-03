#ifndef _ECORE_PRIVATE_H
#define _ECORE_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

#include <eina_types.h>

#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif

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

#ifdef __GNUC__
# if __GNUC__ >= 4
// BROKEN in gcc 4 on amd64
//#  pragma GCC visibility push(hidden)
# endif
#endif

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif

#ifndef MIN
# define MIN(x, y) (((x) > (y)) ? (y) : (x))
#endif

#ifndef MAX
# define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef ABS
# define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#ifndef CLAMP
# define CLAMP(x, min, max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#endif

#define READBUFSIZ 65536

#define ECORE_MAGIC_NONE            0x1234fedc
#define ECORE_MAGIC_EXE             0xf7e812f5
#define ECORE_MAGIC_TIMER           0xf7d713f4
#define ECORE_MAGIC_IDLER           0xf7c614f3
#define ECORE_MAGIC_IDLE_ENTERER    0xf7b515f2
#define ECORE_MAGIC_IDLE_EXITER     0xf7601afd
#define ECORE_MAGIC_FD_HANDLER      0xf7a416f1
#define ECORE_MAGIC_EVENT_HANDLER   0xf79317f0
#define ECORE_MAGIC_EVENT_FILTER    0xf78218ff
#define ECORE_MAGIC_EVENT           0xf77119fe
#define ECORE_MAGIC_ANIMATOR        0xf7643ea5
#define ECORE_MAGIC_POLLER          0xf7568127
#define ECORE_MAGIC_PIPE            0xf7458226


#define ECORE_MAGIC                 Ecore_Magic  __magic

#define ECORE_MAGIC_SET(d, m)       (d)->__magic = (m)
#define ECORE_MAGIC_CHECK(d, m)     ((d) && ((d)->__magic == (m)))
#define ECORE_MAGIC_FAIL(d, m, fn)  _ecore_magic_fail((d), (d) ? (d)->__magic : 0, (m), (fn));

/* undef the following, we want our version */
#undef FREE
#define FREE(ptr) free(ptr); ptr = NULL;

#undef IF_FREE
#define IF_FREE(ptr) if (ptr) free(ptr); ptr = NULL;

#undef IF_FN_DEL
#define IF_FN_DEL(_fn, ptr) if (ptr) { _fn(ptr); ptr = NULL; }

EAPI void ecore_print_warning(const char *function, const char *sparam);

/* convenience macros for checking pointer parameters for non-NULL */
#undef CHECK_PARAM_POINTER_RETURN
#define CHECK_PARAM_POINTER_RETURN(sparam, param, ret) \
     if (!(param)) \
	 { \
	    ecore_print_warning(__FUNCTION__, sparam); \
	    return ret; \
	 }

#undef CHECK_PARAM_POINTER
#define CHECK_PARAM_POINTER(sparam, param) \
     if (!(param)) \
	 { \
	    ecore_print_warning(__FUNCTION__, sparam); \
	    return; \
	 }

typedef unsigned int              Ecore_Magic;

typedef struct _Ecore_List2       Ecore_List2;
typedef struct _Ecore_List2_Data  Ecore_List2_Data;

struct _Ecore_List2
{
   Ecore_List2  *next, *prev;
   Ecore_List2  *last;
};

struct _Ecore_List2_Data
{
   Ecore_List2   __list_data;
   void *data;
};

#ifndef _ECORE_H
enum _Ecore_Fd_Handler_Flags
{
   ECORE_FD_READ = 1,
   ECORE_FD_WRITE = 2,
   ECORE_FD_ERROR = 4
};
typedef enum _Ecore_Fd_Handler_Flags Ecore_Fd_Handler_Flags;
enum _Ecore_Exe_Flags
{
   ECORE_EXE_PIPE_READ = 1,
   ECORE_EXE_PIPE_WRITE = 2,
   ECORE_EXE_PIPE_ERROR = 4,
   ECORE_EXE_PIPE_READ_LINE_BUFFERED = 8,
   ECORE_EXE_PIPE_ERROR_LINE_BUFFERED = 16,
   ECORE_EXE_PIPE_AUTO = 32,
   ECORE_EXE_RESPAWN = 64,
   ECORE_EXE_USE_SH = 128,
   ECORE_EXE_NOT_LEADER = 256
   /* FIXME: Getting respawn to work
    *
    * There is no way that we can do anything about the internal state info of
    * an external exe.  The same can be said about the state of user code.  User
    * code in this context means the code that is using ecore_exe to manage exe's
    * for it.
    *
    * Document that the exe must be respawnable, in other words, there is no
    * state that it cannot regenerate by just killing it and starting it again.
    * This includes state that the user code knows about, as the respawn is
    * transparent to that code.  On the other hand, maybe a respawn event might
    * be useful, or maybe resend the currently non existant add event.  For
    * consistancy with ecore_con, an add event is good anyway.
    *
    * The Ecore_exe structure is reused for respawning, so that the (opaque)
    * pointer held by the user remains valid.  This means that the Ecore_Exe
    * init and del functions may need to be split into two parts each to avoid
    * duplicating code - common code part, and the rest.  This implies that
    * the unchanging members mentioned next should NEVER change.
    *
    * These structure members don't need to change -
    *   __list_data       - we stay on the list
    *   ECORE_MAGIC       - this is a constant
    *   data              - passed in originally
    *   cmd               - passed in originally
    *   flags             - passed in originally
    *
    * These structure members need to change -
    *   tag               - state that must be regenerated, zap it
    *   pid               - it will be different
    *   child_fd_write    - it will be different
    *   child_fd_read     - it will be different
    *   child_fd_error    - it will be different
    *   write_fd_handler  - we cannot change the fd used by a handler, this changes coz the fd changes.
    *   read_fd_handler   - we cannot change the fd used by a handler, this changes coz the fd changes.
    *   error_fd_handler  - we cannot change the fd used by a handler, this changes coz the fd changes.
    *
    * Hmm, the read, write, and error buffers could be tricky.
    * They are not atomic, and could be in a semi complete state.
    * They fall into the "state must be regenerated" mentioned above.
    * A respawn/add event should take care of it.
    *
    * These structure members need to change -
    *   write_data_buf    - state that must be regenerated, zap it
    *   write_data_size   - state that must be regenerated, zap it
    *   write_data_offset - state that must be regenerated, zap it
    *   read_data_buf     - state that must be regenerated, zap it
    *   read_data_size    - state that must be regenerated, zap it
    *   error_data_buf    - state that must be regenerated, zap it
    *   error_data_size   - state that must be regenerated, zap it
    *   close_write       - state that must be regenerated, zap it
    *
    * There is the problem that an exe that fell over and needs respawning
    * might keep falling over, keep needing to be respawned, and tie up system
    * resources with the constant respawning.  An exponentially increasing
    * timeout (with maximum timeout) between respawns should take care of that.
    * Although this is not a "contention for a resource" problem, the exe falling
    * over may be, so a random element added to the timeout may help, and won't
    * hurt.  The user code may need to be informed that a timeout is in progress.
    */
};
typedef enum _Ecore_Exe_Flags Ecore_Exe_Flags;
enum _Ecore_Poller_Type
{
   ECORE_POLLER_CORE = 0
};
typedef enum _Ecore_Poller_Type Ecore_Poller_Type;

#ifndef _WIN32
typedef struct _Ecore_Exe           Ecore_Exe;
#endif
typedef struct _Ecore_Timer         Ecore_Timer;
typedef struct _Ecore_Idler         Ecore_Idler;
typedef struct _Ecore_Idle_Enterer  Ecore_Idle_Enterer;
typedef struct _Ecore_Idle_Exiter   Ecore_Idle_Exiter;
typedef struct _Ecore_Fd_Handler    Ecore_Fd_Handler;
typedef struct _Ecore_Event_Handler Ecore_Event_Handler;
typedef struct _Ecore_Event_Filter  Ecore_Event_Filter;
typedef struct _Ecore_Event         Ecore_Event;
typedef struct _Ecore_Animator      Ecore_Animator;
typedef struct _Ecore_Pipe          Ecore_Pipe;
typedef struct _Ecore_Poller        Ecore_Poller;

#ifndef _WIN32
struct _Ecore_Exe
{
   Ecore_List2   __list_data;
   ECORE_MAGIC;
   pid_t        pid;
   void        *data;
   char        *tag;
   char        *cmd;
   Ecore_Exe_Flags  flags;
   Ecore_Fd_Handler *write_fd_handler; /* the fd_handler to handle write to child - if this was used, or NULL if not */
   Ecore_Fd_Handler *read_fd_handler; /* the fd_handler to handle read from child - if this was used, or NULL if not */
   Ecore_Fd_Handler *error_fd_handler; /* the fd_handler to handle errors from child - if this was used, or NULL if not */
   void        *write_data_buf; /* a data buffer for data to write to the child -
                                 * realloced as needed for more data and flushed when the fd handler says writes are possible
				 */
   int          write_data_size; /* the size in bytes of the data buffer */
   int          write_data_offset; /* the offset in bytes in the data buffer */
   void        *read_data_buf; /* data read from the child awating delivery to an event */
   int          read_data_size; /* data read from child in bytes */
   void        *error_data_buf; /* errors read from the child awating delivery to an event */
   int          error_data_size; /* errors read from child in bytes */
   int          child_fd_write;	/* fd to write TO to send data to the child */
   int          child_fd_read;	/* fd to read FROM when child has sent us (the parent) data */
   int          child_fd_error;	/* fd to read FROM when child has sent us (the parent) errors */
   int          child_fd_write_x;	/* fd to write TO to send data to the child */
   int          child_fd_read_x;	/* fd to read FROM when child has sent us (the parent) data */
   int          child_fd_error_x;	/* fd to read FROM when child has sent us (the parent) errors */
   int          close_stdin;

   int start_bytes, end_bytes, start_lines, end_lines; /* Number of bytes/lines to auto pipe at start/end of stdout/stderr. */

   Ecore_Timer *doomsday_clock; /* The Timer of Death.  Muahahahaha. */
   void        *doomsday_clock_dead; /* data for the doomsday clock */
};
#endif

struct _Ecore_Timer
{
   Ecore_List2     __list_data;
   ECORE_MAGIC;
   double          in;
   double          at;
   double          pending;
   unsigned char   delete_me : 1;
   unsigned char   just_added : 1;
   unsigned char   frozen : 1;
   int           (*func) (void *data);
   void           *data;
};

struct _Ecore_Idler
{
   Ecore_List2   __list_data;
   ECORE_MAGIC;
   int          delete_me : 1;
   int        (*func) (void *data);
   void        *data;
};

struct _Ecore_Idle_Enterer
{
   Ecore_List2   __list_data;
   ECORE_MAGIC;
   int          delete_me : 1;
   int        (*func) (void *data);
   void        *data;
};

struct _Ecore_Idle_Exiter
{
   Ecore_List2   __list_data;
   ECORE_MAGIC;
   int          delete_me : 1;
   int        (*func) (void *data);
   void        *data;
};

struct _Ecore_Fd_Handler
{
   Ecore_List2               __list_data;
   ECORE_MAGIC;
   int                      fd;
   Ecore_Fd_Handler_Flags   flags;
   int                      read_active : 1;
   int                      write_active : 1;
   int                      error_active : 1;
   int                      delete_me : 1;
   int                    (*func) (void *data, Ecore_Fd_Handler *fd_handler);
   void                    *data;
   int                    (*buf_func) (void *data, Ecore_Fd_Handler *fd_handler);
   void                    *buf_data;
   void                   (*prep_func) (void *data, Ecore_Fd_Handler *fd_handler);
   void                    *prep_data;
};

struct _Ecore_Event_Handler
{
   Ecore_List2   __list_data;
   ECORE_MAGIC;
   int          type;
   int          delete_me : 1;
   int        (*func) (void *data, int type, void *event);
   void        *data;
};

struct _Ecore_Event_Filter
{
   Ecore_List2   __list_data;
   ECORE_MAGIC;
   int          delete_me : 1;
   void *     (*func_start) (void *data);
   int        (*func_filter) (void *data, void *loop_data, int type, void *event);
   void       (*func_end) (void *data, void *loop_data);
   void        *loop_data;
   void        *data;
};

struct _Ecore_Event
{
   Ecore_List2   __list_data;
   ECORE_MAGIC;
   int          type;
   void        *event;
   int          delete_me : 1;
   void       (*func_free) (void *data, void *ev);
   void        *data;
};

struct _Ecore_Animator
{
   Ecore_List2   __list_data;
   ECORE_MAGIC;
   signed char  delete_me : 1;
   int        (*func) (void *data);
   void        *data;
};

struct _Ecore_Poller
{
   Ecore_List2   __list_data;
   ECORE_MAGIC;
   int          ibit;
   signed char  delete_me : 1;
   int        (*func) (void *data);
   void        *data;
};

#endif

EAPI void          _ecore_magic_fail(const void *d, Ecore_Magic m, Ecore_Magic req_m, const char *fname);

void          _ecore_timer_shutdown(void);
void          _ecore_timer_cleanup(void);
void          _ecore_timer_enable_new(void);
double        _ecore_timer_next_get(void);
int           _ecore_timer_call(double when);

void          _ecore_idler_shutdown(void);
int           _ecore_idler_call(void);
int           _ecore_idler_exist(void);

void          _ecore_idle_enterer_shutdown(void);
void          _ecore_idle_enterer_call(void);
int           _ecore_idle_enterer_exist(void);

void          _ecore_idle_exiter_shutdown(void);
void          _ecore_idle_exiter_call(void);
int           _ecore_idle_exiter_exist(void);

void          _ecore_event_shutdown(void);
int           _ecore_event_exist(void);
Ecore_Event  *_ecore_event_add(int type, void *ev, void (*func_free) (void *data, void *ev), void *data);
void         *_ecore_event_del(Ecore_Event *event);
void          _ecore_event_call(void);

EAPI void         *_ecore_event_signal_user_new(void);
void         *_ecore_event_signal_hup_new(void);
void         *_ecore_event_signal_exit_new(void);
void         *_ecore_event_signal_power_new(void);
void         *_ecore_event_signal_realtime_new(void);

void          _ecore_main_shutdown(void);

#ifdef _WIN32
static inline void _ecore_signal_shutdown(void) { }
static inline void _ecore_signal_init(void) { }
static inline int  _ecore_signal_count_get(void) { return 0; }
static inline void _ecore_signal_call(void) { }
#else
void          _ecore_signal_shutdown(void);
void          _ecore_signal_init(void);
int           _ecore_signal_count_get(void);
void          _ecore_signal_call(void);
#endif

#ifdef _WIN32
static inline void _ecore_exe_init(void) { }
static inline void _ecore_exe_shutdown(void) { }
#else
void          _ecore_exe_init(void);
void          _ecore_exe_shutdown(void);
Ecore_Exe    *_ecore_exe_find(pid_t pid);
void         *_ecore_exe_event_del_new(void);
void          _ecore_exe_event_del_free(void *data, void *ev);
#endif

void          _ecore_animator_shutdown(void);

void          _ecore_poller_shutdown(void);

EAPI void         *_ecore_list2_append           (void *in_list, void *in_item);
EAPI void         *_ecore_list2_prepend          (void *in_list, void *in_item);
EAPI void         *_ecore_list2_append_relative  (void *in_list, void *in_item, void *in_relative);
EAPI void         *_ecore_list2_prepend_relative (void *in_list, void *in_item, void *in_relative);
EAPI void         *_ecore_list2_remove           (void *in_list, void *in_item);
EAPI void         *_ecore_list2_find             (void *in_list, void *in_item);

void          _ecore_fps_debug_init(void);
void          _ecore_fps_debug_shutdown(void);
void          _ecore_fps_debug_runtime_add(double t);



extern int    _ecore_fps_debug;
extern double _ecore_loop_time;

#endif
