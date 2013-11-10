#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup Ecore_Poller_Group
 *
 * @{
 */

/**
 * @brief Changes the polling interval rate of @p poller.
 * @param poller The Ecore_Poller to change the interval of.
 * @param interval The tick interval to set; must be a power of 2 and <= 32768.
 * @return Returns true on success, false on failure.
 *
 * This allows the changing of a poller's polling interval. It is useful when
 * you want to alter a poll rate without deleting and re-creating a poller.
 */
EAPI Eina_Bool ecore_poller_poller_interval_set(Ecore_Poller *poller, int interval);
/**
 * @brief Gets the polling interval rate of @p poller.
 * @param poller The Ecore_Poller to change the interval of.
 * @return Returns the interval, in ticks, that @p poller polls at.
 *
 * This returns a poller's polling interval, or 0 on error.
 */
EAPI int ecore_poller_poller_interval_get(Ecore_Poller *poller);
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
 * ECORE_CALLBACK_RENEW(or 1) or ECORE_CALLBACK_CANCEL(or 0). If it
 * returns 1, it will be called again at the next tick, or if it returns
 * 0 it will be deleted automatically making any references/handles for it
 * invalid.
 */
EAPI Ecore_Poller *ecore_poller_add(Ecore_Poller_Type type, int interval, Ecore_Task_Cb func, const void *data);
/**
 * @brief Delete the specified poller from the timer list.
 * @param poller The poller to delete.
 * @return The data pointer set for the timer when @ref ecore_poller_add was
 * called on success, @c NULL otherwise.
 *
 * @note @a poller must be a valid handle. If the poller function has already
 * returned 0, the handle is no longer valid (and does not need to be deleted).
 */
EAPI void *ecore_poller_del(Ecore_Poller *poller);

/**
 * @}
 */

/**
 * @ingroup Ecore_Animator_Group
 *
 * @{
 */

/**
 * @brief Add an animator to call @p func at every animation tick during main
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
 * If it returns EINA_TRUE (or ECORE_CALLBACK_RENEW), it will be called again at
 * the next tick, or if it returns EINA_FALSE (or ECORE_CALLBACK_CANCEL) it will be
 * deleted automatically making any references/handles for it invalid.
 *
 * @note The default @p frametime value is 1/30th of a second.
 *
 * @see ecore_animator_timeline_add()
 * @see ecore_animator_frametime_set()
 */
EAPI Ecore_Animator *ecore_animator_add(Ecore_Task_Cb func, const void *data);
/**
 * @brief Add an animator that runs for a limited time
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
 * parameter that was being passed to the callback on success, or @c NULL on
 * failure. After this call returns the specified animator object @p animator
 * is invalid and should not be used again. It will not get called again after
 * deletion.
 */
EAPI void *ecore_animator_del(Ecore_Animator *animator);
/**
 * @brief Suspend the specified animator.
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
 * @brief Restore execution of the specified animator.
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
EAPI Ecore_Timer *ecore_timer_add(double in, Ecore_Task_Cb func, const void *data);
EAPI Ecore_Timer *ecore_timer_loop_add(double in, Ecore_Task_Cb func, const void *data);
EAPI void *ecore_timer_del(Ecore_Timer *timer);
EAPI void ecore_timer_interval_set(Ecore_Timer *timer, double in);
EAPI double ecore_timer_interval_get(Ecore_Timer *timer);
EAPI void ecore_timer_freeze(Ecore_Timer *timer);
EAPI Eina_Bool ecore_timer_freeze_get(Ecore_Timer *timer);
EAPI void ecore_timer_thaw(Ecore_Timer *timer);
EAPI void ecore_timer_delay(Ecore_Timer *timer, double add);
EAPI void ecore_timer_reset(Ecore_Timer *timer);
EAPI double ecore_timer_pending_get(Ecore_Timer *timer);
/**
 * @}
 */

/**
 * @ingroup Ecore_Idle_Group
 *
 * @{
 */
/**
 * Add an idler handler.
 * @param  func The function to call when idling.
 * @param  data The data to be passed to this @p func call.
 * @return A idler handle if successfully added, @c NULL otherwise.
 *
 * Add an idler handle to the event loop, returning a handle on
 * success and @c NULL otherwise. The function @p func will be called
 * repeatedly while no other events are ready to be processed, as
 * long as it returns @c 1 (or ECORE_CALLBACK_RENEW). A return of @c 0
 * (or ECORE_CALLBACK_CANCEL) deletes the idler.
 *
 * Idlers are useful for progressively prossessing data without blocking.
 */
EAPI Ecore_Idler *ecore_idler_add(Ecore_Task_Cb func, const void *data);

/**
 * Delete an idler callback from the list to be executed.
 * @param  idler The handle of the idler callback to delete
 * @return The data pointer passed to the idler callback on success, @c NULL
 *         otherwise.
 */
EAPI void *ecore_idler_del(Ecore_Idler *idler);

EAPI Ecore_Idle_Enterer *ecore_idle_enterer_add(Ecore_Task_Cb func, const void *data);
EAPI Ecore_Idle_Enterer *ecore_idle_enterer_before_add(Ecore_Task_Cb func, const void *data);
EAPI void *ecore_idle_enterer_del(Ecore_Idle_Enterer *idle_enterer);

EAPI Ecore_Idle_Exiter *ecore_idle_exiter_add(Ecore_Task_Cb func, const void *data);
EAPI void *ecore_idle_exiter_del(Ecore_Idle_Exiter *idle_exiter);
/**
 * @}
 */

/**
 * @ingroup Ecore_Job_Group
 *
 * @{
 */
EAPI Ecore_Job *ecore_job_add(Ecore_Cb func, const void *data);
EAPI void *ecore_job_del(Ecore_Job *obj);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif
