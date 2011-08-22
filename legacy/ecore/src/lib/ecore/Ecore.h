/** 
@brief Ecore Library Public API Calls
 
These routines are used for Ecore Library interaction
*/

/**

@mainpage Ecore

@version 1.1
@date 2000-2011

Please see the @ref authors page for contact details.

@section intro Introduction

Ecore is a library of convenience functions. A brief explanation of how to use
it can be found in @ref Ecore_Main_Loop_Page.

The Ecore library provides the following modules:
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
make
su -
...
make install
@endverbatim

*/

/**
@page authors Authors
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

Please contact <enlightenment-devel@lists.sourceforge.net> to get in
contact with the developers and maintainers.
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
@image latex prog_flow.eps width=\textwidth



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

This program is very simple and doesn't check for errors, but it does start up
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
   * @defgroup Ecore_Init_Group Ecore initialization and shutdown functions.
   *
   * @{
   */

   EAPI int  ecore_init(void);
   EAPI int  ecore_shutdown(void);

  /**
   * @}
   */

  /**
   *
   * @defgroup Ecore_Main_Loop_Group Ecore main loop functions
   *
   * These are functions acting on Ecore's main loop itself or on
   * events and infrastructure directly linked to it. This loop is
   * designed to work on embedded systems all the way to large and
   * powerful multi-cpu workstations.
   *
   * It serialises all system signals and events into a single event
   * queue, that can be easily processed without needing to worry
   * about concurrency. A properly written, event-driven program
   * using this kind of programming does not need threads. It makes
   * the program very robust and easy to follow.
   *
   * For example, for the main loop to be of any use, you need to be
   * able to add @b events and event handlers on it. Events for file
   * descriptor events are covered in @ref Ecore_FD_Handler_Group.
   *
   * Timer functions are covered in @ref Ecore_Time_Group.
   *
   * There is also provision for callbacks for when the loop enters or
   * exits an @b idle state. See @ref Ecore_Idle_Group for more
   * information on it.
   *
   * Functions are also provided for spawning child processes using
   * @c fork(). See @ref Ecore_Exe_Group for more details on it.
   *
   * Here is an example of simple program and its basic event loop
   * flow:
   *
   * @image html prog_flow.png
   * @image latex prog_flow.eps width=\textwidth
   *
   * For examples of setting up and using a main loop, see
   * @ref Ecore_Main_Loop_Page.
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

   /**
    * @typedef Ecore_Task_Cb Ecore_Task_Cb
    * A callback run for a task (timer, idler, poller, animator, etc)
    */
   typedef Eina_Bool (*Ecore_Task_Cb) (void *data);

   /**
    * @typedef Ecore_Eselect_Function Ecore_Eselect_Function
    * A function which can be used to replace select() in the main loop
    */
   typedef int (*Ecore_Select_Function)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

   EAPI void              ecore_main_loop_iterate(void);

   EAPI void              ecore_main_loop_select_func_set(Ecore_Select_Function func);
   EAPI Ecore_Select_Function ecore_main_loop_select_func_get(void);

   EAPI Eina_Bool         ecore_main_loop_glib_integrate(void);
   EAPI void              ecore_main_loop_glib_always_integrate_disable(void);

   EAPI void              ecore_main_loop_begin(void);
   EAPI void              ecore_main_loop_quit(void);

   /**
    * @typedef Ecore_Cb Ecore_Cb
    * A generic callback called as a hook when a certain point in
    * execution is reached.
    */
   typedef void (*Ecore_Cb) (void *data);

   /**
    * @typedef Ecore_Data_Cb Ecore_Data_Cb
    * A callback which is used to return data to the main function
    */
   typedef void *(*Ecore_Data_Cb) (void *data);

  /**
   * @brief Call callback asynchronously in the main loop.
   * @since 1.1.0
   *
   * @param callback The callback to call in the main loop
   * @param data The data to give to that call back
   *
   * For all call that need to happen in the main loop (most EFL functions do),
   * this helper function provide the infrastructure needed to do it safely
   * by avoind dead lock, race condition and properly wake up the main loop.
   *
   * Remember after that function call, you should never touch again the @p data
   * in the thread, it is owned by the main loop and you callback should take
   * care of freeing it if necessary.
   */
   EAPI void                 ecore_main_loop_thread_safe_call_async(Ecore_Cb callback, void *data);

  /**
   * @brief Call callback synchronously in the main loop.
   * @since 1.1.0
   *
   * @param callback The callback to call in the main loop
   * @param data The data to give to that call back
   * @return the value returned by the callback in the main loop
   *
   * For all call that need to happen in the main loop (most EFL functions do),
   * this helper function provide the infrastructure needed to do it safely
   * by avoind dead lock, race condition and properly wake up the main loop.
   *
   * Remember this function will block until the callback is executed in the
   * main loop. It can take time and you have no guaranty about the timeline.
   */
  EAPI void                 *ecore_main_loop_thread_safe_call_sync(Ecore_Data_Cb callback, void *data);

  /**
   * @brief This function suspend the main loop in a know state
   * @since 1.1.0
   *
   * @result the number of time ecore_thread_main_loop_begin() has been called
   * in this thread, if the main loop was suspended correctly. If not, it return @c -1.
   *
   * This function suspend the main loop in a know state, this let you
   * use any EFL call you want after it return. Be carefull, the main loop
   * is blocked until you call ecore_thread_main_loop_end(). This is
   * the only sane way to achieve pseudo thread safety.
   *
   * Notice that until the main loop is blocked, the thread is blocked
   * and their is noway around that.
   *
   * We still advise you, when possible, to use ecore_main_loop_thread_safe_call_async()
   * as it will not block the thread nor the main loop.
   */
  EAPI int ecore_thread_main_loop_begin(void);

  /**
   * @brief Unlock the main loop.
   * @since 1.1.0
   *
   * @result the number of time ecore_thread_main_loop_end() need to be called before
   * the main loop is unlocked again. @c -1 will be returned if you are trying to unlock
   * when there wasn't enough call to ecore_thread_main_loop_begin().
   *
   * After a call to ecore_thread_main_loop_begin(), you need to absolutly
   * call ecore_thread_main_loop_end(), or you application will stay frozen.
   */
  EAPI int ecore_thread_main_loop_end(void);

   /**
    * @}
    */

   /**
    * @defgroup Ecore_Event_Group Ecore Event functions
    *
    * Ecore events are used to wake up the Ecore main loop to warn
    * about state changes, tasks completed, data available for reading
    * or writing, etc. They are the base of the event oriented
    * programming.
    *
    * The idea is to write many functions (callbacks) that will be
    * registered to specific events, and called when these events
    * happen. This way, when the system state changes (a mouse click is
    * detected, a key is pressed, or the content of a file changes, for
    * example), the respective callbacks will be called with some
    * information about that event. Usually the function/callback will
    * have a data pointer to the event info (the position in the screen
    * where the mouse was clicked, the name of the key that was
    * pressed, or the name of the file that has changed).
    *
    * The basic usage, when one needs to watch for an existing event,
    * is to register a callback to it using ecore_event_add(). Of
    * course it's necessary to know beforehand what are the types of
    * events that the system/library will emmit.  This should be
    * available with the documentation from that system/library.
    *
    * When writing a library or group of functions that need to inform
    * about something, and you already are running on top of a main
    * loop, it is usually a good approach to use events. This way you
    * allow others to register as many callbacks as necessary to this
    * event, and don't have to care about who is registering to it. The
    * functions ecore_event_type_new() and ecore_event_add() are
    * available for this purpose.
    *
    * Example that deals with events:
    *
    * @li @ref ecore_event_example_c
    *
    * @ingroup Ecore_Main_Loop_Group
    *
    * @{
    */

#define ECORE_EVENT_NONE            0
#define ECORE_EVENT_SIGNAL_USER     1 /**< User signal event */
#define ECORE_EVENT_SIGNAL_HUP      2 /**< Hup signal event */
#define ECORE_EVENT_SIGNAL_EXIT     3 /**< Exit signal event */
#define ECORE_EVENT_SIGNAL_POWER    4 /**< Power signal event */
#define ECORE_EVENT_SIGNAL_REALTIME 5 /**< Realtime signal event */
#define ECORE_EVENT_COUNT           6

   typedef struct _Ecore_Win32_Handler         Ecore_Win32_Handler; /**< A handle for HANDLE handlers on Windows */
   typedef struct _Ecore_Event_Handler         Ecore_Event_Handler; /**< A handle for an event handler */
   typedef struct _Ecore_Event_Filter          Ecore_Event_Filter; /**< A handle for an event filter */
   typedef struct _Ecore_Event                 Ecore_Event; /**< A handle for an event */
   typedef struct _Ecore_Event_Signal_User     Ecore_Event_Signal_User; /**< User signal event */
   typedef struct _Ecore_Event_Signal_Hup      Ecore_Event_Signal_Hup; /**< Hup signal event */
   typedef struct _Ecore_Event_Signal_Exit     Ecore_Event_Signal_Exit; /**< Exit signal event */
   typedef struct _Ecore_Event_Signal_Power    Ecore_Event_Signal_Power; /**< Power signal event */
   typedef struct _Ecore_Event_Signal_Realtime Ecore_Event_Signal_Realtime; /**< Realtime signal event */

   /**
    * @typedef Ecore_Filter_Cb
    * A callback used for filtering events from the main loop.
    */
   typedef Eina_Bool (*Ecore_Filter_Cb) (void *data, void *loop_data, int type, void *event);

   /**
    * @typedef Ecore_End_Cb Ecore_End_Cb
    * This is the callback which is called at the end of a function,
    * usually for cleanup purposes.
    */
   typedef void (*Ecore_End_Cb) (void *user_data, void *func_data);

   /**
    * @typedef Ecore_Event_Handler_Cb Ecore_Event_Handler_Cb
    * A callback used by the main loop to handle events of a specified
    * type.
    */
   typedef Eina_Bool (*Ecore_Event_Handler_Cb) (void *data, int type, void *event);

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

   EAPI Ecore_Event_Handler *ecore_event_handler_add(int type, Ecore_Event_Handler_Cb func, const void *data);
   EAPI void                *ecore_event_handler_del(Ecore_Event_Handler *event_handler);
   EAPI Ecore_Event         *ecore_event_add(int type, void *ev, Ecore_End_Cb func_free, void *data);
   EAPI void                *ecore_event_del(Ecore_Event *event);
   EAPI void                *ecore_event_handler_data_get(Ecore_Event_Handler *eh);
   EAPI void                *ecore_event_handler_data_set(Ecore_Event_Handler *eh, const void *data);
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
     * Functions that deal with and send signals to spawned processes.
     *
     * @ingroup Ecore_Main_Loop_Group
     *
     * @{
     */

 #define ECORE_EXE_PRIORITY_INHERIT 9999

    EAPI extern int ECORE_EXE_EVENT_ADD; /**< A child process has been added */
    EAPI extern int ECORE_EXE_EVENT_DEL; /**< A child process has been deleted (it exited, naming consistent with the rest of ecore). */
   EAPI extern int ECORE_EXE_EVENT_DATA; /**< Data from a child process. */
   EAPI extern int ECORE_EXE_EVENT_ERROR; /**< Errors from a child process. */

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

   typedef struct _Ecore_Exe                   Ecore_Exe; /**< A handle for spawned processes */

   /**
    * @typedef Ecore_Exe_Cb Ecore_Exe_Cb
    * A callback to run with the associated @ref Ecore_Exe, usually
    * for cleanup purposes.
    */
   typedef void (*Ecore_Exe_Cb)(void *data, const Ecore_Exe *exe);

   typedef struct _Ecore_Exe_Event_Add         Ecore_Exe_Event_Add; /**< Spawned Exe add event */
   typedef struct _Ecore_Exe_Event_Del         Ecore_Exe_Event_Del; /**< Spawned Exe exit event */
   typedef struct _Ecore_Exe_Event_Data_Line   Ecore_Exe_Event_Data_Line; /**< Lines from a child process */
   typedef struct _Ecore_Exe_Event_Data        Ecore_Exe_Event_Data; /**< Data from a child process */

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
    * @defgroup Ecore_FD_Handler_Group File Event Handling Functions
    *
    * Functions that deal with file descriptor handlers.
    *
    * The @ref Ecore_Fd_Handler can be used to watch a file descriptor
    * for data available for reading, for the availability to write
    * without blocking, and for errors on the file descriptor.
    *
    * ecore_main_fd_handler_add() is used to setup a handler for a
    * given file descriptor. This file descriptor can be the standard
    * input, a network socket, a stream received through some driver
    * of a hardware decoder, etc. Thus it can contain errors, like a
    * disconnection, a broken pipe, and so, and that's why it's
    * possible to check for these errors with the @ref ECORE_FD_ERROR
    * flag.
    *
    * An @ref Ecore_Fd_Handler can be used to watch on a file
    * descriptor without blocking, still being able to receive events,
    * expire timers, and other watch for other things that happen in
    * the Ecore main loop.
    *
    * Example of use of a file descriptor handler:
    * @li @ref ecore_fd_handler_example_c
    *
    * @ingroup Ecore_Main_Loop_Group
    *
    * @{
    */

   typedef struct _Ecore_Fd_Handler            Ecore_Fd_Handler; /**< A handle for Fd handlers */

   enum _Ecore_Fd_Handler_Flags
     {
        ECORE_FD_READ = 1, /**< Fd Read mask */
        ECORE_FD_WRITE = 2, /**< Fd Write mask */
        ECORE_FD_ERROR = 4 /**< Fd Error mask */
     };
   typedef enum _Ecore_Fd_Handler_Flags Ecore_Fd_Handler_Flags;

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
   * @defgroup Ecore_Poller_Group Ecore Poll functions
   *
   * These functions are for the need to poll information, but provide
   * a shared abstracted API to pool such polling to minimise wakeup
   * and ensure all the polling happens in as few spots as possible
   * areound a core poll interval.  For now only 1 core poller type is
   * supprted: ECORE_POLLER_CORE
   *
   * Example of @ref Ecore_Poller:
   * @li @ref ecore_poller_example_c
   *
   * @ingroup Ecore_Main_Loop_Group
   *
   * @{
   */

   enum _Ecore_Poller_Type /* Poller types */
     {
        ECORE_POLLER_CORE = 0 /**< The core poller interval */
     };
   typedef enum _Ecore_Poller_Type Ecore_Poller_Type;

   typedef struct _Ecore_Poller                Ecore_Poller; /**< A handle for pollers */

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
   * @defgroup Ecore_Animator_Group Ecore Animator functions
   *
   * @brief Ecore animators are a helper to simplify creating
   * animations.
   *
   * Creating an animation is as simple as saying for how long it
   * should be run and having a callback that does the animation,
   * something like this:
   * @code
   * static Eina_Bool
   * _do_animation(void *data, double pos)
   * {
   *    evas_object_move(data, 100 * pos, 100 * pos);
   *    ... do some more animating ...
   * }
   * ...
   * ecore_animator_timeline_add(2, _do_animation, my_evas_object);
   * @endcode
   * In the sample above we create an animation to move
   * @c my_evas_object from position (0,0) to (100,100) in 2 seconds.
   *
   * If your animation will run for an unspecified amount of time you
   * can use ecore_animator_add(), which is like using
   * ecore_timer_add() with the interval being the
   * @ref ecore_animator_frametime_set "framerate". Note that this has
   * tangible benefits to creating a timer for each animation in terms
   * of performance.
   *
   * For a more detailed example that show several animation see
   * @ref tutorial_ecore_animator.
   *
   * @ingroup Ecore_Main_Loop_Group
   *
   * @{
   */

   typedef struct _Ecore_Animator              Ecore_Animator; /**< A handle for animators */

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
        ECORE_POS_MAP_SPRING  /**< Start at 0.0 then "wobble" like a spring rest position 1.0, and wobble v2 times, with decay factor of v1 */
     };
   typedef enum _Ecore_Pos_Map Ecore_Pos_Map;

   enum _Ecore_Animator_Source /* Timing sources for animators */
     {
        ECORE_ANIMATOR_SOURCE_TIMER, /**< The default system clock/timer based animator that ticks every "frametime" seconds */
        ECORE_ANIMATOR_SOURCE_CUSTOM /**< A custom animator trigger that you need to call ecore_animator_trigger() to make it tick */
     };
   typedef enum _Ecore_Animator_Source Ecore_Animator_Source;

   /**
    * @typedef Ecore_Timeline_Cb Ecore_Timeline_Cb
    * A callback run for a task (animators with runtimes)
    */
   typedef Eina_Bool (*Ecore_Timeline_Cb) (void *data, double pos);

  /**
   * @brief Add an animator to call @p func at every animaton tick during main
   * loop execution.
   *
   * @param func The function to call when it ticks off
   * @param data The data to pass to the function
   * @return A handle to the new animator
   *
   * This function adds a animator and returns its handle on success and NULL on
   * failure. The function @p func will be called every N seconds where N is the
   * @p frametime interval set by ecore_animator_frametime_set(). The function
   * will be passed the @p data pointer as its parameter.
   *
   * When the animator @p func is called, it must return a value of either 1 or
   * 0. If it returns 1 (or ECORE_CALLBACK_RENEW), it will be called again at
   * the next tick, or if it returns 0 (or ECORE_CALLBACK_CANCEL) it will be
   * deleted automatically making any references/handles for it invalid.
   *
   * @note The default @p frametime value is 1/30th of a second.
   *
   * @see ecore_animator_timeline_add()
   * @see ecore_animator_frametime_set()
   */
   EAPI Ecore_Animator *ecore_animator_add(Ecore_Task_Cb func, const void *data);
  /**
   * @brief Add a animator that runs for a limited time
   *
   * @param runtime The time to run in seconds
   * @param func The function to call when it ticks off
   * @param data The data to pass to the function
   * @return A handle to the new animator
   *
   * This function is just like ecore_animator_add() except the animator only
   * runs for a limited time specified in seconds by @p runtime. Once the
   * runtime the animator has elapsed (animator finished) it will automatically
   * be deleted. The callback function @p func can return ECORE_CALLBACK_RENEW
   * to keep the animator running or ECORE_CALLBACK_CANCEL ro stop it and have
   * it be deleted automatically at any time.
   *
   * The @p func will ALSO be passed a position parameter that will be in value
   * from 0.0 to 1.0 to indicate where along the timeline (0.0 start, 1.0 end)
   * the animator run is at. If the callback wishes not to have a linear
   * transition it can "map" this value to one of several curves and mappings
   * via ecore_animator_pos_map().
   *
   * @note The default @p frametime value is 1/30th of a second.
   *
   * @see ecore_animator_add()
   * @see ecore_animator_pos_map()
   * @since 1.1.0
   */
   EAPI Ecore_Animator *ecore_animator_timeline_add(double runtime, Ecore_Timeline_Cb func, const void *data);
  /**
   * @brief Delete the specified animator from the animator list.
   *
   * @param animator The animator to delete
   * @return The data pointer set for the animator on add
   *
   * Delete the specified @p animator from the set of animators that are
   * executed during main loop execution. This function returns the data
   * parameter that was being passed to the callback on success, or NULL on
   * failure. After this call returns the specified animator object @p animator
   * is invalid and should not be used again. It will not get called again after
   * deletion.
   */
   EAPI void           *ecore_animator_del(Ecore_Animator *animator);
  /**
   * @brief Suspend the specified animator.
   *
   * @param animator The animator to delete
   *
   * The specified @p animator will be temporarly removed from the set of
   * animators that are executed during main loop.
   *
   * @warning Freezing an animator doesn't freeze accounting of how long that
   * animator has been running. Therefore if the animator was created with
   * ecore_animator_timeline_add() the @p pos argument given to the callback
   * will increase as if the animator hadn't been frozen and the animator may
   * have it's execution halted if @p runtime elapsed.
   */
   EAPI void            ecore_animator_freeze(Ecore_Animator *animator);
  /**
   * @brief Restore execution of the specified animator.
   *
   * @param animator The animator to delete
   *
   * The specified @p animator will be put back in the set of animators that are
   * executed during main loop.
   */
   EAPI void            ecore_animator_thaw(Ecore_Animator *animator);
  /**
   * @brief Set the animator call interval in seconds.
   *
   * @param frametime The time in seconds in between animator ticks.
   *
   * This function sets the time interval (in seconds) between animator ticks.
   * At every tick the callback of every existing animator will be called.
   *
   * @warning Too small a value may cause performance issues and too high a
   * value may cause your animation to seem "jerky".
   *
   * @note The default @p frametime value is 1/30th of a second.
   */
   EAPI void            ecore_animator_frametime_set(double frametime);
  /**
   * @brief Get the animator call interval in seconds.
   *
   * @return The time in second in between animator ticks.
   *
   * This function retrieves the time in seconds between animator ticks.
   *
   * @see ecore_animator_frametime_set()
   */
   EAPI double          ecore_animator_frametime_get(void);
  /**
   * @brief Maps an input position from 0.0 to 1.0 along a timeline to a
   * position in a different curve.
   *
   * @param pos The input position to map
   * @param map The mapping to use
   * @param v1 A parameter use by the mapping (pass 0.0 if not used)
   * @param v2 A parameter use by the mapping (pass 0.0 if not used)
   * @return The mapped value
   *
   * Takes an input position (0.0 to 1.0) and maps to a new position (normally
   * between 0.0 and 1.0, but it may go above/below 0.0 or 1.0 to show that it
   * has "overshot" the mark) using some interpolation (mapping) algorithm.
   *
   * This function useful to create non-linear animations. It offers a variety
   * of possible animaton curves to be used:
   * @li ECORE_POS_MAP_LINEAR - Linear, returns @p pos
   * @li ECORE_POS_MAP_ACCELERATE - Start slow then speed up
   * @li ECORE_POS_MAP_DECELERATE - Start fast then slow down
   * @li ECORE_POS_MAP_SINUSOIDAL - Start slow, speed up then slow down at end
   * @li ECORE_POS_MAP_ACCELERATE_FACTOR - Start slow then speed up, v1 being a
   * power factor, 0.0 being linear, 1.0 being ECORE_POS_MAP_ACCELERATE, 2.0
   * being much more pronounced accelerate (squared), 3.0 being cubed, etc.
   * @li ECORE_POS_MAP_DECELERATE_FACTOR - Start fast then slow down, v1 being a
   * power factor, 0.0 being linear, 1.0 being ECORE_POS_MAP_DECELERATE, 2.0
   * being much more pronounced decelerate (squared), 3.0 being cubed, etc.
   * @li ECORE_POS_MAP_SINUSOIDAL_FACTOR - Start slow, speed up then slow down
   * at end, v1 being a power factor, 0.0 being linear, 1.0 being
   * ECORE_POS_MAP_SINUSOIDAL, 2.0 being much more pronounced sinusoidal
   * (squared), 3.0 being cubed, etc.
   * @li ECORE_POS_MAP_DIVISOR_INTERP - Start at gradient * v1, interpolated via
   * power of v2 curve
   * @li ECORE_POS_MAP_BOUNCE - Start at 0.0 then "drop" like a ball bouncing to
   * the ground at 1.0, and bounce v2 times, with decay factor of v1
   * @li ECORE_POS_MAP_SPRING - Start at 0.0 then "wobble" like a spring rest
   * position 1.0, and wobble v2 times, with decay factor of v1
   * @note When not listed v1 and v2 have no effect.
   *
   * @image html ecore-pos-map.png
   * @image latex ecore-pos-map.eps width=\textwidth
   *
   * One way to use this would be:
   * @code
   * double pos; // input position in a timeline from 0.0 to 1.0
   * double out; // output position after mapping
   * int x1, y1, x2, y2; // x1 & y1 are start position, x2 & y2 are end position
   * int x, y; // x & y are the calculated position
   *
   * out = ecore_animator_pos_map(pos, ECORE_POS_MAP_BOUNCE, 1.8, 7);
   * x = (x1 * out) + (x2 * (1.0 - out));
   * y = (y1 * out) + (y2 * (1.0 - out));
   * move_my_object_to(myobject, x, y);
   * @endcode
   * This will make an animaton that bounces 7 each times diminishing by a
   * factor of 1.8.
   *
   * @see _Ecore_Pos_Map
   *
   * @since 1.1.0
   */
   EAPI double          ecore_animator_pos_map(double pos, Ecore_Pos_Map map, double v1, double v2);
  /**
   * @brief Set the source of animator ticks for the mainloop
   *
   * @param source The source of animator ticks to use
   *
   * This sets the source of animator ticks. When an animator is active the
   * mainloop will "tick" over frame by frame calling all animators that are
   * registered until none are. The mainloop will tick at a given rate based
   * on the animator source. The default source is the system clock timer
   * source - ECORE_ANIMATOR_SOURCE_TIMER. This source uses the system clock
   * to tick over every N seconds (specified by ecore_animator_frametime_set(),
   * with the default being 1/30th of a second unless set otherwise). You can
   * set a custom tick source by setting the source to
   * ECORE_ANIMATOR_SOURCE_CUSTOM and then drive it yourself based on some input
   * tick source (like another application via ipc, some vertical blanking
   * interrupt interrupt etc.) using
   * ecore_animator_custom_source_tick_begin_callback_set() and
   * ecore_animator_custom_source_tick_end_callback_set() to set the functions
   * that will be called to start and stop the ticking source, which when it
   * gets a "tick" should call ecore_animator_custom_tick() to make the "tick" over 1
   * frame.
   */
   EAPI void            ecore_animator_source_set(Ecore_Animator_Source source);
  /**
   * @brief Get the animator source currently set.
   *
   * @return The current animator source
   *
   * This gets the current animator source.
   *
   * @see ecore_animator_source_set()
   */
   EAPI Ecore_Animator_Source ecore_animator_source_get(void);
  /**
   * @brief Set the function that begins a custom animator tick source
   *
   * @param func The function to call when ticking is to begin
   * @param data The data passed to the tick begin function as its parameter
   *
   * The Ecore Animator infrastructure handles tracking if animators are needed
   * or not and which ones need to be called and when, but when the tick source
   * is custom, you have to provide a tick source by calling
   * ecore_animator_custom_tick() to indicate a frame tick happened. In order
   * to allow the source of ticks to be dynamically enabled or disabled as
   * needed, the @p func when set is called to enable the tick source to
   * produce tick events that call ecore_animator_custom_tick(). If @p func
   * is NULL then no function is called to begin custom ticking.
   *
   * @see ecore_animator_source_set()
   * @see ecore_animator_custom_source_tick_end_callback_set()
   * @see ecore_animator_custom_tick()
   */
   EAPI void            ecore_animator_custom_source_tick_begin_callback_set(Ecore_Cb func, const void *data);
  /**
   * @brief Set the function that ends a custom animator tick source
   *
   * @param func The function to call when ticking is to end
   * @param data The data passed to the tick end function as its parameter
   *
   * This function is a matching pair to the function set by
   * ecore_animator_custom_source_tick_begin_callback_set() and is called
   * when ticking is to stop. If @p func is NULL then no function will be
   * called to stop ticking. For more information please see
   * ecore_animator_custom_source_tick_begin_callback_set().
   *
   * @see ecore_animator_source_set()
   * @see ecore_animator_custom_source_tick_begin_callback_set()
   * @see ecore_animator_custom_tick()
   */
   EAPI void            ecore_animator_custom_source_tick_end_callback_set(Ecore_Cb func, const void *data);
  /**
   * @brief Trigger a custom animator tick
   *
   * When animator source is set to ECORE_ANIMATOR_SOURCE_CUSTOM, then calling
   * this function triggers a run of all animators currently registered with
   * Ecore as this indicates a "frame tick" happened. This will do nothing if
   * the animator source(set by ecore_animator_source_set()) is not set to
   * ECORE_ANIMATOR_SOURCE_CUSTOM.
   *
   * @see ecore_animator_source_set()
   * @see ecore_animator_custom_source_tick_begin_callback_set
   * @see ecore_animator_custom_source_tick_end_callback_set()()
   */
   EAPI void            ecore_animator_custom_tick(void);

  /**
   * @}
   */

   /**
    * @defgroup Ecore_Time_Group Ecore Time functions
    *
    * Functions that deal with time. These functions include those
    * that simply retrieve it in a given format, and those that create
    * events based on it.
    *
    * The timer allows callbacks to be called at specific intervals.
    *
    * Examples with functions that deal with time:
    * @li @ref ecore_time_functions_example_c
    * @li @ref ecore_timer_example_c
    *
    * @ingroup Ecore_Main_Loop_Group
    *
    * @{
    */

   typedef struct _Ecore_Timer                 Ecore_Timer; /**< A handle for timers */

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
    * @defgroup Ecore_Idle_Group Ecore Idle functions
    *
    * Callbacks that are called when the program enters or exits an
    * idle state.
    *
    * The ecore main loop enters an idle state when it is waiting for
    * timers to time out, data to come in on a file descriptor or any
    * other event to occur.  You can set callbacks to be called when
    * the main loop enters an idle state, during an idle state or just
    * after the program wakes up.
    *
    * Enterer callbacks are good for updating your program's state, if
    * it has a state engine.  Once all of the enterer handlers are
    * called, the program will enter a "sleeping" state.
    *
    * Idler callbacks are called when the main loop has called all
    * enterer handlers.  They are useful for interfaces that require
    * polling and timers would be too slow to use.
    *
    * If no idler callbacks are specified, then the process literally
    * goes to sleep.  Otherwise, the idler callbacks are called
    * continuously while the loop is "idle", using as much CPU as is
    * available to the process.
    *
    * Exiter callbacks are called when the main loop wakes up from an
    * idle state.
    *
    * @note Idle state doesn't mean that the @b program is idle, but
    * that the <b>main loop</b> is idle. It doesn't have any timers,
    * events, fd handlers or anything else to process (which in most
    * <em>event driven</em> programs also means that the @b program is
    * idle too, but it's not a rule). The program itself may be doing
    * a lot of processing in the idler, or in another thread, for
    * example.
    *
    * Example with functions that deal with idle state:
    *
    * @li @ref ecore_idler_example_c
    *
    * @ingroup Ecore_Main_Loop_Group
    *
    * @{
    */

   typedef struct _Ecore_Idler                 Ecore_Idler; /**< A handle for idlers */
   typedef struct _Ecore_Idle_Enterer          Ecore_Idle_Enterer; /**< A handle for idle enterers */
   typedef struct _Ecore_Idle_Exiter           Ecore_Idle_Exiter; /**< A handle for idle exiters */

   /**
    * Add an idler handler.
    * @param  func The function to call when idling.
    * @param  data The data to be passed to this @p func call.
    * @return A idler handle if successfully added.  NULL otherwise.
    *
    * Add an idler handle to the event loop, returning a handle on
    * success and NULL otherwise.  The function @p func will be called
    * repeatedly while no other events are ready to be processed, as
    * long as it returns 1 (or ECORE_CALLBACK_RENEW). A return of 0
    * (or ECORE_CALLBACK_CANCEL) deletes the idler.
    *
    * Idlers are useful for progressively prossessing data without blocking.
    */
   EAPI Ecore_Idler *ecore_idler_add(Ecore_Task_Cb func, const void *data);

   /**
    * Delete an idler callback from the list to be executed.
    * @param  idler The handle of the idler callback to delete
    * @return The data pointer passed to the idler callback on success.  NULL
    *         otherwise.
    */
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
   * @defgroup Ecore_Thread_Group Ecore Thread functions
   *
   * Facilities to run heavy tasks in different threads to avoid blocking
   * the main loop.
   *
   * The EFL is, for the most part, not thread safe. This means that if you
   * have some task running in another thread and you have, for example, an
   * Evas object to show the status progress of this task, you cannot update
   * the object from within the thread. This can only be done from the main
   * thread, the one running the main loop. This problem can be solved
   * by running a thread that sends messages to the main one using an
   * @ref Ecore_Pipe_Group "Ecore_Pipe", but when you need to handle other
   * things like cancelling the thread, your code grows in coplexity and gets
   * much harder to maintain.
   *
   * Ecore Thread is here to solve that problem. It is @b not a simple wrapper
   * around standard POSIX threads (or the equivalent in other systems) and
   * it's not meant to be used to run parallel tasks throughout the entire
   * duration of the program, especially when these tasks are performance
   * critical, as Ecore manages these tasks using a pool of threads based on
   * system configuration.
   *
   * What Ecore Thread does, is make it a lot easier to dispatch a worker
   * function to perform some heavy task and then get the result once it
   * completes, without blocking the application's UI. In addition, cancelling
   * and rescheduling comes practically for free and the developer needs not
   * worry about how many threads are launched, since Ecore will schedule
   * them according to the number of processors the system has and maximum
   * amount of concurrent threads set for the application.
   *
   * At the system level, Ecore will start a new thread on an as-needed basis
   * until the maximum set is reached. When no more threads can be launched,
   * new worker functions will be queued in a waiting list until a thread
   * becomes available. This way, system threads will be shared throughout
   * different worker functions, but running only one at a time. At the same
   * time, a worker function that is rescheduled may be run on a different
   * thread the next time.
   *
   * The ::Ecore_Thread handler has two meanings, depending on what context
   * it is on. The one returned when starting a worker with any of the
   * functions ecore_thread_run() or ecore_thread_feedback_run() is an
   * identifier of that specific instance of the function and can be used from
   * the main loop with the ecore_thread_cancel() and ecore_thread_check()
   * functions. This handler must not be shared with the worker function
   * function running in the thread. This same handler will be the one received
   * on the @c end, @c cancel and @c feedback callbacks.
   *
   * The worker function, that's the one running in the thread, also receives
   * an ::Ecore_Thread handler that can be used with ecore_thread_cancel() and
   * ecore_thread_check(), sharing the flag with the main loop. But this
   * handler is also associated with the thread where the function is running.
   * This has strong implications when working with thread local data.
   *
   * There are two kinds of worker threads Ecore handles: simple, or short,
   * workers and feedback workers.
   *
   * The first kind is for simple functions that perform a
   * usually small but time consuming task. Ecore will run this function in
   * a thread as soon as one becomes available and notify the calling user of
   * its completion once the task is done.
   *
   * The following image shows the flow of a program running four tasks on
   * a pool of two threads.
   *
   * @image html ecore_thread.png
   * @image rtf ecore_thread.png
   * @image latex ecore_thread.eps width=\textwidth
   *
   * For larger tasks that may require continuous communication with the main
   * program, the feedback workers provide the same functionality plus a way
   * for the function running in the thread to send messages to the main
   * thread.
   *
   * The next diagram omits some details shown in the previous one regarding
   * how threads are spawned and tasks are queued, but illustrates how feedback
   * jobs communicate with the main loop and the special case of threads
   * running out of pool.
   *
   * @image html ecore_thread_feedback.png
   * @image rtf ecore_thread_feedback.png
   * @image latex ecore_thread_feedback.eps width=\textwidth
   *
   * See an overview example in @ref ecore_thread_example_c.
   *
   * @ingroup Ecore_Main_Loop_Group
   *
   * @{
   */

   typedef struct _Ecore_Thread                Ecore_Thread; /**< A handle for threaded jobs */

   /**
    * @typedef Ecore_Thread_Cb Ecore_Thread_Cb
    * A callback used by Ecore_Thread helper.
    */
  typedef void (*Ecore_Thread_Cb) (void *data, Ecore_Thread *thread);
   /**
    * @typedef Ecore_Thread_Notify_Cb Ecore_Thread_Notify_Cb
    * A callback used by the main loop to receive data sent by an
    * @ref Ecore_Thread_Group.
    */
  typedef void (*Ecore_Thread_Notify_Cb) (void *data, Ecore_Thread *thread, void *msg_data);

  /**
   * Schedule a task to run in a parallel thread to avoid locking the main loop
   *
   * @param func_blocking The function that should run in another thread.
   * @param func_end Function to call from main loop when @p func_blocking
   * completes its task successfully (may be NULL)
   * @param func_cancel Function to call from main loop if the thread running
   * @p func_blocking is cancelled or fails to start (may be NULL)
   * @param data User context data to pass to all callbacks.
   * @return A new thread handler, or NULL on failure
   *
   * This function will try to create a new thread to run @p func_blocking in,
   * or if the maximum number of concurrent threads has been reached, will
   * add it to the pending list, where it will wait until a thread becomes
   * available. The return value will be an ::Ecore_Thread handle that can
   * be used to cancel the thread before its completion.
   *
   * @note This function should always return immediately, but in the rare
   * case that Ecore is built with no thread support, @p func_blocking will
   * be called here, actually blocking the main loop.
   *
   * Once a thread becomes available, @p func_blocking will be run in it until
   * it finishes, then @p func_end is called from the thread containing the
   * main loop to inform the user of its completion. While in @p func_blocking,
   * no functions from the EFL can be used, except for those from Eina that are
   * marked to be thread-safe. Even for the latter, caution needs to be taken
   * if the data is shared across several threads.
   *
   * @p func_end will be called from the main thread when @p func_blocking ends,
   * so here it's safe to use anything from the EFL freely.
   *
   * The thread can also be cancelled before its completion calling
   * ecore_thread_cancel(), either from the main thread or @p func_blocking.
   * In this case, @p func_cancel will be called, also from the main thread
   * to inform of this happening. If the thread could not be created, this
   * function will be called and it's @c thread parameter will be NULL. It's
   * also safe to call any EFL function here, as it will be running in the
   * main thread.
   *
   * Inside @p func_blocking, it's possible to call ecore_thread_reschedule()
   * to tell Ecore that this function should be called again.
   *
   * Be aware that no assumptions can be made about the order in which the
   * @p func_end callbacks for each task will be called. Once the function is
   * running in a different thread, it's the OS that will handle its running
   * schedule, and different functions may take longer to finish than others.
   * Also remember that just starting several tasks together doesn't mean they
   * will be running at the same time. Ecore will schedule them based on the
   * number of threads available for the particular system it's running in,
   * so some of the jobs started may be waiting until another one finishes
   * before it can execute its own @p func_blocking.
   *
   * @see ecore_thread_feedback_run()
   * @see ecore_thread_cancel()
   * @see ecore_thread_reschedule()
   * @see ecore_thread_max_set()
   */
   EAPI Ecore_Thread *ecore_thread_run(Ecore_Thread_Cb func_blocking,
                                       Ecore_Thread_Cb func_end,
                                       Ecore_Thread_Cb func_cancel,
                                       const void *data);
  /**
   * Launch a thread to run a task than can talk back to the main thread
   *
   * @param func_heavy The function that should run in another thread.
   * @param func_notify Function that receives the data sent from the thread
   * @param func_end Function to call from main loop when @p func_heavy
   * completes its task successfully
   * @param func_cancel Function to call from main loop if the thread running
   * @p func_heavy is cancelled or fails to start
   * @param data User context data to pass to all callback.
   * @param try_no_queue If you want to run outside of the thread pool.
   * @return A new thread handler, or NULL on failure
   *
   * See ecore_thread_run() for a general description of this function.
   *
   * The difference with the above is that ecore_thread_run() is meant for
   * tasks that don't need to communicate anything until they finish, while
   * this function is provided with a new callback, @p func_notify, that will
   * be called from the main thread for every message sent from @p func_heavy
   * with ecore_thread_feedback().
   *
   * Like with ecore_thread_run(), a new thread will be launched to run
   * @p func_heavy unless the maximum number of simultaneous threadas has been
   * reached, in which case the function will be scheduled to run whenever a
   * running task ends and a thread becomes free. But if @p try_no_queue is
   * set, Ecore will first try to launch a thread outside of the pool to run
   * the task. If it fails, it will revert to the normal behaviour of using a
   * thread from the pool as if @p try_no_queue had not been set.
   *
   * Keep in mind that Ecore handles the thread pool based on the number of
   * CPUs available, but running a thread outside of the pool doesn't count for
   * this, so having too many of them may have drastic effects over the
   * program's performance.
   *
   * @see ecore_thread_feedback()
   * @see ecore_thread_run()
   * @see ecore_thread_cancel()
   * @see ecore_thread_reschedule()
   * @see ecore_thread_max_set()
   */
   EAPI Ecore_Thread *ecore_thread_feedback_run(Ecore_Thread_Cb func_heavy,
                                                Ecore_Thread_Notify_Cb func_notify,
                                                Ecore_Thread_Cb func_end,
                                                Ecore_Thread_Cb func_cancel,
                                                const void *data,
                                                Eina_Bool try_no_queue);
  /**
   * Cancel a running thread.
   *
   * @param thread The thread to cancel.
   * @return Will return EINA_TRUE if the thread has been cancelled,
   *         EINA_FALSE if it is pending.
   *
   * This function can be called both in the main loop or in the running thread.
   *
   * This function cancels a running thread. If @p thread can be immediately
   * cancelled (it's still pending execution after creation or rescheduling),
   * then the @c cancel callback will be called, @p thread will be freed and
   * the function will return EINA_TRUE.
   *
   * If the thread is already running, then this function returns EINA_FALSE
   * after marking the @p thread as pending cancellation. For the thread to
   * actually be terminated, it needs to return from the user function back
   * into Ecore control. This can happen in several ways:
   * @li The function ends and returns normally. If it hadn't been cancelled,
   * @c func_end would be called here, but instead @c func_cancel will happen.
   * @li The function returns after requesting to be rescheduled with
   * ecore_thread_reschedule().
   * @li The function is prepared to leave early by checking if
   * ecore_thread_check() returns EINA_TRUE.
   *
   * The user function can cancel itself by calling ecore_thread_cancel(), but
   * it should always use the ::Ecore_Thread handle passed to it and never
   * share it with the main loop thread by means of shared user data or any
   * other way.
   *
   * @p thread will be freed and should not be used again if this function
   * returns EINA_TRUE or after the @c func_cancel callback returns.
   *
   * @see ecore_thread_check()
   */
   EAPI Eina_Bool     ecore_thread_cancel(Ecore_Thread *thread);
  /**
   * Checks if a thread is pending cancellation
   *
   * @param thread The thread to test.
   * @return EINA_TRUE if the thread is pending cancellation,
   *         EINA_FALSE if it is not.
   *
   * This function can be called both in the main loop or in the running thread.
   *
   * When ecore_thread_cancel() is called on an already running task, the
   * thread is marked as pending cancellation. This function returns EINA_TRUE
   * if this mark is set for the given @p thread and can be used from the
   * main loop thread to check if a still active thread has been cancelled,
   * or from the user function running in the thread to check if it should
   * stop doing what it's doing and return early, effectively cancelling the
   * task.
   *
   * @see ecore_thread_cancel()
   */
   EAPI Eina_Bool     ecore_thread_check(Ecore_Thread *thread);
  /**
   * Sends data from the worker thread to the main loop
   *
   * @param thread The current ::Ecore_Thread context to send data from
   * @param msg_data Data to be transmitted to the main loop
   * @return EINA_TRUE if @p msg_data was successfully sent to main loop,
   *         EINA_FALSE if anything goes wrong.
   *
   * You should use this function only in the @c func_heavy call.
   *
   * Only the address to @p msg_data will be sent and once this function
   * returns EINA_TRUE, the job running in the thread should never touch the
   * contents of it again. The data sent should be malloc()'ed or something
   * similar, as long as it's not memory local to the thread that risks being
   * overwritten or deleted once it goes out of scope or the thread finishes.
   *
   * Care must be taken that @p msg_data is properly freed in the @c func_notify
   * callback set when creating the thread.
   *
   * @see ecore_thread_feedback_run()
   */
   EAPI Eina_Bool     ecore_thread_feedback(Ecore_Thread *thread, const void *msg_data);
  /**
   * Asks for the function in the thread to be called again at a later time
   *
   * @param thread The current ::Ecore_Thread context to rescheduled
   * @return EINA_TRUE if the task was successfully rescheduled,
   *         EINA_FALSE if anything goes wrong.
   *
   * This function should be called only from the same function represented
   * by @pthread.
   *
   * Calling this function will mark the thread for a reschedule, so as soon
   * as it returns, it will be added to the end of the list of pending tasks.
   * If no other tasks are waiting or there are sufficient threads available,
   * the rescheduled task will be launched again immediately.
   *
   * This should never return EINA_FALSE, unless it was called from the wrong
   * thread or with the wrong arguments.
   *
   * The @c func_end callback set when the thread is created will not be
   * called until the function in the thread returns without being rescheduled.
   * Similarly, if the @p thread is cancelled, the reschedule will not take
   * effect.
   */
   EAPI Eina_Bool     ecore_thread_reschedule(Ecore_Thread *thread);
  /**
   * Gets the number of active threads running jobs
   *
   * @return Number of active threads running jobs
   *
   * This returns the number of threads currently running jobs of any type
   * through the Ecore_Thread API.
   *
   * @note Jobs started through the ecore_thread_feedback_run() function with
   * the @c try_no_queue parameter set to EINA_TRUE will not be accounted for
   * in the return of this function unless the thread creation fails and it
   * falls back to using one from the pool.
   */
   EAPI int           ecore_thread_active_get(void);
  /**
   * Gets the number of short jobs waiting for a thread to run
   *
   * @return Number of pending threads running "short" jobs
   *
   * This returns the number of tasks started with ecore_thread_run() that are
   * pending, waiting for a thread to become available to run them.
   */
   EAPI int           ecore_thread_pending_get(void);
  /**
   * Gets the number of feedback jobs waiting for a thread to run
   *
   * @return Number of pending threads running "feedback" jobs
   *
   * This returns the number of tasks started with ecore_thread_feedback_run()
   * that are pending, waiting for a thread to become available to run them.
   */
   EAPI int           ecore_thread_pending_feedback_get(void);
  /**
   * Gets the total number of pending jobs
   *
   * @return Number of pending threads running jobs
   *
   * Same as the sum of ecore_thread_pending_get() and
   * ecore_thread_pending_feedback_get().
   */
   EAPI int           ecore_thread_pending_total_get(void);
  /**
   * Gets the maximum number of threads that can run simultaneously
   *
   * @return Max possible number of Ecore_Thread's running concurrently
   *
   * This returns the maximum number of Ecore_Thread's that may be running at
   * the same time. If this number is reached, new jobs started by either
   * ecore_thread_run() or ecore_thread_feedback_run() will be added to the
   * respective pending queue until one of the running threads finishes its
   * task and becomes available to run a new one.
   *
   * By default, this will be the number of available CPUs for the
   * running program (as returned by eina_cpu_count()), or 1 if this value
   * could not be fetched.
   *
   * @see ecore_thread_max_set()
   * @see ecore_thread_max_reset()
   */
   EAPI int           ecore_thread_max_get(void);
  /**
   * Sets the maximum number of threads allowed to run simultaneously
   *
   * @param num The new maximum
   *
   * This sets a new value for the maximum number of concurrently running
   * Ecore_Thread's. It @b must an integer between 1 and (2 * @c x), where @c x
   * is the number for CPUs available.
   *
   * @see ecore_thread_max_get()
   * @see ecore_thread_max_reset()
   */
   EAPI void          ecore_thread_max_set(int num);
  /**
   * Resets the maximum number of concurrently running threads to the default
   *
   * This resets the value returned by ecore_thread_max_get() back to its
   * default.
   *
   * @see ecore_thread_max_get()
   * @see ecore_thread_max_set()
   */
   EAPI void          ecore_thread_max_reset(void);
  /**
   * Gets the number of threads available for running tasks
   *
   * @return The number of available threads
   *
   * Same as doing ecore_thread_max_get() - ecore_thread_active_get().
   *
   * This function may return a negative number only in the case the user
   * changed the maximum number of running threads while other tasks are
   * running.
   */
   EAPI int           ecore_thread_available_get(void);
  /**
   * Adds some data to a hash local to the thread
   *
   * @param thread The thread context the data belongs to
   * @param key The name under which the data will be stored
   * @param value The data to add
   * @param cb Function to free the data when removed from the hash
   * @param direct If true, this will not copy the key string (like
   * eina_hash_direct_add())
   * @return EINA_TRUE on success, EINA_FALSE on failure
   *
   * Ecore Thread has a mechanism to share data across several worker functions
   * that run on the same system thread. That is, the data is stored per
   * thread and for a worker function to have access to it, it must be run
   * by the same thread that stored the data.
   *
   * When there are no more workers pending, the thread will be destroyed
   * along with the internal hash and any data left in it will be freed with
   * the @p cb function given.
   *
   * This set of functions is useful to share things around several instances
   * of a function when that thing is costly to create and can be reused, but
   * may only be used by one function at a time.
   *
   * For example, if you have a program doing requisitions to a database,
   * these requisitions can be done in threads so that waiting for the
   * database to respond doesn't block the UI. Each of these threads will
   * run a function, and each function will be dependent on a connection to
   * the database, which may not be able to handle more than one request at
   * a time so for each running function you will need one connection handle.
   * The options then are:
   * @li Each function opens a connection when it's called, does the work and
   * closes the connection when it finishes. This may be costly, wasting a lot
   * of time on resolving hostnames, negotiating permissions and allocating
   * memory.
   * @li Open the connections in the main loop and pass it to the threads
   * using the data pointer. Even worse, it's just as costly as before and now
   * it may even be kept with connections open doing nothing until a thread
   * becomes available to run the function.
   * @li Have a way to share connection handles, so that each instance of the
   * function can check if an available connection exists, and if it doesn't,
   * create one and add it to the pool. When no more connections are needed,
   * they are all closed.
   *
   * The last option is the most efficient, but it requires a lot of work to
   * implement properly. Using thread local data helps to achieve the same
   * result while avoiding doing all the tracking work on your code. The way
   * to use it would be, at the worker function, to ask for the connection
   * with ecore_thread_local_data_find() and if it doesn't exist, then open
   * a new one and save it with ecore_thread_local_data_add(). Do the work and
   * forget about the connection handle, when everything is done the function
   * just ends. The next worker to run on that thread will check if a
   * connection exists and find that it does, so the process of opening a
   * new one has been spared. When no more workers exist, the thread is
   * destroyed and the callback used when saving the connection will be called
   * to close it.
   *
   * This function adds the data @p value to the thread data under the given
   * @p key.
   * No other value in the hash may have the same @p key. If you need to
   * change the value under a @p key, or you don't know if one exists already,
   * you can use ecore_thread_local_data_set().
   *
   * Neither @p key nor @p value may be NULL and @p key will be copied in the
   * hash, unless @p direct is set, in which case the string used should not
   * be freed until the data is removed from the hash.
   *
   * The @p cb function will be called when the data in the hash needs to be
   * freed, be it because it got deleted with ecore_thread_local_data_del() or
   * because @p thread was terminated and the hash destroyed. This parameter
   * may be NULL, in which case @p value needs to be manually freed after
   * removing it from the hash with either ecore_thread_local_data_del() or
   * ecore_thread_local_data_set(), but it's very unlikely that this is what
   * you want.
   *
   * This function, and all of the others in the @c ecore_thread_local_data
   * family of functions, can only be called within the worker function running
   * in the thread. Do not call them from the main loop or from a thread
   * other than the one represented by @p thread.
   *
   * @see ecore_thread_local_data_set()
   * @see ecore_thread_local_data_find()
   * @see ecore_thread_local_data_del()
   */
   EAPI Eina_Bool     ecore_thread_local_data_add(Ecore_Thread *thread, const char *key, void *value, Eina_Free_Cb cb, Eina_Bool direct);
  /**
   * Sets some data in the hash local to the given thread
   *
   * @param thread The thread context the data belongs to
   * @param key The name under which the data will be stored
   * @param value The data to add
   * @param cb Function to free the data when removed from the hash
   *
   * If no data exists in the hash under the @p key, this function adds
   * @p value in the hash under the given @p key and returns NULL.
   * The key itself is copied.
   *
   * If the hash already contains something under @p key, the data will be
   * replaced by @p value and the old value will be returned.
   *
   * NULL will also be returned if either @p key or @p value are NULL, or if
   * an error occurred.
   *
   * This function, and all of the others in the @c ecore_thread_local_data
   * family of functions, can only be called within the worker function running
   * in the thread. Do not call them from the main loop or from a thread
   * other than the one represented by @p thread.
   *
   * @see ecore_thread_local_data_add()
   * @see ecore_thread_local_data_del()
   * @see ecore_thread_local_data_find()
   */
   EAPI void         *ecore_thread_local_data_set(Ecore_Thread *thread, const char *key, void *value, Eina_Free_Cb cb);
  /**
   * Gets data stored in the hash local to the given thread
   *
   * @param thread The thread context the data belongs to
   * @param key The name under which the data is stored
   * @return The value under the given key, or NULL on error
   *
   * Finds and return the data stored in the shared hash under the key @p key.
   *
   * This function, and all of the others in the @c ecore_thread_local_data
   * family of functions, can only be called within the worker function running
   * in the thread. Do not call them from the main loop or from a thread
   * other than the one represented by @p thread.
   *
   * @see ecore_thread_local_data_add()
   * @see ecore_thread_local_data_wait()
   */
   EAPI void         *ecore_thread_local_data_find(Ecore_Thread *thread, const char *key);
  /**
   * Deletes from the thread's hash the data corresponding to the given key
   *
   * @param thread The thread context the data belongs to
   * @param key The name under which the data is stored
   * @return EINA_TRUE on success, EINA_FALSE on failure
   *
   * If there's any data stored associated with @p key in the global hash,
   * this function will remove it from it and return EINA_TRUE. If no data
   * exists or an error occurs, it returns EINA_FALSE.
   *
   * If the data was added to the hash with a free function, then it will
   * also be freed after removing it from the hash, otherwise it requires
   * to be manually freed by the user, which means that if no other reference
   * to it exists before calling this function, it will result in a memory
   * leak.
   *
   * This function, and all of the others in the @c ecore_thread_local_data
   * family of functions, can only be called within the worker function running
   * in the thread. Do not call them from the main loop or from a thread
   * other than the one represented by @p thread.
   *
   * @see ecore_thread_local_data_add()
   */
   EAPI Eina_Bool     ecore_thread_local_data_del(Ecore_Thread *thread, const char *key);

  /**
   * Adds some data to a hash shared by all threads
   *
   * @param key The name under which the data will be stored
   * @param value The data to add
   * @param cb Function to free the data when removed from the hash
   * @param direct If true, this will not copy the key string (like
   * eina_hash_direct_add())
   * @return EINA_TRUE on success, EINA_FALSE on failure
   *
   * Ecore Thread keeps a hash that can be used to share data across several
   * threads, including the main loop one, without having to manually handle
   * mutexes to do so safely.
   *
   * This function adds the data @p value to this hash under the given @p key.
   * No other value in the hash may have the same @p key. If you need to
   * change the value under a @p key, or you don't know if one exists already,
   * you can use ecore_thread_global_data_set().
   *
   * Neither @p key nor @p value may be NULL and @p key will be copied in the
   * hash, unless @p direct is set, in which case the string used should not
   * be freed until the data is removed from the hash.
   *
   * The @p cb function will be called when the data in the hash needs to be
   * freed, be it because it got deleted with ecore_thread_global_data_del() or
   * because Ecore Thread was shut down and the hash destroyed. This parameter
   * may be NULL, in which case @p value needs to be manually freed after
   * removing it from the hash with either ecore_thread_global_data_del() or
   * ecore_thread_global_data_set().
   *
   * Manually freeing any data that was added to the hash with a @p cb function
   * is likely to produce a segmentation fault, or any other strange
   * happenings, later on in the program.
   *
   * @see ecore_thread_global_data_del()
   * @see ecore_thread_global_data_set()
   * @see ecore_thread_global_data_find()
   */
   EAPI Eina_Bool     ecore_thread_global_data_add(const char *key, void *value, Eina_Free_Cb cb, Eina_Bool direct);
  /**
   * Sets some data in the hash shared by all threads
   *
   * @param key The name under which the data will be stored
   * @param value The data to add
   * @param cb Function to free the data when removed from the hash
   *
   * If no data exists in the hash under the @p key, this function adds
   * @p value in the hash under the given @p key and returns NULL.
   * The key itself is copied.
   *
   * If the hash already contains something under @p key, the data will be
   * replaced by @p value and the old value will be returned.
   *
   * NULL will also be returned if either @p key or @p value are NULL, or if
   * an error occurred.
   *
   * @see ecore_thread_global_data_add()
   * @see ecore_thread_global_data_del()
   * @see ecore_thread_global_data_find()
   */
   EAPI void         *ecore_thread_global_data_set(const char *key, void *value, Eina_Free_Cb cb);
  /**
   * Gets data stored in the hash shared by all threads
   *
   * @param key The name under which the data is stored
   * @return The value under the given key, or NULL on error
   *
   * Finds and return the data stored in the shared hash under the key @p key.
   *
   * Keep in mind that the data returned may be used by more than one thread
   * at the same time and no reference counting is done on it by Ecore.
   * Freeing the data or modifying its contents may require additional
   * precautions to be considered, depending on the application's design.
   *
   * @see ecore_thread_global_data_add()
   * @see ecore_thread_global_data_wait()
   */
   EAPI void         *ecore_thread_global_data_find(const char *key);
  /**
   * Deletes from the shared hash the data corresponding to the given key
   *
   * @param key The name under which the data is stored
   * @return EINA_TRUE on success, EINA_FALSE on failure
   *
   * If there's any data stored associated with @p key in the global hash,
   * this function will remove it from it and return EINA_TRUE. If no data
   * exists or an error occurs, it returns EINA_FALSE.
   *
   * If the data was added to the hash with a free function, then it will
   * also be freed after removing it from the hash, otherwise it requires
   * to be manually freed by the user, which means that if no other reference
   * to it exists before calling this function, it will result in a memory
   * leak.
   *
   * Note, also, that freeing data that other threads may be using will result
   * in a crash, so appropriate care must be taken by the application when
   * that possibility exists.
   *
   * @see ecore_thread_global_data_add()
   */
   EAPI Eina_Bool     ecore_thread_global_data_del(const char *key);
  /**
   * Gets data stored in the shared hash, or wait for it if it doesn't exist
   *
   * @param key The name under which the data is stored
   * @param seconds The amount of time in seconds to wait for the data.
   * @return The value under the given key, or NULL on error
   *
   * Finds and return the data stored in the shared hash under the key @p key.
   *
   * If there's nothing in the hash under the given @p key, the function
   * will block and wait up to @p seconds seconds for some other thread to
   * add it with either ecore_thread_global_data_add() or
   * ecore_thread_global_data_set(). If after waiting there's still no data
   * to get, NULL will be returned.
   *
   * If @p seconds is 0, then no waiting will happen and this function works
   * like ecore_thread_global_data_find(). If @p seconds is less than 0, then
   * the function will wait indefinitely.
   *
   * Keep in mind that the data returned may be used by more than one thread
   * at the same time and no reference counting is done on it by Ecore.
   * Freeing the data or modifying its contents may require additional
   * precautions to be considered, depending on the application's design.
   *
   * @see ecore_thread_global_data_add()
   * @see ecore_thread_global_data_find()
   */
   EAPI void         *ecore_thread_global_data_wait(const char *key, double seconds);

  /**
   * @}
   */

  /**
   * @defgroup Ecore_Pipe_Group Pipe wrapper
   *
   * These functions wrap the pipe / write / read functions to easily
   * integrate its use into ecore's main loop.
   *
   * The ecore_pipe_add() function creates file descriptors (sockets
   * on Windows) and attach a handle to the ecore main loop. That
   * handle is called when data is read in the pipe. To write data in
   * the pipe, just call ecore_pipe_write(). When you are done, just
   * call ecore_pipe_del().
   *
   * For examples see here:
   * @li @ref tutorial_ecore_pipe_gstreamer_example
   * @li @ref tutorial_ecore_pipe_simple_example
   *
   * @ingroup Ecore_Main_Loop_Group
   *
   * @{
   */

   typedef struct _Ecore_Pipe                  Ecore_Pipe; /**< A handle for pipes */

   /**
    * @typedef Ecore_Pipe_Cb Ecore_Pipe_Cb
    * The callback that data written to the pipe is sent to.
    */
   typedef void (*Ecore_Pipe_Cb) (void *data, void *buffer, unsigned int nbyte);

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
    * @defgroup Ecore_Job_Group Ecore Job functions
    *
    * You can queue jobs that are to be done by the main loop when the
    * current event is dealt with.
    *
    * Jobs are processed by the main loop similarly to events. They
    * also will be executed in the order in which they were added.
    *
    * A good use for them is when you don't want to execute an action
    * immeditately, but want to give the control back to the main loop
    * so that it will call your job callback when jobs start being
    * processed (and if there are other jobs added before yours, they
    * will be processed first). This also gives the chance to other
    * actions in your program to cancel the job before it is started.
    *
    * Examples of using @ref Ecore_Job:
    * @li @ref ecore_job_example_c
    *
    * @ingroup Ecore_Main_Loop_Group
    *
    * @{
    */

   typedef struct _Ecore_Job Ecore_Job; /**< A job handle */

   EAPI Ecore_Job *ecore_job_add(Ecore_Cb func, const void *data);
   EAPI void      *ecore_job_del(Ecore_Job *job);

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
   * @defgroup Ecore_Throttle_Group Ecore Throttle functions
   *
   * @ingroup Ecore_Main_Loop_Group
   *
   * @{
   */

   EAPI void       ecore_throttle_adjust(double amount );
   EAPI double     ecore_throttle_get(void);

  /**
   * @}
   */

#ifdef __cplusplus
}
#endif
#endif
