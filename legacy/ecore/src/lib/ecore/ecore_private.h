#ifndef _ECORE_PRIVATE_H
#define _ECORE_PRIVATE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define ECORE_MAGIC_NONE            0x1234fedc
#define ECORE_MAGIC_EXE             0xf7e812f5
#define ECORE_MAGIC_TIMER           0xf7d713f4
#define ECORE_MAGIC_IDLER           0xf7c614f3
#define ECORE_MAGIC_IDLE_ENTERER    0xf7b515f2
#define ECORE_MAGIC_FD_HANDLER      0xf7a416f1
#define ECORE_MAGIC_EVENT_HANDLER   0xf79317f0
#define ECORE_MAGIC_EVENT_FILTER    0xf78218ff
#define ECORE_MAGIC_EVENT           0xf77119fe

#define ECORE_MAGIC                 Ecore_Magic  __magic

#define ECORE_MAGIC_SET(d, m)       (d)->__magic = (m)
#define ECORE_MAGIC_CHECK(d, m)     ((d) && ((d)->__magic == (m)))
#define ECORE_MAGIC_FAIL(d, m, fn)  _ecore_magic_fail((d), (d) ? (d)->__magic : 0, (m), (fn));

typedef int                         Ecore_Magic;

typedef struct _Ecore_List          Ecore_List;

struct _Ecore_List
{
   Ecore_List  *next, *prev;
   Ecore_List  *last;
};

#ifndef _ECORE_H
enum _Ecore_Fd_Handler_Flags
{
   ECORE_FD_READ = 1,
   ECORE_FD_WRITE = 2,
   ECORE_FD_ERROR = 4
};
typedef enum _Ecore_Fd_Handler_Flags Ecore_Fd_Handler_Flags;

typedef struct _Ecore_Exe           Ecore_Exe;
typedef struct _Ecore_Timer         Ecore_Timer;
typedef struct _Ecore_Idler         Ecore_Idler;
typedef struct _Ecore_Idle_Enterer  Ecore_Idle_Enterer;
typedef struct _Ecore_Fd_Handler    Ecore_Fd_Handler;
typedef struct _Ecore_Event_Handler Ecore_Event_Handler;
typedef struct _Ecore_Event_Filter  Ecore_Event_Filter;
typedef struct _Ecore_Event         Ecore_Event;

struct _Ecore_Exe
{
   Ecore_List   __list_data;
   ECORE_MAGIC;
   pid_t        pid;
   void        *data;
};

struct _Ecore_Timer
{
   Ecore_List   __list_data;
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
   Ecore_List   __list_data;
   ECORE_MAGIC;
   int          delete_me : 1;
   int        (*func) (void *data);   
   void        *data;
};

struct _Ecore_Idle_Enterer
{
   Ecore_List   __list_data;
   ECORE_MAGIC;
   int          delete_me : 1;
   int        (*func) (void *data);   
   void        *data;
};

struct _Ecore_Fd_Handler
{
   Ecore_List               __list_data;
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
};

struct _Ecore_Event_Handler
{
   Ecore_List   __list_data;
   ECORE_MAGIC;
   int          type;
   int          delete_me : 1;
   int        (*func) (void *data, int type, void *event);
   void        *data;
};

struct _Ecore_Event_Filter
{
   Ecore_List   __list_data;
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
   Ecore_List   __list_data;
   ECORE_MAGIC;
   int          type;
   void        *event;
   int          delete_me : 1;
   void       (*func_free) (void *data, void *ev);
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

void          _ecore_event_shutdown(void);
int           _ecore_event_exist(void);    
Ecore_Event  *_ecore_event_add(int type, void *ev, void (*func_free) (void *data, void *ev), void *data);
void         *_ecore_event_del(Ecore_Event *event);
void          _ecore_event_call(void);

void         *_ecore_event_exe_exit_new(void);
void          _ecore_event_exe_exit_free(void *data, void *ev);
void         *_ecore_event_signal_user_new(void);
void          _ecore_event_signal_user_free(void *data, void *ev);
void         *_ecore_event_signal_hup_new(void);
void          _ecore_event_signal_hup_free(void *data, void *ev);
void         *_ecore_event_signal_exit_new(void);
void          _ecore_event_signal_exit_free(void *data, void *ev);
void         *_ecore_event_signal_power_new(void);
void          _ecore_event_signal_power_free(void *data, void *ev);

void          _ecore_main_shutdown(void);
    
void          _ecore_signal_shutdown(void);
void          _ecore_signal_init(void);
int           _ecore_signal_count_get(void);
void          _ecore_signal_call(void);

void          _ecore_exe_shutdown(void);
Ecore_Exe    *_ecore_exe_find(pid_t pid);
void         *_ecore_exe_free(Ecore_Exe *exe);

void         *_ecore_list_append           (void *in_list, void *in_item);
void         *_ecore_list_prepend          (void *in_list, void *in_item);
void         *_ecore_list_append_relative  (void *in_list, void *in_item, void *in_relative);
void         *_ecore_list_prepend_relative (void *in_list, void *in_item, void *in_relative);
void         *_ecore_list_remove           (void *in_list, void *in_item);
void         *_ecore_list_find             (void *in_list, void *in_item);

#endif
