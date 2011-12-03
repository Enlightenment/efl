#ifndef ECORE_TYPES_H
#define ECORE_TYPES_H

#ifdef WANT_ECORE_TIMER_DUMP
# include <string.h>
# include <execinfo.h>
# define ECORE_TIMER_DEBUG_BT_NUM 64
typedef void (*Ecore_Timer_Bt_Func)();
#endif

struct _Ecore_Animator
{
   EINA_INLIST;
                     ECORE_MAGIC;

   Ecore_Task_Cb     func;
   void             *data;

   double            start, run;
   Ecore_Timeline_Cb run_func;
   void             *run_data;

   Eina_Bool         delete_me : 1;
   Eina_Bool         suspended : 1;
};

struct _Ecore_Event_Handler
{
   EINA_INLIST;
                          ECORE_MAGIC;
   int                    type;
   Ecore_Event_Handler_Cb func;
   void                  *data;
   int                    references;
   Eina_Bool              delete_me : 1;
};

struct _Ecore_Event_Filter
{
   EINA_INLIST;
                   ECORE_MAGIC;
   Ecore_Data_Cb   func_start;
   Ecore_Filter_Cb func_filter;
   Ecore_End_Cb    func_end;
   void           *loop_data;
   void           *data;
   int             references;
   Eina_Bool       delete_me : 1;
};

struct _Ecore_Event
{
   EINA_INLIST;
                ECORE_MAGIC;
   int          type;
   void        *event;
   Ecore_End_Cb func_free;
   void        *data;
   int          references;
   Eina_Bool    delete_me : 1;
};

struct _Ecore_Idle_Enterer
{
   EINA_INLIST;
                 ECORE_MAGIC;
   Ecore_Task_Cb func;
   void         *data;
   int           references;
   Eina_Bool     delete_me : 1;
};

struct _Ecore_Idle_Exiter
{
   EINA_INLIST;
                 ECORE_MAGIC;
   Ecore_Task_Cb func;
   void         *data;
   int           references;
   Eina_Bool     delete_me : 1;
};

struct _Ecore_Idler
{
   EINA_INLIST;
                 ECORE_MAGIC;
   Ecore_Task_Cb func;
   void         *data;
   int           references;
   Eina_Bool     delete_me : 1;
};

struct _Ecore_Job
{
                ECORE_MAGIC;
   Ecore_Event *event;
   Ecore_Cb     func;
   void        *data;
};

struct _Ecore_Fd_Handler
{
   EINA_INLIST;
                          ECORE_MAGIC;
   Ecore_Fd_Handler      *next_ready;
   int                    fd;
   Ecore_Fd_Handler_Flags flags;
   Ecore_Fd_Cb            func;
   void                  *data;
   Ecore_Fd_Cb            buf_func;
   void                  *buf_data;
   Ecore_Fd_Prep_Cb       prep_func;
   void                  *prep_data;
   int                    references;
   Eina_Bool              read_active : 1;
   Eina_Bool              write_active : 1;
   Eina_Bool              error_active : 1;
   Eina_Bool              delete_me : 1;
#if defined(USE_G_MAIN_LOOP)
   GPollFD                gfd;
#endif
};

#ifdef _WIN32
struct _Ecore_Win32_Handler
{
   EINA_INLIST;
                         ECORE_MAGIC;
   HANDLE                h;
   Ecore_Win32_Handle_Cb func;
   void                 *data;
   int                   references;
   Eina_Bool             delete_me : 1;
};
#endif

struct _Ecore_Pipe
{
                     ECORE_MAGIC;
   int               fd_read;
   int               fd_write;
   Ecore_Fd_Handler *fd_handler;
   const void       *data;
   Ecore_Pipe_Cb     handler;
   unsigned int      len;
   int               handling;
   size_t            already_read;
   void             *passed_data;
   int               message;
   Eina_Bool         delete_me : 1;
};

struct _Ecore_Poller
{
   EINA_INLIST;
                 ECORE_MAGIC;
   int           ibit;
   unsigned char delete_me : 1;
   Ecore_Task_Cb func;
   void         *data;
};

struct _Ecore_Timer
{
   EINA_INLIST;
                       ECORE_MAGIC;
   double              in;
   double              at;
   double              pending;
   Ecore_Task_Cb       func;
   void               *data;

#ifdef WANT_ECORE_TIMER_DUMP
   Ecore_Timer_Bt_Func timer_bt[ECORE_TIMER_DEBUG_BT_NUM];
   int                 timer_bt_num;
#endif

   int                 references;
   unsigned char       delete_me : 1;
   unsigned char       just_added : 1;
   unsigned char       frozen : 1;
};

#endif
