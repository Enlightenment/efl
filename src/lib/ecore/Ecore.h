/**
 * @defgroup Ecore_Group Ecore
 * @ingroup EFL_Group
 *
 * @brief Ecore Library Public API Calls
 *
 * @remarks These routines are used for Ecore Library interaction.
 *
 * See @ref ecore_main for more details
 *
 * @page ecore_main Ecore
 *
 * @date 2000 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref ecore_main_intro
 * @li @ref ecore_main_next_steps
 *
 * @section ecore_main_intro Introduction
 *
 * Ecore is a library of convenience functions. A brief explanation of how to use
 * it can be found in @ref Ecore_Main_Loop_Page.
 *
 * The Ecore library provides the following modules:
 * @li @ref Ecore_Main_Loop_Group
 * @internal
 * @li @ref Ecore_File_Group
 * @li @ref Ecore_Con_Group
 * @li @ref Ecore_Evas_Group
 * @li @ref Ecore_FB_Group
 * @li @ref Ecore_IMF_Group
 * @li @ref Ecore_IMF_Context_Group
 * @li @ref Ecore_IMF_Evas_Group
 * @endinternal
 * @li @link Ecore_Ipc.h    Ecore_IPC - Inter Process Communication functions. @endlink
 * @internal
 * @li @link Ecore_X.h      Ecore_X - X Windows System wrapper. @endlink
 * @endinternal
 *
 * @section ecore_main_next_steps Next Steps
 *
 * After you understood what Ecore is and installed it in your system
 * you should proceed understanding the programming interface. We'd
 * recommend you to take a while to learn @ref Eina_Group as it is very
 * convenient and optimized, and Ecore uses it extensively.
 *
 * Recommended reading:
 *
 * @li @ref Ecore_Timer_Group
 * @li @ref Ecore_Idle_Group
 * @li @ref Ecore_FD_Handler_Group
 * @li @ref Ecore_Event_Group
 * @internal
 * @li @ref Ecore_Exe_Group
 * @endinternal
 *
 */

/**
 * @page Ecore_Main_Loop_Page The Ecore Main Loop
 *
 * @section Ecore_Main_Loop_Page_intro What is Ecore?
 *
 * Ecore is a clean and tiny event loop library with many modules to do lots of
 * convenient things for a programmer as well as to save time and effort. It's small and
 * lean, designed to work from embedded systems all the way up to large and
 * powerful multi-cpu workstations. The main loop has a number of primitives to
 * be used with its main loop. It serializes all the primitives and allows for
 * great responsiveness without the need for threads(or any other concurrency).
 *
 * @subsection timers Timers
 *
 * Timers serve two main purposes: doing something at a specified time and
 * repeatedly doing something with a set interval.
 * @see Ecore_Timer_Group
 *
 * @subsection pollers Pollers
 *
 * Pollers allow for polling to be centralized into a single place. Therefore,
 * alleviating the need for different parts of the program to wake up at
 * different times to do polling, thereby making the code simpler and more
 * efficient.
 * @see Ecore_Poller_Group
 *
 * @subsection idler Idlers
 *
 * There are three types of idlers: enterers, idlers(proper), and exiters, they
 * are called respectively when the program is about to enter an idle state,
 * when the program is idle, and when the program is leaving an idle state. Idler
 * enterers are usually a good place to update the program state. Proper idlers
 * are the appropriate place to do heavy computational tasks thereby using what
 * would otherwise be wasted CPU cycles. Exiters are the perfect place to do
 * anything that your program should do just before processing events(also timers,
 * poolers, file descriptor handlers, and animators)
 * @see Ecore_Idle_Group
 *
 * @subsection fd_handler File descriptor handlers
 *
 * File descriptor handlers allow you to monitor when there is data available to
 * read on file descriptors, when writing is not blocked or when there is an
 * error. Any valid file descriptor can be used with this API, regardless of whether
 * it is obtained with an OS specific API or from ecore.
 * @see Ecore_FD_Handler_Group
 *
 * @subsection animators Animators
 *
 * Ecore provides a facility called animators, so named since the intended use
 * is in animations, that facilitates knowing what percentage of a given
 * interval has elapsed. This is perfect for performing animations, but is not
 * limited to that use. It can, for example, also be used to create a progress
 * bar.
 * @see Ecore_Animator_Group
 *
 * @subsection ev_handlers Event handlers
 *
 * Event handlers are, arguably, the most important feature of the ecore main
 * loop, they are what allows the programmer to easily handle user interaction.
 * Events, however, are not the only things that the user does. Events can represent
 * anything for which a type is created.
 * @see Ecore_Event_Group
 *
 * All of these primitives are discussed in more detail in their respective
 * pages that are linked above.
 *
 * Here is a diagram of the main loop flow of a simple program:
 *
 * @image html  prog_flow.png
 * @image latex prog_flow.eps "prog flow" width=\textwidth
 *
 * @section Ecore_Main_Loop_Page_work How does Ecore work?
 *
 * Ecore is very easy to learn and use. All the function calls are designed to
 * be easy to remember, explicit in describing what they do, and heavily
 * name-spaced. Ecore programs can start easily and are very simple.
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
 * and begin a main loop that is waiting for events or timers to tick off. This program
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
 * which our program has started so that we can calculate an offset. We set
 * up a timer to tick off in @c 0.5 seconds, and since it returns @c 1, it
 * keeps ticking off every @c 0.5 seconds until it returns @c 0, or is deleted
 * by hand. An event handler is set up to call a function -
 * exit_func(),
 * whenever an event of type ECORE_EVENT_SIGNAL_EXIT is received (CTRL-C
 * on the command line causes such an event to happen). If this event
 * occurs it tells you what kind of exit signal is received, and asks
 * the main loop to quit when it is finished by calling
 * ecore_main_loop_quit().
 *
 * The handles returned by ecore_timer_add() and
 * ecore_event_handler_add() are
 * only stored here as an example. If you don't need to address the timer or
 * event handler again you don't need to store the result, so just call the
 * function and don't assign the result to any variable.
 *
 * This program looks slightly more complex than needed to do these simple
 * things, but in principle, programs don't get any more complex. You add more
 * event handlers for more events, you have more timers, BUT it all
 * follows the same principles as shown in this example.
 *
 */

/*
   @page Ecore_Config_Page The Enlightened Property Library

   The Enlightened Property Library (Ecore_Config) is an abstraction
   from the complexities of writing your own configuration. It provides
   many features using the Enlightenment 17 development libraries.

   To use the library, you:
   @li Set the default values of your properties.
   @li Load the configuration from a file. You must set the default values
    first, so that the library knows the correct type of each argument.

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
#elif defined (__FreeBSD__) || defined (__OpenBSD__)
# include <sys/select.h>
# include <signal.h>
#elif defined (__ANDROID__)
# include <sys/select.h>
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

/**
 * @internal
 * @defgroup Ecore_Init_Group Ecore initialization, shutdown functions and reset on fork.
 * @ingroup Ecore_Group
 *
 * @{
 */

/**
 * @brief Initialize the Ecore library.
 *
 * @details This function sets up connections, sockets, all singal handlers and
 *          the basic event loop, etc. If it succeeds, 1 or greater will be
 *          returned, otherwise 0 will be returned.
 *
 * @remarks This function initializes the Ecore library, making the proper calls
 *          to internal initialization functions. It will also initialize its
 *          @b dependencies, making calls to @c eina_init().
 *          So, there is no need to call those functions again, in your code.
 *          To shutdown Ecore, there is the function ecore_shutdown().
 *
 * @code
 * #include <Ecore.h>
 *
 * int main(int argc, char **argv)
 * {
 *   if (!ecore_init())
 *   {
 *     printf("ERROR: Cannot init Ecore!\n");
 *     return -1;
 *   }
 *   ecore_main_loop_begin();
 *   ecore_shutdown();
 * }
 * @endcode
 *
 * @return 1 or greater on success, 0 otherwise
 *
 * @see ecore_shutdown()
 * @see eina_init()
 */
EAPI int ecore_init(void);

/**
 * @brief Shutdown the Ecore library.
 *
 * @details Shut down connections, signal handlers sockets etc.
 *
 * @remarks This function shuts down all things set up in ecore_init() and
 *          cleans up all event queues, handlers, filters, timers, idlers,
 *          idle enterers/exiters etc. set up after ecore_init() was called.
 *
 * @remarks Do not call this function from any callback that may be called
 *          from the main loop, as the main loop will then fall over and not
 *          function properly.
 *
 * @details This function shuts down the Edje library. It will also call the
 *          shutdown functions of its @b dependencies, which is @c
 *          eina_shutdown().
 *          so there is no need to call these functions again in your code.
 *          This returns The number of times the library has been initialised
 *          without being shutdown.
 *
 * @return 0 if ecore shuts down, greater than 0 otherwise.
 *
 * @see ecore_init()
 * @see eina_shutdown()
 */
EAPI int ecore_shutdown(void);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Ecore_Application_Group Ecore Application
 * @ingroup Ecore_Group
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
 * @defgroup Ecore_Main_Loop_Group Ecore Main Loop
 * @ingroup Ecore_Group
 *
 * @brief This group discusses functions that are acting on Ecore's main loop itself or
 *        on events and infrastructure directly linked to it. Most programs only need
 *        to start and end the main loop, the rest of the function discussed here is
 *        meant to be used in special situations, and with great care.
 *
 *        For details on the usage of ecore's main loop and how it interacts with other
 *        ecore facilities see: @ref Ecore_Main_Loop_Page.
 *
 * @{
 */

#define ECORE_VERSION_MAJOR 1
#define ECORE_VERSION_MINOR 8

typedef struct _Ecore_Version
{
   int major;
   int minor;
   int micro;
   int revision;
} Ecore_Version;

EAPI extern Ecore_Version *ecore_version;

#define ECORE_CALLBACK_CANCEL  EINA_FALSE /**< Return value to remove a callback */
#define ECORE_CALLBACK_RENEW   EINA_TRUE /**< Return value to keep a callback */

#define ECORE_CALLBACK_PASS_ON EINA_TRUE /**< Return value to pass an event to the next handler */
#define ECORE_CALLBACK_DONE    EINA_FALSE /**< Return value to stop event handling */

/**
 * @typedef Ecore_Task_Cb Ecore_Task_Cb
 * @brief The boolean type for a callback that is run for a task (timer, idler, poller, animator, and so on).
 */
typedef Eina_Bool (*Ecore_Task_Cb)(void *data);

/**
 * @typedef Ecore_Cb Ecore_Cb
 * @brief Called as a hook when a certain point in the execution is reached.
 */
typedef void (*Ecore_Cb)(void *data);

/**
 * @typedef Ecore_Data_Cb Ecore_Data_Cb
 * @brief Called to return data to the main function.
 */
typedef void *(*Ecore_Data_Cb)(void *data);

/**
 * @typedef Ecore_Select_Function
 * @brief The integer type for a function that can be used to replace select() in the main loop.
 */
typedef int (*Ecore_Select_Function)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

/**
 * @brief Adds a function to be called by ecore_fork_reset().
 *
 * @details This queues @a func to be called (and passes @a data as its argument) when
 *          ecore_fork_reset() is called. This allows other libraries and subsystems
 *          to also reset their internal state after a fork.
 *
 * @param[in] func The function to be called
 * @param[in] data A data pointer to pass to the called function @a func
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE.
 *
 * @since 1.7
 * @since_tizen 2.3
 */
EAPI Eina_Bool ecore_fork_reset_callback_add(Ecore_Cb func, const void *data);

/**
 * @brief Removes the specified callback.
 *
 * @details This deletes the callback added by ecore_fork_reset_callback_add() using
 *          the function and data pointer to specify which callback to remove.
 *
 * @param[in] func The function to be called
 * @param[in] data A data pointer to pass to the called function @a func
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE.
 *
 * @since 1.7
 * @since_tizen 2.3
 */
EAPI Eina_Bool ecore_fork_reset_callback_del(Ecore_Cb func, const void *data);

/**
 * @brief Resets the ecore's internal state after a fork.
 *
 * @since 1.7
 * @since_tizen 2.3
 *
 * @remarks Ecore maintains the internal data that can be affected by the fork() system call,
 *          which creates a duplicate of the current process. This also duplicates
 *          file descriptors, which is problematic as these file descriptors still
 *          point to their original sources. This function makes ecore's reset internal
 *          state (e.g. pipes used for signalling between threads) so they function
 *          correctly afterwards.
 *
 * @remarks It is highly suggested that you call this function after any fork()
 *          system call inside the child process. If you intend to use ecore features
 *          after this point and not call exec() family functions. Not doing so
 *          causes a possible misbehaviour.
 */
EAPI void ecore_fork_reset(void);

/**
 * @brief Runs a single iteration of the main loop to process everything on the
 *       queue.
 *
 * @details It does everything that is already done inside an @c Ecore main loop,
 *          like checking for expired timers, idlers, etc. But it will do it
 *          only once and return, instead of keep watching for new events.
 *
 * @remarks DO NOT use this function unless you are the person God comes to ask
 *          for advice when He has trouble managing the Universe.
 *
 * @see ecore_main_loop_iterate_may_block()
 */
EAPI void ecore_main_loop_iterate(void);

/**
 * @brief Sets the function to use when monitoring multiple file descriptors,
 *        and waiting until one of more of the file descriptors before ready
 *        for some class of I/O operation.
 *
 * @remarks This function will be used instead of the system call select and
 *          could possible be used to integrate the Ecore event loop with an
 *          external event loop.
 *
 * @remarks you don't know how to use, don't even try to use it.
 *
 * @param func The function to be used.
 *
 * @see ecore_main_loop_select_func_get()
 */
EAPI void ecore_main_loop_select_func_set(Ecore_Select_Function func);

/**
 * @brief Gets the select function set by ecore_select_func_set(),
 *        or the native select function if none was set.
 *
 * @return The select function
 *
 * @see  ecore_main_loop_select_func_get()
 */
EAPI Ecore_Select_Function ecore_main_loop_select_func_get(void);

/**
 * @brief Request ecore to integrate GLib's main loop.
 *
 * @details This will add a small overhead during every main loop interaction
 *          by checking glib's default main context (used by its main loop). If
 *          it have events to be checked (timers, file descriptors or idlers),
 *          then these will be polled alongside with Ecore's own events, then
 *          dispatched before Ecore's. This is done by calling
 *          ecore_main_loop_select_func_set().
 *
 * @remarks This will cooperate with previously set
 *          ecore_main_loop_select_func_set() by calling the old function.
 *          Similarly, if you want to override
 *          ecore_main_loop_select_func_set() after main loop is integrated,
 *          call the new select function set by this call (get it by calling
 *          ecore_main_loop_select_func_get() right after
 *          ecore_main_loop_glib_integrate()).
 *
 * @remarks This is useful to use GMainLoop libraries, like GTK, GUPnP,
 *          LibSoup, GConf and more. Adobe Flash plugin and other plugins
 *          systems depend on this as well.
 *
 * @remarks Once initialized/integrated, it will be valid until Ecore is
 *          completely shut down.
 *
 * Example of use:
 * @code
 *
 * int main(void)
 * {
 *    ecore_init();
 *    ecore_main_loop_glib_integrate();
 *
 *    // some code here
 *
 *    ecore_main_loop_begin();
 *
 *    ecore_shutdown();
 *
 *    return 0;
 * }
 *
 * @endcode
 *
 * @remarks This is only available if Ecore was compiled with GLib support.
 * @remarks You don't need to call this function if Ecore was compiled with
 *          --with-glib=always.
 *
 * @return  #EINA_TRUE on success of @c EINA_FALSE if it failed,
 *          likely no GLib support in Ecore.
 */
EAPI Eina_Bool ecore_main_loop_glib_integrate(void);

/**
 * @brief Disable always integrating glib
 *
 * @remarks If ecore is compiled with --with-glib=always (to always call
 *          ecore_main_loop_glib_integrate() when ecore_init() is called),
 *          then calling this before calling ecore_init() will disable the
 *          integration. This is for apps that explicitly do not want this
 *          to happen for whatever reasons they may have.
 */
EAPI void ecore_main_loop_glib_always_integrate_disable(void);

/**
 * @brief Runs the application main loop.
 *
 * @details This function will not return until @ref ecore_main_loop_quit is
 *          called. It will check for expired timers, idlers, file descriptors
 *          being watched by fd handlers, etc. Once everything is done, before
 *          entering again on idle state, any callback set as @c Idle_Enterer
 *          will be called.
 *
 * @remarks Each main loop iteration is done by calling
 *          ecore_main_loop_iterate() internally.
 *
 * @remarks The polling (select) function used can be changed with
 *          ecore_main_loop_select_func_set().
 *
 * @remarks The function used to check for file descriptors, events, and that
 *          has a timeout for the timers can be changed using
 *          ecore_main_loop_select_func_set().
 */
EAPI void ecore_main_loop_begin(void);

/**
 * @brief Quits the main loop once all the events currently on the queue have
 *        been processed.
 *
 * @details This function returns immediately, but will mark the
 *          ecore_main_loop_begin() function to return at the end of the
 *          current main loop iteration.
 */
EAPI void ecore_main_loop_quit(void);

/**
 * @brief Called asynchronously in the main loop.
 *
 * @since 1.1.0
 *
 * @remarks For all calls that need to happen in the main loop (most EFL functions do),
 *          this helper function provides the infrastructure needed to do it safely
 *          by avoiding a dead lock, race condition, and by properly waking up the main loop.
 *
 * @remarks Remember that after the function call, you should never touch the @a data
 *          in the thread again, it is owned by the main loop and your callback should take
 *          care of freeing it, if necessary.
 *
 * @param callback The callback to call in the main loop
 * @param data The data to give to that call
 */
EAPI void ecore_main_loop_thread_safe_call_async(Ecore_Cb callback, void *data);

/**
 * @brief Called synchronously in the main loop.
 *
 * @since 1.1.0
 *
 * @remarks For all calls that need to happen in the main loop (most EFL functions do),
 *          this helper function provides the infrastructure needed to do it safely
 *          by avoiding a dead lock, race condition, and by properly waking up the main loop.
 *
 * @remarks Remember that this function blocks until the callback is executed in the
 *          main loop. It can take time and you have no guarantee about the timeline.
 *
 * @param callback The callback to call in the main loop
 * @param data The data to give to that call
 * @return The value returned by the callback in the main loop
 */
EAPI void *ecore_main_loop_thread_safe_call_sync(Ecore_Data_Cb callback, void *data);

/**
 * @brief Suspends the main loop in the know state.
 *
 * @details This function suspends the main loop in the know state. This lets you
 *          use any EFL call that you want after it returns. Be careful, the main loop
 *          is blocked until you call ecore_thread_main_loop_end(). This is
 *          the only way to achieve pseudo thread safety.
 *
 * @since 1.1.0
 *
 * @remarks Notice that till the main loop is blocked, the thread is blocked
 *          and there is no way around that.
 *
 * @remarks We still advise you, if possible, to use ecore_main_loop_thread_safe_call_async()
 *          as it does not block the thread or the main loop.
 *
 * @return The number of times ecore_thread_main_loop_begin() has been called
 *         in this thread, if the main loop is suspended correctly \n
 *         If not, it returns @c -1.
 */
EAPI int ecore_thread_main_loop_begin(void);

/**
 * @brief Unlocks the main loop.
 *
 * @since 1.1.0
 *
 * @remarks After a call to ecore_thread_main_loop_begin(), you need to absolutely
 *          call ecore_thread_main_loop_end(), or your application stays frozen.
 *
 * @return  The number of times ecore_thread_main_loop_end() needs to be called before
 *          the main loop is unlocked again \n
 *          @c -1  is retured if you are trying to unlock
 *          when there aren't enough calls to ecore_thread_main_loop_begin().
 *
 */
EAPI int ecore_thread_main_loop_end(void);

/**
 * @}
 */

/**
 * @defgroup Ecore_Event_Group Ecore Event
 * @ingroup Ecore_Main_Loop_Group
 *
 * @brief Ecore event are a helper to create events are being notified of events.
 *
 * Ecore events provide two main features that are of use to those using ecore:
 * creating events and being notified of events. Those two are usually used
 * in different contexts, creating events is mainly done by libraries wrapping
 * some system functionality while being notified of events is mainly a
 * necessity of applications.
 *
 * For a program to be notified of events it's interested in, it needs to have a
 * function to process the event and to register that function as the callback
 * to the event, that's all:
 * @code
 * ecore_event_handler_add(EVENT_TYPE, _my_event_handler, some_data);
 * ...
 * static Eina_Bool
 * _my_event_handler(void *data, int type, void *event)
 * {
 *    //Data is some_data
 *    //Event is provided by whoever created the event
 *    //Do really cool stuff with the event
 * }
 * @endcode
 *
 * One very important thing to note here is the @c EVENT_TYPE. To register a
 * handler for an event, you must know its type before hand. Ecore provides
 * the following events that are emitted in response to POSIX
 * signals(https://en.wikipedia.org/wiki/Signal_%28computing%29):
 * @li @b ECORE_EVENT_SIGNAL_USER
 * @li @b ECORE_EVENT_SIGNAL_HUP
 * @li @b ECORE_EVENT_SIGNAL_POWER
 * @li @b ECORE_EVENT_SIGNAL_EXIT
 *
 * Don't override these using the @c signal or @c sigaction calls.
 * These, however, aren't the only signals one can handle. Many
 * libraries(including ecore modules) have their own signals that can be
 * listened to and handled. To do that one only needs to know the type of the
 * event. This information can be found on the documentation of the library
 * emitting the signal. 
 * @internal
 * So, for example, for events related to windowing one
 * would use @ref Ecore_Evas_Group.
 *
 * Examples of libraries that integrate into ecore's main loop by providing
 * events are @ref Ecore_Con_Group, @ref Ecore_Evas_Group, and @ref
 * Ecore_Exe_Group, amongst others.
 * @endinternal
 *
 * This usage can be divided into two parts,
 * setup and adding events. The setup is very simple, all that needs to be done is
 * getting a type ID for the event:
 * @code
 * int MY_EV_TYPE = ecore_event_type_new();
 * @endcode
 * This variable should be declared in the header since it is needed by
 * anyone wishing to register a handler to your event.
 *
 * The complexity of adding an event to the queue depends on whether that
 * event sends or uses @a event, if it doesn't it is a one-liner:
 * @code
 * ecore_event_add(MY_EV_TYPE, NULL, NULL, NULL);
 * @endcode
 *			The usage when an @c event is needed is not that complex and can be
 *			seen in @ref ecore_event_add.
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

typedef struct _Ecore_Win32_Handler         Ecore_Win32_Handler;    /**< @internal @brief A handle for HANDLE handlers on Windows */
typedef struct _Ecore_Event_Handler         Ecore_Event_Handler;    /**< @brief A handle for an event handler */
typedef struct _Ecore_Event_Filter          Ecore_Event_Filter;    /**< @brief A handle for an event filter */
typedef struct _Ecore_Event                 Ecore_Event;    /**< @brief A handle for an event */
typedef struct _Ecore_Event_Signal_User     Ecore_Event_Signal_User;    /**< @brief User signal event */
typedef struct _Ecore_Event_Signal_Hup      Ecore_Event_Signal_Hup;    /**< @brief Hup signal event */
typedef struct _Ecore_Event_Signal_Exit     Ecore_Event_Signal_Exit;    /**< @brief Exit signal event */
typedef struct _Ecore_Event_Signal_Power    Ecore_Event_Signal_Power;    /**< @brief Power signal event */
typedef struct _Ecore_Event_Signal_Realtime Ecore_Event_Signal_Realtime;    /**< @brief Realtime signal event */

/**
 * @typedef Ecore_Filter_Cb
 * @brief The boolean type for a callback used for filtering events from the main loop.
 */
typedef Eina_Bool (*Ecore_Filter_Cb)(void *data, void *loop_data, int type, void *event);

/**
 * @typedef Ecore_End_Cb Ecore_End_Cb
 * @brief Called at the end of a function, usually for cleanup purposes.
 */
typedef void (*Ecore_End_Cb)(void *user_data, void *func_data);

/**
 * @typedef Ecore_Event_Handler_Cb Ecore_Event_Handler_Cb
 * @brief The boolean type used by the main loop to handle events of a specified type.
 */
typedef Eina_Bool (*Ecore_Event_Handler_Cb)(void *data, int type, void *event);

struct _Ecore_Event_Signal_User    /** User signal event */
{
   int       number;  /**< The signal number. Either 1 or 2 */
   void     *ext_data;  /**< Extension data - not used */

#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t data; /**< Signal info */
#endif
};

struct _Ecore_Event_Signal_Hup    /** Hup signal event */
{
   void     *ext_data;  /**< Extension data - not used */

#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t data; /**< Signal info */
#endif
};

struct _Ecore_Event_Signal_Exit    /** Exit request event */
{
   Eina_Bool interrupt : 1; /**< Set if the exit request is an interrupt signal*/
   Eina_Bool quit      : 1; /**< Set if the exit request is a quit signal */
   Eina_Bool terminate : 1; /**< Set if the exit request is a terminate signal */
   void     *ext_data; /**< Extension data - not used */

#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t data; /**< Signal info */
#endif
};

struct _Ecore_Event_Signal_Power    /** Power event */
{
   void     *ext_data;  /**< Extension data - not used */

#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t data; /**< Signal info */
#endif
};

struct _Ecore_Event_Signal_Realtime    /** Realtime event */
{
   int       num; /**< The realtime signal's number */

#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t data; /**< Signal info */
#endif
};

/**
 * @brief Adds an event handler.
 *
 * @details This adds an event handler to the list of handlers. This, on success, returns
 *          a handle to the event handler object that is created, that can be used
 *          later to remove the handler using ecore_event_handler_del(). The @a type
 *          parameter is the integer of the event type that triggers this callback
 *          to be called. The callback @a func is called when this event is processed
 *          and is passed the event type, a pointer to the private event
 *          structure that is specific to that event type, and a data pointer that is
 *          provided in this call as the @a data parameter.
 *
 * @since_tizen 2.3
 *
 * @remarks When the callback @a func is called, it must return @c 1 or @c 0. If it returns
 *          @c 1 (or @c ECORE_CALLBACK_PASS_ON), it keeps being called as per normal, for
 *          each handler set up for that event type. If it returns @c 0 (or
 *          @c ECORE_CALLBACK_DONE), it ceases processing handlers for that particular
 *          event, so all handlers set to handle that event type that have not already
 *          been called, are not called.
 *
 * @param[in] type The type of the event that this handler gets called for
 * @param[in] func The function to call when the event is found in the queue
 * @param[in] data A data pointer to pass to the called function @a func
 * @return A new Event handler,
 *         otherwise @c NULL on failure
 */
EAPI Ecore_Event_Handler *ecore_event_handler_add(int type, Ecore_Event_Handler_Cb func, const void *data);

/**
 * @brief Deletes an event handler.
 *
 * @details This deletes a specified event handler from the handler list. On success, this
 *          deletes the event handler and returns the pointer passed as @a data when the
 *          handler is added by ecore_event_handler_add(). On failure, @c NULL is
 *          returned. Once a handler is deleted it is no longer called.
 *
 * @since_tizen 2.3
 *
 * @param[in] event_handler The event handler handle to delete
 * @return The data passed to the handler
 */
EAPI void *ecore_event_handler_del(Ecore_Event_Handler *event_handler);

/**
 * @brief Adds an event to the event queue.
 *
 * @remarks If it succeeds, an event of type @a type is added to the queue for
 *          processing by event handlers added by ecore_event_handler_add(). The @a ev
 *          parameter is passed as the @a event parameter of the handler. When the
 *          event is no longer needed, @a func_free is called and it passes @a ev for
 *          cleaning up. If @a func_free is @c NULL, free() is called with the private
 *          structure pointer.
 *
 * @since_tizen 2.3
 *
 * @param[in] type The event type to add to the end of the event queue
 * @param[in] ev The data structure passed as @a event to event handlers
 * @param[in] func_free The function to be called to free @a ev
 * @param[in] data The data pointer to be passed to the free function
 * @return A Handle for that event on success, 
 *         otherwise @c NULL on failure
 */
EAPI Ecore_Event *ecore_event_add(int type, void *ev, Ecore_End_Cb func_free, void *data);

/**
 * @brief Deletes an event from the queue.
 *
 * @details This deletes the event @a event from the event queue, and returns the
 *          @a data parameter originally set when adding it using ecore_event_add(). This
 *          does not immediately call the free function, and it may be called later for
 *          cleanup, and so if the free function depends on the data pointer to work,
 *          you should defer cleaning of this till the free function is called later.
 *
 * @since_tizen 2.3
 *
 * @param[in] event The event handle to delete
 * @return The data pointer originally set for the event free function
 */
EAPI void *ecore_event_del(Ecore_Event *event);

/**
 * @brief Gets the data associated with an #Ecore_Event_Handler.
 *
 * @details This function returns the data previously associated with @a eh by
 *          ecore_event_handler_add().
 *
 * @since_tizen 2.3
 *
 * @param[in] eh The event handler
 * @return The data
 */
EAPI void *ecore_event_handler_data_get(Ecore_Event_Handler *eh);

/**
 * @brief Sets the data associated with an #Ecore_Event_Handler.
 *
 * @details This function sets @a data to @a eh and returns the old data pointer
 *          that had been previously associated with @a eh by ecore_event_handler_add().
 *
 * @since_tizen 2.3
 *
 * @param[in] eh The event handler
 * @param[in] data The data to associate
 * @return The previous data
 */
EAPI void *ecore_event_handler_data_set(Ecore_Event_Handler *eh, const void *data);

/**
 * @brief Allocates a new event type ID sensibly and returns the new ID.
 *
 * @details  This function allocates a new event type ID and returns it. Once an event
 *           type has been allocated it can never be de-allocated during the life of
 *           the program. There is no guarantee of the contents of this event ID, or how
 *           it is calculated, except that the ID is unique to the current instance
 *           of the process.
 *
 * @since_tizen 2.3
 *
 * @return A new event type ID
 *
 */
EAPI int ecore_event_type_new(void);

/**
 * @brief Adds a filter to the current event queue.
 *
 * @details This adds a callback to filter events from the event queue. Filters are called on
 *          the queue just before Event handler processing to try and remove redundant
 *          events. Just when processing is about to start @a func_start is called and
 *          passed the @a data pointer. The return value of this function is passed to
 *          @a func_filter as loop_data. @a func_filter is also passed @a data, the
 *          event type, and the event structure. If this @a func_filter returns
 *          @c EINA_FALSE, the event is removed from the queue. If it returns
 *          #EINA_TRUE, the event is kept. When processing is finished @a func_end is
 *          called and is passed the loop_data(returned by @a func_start) and @a data
 *          pointer to clean up.
 *
 * @since_tizen 2.3
 *
 * @param[in] func_start The function to call just before filtering and returning data
 * @param[in] func_filter The function to call on each event
 * @param[in] func_end The function to call after the queue has been filtered
 * @param[in] data The data to pass to the filter functions
 * @return A filter handle on success,
 *         otherwise @c NULL on failure
 *
 */
EAPI Ecore_Event_Filter *ecore_event_filter_add(Ecore_Data_Cb func_start, Ecore_Filter_Cb func_filter, Ecore_End_Cb func_end, const void *data);

/**
 * @brief Deletes an event filter.
 *
 * @details  This deletes a filter that has been added by its @a ef handle.
 *
 * @since_tizen 2.3
 *
 * @param[in] ef The event filter handle
 * @return The data set for the filter on success, 
 *         otherwise @c NULL
 */
EAPI void *ecore_event_filter_del(Ecore_Event_Filter *ef);

/**
 * @brief Returns the current event type being handled.
 *
 * @since_tizen 2.3
 *
 * @remarks If the program is currently inside an Ecore event handler callback this
 *          returns the type of the current event being processed.
 *
 *          This is useful when certain Ecore modules such as Ecore_Evas "swallow"
 *          events and not all the original information is passed on. In special cases,
 *          this extra information may be useful or needed and using this call can let
 *          the program know if the event type being handled is the one about which it wants to get more
 *          information.
 *
 * @return The current event type being handled if inside a handler callback,
 *         otherwise @c ECORE_EVENT_NONE
 */
EAPI int ecore_event_current_type_get(void);
/**
 * @brief Returns the current event type pointer handled.
 *
 * @since_tizen 2.3
 *
 * @remarks If the program is currently inside an Ecore event handler callback this
 *          returns the pointer of the current event being processed.
 *
 * @remarks This is useful when certain Ecore modules such as Ecore_Evas "swallow"
 *          events and not all the original information is passed on. In special cases,
 *          this extra information may be useful or needed and using this call can let
 *          the program access the event data if the type of the event is handled by
 *          the program.
 *
 * @return The current event pointer being handled if inside a handler callback,
 *         otherwise @c NULL
 */
EAPI void *ecore_event_current_event_get(void);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Ecore_Exe_Group Process Spawning
 * @ingroup Ecore_Main_Loop_Group
 *
 * This module is responsible for managing portable processes using Ecore.
 * With this module you're able to spawn processes and you can also pause and
 * quit your spawned processes.
 * An interaction between your process and those spawned is possible
 * using pipes or signals.
 *
 * @{
 */

/** Inherit priority from the parent process */
#define ECORE_EXE_PRIORITY_INHERIT 9999

EAPI extern int ECORE_EXE_EVENT_ADD;     /**< @brief A child process has been added */
EAPI extern int ECORE_EXE_EVENT_DEL;     /**< @brief A child process has been deleted (it exited, naming is consistent with the rest of ecore) */
EAPI extern int ECORE_EXE_EVENT_DATA;    /**< @brief Data from a child process */
EAPI extern int ECORE_EXE_EVENT_ERROR;   /**< @brief Errors from a child process */

/**
 * @internal
 * @enum _Ecore_Exe_Flags
 * @brief Enumeration that defines the flags for executing a child with its stdin and/or stdout piped back.
 */
enum _Ecore_Exe_Flags    /* Flags for executing a child with its stdin and/or stdout piped back */
{
   ECORE_EXE_NONE = 0, /**< No exe flags at all */
   ECORE_EXE_PIPE_READ = 1, /**< Exe Pipe Read mask */
   ECORE_EXE_PIPE_WRITE = 2, /**< Exe Pipe Write mask */
   ECORE_EXE_PIPE_ERROR = 4, /**< Exe Pipe error mask */
   ECORE_EXE_PIPE_READ_LINE_BUFFERED = 8, /**< Reads are buffered till a new line and 1 line is split per Ecore_Exe_Event_Data_Line */
   ECORE_EXE_PIPE_ERROR_LINE_BUFFERED = 16, /**< Errors are buffered till a new line and 1 line is split per Ecore_Exe_Event_Data_Line */
   ECORE_EXE_PIPE_AUTO = 32, /**< stdout and stderr are buffered automatically */
   ECORE_EXE_RESPAWN = 64, /**< FIXME: Exe is restarted if it dies */
   ECORE_EXE_USE_SH = 128, /**< Use /bin/sh to run the command */
   ECORE_EXE_NOT_LEADER = 256, /**< Do not use setsid() to make the executed process its own session leader */
   ECORE_EXE_TERM_WITH_PARENT = 512 /**< Makes a child receive SIGTERM when the parent dies */
};
typedef enum _Ecore_Exe_Flags Ecore_Exe_Flags;

/**
 * @internal
 * @enum _Ecore_Exe_Win32_Priority
 * @brief Enumeration that defines the priority of the proccess.
 */
enum _Ecore_Exe_Win32_Priority
{
   ECORE_EXE_WIN32_PRIORITY_IDLE, /**< Idle priority, for monitoring the system */
   ECORE_EXE_WIN32_PRIORITY_BELOW_NORMAL, /**< Below default priority */
   ECORE_EXE_WIN32_PRIORITY_NORMAL, /**< Default priority */
   ECORE_EXE_WIN32_PRIORITY_ABOVE_NORMAL, /**< Above default priority */
   ECORE_EXE_WIN32_PRIORITY_HIGH, /**< High priority, use with care as other threads in the system do not get processor time */
   ECORE_EXE_WIN32_PRIORITY_REALTIME     /**< Realtime priority, should be almost never used as it can interrupt system threads that manage mouse input, keyboard input, and background disk flushing */
};
typedef enum _Ecore_Exe_Win32_Priority Ecore_Exe_Win32_Priority;

typedef struct _Ecore_Exe              Ecore_Exe; /**< @brief A handle for spawned processes */

/**
 * @typedef Ecore_Exe_Cb Ecore_Exe_Cb
 * @brief Called to run with the associated @ref Ecore_Exe, usually
 *        for cleanup purposes.
 */
typedef void                            (*Ecore_Exe_Cb)(void *data, const Ecore_Exe *exe);

typedef struct _Ecore_Exe_Event_Add       Ecore_Exe_Event_Add; /**< @brief Spawned Exe add event */
typedef struct _Ecore_Exe_Event_Del       Ecore_Exe_Event_Del; /**< @brief Spawned Exe exit event */
typedef struct _Ecore_Exe_Event_Data_Line Ecore_Exe_Event_Data_Line; /**< @brief Lines from a child process */
typedef struct _Ecore_Exe_Event_Data      Ecore_Exe_Event_Data; /**< @brief Data from a child process */

/**
* @internal
* @brief Structure of Ecore Exe Event Add
*/
struct _Ecore_Exe_Event_Add    /** Process add event */
{
   Ecore_Exe *exe; /**< The handle to the added process */
   void      *ext_data; /**< Extension data - not used */
};

struct _Ecore_Exe_Event_Del    /** Process exit event */
{
   pid_t      pid; /**< The process ID of the process that exited */
   int        exit_code; /**< The exit code of the process */
   Ecore_Exe *exe; /**< The handle to the exited process, otherwise @c NULL if not found */
   int        exit_signal; /** < The signal that caused the process to exit */
   Eina_Bool  exited    : 1; /** < Set to @c 1 if the process exited on its own */
   Eina_Bool  signalled : 1; /** < Set to @c 1 if the process exited due to an uncaught signal */
   void      *ext_data; /**< Extension data - not used */
#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t  data; /**< Signal info */
#endif
};

struct _Ecore_Exe_Event_Data_Line    /**< Lines from a child process */
{
   char *line; /**< The bytes of a line of buffered data */
   int   size; /**< The size of the line buffer in bytes */
};

struct _Ecore_Exe_Event_Data    /** Data from a child process event */
{
   Ecore_Exe                 *exe; /**< The handle to the process */
   void                      *data; /**< The raw binary data from the child process that is received */
   int                        size; /**< The size of this data in bytes */
   Ecore_Exe_Event_Data_Line *lines; /**< An array of line data if line buffered, the last one has its line member set to @c NULL */
};

EAPI void ecore_exe_run_priority_set(int pri);
EAPI int ecore_exe_run_priority_get(void);
EAPI Ecore_Exe *ecore_exe_run(const char *exe_cmd, const void *data);
EAPI Ecore_Exe *ecore_exe_pipe_run(const char *exe_cmd, Ecore_Exe_Flags flags, const void *data);
EAPI void ecore_exe_callback_pre_free_set(Ecore_Exe *exe, Ecore_Exe_Cb func);
EAPI Eina_Bool ecore_exe_send(Ecore_Exe *exe, const void *data, int size);
EAPI void ecore_exe_close_stdin(Ecore_Exe *exe);
EAPI void ecore_exe_auto_limits_set(Ecore_Exe *exe, int start_bytes, int end_bytes, int start_lines, int end_lines);
EAPI Ecore_Exe_Event_Data *ecore_exe_event_data_get(Ecore_Exe *exe, Ecore_Exe_Flags flags);
EAPI void ecore_exe_event_data_free(Ecore_Exe_Event_Data *data);
EAPI void *ecore_exe_free(Ecore_Exe *exe);
EAPI pid_t ecore_exe_pid_get(const Ecore_Exe *exe);
EAPI void ecore_exe_tag_set(Ecore_Exe *exe, const char *tag);
EAPI const char *ecore_exe_tag_get(const Ecore_Exe *exe);
EAPI const char *ecore_exe_cmd_get(const Ecore_Exe *exe);
EAPI void *ecore_exe_data_get(const Ecore_Exe *exe);
EAPI void *ecore_exe_data_set(Ecore_Exe *exe, void *data);
EAPI Ecore_Exe_Flags ecore_exe_flags_get(const Ecore_Exe *exe);
EAPI void ecore_exe_pause(Ecore_Exe *exe);
EAPI void ecore_exe_continue(Ecore_Exe *exe);
EAPI void ecore_exe_interrupt(Ecore_Exe *exe);
EAPI void ecore_exe_quit(Ecore_Exe *exe);
EAPI void ecore_exe_terminate(Ecore_Exe *exe);
EAPI void ecore_exe_kill(Ecore_Exe *exe);
EAPI void ecore_exe_signal(Ecore_Exe *exe, int num);
EAPI void ecore_exe_hup(Ecore_Exe *exe);

/**
 * @}
 */

/**
 * @defgroup Ecore_FD_Handler_Group Ecore File Descriptor Handling
 * @ingroup Ecore_Main_Loop_Group
 *
 * @brief This group discusses functions that deal with file descriptor handlers.
 *
 * File descriptor handlers facilitate reading, writing, and checking for errors
 * without blocking the program or doing expensive pooling. This can be used to
 * monitor a socket, pipe, or some other stream for which an FD can be present.
 *
 * File descriptor handlers can't be used to monitor file creation,
 * modification, or deletion, 
 * @internal
 * see @ref Ecore_File_Group for this.
 * @endinternal
 *
 * One common FD to be monitored is the standard input(stdin), monitoring it for
 * reading requires a single call:
 * @code
 * static Eina_Bool
 * _my_cb_func(void *data, Ecore_Fd_Handler *handler)
 * {
 *    char c;
 *    scanf("%c", &c); //Guaranteed not to block
 *    ... do stuff with c ...
 * }
 * ecore_main_fd_handler_add(STDIN_FILENO, ECORE_FD_READ, _my_cb_func, NULL, NULL, NULL);
 * @endcode
 *
 * When using a socket, pipe, or some other stream it's important to remember that
 * errors may occur and we must monitor not only for reading/writing, but also
 * for errors using the @ref ECORE_FD_ERROR flag.
 *
 * @{
 */

/**
 * @brief typedef to struct _Ecore_Fd_Handler
 */
typedef struct _Ecore_Fd_Handler Ecore_Fd_Handler; /**< A handle for FD handlers */

/**
 * @enum _Ecore_Fd_Handler_Flags
 * @brief Enumeration that defines the handler flags to monitor the file descriptor for: reading, writing, or error.
 */
enum _Ecore_Fd_Handler_Flags
{
   ECORE_FD_READ = 1, /**< FD Read mask */
   ECORE_FD_WRITE = 2, /**< FD Write mask */
   ECORE_FD_ERROR = 4 /**< FD Error mask */
};

/**
 * @brief typedef to enum _Ecore_Fd_Handler_Flags
 */
typedef enum _Ecore_Fd_Handler_Flags Ecore_Fd_Handler_Flags;

/**
 * @typedef Ecore_Fd_Cb Ecore_Fd_Cb
 * @brief The boolean type for a callback used by an @ref Ecore_Fd_Handler.
 */
typedef Eina_Bool (*Ecore_Fd_Cb)(void *data, Ecore_Fd_Handler *fd_handler);

/**
 * @typedef Ecore_Fd_Prep_Cb Ecore_Fd_Prep_Cb
 * @brief Called to be used by an @ref Ecore_Fd_Handler.
 */
typedef void (*Ecore_Fd_Prep_Cb)(void *data, Ecore_Fd_Handler *fd_handler);

/**
 * @internal
 * @typedef Ecore_Win32_Handle_Cb Ecore_Win32_Handle_Cb
 * @brief The boolean type for a callback used by an @ref Ecore_Win32_Handler.
 */
typedef Eina_Bool (*Ecore_Win32_Handle_Cb)(void *data, Ecore_Win32_Handler *wh);

/**
 * @brief Adds a callback for activity on the given file descriptor.
 *
 * @since_tizen 2.3
 *
 * @remarks @a func is called during the execution of @ref Ecore_Main_Loop_Page
 *          when the file descriptor is available for reading, writing, or there has been
 *          an error(depending on the given @a flags).
 *
 * @remarks When @a func returns @c ECORE_CALLBACK_CANCEL, it indicates that the
 *          handler should be marked for deletion (identical to calling @ref
 *          ecore_main_fd_handler_del).
 *
 * @remarks @a buf_func is meant for @b internal use only and should be @b
 *          avoided.
 *
 * @remarks The return value of @a buf_func has a different meaning, when it returns
 *          @c ECORE_CALLBACK_CANCEL, it indicates that @a func @b shouldn't be called, and
 *          when it returns @c ECORE_CALLBACK_RENEW it indicates @a func should be called.
 *          The return value of @a buf_func does not cause the FD handler to get deleted.
 *
 * @remarks @a buf_func is called during event loop handling to check if data that has
 *          been read from the file descriptor is in a buffer and is available to read.
 *          Some systems, notably xlib, handle their own buffering, and would otherwise
 *          not work with select(). These systems should use a @a buf_func. This is the
 *          most annoying hack, only ecore_x uses it, so refer to that for an example.
 *
 * @remarks This function should @b not be used for monitoring "normal" files, like text files.
 *
 * @param[in] fd The file descriptor to watch
 * @param[in] flags The flags to monitor it, for reading use @c ECORE_FD_READ, for writing use @c
 *              ECORE_FD_WRITE, and for error use @c ECORE_FD_ERROR \n
 *              Values by |(ored).
 * @param[in] func The callback function
 * @param[in] data The data to pass to the callback
 * @param[in] buf_func The function to call to check if any data has been buffered
 *                 and already read from the fd \n
 *                 May be @c NULL.
 * @param[in] buf_data The data to pass to the @a buf_func function
 * @return An fd handler handle on success,
 *         otherwise @c NULL on failure
 *
 */
EAPI Ecore_Fd_Handler *ecore_main_fd_handler_add(int fd, Ecore_Fd_Handler_Flags flags, Ecore_Fd_Cb func, const void *data, Ecore_Fd_Cb buf_func, const void *buf_data);

/**
 * @brief Adds a callback for activity on the given file descriptor.
 *
 * @since 1.7
 *
 * @since_tizen 2.3
 *
 * @remarks This function is identical to ecore_main_fd_handler_add, except that it supports regular files.
 *
 * @remarks This function should ONLY be called with @c ECORE_FD_ERROR, otherwise it calls the fd
 *          handler constantly.
 * @remarks Do not use this function unless you know what you are doing.
 *
 * @param[in] fd The file descriptor to watch
 * @param[in] flags The flags to monitor it, for reading use @c ECORE_FD_READ, for writing use @c
 *              ECORE_FD_WRITE, and for error use @c ECORE_FD_ERROR \n
 *              Values by |(ored).
 * @param[in] func The callback function
 * @param[in] data The data to pass to the callback
 * @param[in] buf_func The function to call to check if any data has been buffered
 *                 and already read from the fd \n
 *                 May be @c NULL.
 * @param[in] buf_data The data to pass to the @a buf_func function.
 * @return An fd handler handle on success,
 *         otherwise @c NULL on failure
 */
EAPI Ecore_Fd_Handler *ecore_main_fd_handler_file_add(int fd, Ecore_Fd_Handler_Flags flags, Ecore_Fd_Cb func, const void *data, Ecore_Fd_Cb buf_func, const void *buf_data);

/**
 * @brief Sets the prepare callback with data for a given #Ecore_Fd_Handler.
 *
 * @since_tizen 2.3
 *
 * @remarks This function is called prior to any fd handler's callback function
 *          (even the other fd handlers), before entering the main loop select function.
 *
 * @remarks Once a prepare callback is set for an fd handler, it cannot be changed.
 *          You need to delete the fd handler and create a new one, to set another
 *          callback.
 *
 * @remarks You probably don't need this function. It is only necessary for very
 *          uncommon cases that need special behavior.
 *
 * @param[in] fd_handler The fd handler
 * @param[in] func The prep function
 * @param[in] data The data to pass to the prep function
 */
EAPI void ecore_main_fd_handler_prepare_callback_set(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Prep_Cb func, const void *data);

/**
 * @brief Marks an FD handler for deletion.
 *
 * @since_tizen 2.3
 *
 * @details This function marks an fd handler to be deleted during an iteration of the
 *          main loop. It does NOT close the associated fd.
 *
 * @remarks If the underlying fd is already closed ecore may complain if the
 *          main loop is using epoll internally, and also in some rare cases this may
 *          cause crashes and instability. Remember to delete your fd handlers before the
 *          fds they listen to are closed.
 *
 * @param[in] fd_handler The fd handler
 * @return The data pointer set using @ref ecore_main_fd_handler_add, for
 *         @a fd_handler on success,
 *         otherwise @c NULL on failure
 */
EAPI void *ecore_main_fd_handler_del(Ecore_Fd_Handler *fd_handler);

/**
 * @brief Retrieves the file descriptor that the given handler is handling.
 *
 * @since_tizen 2.3
 *
 * @param[in] fd_handler The given fd handler
 * @return The file descriptor that the handler is watching
 */
EAPI int ecore_main_fd_handler_fd_get(Ecore_Fd_Handler *fd_handler);

/**
 * @brief Gets which flags are active on an FD handler.
 *
 * @since_tizen 2.3
 *
 * @param[in] fd_handler The given fd handler
 * @param[in] flags The flags, @c ECORE_FD_READ, @c ECORE_FD_WRITE, or
 *        @c ECORE_FD_ERROR to query
 * @return #EINA_TRUE if any of the given flags are active,
 *         otherwise @c EINA_FALSE
 */
EAPI Eina_Bool ecore_main_fd_handler_active_get(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags);

/**
 * @brief Sets what active streams the given FD handler should be monitoring.
 *
 * @since_tizen 2.3
 *
 * @param[in] fd_handler The given fd handler
 * @param[in] flags The flags to be watching
 */
EAPI void ecore_main_fd_handler_active_set(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags);

/**
 * @internal
 */
EAPI Ecore_Win32_Handler *ecore_main_win32_handler_add(void *h, Ecore_Win32_Handle_Cb func, const void *data);

/**
 * @internal
 */
EAPI void *ecore_main_win32_handler_del(Ecore_Win32_Handler *win32_handler);

/**
 * @}
 */

/**
 * @defgroup Ecore_Time_Group Ecore Time
 * @ingroup Ecore_Main_Loop_Group
 *
 * @brief This group discusses the functions to retrieve time in a given format.
 *
 * @{
 */

/**
 * @brief Retrieves the current system time as a floating point value in seconds.
 *
 * @details This uses a monotonic clock and thus never goes back in time while
 *          machine is live (even if user changes time or timezone changes,
 *          however it may be reset whenever the machine is restarted).
 *
 * @since_tizen 2.3
 *
 * @return The number of seconds. Start time is not defined (it may be
 *         when the machine was booted, unix time, etc), all it is
 *         defined is that it never goes backwards (unless you got big critical
 *         messages when the application started).
 *
 * @see ecore_loop_time_get().
 * @see ecore_time_unix_get().
 */
EAPI double ecore_time_get(void);

/**
 * @brief Retrieves the current UNIX time as a floating point value in seconds.
 *
 * @since_tizen 2.3
 *
 * @return  The number of seconds since 12.00AM 1st January 1970.
 *
 * @see ecore_time_get().
 * @see ecore_loop_time_get().
 */
EAPI double ecore_time_unix_get(void);

/**
 * @brief Retrieves the time at which the last loop stopped waiting for
 *        timeouts or events.
 *
 * @since_tizen 2.3
 *
 * @remarks This gets the time that the main loop ceased waiting for timouts
 *          and/or events to come in or for signals or any other interrupt
 *          source. This should be considered a reference point for all time
 *          based activity that should calculate its timepoint from the return
 *          of ecore_loop_time_get(). Use this UNLESS you absolutely must get
 *          the current actual timepoint - then use ecore_time_get().
 *          Note that this time is meant to be used as relative to other times
 *          obtained on this run. If you need absolute time references, use
 *          ecore_time_unix_get() instead.
 *
 * @remarks This function can be called before any loop has ever been run, but
 *          either ecore_init() or ecore_time_get() must have been called once.
 *
 * @return The number of seconds. Start time is not defined (it may be
 *         when the machine was booted, unix time, etc), all it is
 *         defined is that it never goes backwards (unless you got big critical
 *         messages when the application started).
 */
EAPI double ecore_loop_time_get(void);

/**
 * @}
 */

/**
 * @defgroup Ecore_Thread_Group Ecore Thread
 * @ingroup Ecore_Main_Loop_Group
 *
 * @brief Facilities to run heavy tasks in different threads to avoid blocking
 *        the main loop.
 *
 * The EFL is, for the most part, not thread safe. This means that if you
 * have some task running in another thread and you have, for example, an
 * Evas object to show the status progress of this task, you cannot update
 * the object from within the thread. This can only be done from the main
 * thread, the one running the main loop. This problem can be solved
 * by running a thread that sends messages to the main one using an
 * @ref Ecore_Pipe_Group "Ecore_Pipe", but when you need to handle other
 * things like cancelling the thread, your code grows in complexity and gets
 * much harder to maintain.
 *
 * Ecore Thread is here to solve that problem. It is not a simple wrapper
 * around standard POSIX threads (or an equivalent in other systems) and
 * it's not meant to be used to run parallel tasks throughout the entire
 * duration of the program, especially when these tasks are performance
 * critical, as Ecore manages these tasks using a pool of threads based on
 * system configuration.
 *
 * What Ecore Thread does is it makes it a lot easier to dispatch a worker
 * function to perform some heavy tasks and then get the result once it
 * completes, without blocking the application's UI. In addition, cancelling
 * and rescheduling comes practically for free and the developer need not
 * worry about how many threads are launched, since Ecore schedules
 * them according to the number of processors the system has and the maximum
 * amount of concurrent threads set for the application.
 *
 * At the system level, Ecore starts a new thread on an as-needed basis
 * until the maximum set is reached. When no more threads can be launched,
 * new worker functions are queued in a waiting list until a thread
 * becomes available. This way, system threads are shared throughout
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
 * running in the thread. This same handler is the one received
 * on the @c end, @c cancel, and @c feedback callbacks.
 *
 * The worker function, that's the one running in the thread, also receives
 * an ::Ecore_Thread handler that can be used with ecore_thread_cancel() and
 * ecore_thread_check(), sharing the flag with the main loop. But this
 * handler is also associated with the thread where the function is running.
 * This has strong implications when working with thread local data.
 *
 * There are two kinds of worker threads that Ecore handles: simple or short,
 * workers, and feedback workers.
 *
 * The first kind is for simple functions that perform a
 * usually small but time consuming task. Ecore runs this function in
 * a thread as soon as one becomes available and notifies the calling user of
 * its completion once the task is done.
 *
 * The following image shows the flow of a program running four tasks on
 * a pool of two threads.
 *
 * @image html ecore_thread.png
 * @image rtf ecore_thread.png
 * @image latex ecore_thread.eps "ecore thread" width=\textwidth
 *
 * For larger tasks that may require continuous communication with the main
 * program, the feedback workers provide the same functionality plus a way
 * for the function running in the thread to send messages to the main
 * thread.
 *
 * The next diagram omits some details shown in the previous one regarding
 * how threads are spawned and tasks are queued, but illustrates how feedback
 * jobs communicate with the main loop and the special case of threads
 * running out of the pool.
 *
 * @image html ecore_thread_feedback.png
 * @image rtf ecore_thread_feedback.png
 * @image latex ecore_thread_feedback.eps "ecore thread feedback" width=\textwidth
 *
 * @{
 */

typedef struct _Ecore_Thread Ecore_Thread; /**< @brief A handle for threaded jobs */

/**
 * @typedef Ecore_Thread_Cb Ecore_Thread_Cb
 * @brief Called to be used by Ecore_Thread helper.
 */
typedef void (*Ecore_Thread_Cb)(void *data, Ecore_Thread *thread);
/**
 * @typedef Ecore_Thread_Notify_Cb Ecore_Thread_Notify_Cb
 * @brief Called to be used by the main loop to receive data sent by an
 *        @ref Ecore_Thread_Group.
 */
typedef void (*Ecore_Thread_Notify_Cb)(void *data, Ecore_Thread *thread, void *msg_data);

/**
 * @brief Schedules a task to run in a parallel thread to avoid locking the main loop.
 *
 * @details This function tries to create a new thread to run @a func_blocking in,
 *          or if the maximum number of concurrent threads has been reached it
 *          adds it to the pending list, where it waits until a thread becomes
 *          available. The return value is an ::Ecore_Thread handle that can
 *          be used to cancel the thread before its completion.
 *
 * @since_tizen 2.3
 *
 * @remarks This function should always return immediately, but in the rare
 *          case that Ecore is built with no thread support, @a func_blocking is
 *          be called here, actually blocking the main loop.
 *
 * @remarks Once a thread becomes available, @a func_blocking is run in it until
 *          it finishes, then @a func_end is called from the thread containing the
 *          main loop to inform the user of its completion. While in @a func_blocking,
 *          no functions from the EFL can be used, except for those from Eina that are
 *          marked to be thread-safe. Even for the latter, caution needs to be taken
 *          if the data is shared across several threads.
 *
 * @remarks @a func_end is called from the main thread when @a func_blocking ends,
 *          so here it's safe to use anything from the EFL freely.
 *
 * @remarks The thread can also be cancelled before its completion by calling
 *          ecore_thread_cancel(), either from the main thread or @a func_blocking.
 *          In this case, @a func_cancel is called, also from the main thread
 *          to inform of this happening. If the thread could not be created, this
 *          function is called and its @c thread parameter is @c NULL. It's
 *          also safe to call any EFL function here, as it is running in the
 *          main thread.
 *
 * @remarks Inside @a func_blocking, it's possible to call ecore_thread_reschedule()
 *          to tell Ecore that this function should be called again.
 *
 * @remarks Be aware that no assumptions can be made about the order in which the
 *          @a func_end callbacks for each task are called. Once the function is
 *          running in a different thread, it's the OS that handles its running
 *          schedule, and different functions may take longer to finish than others.
 *          Also remember that just starting several tasks together doesn't mean they
 *          are going to run at the same time. Ecore schedules them based on the
 *          number of threads available for the particular system it's running in,
 *          so some of the jobs started may be waiting until another one finishes
 *          before it can execute its own @a func_blocking.
 *
 * @param[in] func_blocking The function that should run in another thread
 * @param[in] func_end The function to call from the main loop when @a func_blocking
 *                 completes its task successfully (may be @c NULL)
 * @param[in] func_cancel The function to call from the main loop if the thread running
 *                    @a func_blocking is cancelled or fails to start (may be @c NULL)
 * @param[in] data The user context data to pass to all callbacks
 * @return A new thread handler,
 *         otherwise @c NULL on failure
 *
 * @see ecore_thread_feedback_run()
 * @see ecore_thread_cancel()
 * @see ecore_thread_reschedule()
 * @see ecore_thread_max_set()
 */
EAPI Ecore_Thread *ecore_thread_run(Ecore_Thread_Cb func_blocking, Ecore_Thread_Cb func_end, Ecore_Thread_Cb func_cancel, const void *data);

/**
 * @brief Launches a thread to run a task that can talk back to the main thread.
 *
 * @since_tizen 2.3
 *
 * @remarks The difference in the above is that ecore_thread_run() is meant for
 *          tasks that don't need to communicate anything until they finish, while
 *          this function is provided with a new callback, @a func_notify, that is
 *          called from the main thread for every message sent from @a func_heavy
 *          with ecore_thread_feedback().
 *
 * @remarks Like with ecore_thread_run(), a new thread is launched to run
 *          @a func_heavy unless the maximum number of simultaneous threads has been
 *          reached, in which case the function is scheduled to run whenever a
 *          running task ends and a thread becomes free. But if @a try_no_queue is
 *          set, Ecore first tries to launch a thread outside of the pool to run
 *          the task. If it fails, it reverts to the normal behaviour of using a
 *          thread from the pool as if @a try_no_queue had not been set.
 *
 * @remarks Keep in mind that Ecore handles the thread pool based on the number of
 *          CPUs available, but running a thread outside of the pool doesn't count for
 *          this, so having too many of them may have drastic effects over the
 *          program's performance.
 *
 * @remarks See ecore_thread_run() for a general description of this function.
 *
 * @param[in] func_heavy The function that should run in another thread
 * @param[in] func_notify the function that receives the data sent from the thread
 * @param[in] func_end The function to call from the main loop when @a func_heavy
 *                 completes its task successfully
 * @param[in] func_cancel The function to call from the main loop if the thread running
 *                    @a func_heavy is cancelled or fails to start
 * @param[in] data The user context data to pass to all callbacks
 * @param[in] try_no_queue The boolean value that indicates whether to run outside the thread pool
 * @return A new thread handler,
 *         otherwise @c NULL on failure
 *
 * @see ecore_thread_feedback()
 * @see ecore_thread_run()
 * @see ecore_thread_cancel()
 * @see ecore_thread_reschedule()
 * @see ecore_thread_max_set()
 */
EAPI Ecore_Thread *ecore_thread_feedback_run(Ecore_Thread_Cb func_heavy, Ecore_Thread_Notify_Cb func_notify,
                                             Ecore_Thread_Cb func_end, Ecore_Thread_Cb func_cancel,
                                             const void *data, Eina_Bool try_no_queue);

/**
 * @brief Cancels a running thread.
 *
 * @details This function cancels a running thread. If @a thread can be immediately
 *          cancelled (its still pending execution after creation or rescheduling),
 *          then the @a cancel callback is called, @a thread is freed and
 *          the function returns #EINA_TRUE.
 *
 * @since_tizen 2.3
 *
 * @remarks If the thread is already running, then this function returns @c EINA_FALSE
 *          after marking the @a thread as pending cancellation. For the thread to
 *          actually be terminated, it needs to return from the user function back
 *          into Ecore control. This can happen in several ways:
 * @li The function ends and returns normally. If it hadn't been cancelled,
 *     @a func_end would be called here, but instead @a func_cancel happens.
 * @li The function returns after requesting to be rescheduled with
 *     ecore_thread_reschedule().
 * @li The function is prepared to leave early by checking if
 *     ecore_thread_check() returns #EINA_TRUE.
 *
 * @remarks The user function can cancel itself by calling ecore_thread_cancel(), but
 *          it should always use the ::Ecore_Thread handle passed to it and never
 *          share it with the main loop thread by means of shared user data or in any
 *          other way.
 *
 * @remarks @a thread is freed and should not be used again if this function
 *          returns #EINA_TRUE or after the @a func_cancel callback returns.
 *
 * @remarks This function can be called both in the main loop and in the running thread.
 *
 * @param[in] thread The thread to cancel
 * @return #EINA_TRUE if the thread has been cancelled,
 *         otherwise @c EINA_FALSE if it is pending
 *
 * @see ecore_thread_check()
 */
EAPI Eina_Bool ecore_thread_cancel(Ecore_Thread *thread);

/**
 * @brief Checks whether a thread is in pending cancellation.
 *
 * @details This function can be called both in the main loop and in the running thread.
 *
 * @since_tizen 2.3
 *
 * @remarks When ecore_thread_cancel() is called on an already running task, the
 *          thread is marked as pending cancellation. This function returns #EINA_TRUE
 *          if this mark is set for the given @a thread and can be used from the
 *          main loop thread to check if a still active thread has been cancelled,
 *          or from the user function running in the thread to check if it should
 *          stop doing what it's doing and return early, effectively cancelling the
 *          task.
 *
 * @param[in] thread The thread to test
 * @return #EINA_TRUE if the thread is in pending cancellation,
 *         otherwise @c EINA_FALSE if it is not
 *
 * @see ecore_thread_cancel()
 */
EAPI Eina_Bool ecore_thread_check(Ecore_Thread *thread);

/**
 * @brief Sends data from the worker thread to the main loop.
 *
 * @since_tizen 2.3
 *
 * @remarks You should use this function only in the @a func_heavy call.
 *
 * @remarks Only the address to @a msg_data is sent and once this function
 *          returns #EINA_TRUE, the job running in the thread should never touch the
 *          contents of it again. The data sent should be malloc()'ed or something
 *          similar, as long as it's not the memory that is local to the thread that risks being
 *          overwritten or deleted once it goes out of scope or the thread finishes.
 *
 * @remarks Care must be taken that @a msg_data is properly freed in the @a func_notify
 *          callback set when creating the thread.
 *
 * @param[in] thread The current ::Ecore_Thread context to send data from
 * @param[in] msg_data The data to be transmitted to the main loop
 * @return #EINA_TRUE if @a msg_data is successfully sent to the main loop,
 *         otherwise @c EINA_FALSE if anything goes wrong
 *
 * @see ecore_thread_feedback_run()
 */
EAPI Eina_Bool ecore_thread_feedback(Ecore_Thread *thread, const void *msg_data);

/**
 * @brief Asks for the function in the thread to be called again at a later period.
 *
 * @since_tizen 2.3
 *
 * @remarks This function should be called only from the function represented
 *          by @a thread.
 *
 *          Calling this function marks the thread for a reschedule, so as soon
 *          as it returns, it is added to the end of the list of pending tasks.
 *          If no other tasks are waiting or there are sufficient threads available,
 *          the rescheduled task is launched again immediately.
 *
 *          This should never return @c EINA_FALSE, unless it is called from the wrong
 *          thread or with the wrong arguments.
 *
 * @remarks The @a func_end callback set when the thread is created is not
 *          called until the function in the thread returns without being rescheduled.
 *          Similarly, if the @a thread is cancelled, the reschedule does not take
 *          effect.
 *
 * @param[in] thread The current ::Ecore_Thread context to reschedule
 * @return #EINA_TRUE if the task is successfully rescheduled,
 *         otherwise @c EINA_FALSE if anything goes wrong
 *
 */
EAPI Eina_Bool ecore_thread_reschedule(Ecore_Thread *thread);

/**
 * @brief Gets the number of active threads running jobs.
 *
 * @details This returns the number of threads currently running jobs of any type
 *          through the Ecore_Thread API.
 *
 * @since_tizen 2.3
 *
 * @remarks Jobs started through the ecore_thread_feedback_run() function with
 *          the @a try_no_queue parameter set to #EINA_TRUE are not accounted for
 *          in the return of this function unless the thread creation fails and it
 *          falls back to using one from the pool.
 *
 * @return The number of active threads running jobs
 *
 */
EAPI int ecore_thread_active_get(void);

/**
 * @brief Gets the number of short jobs waiting for a thread to run.
 *
 * @details This returns the number of tasks started with ecore_thread_run() that are
 *          pending and waiting for a thread to become available to run them.
 *
 * @since_tizen 2.3
 *
 * @return The number of pending threads running "short" jobs
 *
 */
EAPI int ecore_thread_pending_get(void);

/**
 * @brief Gets the number of feedback jobs waiting for a thread to run.
 *
 * @details This returns the number of tasks started with ecore_thread_feedback_run()
 *          that are pending and waiting for a thread to become available to run them.
 *
 * @since_tizen 2.3
 *
 * @return The number of pending threads running "feedback" jobs
 *
 */
EAPI int ecore_thread_pending_feedback_get(void);

/**
 * @brief Gets the total number of pending jobs.
 *
 * @since_tizen 2.3
 *
 * @remarks This is same as the sum of ecore_thread_pending_get() and
 *          ecore_thread_pending_feedback_get().
 *
 * @return The number of pending threads running jobs
 *
 */
EAPI int ecore_thread_pending_total_get(void);

/**
 * @brief Gets the maximum number of threads that can run simultaneously.
 *
 * @details This returns the maximum number of Ecore_Thread's that may be running at
 *          the same time. If this number is reached, new jobs started by either
 *          ecore_thread_run() or ecore_thread_feedback_run() are added to the
 *          respective pending queues until one of the running threads finishes its
 *          task and becomes available to run a new one.
 *
 * @since_tizen 2.3
 *
 * @remarks By default, this is the number of available CPUs for the
 *          running program (as returned by eina_cpu_count()), or @c 1 if this value
 *          could not be fetched.
 *
 * @return The maximum possible number of Ecore_Thread's running concurrently
 *
 * @see ecore_thread_max_set()
 * @see ecore_thread_max_reset()
 */
EAPI int ecore_thread_max_get(void);

/**
 * @brief Sets the maximum number of threads allowed to run simultaneously.
 *
 * @details This sets a new value for the maximum number of concurrently running
 *          Ecore_Thread's. It @b must be an integer between @c 1 and (@c 16 * @c x), where @c x
 *          is the number for CPUs available.
 *
 * @since_tizen 2.3
 *
 * @param[in] num The new maximum
 *
 * @see ecore_thread_max_get()
 * @see ecore_thread_max_reset()
 */
EAPI void ecore_thread_max_set(int num);

/**
 * @brief Resets the maximum number of concurrently running threads to the default.
 *
 * @details This resets the value returned by ecore_thread_max_get() back to its
 *          default.
 *
 * @since_tizen 2.3
 *
 * @see ecore_thread_max_get()
 * @see ecore_thread_max_set()
 */
EAPI void ecore_thread_max_reset(void);

/**
 * @brief Gets the number of threads available for running tasks.
 *
 * @since_tizen 2.3
 *
 * @remarks This is same as doing ecore_thread_max_get() - ecore_thread_active_get().
 *
 * @remarks This function may return a negative number only in the case when the user
 *          changes the maximum number of running threads while other tasks are
 *          running.
 *
 * @return The number of available threads
 *
 */
EAPI int ecore_thread_available_get(void);

/**
 * @brief Adds some data present in the hash local to the thread. 
 *
 * @since_tizen 2.3
 *
 * @remarks Ecore Thread has a mechanism to share data across several worker functions
 *          that run on the same system thread. That is, the data is stored per
 *          thread and for a worker function to have access to it, it must be run
 *          by the same thread that stored the data.
 *
 * @remarks When there are no more workers pending, the thread is destroyed
 *          along with the internal hash and any data left in it is freed with
 *          the given @a cb function.
 *
 * @         This set of functions is useful to share things around several instances
 *          of a function when that thing is costly to create and can be reused, but
 *          may only be used by one function at a time.
 *
 *          For example, if you have a program doing requisitions to a database,
 *          these requisitions can be done in threads so that waiting for the
 *          database to respond doesn't block the UI. Each of these threads
 *          run a function, and each function is dependent on a connection to
 *          the database, which may not be able to handle more than one request at
 *          a time so for each running function you need one connection handle.
 * 
 *          The options then are:
 *          @li Each function opens a connection when it's called, does the work and
 *          closes the connection when it finishes. This may be costly, wasting a lot
 *          of time on resolving hostnames, negotiating permissions, and allocating
 *          memory.
 *          @li Open the connections in the main loop and pass it to the threads
 *          using the data pointer. Even worse, it's just as costly as before and now
 *          it may even be kept with connections open doing nothing until a thread
 *          becomes available to run the function.
 *          @li Have a way to share connection handles, so that each instance of the
 *          function can check if an available connection exists, and if it doesn't,
 *          create one and add it to the pool. When no more connections are needed,
 *          they are all closed.
 *
 *          The last option is the most efficient, but it requires a lot of work to
 *          be implemented properly. Using thread local data helps to achieve the same
 *          result while avoiding all the tracking work on your code. The way
 *          to use it would be at the worker function, to ask for the connection
 *          using ecore_thread_local_data_find() and if it doesn't exist, then open
 *          a new one and save it with ecore_thread_local_data_add(). Complete the work and
 *          forget about the connection handle, when everything is done the function
 *          just ends. The next worker to run on that thread checks if a
 *          connection exists and finds that it does, so the process of opening a
 *          new one has been spared. When no more workers exist, the thread is
 *          destroyed and the callback used when saving the connection is called
 *          to close it.
 *
 * @remarks This function adds the data @a value to the thread data under the given
 *          @a key. No other value in the hash may have the same @a key. If you need to
 *          change the value under a @a key, or you don't know if one exists already,
 *          you can use ecore_thread_local_data_set().
 *
 *          Neither @a key nor @a value may be @c NULL and @a key gets copied in the
 *          hash, unless @a direct is set, in which case the string used should not
 *          be freed until the data is removed from the hash.
 *
 * @remarks The @a cb function is called when the data in the hash needs to be
 *          freed, be it because it got deleted by ecore_thread_local_data_del() or
 *          because @a thread got terminated and the hash got destroyed. This parameter
 *          may be @c NULL, in which case @a value needs to be manually freed after
 *          removing it from the hash with either ecore_thread_local_data_del() or
 *          ecore_thread_local_data_set(), but it's very unlikely that this is what
 *          you want.
 *
 *          This function, and all of the others in the @a ecore_thread_local_data
 *          family of functions, can only be called within the worker function running
 *          in the thread. Do not call them from the main loop or from a thread
 *          other than the one represented by @a thread.
 *
 * @param[in] thread The thread context the data belongs to
 * @param[in] key The name under which the data is stored
 * @param[in] value The data to add
 * @param[in] cb The function to free the data when removed from the hash
 * @param[in] direct If @c true, this does not copy the key string (like eina_hash_direct_add()),
 *               otherwise @c false
 * @return #EINA_TRUE on success,
 *         otherwise @c EINA_FALSE on failure
 *
 * @see ecore_thread_local_data_set()
 * @see ecore_thread_local_data_find()
 * @see ecore_thread_local_data_del()
 */
EAPI Eina_Bool ecore_thread_local_data_add(Ecore_Thread *thread, const char *key, void *value,
                                           Eina_Free_Cb cb, Eina_Bool direct);

/**
 * @brief Sets some data present in the hash local to the given thread.
 *
 * @since_tizen 2.3
 *
 * @remarks If no data exists in the hash under the @a key, this function adds
 *          @a value in the hash under the given @a key and returns @c NULL.
 *          The key itself is copied.
 *
 *          If the hash already contains something under @a key, the data is
 *          replaced by @a value and the old value is returned.
 *
 *          @c NULL is also returned if either @a key or @a value are @c NULL, or
 *          if an error occurs.
 *
 * @remarks This function, and all of the others in the @a ecore_thread_local_data
 *          family of functions, can only be called within the worker function running
 *          in the thread. Do not call them from the main loop or from a thread
 *          other than the one represented by @a thread.
 *
 * @param[in] thread The thread context the data belongs to
 * @param[in] key The name under which the data is stored
 * @param[in] value The data to add
 * @param[in] cb The function to free the data when removed from the hash
 *
 * @see ecore_thread_local_data_add()
 * @see ecore_thread_local_data_del()
 * @see ecore_thread_local_data_find()
 */
EAPI void *ecore_thread_local_data_set(Ecore_Thread *thread, const char *key, void *value, Eina_Free_Cb cb);

/**
 * @brief Gets data stored in the hash local to the given thread.
 *
 * @since_tizen 2.3
 *
 * @details This finds and returns the data stored in the shared hash under the key @a key.
 *
 * @remarks This function, and all the others in the @a ecore_thread_local_data
 *          family of functions, can only be called within the worker function running
 *          in the thread. Do not call them from the main loop or from a thread
 *          other than the one represented by @a thread.
 *
 * @param[in] thread The thread context the data belongs to
 * @param[in] key The name under which the data is stored
 * @return The value under the given key,
 *         otherwise @c NULL on an error
 *
 * @see ecore_thread_local_data_add()
 * @see ecore_thread_local_data_wait()
 */
EAPI void *ecore_thread_local_data_find(Ecore_Thread *thread, const char *key);

/**
 * @brief Deletes the data corresponding to the given key from the thread's hash.
 *
 * @since_tizen 2.3
 *
 * @remarks If there's any data associated with @a key that is stored in the global hash,
 *          this function removes it from the hash and returns #EINA_TRUE. If no data
 *          exists or an error occurs, it returns @c EINA_FALSE.
 *
 * @remarks If the data is added to the hash with a free function, then it is
 *          also freed after removing it from the hash, otherwise it requires
 *          to be manually freed by the user, which means that if no other reference
 *          to it exists before calling this function, it results in a memory
 *          leak.
 *
 * @remarks This function, and all the others in the @a ecore_thread_local_data
 *          family of functions, can only be called within the worker function running
 *          in the thread. Do not call them from the main loop or from a thread
 *          other than the one represented by @a thread.
 *
 * @param[in] thread The thread context the data belongs to
 * @param[in] key The name under which the data is stored
 * @return #EINA_TRUE on success,
 *         otherwise @c EINA_FALSE on failure
 *
 * @see ecore_thread_local_data_add()
 */
EAPI Eina_Bool ecore_thread_local_data_del(Ecore_Thread *thread, const char *key);

/**
 * @brief Adds some data to a hash shared by all threads.
 *
 * @since_tizen 2.3
 *
 * @remarks Ecore Thread keeps a hash that can be used to share data across several
 *          threads, including the main loop thread, without having to manually handle
 *          mutexes to do it safely.
 *
 * @remarks This function adds the data @a value to this hash under the given @a key.
 *          No other value in the hash may have the same @a key. If you need to
 *          change the value under a @a key, or you don't know if one exists already,
 *          you can use ecore_thread_global_data_set().
 *
 *          Neither @a key nor @a value may be @c NULL and @a key gets copied in the
 *          hash, unless @a direct is set, in which case the string used should not
 *          be freed until the data is removed from the hash.
 *
 * @remarks The @a cb function is called when the data in the hash needs to be
 *          freed, be it because it got deleted with ecore_thread_global_data_del() or
 *          because Ecore Thread got shut down and the hash got destroyed. This parameter
 *          may be @c NULL, in which case @a value needs to be manually freed after
 *          removing it from the hash with either by ecore_thread_global_data_del() or
 *          ecore_thread_global_data_set().
 *
 *          Manually freeing any data that is added to the hash with the @a cb function
 *          is likely to produce a segmentation fault, or any other strange
 *          happening at a later stage in the program.
 *
 * @param[in] key The name under which the data is stored
 * @param[in] value The data to add
 * @param[in] cb The function to free the data when removed from the hash
 * @param[in] direct If @c true, this does not copy the key string (like eina_hash_direct_add()),
 *               otherwise @c false
 * @return #EINA_TRUE on success,
 *         otherwise @c EINA_FALSE on failure
 *
 * @see ecore_thread_global_data_del()
 * @see ecore_thread_global_data_set()
 * @see ecore_thread_global_data_find()
 */
EAPI Eina_Bool ecore_thread_global_data_add(const char *key, void *value, Eina_Free_Cb cb, Eina_Bool direct);

/**
 * @brief Sets some data in the hash shared by all threads.
 *
 * @since_tizen 2.3
 *
 * @remarks If no data exists in the hash under the @a key, this function adds
 *          @a value in the hash under the given @a key and returns @c NULL.
 *          The key itself is copied.
 *
 *          If the hash already contains something under @a key, the data is
 *          replaced by @a value and the old value is returned.
 *
 *          @c NULL is also returned if either @a key or @a value is @c NULL, or
 *          if an error occurs.
 *
 *
 * @param[in] key The name under which the data is stored
 * @param[in] value The data to add
 * @param[in] cb The function to free the data when removed from the hash
 *
 * @see ecore_thread_global_data_add()
 * @see ecore_thread_global_data_del()
 * @see ecore_thread_global_data_find()
 */
EAPI void *ecore_thread_global_data_set(const char *key, void *value, Eina_Free_Cb cb);

/**
 * @brief Gets data stored in the hash shared by all threads.
 *
 * @since_tizen 2.3
 *
 * @details This finds and returns the data stored in the shared hash under the key @a key.
 *
 * @remarks Keep in mind that the data returned may be used by more than one thread
 *          at the same time and no reference counting is done on it by Ecore.
 *          Freeing the data or modifying its contents may require additional
 *          precautions to be considered, depending on the application's design.
 *
 * @param[in] key The name under which the data is stored
 * @return The value under the given key, 
 *         otherwise @c NULL on an error
 *
 * @see ecore_thread_global_data_add()
 * @see ecore_thread_global_data_wait()
 */
EAPI void *ecore_thread_global_data_find(const char *key);

/**
 * @brief Deletes the data corresponding to the given key from the shared hash.
 *
 * @since_tizen 2.3
 *
 * @remarks If there's any data associated with @p key that is stored in the global hash,
 *          this function removes it from the hash and returns #EINA_TRUE. If no data
 *          exists or an error occurs, it returns @c EINA_FALSE.
 *
 * @remarks If the data is added to the hash with a free function, then it is
 *          also freed after removing it from the hash, otherwise it requires
 *          to be manually freed by the user, which means that if no other reference
 *          to it exists before calling this function, it results in a memory
 *          leak.
 *
 *          Note, also, that freeing data that other threads may be using results
 *          in a crash, so appropriate care must be taken by the application when
 *          that possibility exists.
 *
 * @param[in] key The name under which the data is stored
 * @return #EINA_TRUE on success, 
 *         otherwise @c EINA_FALSE on failure
 *
 * @see ecore_thread_global_data_add()
 */
EAPI Eina_Bool ecore_thread_global_data_del(const char *key);

/**
 * @brief Gets data stored in the shared hash or waits for it if it doesn't exist.
 *
 * @since_tizen 2.3
 *
 * @remarks This finds and returns the data stored in the shared hash under the key @a key.
 *
 *          If there's nothing in the hash under the given @a key, the function
 *          blocks and waits for @a seconds seconds for some other thread to
 *          add it with either ecore_thread_global_data_add() or
 *          ecore_thread_global_data_set(). If after waiting there's still no data
 *          to obtain, @c NULL is returned.
 *
 *          If @a seconds is @c 0, then no waiting happens and this function works
 *          like ecore_thread_global_data_find(). If @a seconds is less than @c 0, then
 *          the function waits indefinitely.
 *
 * @remarks Keep in mind that the data returned may be used by more than one thread
 *          at the same time and no reference counting is done on it by Ecore.
 *          Freeing the data or modifying its contents may require additional
 *          precautions to be considered, depending on the application's design.
 *
 * @param[in] key The name under which the data is stored
 * @param[in] seconds The amount of time in seconds to wait for the data
 * @return The value under the given key, 
 *         otherwise @c NULL on an error
 *
 * @see ecore_thread_global_data_add()
 * @see ecore_thread_global_data_find()
 */
EAPI void *ecore_thread_global_data_wait(const char *key, double seconds);

/**
 * @}
 */

/**
 * @defgroup Ecore_Timer_Group Ecore Timer
 * @ingroup Ecore_Main_Loop_Group
 *
 * @brief Ecore provides very flexible timer functionality.
 *
 * The basic usage of timers is to call a certain function at a certain 
 * interval, which can be achieved with a single line:
 * @code
 * Eina_Bool my_func(void *data) {
 *    do_funky_stuff_with_data(data);
 *    return ECORE_CALLBACK_RENEW;
 * }
 * ecore_timer_add(interval_in_seconds, my_func, data_given_to_function);
 * @endcode
 * If the function is to be executed only once simply return
 * @c CORE_CALLBACK_CANCEL instead.
 *
 * @{
 */

typedef struct _Ecore_Timer Ecore_Timer; /**< @brief A handle for timers */

/**
 * @brief Creates a timer to call the given function in the given period of time.
 *
 * @details This function adds a timer and returns its handle on success and NULL on
 *          failure. The function @p func will be called every @p in seconds. The
 *          function will be passed the @p data pointer as its parameter.
 *
 * @since_tizen 2.3
 *
 * @remarks When the timer @p func is called, it must return a value of either 1
 *          (or ECORE_CALLBACK_RENEW) or 0 (or ECORE_CALLBACK_CANCEL).
 *          If it returns 1, it will be called again at the next tick, or if it returns
 *          0 it will be deleted automatically making any references/handles for it
 *          invalid.
 *
 * @param[in]   in   The interval in seconds.
 * @param[in]   func The given function.  If @p func returns 1, the timer is
 *               rescheduled for the next interval @p in.
 * @param[in]   data Data to pass to @p func when it is called.
 * @return  A timer object on success.  @c NULL on failure.
 */
EAPI Ecore_Timer *ecore_timer_add(double in, Ecore_Task_Cb func, const void *data);

/**
 * @brief Creates a timer to call the given function in the given period of time.
 *
 * @since_tizen 2.3
 *
 * @remarks This is same as ecore_timer_add(), but "now" is the time from
 *          ecore_loop_time_get(), not ecore_time_get(), as ecore_timer_add() uses it. See
 *          ecore_timer_add() for more details.
 *
 * @param[in]   in   The interval in seconds from the current loop time
 * @param[in]   func The given function \n
 *               If @a func returns @c 1, the timer is
 *               rescheduled for the next interval @a in.
 * @param[in]   data The data to pass to @a func when it is called
 * @return  A timer object on success,
 *          otherwise @c NULL on failure
 */
EAPI Ecore_Timer *ecore_timer_loop_add(double in, Ecore_Task_Cb func, const void *data);

/**
 * @brief Deletes the specified timer from the timer list.
 *
 * @details This deletes the specified @a timer from the set of timer that are
 *          executed during main loop execution. This function returns the data
 *          parameter that is being passed to the callback on success, otherwise @c NULL on
 *          failure.
 *
 * @since_tizen 2.3
 *
 * @param[in]  timer The timer to delete
 * @return The data pointer set for the timer on add
 *
 */
EAPI void *ecore_timer_del(Ecore_Timer *timer);

/**
 * @brief Change the interval the timer ticks off.
 *
 * @since_tizen 2.3
 *
 * @param[in]   timer The timer to change.
 * @param[in]   in    The interval in seconds.
 */
EAPI void ecore_timer_interval_set(Ecore_Timer *timer, double in);

/**
 * @brief Get the interval the timer ticks on.
 *
 * @since_tizen 2.3
 *
 * @param[in]   timer The timer to retrieve the interval from
 * @return  The interval on success. -1 on failure.
 */
EAPI double ecore_timer_interval_get(Ecore_Timer *timer);

/**
 * @brief Pauses a running timer.
 *
 * @since_tizen 2.3
 *
 * @remarks The timer callback won't be called while the timer is paused. The remaining
 *          time until the timer expires will be saved, so the timer can be resumed with
 *          that same remaining time to expire, instead of expiring instantly.  Use
 *          ecore_timer_thaw() to resume it.
 *
 * @remarks Nothing happens if the timer was already paused.
 *
 * @param[in] timer The timer to be paused.
 *
 * @see ecore_timer_thaw()
 */
EAPI void ecore_timer_freeze(Ecore_Timer *timer);

/**
 * @brief Resumes a frozen (paused) timer.
 *
 * @since_tizen 2.3
 *
 * @remarks The timer will be resumed from its previous relative position in time. That
 *          means, if it had X seconds remaining until expire when it was paused, it will
 *          be started now with those same X seconds remaining to expire again. But
 *          notice that the interval time won't be touched by this call or by
 *          ecore_timer_freeze().
 *
 * @param[in] timer The timer to be resumed.
 *
 * @see ecore_timer_freeze()
 */
EAPI void ecore_timer_thaw(Ecore_Timer *timer);

/**
 * @brief   Add some delay for the next occurrence of a timer.
 *
 * @since_tizen 2.3
 *
 * @remarks This doesn't affect the interval of a timer.
 *
 * @param[in]   timer The timer to change.
 * @param[in]   add   The delay to add to the next iteration.
 */
EAPI void ecore_timer_delay(Ecore_Timer *timer, double add);

/**
 * @brief Reset a timer to its full interval. This effectively makes
 *        the timer start ticking off from zero now.
 *
 * @param[in]   timer The timer
 *
 * @since_tizen 2.3
 */
EAPI void ecore_timer_reset(Ecore_Timer *timer);

/**
 * @brief Get the pending time regarding a timer.
 *
 * @param[in] timer The timer
 * @return    The pending time
 *
 * @since_tizen 2.3
 */
EAPI double ecore_timer_pending_get(Ecore_Timer *timer);

/**
 * @brief Retrieves the current precision used by timer infrastructure.
 *
 * @since_tizen 2.3
 *
 * @return Current precision.
 *
 * @see ecore_timer_precision_set()
 */
EAPI double ecore_timer_precision_get(void);

/**
 * @brief Sets the precision to be used by timer infrastructure.
 *
 * @since_tizen 2.3
 *
 * @remarks This sets the precision for @b all timers. The precision determines how much
 *          of an difference from the requested interval is acceptable. One common reason
 *          to use this function is to @b increase the allowed timeout and thus @b
 *          decrease precision of the timers, this is because less precise the timers
 *          result in the system waking up less often and thus consuming less resources.
 *
 * @remarks Be aware that kernel may delay delivery even further, these delays
 *          are always possible due other tasks having higher priorities or
 *         other scheduler policies.
 *
 * @remarks Example:
 *          We have 2 timers, one that expires in a 2.0s and another that
 *          expires in 2.1s, if precision is 0.1s, then the Ecore will request
 *          for the next expire to happen in 2.1s and not 2.0s and another one
 *          of 0.1 as it would before.
 *
 * @remarks Ecore is smart enough to see if there are timers in the
 *          precision range, if it does not, in our example if no second timer
 *          in (T + precision) existed, then it would use the minimum timeout.

 * @param[in] precision difference from the requested internval.
 */
EAPI void ecore_timer_precision_set(double precision);

/**
 * @brief Dump the all timers.
 *
 * @since_tizen 2.3
 *
 * @return The information of all timers
 */
EAPI char *ecore_timer_dump(void);

/**
 * @}
 */

/**
 * @defgroup Ecore_Idle_Group Ecore Idle
 * @ingroup Ecore_Main_Loop_Group
 *
 * @brief The idler functionality in Ecore allows for callbacks to be called when the
 *        program isn't handling @ref Ecore_Event_Group "events", @ref Ecore_Timer_Group
 *        "timers", or @ref Ecore_FD_Handler_Group "fd handlers".
 *
 * There are three types of idlers: Enterers, Idlers(proper), and Exiters. They
 * are called respectively when the program is about to enter an idle state,
 * when the program is in an idle state and when the program has just left an
 * idle state and begins processing @ref Ecore_Event_Group "events", @ref
 * Ecore_Timer_Group "timers", or @ref Ecore_FD_Handler_Group "fd handlers".
 *
 * Enterer callbacks are good for updating your program's state, if
 * it has a state engine. Once all of the enterer handlers are
 * called, the program enters a "sleeping" state.
 *
 * Idler callbacks are called when the main loop has called all
 * enterer handlers. They are useful for interfaces that require
 * polling and timers without which they would be too slow to use.
 *
 * Exiter callbacks are called when the main loop wakes up from an idle state.
 *
 * If no idler callbacks are specified, then the process literally
 * goes to sleep.  Otherwise, the idler callbacks are called
 * continuously while the loop is "idle", using as much CPU as is
 * available to the process.
 *
 * Idle state doesn't mean that the @b program is idle, but
 * that the <b>main loop</b> is idle. It doesn't have any timers,
 * events, fd handlers, or anything else to process (which in most
 * <em>event driven</em> programs also means that the @b program is
 * idle too, but it's not a rule). The program itself may be doing
 * a lot of processing in the idler, or in another thread, for
 * example.
 *
 * @{
 */

typedef struct _Ecore_Idler        Ecore_Idler; /**< @brief A handle for idlers */
typedef struct _Ecore_Idle_Enterer Ecore_Idle_Enterer; /**< @brief A handle for idle enterers */
typedef struct _Ecore_Idle_Exiter  Ecore_Idle_Exiter; /**< @brief A handle for idle exiters */

/**
 * @brief Adds an idler handler.
 *
 * @details This adds an idler handle to the event loop, returning a handle on
 *          success and @c NULL otherwise. The function @a func is called
 *          repeatedly while no other events are ready to be processed, as
 *          long as it returns @c 1 (or @c ECORE_CALLBACK_RENEW). A return of @c 0
 *          (or @c ECORE_CALLBACK_CANCEL) deletes the idler.
 *
 * @since_tizen 2.3
 *
 * @remarks Idlers are useful for progressively processing data without blocking.
 *
 * @param[in]  func The function to call when idling
 * @param[in]  data The data to be passed to this @a func call
 * @return A idler handle if successfully added,
 *         otherwise @c NULL
 *
 */
EAPI Ecore_Idler *ecore_idler_add(Ecore_Task_Cb func, const void *data);

/**
 * @brief Deletes an idler callback from the list to be executed.
 *
 * @since_tizen 2.3
 *
 * @param[in]  idler The handle of the idler callback to delete
 * @return The data pointer passed to the idler callback on success,
 *         otherwise @c NULL
 */
EAPI void *ecore_idler_del(Ecore_Idler *idler);

/**
 * @brief Add an idle enterer handler.
 *
 * @since_tizen 2.3
 *
 * @remarks The function func will be called every time the main loop is entering
 *          idle state, as long as it returns 1 (or ECORE_CALLBACK_RENEW). A return of 0
 *          (or ECORE_CALLBACK_CANCEL) deletes the idle enterer.
 *
 * @param[in]   func The function to call when entering an idle state.
 * @param[in]   data The data to be passed to the @p func call
 * @return  A handle to the idle enterer callback if successful.  Otherwise,
 *          NULL is returned.
 */
EAPI Ecore_Idle_Enterer *ecore_idle_enterer_add(Ecore_Task_Cb func, const void *data);

/**
 * @brief Add an idle enterer handler at the start of the list so it gets called earlier than others.
 *
 * @since_tizen 2.3
 *
 * @remarks The function func will be called every time the main loop is entering
 *          idle state, as long as it returns 1 (or ECORE_CALLBACK_RENEW). A return of 0
 *          (or ECORE_CALLBACK_CANCEL) deletes the idle enterer.
 *
 * @param[in]   func The function to call when entering an idle state.
 * @param[in]   data The data to be passed to the @p func call
 * @return  A handle to the idle enterer callback if successful.  Otherwise,
 *          NULL is returned.
 */
EAPI Ecore_Idle_Enterer *ecore_idle_enterer_before_add(Ecore_Task_Cb func, const void *data);

/**
 * @brief Delete an idle enterer callback.
 *
 * @since_tizen 2.3
 *
 * @param[in]   idle_enterer The idle enterer to delete
 * @return  The data pointer passed to the idler enterer callback on success.
 *          NULL otherwise.
 */
EAPI void *ecore_idle_enterer_del(Ecore_Idle_Enterer *idle_enterer);

/**
 * @brief Add an idle exiter handler.
 *
 * @since_tizen 2.3
 *
 * @remarks The function func will be called every time the main loop is exiting
 *          idle state, as long as it returns 1 (or ECORE_CALLBACK_RENEW). A return of 0
 *          (or ECORE_CALLBACK_CANCEL) deletes the idle exiter.
 *
 * @param[in] func The function to call when exiting an idle state.
 * @param[in] data The data to be passed to the @p func call
 * @return A handle to the idle exiter callback on success.  NULL otherwise.
 */
EAPI Ecore_Idle_Exiter *ecore_idle_exiter_add(Ecore_Task_Cb func, const void *data);

/**
 * @brief Delete an idle exiter handler from the list to be run on exiting idle state.
 *
 * @since_tizen 2.3
 *
 * @param[in] idle_exiter The idle exiter to delete
 * @return The data pointer that was being being passed to the handler if
 *         successful.  NULL otherwise.
 */
EAPI void *ecore_idle_exiter_del(Ecore_Idle_Exiter *idle_exiter);

/**
 * @}
 */

/**
 * @defgroup Ecore_Pipe_Group Ecore Pipe Wrapper
 * @ingroup Ecore_Main_Loop_Group
 *
 * @brief This group discusses the functions that wrap the write / read functions of the pipe to easily
 *        integrate its use into ecore's main loop.
 *
 * @remarks The ecore_pipe_add() function creates file descriptors (sockets
 *          on Windows) and attaches a handle to the ecore main loop. That
 *          handle is called when data is read in the pipe. To write data in
 *          the pipe, just call ecore_pipe_write(). When you are done, just
 *          call ecore_pipe_del().
 *
 * @{
 */

typedef struct _Ecore_Pipe Ecore_Pipe; /**< @brief A handle for pipes */

/**
 * @typedef Ecore_Pipe_Cb Ecore_Pipe_Cb
 * @brief Called to send data written to the pipe.
 */
typedef void (*Ecore_Pipe_Cb)(void *data, void *buffer, unsigned int nbyte);

/**
 * @brief Create two file descriptors (sockets on Windows).
 *
 * @details Add a callback that will be called when the file descriptor that
 *          is listened receives data. An event is also put in the event
 *          queue when data is received.
 *
 * @since_tizen 2.3
 *
 * @param[in] handler The handler called when data is received.
 * @param[in] data    Data to pass to @p handler when it is called.
 * @return        A newly created Ecore_Pipe object if successful.
 *                @c NULL otherwise.
 */
EAPI Ecore_Pipe *ecore_pipe_add(Ecore_Pipe_Cb handler, const void *data);

/**
 * @brief Free an Ecore_Pipe object created with ecore_pipe_add().
 *
 * @since_tizen 2.3
 *
 * @param[in] p The Ecore_Pipe object to be freed.
 * @return The pointer to the private data
 */
EAPI void *ecore_pipe_del(Ecore_Pipe *p);

/**
 * @brief Write on the file descriptor the data passed as parameter.
 *
 * @since_tizen 2.3
 *
 * @param[in] p      The Ecore_Pipe object.
 * @param[in] buffer The data to write into the pipe.
 * @param[in] nbytes The size of the @p buffer in bytes
 * @return       #EINA_TRUE on a successful write, @c EINA_FALSE on error.
 */
EAPI Eina_Bool ecore_pipe_write(Ecore_Pipe *p, const void *buffer, unsigned int nbytes);

/**
 * @brief Close the write end of an Ecore_Pipe object created with ecore_pipe_add().
 *
 * @since_tizen 2.3
 *
 * @param[in] p The Ecore_Pipe object.
 */
EAPI void ecore_pipe_write_close(Ecore_Pipe *p);

/**
 * @brief  Close the read end of an Ecore_Pipe object created with
 *         ecore_pipe_add().
 *
 * @since_tizen 2.3
 *
 * @param[in] p The Ecore_Pipe object.
 */
EAPI void ecore_pipe_read_close(Ecore_Pipe *p);

/**
 * @brief Start monitoring again the pipe for reading. See ecore_pipe_freeze()
 *        for stopping the monitoring activity. This will not work if
 *        ecore_pipe_read_close() was previously called on the same pipe.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @param[in] p The Ecore_Pipe object.
 */
EAPI void ecore_pipe_thaw(Ecore_Pipe *p);

/**
 * @brief Stop monitoring if necessary the pipe for reading.
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @param[in] p The Ecore_Pipe object.
 *
 * @see ecore_pipe_thaw() for monitoring it again.
 *
 */
EAPI void ecore_pipe_freeze(Ecore_Pipe *p);

/**
 * @brief Wait from another thread on the read side of a pipe.
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @remarks Negative value for @p wait means infite wait.
 *
 * @param[in] p The pipe to watch on.
 * @param[in] message_count The minimal number of message to wait before exiting.
 * @param[in] wait The amount of time in second to wait before exiting.
 * @return the number of message catched during that wait call.
 *
 */
EAPI int ecore_pipe_wait(Ecore_Pipe *p, int message_count, double wait);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Ecore_Throttle_Group Ecore Throttle
 * @ingroup Ecore_Main_Loop_Group
 *
 * @{
 */

/**
 * @brief Increase throttle amount
 *
 * @details This will increase or decrease (if @p amount is positive or negative) the
 *          amount of "voluntary throttling" ecore will do to its main loop while
 *          running. This is intended to be used to limit animations and wakeups when
 *          in a strict power management state. The higher the current throttle value
 *          (which can be retrieved by ecore_throttle_get() ), the more throttling
 *          takes place. If the current throttle value is 0, then no throttling takes
 *          place at all.
 *
 * @remarks The value represents how long the ecore main loop will sleep (in seconds)
 *          before it goes into a fully idle state waiting for events, input or
 *          timing events to wake it up. For example, if the current throttle level
 *          is 0.5, then after every time the main loop cycles and goes into idle
 *          affter processing all events, the main loop will explicitly sleep for 0.5
 *          seconds before sitting and waiting for incoming events or timeouts, thus
 *          preventing animation, async IO and network handling etc. for that period
 *          of time. Of course these events, data and timeouts will be buffered,
 *          thus not losing anything, simply delaying when they get handled by the
 *          throttle value.
 *
 * Example:
 * @code
 * void enter_powersave(void) {
 *    ecore_throttle_adjust(0.2);
 *    printf("Now at throttle level: %1.3f\n", ecore_throttle_get());
 * }
 *
 * void enter_deep_powersave(void) {
 *    ecore_throttle_adjust(0.5);
 *    printf("Now at throttle level: %1.3f\n", ecore_throttle_get());
 * }
 *
 * void exit_powersave(void) {
 *    ecore_throttle_adjust(-0.2);
 *    printf("Now at throttle level: %1.3f\n", ecore_throttle_get());
 * }
 *
 * void exit_deep_powersave(void) {
 *    ecore_throttle_adjust(-0.5);
 *    printf("Now at throttle level: %1.3f\n", ecore_throttle_get());
 * }
 * @endcode
 *
 * @param[in]   amount Amount (in seconds) to adjust by
 */

EAPI void ecore_throttle_adjust(double amount);

/**
 * @brief Get current throttle level
 *
 * @remarks This gets the current throttling level, which can be adjusted by
 * ecore_throttle_adjust(). The value is in seconds.
 *
 * @return  The current throttle level
 *
 * @see ecore_throttle_adjust() for more information.
 *
 */
EAPI double ecore_throttle_get(void);

/**
 * @}
 */

/**
 * @defgroup Ecore_Poller_Group Ecore Poller
 * @ingroup Ecore_Main_Loop_Group
 *
 * @brief Ecore poller provides infrastructure for the creation of pollers. 
 *
 * Pollers are, in essence, callbacks that share a single timer per type. Because not
 * all pollers need to be called at the same frequency the user may specify the
 * frequency in ticks(each expiration of the shared timer is called a tick, in
 * ecore poller parlance) for each added poller. Ecore pollers should only be
 * used when the poller doesn't have specific requirements on the exact times
 * to poll.
 *
 * This architecture means that the main loop is only woken up once to handle
 * all pollers of that type, this saves power as the CPU has more of a
 * chance to go into a low power state the longer it is asleep, so this
 * should be used in situations where power usage is a concern.
 *
 * For now only 1 core poller type is supported: @c ECORE_POLLER_CORE.
 * The default interval for @c ECORE_POLLER_CORE is @c 0.125(or 1/8th) second.
 *
 * The creation of a poller is extremely simple and only requires one line:
 * @code
 * ecore_poller_add(ECORE_POLLER_CORE, 1, my_poller_function, NULL);
 * @endcode
 * This sample creates a poller to call @a my_poller_function at every tick with
 * @c NULL as data.
 *
 * @{
 */

/**
 * @enum _Ecore_Poller_Type
 * @brief Enumeration that defines the frequency of ticks for the poller.
 */
enum _Ecore_Poller_Type    /* Poller types */
{
   ECORE_POLLER_CORE = 0, /**< The core poller interval */
#ifdef __linux
   ECORE_POLLER_LAZY = 1, /**< Core poller based on timerfd,
                               timer is deferrable in case the kernel supports it (no fire at IDLE time) */ 
#endif
   ECORE_POLLER_TYPE_MAX
};

/**
 * @brief typedef to enum _Ecore_Poller_Type
 */
typedef enum _Ecore_Poller_Type Ecore_Poller_Type;

typedef struct _Ecore_Poller    Ecore_Poller; /**< @brief A handle for pollers */

/**
 * @brief Sets the time(in seconds) between ticks for the given poller type.
 *
 * @details This adjusts the time between ticks of the given timer type defined by
 *          @a type to the time period defined by @a poll_time.
 *
 * @since_tizen 2.3
 *
 * @param[in] type The poller type to adjust
 * @param[in] poll_time The time(in seconds) between ticks of the timer
 *
 */
EAPI void ecore_poller_poll_interval_set(Ecore_Poller_Type type, double poll_time);

/**
 * @brief Gets the time(in seconds) between ticks for the given poller type.
 *
 * @details This gets the time between ticks of the specified poller timer.
 *
 * @since_tizen 2.3
 *
 * @param[in] type The poller type to query
 * @return The time in seconds between ticks of the poller timer
 *
 */
EAPI double ecore_poller_poll_interval_get(Ecore_Poller_Type type);

/**
 * @brief Changes the polling interval rate of @a poller.
 *
 * @details This allows the changing of a poller's polling interval. It is useful when
 *          you want to alter a poll rate without deleting and re-creating a poller.
 *
 * @since_tizen 2.3
 *
 * @param[in] poller The Ecore_Poller to change the interval of
 * @param[in] interval The tick interval to set, must be a power of 2 and <= 32768
 * @return @c true on success, otherwise @c false on failure
 *
 */
EAPI Eina_Bool ecore_poller_poller_interval_set(Ecore_Poller *poller, int interval);

/**
 * @brief Gets the polling interval rate of @a poller.
 *
 * @details This returns a poller's polling interval, otherwise @c 0 on error.
 *
 * @since_tizen 2.3
 *
 * @param[in] poller The Ecore_Poller to change the interval of
 * @return The interval, in ticks, that @a poller polls at
 *
 */
EAPI int ecore_poller_poller_interval_get(Ecore_Poller *poller);

/**
 * @brief Creates a poller to call the given function at a particular tick interval.
 *
 * @details This function adds @a func as a poller callback that is called every @a
 *          interval ticks together with other pollers of type @a type. @a func is
 *          passed the @a data pointer as a parameter.
 *
 * @since_tizen 2.3
 *
 * @remarks The @a interval must be between @c 1 and @c 32768 inclusive, and must be a power of
 *          @c 2 (i.e. 1, 2, 4, 8, 16, ... 16384, 32768). The exact tick in which @a func
 *          is called is undefined, as only the interval between calls can be
 *          defined. Ecore endeavors to keep pollers synchronized and calls as
 *          many in 1 wakeup event as possible. If @a interval is not a power of @c 2, the
 *          closest power of @c 2 greater than @a interval is used.
 *
 * @remarks When the poller @a func is called, it must return a value of either
 *          @c ECORE_CALLBACK_RENEW(or @c 1) or @c ECORE_CALLBACK_CANCEL(or @c 0). If it
 *          returns @c 1, it is called again at the next tick, or if it returns
 *          @c 0 it is deleted automatically making any references/handles for it
 *          invalid.
 *
 * @param[in] type The ticker type to attach the poller to \n
 *             Must be @c ECORE_POLLER_CORE.
 * @param[in] interval The poll interval
 * @param[in] func The poller function
 * @param[in] data The data to pass to @a func when it is called
 * @return A poller object on success,
 *         otherwise @c NULL on failure
 *
 */
EAPI Ecore_Poller *ecore_poller_add(Ecore_Poller_Type type, int interval, Ecore_Task_Cb func, const void *data);

/**
 * @brief Deletes the specified poller from the timer list.
 *
 * @since_tizen 2.3
 *
 * @remarks @a poller must be a valid handle. If the poller function has already
 *          returned @c 0, the handle is no longer valid (and does not need to be deleted).
 *
 * @param[in] poller The poller to delete
 * @return The data pointer set for the timer when @ref ecore_poller_add is called on success,
 *         otherwise @c NULL on failure
 */
EAPI void *ecore_poller_del(Ecore_Poller *poller);

/**
 * @}
 */

/**
 * @defgroup Ecore_Animator_Group Ecore Animator
 * @ingroup Ecore_Main_Loop_Group
 *
 * @brief Ecore animators are a helper to simplify creating animations.
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
 *
 * In the sample above we create an animation to move
 * @c my_evas_object from position (0,0) to (100,100) in @c 2 seconds.
 *
 * If your animation runs for an unspecified amount of time you
 * can use ecore_animator_add(), which is like using
 * ecore_timer_add() with the interval being the
 * @ref ecore_animator_frametime_set "framerate". Note that this has
 * tangible benefits of creating a timer for each animation in terms
 * of performance.
 *
 * @{
 */

/**
 * @brief handle for ecore animator.
 */
typedef struct _Ecore_Animator Ecore_Animator;

/**
 * @enum _Ecore_Pos_Map
 * @brief Enumeration that defines the position mappings for the animation.
 */
enum _Ecore_Pos_Map    /* Position mappings */
{
   ECORE_POS_MAP_LINEAR, /**< Linear 0.0 -> 1.0 */
   ECORE_POS_MAP_ACCELERATE, /**< Start slow then speed up */
   ECORE_POS_MAP_DECELERATE, /**< Start fast then slow down */
   ECORE_POS_MAP_SINUSOIDAL, /**< Start slow, speed up then slow down at the end */
   ECORE_POS_MAP_ACCELERATE_FACTOR, /**< Start slow then speed up, v1 being a power factor, @c 0.0 being linear, @c 1.0 being normal accelerate, @c 2.0 being much more pronounced accelerate (squared), @c 3.0 being cubed, and so on */
   ECORE_POS_MAP_DECELERATE_FACTOR, /**< Start fast then slow down, v1 being a power factor, @c 0.0 being linear, @c 1.0 being normal decelerate, @c 2.0 being much more pronounced decelerate (squared), @c 3.0 being cubed, and so on */
   ECORE_POS_MAP_SINUSOIDAL_FACTOR, /**< Start slow, speed up then slow down at the end, v1 being a power factor, @c 0.0 being linear, @c 1.0 being normal sinusoidal, @c 2.0 being much more pronounced sinusoidal (squared), @c 3.0 being cubed, and so on */
   ECORE_POS_MAP_DIVISOR_INTERP, /**< Start at gradient * v1, interpolated via power of v2 curve */
   ECORE_POS_MAP_BOUNCE, /**< Start at @c 0.0 then "drop" like a ball bouncing to the ground at @c 1.0, and bounce v2 times, with decay factor of v1 */
   ECORE_POS_MAP_SPRING /**< Start at @c 0.0 then "wobble" like a spring with rest position @c 1.0, and wobble v2 times, with decay factor of v1 */
};

/**
 * @brief typedef to enum _Ecore_Pos_Map
 */
typedef enum _Ecore_Pos_Map Ecore_Pos_Map;

/**
 * @enum _Ecore_Animator_Source
 * @brief Enumeration that defines the timing sources for animators.
 */
enum _Ecore_Animator_Source    /* Timing sources for animators */
{
   ECORE_ANIMATOR_SOURCE_TIMER, /**< The default system clock/timer based animator that ticks every "frametime" seconds */
   ECORE_ANIMATOR_SOURCE_CUSTOM /**< A custom animator trigger which ticks when you call ecore_animator_trigger() */
};

/**
 * @brief typedef to enum _Ecore_Animator_Source
 */
typedef enum _Ecore_Animator_Source Ecore_Animator_Source;

/**
 * @typedef Ecore_Timeline_Cb Ecore_Timeline_Cb
 * @brief The boolean type for a callback run for a task (animators with runtimes)
 */
typedef Eina_Bool (*Ecore_Timeline_Cb)(void *data, double pos);

/**
 * @brief Adds an animator to call @a func at every animation tick during main
 *        loop execution.
 *
 * @details This function adds an animator and returns its handle on success, and @c NULL
 *          on failure. The function @a func is called every N seconds where N is
 *          the @a frametime interval set by ecore_animator_frametime_set(). The
 *          function is passed the @a data pointer as its parameter.
 *
 * @since_tizen 2.3
 *
 * @remarks When the animator @a func is called, it must return a value of either @c 1 or
 *          @c 0. If it returns @c 1 (or @c ECORE_CALLBACK_RENEW), it is called again at
 *          the next tick, or if it returns @c 0 (or @c ECORE_CALLBACK_CANCEL) it is
 *          deleted automatically making any references/handles for it invalid.
 *
 * @remarks The default @a frametime value is 1/30th of a second.
 *
 * @param[in] func The function to call when it ticks off
 * @param[in] data The data to pass to the function
 * @return A handle to the new animator
 *
 * @see ecore_animator_timeline_add()
 * @see ecore_animator_frametime_set()
 */
EAPI Ecore_Animator *ecore_animator_add(Ecore_Task_Cb func, const void *data);

/**
 * @brief Adds an animator that runs for a limited time.
 *
 * @details This function is just like ecore_animator_add() except that the animator only
 *          runs for a limited time specified in seconds by @a runtime. Once the
 *          runtime of the animator has elapsed (animator finished) it is automatically
 *          deleted. The callback function @a func can return @c ECORE_CALLBACK_RENEW
 *          to keep the animator running or @c ECORE_CALLBACK_CANCEL to stop it and have
 *          it deleted automatically at any time.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @remarks The @a func is ALSO passed a position parameter that has a value
 *          from @c 0.0 to @c 1.0 to indicate where along the timeline (@c 0.0 for start, @c 1.0 for end)
 *          is the animator run at. If the callback wishes not to have a linear
 *          transition it can "map" this value to one of the several curves and mappings
 *          via ecore_animator_pos_map().
 *
 * @remarks The default @a frametime value is 1/30th of a second.
 *
 * @param[in] runtime The time to run in seconds
 * @param[in] func The function to call when it ticks off
 * @param[in] data The data to pass to the function
 * @return A handle to the new animator
 *
 * @see ecore_animator_add()
 * @see ecore_animator_pos_map()
 */
EAPI Ecore_Animator *ecore_animator_timeline_add(double runtime, Ecore_Timeline_Cb func, const void *data);

/**
 * @brief Deletes the specified animator from the animator list.
 *
 * @details This deletes the specified @a animator from the set of animators that are
 *          executed during main loop execution. This function returns the data
 *          parameter that is being passed to the callback on success, otherwise @c NULL on
 *          failure. After this call returns the specified animator object @a animator
 *          is invalid and should not be used again. It does not get called again after
 *          deletion.
 *
 * @since_tizen 2.3
 *
 * @param[in] animator The animator to delete
 * @return The data pointer set for the animator on add
 *
 */
EAPI void *ecore_animator_del(Ecore_Animator *animator);

/**
 * @brief Suspends the specified animator.
 *
 * @since_tizen 2.3
 *
 * @remarks The specified @a animator is temporarily removed from the set of
 *          animators that are executed during the main loop.
 *
 * @remarks Freezing an animator doesn't freeze accounting of how long that
 *          animator has been running. Therefore if the animator is created with
 *          ecore_animator_timeline_add() the @a pos argument given to the callback
 *          increases as if the animator hadn't been frozen and the animator may
 *          have its execution halted if @a runtime elapses.
 *
 * @param[in] animator The animator to delete
 *
 */
EAPI void ecore_animator_freeze(Ecore_Animator *animator);

/**
 * @brief Restores execution of the specified animator.
 *
 * @since_tizen 2.3
 *
 * @remarks The specified @a animator is put back in the set of animators that are
 *          executed during the main loop.
 *
 * @param[in] animator The animator to delete
 *
 */
EAPI void ecore_animator_thaw(Ecore_Animator *animator);

/**
 * @brief Sets the animator call interval in seconds.
 *
 * @details This function sets the time interval (in seconds) between animator ticks.
 *          At every tick the callback of every existing animator is called.
 *
 * @since_tizen 2.3
 *
 * @remarks Too small a value may cause performance issues and too high a
 *          value may cause your animation to look "jerky".
 *
 * @remarks The default @a frametime value is 1/30th of a second.
 *
 * @param[in] frametime The time in seconds between animator ticks
 *
 */
EAPI void ecore_animator_frametime_set(double frametime);

/**
 * @brief Gets the animator call interval in seconds.
 *
 * @details This function retrieves the time in seconds between animator ticks.
 *
 * @since_tizen 2.3
 *
 * @return The time in seconds between animator ticks
 *
 * @see ecore_animator_frametime_set()
 */
EAPI double ecore_animator_frametime_get(void);

/**
 * @brief Maps an input position from @c 0.0 to @c 1.0 along a timeline to a
 *        position in a different curve.
 *
 * @details This takes an input position (@c 0.0 to @c 1.0) and maps it to a new position (normally
 *          between @c 0.0 and @c 1.0, but it may go above/below @c 0.0 or @c 1.0 to show that it
 *          has "overshot" the mark) using some interpolation (mapping) algorithm.
 *
 * @since_tizen 2.3
 *
 * @remarks This function is useful to create non-linear animations. It offers a variety
 *          of possible animation curves to be used:
 * @li ECORE_POS_MAP_LINEAR - Linear, returns @a pos.
 * @li ECORE_POS_MAP_ACCELERATE - Start slow then speed up.
 * @li ECORE_POS_MAP_DECELERATE - Start fast then slow down.
 * @li ECORE_POS_MAP_SINUSOIDAL - Start slow, speed up then slow down at the end.
 * @li ECORE_POS_MAP_ACCELERATE_FACTOR - Start slow then speed up, v1 being a
 * power factor, @c 0.0 being linear, @c 1.0 being @c ECORE_POS_MAP_ACCELERATE, @c 2.0
 * being much more pronounced accelerate (squared), @c 3.0 being cubed, and so on.
 * @li ECORE_POS_MAP_DECELERATE_FACTOR - Start fast then slow down, v1 being a
 * power factor, @c 0.0 being linear, @c 1.0 being @c ECORE_POS_MAP_DECELERATE, @c 2.0
 * being much more pronounced decelerate (squared), @c 3.0 being cubed, and so on.
 * @li ECORE_POS_MAP_SINUSOIDAL_FACTOR - Start slow, speed up then slow down
 * at the end, v1 being a power factor, @c 0.0 being linear, @c 1.0 being
 * @c ECORE_POS_MAP_SINUSOIDAL, @c 2.0 being much more pronounced sinusoidal
 * (squared), @c 3.0 being cubed, and so on.
 * @li ECORE_POS_MAP_DIVISOR_INTERP - Start at gradient * v1, interpolated via
 * power of v2 curve.
 * @li ECORE_POS_MAP_BOUNCE - Start at @c 0.0 then "drop" like a ball bouncing to
 * the ground at @c 1.0, and bounce v2 times, with decay factor of v1.
 * @li ECORE_POS_MAP_SPRING - Start at @c 0.0 then "wobble" like a spring with rest
 * position @c 1.0, and wobble v2 times, with decay factor of v1
 * @remarks When not listed v1 and v2 have no effect.
 *
 * @image html ecore-pos-map.png
 * @image latex ecore-pos-map.eps "ecore pos map" width=\textwidth
 *
 * @remarks One way to use this would be:
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
 *          This makes an animation that bounces @c 7 diminish each time by a
 *          factor of @c 1.8.
 *
 * @param[in] pos The input position to map
 * @param[in] map The mapping to use
 * @param[in] v1 A parameter used by the mapping (pass @c 0.0 if not used)
 * @param[in] v2 A parameter used by the mapping (pass @c 0.0 if not used)
 * @return The mapped value
 *
 * @see _Ecore_Pos_Map
 *
 * @since 1.1.0
 */
EAPI double ecore_animator_pos_map(double pos, Ecore_Pos_Map map, double v1, double v2);

/**
 * @brief Sets the source of the animator ticks for the mainloop.
 *
 * @details This sets the source of the animator ticks. When an animator is active the
 *          mainloop will "tick" over frame by frame calling all animators that are
 *          registered until none are left. The mainloop ticks at a given rate based
 *          on the animator source. The default source is the system clock timer
 *          source - @c ECORE_ANIMATOR_SOURCE_TIMER. This source uses the system clock
 *          to tick over every N seconds (specified by ecore_animator_frametime_set(),
 *          with the default being 1/30th of a second unless set otherwise). You can
 *          set a custom tick source by setting the source to
 *          @c ECORE_ANIMATOR_SOURCE_CUSTOM and then driving it yourself based on some input
 *          tick source (like another application via ipc, some vertical blanking
 *          interrupt and so on) using ecore_animator_custom_source_tick_begin_callback_set() and
 *          ecore_animator_custom_source_tick_end_callback_set() to set the functions
 *          that are called to start and stop the ticking source, which when
 *          gets a "tick" should call ecore_animator_custom_tick() to make the "tick" over @c 1
 *          frame.
 *
 * @since_tizen 2.3
 *
 * @param[in] source The source of the animator ticks to use
 *
 */
EAPI void ecore_animator_source_set(Ecore_Animator_Source source);

/**
 * @brief Gets the animator source currently set.
 *
 * @details This gets the current animator source.
 *
 * @since_tizen 2.3
 *
 * @return The current animator source
 *
 * @see ecore_animator_source_set()
 */
EAPI Ecore_Animator_Source ecore_animator_source_get(void);

/**
 * @brief Sets the function that begins a custom animator tick source.
 *
 * @since_tizen 2.3
 *
 * @remarks The Ecore Animator infrastructure handles tracking of whether animators are needed
 *          and which ones need to be called and when, but when the tick source
 *          is custom, you have to provide a tick source by calling
 *          ecore_animator_custom_tick() to indicate that a frame tick happened. In order
 *          to allow the source of ticks to be dynamically enabled or disabled as
 *          needed, @a func when set is called to enable the tick source to
 *          produce tick events that call ecore_animator_custom_tick(). If @a func
 *          is @c NULL then no function is called to begin custom ticking.
 *
 * @param[in] func The function to call when ticking is to begin
 * @param[in] data The data passed to the tick begin function as its parameter
 *
 * @see ecore_animator_source_set()
 * @see ecore_animator_custom_source_tick_end_callback_set()
 * @see ecore_animator_custom_tick()
 */
EAPI void ecore_animator_custom_source_tick_begin_callback_set(Ecore_Cb func, const void *data);

/**
 * @brief Sets the function that ends a custom animator tick source.
 *
 * @since_tizen 2.3
 *
 * @remarks This function is a matching pair to the function set by
 *          ecore_animator_custom_source_tick_begin_callback_set() and is called
 *          when ticking is to stop. If @a func is @c NULL then no function is
 *          called to stop ticking. For more information see
 *          ecore_animator_custom_source_tick_begin_callback_set().
 *
 * @param[in] func The function to call when ticking is to end
 * @param[in] data The data passed to the tick end function as its parameter
 *
 * @see ecore_animator_source_set()
 * @see ecore_animator_custom_source_tick_begin_callback_set()
 * @see ecore_animator_custom_tick()
 */
EAPI void ecore_animator_custom_source_tick_end_callback_set(Ecore_Cb func, const void *data);

/**
 * @brief Triggers a custom animator tick.
 *
 * @since_tizen 2.3
 *
 * @remarks When animator source is set to @c ECORE_ANIMATOR_SOURCE_CUSTOM, then calling
 *          this function triggers a run of all animators currently registered with
 *          Ecore as this indicates that a "frame tick" happened. This does nothing if
 *          the animator source(set by ecore_animator_source_set()) is not set to
 *          @c ECORE_ANIMATOR_SOURCE_CUSTOM.
 *
 * @see ecore_animator_source_set()
 * @see ecore_animator_custom_source_tick_begin_callback_set
 * @see ecore_animator_custom_source_tick_end_callback_set()()
 */
EAPI void ecore_animator_custom_tick(void);

/**
 * @}
 */

/**
 * @defgroup Ecore_Job_Group Ecore Job
 * @ingroup Ecore_Main_Loop_Group
 *
 * @brief You can queue jobs that are to be done by the main loop when the
 *        current event is dealt with.
 *
 * Jobs are processed by the main loop in a manner which is similar to events. They
 * are also executed in the order in which they are added.
 *
 * A good use for them is when you don't want to execute an action
 * immediately, but want to give the control back to the main loop
 * so that it calls your job callback when jobs start being
 * processed (and if there are other jobs added before yours, they
 * are processed first). This also gives a chance to other
 * actions in your program to cancel the job before it is started.
 *
 * @{
 */

typedef struct _Ecore_Job Ecore_Job;    /**< @brief A job handle */

/**
 * @brief Add a job to the event queue.
 *
 * @since_tizen 2.3
 *
 * @remarks Once the job has been executed, the job handle is invalid.
 *
 * @param[in]   func The function to call when the job gets handled.
 * @param[in]   data Data pointer to be passed to the job function when the job is
 *               handled.
 * @return  The handle of the job.  @c NULL is returned if the job could not be
 *          added to the queue.
 */
EAPI Ecore_Job *ecore_job_add(Ecore_Cb func, const void *data);

/**
 * @brief Delete a queued job that has not yet been executed.
 *
 * @since_tizen 2.3
 *
 * @param[in]   job  Handle of the job to delete.
 * @return  The data pointer that was to be passed to the job.
 */
EAPI void *ecore_job_del(Ecore_Job *job);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
