/** 
@brief Ecore Library Public API Calls
 
These routines are used for Ecore Library interaction
*/

/**

@mainpage Ecore

@image html  e.png

@version @PACKAGE_VERSION@
@author Carsten Haitzler <raster@rasterman.com>
@author Tom Gilbert <tom@linuxbrit.co.uk>
@author Burra <burra@colorado.edu>
@author Chris Ross <chris@darkrock.co.uk>
@author Term <term@twistedpath.org>
@author Tilman Sauerbeck <tilman@code-monkey.de>
@author Ibukun Olumuyiwa <ibukun@computer.org>
@author Yuri <da2001@hotmail.ru>
@author Nicholas Curran <quasar@bigblue.net.au>
@author Howell Tam <pigeon@pigeond.net>
@author Nathan Ingersoll <rbdpngn@users.sourceforge.net>
@author Andrew Elcock <andy@elcock.org>
@author Kim Woelders <kim@woelders.dk>
@author Sebastian Dransfeld <sebastid@tango.flipp.net>
@author Simon Poole <simon.armlinux@themalago.net>
@author Jorge Luis Zapata Muga <jorgeluis.zapata@gmail.com>
@author dan sinclair <zero@everburning.com>
@author Michael 'Mickey' Lauer <mickey@tm.informatik.uni-frankfurt.de>
@author David 'onefang' Seikel <onefang@gmail.com>
@author Hisham 'CodeWarrior' Mardam Bey <hisham@hisham.cc>
@author Brian 'rephorm' Mattern <rephorm@rephorm.com>
@author Tim Horton <hortont424@gmail.com>
@author Arnaud de Turckheim 'quarium' <quarium@gmail.com>
@author Matt Barclay <mbarclay@gmail.com>
@author Peter Wehrfritz <peter.wehrfritz@web.de>
@author Albin "Lutin" Tonnerre <albin.tonnerre@gmail.com>
@author Vincent Torri <vincent.torri@gmail.com>
@author Lars Munch <lars@segv.dk>
@author Andre Dieb <andre.dieb@gmail.com>
@author Mathieu Taillefumier <mathieu.taillefumier@free.fr>
@author Rui Miguel Silva Seabra <rms@1407.org>
@author Samsung Electronics
@author Samsung SAIT
@author Nicolas Aguirre <aguirre.nicolas@gmail.com>
@author Brett Nash <nash@nash.id.au>
@author Mike Blumenkrantz <mike@zentific.com>
@author Leif Middelschulte <leif.middelschulte@gmail.com>
@author Mike McCormack <mj.mccormack@samsung.com>
@author Sangho Park <gouache95@gmail.com>
@author Jihoon Kim <jihoon48.kim@samsung.com> <imfine98@gmail.com>
@author Daniel Juyung Seo <seojuyung2@gmail.com> <juyung.seo@samsung.com>
@date 2000-2011

@section intro Introduction

Ecore is a library of convenience functions. A brief explanation of how to use
it can be found in @ref Ecore_Main_Loop_Page.

The Ecore library provides the following modules:
@li @ref Ecore_Group
@li @ref Ecore_File_Group
@li @ref Ecore_Con_Group
@li @link Ecore_Evas.h   Ecore_Evas - Evas convenience functions. @endlink
@li @ref Ecore_FB_Group
@li @link Ecore_Ipc.h    Ecore_IPC - Inter Process Communication functions. @endlink
@li @link Ecore_X.h      Ecore_X - X Windows System wrapper. @endlink
@li @ref Ecore_Win32_Group
@li @ref Ecore_WinCE_Group

For more info on Ecore usage, there are these @ref Examples.

@section compiling How to compile using Ecore?
pkgconfig (.pc) files are installed for every ecore module.
Thus, to compile using any of them, you can use something like the following:

@verbatim
gcc *.c $(pkg-config ecore ecore-$x ecore-$y [...] --cflags --libs)
@endverbatim

@section install How is it installed?

Suggested configure options for evas for a Linux desktop X display:

@verbatim
./configure \
--enable-ecore-x \
--enable-ecore-fb \
--enable-ecore-evas \
--enable-ecore-evas-gl \
--enable-ecore-con \
--enable-ecore-ipc
make CFLAGS="-O9 -mpentiumpro -march=pentiumpro -mcpu=pentiumpro"
@endverbatim

@todo (1.0) Document API
*/

/*
@page Ecore_Main_Loop_Page The Ecore Main Loop

@section intro What is Ecore?

Ecore is a clean and tiny event loop library with many modules to do lots of
convenient things for a programmer, to save time and effort.

It's small and lean, designed to work on embedded systems all the way to
large and powerful multi-cpu workstations. It serialises all system signals,
events etc. into a single event queue, that is easily processed without
needing to worry about concurrency. A properly written, event-driven program
using this kind of programming doesn't need threads, nor has to worry about
concurrency. It turns a program into a state machine, and makes it very
robust and easy to follow.

Ecore gives you other handy primitives, such as timers to tick over for you
and call specified functions at particular times so the programmer can use
this to do things, like animate, or time out on connections or tasks that take
too long etc.

Idle handlers are provided too, as well as calls on entering an idle state
(often a very good time to update the state of the program). All events that
enter the system are passed to specific callback functions that the program
sets up to handle those events. Handling them is simple and other Ecore
modules produce more events on the queue, coming from other sources such as
file descriptors etc.

Ecore also lets you have functions called when file descriptors become active
for reading or writing, allowing for streamlined, non-blocking IO.

Here is an example of a simple program and its basic event loop flow:

@image html  prog_flow.png



@section work How does Ecore work?

Ecore is very easy to learn and use. All the function calls are designed to
be easy to remember, explicit in describing what they do, and heavily
name-spaced. Ecore programs can start and be very simple.

For example:

@code
#include <Ecore.h>

int main(int argc, const char **argv)
{
  ecore_init();
  ecore_app_args_set(argc, argv);
  ecore_main_loop_begin();
  ecore_shutdown();
  return 0;
}
@endcode

This program is very simple and does't check for errors, but it does start up
and begin a main loop waiting for events or timers to tick off. This program
doesn't set up any, but now we can expand on this simple program a little
more by adding some event handlers and timers.

@code
#include <Ecore.h>

Ecore_Timer         *timer1     = NULL;
Ecore_Event_Handler *handler1   = NULL;
double               start_time = 0.0;

int timer_func(void *data)
{
  printf("Tick timer. Sec: %3.2f\n", ecore_time_get() - start_time);
  return 1;
}

int exit_func(void *data, int ev_type, void *ev)
{
  Ecore_Event_Signal_Exit *e;

  e = (Ecore_Event_Signal_Exit *)ev;
  if (e->interrupt)      printf("Exit: interrupt\n");
  else if (e->quit)      printf("Exit: quit\n");
  else if (e->terminate) printf("Exit: terminate\n");
  ecore_main_loop_quit();
  return 1;
}

int main(int argc, const char **argv)
{
  ecore_init();
  ecore_app_args_set(argc, argv);  
  start_time = ecore_time_get();
  handler1 = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_func, NULL);
  timer1 = ecore_timer_add(0.5, timer_func, NULL);  
  ecore_main_loop_begin();
  ecore_shutdown();
  return 0;
}
@endcode

In the previous example, we initialize our application and get the time at
which our program has started so we can calculate an offset. We set
up a timer to tick off in 0.5 seconds, and since it returns 1, will
keep ticking off every 0.5 seconds until it returns 0, or is deleted
by hand. An event handler is set up to call a function - exit_func(),
whenever an event of type ECORE_EVENT_SIGNAL_EXIT is received (CTRL-C
on the command line will cause such an event to happen). If this event
occurs it tells you what kind of exit signal was received, and asks
the main loop to quit when it is finished by calling
ecore_main_loop_quit().

The handles returned by ecore_timer_add() and ecore_event_handler_add() are 
only stored here as an example. If you don't need to address the timer or 
event handler again you don't need to store the result, so just call the 
function, and don't assign the result to any variable.

This program looks slightly more complex than needed to do these simple
things, but in principle, programs don't get any more complex. You add more
event handlers, for more events, will have more timers and such, BUT it all
follows the same principles as shown in this example.

*/

/*
@page Ecore_Config_Page The Enlightened Property Library

The Enlightened Property Library (Ecore_Config) is an adbstraction
from the complexities of writing your own configuration. It provides
many features using the Enlightenment 17 development libraries.

To use the library, you:
@li Set the default values of your properties.
@li Load the configuration from a file.  You must set the default values
    first, so that the library knows the correct type of each argument.

The following examples show how to use the Enlightened Property Library:
@li @link config_basic_example.c config_basic_example.c @endlink
@li @link config_listener_example.c config_listener_example.c @endlink

*/

/**
@page X_Window_System_Page X Window System

The Ecore library includes a wrapper for handling the X window system.
This page briefly explains what the X window system is and various terms
that are used.
*/

#ifndef _ECORE_H
#define _ECORE_H

#ifdef _MSC_VER
# include <Evil.h>
#endif

#include <Eina.h>

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

#ifdef _WIN32
# include <winsock2.h>
#elif (defined (__FreeBSD__) && (__FreeBSD_version >= 420001)) || defined (__OpenBSD__)
# include <sys/select.h>
# include <signal.h>
#else
# include <sys/time.h>
# include <signal.h>
#endif

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * @file Ecore.h
   * @defgroup Ecore_Group Ecore - Main Loop and Job Functions.
   * @brief The file that provides the program utility, main loop and timer
   *        functions.
   *
   * This header provides the Ecore event handling loop.  For more
   * details, see @ref Ecore_Main_Loop_Group.
   *
   * For the main loop to be of any use, you need to be able to add events
   * and event handlers.  Events for file descriptor events are covered in
   * @ref Ecore_FD_Handler_Group.
   *
   * Time functions are covered in @ref Ecore_Time_Group.
   *
   * There is also provision for callbacks for when the loop enters or
   * exits an idle state. See @ref Idle_Group for more information.
   *
   * Functions are also provided for spawning child processes using fork.
   * See @ref Ecore_Exe_Group for more details.
   *
   * @{
   */

#define ECORE_VERSION_MAJOR 1
#define ECORE_VERSION_MINOR 0

   typedef struct _Ecore_Version
     {
        int major;
        int minor;
        int micro;
        int revision;
     } Ecore_Version;
   
   EAPI extern Ecore_Version *ecore_version;

#define ECORE_CALLBACK_CANCEL EINA_FALSE /**< Return value to remove a callback */
#define ECORE_CALLBACK_RENEW EINA_TRUE  /**< Return value to keep a callback */

#define ECORE_CALLBACK_PASS_ON EINA_TRUE /**< Return value to pass event to next handler */
#define ECORE_CALLBACK_DONE EINA_FALSE /**< Return value to stop event handling */

#define ECORE_EVENT_NONE            0
#define ECORE_EVENT_SIGNAL_USER     1 /**< User signal event */
#define ECORE_EVENT_SIGNAL_HUP      2 /**< Hup signal event */
#define ECORE_EVENT_SIGNAL_EXIT     3 /**< Exit signal event */
#define ECORE_EVENT_SIGNAL_POWER    4 /**< Power signal event */
#define ECORE_EVENT_SIGNAL_REALTIME 5 /**< Realtime signal event */
#define ECORE_EVENT_COUNT           6

#define ECORE_EXE_PRIORITY_INHERIT 9999
   
   EAPI extern int ECORE_EXE_EVENT_ADD; /**< A child process has been added */
   EAPI extern int ECORE_EXE_EVENT_DEL; /**< A child process has been deleted (it exited, naming consistent with the rest of ecore). */
   EAPI extern int ECORE_EXE_EVENT_DATA; /**< Data from a child process. */
   EAPI extern int ECORE_EXE_EVENT_ERROR; /**< Errors from a child process. */

   enum _Ecore_Fd_Handler_Flags
     {
        ECORE_FD_READ = 1, /**< Fd Read mask */
        ECORE_FD_WRITE = 2, /**< Fd Write mask */
        ECORE_FD_ERROR = 4 /**< Fd Error mask */
     };
   typedef enum _Ecore_Fd_Handler_Flags Ecore_Fd_Handler_Flags;

   enum _Ecore_Exe_Flags /* flags for executing a child with its stdin and/or stdout piped back */
     {
        ECORE_EXE_NONE = 0, /**< No exe flags at all */
        ECORE_EXE_PIPE_READ = 1, /**< Exe Pipe Read mask */
        ECORE_EXE_PIPE_WRITE = 2, /**< Exe Pipe Write mask */
        ECORE_EXE_PIPE_ERROR = 4, /**< Exe Pipe error mask */
        ECORE_EXE_PIPE_READ_LINE_BUFFERED = 8, /**< Reads are buffered until a newline and delivered 1 event per line */
        ECORE_EXE_PIPE_ERROR_LINE_BUFFERED = 16, /**< Errors are buffered until a newline and delivered 1 event per line */
        ECORE_EXE_PIPE_AUTO = 32, /**< stdout and stderr are buffered automatically */
        ECORE_EXE_RESPAWN = 64, /**< FIXME: Exe is restarted if it dies */
        ECORE_EXE_USE_SH = 128, /**< Use /bin/sh to run the command. */
        ECORE_EXE_NOT_LEADER = 256 /**< Do not use setsid() to have the executed process be its own session leader */
     };
   typedef enum _Ecore_Exe_Flags Ecore_Exe_Flags;

   enum _Ecore_Exe_Win32_Priority
     {
       ECORE_EXE_WIN32_PRIORITY_IDLE, /**< Idle priority, for monitoring the system */
       ECORE_EXE_WIN32_PRIORITY_BELOW_NORMAL, /**< Below default priority */
       ECORE_EXE_WIN32_PRIORITY_NORMAL, /**< Default priority */
       ECORE_EXE_WIN32_PRIORITY_ABOVE_NORMAL, /**< Above default priority */
       ECORE_EXE_WIN32_PRIORITY_HIGH, /**< High priority, use with care as other threads in the system will not get processor time */
       ECORE_EXE_WIN32_PRIORITY_REALTIME /**< Realtime priority, should be almost never used as it can interrupt system threads that manage mouse input, keyboard input, and background disk flushing */
     };
   typedef enum _Ecore_Exe_Win32_Priority Ecore_Exe_Win32_Priority;

   enum _Ecore_Poller_Type /* Poller types */
     {
        ECORE_POLLER_CORE = 0 /**< The core poller interval */
     };
   typedef enum _Ecore_Poller_Type Ecore_Poller_Type;
   
   enum _Ecore_Pos_Map /* Position mappings */
     {
        ECORE_POS_MAP_LINEAR,     /**< Linear 0.0 -> 1.0 */
        ECORE_POS_MAP_ACCELERATE, /**< Start slow then speed up */
        ECORE_POS_MAP_DECELERATE, /**< Start fast then slow down */
        ECORE_POS_MAP_SINUSOIDAL, /**< Start slow, speed up then slow down at end */
        ECORE_POS_MAP_ACCELERATE_FACTOR, /**< Start slow then speed up, v1 being a power factor, 0.0 being linear, 1.0 being normal accelerate, 2.0 being much more pronounced accelerate (squared), 3.0 being cubed, etc. */
        ECORE_POS_MAP_DECELERATE_FACTOR, /**< Start fast then slow down, v1 being a power factor, 0.0 being linear, 1.0 being normal decelerate, 2.0 being much more pronounced decelerate (squared), 3.0 being cubed, etc. */
        ECORE_POS_MAP_SINUSOIDAL_FACTOR, /**< Start slow, speed up then slow down at end, v1 being a power factor, 0.0 being linear, 1.0 being normal sinusoidal, 2.0 being much more pronounced sinusoidal (squared), 3.0 being cubed, etc. */
        ECORE_POS_MAP_DIVISOR_INTERP, /**< Start at gradient * v1, interpolated via power of v2 curve */
        ECORE_POS_MAP_BOUNCE, /**< Start at 0.0 then "drop" like a ball bouncing to the ground at 1.0, and bounce v2 times, with decay factor of v1 */
        ECORE_POS_MAP_SPRING  /**< Start at 0.0 then "wobble" like a sping rest position 1.0, and wobble v2 times, with decay factor of v1 */
     };
   typedef enum _Ecore_Pos_Map Ecore_Pos_Map;

   enum _Ecore_Animator_Source /* Timing sources for animators */
     {
        ECORE_ANIMATOR_SOURCE_TIMER, /**< The default system clock/timer based animator that ticks every "frametime" seconds */
        ECORE_ANIMATOR_SOURCE_CUSTOM /**< A custom animator trigger that you need to call ecore_animator_trigger() to make it tick */
     };
   typedef enum _Ecore_Animator_Source Ecore_Animator_Source;
   
   typedef struct _Ecore_Exe                   Ecore_Exe; /**< A handle for spawned processes */
   typedef struct _Ecore_Timer                 Ecore_Timer; /**< A handle for timers */
   typedef struct _Ecore_Idler                 Ecore_Idler; /**< A handle for idlers */
   typedef struct _Ecore_Idle_Enterer          Ecore_Idle_Enterer; /**< A handle for idle enterers */
   typedef struct _Ecore_Idle_Exiter           Ecore_Idle_Exiter; /**< A handle for idle exiters */
   typedef struct _Ecore_Fd_Handler            Ecore_Fd_Handler; /**< A handle for Fd handlers */
   typedef struct _Ecore_Win32_Handler         Ecore_Win32_Handler; /**< A handle for HANDLE handlers on Windows */
   typedef struct _Ecore_Event_Handler         Ecore_Event_Handler; /**< A handle for an event handler */
   typedef struct _Ecore_Event_Filter          Ecore_Event_Filter; /**< A handle for an event filter */
   typedef struct _Ecore_Event                 Ecore_Event; /**< A handle for an event */
   typedef struct _Ecore_Animator              Ecore_Animator; /**< A handle for animators */
   typedef struct _Ecore_Pipe                  Ecore_Pipe; /**< A handle for pipes */
   typedef struct _Ecore_Poller                Ecore_Poller; /**< A handle for pollers */
   typedef struct _Ecore_Event_Signal_User     Ecore_Event_Signal_User; /**< User signal event */
   typedef struct _Ecore_Event_Signal_Hup      Ecore_Event_Signal_Hup; /**< Hup signal event */
   typedef struct _Ecore_Event_Signal_Exit     Ecore_Event_Signal_Exit; /**< Exit signal event */
   typedef struct _Ecore_Event_Signal_Power    Ecore_Event_Signal_Power; /**< Power signal event */
   typedef struct _Ecore_Event_Signal_Realtime Ecore_Event_Signal_Realtime; /**< Realtime signal event */
   typedef struct _Ecore_Exe_Event_Add         Ecore_Exe_Event_Add; /**< Spawned Exe add event */
   typedef struct _Ecore_Exe_Event_Del         Ecore_Exe_Event_Del; /**< Spawned Exe exit event */
   typedef struct _Ecore_Exe_Event_Data_Line   Ecore_Exe_Event_Data_Line; /**< Lines from a child process */
   typedef struct _Ecore_Exe_Event_Data        Ecore_Exe_Event_Data; /**< Data from a child process */
   typedef struct _Ecore_Thread                Ecore_Thread;

   /**
    * @typedef Ecore_Data_Cb Ecore_Data_Cb
    * A callback which is used to return data to the main function
    */
   typedef void *(*Ecore_Data_Cb) (void *data);
   /**
    * @typedef Ecore_Filter_Cb
    * A callback used for filtering events from the main loop.
    */
   typedef Eina_Bool (*Ecore_Filter_Cb) (void *data, void *loop_data, int type, void *event);
   /**
    * @typedef Ecore_Eselect_Function Ecore_Eselect_Function
    * A function which can be used to replace select() in the main loop
    */
   typedef int (*Ecore_Select_Function)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
   /**
    * @typedef Ecore_End_Cb Ecore_End_Cb
    * This is the callback which is called at the end of a function, usually for cleanup purposes.
    */
   typedef void (*Ecore_End_Cb) (void *user_data, void *func_data);
   /**
    * @typedef Ecore_Pipe_Cb Ecore_Pipe_Cb
    * The callback that data written to the pipe is sent to.
    */
   typedef void (*Ecore_Pipe_Cb) (void *data, void *buffer, unsigned int nbyte);
   /**
    * @typedef Ecore_Exe_Cb Ecore_Exe_Cb
    * A callback to run with the associated @ref Ecore_Exe, usually for cleanup purposes.
    */
   typedef void (*Ecore_Exe_Cb)(void *data, const Ecore_Exe *exe);
   /**
    * @typedef Ecore_Event_Handler_Cb Ecore_Event_Handler_Cb
    * A callback used by the main loop to handle events of a specified type.
    */
   typedef Eina_Bool (*Ecore_Event_Handler_Cb) (void *data, int type, void *event);
   /**
    * @typedef Ecore_Thread_Cb Ecore_Thread_Cb
    * A callback used by Ecore_Thread helper.
    */
  typedef void (*Ecore_Thread_Cb) (void *data, Ecore_Thread *thread);
   /**
    * @typedef Ecore_Thread_Notify_Cb Ecore_Thread_Notify_Cb
    * A callback used by the main loop to receive data sent by an @ref Ecore_Thread_Group.
    */
  typedef void (*Ecore_Thread_Notify_Cb) (void *data, Ecore_Thread *thread, void *msg_data);
   /**
    * @typedef Ecore_Task_Cb Ecore_Task_Cb
    * A callback run for a task (timer, idler, poller, animator, etc)
    */
   typedef Eina_Bool (*Ecore_Task_Cb) (void *data);
   /**
    * @typedef Ecore_Timeline_Cb Ecore_Timeline_Cb
    * A callback run for a task (animators with runtimes)
    */
   typedef Eina_Bool (*Ecore_Timeline_Cb) (void *data, double pos);
   /**
    * @typedef Ecore_Cb Ecore_Cb
    * A generic callback called as a hook when a certain point in execution is reached.
    */
   typedef void (*Ecore_Cb) (void *data);
   /**
    * @typedef Ecore_Fd_Cb Ecore_Fd_Cb
    * A callback used by an @ref Ecore_Fd_Handler.
    */
   typedef Eina_Bool (*Ecore_Fd_Cb) (void *data, Ecore_Fd_Handler *fd_handler);
   /**
    * @typedef Ecore_Fd_Prep_Cb Ecore_Fd_Prep_Cb
    * A callback used by an @ref Ecore_Fd_Handler.
    */
   typedef void (*Ecore_Fd_Prep_Cb) (void *data, Ecore_Fd_Handler *fd_handler);
   /**
    * @typedef Ecore_Win32_Handle_Cb Ecore_Win32_Handle_Cb
    * A callback used by an @ref Ecore_Win32_Handler.
    */
   typedef Eina_Bool (*Ecore_Win32_Handle_Cb) (void *data, Ecore_Win32_Handler *wh);


   typedef struct _Ecore_Job Ecore_Job; /**< A job handle */

   struct _Ecore_Event_Signal_User /** User signal event */
     {
        int   number; /**< The signal number. Either 1 or 2 */
        void *ext_data; /**< Extension data - not used */

#ifndef _WIN32
        siginfo_t data; /**< Signal info */
#endif
     };

   struct _Ecore_Event_Signal_Hup /** Hup signal event */
     {
        void *ext_data; /**< Extension data - not used */

#ifndef _WIN32
        siginfo_t data; /**< Signal info */
#endif
     };

   struct _Ecore_Event_Signal_Exit /** Exit request event */
     {
        Eina_Bool   interrupt : 1; /**< Set if the exit request was an interrupt  signal*/
        Eina_Bool   quit      : 1; /**< set if the exit request was a quit signal */
        Eina_Bool   terminate : 1; /**< Set if the exit request was a terminate singal */
        void          *ext_data; /**< Extension data - not used */

#ifndef _WIN32
        siginfo_t data; /**< Signal info */
#endif
     };

   struct _Ecore_Event_Signal_Power /** Power event */
     {
        void *ext_data; /**< Extension data - not used */

#ifndef _WIN32
        siginfo_t data; /**< Signal info */
#endif
     };

   struct _Ecore_Event_Signal_Realtime /** Realtime event */
     {
        int num; /**< The realtime signal's number */

#ifndef _WIN32
        siginfo_t data; /**< Signal info */
#endif
     };

   struct _Ecore_Exe_Event_Add /** Process add event */
     {
        Ecore_Exe *exe; /**< The handle to the added process */
        void      *ext_data; /**< Extension data - not used */
     };

   struct _Ecore_Exe_Event_Del /** Process exit event */
     {
        pid_t         pid; /**< The process ID of the process that exited */
        int           exit_code; /**< The exit code of the process */
        Ecore_Exe    *exe; /**< The handle to the exited process, or NULL if not found */
        int           exit_signal; /** < The signal that caused the process to exit */
        Eina_Bool  exited    : 1; /** < set to 1 if the process exited of its own accord */
        Eina_Bool  signalled : 1; /** < set to 1 id the process exited due to uncaught signal */
        void         *ext_data; /**< Extension data - not used */
#ifndef _WIN32
        siginfo_t     data; /**< Signal info */
#endif
     };

   struct _Ecore_Exe_Event_Data_Line /**< Lines from a child process */
      {
         char *line;
         int   size;
      };

   struct _Ecore_Exe_Event_Data /** Data from a child process event */
     {
        Ecore_Exe *exe; /**< The handle to the process */
        void *data; /**< the raw binary data from the child process that was received */
        int   size; /**< the size of this data in bytes */
        Ecore_Exe_Event_Data_Line *lines; /**< an array of line data if line buffered, the last one has it's line member set to NULL */
     };

  /**
   * @defgroup Ecore_Init_Group Ecore initialisation and shutdown functions.
   */

   EAPI int  ecore_init(void);
   EAPI int  ecore_shutdown(void);

  /**
   * @}
   */

  /**
   * @defgroup Ecore_Application_Group Ecore Application functions
   *
   * @{
   */

   EAPI void ecore_app_args_set(int argc, const char **argv);
   EAPI void ecore_app_args_get(int *argc, char ***argv);
   EAPI void ecore_app_restart(void);

  /**
   * @}
   */

  /**
   * @defgroup Ecore_Event_Group Ecore Event functions
   *
   * @{
   */

   EAPI Ecore_Event_Handler *ecore_event_handler_add(int type, Ecore_Event_Handler_Cb func, const void *data);
   EAPI void                *ecore_event_handler_del(Ecore_Event_Handler *event_handler);
   EAPI Ecore_Event         *ecore_event_add(int type, void *ev, Ecore_End_Cb func_free, void *data);
   EAPI void                *ecore_event_del(Ecore_Event *event);
   EAPI void                *ecore_event_handler_data_get(Ecore_Event_Handler *eh);
   EAPI void                *ecore_event_handler_data_set(Ecore_Event_Handler *eh, void *data);
   EAPI int                  ecore_event_type_new(void);
   EAPI Ecore_Event_Filter  *ecore_event_filter_add(Ecore_Data_Cb func_start, Ecore_Filter_Cb func_filter, Ecore_End_Cb func_end, const void *data);
   EAPI void                *ecore_event_filter_del(Ecore_Event_Filter *ef);
   EAPI int                  ecore_event_current_type_get(void);
   EAPI void                *ecore_event_current_event_get(void);

  /**
   * @}
   */

  /**
   * @defgroup Ecore_Exe_Group Process Spawning Functions
   *
   * @{
   */

   EAPI void        ecore_exe_run_priority_set(int pri);
   EAPI int         ecore_exe_run_priority_get(void);
   EAPI Ecore_Exe  *ecore_exe_run(const char *exe_cmd, const void *data);
   EAPI Ecore_Exe  *ecore_exe_pipe_run(const char *exe_cmd, Ecore_Exe_Flags flags, const void *data);
   EAPI void        ecore_exe_callback_pre_free_set(Ecore_Exe *exe, Ecore_Exe_Cb func);
   EAPI Eina_Bool   ecore_exe_send(Ecore_Exe *exe, const void *data, int size);
   EAPI void        ecore_exe_close_stdin(Ecore_Exe *exe);
   EAPI void        ecore_exe_auto_limits_set(Ecore_Exe *exe, int start_bytes, int end_bytes, int start_lines, int end_lines);
   EAPI Ecore_Exe_Event_Data *ecore_exe_event_data_get(Ecore_Exe *exe, Ecore_Exe_Flags flags);
   EAPI void        ecore_exe_event_data_free(Ecore_Exe_Event_Data *data);
   EAPI void       *ecore_exe_free(Ecore_Exe *exe);
   EAPI pid_t       ecore_exe_pid_get(const Ecore_Exe *exe);
   EAPI void        ecore_exe_tag_set(Ecore_Exe *exe, const char *tag);
   EAPI const char *ecore_exe_tag_get(const Ecore_Exe *exe);
   EAPI const char *ecore_exe_cmd_get(const Ecore_Exe *exe);
   EAPI void       *ecore_exe_data_get(const Ecore_Exe *exe);
   EAPI void       *ecore_exe_data_set(Ecore_Exe *exe, void *data);
   EAPI Ecore_Exe_Flags ecore_exe_flags_get(const Ecore_Exe *exe);
   EAPI void        ecore_exe_pause(Ecore_Exe *exe);
   EAPI void        ecore_exe_continue(Ecore_Exe *exe);
   EAPI void        ecore_exe_interrupt(Ecore_Exe *exe);
   EAPI void        ecore_exe_quit(Ecore_Exe *exe);
   EAPI void        ecore_exe_terminate(Ecore_Exe *exe);
   EAPI void        ecore_exe_kill(Ecore_Exe *exe);
   EAPI void        ecore_exe_signal(Ecore_Exe *exe, int num);
   EAPI void        ecore_exe_hup(Ecore_Exe *exe);

  /**
   * @}
   */

  /**
   * @defgroup Ecore_Idle_Group Ecore Idle functions
   *
   * @{
   */

   EAPI Ecore_Idler *ecore_idler_add(Ecore_Task_Cb func, const void *data);
   EAPI void        *ecore_idler_del(Ecore_Idler *idler);

   EAPI Ecore_Idle_Enterer *ecore_idle_enterer_add(Ecore_Task_Cb func, const void *data);
   EAPI Ecore_Idle_Enterer *ecore_idle_enterer_before_add(Ecore_Task_Cb func, const void *data);
   EAPI void               *ecore_idle_enterer_del(Ecore_Idle_Enterer *idle_enterer);

   EAPI Ecore_Idle_Exiter *ecore_idle_exiter_add(Ecore_Task_Cb func, const void *data);
   EAPI void              *ecore_idle_exiter_del(Ecore_Idle_Exiter *idle_exiter);

  /**
   * @}
   */

  /**
   * @defgroup Ecore_Main Loop_Group Ecore Main Loop functions
   *
   * @{
   */

   EAPI void              ecore_main_loop_iterate(void);

   EAPI void              ecore_main_loop_select_func_set(Ecore_Select_Function func);
   EAPI Ecore_Select_Function ecore_main_loop_select_func_get(void);

   EAPI Eina_Bool         ecore_main_loop_glib_integrate(void);
   EAPI void              ecore_main_loop_glib_always_integrate_disable(void);
       
   EAPI void              ecore_main_loop_begin(void);
   EAPI void              ecore_main_loop_quit(void);
   EAPI Ecore_Fd_Handler *ecore_main_fd_handler_add(int fd, Ecore_Fd_Handler_Flags flags, Ecore_Fd_Cb func, const void *data,
                                                    Ecore_Fd_Cb buf_func, const void *buf_data);
   EAPI void              ecore_main_fd_handler_prepare_callback_set(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Prep_Cb func, const void *data);
   EAPI void             *ecore_main_fd_handler_del(Ecore_Fd_Handler *fd_handler);
   EAPI int               ecore_main_fd_handler_fd_get(Ecore_Fd_Handler *fd_handler);
   EAPI Eina_Bool         ecore_main_fd_handler_active_get(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags);
   EAPI void              ecore_main_fd_handler_active_set(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags);

   EAPI Ecore_Win32_Handler *ecore_main_win32_handler_add(void *h, Ecore_Win32_Handle_Cb func, const void *data);
   EAPI void                *ecore_main_win32_handler_del(Ecore_Win32_Handler *win32_handler);

  /**
   * @}
   */

  /**
   * @defgroup Ecore_Pipe_Group Pipe wrapper
   *
   * @{
   */

   EAPI Ecore_Pipe  *ecore_pipe_add(Ecore_Pipe_Cb handler, const void *data);
   EAPI void        *ecore_pipe_del(Ecore_Pipe *p);
   EAPI Eina_Bool    ecore_pipe_write(Ecore_Pipe *p, const void *buffer, unsigned int nbytes);
   EAPI void         ecore_pipe_write_close(Ecore_Pipe *p);
   EAPI void         ecore_pipe_read_close(Ecore_Pipe *p);
   EAPI void         ecore_pipe_thaw(Ecore_Pipe *p);
   EAPI void         ecore_pipe_freeze(Ecore_Pipe *p);
   EAPI int          ecore_pipe_wait(Ecore_Pipe *p, int message_count, double wait);

  /**
   * @}
   */

  /**
   * @defgroup Ecore_Thread_Group Ecore Thread functions
   *
   * @{
   */

   EAPI Ecore_Thread *ecore_thread_run(Ecore_Thread_Cb func_blocking,
                                       Ecore_Thread_Cb func_end,
                                       Ecore_Thread_Cb func_cancel,
                                       const void *data);
   EAPI Ecore_Thread *ecore_thread_feedback_run(Ecore_Thread_Cb func_heavy,
                                                Ecore_Thread_Notify_Cb func_notify,
                                                Ecore_Thread_Cb func_end,
                                                Ecore_Thread_Cb func_cancel,
                                                const void *data,
                                                Eina_Bool try_no_queue);
   EAPI Eina_Bool     ecore_thread_cancel(Ecore_Thread *thread);
   EAPI Eina_Bool     ecore_thread_check(Ecore_Thread *thread);
   EAPI Eina_Bool     ecore_thread_feedback(Ecore_Thread *thread, const void *msg_data);
   EAPI Eina_Bool     ecore_thread_reschedule(Ecore_Thread *thread);
   EAPI int           ecore_thread_active_get(void);
   EAPI int           ecore_thread_pending_get(void);
   EAPI int           ecore_thread_pending_feedback_get(void);
   EAPI int           ecore_thread_pending_total_get(void);
   EAPI int           ecore_thread_max_get(void);
   EAPI void          ecore_thread_max_set(int num);
   EAPI void          ecore_thread_max_reset(void);
   EAPI int           ecore_thread_available_get(void);
   EAPI Eina_Bool     ecore_thread_local_data_add(Ecore_Thread *thread, const char *key, void *value, Eina_Free_Cb cb, Eina_Bool direct);
   EAPI void         *ecore_thread_local_data_set(Ecore_Thread *thread, const char *key, void *value, Eina_Free_Cb cb);
   EAPI void         *ecore_thread_local_data_find(Ecore_Thread *thread, const char *key);
   EAPI Eina_Bool     ecore_thread_local_data_del(Ecore_Thread *thread, const char *key);

   EAPI Eina_Bool     ecore_thread_global_data_add(const char *key, void *value, Eina_Free_Cb cb, Eina_Bool direct);
   EAPI void         *ecore_thread_global_data_set(const char *key, void *value, Eina_Free_Cb cb);
   EAPI void         *ecore_thread_global_data_find(const char *key);
   EAPI Eina_Bool     ecore_thread_global_data_del(const char *key);
   EAPI void         *ecore_thread_global_data_wait(const char *key, double seconds);

  /**
   * @}
   */

  /**
   * @defgroup Ecore_Time_Group Ecore Time functions
   *
   * @{
   */

   EAPI double ecore_time_get(void);
   EAPI double ecore_time_unix_get(void);
   EAPI double ecore_loop_time_get(void);

   EAPI Ecore_Timer *ecore_timer_add(double in, Ecore_Task_Cb func, const void *data);
   EAPI Ecore_Timer *ecore_timer_loop_add(double in, Ecore_Task_Cb func, const void *data);
   EAPI void        *ecore_timer_del(Ecore_Timer *timer);
   EAPI void         ecore_timer_interval_set(Ecore_Timer *timer, double in);
   EAPI double       ecore_timer_interval_get(Ecore_Timer *timer);
   EAPI void         ecore_timer_freeze(Ecore_Timer *timer);
   EAPI void         ecore_timer_thaw(Ecore_Timer *timer);
   EAPI void         ecore_timer_delay(Ecore_Timer *timer, double add);
   EAPI double       ecore_timer_pending_get(Ecore_Timer *timer);
   EAPI double       ecore_timer_precision_get(void);
   EAPI void         ecore_timer_precision_set(double precision);
   EAPI char        *ecore_timer_dump(void);

  /**
   * @}
   */

  /**
   * @defgroup Ecore_Animator_Group Ecore Animator functions
   *
   * @{
   */

   EAPI Ecore_Animator *ecore_animator_add(Ecore_Task_Cb func, const void *data);
   EAPI Ecore_Animator *ecore_animator_timeline_add(double runtime, Ecore_Timeline_Cb func, const void *data);
   EAPI void           *ecore_animator_del(Ecore_Animator *animator);
   EAPI void            ecore_animator_freeze(Ecore_Animator *animator);
   EAPI void            ecore_animator_thaw(Ecore_Animator *animator);
   EAPI void            ecore_animator_frametime_set(double frametime);
   EAPI double          ecore_animator_frametime_get(void);
   EAPI double          ecore_animator_pos_map(double pos, Ecore_Pos_Map map, double v1, double v2);
   EAPI void            ecore_animator_source_set(Ecore_Animator_Source source);
   EAPI Ecore_Animator_Source ecore_animator_source_get(void);
   EAPI void            ecore_animator_custom_source_tick_begin_callback_set(Ecore_Cb func, const void *data);
   EAPI void            ecore_animator_custom_source_tick_end_callback_set(Ecore_Cb func, const void *data);
   EAPI void            ecore_animator_custom_tick(void);
         
  /**
   * @}
   */

  /**
   * @defgroup Ecore_Poller_Group Ecore Poll functions
   *
   * @{
   */

   EAPI void          ecore_poller_poll_interval_set(Ecore_Poller_Type type, double poll_time);
   EAPI double        ecore_poller_poll_interval_get(Ecore_Poller_Type type);
   EAPI Eina_Bool     ecore_poller_poller_interval_set(Ecore_Poller *poller, int interval);
   EAPI int           ecore_poller_poller_interval_get(Ecore_Poller *poller);
   EAPI Ecore_Poller *ecore_poller_add(Ecore_Poller_Type type, int interval, Ecore_Task_Cb func, const void *data);
   EAPI void         *ecore_poller_del(Ecore_Poller *poller);

  /**
   * @}
   */

  /**
   * @defgroup Ecore_Job_Group Ecore Job functions
   *
   * @{
   */

   EAPI Ecore_Job *ecore_job_add(Ecore_Cb func, const void *data);
   EAPI void      *ecore_job_del(Ecore_Job *job);

  /**
   * @}
   */

  /**
   * @}
   */

#ifdef __cplusplus
}
#endif
#endif
