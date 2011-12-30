/**
 * @defgroup Button Button
 *
 * @image html img/widget/button/preview-00.png
 * @image latex img/widget/button/preview-00.eps
 * @image html img/widget/button/preview-01.png
 * @image latex img/widget/button/preview-01.eps
 * @image html img/widget/button/preview-02.png
 * @image latex img/widget/button/preview-02.eps
 *
 * This is a push-button. Press it and run some function. It can contain
 * a simple label and icon object and it also has an autorepeat feature.
 *
 * This widgets emits the following signals:
 * @li "clicked": the user clicked the button (press/release).
 * @li "repeated": the user pressed the button without releasing it.
 * @li "pressed": button was pressed.
 * @li "unpressed": button was released after being pressed.
 * In all three cases, the @c event parameter of the callback will be
 * @c NULL.
 *
 * Also, defined in the default theme, the button has the following styles
 * available:
 * @li default: a normal button.
 * @li anchor: Like default, but the button fades away when the mouse is not
 * over it, leaving only the text or icon.
 * @li hoversel_vertical: Internally used by @ref Hoversel to give a
 * continuous look across its options.
 * @li hoversel_vertical_entry: Another internal for @ref Hoversel.
 *
 * Default contents parts of the button widget that you can use for are:
 * @li "icon" - An icon of the button
 *
 * Default text parts of the button widget that you can use for are:
 * @li "default" - Label of the button
 *
 * Follow through a complete example @ref button_example_01 "here".
 * @{
 */

/**
 * Add a new button to the parent's canvas
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 */
EAPI Evas_Object *
                                  elm_button_add(Evas_Object *parent)
EINA_ARG_NONNULL(1);

/**
 * Set the label used in the button
 *
 * The passed @p label can be NULL to clean any existing text in it and
 * leave the button as an icon only object.
 *
 * @param obj The button object
 * @param label The text will be written on the button
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_button_label_set(Evas_Object *obj, const char *label) EINA_ARG_NONNULL(1);

/**
 * Get the label set for the button
 *
 * The string returned is an internal pointer and should not be freed or
 * altered. It will also become invalid when the button is destroyed.
 * The string returned, if not NULL, is a stringshare, so if you need to
 * keep it around even after the button is destroyed, you can use
 * eina_stringshare_ref().
 *
 * @param obj The button object
 * @return The text set to the label, or NULL if nothing is set
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_button_label_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the icon used for the button
 *
 * Setting a new icon will delete any other that was previously set, making
 * any reference to them invalid. If you need to maintain the previous
 * object alive, unset it first with elm_button_icon_unset().
 *
 * @param obj The button object
 * @param icon The icon object for the button
 * @deprecated use elm_object_part_content_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_button_icon_set(Evas_Object *obj, Evas_Object *icon) EINA_ARG_NONNULL(1);

/**
 * Get the icon used for the button
 *
 * Return the icon object which is set for this widget. If the button is
 * destroyed or another icon is set, the returned object will be deleted
 * and any reference to it will be invalid.
 *
 * @param obj The button object
 * @return The icon object that is being used
 *
 * @deprecated use elm_object_part_content_get() instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_button_icon_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Remove the icon set without deleting it and return the object
 *
 * This function drops the reference the button holds of the icon object
 * and returns this last object. It is used in case you want to remove any
 * icon, or set another one, without deleting the actual object. The button
 * will be left without an icon set.
 *
 * @param obj The button object
 * @return The icon object that was being used
 * @deprecated use elm_object_part_content_unset() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_button_icon_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

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
 */
EAPI void                         elm_button_autorepeat_set(Evas_Object *obj, Eina_Bool on) EINA_ARG_NONNULL(1);

/**
 * Get whether the autorepeat feature is enabled
 *
 * @param obj The button object
 * @return EINA_TRUE if autorepeat is on, EINA_FALSE otherwise
 *
 * @see elm_button_autorepeat_set()
 */
EAPI Eina_Bool                    elm_button_autorepeat_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the initial timeout before the autorepeat event is generated
 *
 * Sets the timeout, in seconds, since the button is pressed until the
 * first @c repeated signal is emitted. If @p t is 0.0 or less, there
 * won't be any delay and the even will be fired the moment the button is
 * pressed.
 *
 * @param obj The button object
 * @param t   Timeout in seconds
 *
 * @see elm_button_autorepeat_set()
 * @see elm_button_autorepeat_gap_timeout_set()
 */
EAPI void                         elm_button_autorepeat_initial_timeout_set(Evas_Object *obj, double t) EINA_ARG_NONNULL(1);

/**
 * Get the initial timeout before the autorepeat event is generated
 *
 * @param obj The button object
 * @return Timeout in seconds
 *
 * @see elm_button_autorepeat_initial_timeout_set()
 */
EAPI double                       elm_button_autorepeat_initial_timeout_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
 */
EAPI void                         elm_button_autorepeat_gap_timeout_set(Evas_Object *obj, double t) EINA_ARG_NONNULL(1);

/**
 * Get the interval between each generated autorepeat event
 *
 * @param obj The button object
 * @return Interval in seconds
 */
EAPI double                       elm_button_autorepeat_gap_timeout_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @}
 */
