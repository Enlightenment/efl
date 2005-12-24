#ifndef _ECORE_PRIVATE_H
#define _ECORE_PRIVATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>

#ifndef WIN32
#include <sys/mman.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE___ATTRIBUTE__
#define __UNUSED__ __attribute__((unused))
#else
#define __UNUSED__
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef MIN
#define MIN(x, y) (((x) > (y)) ? (y) : (x))
#endif

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#ifndef CLAMP
#define CLAMP(x, min, max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
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

#define ECORE_MAGIC                 Ecore_Magic  __magic

#define ECORE_MAGIC_SET(d, m)       (d)->__magic = (m)
#define ECORE_MAGIC_CHECK(d, m)     ((d) && ((d)->__magic == (m)))
#define ECORE_MAGIC_FAIL(d, m, fn)  _ecore_magic_fail((d), (d) ? (d)->__magic : 0, (m), (fn));

/* undef the following, we want out version */
#undef FREE
#define FREE(ptr) free(ptr); ptr = NULL;

#undef IF_FREE
#define IF_FREE(ptr) if (ptr) free(ptr); ptr = NULL;

#undef IF_FN_DEL
#define IF_FN_DEL(_fn, ptr) if (ptr) { _fn(ptr); ptr = NULL; }

inline void ecore_print_warning(const char *function, const char *sparam);

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
   ECORE_EXE_PIPE_READ_LINE_BUFFERED = 4,
   ECORE_EXE_RESPAWN = 8
};
typedef enum _Ecore_Exe_Flags Ecore_Exe_Flags;

#ifndef WIN32
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

#ifndef WIN32
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
   void        *write_data_buf; /* a data buffer for data to write to the child - 
                                 * realloced as needed for more data and flushed when the fd handler says writes are possible 
				 */
   int          write_data_size; /* the size in bytes of the data buffer */
   int          write_data_offset; /* the offset in bytes in the data buffer */
   void        *read_data_buf; /* data read from the child awating delivery to an event */
   int          read_data_size; /* data read from child in bytes */
   int          child_fd_write;	/* fd to write TO to send data to the child */
   int          child_fd_read;	/* fd to read FROM when child has sent us (the parent) data */
   /* I thought a bit about wether or not their could be multiple exit events, then realised that since we 
    * delete the exe on the first exit event, the answer is no.  On the other hand, STOPing an exe may trigger
    * this, even though it has not truly exited.  Probably should investigate this further.
    */
   struct _Ecore_Event_Exe_Exit  *exit_event; /* Process exit event */
};
#endif

struct _Ecore_Timer
{
   Ecore_List2   __list_data;
   ECORE_MAGIC;
   double       in;
   double       at;
   signed char  delete_me : 1;
   signed char  just_added : 1;
   int        (*func) (void *data);
   void        *data;
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

#endif

void          _ecore_magic_fail(void *d, Ecore_Magic m, Ecore_Magic req_m, const char *fname);

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

#ifndef WIN32
void         *_ecore_event_exe_exit_new(void);
void          _ecore_event_exe_exit_free(void *data, void *ev);
#endif
void         *_ecore_event_signal_user_new(void);
void         *_ecore_event_signal_hup_new(void);
void         *_ecore_event_signal_exit_new(void);
void         *_ecore_event_signal_power_new(void);
void         *_ecore_event_signal_realtime_new(void);

void          _ecore_main_shutdown(void);
    
void          _ecore_signal_shutdown(void);
void          _ecore_signal_init(void);
int           _ecore_signal_count_get(void);
void          _ecore_signal_call(void);

#ifndef WIN32
void          _ecore_exe_shutdown(void);
Ecore_Exe    *_ecore_exe_find(pid_t pid);
void         *_ecore_exe_free(Ecore_Exe *exe);
#endif

void          _ecore_animator_shutdown(void);
    

void         *_ecore_list2_append           (void *in_list, void *in_item);
void         *_ecore_list2_prepend          (void *in_list, void *in_item);
void         *_ecore_list2_append_relative  (void *in_list, void *in_item, void *in_relative);
void         *_ecore_list2_prepend_relative (void *in_list, void *in_item, void *in_relative);
void         *_ecore_list2_remove           (void *in_list, void *in_item);
void         *_ecore_list2_find             (void *in_list, void *in_item);

void          _ecore_fps_debug_init(void);
void          _ecore_fps_debug_shutdown(void);
void          _ecore_fps_debug_runtime_add(double t);



extern int    _ecore_fps_debug;

#endif
