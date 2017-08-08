/**
 * @brief Ecore Library Public API Calls.
 *        These routines are used for Ecore Library interaction.
 */

/**
 * @page ecore_main Ecore
 * @date 2000 (created)
 * @section toc Table of Contents
 * @li @ref ecore_main_intro
 * @li @ref ecore_main_compiling
 * @li @ref ecore_main_next_steps
 * @li @ref ecore_main_intro_example
 * @section ecore_main_intro Introduction
 * Ecore is a library of convenience functions. A brief explanation of how to use
 * it can be found in @ref Ecore_Main_Loop_Page.
 * The Ecore library provides the following modules:
 * @li @ref Ecore_Init_Group
 * @li @ref Ecore_Getopt_Group
 * @li @ref Ecore_Main_Loop_Group
 * @li @ref Ecore_System_Events
 * @li @ref Ecore_Time_Group
 * @li @ref Ecore_Thread_Group
 * @li @ref Ecore_Pipe_Group
 * @li @ref Ecore_Application_Group
 * @li @ref Ecore_Throttle_Group
 * @li @ref Ecore_Job_Group
 * @li @ref Ecore_File_Group
 * @li @ref Ecore_Con_Group
 * @li @ref Ecore_Evas_Group
 * @li @ref Ecore_FB_Group
 * @li @ref Ecore_Input_Group
 * @li @ref Ecore_IMF_Lib_Group
 * @li @ref Ecore_IPC_Group
 * @li @link Ecore_X.h Ecore_X - X Windows System wrapper. @endlink
 * @li @ref Ecore_Win32_Group
 * @li @ref Ecore_Audio_Group
 * @li @ref Ecore_Avahi_Group
 * @li @ref Ecore_Drm_Group
 * @li @ref Ecore_Wl_Group
 * For more info on Ecore usage, there are these @ref ecore_examples.
 *
 * @section ecore_main_compiling How to compile
 * Ecore is a library your application links to. The procedure for
 * this is very simple. You simply have to compile your application
 * with the appropriate compiler flags that the @p pkg-config script
 * outputs. Note that each module is separate in pkg-config. For
 * example using @ref Ecore_Evas_Group:
 * Compiling C or C++ files into object files:
 * @verbatim
 * gcc -c -o main.o main.c `pkg-config --cflags ecore ecore-evas`
 * @endverbatim
 * Linking object files into a binary executable:
 * @verbatim
 * gcc -o my_application main.o `pkg-config --libs ecore ecore-evas`
 * @endverbatim
 * See @ref pkgconfig
 *
 * @section ecore_main_next_steps Next Steps
 * After you understood what Ecore is and installed it in your system
 * you should proceed to understand the programming interface. We'd
 * recommend you to take a while to learn @ref Eina as it is very
 * convenient and optimized, and Ecore uses it extensively.
 * Recommended reading:
 * @li @ref Ecore_Timer_Group
 * @li @ref Ecore_Idle_Group
 * @li @ref Ecore_FD_Handler_Group
 * @li @ref Ecore_Event_Group
 * @li @ref Ecore_Exe_Group
 * @li @ref Ecore_Animator_Group
 * @li @ref Ecore_Poller_Group
 *
 * @section ecore_main_intro_example Introductory Examples

 * @include ecore_timer_example.c
 * More examples can be found at @ref ecore_examples.
 */

/**
 * @page Ecore_Main_Loop_Page The Ecore Main Loop
 *
 * @section Ecore_Main_Loop_Page_intro What is Ecore?
 *
 * Ecore is a clean and tiny event loop library with many modules to do lots of
 * convenient things for a programmer to save time and effort. It's small and
 * lean, designed to work from embedded systems all the way up to large and
 * powerful multi-cpu workstations. The main loop has a number of primitives to
 * be used with its main loop. It serializes all the primitives and allows for
 * great responsiveness without the need for threads(or any other concurrency).
 *
 * @subsection timers Timers
 *
 * Timers serve two main purposes: doing something at a specified time and
 * repeatedly doing something with a set interval.
 *
 * @see Ecore_Timer_Group
 *
 * @subsection pollers Pollers
 *
 * Pollers allow for polling to be centralized into a single place therefore
 * alleviating the need for different parts of the program to wake up at
 * different times to do polling, thereby making the code simpler and more efficient.
 *
 * @see Ecore_Poller_Group
 *
 * @subsection idler Idlers
 * There are three types of idlers: enterers, idlers(proper) and exiters. They
 * are called, respectively, when the program is about to enter an idle state,
 * when the program is idle, and when the program is leaving an idle state.
 * Idler enterers are usually a good place to update the program state. Proper idlers
 * are the appropriate place to do heavy computational tasks thereby using what
 * would otherwise be wasted CPU cycles. Exiters are the perfect place to do
 * anything your program should do just before processing events (also timers,
 * pollers, file descriptor handlers and animators)
 *
 * @see Ecore_Idle_Group
 *
 * @subsection fd_handler File descriptor handlers
 *
 * File descriptor handlers allow you to monitor when there is data available to
 * read on file descriptors, when writing will not block, or if there was an
 * error. Any valid file descriptor can be used with this API, regardless of if
 * was gotten with an OS specific API or from ecore.
 *
 * @see Ecore_FD_Handler_Group
 *
 * @subsection animators Animators
 *
 * Ecore provides a facility called animators, so named since the intended use
 * was in animations, that facilitates knowing what percentage of a given
 * interval has elapsed. This is perfect for performing animations, but is not
 * limited to that use, it can, for example, also be used to create a progress bar.
 *
 * @see Ecore_Animator_Group
 *
 * @subsection ev_handlers Event handlers
 *
 * Event handlers are, arguably, the most important feature of the ecore main
 * loop, they are what allows the programmer to easily handle user interaction.
 * Events however are not only things the user does, events can represent
 * anything for which a type is created.
 *
 * @see Ecore_Event_Group
 * All of these primitives are discussed in more detail in their respective
 * pages linked above.
 *
 * Here is a diagram of the main loop flow of a simple program:
 *
 * @image html  prog_flow.png
 * @image latex prog_flow.eps width=\textwidth
 *
 *
 *
 * @section Ecore_Main_Loop_Page_work How does Ecore work?
 *
 * Ecore is very easy to learn and use. All the function calls are designed to
 * be easy to remember, explicit in describing what they do, and heavily
 * name-spaced. Ecore programs can start and be very simple.
 *
 * For example:
 *
 * @code
 * #include <Ecore.h>
 *
 * int
 * main(int argc, const char **argv)
 * {
 *    ecore_init();
 *    ecore_app_args_set(argc, argv);
 *    ecore_main_loop_begin();
 *    ecore_shutdown();
 *    return 0;
 * }
 * @endcode
 *
 * This program is very simple and doesn't check for errors, but it does start up
 * and begin a main loop waiting for events or timers to tick off. This program
 * doesn't set up any, but now we can expand on this simple program a little
 * more by adding some event handlers and timers.
 *
 * @code
 * #include <Ecore.h>
 *
 * Ecore_Timer         *timer1     = NULL;
 * Ecore_Event_Handler *handler1   = NULL;
 * double               start_time = 0.0;
 *
 * int
 * timer_func(void *data)
 * {
 *    printf("Tick timer. Sec: %3.2f\n", ecore_time_get() - start_time);
 *    return ECORE_CALLBACK_RENEW;
 * }
 *
 * int
 * exit_func(void *data, int ev_type, void *ev)
 * {
 *    Ecore_Event_Signal_Exit *e;
 *
 *    e = (Ecore_Event_Signal_Exit *)ev;
 *    if (e->interrupt)      printf("Exit: interrupt\n");
 *    else if (e->quit)      printf("Exit: quit\n");
 *    else if (e->terminate) printf("Exit: terminate\n");
 *    ecore_main_loop_quit();
 *    return 1;
 * }
 *
 * int
 * main(int argc, const char **argv)
 * {
 *    ecore_init();
 *    ecore_app_args_set(argc, argv);
 *    start_time = ecore_time_get();
 *    handler1 = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_func, NULL);
 *    timer1 = ecore_timer_add(0.5, timer_func, NULL);
 *    ecore_main_loop_begin();
 *    ecore_shutdown();
 *    return 0;
 * }
 * @endcode
 *
 * In the previous example, we initialize our application and get the time at
 * which our program has started so we can calculate an offset. We set
 * up a timer to tick off in 0.5 seconds, and since it returns 1, will
 * keep ticking off every 0.5 seconds until it returns 0, or is deleted
 * by hand. An event handler is set up to call a function -
 * exit_func(),
 * whenever an event of type ECORE_EVENT_SIGNAL_EXIT is received (CTRL-C
 * on the command line will cause such an event to happen). If this event
 * occurs it tells you what kind of exit signal was received, and asks
 * the main loop to quit when it is finished by calling
 * ecore_main_loop_quit().
 *
 * The handles returned by ecore_timer_add() and
 * ecore_event_handler_add() are
 * only stored here as an example. If you don't need to address the timer or
 * event handler again you don't need to store the result, so just call the
 * function, and don't assign the result to any variable.
 *
 * This program looks slightly more complex than needed to do these simple
 * things, but in principle, programs don't get any more complex. You add more
 * event handlers, for more events, will have more timers and such, BUT it all
 * follows the same principles as shown in this example.
 *
 */


/**
 * @page Ecore_Config_Page The Enlightened Property Library
 * The Enlightened Property Library (Ecore_Config) is an abstraction
 * from the complexities of writing your own configuration. It provides
 * many features using the Enlightenment 17 development libraries.
 * To use the library, you:
 * @li Set the default values of your properties.
 * @li Load the configuration from a file.  You must set the default values
 * first, so that the library knows the correct type of each argument.
 * The following examples show how to use the Enlightened Property Library:
 * @li @link config_basic_example.c config_basic_example.c @endlink
 * @li @link config_listener_example.c config_listener_example.c @endlink
 */


/**
 * @page X_Window_System_Page X Window System
 * The Ecore library includes a wrapper for handling the X window system.
 * This page briefly explains what the X window system is and various terms
 * that are used.
 */

#ifndef _ECORE_H
#define _ECORE_H

#include <Efl_Config.h>

#ifdef _MSC_VER
# include <Evil.h>
#endif

#include <Eina.h>
#include <Eo.h>
#include <Efl.h>

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
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# undef WIN32_LEAN_AND_MEAN
#elif defined (__FreeBSD__) || defined (__OpenBSD__)
# include <sys/select.h>
# include <signal.h>
#elif defined (__ANDROID__)
# include <sys/select.h>
#elif defined (__sun)
# include <sys/siginfo.h>
#else
# include <sys/time.h>
# if !defined (EXOTIC_NO_SIGNAL)
#  include <signal.h>
# endif
#endif

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "Ecore_Common.h"
#include "efl_promise2.h"
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "Ecore_Legacy.h"
#endif
#ifdef EFL_EO_API_SUPPORT
#include "Ecore_Eo.h"
#endif

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
