/**
 * @defgroup Button Button
 * @ingroup Elementary
 *
 * @image html button_inheritance_tree.png
 * @image latex button_inheritance_tree.eps
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
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for button objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li "clicked": the user clicked the button (press/release).
 * @li "repeated": the user pressed the button without releasing it.
 * @li "pressed": button was pressed.
 * @li "unpressed": button was released after being pressed.
 * In all cases, the @c event parameter of the callback will be
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
 * @li naviframe: Internally used by @ref Naviframe for its back button.
 * @li colorselector: Internally used by @ref Colorselector
 * for its left and right buttons.
 *
 * Default content parts of the button widget that you can use for are:
 * @li "icon" - An icon of the button
 *
 * Default text parts of the button widget that you can use for are:
 * @li "default" - Label of the button
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_signal_callback_add
 * @li @ref elm_object_signal_callback_del
 *
 * Here is some sample code using it:
 * @li @ref button_example_00
 * @li @ref button_example_01
 */

/**
 * @addtogroup Button
 * @{
 */

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
 * @return EINA_TRUE if autorepeat is on, EINA_FALSE otherwise
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

/**
 * @}
 */
