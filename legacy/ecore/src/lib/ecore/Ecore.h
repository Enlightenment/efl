#ifndef _ECORE_H
#define _ECORE_H

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
   
#define ECORE_EVENT_NONE         0
#define ECORE_EVENT_EXE_EXIT     1 /**< Spawned Exe has exit event */
#define ECORE_EVENT_SIGNAL_USER  2 /**< User signal event */
#define ECORE_EVENT_SIGNAL_HUP   3 /**< Hup signal event */
#define ECORE_EVENT_SIGNAL_EXIT  4 /**< Exit signal event */
#define ECORE_EVENT_SIGNAL_POWER 5 /**< Power signal event */
#define ECORE_EVENT_COUNT        6
   
#ifndef _ECORE_PRIVATE_H   
   enum _Ecore_Fd_Handler_Flags
     {
	ECORE_FD_READ = 1, /**< Fd Read mask */
	ECORE_FD_WRITE = 2 /**< Fd Write mask */
     };
   typedef enum _Ecore_Fd_Handler_Flags Ecore_Fd_Handler_Flags;
   
   typedef void Ecore_Exe; /**< A handle for spawned processes */
   typedef void Ecore_Timer; /**< A handle for timers */
   typedef void Ecore_Idler; /**< A handle for idlers */
   typedef void Ecore_Idle_Enterer; /**< A handle for idle enterers */
   typedef void Ecore_Fd_Handler; /**< A handle for Fd hanlders */
   typedef void Ecore_Event_Handler; /**< A handle for an event handler */
   typedef void Ecore_Event_Filter; /**< A handle for an event filter */
   typedef void Ecore_Event; /**< A handle for an event */
#endif
   typedef struct _Ecore_Event_Exe_Exit     Ecore_Event_Exe_Exit; /**< Spawned Exe exit event */
   typedef struct _Ecore_Event_Signal_User  Ecore_Event_Signal_User; /**< User signal event */
   typedef struct _Ecore_Event_Signal_Hup   Ecore_Event_Signal_Hup; /**< Hup signal event */
   typedef struct _Ecore_Event_Signal_Exit  Ecore_Event_Signal_Exit; /**< Exit signal event */
   typedef struct _Ecore_Event_Signal_Power Ecore_Event_Signal_Power; /**< Power signal event */

   struct _Ecore_Event_Exe_Exit /** Process exit event */
     {
	pid_t      pid; /**< The process ID of the process that exited */
	int        exit_code; /**< The exit code of the process */
	Ecore_Exe *exe; /**< The handle to the exited process, or NULL if not found */
	int        exit_signal; /** < The signal that caused the process to exit */
	char       exited    : 1; /** < set to 1 if the process exited of its own accord */
	char       signalled : 1; /** < set to 1 id the process exited due to uncaught signal */
	void      *ext_data; /**< Extension data - not used */
     };

   struct _Ecore_Event_Signal_User /** User signal event */
     {
	int   number; /**< The signal number. Either 1 or 2 */
	void *ext_data; /**< Extension data - not used */
     };
   
   struct _Ecore_Event_Signal_Hup /** Hup signal event */
     {
	void *ext_data; /**< Extension data - not used */
     };
   
   struct _Ecore_Event_Signal_Exit /** Exit request event */
     {
	int   interrupt : 1; /**< Set if the exit request was an interrupt  signal*/
	int   quit      : 1; /**< set if the exit request was a quit signal */
	int   terminate : 1; /**< Set if the exit request was a terminate singal */
	void *ext_data;	/**< Extension data - not used */
     };

   struct _Ecore_Event_Signal_Power /** Power event */
     {
	void *ext_data; /**< Extension data - not used */
     };

   int  ecore_init(void);
   int  ecore_shutdown(void);
       
   void ecore_app_args_set(int argc, const char **argv);
   void ecore_app_args_get(int *argc, char ***argv);
   void ecore_app_restart(void);

   Ecore_Event_Handler *ecore_event_handler_add(int type, int (*func) (void *data, int type, void *event), const void *data);
   void                *ecore_event_handler_del(Ecore_Event_Handler *event_handler);
   Ecore_Event         *ecore_event_add(int type, void *ev, void (*func_free) (void *data, void *ev), void *data);
   void                *ecore_event_del(Ecore_Event *event);
   int                  ecore_event_type_new(void);
   Ecore_Event_Filter  *ecore_event_filter_add(void * (*func_start) (void *data), int (*func_filter) (void *data, void *loop_data, int type, void *event), void (*func_end) (void *data, void *loop_data), const void *data);
   void                *ecore_event_filter_del(Ecore_Event_Filter *ef);
   int                  ecore_event_current_type_get(void);
   void                *ecore_event_current_event_get(void);
       
       

   Ecore_Exe *ecore_exe_run(const char *exe_cmd, const void *data);
   void      *ecore_exe_free(Ecore_Exe *exe);
   pid_t      ecore_exe_pid_get(Ecore_Exe *exe);
   void      *ecore_exe_data_get(Ecore_Exe *exe);
   void       ecore_exe_pause(Ecore_Exe *exe);
   void       ecore_exe_continue(Ecore_Exe *exe);
   void       ecore_exe_terminate(Ecore_Exe *exe);
   void       ecore_exe_kill(Ecore_Exe *exe);
   void       ecore_exe_signal(Ecore_Exe *exe, int num);
   void       ecore_exe_hup(Ecore_Exe *exe);
       
   Ecore_Idler *ecore_idler_add(int (*func) (void *data), const void *data);
   void        *ecore_idler_del(Ecore_Idler *idler);
   
   Ecore_Idle_Enterer *ecore_idle_enterer_add(int (*func) (void *data), const void *data);
   void               *ecore_idle_enterer_del(Ecore_Idle_Enterer *idle_enterer);

   void              ecore_main_loop_iterate(void);
   void              ecore_main_loop_begin(void);
   void              ecore_main_loop_quit(void);
   Ecore_Fd_Handler *ecore_main_fd_handler_add(int fd, Ecore_Fd_Handler_Flags flags, int (*func) (void *data, Ecore_Fd_Handler *fd_handler), const void *data, int (*buf_func) (void *buf_data, Ecore_Fd_Handler *fd_handler), const void *buf_data);
   void             *ecore_main_fd_handler_del(Ecore_Fd_Handler *fd_handler);
   int               ecore_main_fd_handler_fd_get(Ecore_Fd_Handler *fd_handler);
   int               ecore_main_fd_handler_active_get(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags);
   void              ecore_main_fd_handler_active_set(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags);
   
   double ecore_time_get(void);
       
   Ecore_Timer *ecore_timer_add(double in, int (*func) (void *data), const void *data);
   void        *ecore_timer_del(Ecore_Timer *timer);
       
#ifdef __cplusplus
}
#endif
#endif
