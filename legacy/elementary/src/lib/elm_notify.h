/**
 * @defgroup Notify Notify
 * @ingroup Elementary
 *
 * @image html notify_inheritance_tree.png
 * @image latex notify_inheritance_tree.eps
 *
 * @image html img/widget/notify/preview-00.png
 * @image latex img/widget/notify/preview-00.eps
 *
 * Display a container in a particular region of the parent(top, bottom,
 * etc).  A timeout can be set to automatically hide the notify. This is so
 * that, after an evas_object_show() on a notify object, if a timeout was set
 * on it, it will @b automatically get hidden after that time.
 *
 * Signals that you can add callbacks for are:
 * @li "timeout" - when timeout happens on notify and it's hidden
 * @li "block,clicked" - when a click outside of the notify happens
 *
 * This widget inherits from @ref elm-container-class, so that the
 * functions meant to act on it will wor work for mapbuf objects:
 *
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * Default content parts of the notify widget that you can use are:
 * @li @c "default" - The main content of the notify
 *
 * @ref tutorial_notify show usage of the API.
 *
 * @{
 */

/**
 * @brief Possible orient values for notify.
 *
 * This values should be used in conjunction to elm_notify_orient_set() to
 * set the position in which the notify should appear(relative to its parent)
 * and in conjunction with elm_notify_orient_get() to know where the notify
 * is appearing.
 */
typedef enum
{
   ELM_NOTIFY_ORIENT_TOP, /**< Notify should appear in the top of parent, default */
   ELM_NOTIFY_ORIENT_CENTER, /**< Notify should appear in the center of parent */
   ELM_NOTIFY_ORIENT_BOTTOM, /**< Notify should appear in the bottom of parent */
   ELM_NOTIFY_ORIENT_LEFT, /**< Notify should appear in the left of parent */
   ELM_NOTIFY_ORIENT_RIGHT, /**< Notify should appear in the right of parent */
   ELM_NOTIFY_ORIENT_TOP_LEFT, /**< Notify should appear in the top left of parent */
   ELM_NOTIFY_ORIENT_TOP_RIGHT, /**< Notify should appear in the top right of parent */
   ELM_NOTIFY_ORIENT_BOTTOM_LEFT, /**< Notify should appear in the bottom left of parent */
   ELM_NOTIFY_ORIENT_BOTTOM_RIGHT, /**< Notify should appear in the bottom right of parent */
   ELM_NOTIFY_ORIENT_LAST /**< Sentinel value, @b don't use */
} Elm_Notify_Orient;

/**
 * @brief Add a new notify to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Notify
 */
EAPI Evas_Object                 *elm_notify_add(Evas_Object *parent);

/**
 * @brief Set the notify parent
 *
 * @param obj The notify object
 * @param parent The new parent
 *
 * Once the parent object is set, a previously set one will be disconnected
 * and replaced.
 *
 * @ingroup Notify
 */
EAPI void                         elm_notify_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * @brief Get the notify parent
 *
 * @param obj The notify object
 * @return The parent
 *
 * @see elm_notify_parent_set()
 *
 * @ingroup Notify
 */
EAPI Evas_Object                 *elm_notify_parent_get(const Evas_Object *obj);

/**
 * @brief Set the orientation
 *
 * @param obj The notify object
 * @param orient The new orientation
 *
 * Sets the position in which the notify will appear in its parent.
 *
 * @see @ref Elm_Notify_Orient for possible values.
 *
 * @ingroup Notify
 */
EAPI void                         elm_notify_orient_set(Evas_Object *obj, Elm_Notify_Orient orient);

/**
 * @brief Return the orientation
 * @param obj The notify object
 * @return The orientation of the notification
 *
 * @see elm_notify_orient_set()
 * @see Elm_Notify_Orient
 *
 * @ingroup Notify
 */
EAPI Elm_Notify_Orient            elm_notify_orient_get(const Evas_Object *obj);

/**
 * @brief Set the time interval after which the notify window is going to be
 * hidden.
 *
 * @param obj The notify object
 * @param timeout The timeout in seconds
 *
 * This function sets a timeout and starts the timer controlling when the
 * notify is hidden. Since calling evas_object_show() on a notify restarts
 * the timer controlling when the notify is hidden, setting this before the
 * notify is shown will in effect mean starting the timer when the notify is
 * shown.
 *
 * @note Set a value <= 0.0 to disable a running timer.
 *
 * @note If the value > 0.0 and the notify is previously visible, the
 * timer will be started with this value, canceling any running timer.
 *
 * @ingroup Notify
 */
EAPI void                         elm_notify_timeout_set(Evas_Object *obj, double timeout);

/**
 * @brief Return the timeout value (in seconds)
 * @param obj the notify object
 *
 * @see elm_notify_timeout_set()
 *
 * @ingroup Notify
 */
EAPI double                       elm_notify_timeout_get(const Evas_Object *obj);

/**
 * @brief Sets whether events should be passed to by a click outside
 * its area.
 *
 * @param obj The notify object
 * @param allow EINA_TRUE If events are allowed, otherwise not
 *
 * When true if the user clicks outside the window the events will be caught
 * by the others widgets, else the events are blocked.
 *
 * @note The default value is EINA_TRUE.
 *
 * @ingroup Notify
 */
EAPI void                         elm_notify_allow_events_set(Evas_Object *obj, Eina_Bool allow);

/**
 * @brief Return true if events are allowed below the notify object
 * @param obj the notify object
 *
 * @see elm_notify_allow_events_set()
 *
 * @ingroup Notify
 */
EAPI Eina_Bool                    elm_notify_allow_events_get(const Evas_Object *obj);

/**
 * @}
 */
