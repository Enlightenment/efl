/**
 * Add a new clock widget to the given parent Elementary
 * (container) object
 *
 * @param parent The parent object
 * @return a new clock widget handle or @c NULL, on errors
 *
 * This function inserts a new clock widget on the canvas.
 *
 * @ingroup Clock
 */
EAPI Evas_Object      *elm_clock_add(Evas_Object *parent);

/**
 * Set a clock widget's time, programmatically
 *
 * @param obj The clock widget object
 * @param hrs The hours to set
 * @param min The minutes to set
 * @param sec The seconds to set
 *
 * This function updates the time that is showed by the clock
 * widget.
 *
 *  Values @b must be set within the following ranges:
 * - 0 - 23, for hours
 * - 0 - 59, for minutes
 * - 0 - 59, for seconds,
 *
 * even if the clock is not in "military" mode.
 *
 * @warning The behavior for values set out of those ranges is @b
 * undefined.
 *
 * @ingroup Clock
 */
EAPI void              elm_clock_time_set(Evas_Object *obj, int hrs, int min, int sec);

/**
 * Get a clock widget's time values
 *
 * @param obj The clock object
 * @param[out] hrs Pointer to the variable to get the hours value
 * @param[out] min Pointer to the variable to get the minutes value
 * @param[out] sec Pointer to the variable to get the seconds value
 *
 * This function gets the time set for @p obj, returning
 * it on the variables passed as the arguments to function
 *
 * @note Use @c NULL pointers on the time values you're not
 * interested in: they'll be ignored by the function.
 *
 * @ingroup Clock
 */
EAPI void              elm_clock_time_get(const Evas_Object *obj, int *hrs, int *min, int *sec);

/**
 * Set whether a given clock widget is under <b>edition mode</b> or
 * under (default) displaying-only mode.
 *
 * @param obj The clock object
 * @param edit @c EINA_TRUE to put it in edition, @c EINA_FALSE to
 * put it back to "displaying only" mode
 *
 * This function makes a clock's time to be editable or not <b>by
 * user interaction</b>. When in edition mode, clocks @b stop
 * ticking, until one brings them back to canonical mode. The
 * elm_clock_edit_mode_set() function will influence which digits
 * of the clock will be editable.
 *
 * @note am/pm sheets, if being shown, will @b always be editable
 * under edition mode.
 *
 * @see elm_clock_edit_get()
 *
 * @ingroup Clock
 */
EAPI void              elm_clock_edit_set(Evas_Object *obj, Eina_Bool edit);

/**
 * Retrieve whether a given clock widget is under editing mode
 * or under (default) displaying-only mode.
 *
 * @param obj The clock object
 * @return @c EINA_TRUE, if it's in edition mode, @c EINA_FALSE otherwise
 *
 * This function retrieves whether the clock's time can be edited
 * or not by user interaction.
 *
 * @see elm_clock_edit_set() for more details
 *
 * @ingroup Clock
 */
EAPI Eina_Bool         elm_clock_edit_get(const Evas_Object *obj);

/**
 * Set what digits of the given clock widget should be editable
 * when in edition mode.
 *
 * @param obj The clock object
 * @param digedit Bit mask indicating the digits to be editable
 * (values in #Elm_Clock_Edit_Mode).
 *
 * @see elm_clock_edit_mode_get()
 *
 * @ingroup Clock
 */
EAPI void              elm_clock_edit_mode_set(Evas_Object *obj, Elm_Clock_Edit_Mode digedit);

/**
 * Retrieve what digits of the given clock widget should be
 * editable when in edition mode.
 *
 * @param obj The clock object
 * @return Bit mask indicating the digits to be editable
 * (values in #Elm_Clock_Edit_Mode).
 *
 * @see elm_clock_edit_mode_set() for more details
 *
 * @ingroup Clock
 */
EAPI Elm_Clock_Edit_Mode elm_clock_edit_mode_get(const Evas_Object *obj);

/**
 * Set if the given clock widget must show hours in military or
 * am/pm mode
 *
 * @param obj The clock object
 * @param am_pm @c EINA_TRUE to put it in am/pm mode, @c EINA_FALSE
 * to military mode
 *
 * This function sets if the clock must show hours in military or
 * am/pm mode. In some countries like Brazil the military mode
 * (00-24h-format) is used, in opposition to the USA, where the
 * am/pm mode is more commonly used.
 *
 * @see elm_clock_show_am_pm_get()
 *
 * @ingroup Clock
 */
EAPI void              elm_clock_show_am_pm_set(Evas_Object *obj, Eina_Bool am_pm);

/**
 * Get if the given clock widget shows hours in military or am/pm
 * mode
 *
 * @param obj The clock object
 * @return @c EINA_TRUE, if in am/pm mode, @c EINA_FALSE if in
 * military
 *
 * This function gets if the clock shows hours in military or am/pm
 * mode.
 *
 * @see elm_clock_show_am_pm_set() for more details
 *
 * @ingroup Clock
 */
EAPI Eina_Bool         elm_clock_show_am_pm_get(const Evas_Object *obj);

/**
 * Set if the given clock widget must show time with seconds or not
 *
 * @param obj The clock object
 * @param seconds @c EINA_TRUE to show seconds, @c EINA_FALSE otherwise
 *
 * This function sets if the given clock must show or not elapsed
 * seconds. By default, they are @b not shown.
 *
 * @see elm_clock_show_seconds_get()
 *
 * @ingroup Clock
 */
EAPI void              elm_clock_show_seconds_set(Evas_Object *obj, Eina_Bool seconds);

/**
 * Get whether the given clock widget is showing time with seconds
 * or not
 *
 * @param obj The clock object
 * @return @c EINA_TRUE if it's showing seconds, @c EINA_FALSE otherwise
 *
 * This function gets whether @p obj is showing or not the elapsed
 * seconds.
 *
 * @see elm_clock_show_seconds_set()
 *
 * @ingroup Clock
 */
EAPI Eina_Bool         elm_clock_show_seconds_get(const Evas_Object *obj);

/**
 * Set the first interval on time updates for a user mouse button hold
 * on clock widgets' time edition.
 *
 * @param obj The clock object
 * @param interval The first interval value in seconds
 *
 * This interval value is @b decreased while the user holds the
 * mouse pointer either incrementing or decrementing a given the
 * clock digit's value.
 *
 * This helps the user to get to a given time distant from the
 * current one easier/faster, as it will start to flip quicker and
 * quicker on mouse button holds.
 *
 * The calculation for the next flip interval value, starting from
 * the one set with this call, is the previous interval divided by
 * 1.05, so it decreases a little bit.
 *
 * The default starting interval value for automatic flips is
 * @b 0.85 seconds.
 *
 * @see elm_clock_first_interval_get()
 *
 * @ingroup Clock
 */
EAPI void              elm_clock_first_interval_set(Evas_Object *obj, double interval);

/**
 * Get the first interval on time updates for a user mouse button hold
 * on clock widgets' time edition.
 *
 * @param obj The clock object
 * @return The first interval value, in seconds, set on it
 *
 * @see elm_clock_first_interval_set() for more details
 *
 * @ingroup Clock
 */
EAPI double            elm_clock_first_interval_get(const Evas_Object *obj);

/**
 * Set whether the given clock widget should be paused or not.
 * 
 * @param obj The clock object
 * @param pause @c EINA_TRUE to pause clock, @c EINA_FALSE otherwise
 * 
 * This function pauses or starts the clock widget.
 * 
 * @see elm_clock_pause_get()
 * 
 * @ingroup Clock
 * @since 1.9
 */
EAPI void              elm_clock_pause_set(Evas_Object *obj, Eina_Bool pause);

/**
 * Get whether the given clock widget is paused.
 * 
 * @param obj The clock object
 * @return @c EINA_TRUE if it's paused @c EINA_FALSE otherwise
 * 
 * This function gets whether the clock is paused or not.
 * 
 * @see elm_clock_pause_set()
 * 
 * @ingroup Clock
 * @since 1.9
 */
EAPI Eina_Bool         elm_clock_pause_get(const Evas_Object *obj);
