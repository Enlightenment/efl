#ifndef _ECORE_LEGACY_H
#define _ECORE_LEGACY_H

/**
 * @ingroup Ecore_Poller_Group
 *
 * @{
 */
typedef enum
{
  ECORE_POLLER_CORE = 0 /**< The core poller interval */
} Ecore_Poller_Type;

typedef struct _Ecore_Poller Ecore_Poller;

/**
 * @brief Creates a poller to call the given function at a particular tick interval.
 * @param type The ticker type to attach the poller to. Must be ECORE_POLLER_CORE.
 * @param interval The poll interval.
 * @param func The poller function.
 * @param data Data to pass to @a func when it is called.
 * @return A poller object on success, @c NULL otherwise.
 *
 * This function adds @a func as a poller callback that will be called every @a
 * interval ticks together with other pollers of type @a type. @a func will be
 * passed the @p data pointer as a parameter.
 *
 * The @p interval must be between 1 and 32768 inclusive, and must be a power of
 * 2 (i.e. 1, 2, 4, 8, 16, ... 16384, 32768). The exact tick in which @a func
 * will be called is undefined, as only the interval between calls can be
 * defined. Ecore will endeavor to keep pollers synchronized and to call as
 * many in 1 wakeup event as possible. If @a interval is not a power of two, the
 * closest power of 2 greater than @a interval will be used.
 *
 * When the poller @p func is called, it must return a value of either
 * @c ECORE_CALLBACK_RENEW(or @c 1) or @c ECORE_CALLBACK_CANCEL(or @c 0). If it
 * returns @c 1, it will be called again at the next tick, or if it returns
 * @c 0 it will be deleted automatically making any references/handles for it
 * invalid.
 */
EAPI Ecore_Poller *ecore_poller_add(Ecore_Poller_Type type, int interval, Ecore_Task_Cb func, const void *data);

/**
 * @brief Deletes the specified poller from the timer list.
 * @param poller The poller to delete.
 * @return The data pointer set for the timer when @ref ecore_poller_add was
 * called on success, @c NULL otherwise.
 *
 * @note @a poller must be a valid handle. If the poller function has already
 * returned @c 0, the handle is no longer valid (and does not need to be deleted).
 */
EAPI void *ecore_poller_del(Ecore_Poller *poller);

/**
 * @brief Sets the time(in seconds) between ticks for the given poller type.
 * @param type The poller type to adjust.
 * @param poll_time The time(in seconds) between ticks of the timer.
 *
 * This will adjust the time between ticks of the given timer type defined by
 * @p type to the time period defined by @p poll_time.
 */
EAPI void ecore_poller_poll_interval_set(Ecore_Poller_Type type, double poll_time);

/**
 * @brief Gets the time(in seconds) between ticks for the given poller type.
 * @param type The poller type to query.
 * @return The time in seconds between ticks of the poller timer.
 *
 * This will get the time between ticks of the specified poller timer.
 */
EAPI double ecore_poller_poll_interval_get(Ecore_Poller_Type type);

/**
 * @brief Polling interval rate of the poller.
 *
 * @param[in] interval The tick interval; must be a power of 2 and <= 32768.
 *
 * @return @c true on success, @c false on failure.
 *
 * @ingroup Ecore_Poller
 */
EAPI Eina_Bool ecore_poller_poller_interval_set(Ecore_Poller *obj, int interval);

/**
 * @brief Polling interval rate of the poller.
 *
 * @return The tick interval; must be a power of 2 and <= 32768.
 *
 * @ingroup Ecore_Poller
 */
EAPI int ecore_poller_poller_interval_get(const Ecore_Poller *obj);

/**
 * @}
 */

/**
 * @ingroup Ecore_Animator_Group
 *
 * @{
 */

typedef struct _Ecore_Animator Ecore_Animator;

/**
 * @brief Adds an animator to call @p func at every animation tick during main
 * loop execution.
 *
 * @param func The function to call when it ticks off
 * @param data The data to pass to the function
 * @return A handle to the new animator
 *
 * This function adds an animator and returns its handle on success and @c NULL
 * on failure. The function @p func will be called every N seconds where N is
 * the @p frametime interval set by ecore_animator_frametime_set(). The
 * function will be passed the @p data pointer as its parameter.
 *
 * When the animator @p func is called, it must return a boolean value.
 * If it returns @c EINA_TRUE (or @c ECORE_CALLBACK_RENEW), it will be called again at
 * the next tick, or if it returns @c EINA_FALSE (or @c ECORE_CALLBACK_CANCEL) it will be
 * deleted automatically making any references/handles for it invalid.
 * @see ecore_animator_timeline_add()
 * @see ecore_animator_frametime_set()
 *
 * @note The default @p frametime value is 1/30th of a second.
 *
 */
EAPI Ecore_Animator *ecore_animator_add(Ecore_Task_Cb func, const void *data);

/**
 * @brief Adds an animator that runs for a limited time.
 *
 * @param runtime The time to run in seconds
 * @param func The function to call when it ticks off
 * @param data The data to pass to the function
 * @return A handle to the new animator
 *
 * This function is just like ecore_animator_add() except the animator only
 * runs for a limited time specified in seconds by @p runtime. Once the
 * runtime the animator has elapsed (animator finished) it will automatically
 * be deleted. The callback function @p func can return @c ECORE_CALLBACK_RENEW
 * to keep the animator running or @c ECORE_CALLBACK_CANCEL or stop it and have
 * it be deleted automatically at any time. Just like timers, the start of
 * The animation is "now" (when the loop woke up - gettable with
 * ecore_loop_time_get()).
 *
 * The @p func will ALSO be passed a position parameter that will be in value
 * from 0.0 to 1.0 to indicate where along the timeline (0.0 start, 1.0 end)
 * the animator run is at. If the callback wishes not to have a linear
 * transition it can "map" this value to one of several curves and mappings
 * via ecore_animator_pos_map().
 * @see ecore_animator_add()
 * @see ecore_animator_pos_map()
 *
 * @note The default @p frametime value is 1/30th of a second.
 * @note The first position parameter passed to the callback will never be 0.
 *
 * @since 1.1.0
 */
EAPI Ecore_Animator *ecore_animator_timeline_add(double runtime, Ecore_Timeline_Cb func, const void *data);

/**
 * @brief Deletes the specified animator from the animator list.
 *
 * @param animator The animator to delete
 * @return The data pointer set for the animator on add
 *
 * Deletes the specified @p animator from the set of animators that are
 * executed during main loop execution. This function returns the data
 * parameter that was being passed to the callback on success, or @c NULL on
 * failure. After this call returns the specified animator object @p animator
 * is invalid and should not be used again. It will not get called again after
 * deletion.
 */
EAPI void *ecore_animator_del(Ecore_Animator *animator);

/**
 * @brief Suspends the specified animator.
 *
 * @param animator The animator to delete
 *
 * The specified @p animator will be temporarily removed from the set of
 * animators that are executed during main loop.
 *
 * @warning Freezing an animator doesn't freeze accounting of how long that
 * animator has been running. Therefore if the animator was created with
 *ecore_animator_timeline_add() the @p pos argument given to the callback
 * will increase as if the animator hadn't been frozen and the animator may
 * have it's execution halted if @p runtime elapsed.
 */
EAPI void ecore_animator_freeze(Ecore_Animator *animator);

/**
 * @brief Restores execution of the specified animator.
 *
 * @param animator The animator to delete
 *
 * The specified @p animator will be put back in the set of animators that are
 * executed during main loop.
 */
EAPI void ecore_animator_thaw(Ecore_Animator *animator);

/**
 * @}
 */

/**
 * @ingroup Ecore_Timer_Group
 *
 * @{
 */

/**
 * Creates a timer to call the given function in the given period of time.
 * @param   in   The interval in seconds.
 * @param   func The given function.  If @p func returns @c 1, the timer is
 *               rescheduled for the next interval @p in.
 * @param   data Data to pass to @p func when it is called.
 * @return  A timer object on success,  @c NULL on failure.
 *
 * This function adds a timer and returns its handle on success and @c NULL on
 * failure. The function @p func will be called every @p in seconds. The
 * function will be passed the @p data pointer as its parameter.
 *
 * When the timer @p func is called, it must return a value of either @c 1
 * (or @c ECORE_CALLBACK_RENEW) or @c 0 (or @c ECORE_CALLBACK_CANCEL).
 * If it returns @c 1, it will be called again at the next tick, or if it returns
 * @c 0 it will be deleted automatically making any references/handles for it
 * invalid.
 */
EAPI Ecore_Timer *ecore_timer_add(double in, Ecore_Task_Cb func, const void *data);

/**
 * Creates a timer to call the given function in the given period of time.
 * @param   in   The interval in seconds from current loop time.
 * @param   func The given function.  If @p func returns 1, the timer is
 *               rescheduled for the next interval @p in.
 * @param   data Data to pass to @p func when it is called.
 * @return  A timer object on success,  @c NULL on failure.
 *
 * This is the same as ecore_timer_add(), but "now" is the time from
 * ecore_loop_time_get() not ecore_time_get() as ecore_timer_add() uses. See
 * ecore_timer_add() for more details.
 */
EAPI Ecore_Timer *ecore_timer_loop_add(double in, Ecore_Task_Cb func, const void *data);

/**
 * Deletes the specified timer from the timer list.
 * @param   timer The timer to delete.
 * @return  The data pointer set for the timer when @ref ecore_timer_add was
 *          called.  @c NULL is returned if the function is unsuccessful.
 *
 * Note: @p timer must be a valid handle. If the timer function has already
 * returned @c 0, the handle is no longer valid (and does not need to be delete).
 */
EAPI void *ecore_timer_del(Ecore_Timer *timer);

/**
 * Pauses a running timer.
 *
 * @param timer The timer to be paused.
 *
 * @remarks The timer callback won't be called while the timer is paused. The remaining
 *          time until the timer expires will be saved, so the timer can be resumed with
 *          that same remaining time to expire, instead of expiring instantly.  Use
 *          ecore_timer_thaw() to resume it.
 *
 * @note Nothing happens if the timer was already paused.
 *
 * @see ecore_timer_thaw()
 */
EAPI void ecore_timer_freeze(Ecore_Timer *timer);

/**
 * @brief Return whether the timer is freezing.
 *
 * @return True if the timer object is freezed, false otherwise.
 *
 * @see ecore_timer_freeze(), ecore_timer_thaw()
 */
EAPI Eina_Bool ecore_timer_freeze_get(Ecore_Timer *timer);

/**
 * @brief Resumes a frozen (paused) timer.
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

#include "efl_loop_timer.eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Ecore_Idle_Group
 *
 * @{
 */

/**
 * Adds an idler handler.
 * @param  func The function to call when idling.
 * @param  data The data to be passed to this @p func call.
 * @return A idler handle if successfully added, @c NULL otherwise.
 *
 * Add an idler handle to the event loop, returning a handle on
 * success and @c NULL otherwise. The function @p func will be called
 * repeatedly while no other events are ready to be processed, as
 * long as it returns @c 1 (or @c ECORE_CALLBACK_RENEW). A return of @c 0
 * (or @c ECORE_CALLBACK_CANCEL) deletes the idler.
 *
 * Idlers are useful for progressively processing data without blocking.
 */
EAPI Ecore_Idler *ecore_idler_add(Ecore_Task_Cb func, const void *data);

/**
 * Deletes an idler callback from the list to be executed.
 * @param  idler The handle of the idler callback to delete
 * @return The data pointer passed to the idler callback on success, @c NULL
 *         otherwise.
 */
EAPI void *ecore_idler_del(Ecore_Idler *idler);

/**
 * Adds an idle enterer handler.
 * @param   func The function to call when entering an idle state.
 * @param   data The data to be passed to the @p func call
 * @return  A handle to the idle enterer callback if successful.  Otherwise,
 *          @c NULL is returned.
 * @note The function func will be called every time the main loop is entering
 * idle state, as long as it returns @c 1 (or @c ECORE_CALLBACK_RENEW). A return of @c 0
 * (or @c ECORE_CALLBACK_CANCEL) deletes the idle enterer.
 */
EAPI Ecore_Idle_Enterer *ecore_idle_enterer_add(Ecore_Task_Cb func, const void *data);

/**
 * Adds an idle enterer handler at the start of the list so it gets called earlier than others.
 * @param   func The function to call when entering an idle state.
 * @param   data The data to be passed to the @p func call
 * @return  A handle to the idle enterer callback if successful.  Otherwise,
 *          @c NULL is returned.
 * @note The function func will be called every time the main loop is entering
 * idle state, as long as it returns @c 1 (or @c ECORE_CALLBACK_RENEW). A return of @c 0
 * (or @c ECORE_CALLBACK_CANCEL) deletes the idle enterer.
 */
EAPI Ecore_Idle_Enterer *ecore_idle_enterer_before_add(Ecore_Task_Cb func, const void *data);

/**
 * Deletes an idle enterer callback.
 * @param   idle_enterer The idle enterer to delete
 * @return  The data pointer passed to the idler enterer callback on success.
 *          @c NULL otherwise.
 */
EAPI void *ecore_idle_enterer_del(Ecore_Idle_Enterer *idle_enterer);

/**
 * Adds an idle exiter handler.
 * @param func The function to call when exiting an idle state.
 * @param data The data to be passed to the @p func call.
 * @return A handle to the idle exiter callback on success.  @c NULL otherwise.
 * @note The function func will be called every time the main loop is exiting
 * idle state, as long as it returns @c 1 (or @c ECORE_CALLBACK_RENEW). A return of @c 0
 * (or @c ECORE_CALLBACK_CANCEL) deletes the idle exiter.
 */
EAPI Ecore_Idle_Exiter *ecore_idle_exiter_add(Ecore_Task_Cb func, const void *data);

/**
 * Deletes an idle exiter handler from the list to be run on exiting idle state.
 * @param idle_exiter The idle exiter to delete
 * @return The data pointer that was being being passed to the handler if
 *         successful. @c NULL otherwise.
 */
EAPI void *ecore_idle_exiter_del(Ecore_Idle_Exiter *idle_exiter);

/**
 * @}
 */

/**
 * @ingroup Ecore_Exe_Group
 *
 * @{
 */

#include "ecore_exe.eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Ecore_Job_Group
 *
 * @{
 */
/**
 * Adds a job to the event queue.
 * @param   func The function to call when the job gets handled.
 * @param   data Data pointer to be passed to the job function when the job is
 *               handled.
 * @return  The handle of the job.  @c NULL is returned if the job could not be
 *          added to the queue.
 * @note    Once the job has been executed, the job handle is invalid.
 */
EAPI Ecore_Job *ecore_job_add(Ecore_Cb func, const void *data);

/**
 * Deletes a queued job that has not yet been executed.
 * @param   obj  Handle of the job to delete.
 * @return  The data pointer that was to be passed to the job.
 */
EAPI void *ecore_job_del(Ecore_Job *obj);

/**
 * @}
 */

#ifdef EFL_BETA_API_SUPPORT
EAPI Ecore_Animator *ecore_evas_animator_timeline_add(void *evo, double runtime, Ecore_Timeline_Cb func, const void *data);
EAPI Ecore_Animator *ecore_evas_animator_add(void *evo, Ecore_Task_Cb func, const void *data);
#endif /* EFL_BETA_API_SUPPORT */

#endif
