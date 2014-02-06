/**
 * Add a new button to the parent's canvas
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Button
 */
EAPI Evas_Object                 *elm_button_add(Evas_Object *parent);

/**
 * Turn on/off the autorepeat event generated when the button is kept pressed
 *
 * When off, no autorepeat is performed and buttons emit a normal @c clicked
 * signal when they are clicked.
 *
 * When on, keeping a button pressed will continuously emit a @c repeated
 * signal until the button is released. The time it takes until it starts
 * emitting the signal is given by
 * elm_button_autorepeat_initial_timeout_set(), and the time between each
 * new emission by elm_button_autorepeat_gap_timeout_set().
 *
 * @param obj The button object
 * @param on  A bool to turn on/off the event
 *
 * @ingroup Button
 */
EAPI void                         elm_button_autorepeat_set(Evas_Object *obj, Eina_Bool on);

/**
 * Get whether the autorepeat feature is enabled
 *
 * @param obj The button object
 * @return @c EINA_TRUE if autorepeat is on, @c EINA_FALSE otherwise
 *
 * @see elm_button_autorepeat_set()
 *
 * @ingroup Button
 */
EAPI Eina_Bool                    elm_button_autorepeat_get(const Evas_Object *obj);

/**
 * Set the initial timeout before the autorepeat event is generated
 *
 * Sets the timeout, in seconds, since the button is pressed until the
 * first @c repeated signal is emitted. If @p t is 0.0 or less, there
 * won't be any delay and the event will be fired the moment the button is
 * pressed.
 *
 * @param obj The button object
 * @param t   Timeout in seconds
 *
 * @see elm_button_autorepeat_set()
 * @see elm_button_autorepeat_gap_timeout_set()
 *
 * @ingroup Button
 */
EAPI void                         elm_button_autorepeat_initial_timeout_set(Evas_Object *obj, double t);

/**
 * Get the initial timeout before the autorepeat event is generated
 *
 * @param obj The button object
 * @return Timeout in seconds
 *
 * @see elm_button_autorepeat_initial_timeout_set()
 *
 * @ingroup Button
 */
EAPI double                       elm_button_autorepeat_initial_timeout_get(const Evas_Object *obj);

/**
 * Set the interval between each generated autorepeat event
 *
 * After the first @c repeated event is fired, all subsequent ones will
 * follow after a delay of @p t seconds for each.
 *
 * @param obj The button object
 * @param t   Interval in seconds
 *
 * @see elm_button_autorepeat_initial_timeout_set()
 *
 * @ingroup Button
 */
EAPI void                         elm_button_autorepeat_gap_timeout_set(Evas_Object *obj, double t);

/**
 * Get the interval between each generated autorepeat event
 *
 * @param obj The button object
 * @return Interval in seconds
 *
 * @ingroup Button
 */
EAPI double                       elm_button_autorepeat_gap_timeout_get(const Evas_Object *obj);
