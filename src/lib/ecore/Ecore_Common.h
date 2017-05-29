#ifdef __cplusplus
extern "C" {
#endif

#include <Efl_Config.h>

/**
 * @ingroup Ecore
 * @defgroup Ecore_Init_Group Ecore initialization, shutdown functions and reset on fork.
 * @{
 */

/**
 * Sets up connections, signal handlers, sockets etc.
 * @return @c 1 or greater on success, @c 0 otherwise.
 *
 * This function sets up all singal handlers and the basic event loop. If it
 * succeeds, @c 1 will be returned, otherwise @c 0 will be returned.
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
 * This function is affected by some environment variables:
 *
 *  @li @c ECORE_NO_SYSTEM_MODULES=1 may be used to temporarily
 *      disable system modules, often useful for debug.
 *
 *  @li @c ECORE_FPS_DEBUG=1 prints frames per second, usefult to
 *      detect lags and blocking calls.
 *
 *  @li @c ECORE_MEM_STAT=1 will generate @c ecore_mem_stat.${PID}
 *      file with memory statistics.
 *
 *  @li @c ECORE_ERROR_ABORT=1 will abort on errors.
 *
 * This function will call eina_init(), so other environment variables
 * may apply.
 */
EAPI int ecore_init(void);

/**
 * Shuts down connections, signal handlers sockets etc.
 *
 * @return @c 0 if ecore shuts down, greater than @c 0 otherwise.
 * This function shuts down all things set up in ecore_init() and cleans up all
 * event queues, handlers, filters, timers, idlers, idle enterers/exiters
 * etc. set up after ecore_init() was called.
 *
 * Do not call this function from any callback that may be called from the main
 * loop, as the main loop will then fall over and not function properly.
 */
EAPI int ecore_shutdown(void);

#ifdef EFL_BETA_API_SUPPORT
/**
 * @brief Inform EFL of the version this application was built for.
 *
 * This is transparently called from $EFL_MAIN().
 *
 * @since 1.18 (as beta)
 */
EWAPI void efl_build_version_set(int vmaj, int vmin, int vmic, int revision, const char *flavor, const char *build_id);
#endif

/**
 * @}
 */

/**
 * @ingroup Ecore
 * @defgroup Ecore_Main_Loop_Group Ecore main loop
 *
 * This group discusses functions that are acting on Ecore's main loop itself or
 * on events and infrastructure directly linked to it. Most programs only need
 * to start and end the main loop, the rest of the functions discussed here are
 * meant to be used in special situations, and with great care.
 *
 * For details on the usage of ecore's main loop and how it interacts with other
 * ecore facilities see: @ref Ecore_Main_Loop_Page.
 *
 * @{
 */

#define ECORE_VERSION_MAJOR EFL_VERSION_MAJOR /**< Ecore version major number */
#define ECORE_VERSION_MINOR EFL_VERSION_MINOR /**< Ecore version minor number */

/**
 * @typedef Ecore_Version
 *
 * This is the Ecore version information structure that can be used at
 * runtime to detect which version of ecore is being used and adapt
 * appropriately as follows for example:
 *
 * @code
 * #if defined(ECORE_VERSION_MAJOR) && (ECORE_VERSION_MAJOR >= 1) && defined(ECORE_VERSION_MINOR) && (ECORE_VERSION_MINOR > 0)
 * printf("Ecore version: %i.%i.%i\n",
 *        ecore_version->major,
 *        ecore_version->minor,
 *        ecore_version->micro);
 * if (ecore_version->revision > 0)
 *   {
 *     printf("  Built from Git revision # %i\n", ecore_version->revision);
 *   }
 * #endif
 * @endcode
 *
 */
typedef struct _Ecore_Version
{
   int major; /** < major (binary or source incompatible changes) */
   int minor; /** < minor (new features, bugfixes, major improvements version) */
   int micro; /** < micro (bugfix, internal improvements, no new features version) */
   int revision; /** < git revision (0 if a proper release or the git revision number Ecore is built from) */
} Ecore_Version;

EAPI extern Ecore_Version *ecore_version;

#define ECORE_CALLBACK_CANCEL  EINA_FALSE /**< Return value to remove a callback */
#define ECORE_CALLBACK_RENEW   EINA_TRUE /**< Return value to keep a callback */

#define ECORE_CALLBACK_PASS_ON EINA_TRUE /**< Return value to pass event to next handler */
#define ECORE_CALLBACK_DONE    EINA_FALSE /**< Return value to stop event handling */

/**
 * @typedef Ecore_Task_Cb Ecore_Task_Cb
 * A callback run for a task (timer, idler, poller, animator, etc).
 */
typedef Eina_Bool (*Ecore_Task_Cb)(void *data);

/**
 * @typedef Ecore_Select_Function
 * A function which can be used to replace select() in the main loop.
 */
typedef int (*Ecore_Select_Function)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

/**
 * Runs a single iteration of the main loop to process everything on the
 * queue.
 *
 * It does everything that is already done inside an @c Ecore main loop, like
 * checking for expired timers, idlers, etc. But it will do it only once and
 * return, instead of keeping watch for new events.
 *
 * DO NOT use this function unless you are the person God comes to ask for
 * advice when He has trouble managing the Universe.
 *
 * @see ecore_main_loop_iterate_may_block()
 */
EAPI void ecore_main_loop_iterate(void);

/**
 * Runs a single iteration of the main loop to process everything on the
 * queue with blocking/non-blocking status.
 *
 * @param may_block A flag if the main loop has a possibility of blocking.
 * (@c EINA_TRUE = may block/@c EINA_FALSE = non block)
 *
 * This is an extension API for ecore_main_loop_iterate() with additional
 * parameter. It does everything that is already done inside an
 * @c Ecore main loop, like checking for expired timers, idlers, etc. But it
 * will do it only once and return, instead of keep watching for new events.
 *
 * DO NOT use this function unless you are the person God comes to ask for
 * advice when He has trouble managing the Universe.
 *
 * @return @c 1 if event exists, else @c 0.
 * @see ecore_main_loop_iterate()
 */
EAPI int ecore_main_loop_iterate_may_block(int may_block);

/**
 * Sets the function to use when monitoring multiple file descriptors,
 * and waiting until one of more of the file descriptors are ready
 * for some class of I/O operation.
 *
 * This function will be used instead of the system call select and
 * could possibly be used to integrate the Ecore event loop with an
 * external event loop.
 *
 * @warning you don't know how to use, don't even try to use it.
 *
 * @param func The function to be used.
 */
EAPI void ecore_main_loop_select_func_set(Ecore_Select_Function func);

/**
 * Gets the select function set by ecore_select_func_set(),
 * or the native select function if none was set.
 *
 */
EAPI Ecore_Select_Function ecore_main_loop_select_func_get(void);

/**
 * Requests ecore to integrate GLib's main loop.
 *
 * This will add a small overhead during every main loop interaction
 * by checking GLib's default main context (used by its main loop).
 * If it have events to be checked (timers, file descriptors or idlers),
 * then these will be polled alongside with Ecore's own events, then
 * dispatched before Ecore's. This is done by calling
 * ecore_main_loop_select_func_set().
 *
 * This will cooperate with previously set
 * ecore_main_loop_select_func_set() by calling the old function.
 * Similarly, if you want to override
 * ecore_main_loop_select_func_set() after main loop is integrated,
 * call the new select function set by this call (get it by calling
 * ecore_main_loop_select_func_get() right after
 * ecore_main_loop_glib_integrate()).
 *
 * This is useful to use GMainLoop libraries, like GTK, GUPnP,
 * LibSoup, GConf and more. Adobe Flash plugin and other plugins
 * systems depend on this as well.
 *
 * Once initialized/integrated, it will be valid until Ecore is
 * completely shut down.
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
 * @return @c EINA_TRUE on success of,
 *         @c EINA_FALSE if it failed likely due to no GLib support in Ecore.
 * @note This is only available if Ecore was compiled with GLib support.
 * @note You don't need to call this function if Ecore was compiled with.
 * --with-glib=always.
 *
 */
EAPI Eina_Bool ecore_main_loop_glib_integrate(void);

/**
 * Disables always integrating GLib.
 *
 * If ecore is compiled with --with-glib=always (to always call
 * ecore_main_loop_glib_integrate() when ecore_init() is called), then calling
 * this before calling ecore_init() will disable the integration.
 * This is for apps that explicitly do not want this to happen for whatever
 * reasons they may have.
 */
EAPI void ecore_main_loop_glib_always_integrate_disable(void);

/**
 * Runs the application main loop.
 *
 * This function will not return until @ref ecore_main_loop_quit is called. It
 * will check for expired timers, idlers, file descriptors being watched by fd
 * handlers, etc. Once everything is done, before entering again on idle state,
 * any callback set as @c Idle_Enterer will be called.
 *
 * This function should be called once only from the same thread that
 * initted ecore, eina etc. (ecore_init(), eina_init(), ...) and should never
 * be nested. Never call it from within an instance of itself.
 *
 * Each main loop iteration is done by calling ecore_main_loop_iterate()
 * internally.
 *
 * The polling (select) function used can be changed with
 * ecore_main_loop_select_func_set().
 *
 * The function used to check for file descriptors, events, and that has a
 * timeout for the timers can be changed using
 * ecore_main_loop_select_func_set().
 */
EAPI void ecore_main_loop_begin(void);

/**
 * Quits the main loop once all the events currently on the queue have
 * been processed.
 *
 * This function returns immediately, but will mark the ecore_main_loop_begin()
 * function to return at the end of the current main loop iteration.
 */
EAPI void ecore_main_loop_quit(void);

/**
 * Returns if an animator has ticked off during this loop iteration.
 *
 * @return @c EINA_TRUE if an animator has been called, @c EINA_FALSE otherwise.
 *
 * There should be little need for anyone to use this - ever.
 *
 * @since 1.9
 */
EAPI Eina_Bool ecore_main_loop_animator_ticked_get(void);

/**
 * Returns if the ecore_main_loop is running.
 *
 * @return An integer specifying if the ecore_main_loop is running,
 *         @c 0 if not running,
 *         > @c 0 if running
 *
 * @since 1.13
 */
EAPI int ecore_main_loop_nested_get(void);

/**
 * @typedef Ecore_Cb Ecore_Cb
 * A generic callback called as a hook when a certain point in
 * execution is reached.
 */
typedef void (*Ecore_Cb)(void *data);

/**
 * @typedef Ecore_Data_Cb Ecore_Data_Cb
 * A callback which is used to return data to the main function
 */
typedef void *(*Ecore_Data_Cb)(void *data);

/**
 * Adds a function to be called by ecore_fork_reset().
 *
 * @param func The function to add.
 * @param data The data to pass to this function.
 *
 * This queues @p func to be called (and passed @p data as its argument) when
 * ecore_fork_reset() is called. This allows other libraries and subsystems
 * to also reset their internal state after a fork.
 *
 * @return @c EINA_TRUE on success, else @c EINA_FALSE.
 * @since 1.7
 */
EAPI Eina_Bool ecore_fork_reset_callback_add(Ecore_Cb func, const void *data);

/**
 * This removes the callback specified.
 *
 * @param func The function to delete.
 * @param data The data passed to this function.
 *
 * This deletes the callback added by ecore_fork_reset_callback_add() using
 * the function and data pointer to specify which to remove.
 *
 * @return @c EINA_TRUE on success, else @c EINA_FALSE.
 * @since 1.7
 */
EAPI Eina_Bool ecore_fork_reset_callback_del(Ecore_Cb func, const void *data);

/**
 * Resets the ecore internal state after a fork.
 *
 * Ecore maintains internal data that can be affected by the fork() system call
 * which creates a duplicate of the current process. This also duplicates
 * file descriptors which is problematic in that these file descriptors still
 * point to their original sources. This function makes ecore reset internal
 * state (e.g. pipes used for signalling between threads) so they function
 * correctly afterwards.
 *
 * It is highly suggested that you call this function after any fork()
 * system call inside the child process if you intend to use ecore features
 * after this point and not call any exec() family functions. Not doing so
 * will cause possible misbehaviour.
 *
 * @since 1.7
 */
EAPI void ecore_fork_reset(void);

/**
 * @brief Calls callback asynchronously in the main loop.
 * @since 1.1.0
 *
 * @param callback The callback to call in the main loop
 * @param data The data to give to that call back
 *
 * For all calls that need to happen in the main loop (most EFL functions do),
 * this helper function provides the infrastructure needed to do it safely
 * by avoiding dead lock, race condition and properly wake up the main loop.
 *
 * Remember after that function call, you should never touch again the @p data
 * in the thread, it is owned by the main loop and your callback should take
 * care of freeing it if necessary.
 */
EAPI void ecore_main_loop_thread_safe_call_async(Ecore_Cb callback, void *data);

/**
 * @brief Calls callback synchronously in the main loop.
 * @since 1.1.0
 *
 * @param callback The callback to call in the main loop
 * @param data The data to give to that call back
 * @return The value returned by the callback in the main loop.
 *
 * For all calls that need to happen in the main loop (most EFL functions do),
 * this helper function provides the infrastructure needed to do it safely
 * by avoiding dead lock, race condition and properly wake up the main loop.
 *
 * Remember this function will block until the callback is executed in the
 * main loop. It can take time and you have no guaranty about the timeline.
 */
EAPI void *ecore_main_loop_thread_safe_call_sync(Ecore_Data_Cb callback, void *data);

/**
 * @brief Waits for the next thread call in the main loop.
 * @since 1.13.0
 *
 * @param wait How long to wait for this callback to be called.
 *
 * Note: This function should only be called in the main loop
 * and will actually block the main loop until either a call
 * is triggered from a thread or the time specified by wait has
 * passed.
 */
EAPI void ecore_main_loop_thread_safe_call_wait(double wait);

/**
 * @brief This function suspends the main loop in a know state.
 * @since 1.1.0
 *
 * @result The number of time ecore_thread_main_loop_begin() has been called
 * in this thread, if the main loop was suspended correctly. If not, it return @c -1.
 *
 * This function suspend the main loop in a know state, this let you
 * use any EFL call you want after it return. Be carefully, the main loop
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
 * @brief Unlocks the main loop.
 * @since 1.1.0
 *
 * @result The number of time ecore_thread_main_loop_end() need to be called before
 * the main loop is unlocked again. @c -1 will be returned if you are trying to unlock
 * when there wasn't enough call to ecore_thread_main_loop_begin().
 *
 * After a call to ecore_thread_main_loop_begin(), you need to absolutely
 * call ecore_thread_main_loop_end(), or you application will stay frozen.
 */
EAPI int ecore_thread_main_loop_end(void);

/**
 * @}
 */

/**
 * @ingroup Ecore_Main_Loop_Group
 * @defgroup Ecore_Event_Group Ecore Event functions
 *
 * Ecore events provide two main features that are of use to those using ecore:
 * creating events and being notified of events. Those two will usually be used
 * in different contexts, creating events is mainly done by libraries wrapping
 * some system functionality while being notified of events is mainly a
 * necessity of applications.
 *
 * For a program to be notified of events it's interested in it needs to have a
 * function to process the event and to register that function as the callback
 * to the event, that's all:
 * @code
 * ecore_event_handler_add(EVENT_TYPE, _my_event_handler, some_data);
 * ...
 * static Eina_Bool
 * _my_event_handler(void *data, int type, void *event)
 * {
 *    //data is some_data
 *    //event is provided by whoever created the event
 *    //Do really cool stuff with event
 * }
 * @endcode
 *
 * One very important thing to note here is the @c EVENT_TYPE, to register a
 * handler for an event you must know its type before hand. Ecore provides
 * the following events which are emitted in response to POSIX
 * signals(https://en.wikipedia.org/wiki/Signal_%28computing%29):
 * @li @b ECORE_EVENT_SIGNAL_USER
 * @li @b ECORE_EVENT_SIGNAL_HUP
 * @li @b ECORE_EVENT_SIGNAL_POWER
 * @li @b ECORE_EVENT_SIGNAL_EXIT
 *
 * @warning Don't override these using the @c signal or @c sigaction calls.
 * These, however, aren't the only signals one can handle. Many
 * libraries(including ecore modules) have their own signals that can be
 * listened for and handled, to do that one only needs to know the type of the
 * event. This information can be found on the documentation of the library
 * emitting the signal, so, for example, for events related to windowing one
 * would look in @ref Ecore_Evas_Group.
 *
 * Examples of libraries that integrate into ecore's main loop by providing
 * events are @ref Ecore_Con_Group, @ref Ecore_Evas_Group and @ref
 * Ecore_Exe_Group, amongst others. This usage can be divided into two parts,
 * setup and adding events. The setup is very simple, all that needs doing is
 * getting a type id for the event:
 * @code
 * int MY_EV_TYPE = ecore_event_type_new();
 * @endcode
 * @note This variable should be declared in the header since it'll be needed by
 * anyone wishing to register a handler to your event.
 *
 * The complexity of adding of an event to the queue depends on whether that
 * event sends uses @c event, if it doesn't it a one-liner:
 * @code
 * ecore_event_add(MY_EV_TYPE, NULL, NULL, NULL);
 * @endcode
 * The usage when an @c event is needed is not that much more complex and can be
 * seen in @ref ecore_event_add.
 *
 * Examples that deals with events:
 * @li @ref ecore_event_example_01_c
 * @li @ref ecore_event_example_02_c
 *
 * @{
 */

#define ECORE_EVENT_NONE            0 /**< None event */
#define ECORE_EVENT_SIGNAL_USER     1 /**< User signal event */
#define ECORE_EVENT_SIGNAL_HUP      2 /**< Hup signal event */
#define ECORE_EVENT_SIGNAL_EXIT     3 /**< Exit signal event */
#define ECORE_EVENT_SIGNAL_POWER    4 /**< Power signal event */
#define ECORE_EVENT_SIGNAL_REALTIME 5 /**< Realtime signal event */
#define ECORE_EVENT_MEMORY_STATE             6 /**< Memory state changed, see ecore_memory_state_get() */
#define ECORE_EVENT_POWER_STATE              7 /**< Power state changed, see ecore_power_state_get() */
#define ECORE_EVENT_LOCALE_CHANGED           8 /**< Locale changed */
#define ECORE_EVENT_HOSTNAME_CHANGED         9 /**< Hostname changed */
#define ECORE_EVENT_SYSTEM_TIMEDATE_CHANGED 10 /**< Time or Date changed */
#define ECORE_EVENT_COUNT                   11 /**< Number of events */

typedef struct _Ecore_Win32_Handler         Ecore_Win32_Handler;    /**< A handle for HANDLE handlers on Windows */
typedef struct _Ecore_Event_Handler         Ecore_Event_Handler;    /**< A handle for an event handler */
typedef struct _Ecore_Event_Filter          Ecore_Event_Filter;    /**< A handle for an event filter */
typedef struct _Ecore_Event                 Ecore_Event;    /**< A handle for an event */
typedef struct _Ecore_Event_Signal_User     Ecore_Event_Signal_User;    /**< User signal event */
typedef struct _Ecore_Event_Signal_Hup      Ecore_Event_Signal_Hup;    /**< Hup signal event */
typedef struct _Ecore_Event_Signal_Exit     Ecore_Event_Signal_Exit;    /**< Exit signal event */
typedef struct _Ecore_Event_Signal_Power    Ecore_Event_Signal_Power;    /**< Power signal event */
typedef struct _Ecore_Event_Signal_Realtime Ecore_Event_Signal_Realtime;    /**< Realtime signal event */

/**
 * @typedef Ecore_Filter_Cb
 * A callback used for filtering events from the main loop.
 */
typedef Eina_Bool (*Ecore_Filter_Cb)(void *data, void *loop_data, int type, void *event);

/**
 * @typedef Ecore_End_Cb Ecore_End_Cb
 * This is the callback which is called at the end of a function,
 * usually for cleanup purposes.
 */
typedef void (*Ecore_End_Cb)(void *user_data, void *func_data);

/**
 * @typedef Ecore_Event_Handler_Cb Ecore_Event_Handler_Cb
 * A callback used by the main loop to handle events of a specified
 * type.
 */
typedef Eina_Bool (*Ecore_Event_Handler_Cb)(void *data, int type, void *event);

/**
 * @struct _Ecore_Event_Signal_User
 * @brief A structure that stores information of a User signal event.
 */
struct _Ecore_Event_Signal_User
{
   int       number;  /**< The signal number. Either 1 or 2 */
   void     *ext_data;  /**< Extension data - not used */

#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t data; /**< Signal info */
#endif
};

/**
 * @struct _Ecore_Event_Signal_Hup
 * @brief A structure that stores information of a Hup signal event.
 */
struct _Ecore_Event_Signal_Hup
{
   void     *ext_data;  /**< Extension data - not used */

#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t data; /**< Signal info */
#endif
};

/**
 * @struct _Ecore_Event_Signal_Exit
 * @brief A structure that stores information of an Exit request event.
 */
struct _Ecore_Event_Signal_Exit
{
   Eina_Bool interrupt : 1; /**< Set if the exit request was an interrupt  signal*/
   Eina_Bool quit      : 1; /**< set if the exit request was a quit signal */
   Eina_Bool terminate : 1; /**< Set if the exit request was a terminate signal */
   void     *ext_data; /**< Extension data - not used */

#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t data; /**< Signal info */
#endif
};

/**
 * @struct _Ecore_Event_Signal_Power
 * @brief A structure that stores information of a Power event.
 */
struct _Ecore_Event_Signal_Power
{
   void     *ext_data;  /**< Extension data - not used */

#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t data; /**< Signal info */
#endif
};

/**
 * @struct _Ecore_Event_Signal_Realtime
 * @brief A structure that stores information of a Realtime event.
 */
struct _Ecore_Event_Signal_Realtime
{
   int       num; /**< The realtime signal's number */

#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t data; /**< Signal info */
#endif
};

/**
 * @brief Adds an event handler.
 * @param type The type of the event this handler will get called for
 * @param func The function to call when the event is found in the queue
 * @param data A data pointer to pass to the called function @p func
 * @return A new Event handler, or @c NULL on failure.
 *
 * Add an event handler to the list of handlers. This will, on success, return
 * a handle to the event handler object that was created, that can be used
 * later to remove the handler using ecore_event_handler_del(). The @p type
 * parameter is the integer of the event type that will trigger this callback
 * to be called. The callback @p func is called when this event is processed
 * and will be passed the event type, a pointer to the private event
 * structure that is specific to that event type, and a data pointer that is
 * provided in this call as the @p data parameter.
 *
 * When the callback @p func is called, it must return @c 1 or @c 0. If it returns
 * @c 1 (or @c ECORE_CALLBACK_PASS_ON), It will keep being called as per normal, for
 * each handler set up for that event type. If it returns @c 0 (or
 * @c ECORE_CALLBACK_DONE), it will cease processing handlers for that particular
 * event, so all handler set to handle that event type that have not already
 * been called, will not be.
 */
EAPI Ecore_Event_Handler *ecore_event_handler_add(int type, Ecore_Event_Handler_Cb func, const void *data);

/**
 * @brief Deletes an event handler.
 * @param event_handler Event handler handle to delete
 * @return Data passed to handler
 *
 * Delete a specified event handler from the handler list. On success this will
 * delete the event handler and return the pointer passed as @p data when the
 * handler was added by ecore_event_handler_add(). On failure @c NULL will be
 * returned. Once a handler is deleted it will no longer be called.
 */
EAPI void *ecore_event_handler_del(Ecore_Event_Handler *event_handler);

/**
 * @brief Adds an event to the event queue.
 * @param type The event type to add to the end of the event queue
 * @param ev The data structure passed as @c event to event handlers
 * @param func_free The function to be called to free @a ev
 * @param data The data pointer to be passed to the free function
 * @return A Handle for that event on success, otherwise NULL
 *
 * If it succeeds, an event of type @a type will be added to the queue for
 * processing by event handlers added by ecore_event_handler_add(). The @a ev
 * parameter will be passed as the @c event parameter of the handler. When the
 * event is no longer needed, @a func_free will be called and passed @a ev for
 * cleaning up. If @p func_free is NULL, free() will be called with the private
 * structure pointer.
 */
EAPI Ecore_Event *ecore_event_add(int type, void *ev, Ecore_End_Cb func_free, void *data);

/**
 * @brief Deletes an event from the queue.
 * @param event The event handle to delete
 * @return The data pointer originally set for the event free function
 *
 * This deletes the event @p event from the event queue, and returns the
 * @p data parameter originally set when adding it with ecore_event_add(). This
 * does not immediately call the free function, and it may be called later on
 * cleanup, and so if the free function depends on the data pointer to work,
 * you should defer cleaning of this till the free function is called later.
 */
EAPI void *ecore_event_del(Ecore_Event *event);

/**
 * @brief Gets the data associated with an #Ecore_Event_Handler.
 * @param eh The event handler
 * @return The data
 *
 * This function returns the data previously associated with @p eh by
 * ecore_event_handler_add().
 */
EAPI void *ecore_event_handler_data_get(Ecore_Event_Handler *eh);

/**
 * @brief Sets the data associated with an #Ecore_Event_Handler.
 * @param eh The event handler
 * @param data The data to associate
 * @return The previous data
 *
 * This function sets @p data to @p eh and returns the old data pointer
 * which was previously associated with @p eh by ecore_event_handler_add().
 */
EAPI void *ecore_event_handler_data_set(Ecore_Event_Handler *eh, const void *data);

/**
 * @brief Allocates a new event type id sensibly and returns the new id.
 * @return A new event type id.
 *
 * This function allocates a new event type id and returns it. Once an event
 * type has been allocated it can never be de-allocated during the life of
 * the program. There is no guarantee of the contents of this event ID, or how
 * it is calculated, except that the ID will be unique to the current instance
 * of the process.
 */
EAPI int ecore_event_type_new(void);

/**
 * @brief Forcefully flush all pending type without processing them
 * @param Serie of Ecore_Event finished by ECORE_EVENT_NONE.
 *
 * This function is to be called before calling ecore_shutdown() if any event
 * has still a chance to be in the ecore event queue.
 */
EAPI void ecore_event_type_flush_internal(int type, ...);

/**
 * @brief Forcefully flush all pending type without processing them
 * @param Serie of Ecore_Event.
 *
 * This function is to be called before calling ecore_shutdown() if any event
 * has still a chance to be in the ecore event queue.
 */
#define ecore_event_type_flush(...)                                     \
  ecore_event_type_flush_internal(__VA_ARGS__, ECORE_EVENT_NONE);


/**
 * @brief Adds a filter the current event queue.
 *
 * @param func_start Function to call just before filtering and return data
 * @param func_filter Function to call on each event
 * @param func_end Function to call after the queue has been filtered
 * @param data Data to pass to the filter functions
 * @return A filter handle on success, @c NULL otherwise.
 *
 * Adds a callback to filter events from the event queue. Filters are called on
 * the queue just before Event handler processing to try and remove redundant
 * events. Just as processing is about to start @a func_start is called and
 * passed the @a data pointer, the return value of this functions is passed to
 * @a func_filter as loop_data. @a func_filter is also passed @a data and the
 * event type and event structure. If this @a func_filter returns
 * @c EINA_FALSE, the event is removed from the queue, if it returns
 * @c EINA_TRUE, the event is kept. When processing is finished @p func_end is
 * called and is passed the loop_data(returned by @c func_start) and @p data
 * pointer to clean up.
 */
EAPI Ecore_Event_Filter *ecore_event_filter_add(Ecore_Data_Cb func_start, Ecore_Filter_Cb func_filter, Ecore_End_Cb func_end, const void *data);

/**
 * @brief Deletes an event filter.
 * @param ef The event filter handle
 * @return The data set for the filter on success, @c NULL otherwise.
 *
 * Delete a filter that has been added by its @p ef handle.
 */
EAPI void *ecore_event_filter_del(Ecore_Event_Filter *ef);

/**
 * @brief Returns the current event type being handled.
 * @return The current event type being handled if inside a handler callback,
 * @c ECORE_EVENT_NONE otherwise.
 *
 * If the program is currently inside an Ecore event handler callback this
 * will return the type of the current event being processed.
 *
 * This is useful when certain Ecore modules such as Ecore_Evas "swallow"
 * events and not all the original information is passed on. In special cases
 * this extra information may be useful or needed and using this call can let
 * the program know if the event type being handled is one it wants to get more
 * information about.
 */
EAPI int ecore_event_current_type_get(void);

/**
 * @brief Returns the current event type pointer handled.
 * @return The current event pointer being handled if inside a handler callback,
 * @c NULL otherwise.
 *
 * If the program is currently inside an Ecore event handler callback this
 * will return the pointer of the current event being processed.
 *
 * This is useful when certain Ecore modules such as Ecore_Evas "swallow"
 * events and not all the original information is passed on. In special cases
 * this extra information may be useful or needed and using this call can let
 * the program access the event data if the type of the event is handled by
 * the program.
 */
EAPI void *ecore_event_current_event_get(void);

/**
 * @}
 */

/**
 * @ingroup Ecore_Event_Group
 * @defgroup Ecore_System_Events System Events
 *
 * Ecore is aware of some system events that one may be interested, they are described below:
 *
 * @li #ECORE_EVENT_MEMORY_STATE indicates system changed its free
 *     memory status, going to or being back from "low memory"
 *     state. When going to low memory state the libraries and
 *     applications may help the system by reducing memory usage,
 *     dropping caches and unused resources. The event carries no
 *     information, the current state should be queried with
 *     ecore_low_memory_get().
 * @li #ECORE_EVENT_POWER_STATE indicates system changed its battery
 *     level, going to or being back from "low battery" state. When
 *     going to low battery state the libraries and applications may
 *     help the system by reducing number of wake ups and processing,
 *     increasing @ref Ecore_Animator frame time values, reducing the
 *     number of @ref Ecore_Timer and @ref Ecore_Idler or even going
 *     to extreme of trading quality over speed to finish tasks
 *     sooner. The event carries no information, the current state
 *     should be queried with ecore_low_battery_get().
 * @li #ECORE_EVENT_LOCALE_CHANGED indicates the system locale and/or
 *     language changed. This event carries no information and there
 *     is no ecore specific call to get the new locale. It is advised
 *     that the emitter of this event to set the new locale, for
 *     instance in POSIX one should call setlocale() before adding the
 *     event. Libraries and applications should then reload their
 *     resources and reformat their strings to use the new values.
 * @li #ECORE_EVENT_HOSTNAME_CHANGED indicates the system hostname
 *     changed. This event carries no information and the new value
 *     should be queried with platform specific calls, such as
 *     gethostname() on POSIX.
 * @li #ECORE_EVENT_SYSTEM_TIMEDATE_CHANGED indicates the system time
 *     or date changed. This may happen as result of Daylight Saving
 *     changes, NTP fixing the clock, changing timezones or user
 *     setting a new date manually. This event carries no information
 *     and the new value should be queried with ecore_time_unix_get()
 *     or platform specific such as gettimeofday()
 * @{
 */

/**
 * @enum _Ecore_Memory_State
 * Indicates current system memory state.
 * @since 1.8
 */
enum _Ecore_Memory_State    /* Memory state */
{
   ECORE_MEMORY_STATE_NORMAL, /**< The normal memory usage state. No need to do anything special here - operation as normal. */
   ECORE_MEMORY_STATE_LOW /**< The system is low on memory resources. This would indicate that it may be a good idea to free memory you don't need and minimize footprint to avoid general system problems. */
};
typedef enum _Ecore_Memory_State Ecore_Memory_State;

/**
 * @brief Gets the current status of memory on the system.
 * @return The current memory state for the system as a whole.
 * @since 1.8
 */
EAPI Ecore_Memory_State ecore_memory_state_get(void);

/**
 * @brief Sets the memory state.
 * @param state The memory state to set.
 *
 * This function will store information about the current
 * memory state and if it changed will automatically create an
 * #ECORE_EVENT_MEMORY_STATE event.
 *
 * @note This function should not be called by user, instead a
 * monitoring entity that is system dependent. Usually an ecore module
 * that is platform-specific.
 *
 * @since 1.8
 */
EAPI void ecore_memory_state_set(Ecore_Memory_State state);


/**
 * @enum _Ecore_Power_State
 * Indicates current system memory state.
 * @since 1.8
 */
enum _Ecore_Power_State    /* Power state */
{
   ECORE_POWER_STATE_MAINS, /**< The system is connected to a mains supply of power, thus there is no need to limit processing to save battery life at all. */
   ECORE_POWER_STATE_BATTERY, /**< The system is running off battery power, but is otherwise running normally. */
   ECORE_POWER_STATE_LOW, /**< The system is low on power (on battery) and the process should do its best to conserve power. For example it may reduce or suspend polling of network resources, turn off animations or reduce framerate etc. */
   //ECORE_POWER_STATE_CRITICAL, /**< The system is very low on power (on battery) and the process should begin taking even more conservative action @since 1.13*/
   //ECORE_POWER_STATE_EMERGENCY /**< The system is extremely low on power (on battery) and the process should prepare for suspend/hibernate/power loss @since 1.13 */
};
typedef enum _Ecore_Power_State Ecore_Power_State;

/**
 * @brief Gets the current power state.
 * @return The current power state for the system.
 * @since 1.8
 */
EAPI Ecore_Power_State ecore_power_state_get(void);

/**
 * @brief Sets the power state.
 * @param state The power state to set.
 *
 * This function will store information about the current power
 * state and if it changed will automatically create an
 * #ECORE_EVENT_POWER_STATE event.
 *
 * @note This function should not be called by user, instead a
 * monitoring entity that is system dependent. Usually an ecore module
 * that is platform-specific.
 *
 * @since 1.8
 */
EAPI void ecore_power_state_set(Ecore_Power_State state);

/**
 * @}
 */



/**
 * @ingroup Ecore_Main_Loop_Group
 * @defgroup Ecore_Exe_Group Process Spawning Functions
 *
 * This module is responsible for managing portable processes using Ecore.
 * With this module you're able to spawn processes and you also can pause,
 * quit your spawned processes.
 * An interaction between your process and those spawned is possible
 * using pipes or signals.
 *
 * Example
 * @li @ref Ecore_exe_simple_example_c
 *
 *
 * @{
 */

/** Inherit priority from parent process */
#define ECORE_EXE_PRIORITY_INHERIT 9999

EAPI extern int ECORE_EXE_EVENT_ADD;     /**< A child process has been added */
EAPI extern int ECORE_EXE_EVENT_DEL;     /**< A child process has been deleted (it exited, naming consistent with the rest of ecore). */
EAPI extern int ECORE_EXE_EVENT_DATA;    /**< Data from a child process. */
EAPI extern int ECORE_EXE_EVENT_ERROR;    /**< Errors from a child process. */

/**
 * @enum _Ecore_Exe_Win32_Priority
 * Defines the priority of the process.
 */
enum _Ecore_Exe_Win32_Priority
{
   ECORE_EXE_WIN32_PRIORITY_IDLE, /**< Idle priority, for monitoring the system */
   ECORE_EXE_WIN32_PRIORITY_BELOW_NORMAL, /**< Below default priority */
   ECORE_EXE_WIN32_PRIORITY_NORMAL, /**< Default priority */
   ECORE_EXE_WIN32_PRIORITY_ABOVE_NORMAL, /**< Above default priority */
   ECORE_EXE_WIN32_PRIORITY_HIGH, /**< High priority, use with care as other threads in the system will not get processor time */
   ECORE_EXE_WIN32_PRIORITY_REALTIME     /**< Realtime priority, should be almost never used as it can interrupt system threads that manage mouse input, keyboard input, and background disk flushing */
};
typedef enum _Ecore_Exe_Win32_Priority Ecore_Exe_Win32_Priority;

#ifdef EFL_BETA_API_SUPPORT
#include "ecore_exe.eo.h"
#endif
#include "ecore_exe.eo.legacy.h"

#define _ECORE_EXE_EO_CLASS_TYPE

/**
 * @typedef Ecore_Exe_Cb Ecore_Exe_Cb
 * A callback to run with the associated @ref Ecore_Exe, usually
 * for cleanup purposes.
 */
typedef void                            (*Ecore_Exe_Cb)(void *data, const Ecore_Exe *exe);

typedef struct _Ecore_Exe_Event_Add       Ecore_Exe_Event_Add; /**< Spawned Exe add event */
typedef struct _Ecore_Exe_Event_Del       Ecore_Exe_Event_Del; /**< Spawned Exe exit event */

/**
 * @struct _Ecore_Exe_Event_Add
 * @brief Definition for a structure that stores information of a Process add event.
 */
struct _Ecore_Exe_Event_Add
{
   Ecore_Exe *exe; /**< The handle to the added process */
   void      *ext_data; /**< Extension data - not used */
};

/**
 * @struct _Ecore_Exe_Event_Del
 * @brief Definition for a structure that stores information of a Process exit event.
 */
struct _Ecore_Exe_Event_Del
{
   pid_t      pid; /**< The process ID of the process that exited */
   int        exit_code; /**< The exit code of the process */
   Ecore_Exe *exe; /**< The handle to the exited process, or @c NULL if not found */
   int        exit_signal; /**< The signal that caused the process to exit */
   Eina_Bool  exited    : 1; /**< Set to 1 if the process exited of its own accord */
   Eina_Bool  signalled : 1; /**< Set to 1 if the process exited due to uncaught signal */
   void      *ext_data; /**< Extension data - not used */
#if !defined (_WIN32) && !defined (__lv2ppu__) && !defined (EXOTIC_NO_SIGNAL)
   siginfo_t  data; /**< Signal info */
#endif
};

/**
 * Sets the priority at which to launch processes.
 *
 * This sets the priority of processes run by ecore_exe_run() and
 * ecore_exe_pipe_run().
 * @li On Windows, the child process is created by default with the
 * @ref ECORE_EXE_WIN32_PRIORITY_NORMAL priority, unless the calling
 * process is in @ref ECORE_EXE_WIN32_PRIORITY_IDLE or
 * @ref ECORE_EXE_WIN32_PRIORITY_BELOW_NORMAL priority. In that case, the
 * child process inherits this priority.
 * @li On other platforms, if set to @ref ECORE_EXE_PRIORITY_INHERIT child
 * processes inherits the priority of their parent. This is the default.
 *
 * @param pri Value an Ecore_Exe_Win32_Priority value on Windows, @c -20
 * to @c 19 or @ref ECORE_EXE_PRIORITY_INHERIT on other OS.
 *
 */
EAPI void ecore_exe_run_priority_set(int pri);

/**
 * Gets the priority at which to launch processes.
 *
 * This gets the priority of launched processes. See
 * ecore_exe_run_priority_set() for details. This just returns the value set
 * by this call.
 *
 * @return The value set by ecore_exe_run_priority_set()
 */
EAPI int ecore_exe_run_priority_get(void);

/**
 * Spawns a child process.
 *
 * This is now just a thin wrapper around ecore_exe_pipe_run()
 * @param   exe_cmd The command to run with @c /bin/sh.
 * @param   data    Data to attach to the returned process handle.
 * @return  A process handle to the spawned process.
 * @note When you use this function you will have no permissions
 * to write or read on the pipe that connects you with the spwaned process.
 * If you need to do that use ecore_exe_pipe_run() with the
 * appropriated flags.
 *
 */
EAPI Ecore_Exe *ecore_exe_run(const char *exe_cmd, const void *data);

/**
 * Spawns a child process with its stdin/out available for communication.
 *
 * This function forks and runs the given command using @c /bin/sh.
 *
 * Note that the process handle is only valid until a child process
 * terminated event is received.  After all handlers for the child process
 * terminated event have been called, the handle will be freed by Ecore.
 *
 * This function does the same thing as ecore_exe_run(), but also makes the
 * standard in and/or out as well as stderr from the child process available
 * for reading or writing. To write use ecore_exe_send(). To read listen to
 * ECORE_EXE_EVENT_DATA or ECORE_EXE_EVENT_ERROR events (set up handlers).
 * Ecore may buffer read and error data until a newline character if asked
 * for with the @p flags. All data will be included in the events (newlines
 * will be replaced with NULLS if line buffered). ECORE_EXE_EVENT_DATA events
 * will only happen if the process is run with ECORE_EXE_PIPE_READ enabled
 * in the flags. The same with the error version. Writing will only be
 * allowed with ECORE_EXE_PIPE_WRITE enabled in the flags.
 *
 * @param   exe_cmd The command to run with @c /bin/sh.
 * @param   flags   The flag parameters for how to deal with inter-process I/O
 * @param   data    Data to attach to the returned process handle.
 * @return  A process handle to the spawned process.
 */
EAPI Ecore_Exe *ecore_exe_pipe_run(const char *exe_cmd, Ecore_Exe_Flags flags, const void *data);

/**
 * Defines a function to be called before really freeing the handle data.
 *
 * This might be useful for language bindings such as Python and Perl
 * that need to deallocate wrappers associated with this handle.
 *
 * This handle should never be modified by this call. It should be
 * considered informative only. All getters are valid when the given
 * function is called back.
 *
 * @param exe The child process to attach the pre_free function.
 * @param func The function to call before @a exe is freed.
 */
EAPI void ecore_exe_callback_pre_free_set(Ecore_Exe *exe, Ecore_Exe_Cb func);

/**
 * Sends data to the given child process which it receives on stdin.
 *
 * This function writes to a child processes standard in, with unlimited
 * buffering. This call will never block. It may fail if the system runs out
 * of memory.
 *
 * @param exe  The child process to send to
 * @param data The data to send
 * @param size The size of the data to send, in bytes
 * @return @c EINA_TRUE if successful, @c EINA_FALSE on failure.
 */
EAPI Eina_Bool ecore_exe_send(Ecore_Exe *exe, const void *data, int size);

/**
 * The stdin of the given child process will close when the write buffer is empty.
 *
 * @param exe  The child process
 */
EAPI void ecore_exe_close_stdin(Ecore_Exe *exe);

/**
 * Sets the auto pipe limits for the given process handle. On Windows
 * this function does nothing.
 *
 * @param   exe The given process handle.
 * @param   start_bytes Limit of bytes at start of output to buffer.
 * @param   end_bytes Limit of bytes at end of output to buffer.
 * @param   start_lines Limit of lines at start of output to buffer.
 * @param   end_lines Limit of lines at end of output to buffer.
 *
 */
EAPI void ecore_exe_auto_limits_set(Ecore_Exe *exe, int start_bytes, int end_bytes, int start_lines, int end_lines);

/**
 * Gets the auto pipe data for the given process handle
 *
 * @param   exe The given process handle.
 * @param   flags   Is this a ECORE_EXE_PIPE_READ or ECORE_EXE_PIPE_ERROR?
 * @return The event data.
 */
EAPI Ecore_Exe_Event_Data *ecore_exe_event_data_get(Ecore_Exe *exe, Ecore_Exe_Flags flags);

/**
 * Frees the given event data.
 *
 * @param   data The given event data.
 */
EAPI void ecore_exe_event_data_free(Ecore_Exe_Event_Data *data);

/**
 * Frees the given process handle.
 *
 * Note that the process that the handle represents is unaffected by this
 * function.
 *
 * @param   exe The given process handle.
 * @return  The data attached to the handle when @ref ecore_exe_run was
 *          called.
 */
EAPI void *ecore_exe_free(Ecore_Exe *exe);

/**
 * Retrieves the process ID of the given spawned process.
 * @param   exe Handle to the given spawned process.
 * @return  The process ID on success,  @c -1 otherwise.
 */
EAPI pid_t ecore_exe_pid_get(const Ecore_Exe *exe);

/**
 * Sets the string tag for the given process handle.
 *
 * @param   exe The given process handle.
 * @param   tag The string tag to set on the process handle.
 */
EAPI void ecore_exe_tag_set(Ecore_Exe *exe, const char *tag);

/**
 * Retrieves the tag attached to the given process handle. There is no need to
 * free it as it just returns the internal pointer value. This value is only
 * valid as long as the @p exe is valid or until the tag is set to something
 * else on this @p exe.
 *
 * @param   exe The given process handle.
 * @return The string attached to @p exe. It is a handle to existing
 *         internal string and should not be modified, use
 *         ecore_exe_tag_set() to change it. It might be @c NULL.
 */
EAPI const char *ecore_exe_tag_get(const Ecore_Exe *exe);

/**
 * Retrieves the command of the given spawned process.
 * @param   exe Handle to the given spawned process.
 * @return The command on success, @c NULL otherwise. This string is the
 *         pointer to the internal value and must not be modified in
 *         any way.
 */
EAPI const char *ecore_exe_cmd_get(const Ecore_Exe *exe);

/**
 * Retrieves the data attached to the given process handle.
 * @param   exe The given process handle.
 * @return The data pointer attached to @p exe Given to
 *         ecore_exe_run() or ecore_exe_pipe_run()
 */
EAPI void *ecore_exe_data_get(const Ecore_Exe *exe);

/**
 * Sets the data attached to the given process handle.
 * @param   exe The given process handle.
 * @param   data The pointer to attach.
 * @return The data pointer previously attached to @p exe with
 *         ecore_exe_run(), ecore_exe_pipe_run(), or ecore_exe_data_set()
 * @since 1.1
 */
EAPI void *ecore_exe_data_set(Ecore_Exe *exe, void *data);

/**
 * Retrieves the flags attached to the given process handle.
 * @param   exe The given process handle.
 * @return  The flags attached to @p exe.
 */
EAPI Ecore_Exe_Flags ecore_exe_flags_get(const Ecore_Exe *exe);

/**
 * Pauses the given process by sending it a @c SIGSTOP signal.
 * @param   exe Process handle to the given process.
 */
EAPI void ecore_exe_pause(Ecore_Exe *exe);

/**
 * Continues the given paused process by sending it a @c SIGCONT signal.
 * @param   exe Process handle to the given process.
 */
EAPI void ecore_exe_continue(Ecore_Exe *exe);

/**
 * Sends the given spawned process a interrupt (@c SIGINT) signal.
 * @param   exe Process handle to the given process.
 */
EAPI void ecore_exe_interrupt(Ecore_Exe *exe);

/**
 * Sends the given spawned process a quit (@c SIGQUIT) signal.
 * @param   exe Process handle to the given process.
 */
EAPI void ecore_exe_quit(Ecore_Exe *exe);

/**
 * Sends the given spawned process a terminate (@c SIGTERM) signal.
 * @param   exe Process handle to the given process.
 */
EAPI void ecore_exe_terminate(Ecore_Exe *exe);

/**
 * Kills the given spawned process by sending it a @c SIGKILL signal.
 * @param   exe Process handle to the given process.
 */
EAPI void ecore_exe_kill(Ecore_Exe *exe);

/**
 * Sends a @c SIGUSR signal to the given spawned process.
 * @param   exe Process handle to the given process.
 * @param   num The number user signal to send.  Must be either @c 1 or @c 2, or
 *              the signal will be ignored.
 */
EAPI void ecore_exe_signal(Ecore_Exe *exe, int num);

/**
 * Sends a @c SIGHUP signal to the given spawned process.
 * @param   exe Process handle to the given process.
 */
EAPI void ecore_exe_hup(Ecore_Exe *exe);

/**
 * @}
 */

/**
 * @ingroup Ecore_Main_Loop_Group
 * @defgroup Ecore_FD_Handler_Group File Descriptor Handling Functions
 *
 * @brief Definition for functions that deal with file descriptor handlers.
 *
 * File descriptor handlers facilitate reading, writing and checking for errors
 * without blocking the program or doing expensive pooling. This can be used to
 * monitor a socket, pipe, or other stream for which an FD can be had.
 *
 * @warning File descriptor handlers can't be used to monitor for file creation,
 * modification or deletion, see @ref Ecore_File_Group for this.
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
 * When using a socket, pipe or other stream it's important to remember that
 * errors may occur and as such to monitor not only for reading/writing but also
 * for errors using the @ref ECORE_FD_ERROR flag.
 *
 * Example of use of a file descriptor handler:
 * @li @ref ecore_fd_handler_example_c
 *
 * @{
 */

typedef struct _Ecore_Fd_Handler Ecore_Fd_Handler; /**< A handle for Fd handlers */

/**
 * @enum _Ecore_Fd_Handler_Flags
 * What to monitor the file descriptor for: reading, writing or error.
 */
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
typedef Eina_Bool (*Ecore_Fd_Cb)(void *data, Ecore_Fd_Handler *fd_handler);

/**
 * @typedef Ecore_Fd_Prep_Cb Ecore_Fd_Prep_Cb
 * A callback used by an @ref Ecore_Fd_Handler.
 */
typedef void (*Ecore_Fd_Prep_Cb)(void *data, Ecore_Fd_Handler *fd_handler);

/**
 * @typedef Ecore_Win32_Handle_Cb Ecore_Win32_Handle_Cb
 * A callback used by an @ref Ecore_Win32_Handler.
 */
typedef Eina_Bool (*Ecore_Win32_Handle_Cb)(void *data, Ecore_Win32_Handler *wh);

/**
 * @brief Adds a callback for activity on the given file descriptor.
 *
 * @param fd The file descriptor to watch.
 * @param flags To monitor it for reading use @c ECORE_FD_READ, for writing @c
 * ECORE_FD_WRITE, and for error @c ECORE_FD_ERROR. Values by |(ored).
 * @param func The callback function.
 * @param data The data to pass to the callback.
 * @param buf_func The function to call to check if any data has been buffered
 * and already read from the fd. May be @c NULL.
 * @param buf_data The data to pass to the @p buf_func function.
 * @return A fd handler handle on success, @c NULL otherwise.
 *
 * @a func will be called during the execution of @ref Ecore_Main_Loop_Page
 * when the file descriptor is available for reading, writing, or there has been
 * an error(depending on the given @a flags).
 *
 * When @a func returns @c ECORE_CALLBACK_CANCEL, it indicates that the
 * handler should be marked for deletion (identical to calling @ref
 * ecore_main_fd_handler_del).
 *
 * @warning @a buf_func is meant for @b internal use only and should be @b
 * avoided.
 *
 * The return value of @a buf_func has a different meaning, when it returns
 * ECORE_CALLBACK_CANCEL, it indicates that @a func @b shouldn't be called, and
 * when it returns ECORE_CALLBACK_RENEW it indicates @a func should be called.
 * The return value of @a buf_func will not cause the FD handler to be deleted.
 *
 * @a buf_func is called during event loop handling to check if data that has
 * been read from the file descriptor is in a buffer and is available to read.
 * Some systems, notably xlib, handle their own buffering, and would otherwise
 * not work with select(). These systems should use a @a buf_func. This is a
 * most annoying hack, only ecore_x uses it, so refer to that for an example.
 *
 * @warning This function should @b not be used for monitoring "normal" files, like text files.
 *
 */
EAPI Ecore_Fd_Handler *ecore_main_fd_handler_add(int fd, Ecore_Fd_Handler_Flags flags, Ecore_Fd_Cb func, const void *data, Ecore_Fd_Cb buf_func, const void *buf_data);

/**
 * @brief Adds a callback for activity on the given file descriptor.
 *
 * @param fd The file descriptor to watch.
 * @param flags To monitor it for reading use @c ECORE_FD_READ, for writing @c
 * ECORE_FD_WRITE, and for error @c ECORE_FD_ERROR. Values by |(ored).
 * @param func The callback function.
 * @param data The data to pass to the callback.
 * @param buf_func The function to call to check if any data has been buffered
 * and already read from the fd. May be @c NULL.
 * @param buf_data The data to pass to the @p buf_func function.
 * @return A fd handler handle on success, @c NULL otherwise.
 *
 * This function is identical to ecore_main_fd_handler_add, except that it supports regular files.
 * @warning This function should ONLY be called with ECORE_FD_ERROR, otherwise it will call the fd
 * handler constantly.
 * @warning Do not use this function unless you know what you are doing.
 *
 * @since 1.7
 */
EAPI Ecore_Fd_Handler *ecore_main_fd_handler_file_add(int fd, Ecore_Fd_Handler_Flags flags, Ecore_Fd_Cb func, const void *data, Ecore_Fd_Cb buf_func, const void *buf_data);

/**
 * @brief Sets the prepare callback with data for a given #Ecore_Fd_Handler.
 *
 * @param fd_handler The fd handler
 * @param func The prep function
 * @param data The data to pass to the prep function
 *
 * This function will be called prior to any fd handler's callback function
 * (even the other fd handlers), before entering the main loop select function.
 *
 * @note Once a prepare callback is set for a fd handler, it cannot be changed.
 * You need to delete the fd handler and create a new one, to set another
 * callback.
 * @note You probably don't need this function. It is only necessary for very
 * uncommon cases that need special behavior.
 */
EAPI void ecore_main_fd_handler_prepare_callback_set(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Prep_Cb func, const void *data);
/**
 * @brief Marks an FD handler for deletion.
 * @param fd_handler The FD handler.
 * @return The data pointer set using @ref ecore_main_fd_handler_add, for
 * @a fd_handler on success, @c NULL otherwise.
 * This function marks an fd handler to be deleted during an iteration of the
 * main loop. It does NOT close the associated fd!
 *
 * @warning If the underlying fd is already closed ecore may complain if the
 * main loop is using epoll internally, and also in some rare cases this may
 * cause crashes and instability. Remember to delete your fd handlers before the
 * fds they listen to are closed.
 */
EAPI void *ecore_main_fd_handler_del(Ecore_Fd_Handler *fd_handler);
/**
 * @brief Retrieves the file descriptor that the given handler is handling.
 * @param fd_handler The given FD handler.
 * @return The file descriptor the handler is watching.
 */
EAPI int ecore_main_fd_handler_fd_get(Ecore_Fd_Handler *fd_handler);
/**
 * @brief Gets which flags are active on an FD handler.
 * @param fd_handler The given FD handler.
 * @param flags The flags, @c ECORE_FD_READ, @c ECORE_FD_WRITE or
 * @c ECORE_FD_ERROR to query.
 * @return @c EINA_TRUE if any of the given flags are active, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool ecore_main_fd_handler_active_get(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags);
/**
 * @brief Sets what active streams the given FD handler should be monitoring.
 * @param fd_handler The given FD handler.
 * @param flags The flags to be watching.
 */
EAPI void ecore_main_fd_handler_active_set(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags);

/**
 * @brief Creates a Ecore_Win32_Handler object and add it to the win32_handlers list.
 * @param h    The win32 handler.
 * @param func The function to add as a callback.
 * @param data The data to pass to the callback when it is called.
 */
EAPI Ecore_Win32_Handler *ecore_main_win32_handler_add(void *h, Ecore_Win32_Handle_Cb func, const void *data);
/**
 * @brief Sets Ecore_Win32_Handler object to delete state.
 * The handler will be deleted in the _ecore_main_win32_handlers_cleanup function.
 *
 * @param win32_handler The Ecore_Win32_Handler object.
 */
EAPI void *ecore_main_win32_handler_del(Ecore_Win32_Handler *win32_handler);

/**
 * @}
 */

/**
 * @ingroup Ecore
 * @defgroup Ecore_Time_Group Ecore time functions
 *
 * These are function to retrieve time in a given format.
 *
 * Examples:
 * @li @ref ecore_time_functions_example_c
 * @{
 */

/**
 * Retrieves the current system time as a floating point value in seconds.
 *
 * This uses a monotonic clock and thus never goes back in time while
 * machine is live (even if user changes time or timezone changes,
 * however it may be reset whenever the machine is restarted).
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
 * Retrieves the current UNIX time as a floating point value in seconds.
 *
 * @see ecore_time_get().
 * @see ecore_loop_time_get().
 *
 * @return  The number of seconds since 12.00AM 1st January 1970.
 */
EAPI double ecore_time_unix_get(void);

/**
 * Retrieves the time at which the last loop stopped waiting for timeouts or
 * events.
 *
 * This gets the time that the main loop ceased waiting for timouts and/or
 * events to come in or for signals or any other interrupt source. This should
 * be considered a reference point for all time based activity that should
 * calculate its timepoint from the return of ecore_loop_time_get(). Use this
 * UNLESS you absolutely must get the current actual timepoint - then use
 * ecore_time_get(). Note that this time is meant to be used as relative to
 * other times obtained on this run. If you need absolute time references, use
 * ecore_time_unix_get() instead.
 *
 * This function can be called before any loop has ever been run, but either
 * ecore_init() or ecore_time_get() must have been called once.
 *
 * @return The number of seconds. Start time is not defined (it may be
 *         when the machine was booted, unix time, etc), all it is
 *         defined is that it never goes backwards (unless you got big critical
 *         messages when the application started).
 */
EAPI double ecore_loop_time_get(void);

/**
 * Sets the loop time.
 * 
 * @param t The new loop time
 * 
 * You should never need/call this, unless you are implementing a custom
 * tick source for an ecore animator. Only then inside your function that
 * calls ecore_animator_custom_tick(), just before it, if you are able to
 * get accurate timing information as to when the source of your tick
 * woke up, use this to adjust the ecore loop time to be perfectly
 * accurate. It is not a requirement, but makes things smoother. You should
 * not use it otherwise as it could harm timeline handling throughout the
 * application. Also note that the time point must match whatever zero time
 * you get from ecore_time_get() and ecore_loop_time_get() (same 0 point).
 * What this point is is undefined, sou unless your source uses the same
 * 0 time, then you may have to adjust and do some guessing.
 * 
 * @see ecore_animator_custom_tick()
 * @see ecore_loop_time_get()
 * @since 1.11
 */
EAPI void ecore_loop_time_set(double t);

/**
 * @}
 */

/**
 * @ingroup Ecore_Main_Loop_Group
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
 * things like cancelling the thread, your code grows in complexity and gets
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
 *ecore_thread_check(), sharing the flag with the main loop. But this
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
 * @{
 */

typedef struct _Ecore_Thread Ecore_Thread; /**< A handle for threaded jobs */

/**
 * @typedef Ecore_Thread_Cb Ecore_Thread_Cb
 * A callback used by Ecore_Thread helper.
 */
typedef void (*Ecore_Thread_Cb)(void *data, Ecore_Thread *thread);

/**
 * @typedef Ecore_Thread_Notify_Cb Ecore_Thread_Notify_Cb
 * A callback used by the main loop to receive data sent by an
 * @ref Ecore_Thread_Group.
 */
typedef void (*Ecore_Thread_Notify_Cb)(void *data, Ecore_Thread *thread, void *msg_data);

/**
 * Schedules a task to run in a parallel thread to avoid locking the main loop.
 *
 * @param func_blocking The function that should run in another thread.
 * @param func_end Function to call from main loop when @p func_blocking
 * completes its task successfully (may be NULL)
 * @param func_cancel Function to call from main loop if the thread running
 * @p func_blocking is cancelled or fails to start (may be NULL)
 * @param data User context data to pass to all callbacks.
 * @return A new thread handler, or @c NULL on failure.
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
 *ecore_thread_cancel(), either from the main thread or @p func_blocking.
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
EAPI Ecore_Thread *ecore_thread_run(Ecore_Thread_Cb func_blocking, Ecore_Thread_Cb func_end, Ecore_Thread_Cb func_cancel, const void *data);

/**
 * Launches a thread to run a task that can talk back to the main thread.
 *
 * @param func_heavy The function that should run in another thread.
 * @param func_notify Function that receives the data sent from the thread
 * @param func_end Function to call from main loop when @p func_heavy
 * completes its task successfully
 * @param func_cancel Function to call from main loop if the thread running
 * @p func_heavy is cancelled or fails to start
 * @param data User context data to pass to all callback.
 * @param try_no_queue If you want to run outside of the thread pool.
 * @return A new thread handler, or @c NULL on failure.
 *
 * See ecore_thread_run() for a general description of this function.
 *
 * The difference with the above is that ecore_thread_run() is meant for
 * tasks that don't need to communicate anything until they finish, while
 * this function is provided with a new callback, @p func_notify, that will
 * be called from the main thread for every message sent from @p func_heavy
 * with ecore_thread_feedback().
 *
 * Like ecore_thread_run(), a new thread will be launched to run
 * @p func_heavy unless the maximum number of simultaneous threads has been
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
EAPI Ecore_Thread *ecore_thread_feedback_run(Ecore_Thread_Cb func_heavy, Ecore_Thread_Notify_Cb func_notify,
                                             Ecore_Thread_Cb func_end, Ecore_Thread_Cb func_cancel,
                                             const void *data, Eina_Bool try_no_queue);

/**
 * Cancels a running thread.
 *
 * @param thread The thread to cancel.
 * @return Will return @c EINA_TRUE if the thread has been cancelled,
 *         @c EINA_FALSE if it is pending.
 *
 * This function can be called both in the main loop or in the running thread.
 *
 * This function cancels a running thread. If @p thread can be immediately
 * cancelled (it's still pending execution after creation or rescheduling),
 * then the @c cancel callback will be called, @p thread will be freed and
 * the function will return @c EINA_TRUE.
 *
 * If the thread is already running, then this function returns @c EINA_FALSE
 * after marking the @p thread as pending cancellation. For the thread to
 * actually be terminated, it needs to return from the user function back
 * into Ecore control. This can happen in several ways:
 * @li The function ends and returns normally. If it hadn't been cancelled,
 * @c func_end would be called here, but instead @c func_cancel will happen.
 * @li The function returns after requesting to be rescheduled with
 * ecore_thread_reschedule().
 * @li The function is prepared to leave early by checking if
 * ecore_thread_check() returns @c EINA_TRUE.

 * @li The function marks the thread as cancellable using
 * eina_thread_cancellable_set(), allowing the thread to be terminated
 * at explicit cancellation points defined with
 * eina_thread_cancel_checkpoint() or with syscalls mentioned at
 * man:pthreads(7). This allows blocking operations such as network or
 * disk access to be stopped without polling
 * ecore_thread_check(). Note that a cancelled thread may leak
 * resources if no cleanup function is registered with
 * EINA_THREAD_CLEANUP_PUSH(). Consider running such code using
 * eina_thread_cancellable_run().
 *
 * The user function can cancel itself by calling ecore_thread_cancel(), but
 * it should always use the ::Ecore_Thread handle passed to it and never
 * share it with the main loop thread by means of shared user data or any
 * other way.
 *
 * @p thread will be freed and should not be used again if this function
 * returns @c EINA_TRUE or after the @c func_cancel callback returns.
 *
 * @see ecore_thread_check()
 * @see eina_thread_cancellable_run()
 */
EAPI Eina_Bool ecore_thread_cancel(Ecore_Thread *thread);

/**
 * @brief Blocks the main loop until the thread execution is over.
 * @since 1.13.0
 *
 * @param thread The thread to wait on.
 * @param wait Maximum time to wait before exiting anyway.
 * @return EINA_TRUE if the thread execution is over.
 */
EAPI Eina_Bool ecore_thread_wait(Ecore_Thread *thread, double wait);

/**
 * Checks if a thread is pending cancellation.
 *
 * @param thread The thread to test.
 * @return @c EINA_TRUE if the thread is pending cancellation,
 *         @c EINA_FALSE if it is not.
 *
 * This function can be called both in the main loop or in the running thread.
 *
 * When ecore_thread_cancel() is called on an already running task, the
 * thread is marked as pending cancellation. This function returns @c EINA_TRUE
 * if this mark is set for the given @p thread and can be used from the
 * main loop thread to check if a still active thread has been cancelled,
 * or from the user function running in the thread to check if it should
 * stop doing what it's doing and return early, effectively cancelling the
 * task.
 *
 * @see ecore_thread_cancel()
 */
EAPI Eina_Bool ecore_thread_check(Ecore_Thread *thread);

/**
 * Sends data from the worker thread to the main loop.
 *
 * @param thread The current ::Ecore_Thread context to send data from
 * @param msg_data Data to be transmitted to the main loop
 * @return @c EINA_TRUE if @p msg_data was successfully sent to main loop,
 *         @c EINA_FALSE if anything goes wrong.
 *
 * You should use this function only in the @c func_heavy call.
 *
 * Only the address to @p msg_data will be sent and once this function
 * returns @c EINA_TRUE, the job running in the thread should never touch the
 * contents of it again. The data sent should be malloc()'ed or something
 * similar, as long as it's not memory local to the thread that risks being
 * overwritten or deleted once it goes out of scope or the thread finishes.
 *
 * Care must be taken that @p msg_data is properly freed in the @c func_notify
 * callback set when creating the thread.
 *
 * @see ecore_thread_feedback_run()
 */
EAPI Eina_Bool ecore_thread_feedback(Ecore_Thread *thread, const void *msg_data);

/**
 * Asks for the function in the thread to be called again at a later time.
 *
 * @param thread The current ::Ecore_Thread context to rescheduled
 * @return @c EINA_TRUE if the task was successfully rescheduled,
 *         @c EINA_FALSE if anything goes wrong.
 *
 * This function should be called only from the same function represented
 * by @p thread.
 *
 * Calling this function will mark the thread for a reschedule, so as soon
 * as it returns, it will be added to the end of the list of pending tasks.
 * If no other tasks are waiting or there are sufficient threads available,
 * the rescheduled task will be launched again immediately.
 *
 * This should never return @c EINA_FALSE, unless it was called from the wrong
 * thread or with the wrong arguments.
 *
 * The @c func_end callback set when the thread is created will not be
 * called until the function in the thread returns without being rescheduled.
 * Similarly, if the @p thread is cancelled, the reschedule will not take
 * effect.
 */
EAPI Eina_Bool ecore_thread_reschedule(Ecore_Thread *thread);

/**
 * Gets the number of active threads running jobs.
 *
 * @return Number of active threads running jobs
 *
 * This returns the number of threads currently running jobs of any type
 * through the Ecore_Thread API.
 *
 * @note Jobs started through the ecore_thread_feedback_run() function with
 * the @c try_no_queue parameter set to @c EINA_TRUE will not be accounted for
 * in the return of this function unless the thread creation fails and it
 * falls back to using one from the pool.
 */
EAPI int ecore_thread_active_get(void);

/**
 * Gets the number of short jobs waiting for a thread to run.
 *
 * @return Number of pending threads running "short" jobs
 *
 * This returns the number of tasks started with ecore_thread_run() that are
 * pending, waiting for a thread to become available to run them.
 */
EAPI int ecore_thread_pending_get(void);

/**
 * Gets the number of feedback jobs waiting for a thread to run.
 *
 * @return Number of pending threads running "feedback" jobs
 *
 * This returns the number of tasks started with ecore_thread_feedback_run()
 * that are pending, waiting for a thread to become available to run them.
 */
EAPI int ecore_thread_pending_feedback_get(void);

/**
 * Gets the total number of pending jobs.
 *
 * @return Number of pending threads running jobs
 *
 * Same as the sum of ecore_thread_pending_get() and
 *ecore_thread_pending_feedback_get().
 */
EAPI int ecore_thread_pending_total_get(void);

/**
 * Gets the maximum number of threads that can run simultaneously.
 *
 * @return Max possible number of Ecore_Thread's running concurrently
 *
 * This returns the maximum number of Ecore_Thread's that may be running at
 * the same time. If this number is reached, new jobs started by either
 *ecore_thread_run() or ecore_thread_feedback_run() will be added to the
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
EAPI int ecore_thread_max_get(void);

/**
 * Sets the maximum number of threads allowed to run simultaneously.
 *
 * @param num The new maximum
 *
 * This sets a new value for the maximum number of concurrently running
 * Ecore_Thread's. It @b must an integer between 1 and (16 * @c x), where @c x
 * is the number for CPUs available.
 *
 * @see ecore_thread_max_get()
 * @see ecore_thread_max_reset()
 */
EAPI void ecore_thread_max_set(int num);

/**
 * Resets the maximum number of concurrently running threads to the default.
 *
 * This resets the value returned by ecore_thread_max_get() back to its
 * default.
 *
 * @see ecore_thread_max_get()
 * @see ecore_thread_max_set()
 */
EAPI void ecore_thread_max_reset(void);

/**
 * Gets the number of threads available for running tasks.
 *
 * @return The number of available threads
 *
 * Same as doing ecore_thread_max_get() - ecore_thread_active_get().
 *
 * This function may return a negative number only in the case the user
 * changed the maximum number of running threads while other tasks are
 * running.
 */
EAPI int ecore_thread_available_get(void);

/**
 * Adds some data to a hash local to the thread.
 *
 * @param thread The thread context the data belongs to
 * @param key The name under which the data will be stored
 * @param value The data to add
 * @param cb Function to free the data when removed from the hash
 * @param direct If @c true, this will not copy the key string (like
 * eina_hash_direct_add())
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
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
 * Neither @p key nor @p value may be @c NULL and @p key will be copied in the
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
EAPI Eina_Bool ecore_thread_local_data_add(Ecore_Thread *thread, const char *key, void *value,
                                           Eina_Free_Cb cb, Eina_Bool direct);

/**
 * Sets some data in the hash local to the given thread.
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
 * @c NULL will also be returned if either @p key or @p value are @c NULL, or
 * if an error occurred.
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
EAPI void *ecore_thread_local_data_set(Ecore_Thread *thread, const char *key, void *value, Eina_Free_Cb cb);

/**
 * Gets data stored in the hash local to the given thread.
 *
 * @param thread The thread context the data belongs to
 * @param key The name under which the data is stored
 * @return The value under the given key, or @c NULL on error.
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
EAPI void *ecore_thread_local_data_find(Ecore_Thread *thread, const char *key);

/**
 * Deletes from the thread's hash the data corresponding to the given key.
 *
 * @param thread The thread context the data belongs to
 * @param key The name under which the data is stored
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * If there's any data stored associated with @p key in the global hash,
 * this function will remove it from it and return @c EINA_TRUE. If no data
 * exists or an error occurs, it returns @c EINA_FALSE.
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
EAPI Eina_Bool ecore_thread_local_data_del(Ecore_Thread *thread, const char *key);

/**
 * Adds some data to a hash shared by all threads.
 *
 * @param key The name under which the data will be stored
 * @param value The data to add
 * @param cb Function to free the data when removed from the hash
 * @param direct If @c true, this will not copy the key string (like
 * eina_hash_direct_add())
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
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
 * Neither @p key nor @p value may be @c NULL and @p key will be copied in the
 * hash, unless @p direct is set, in which case the string used should not
 * be freed until the data is removed from the hash.
 *
 * The @p cb function will be called when the data in the hash needs to be
 * freed, be it because it got deleted with ecore_thread_global_data_del() or
 * because Ecore Thread was shut down and the hash destroyed. This parameter
 * may be NULL, in which case @p value needs to be manually freed after
 * removing it from the hash with either ecore_thread_global_data_del() or
 *ecore_thread_global_data_set().
 *
 * Manually freeing any data that was added to the hash with a @p cb function
 * is likely to produce a segmentation fault, or any other strange
 * happenings, later on in the program.
 *
 * @see ecore_thread_global_data_del()
 * @see ecore_thread_global_data_set()
 * @see ecore_thread_global_data_find()
 */
EAPI Eina_Bool ecore_thread_global_data_add(const char *key, void *value, Eina_Free_Cb cb, Eina_Bool direct);

/**
 * Sets some data in the hash shared by all threads.
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
 * @c NULL will also be returned if either @p key or @p value are @c NULL, or
 * if an error occurred.
 *
 * @see ecore_thread_global_data_add()
 * @see ecore_thread_global_data_del()
 * @see ecore_thread_global_data_find()
 */
EAPI void *ecore_thread_global_data_set(const char *key, void *value, Eina_Free_Cb cb);

/**
 * Gets data stored in the hash shared by all threads.
 *
 * @param key The name under which the data is stored
 * @return The value under the given key, or @c NULL on error.
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
EAPI void *ecore_thread_global_data_find(const char *key);

/**
 * Deletes from the shared hash the data corresponding to the given key.
 *
 * @param key The name under which the data is stored
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * If there's any data stored associated with @p key in the global hash,
 * this function will remove it from it and return @c EINA_TRUE. If no data
 * exists or an error occurs, it returns @c EINA_FALSE.
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
EAPI Eina_Bool ecore_thread_global_data_del(const char *key);

/**
 * Gets data stored in the shared hash, or wait for it if it doesn't exist.
 *
 * @param key The name under which the data is stored
 * @param seconds The amount of time in seconds to wait for the data.
 * @return The value under the given key, or @c NULL on error.
 *
 * Finds and return the data stored in the shared hash under the key @p key.
 *
 * If there's nothing in the hash under the given @p key, the function
 * will block and wait up to @p seconds seconds for some other thread to
 * add it with either ecore_thread_global_data_add() or
 * ecore_thread_global_data_set(). If after waiting there's still no data
 * to get, @c NULL will be returned.
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
EAPI void *ecore_thread_global_data_wait(const char *key, double seconds);

/**
 * @}
 */

/**
 * @ingroup Ecore_Main_Loop_Group
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
 * @{
 */

typedef struct _Ecore_Pipe Ecore_Pipe; /**< A handle for pipes */

/**
 * @typedef Ecore_Pipe_Cb Ecore_Pipe_Cb
 * The callback that data written to the pipe is sent to.
 */
typedef void (*Ecore_Pipe_Cb)(void *data, void *buffer, unsigned int nbyte);

/**
 * Creates two file descriptors (sockets on Windows). Adds
 * a callback that will be called when the file descriptor that
 * is listened receives data. An event is also put in the event
 * queue when data is received.
 *
 * @param handler The handler called when data is received.
 * @param data    Data to pass to @p handler when it is called.
 * @return        A newly created Ecore_Pipe object if successful,
 *                @c NULL otherwise.
 */
EAPI Ecore_Pipe *ecore_pipe_add(Ecore_Pipe_Cb handler, const void *data);

/**
 * Creates a pipe with more parameters.
 * 
 * @param handler Same as ecore_pipe_add()
 * @param data Same as ecore_pipe_add()
 * @param fd_read An fd to use for reading or @c -1 otherwise
 * @param fd_write An fd to use for writing or @c -1 otherwise
 * @param read_survive_fork Should read fd survive a fork
 * @param write_survive_fork Should write fd survive a fork
 * 
 * This is the same as ecore_pipe_add() but with some added parameters.
 *
 * @return A pointer to the new Ecore_Pipe object on success, else NULL.
 * @see ecore_pipe_add()
 */
EAPI Ecore_Pipe *ecore_pipe_full_add(Ecore_Pipe_Cb handler,
                                     const void *data,
                                     int fd_read, int fd_write,
                                     Eina_Bool read_survive_fork,
                                     Eina_Bool write_survive_fork);

/**
 * Frees an Ecore_Pipe object created with ecore_pipe_add().
 *
 * @param p The Ecore_Pipe object to be freed.
 * @return The pointer to the private data
 */
EAPI void *ecore_pipe_del(Ecore_Pipe *p);

/**
 * Writes on the file descriptor the data passed as parameter.
 *
 * @param p      The Ecore_Pipe object.
 * @param buffer The data to write into the pipe.
 * @param nbytes The size of the @p buffer in bytes.
 * @return       @c EINA_TRUE on a successful write, @c EINA_FALSE on error.
 */
EAPI Eina_Bool ecore_pipe_write(Ecore_Pipe *p, const void *buffer, unsigned int nbytes);

/**
 * Closes the write end of an Ecore_Pipe object created with ecore_pipe_add().
 *
 * @param p The Ecore_Pipe object.
 */
EAPI void ecore_pipe_write_close(Ecore_Pipe *p);

/**
 * Closes the read end of an Ecore_Pipe object created with ecore_pipe_add().
 *
 * @param p The Ecore_Pipe object.
 */
EAPI void ecore_pipe_read_close(Ecore_Pipe *p);

/**
 * Gets the pipe read file descriptor.
 *
 * @param p The Ecore_Pipe object query.
 * @return The file descriptor, or @c -1 if none
 */
EAPI int ecore_pipe_read_fd(Ecore_Pipe *p);

/**
 * Gets the pipe write file descriptor.
 *
 * @param p The Ecore_Pipe object query.
 * @return The file descriptor, or @c -1 if none
 */
EAPI int ecore_pipe_write_fd(Ecore_Pipe *p);

/**
 * Starts monitoring again the pipe for reading. See ecore_pipe_freeze() for
 * stopping the monitoring activity. This will not work if
 * ecore_pipe_read_close() was previously called on the same pipe.
 *
 * @param p The Ecore_Pipe object.
 * @since 1.1
 */
EAPI void ecore_pipe_thaw(Ecore_Pipe *p);

/**
 * Stops monitoring if necessary the pipe for reading. See ecore_pipe_thaw()
 * for monitoring it again.
 *
 * @param p The Ecore_Pipe object.
 * @since 1.1
 */
EAPI void ecore_pipe_freeze(Ecore_Pipe *p);

/**
 * @brief Waits from another thread on the read side of a pipe.
 *
 * @param p The pipe to watch on.
 * @param message_count The minimal number of message to wait before exiting.
 * @param wait The amount of time in second to wait before exiting.
 * @return the number of message catched during that wait call.
 * @since 1.1
 *
 * Negative value for @p wait means infite wait.
 */
EAPI int ecore_pipe_wait(Ecore_Pipe *p, int message_count, double wait);

/**
 * @}
 */

/**
 * @ingroup Ecore
 * @defgroup Ecore_Application_Group Ecore Application functions
 *
 * @{
 */

/**
 * Sets up the programs command-line arguments.
 * @param argc The same as passed as argc to the programs main() function
 * @param argv The same as passed as argv to the programs main() function
 *
 * A call to this function will store the programs command-line arguments
 * for later use by ecore_app_restart() or ecore_app_args_get().
 */
EAPI void ecore_app_args_set(int argc, const char **argv);

/**
 * Returns the programs stored command-line arguments.
 * @param argc A pointer to the return value to hold argc
 * @param argv A pointer to the return value to hold argv
 *
 * When called, this function returns the arguments for the program stored by
 * ecore_app_args_set(). The integer pointed to by @p argc will be filled, if
 * the pointer is not NULL, and the string array pointer @p argv will be filled
 * also if the pointer is not NULL. The values they are filled with will be the
 * same set by ecore_app_args_set().
 */
EAPI void ecore_app_args_get(int *argc, char ***argv);

/**
 * Restarts the program executable with the command-line arguments stored.
 *
 * This function will restart & re-execute this program in place of itself
 * using the command-line arguments stored by ecore_app_args_set(). This is
 * an easy way for a program to restart itself for cleanup purposes,
 * configuration reasons or in the event of a crash.
 */
EAPI void ecore_app_restart(void);

/**
 * @brief Do not load system modules for this application.
 *
 * Ecore will now load platform-specific system modules such as
 * power-management, time and locate monitors.
 *
 * Whenever this function is called @b before ecore_init(), ecore
 * won't load such modules.
 *
 * This may be useful to some command-line utilities, hardly will be
 * useful for end-user applications.
 *
 * The environment variable ECORE_NO_SYSTEM_MODULES=1 may be used
 * to temporarily disable system modules, often useful for debug.
 *
 * @since 1.8
 */
EAPI void ecore_app_no_system_modules(void);

/**
 * @}
 */

/**
 * @ingroup Ecore_Main_Loop_Group
 * @defgroup Ecore_Throttle_Group Ecore Throttle functions
 *
 * @{
 */

/**
 * Increases throttle amount.
 *
 * This will increase or decrease (if @p amount is positive or negative) the
 * amount of "voluntary throttling" ecore will do to its main loop while
 * running. This is intended to be used to limit animations and wakeups when
 * in a strict power management state. The higher the current throttle value
 * (which can be retrieved by ecore_throttle_get() ), the more throttling
 * takes place. If the current throttle value is 0, then no throttling takes
 * place at all.
 *
 * The value represents how long the ecore main loop will sleep (in seconds)
 * before it goes into a fully idle state waiting for events, input or
 * timing events to wake it up. For example, if the current throttle level
 * is 0.5, then after every time the main loop cycles and goes into idle
 * after processing all events, the main loop will explicitly sleep for 0.5
 * seconds before sitting and waiting for incoming events or timeouts, thus
 * preventing animation, async IO and network handling etc. for that period
 * of time. Of course these events, data and timeouts will be buffered,
 * thus not losing anything, simply delaying when they get handled by the
 * throttle value.
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
 * @param   amount Amount (in seconds) to adjust by
 */
EAPI void ecore_throttle_adjust(double amount);

/**
 * Gets current throttle level.
 *
 * This gets the current throttling level, which can be adjusted by
 * ecore_throttle_adjust(). The value is in seconds. Please see
 * ecore_throttle_adjust() for more information.
 *
 * @return  The current throttle level.
 */
EAPI double ecore_throttle_get(void);

/**
 * @}
 */

/**
 * @ingroup Ecore_Main_Loop_Group
 * @defgroup Ecore_Poller_Group Ecore Poll functions
 *
 * Ecore poller provides infrastructure for the creation of pollers. Pollers
 * are, in essence, callbacks that share a single timer per type. Because not
 * all pollers need to be called at the same frequency the user may specify the
 * frequency in ticks(each expiration of the shared timer is called a tick, in
 * ecore poller parlance) for each added poller. Ecore pollers should only be
 * used when the poller doesn't have specific requirements on the exact times
 * to poll.
 *
 * This architecture means that the main loop is only woken up once to handle
 * all pollers of that type, this will save power as the CPU has more of a
 * chance to go into a low power state the longer it is asleep for, so this
 * should be used in situations where power usage is a concern.
 *
 * For now only 1 core poller type is supported: ECORE_POLLER_CORE, the default
 * interval for ECORE_POLLER_CORE is 0.125(or 1/8th) second.
 *
 * The creation of a poller is extremely simple and only requires one line:
 * @code
 * ecore_poller_add(ECORE_POLLER_CORE, 1, my_poller_function, NULL);
 * @endcode
 * This sample creates a poller to call @c my_poller_function at every tick with
 * @c NULL as data.
 *
 * Example:
 * @li @ref ecore_poller_example_c
 *
 * @{
 */

/**
 * @}
 */

/**
 * @ingroup Ecore_Main_Loop_Group
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
 * @{
 */

/**
 * @enum _Ecore_Animator_Source
 * Defines the timing sources for animators.
 */
enum _Ecore_Animator_Source    /* Timing sources for animators */
{
   ECORE_ANIMATOR_SOURCE_TIMER, /**< The default system clock/timer based animator that ticks every "frametime" seconds */
   ECORE_ANIMATOR_SOURCE_CUSTOM /**< A custom animator trigger that you need to call ecore_animator_custom_tick() to make it tick */
};
typedef enum _Ecore_Animator_Source Ecore_Animator_Source;

/**
 * @typedef Ecore_Timeline_Cb Ecore_Timeline_Cb
 * A callback run for a task (animators with runtimes)
 */
typedef Eina_Bool (*Ecore_Timeline_Cb)(void *data, double pos);

/** Defines the position mappings for the animation. */
typedef enum
{
  ECORE_POS_MAP_LINEAR = 0, /**< Linear 0.0 -> 1.0 */
  ECORE_POS_MAP_ACCELERATE, /**< Start slow then speed up */
  ECORE_POS_MAP_DECELERATE, /**< Start fast then slow down */
  ECORE_POS_MAP_SINUSOIDAL, /**< Start slow, speed up then slow down at end */
  ECORE_POS_MAP_ACCELERATE_FACTOR, /**< Start slow then speed up, v1 being a
                                    * power factor, 0.0 being linear, 1.0 being
                                    * normal accelerate, 2.0 being much more
                                    * pronounced accelerate (squared), 3.0
                                    * being cubed, etc. */
  ECORE_POS_MAP_DECELERATE_FACTOR, /**< Start fast then slow down, v1 being a
                                    * power factor, 0.0 being linear, 1.0 being
                                    * normal decelerate, 2.0 being much more
                                    * pronounced decelerate (squared), 3.0
                                    * being cubed, etc. */
  ECORE_POS_MAP_SINUSOIDAL_FACTOR, /**< Start slow, speed up then slow down at
                                    * end, v1 being a power factor, 0.0 being
                                    * linear, 1.0 being normal sinusoidal, 2.0
                                    * being much more pronounced sinusoidal
                                    * (squared), 3.0 being cubed, etc. */
  ECORE_POS_MAP_DIVISOR_INTERP, /**< Start at gradient * v1, interpolated via
                                 * power of v2 curve */
  ECORE_POS_MAP_BOUNCE, /**< Start at 0.0 then "drop" like a ball bouncing to
                         * the ground at 1.0, and bounce v2 times, with decay
                         * factor of v1 */
  ECORE_POS_MAP_SPRING, /**< Start at 0.0 then "wobble" like a spring rest
                         * position 1.0, and wobble v2 times, with decay factor
                         * of v1 */
  ECORE_POS_MAP_CUBIC_BEZIER /**< Follow the cubic-bezier curve calculated with
                              * the control points (x1, y1), (x2, y2) */
} Ecore_Pos_Map;

/*
 * @since 1.8
 */

/**
 * @brief Sets the animator call interval in seconds.
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
EAPI void ecore_animator_frametime_set(double frametime);

/**
 * @brief Gets the animator call interval in seconds.
 *
 * @return The time in second in between animator ticks.
 *
 * This function retrieves the time in seconds between animator ticks.
 *
 * @see ecore_animator_frametime_set()
 */
EAPI double ecore_animator_frametime_get(void);

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
 * of possible animation curves to be used:
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
 * This will make an animation that bounces 7 each times diminishing by a
 * factor of 1.8.
 *
 * @see Ecore_Pos_Map
 *
 * @since 1.1.0
 */
EAPI double ecore_animator_pos_map(double pos, Ecore_Pos_Map map, double v1, double v2);

/**
 * @brief Maps an input position from 0.0 to 1.0 along a timeline to a
 * position in a different curve.
 *
 * @param pos The input position to map
 * @param map The mapping to use
 * @param v_size The size of the v array.
 * @param v An array with the double parameters to be used by the mapping.
 * NULL if not used.
 * @return The mapped value
 *
 * Takes an input position (0.0 to 1.0) and maps to a new position (normally
 * between 0.0 and 1.0, but it may go above/below 0.0 or 1.0 to show that it
 * has "overshot" the mark) using some interpolation (mapping) algorithm.
 *
 * This function useful to create non-linear animations. It offers a variety
 * of possible animation curves to be used:
 * @li ECORE_POS_MAP_LINEAR - Linear, returns @p pos
 * @li ECORE_POS_MAP_ACCELERATE - Start slow then speed up
 * @li ECORE_POS_MAP_DECELERATE - Start fast then slow down
 * @li ECORE_POS_MAP_SINUSOIDAL - Start slow, speed up then slow down at end
 * @li ECORE_POS_MAP_ACCELERATE_FACTOR - Start slow then speed up, v[0] being a
 * power factor, 0.0 being linear, 1.0 being ECORE_POS_MAP_ACCELERATE, 2.0
 * being much more pronounced accelerate (squared), 3.0 being cubed, etc.
 * @li ECORE_POS_MAP_DECELERATE_FACTOR - Start fast then slow down, v[0] being a
 * power factor, 0.0 being linear, 1.0 being ECORE_POS_MAP_DECELERATE, 2.0
 * being much more pronounced decelerate (squared), 3.0 being cubed, etc.
 * @li ECORE_POS_MAP_SINUSOIDAL_FACTOR - Start slow, speed up then slow down
 * at end, v[0] being a power factor, 0.0 being linear, 1.0 being
 * ECORE_POS_MAP_SINUSOIDAL, 2.0 being much more pronounced sinusoidal
 * (squared), 3.0 being cubed, etc.
 * @li ECORE_POS_MAP_DIVISOR_INTERP - Start at gradient * v[0], interpolated via
 * power of v2 curve
 * @li ECORE_POS_MAP_BOUNCE - Start at 0.0 then "drop" like a ball bouncing to
 * the ground at 1.0, and bounce v2 times, with decay factor of v[0]
 * @li ECORE_POS_MAP_SPRING - Start at 0.0 then "wobble" like a spring rest
 * position 1.0, and wobble v2 times, with decay factor of v[0]
 * @li ECORE_POS_MAP_CUBIC_BEZIER - Use an interpolated cubic-bezier curve
 * ajusted with parameters from v[0] to v[3].
 * @note When not listed v has no effect.
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
 * double v[2] = {1.8, 7};
 *
 * out = ecore_animator_pos_map(pos, ECORE_POS_MAP_BOUNCE, 2, v);
 * x = (x1 * out) + (x2 * (1.0 - out));
 * y = (y1 * out) + (y2 * (1.0 - out));
 * move_my_object_to(myobject, x, y);
 * @endcode
 * This will make an animation that bounces 7 each times diminishing by a
 * factor of 1.8.
 *
 * @see Ecore_Pos_Map
 */
EAPI double ecore_animator_pos_map_n(double pos, Ecore_Pos_Map map, int v_size, double *v);

/**
 * @brief Sets the source of animator ticks for the mainloop.
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
 * interrupt, and etc.) using
 * ecore_animator_custom_source_tick_begin_callback_set() and
 * ecore_animator_custom_source_tick_end_callback_set() to set the functions
 * that will be called to start and stop the ticking source, which when it
 * gets a "tick" should call ecore_animator_custom_tick() to make the "tick"
 * over 1 frame.
 *
 * @see ecore_animator_source_get()
 */
EAPI void ecore_animator_source_set(Ecore_Animator_Source source);

/**
 * @brief Gets the animator source currently set.
 *
 * @return The current animator source
 *
 * This gets the current animator source.
 *
 * @see ecore_animator_source_set()
 */
EAPI Ecore_Animator_Source ecore_animator_source_get(void);

/**
 * @brief Sets the function that begins a custom animator tick source.
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
 * is @c NULL then no function is called to begin custom ticking.
 *
 * @warning Do not use this function unless you know what you are doing.
 *
 * @see ecore_animator_source_set()
 * @see ecore_animator_custom_source_tick_end_callback_set()
 * @see ecore_animator_custom_tick()
 */
EAPI void ecore_animator_custom_source_tick_begin_callback_set(Ecore_Cb func, const void *data);

/**
 * @brief Sets the function that ends a custom animator tick source.
 *
 * @param func The function to call when ticking is to end
 * @param data The data passed to the tick end function as its parameter
 *
 * This function is a matching pair to the function set by
 * ecore_animator_custom_source_tick_begin_callback_set() and is called
 * when ticking is to stop. If @p func is @c NULL then no function will be
 * called to stop ticking. For more information please see
 * ecore_animator_custom_source_tick_begin_callback_set().
 *
 * @warning Do not use this function unless you know what you are doing.
 *
 * @see ecore_animator_source_set()
 * @see ecore_animator_custom_source_tick_begin_callback_set()
 * @see ecore_animator_custom_tick()
 */
EAPI void ecore_animator_custom_source_tick_end_callback_set(Ecore_Cb func, const void *data);

/**
 * @brief Triggers a custom animator tick.
 *
 * When animator source is set to ECORE_ANIMATOR_SOURCE_CUSTOM, then calling
 * this function triggers a run of all animators currently registered with
 * Ecore as this indicates a "frame tick" happened. This will do nothing if
 * the animator source(set by ecore_animator_source_set()) is not set to
 * ECORE_ANIMATOR_SOURCE_CUSTOM.
 *
 * @warning Do not use this function unless you know what you are doing.
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
 * @ingroup Ecore_Main_Loop_Group
 * @defgroup Ecore_Timer_Group Ecore Timer functions
 *
 * Ecore provides very flexible timer functionality. The basic usage of timers,
 * to call a certain function at a certain interval can be achieved with a
 * single line:
 * @code
 * Eina_Bool my_func(void *data) {
 *    do_funky_stuff_with_data(data);
 *    return ECORE_CALLBACK_RENEW;
 * }
 * ecore_timer_add(interval_in_seconds, my_func, data_given_to_function);
 * @endcode
 * @note If the function was to be executed only once simply return
 * @c ECORE_CALLBACK_CANCEL instead.
 *
 * An example that shows the usage of a lot of these:
 * @li @ref ecore_timer_example_c
 *
 * @{
 */

/*
 * @since 1.8
 */
typedef Eo Ecore_Timer; /**< A handle for timers */

#define _ECORE_TIMER_EO_CLASS_TYPE

/**
 * Retrieves the current precision used by timer infrastructure.
 * @return Current precision.
 * @see ecore_timer_precision_set()
 */
EAPI double ecore_timer_precision_get(void);

/**
 * @brief Sets the precision to be used by timer infrastructure.
 *
 * @param precision Allowed introduced timeout delay, in seconds.
 *
 * This sets the precision for @b all timers. The precision determines how much
 * of an difference from the requested interval is acceptable. One common reason
 * to use this function is to @b increase the allowed timeout and thus @b
 * decrease precision of the timers, this is because less precise the timers
 * result in the system waking up less often and thus consuming less resources.
 *
 * Be aware that kernel may delay delivery even further, these delays
 * are always possible due other tasks having higher priorities or
 * other scheduler policies.
 *
 * Example:
 *  We have 2 timers, one that expires in a 2.0s and another that
 *  expires in 2.1s, if precision is 0.1s, then the Ecore will request
 *  for the next expire to happen in 2.1s and not 2.0s and another one
 *  of 0.1 as it would before.
 *
 * @note Ecore is smart enough to see if there are timers in the
 * precision range, if it does not, in our example if no second timer
 * in (T + precision) existed, then it would use the minimum timeout.
 */
EAPI void ecore_timer_precision_set(double precision);

/**
 * This function returns a human readable text-based log for Ecore_Timer events.
 *
 * @return A heap allocated string, or NULL. It MUST be freed manually by the
 * caller using `free`.
 *
 * It only contains an useful implementation if EFL is built in debug build
 * profile, but it's safe to call it for any build profile.
 */
EAPI char *ecore_timer_dump(void);

/**
 * @}
 */

/**
 * @ingroup Ecore_Main_Loop_Group
 * @defgroup Ecore_Idle_Group Ecore Idle functions
 *
 * The idler functionality in Ecore allows for callbacks to be called when the
 * program isn't handling @ref Ecore_Event_Group "events", @ref Ecore_Timer_Group
 * "timers" or @ref Ecore_FD_Handler_Group "fd handlers".
 *
 * There are three types of idlers: Enterers, Idlers(proper) and Exiters. They
 * are called, respectively, when the program is about to enter an idle state,
 * when the program is in an idle state and when the program has just left an
 * idle state and will begin processing @ref Ecore_Event_Group "events", @ref
 * Ecore_Timer_Group "timers" or @ref Ecore_FD_Handler_Group "fd handlers".
 *
 * Enterer callbacks are good for updating your program's state, if
 * it has a state engine.  Once all of the enterer handlers are
 * called, the program will enter a "sleeping" state.
 *
 * Idler callbacks are called when the main loop has called all
 * enterer handlers.  They are useful for interfaces that require
 * polling and timers would be too slow to use.
 *
 * Exiter callbacks are called when the main loop wakes up from an idle state.
 *
 * If no idler callbacks are specified, then the process literally
 * goes to sleep.  Otherwise, the idler callbacks are called
 * continuously while the loop is "idle", using as much CPU as is
 * available to the process.
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
 * @{
 */

/*
 * @since 1.8
 */
typedef struct _Ecore_Factorized_Idle Ecore_Idler; /**< A handle for idlers */

/*
 * @since 1.8
 */
typedef struct _Ecore_Factorized_Idle Ecore_Idle_Enterer; /**< A handle for idle enterers */

/*
 * @since 1.8
 */
typedef struct _Ecore_Factorized_Idle Ecore_Idle_Exiter; /**< A handle for idle exiters */

/**
 * @}
 */

/**
 * @ingroup Ecore_Main_Loop_Group
 * @defgroup Ecore_Job_Group Ecore Job functions
 *
 * You can queue jobs that are to be done by the main loop when the
 * current event is dealt with.
 *
 * Jobs are processed by the main loop similarly to events. They
 * also will be executed in the order in which they were added.
 *
 * A good use for them is when you don't want to execute an action
 * immediately, but want to give the control back to the main loop
 * so that it will call your job callback when jobs start being
 * processed (and if there are other jobs added before yours, they
 * will be processed first). This also gives the chance to other
 * actions in your program to cancel the job before it is started.
 *
 * Examples of using @ref Ecore_Job :
 * @li @ref ecore_job_example_c
 *
 * @{
 */

/**
 * @since 1.8
 */
typedef struct _Ecore_Job Ecore_Job;    /**< A job handle */

#define _ECORE_JOB_EO_CLASS_TYPE
/**
 * @}
 */


#ifdef EFL_BETA_API_SUPPORT

/*
 * @brief Function callback type for when creating Ecore_Thread that
 * uses Efl_Future for communication.
 */
typedef void (*Ecore_Thread_Future_Cb)(const void *data, Eo *promise, Ecore_Thread *thread);

EAPI Efl_Future *ecore_thread_future_run(Ecore_Thread_Future_Cb heavy, const void *data, Eina_Free_Cb free_cb);

#endif

#ifdef __cplusplus
}
#endif
