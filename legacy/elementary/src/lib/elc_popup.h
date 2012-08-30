/**
 * @defgroup Popup Popup
 * @ingroup Elementary
 *
 * This widget is an enhancement of @ref Notify.
 * In addition to Content area, there are two optional sections namely Title
 * area and Action area.
 *
 * Popup Widget displays its content with a particular orientation in the parent
 * area. This orientation can be one among top, center, bottom, left, top-left,
 * top-right, bottom-left and bottom-right.
 * Content part of Popup can be an Evas Object set by application or it can be
 * Text set by application or set of items containing an icon and/or text.
 * The content/item-list can be removed using elm_object_content_set with second
 * parameter passed as NULL.
 *
 * Following figures shows the textual layouts of popup in which Title Area and
 * Action area area are optional ones.
 * Action area can have up to 3 buttons handled using elm_object common APIs
 * mentioned below. If user wants to have more than 3 buttons then these buttons
 * can be put inside the items of a list as content.
 * User needs to handle the clicked signal of these action buttons if required.
 * No event is processed by the widget automatically when clicked on these
 * action buttons.
 *
 * <pre>
 *
 *  |---------------------|    |---------------------|    |---------------------|
 * |     Title Area      |    |     Title Area      |    |     Title Area      |
 * |Icon|    Text        |    |Icon|    Text        |    |Icon|    Text        |
 * |---------------------|    |---------------------|    |---------------------|
 * |       Item 1        |    |                     |    |                     |
 * |---------------------|    |                     |    |                     |
 * |       Item 2        |    |                     |    |    Description      |
 * |---------------------|    |       Content       |    |                     |
 * |       Item 3        |    |                     |    |                     |
 * |---------------------|    |                     |    |                     |
 * |         .           |    |---------------------|    |---------------------|
 * |         .           |    |     Action Area     |    |     Action Area     |
 * |         .           |    | Btn1  |Btn2|. |Btn3 |    | Btn1  |Btn2|  |Btn3 |
 * |---------------------|    |---------------------|    |---------------------|
 * |       Item N        |     Content Based Layout     Description based Layout
 * |---------------------|
 * |     Action Area     |
 * | Btn1  |Btn2|. |Btn3 |
 * |---------------------|
 *    Item Based Layout
 *
 * </pre>
 *
 * Timeout can be set on expiry of which popup instance hides and sends a smart
 * signal "timeout" to the user.
 * The visible region of popup is surrounded by a translucent region called
 * Blocked Event area.
 * By clicking on Blocked Event area, the signal "block,clicked" is sent to
 * the application. This block event area can be avoided by using
 * API elm_popup_allow_events_set.
 * When gets hidden, popup does not get destroyed automatically, application
 * should destroy the popup instance after use.
 * To control the maximum height of the internal scroller for item, we use the
 * height of the action area which is passed by theme based on the number of
 * buttons currently set to popup.
 *
 * Signals that you can add callbacks for are:
 * @li "timeout" - whenever popup is closed as a result of timeout.
 * @li "block,clicked" - whenever user taps on Blocked Event area.
 *
 * Styles available for Popup
 * @li "default"
 *
 * Default contents parts of the popup widget that you can use for are:
 * @li "default" - The content of the popup
 * @li "title,icon" - Title area's icon
 * @li "button1" - 1st button of the action area
 * @li "button2" - 2nd button of the action area
 * @li "button3" - 3rd button of the action area
 *
 * Default text parts of the popup widget that you can use for are:
 * @li "title,text" - This operates on Title area's label
 * @li "default" - content-text set in the content area of the widget
 *
 * Default contents parts of the popup items that you can use for are:
 * @li "default" -Item's icon
 *
 * Default text parts of the popup items that you can use for are:
 * @li "default" - Item's label
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_disabled_set
 * @li @ref elm_object_item_disabled_get
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 * @li @ref elm_object_item_part_content_set
 * @li @ref elm_object_item_part_content_get
 * @li @ref elm_object_item_signal_emit
 * @li @ref elm_object_item_del
 *
 * Here are some sample code to illustrate Popup usage:
 * @li @ref popup_example_01_c
 * @li @ref popup_example_02_c
 * @li @ref popup_example_03_c
 */

/**
 * @brief Possible orient values for popup.
 *
 * These values should be used in conjunction to elm_popup_orient_set() to
 * set the position in which the popup should appear(relative to its parent)
 * and in conjunction with elm_popup_orient_get() to know where the popup
 * is appearing.
 *
 * @ingroup Popup
 */
typedef enum
{
   ELM_POPUP_ORIENT_TOP = 0, /**< Popup should appear in the top of parent, default */
   ELM_POPUP_ORIENT_CENTER, /**< Popup should appear in the center of parent */
   ELM_POPUP_ORIENT_BOTTOM, /**< Popup should appear in the bottom of parent */
   ELM_POPUP_ORIENT_LEFT, /**< Popup should appear in the left of parent */
   ELM_POPUP_ORIENT_RIGHT, /**< Popup should appear in the right of parent */
   ELM_POPUP_ORIENT_TOP_LEFT, /**< Popup should appear in the top left of parent */
   ELM_POPUP_ORIENT_TOP_RIGHT, /**< Popup should appear in the top right of parent */
   ELM_POPUP_ORIENT_BOTTOM_LEFT, /**< Popup should appear in the bottom left of parent */
   ELM_POPUP_ORIENT_BOTTOM_RIGHT, /**< Notify should appear in the bottom right of parent */
   ELM_POPUP_ORIENT_LAST /**< Sentinel value, @b don't use */
 } Elm_Popup_Orient;

/**
 * @brief Adds a new Popup to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Popup
 */
EAPI Evas_Object *elm_popup_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

/**
 * @brief Add a new item to a Popup object
 *
 * Both an item list and a content could not be set at the same time!
 * once you add an item, the previous content will be removed.
 *
 * @param obj popup object
 * @param icon Icon to be set on new item
 * @param label The Label of the new item
 * @param func Convenience function called when item selected
 * @param data Data passed to @p func above
 * @return A handle to the item added or @c NULL, on errors
 *
 * @ingroup Popup
 * @warning When the first item is appended to popup object, any previous content
 * of the content area is deleted. At a time, only one of content, content-text
 * and item(s) can be there in a popup content area.
 */
EAPI Elm_Object_Item *elm_popup_item_append(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Sets the wrapping type of content text packed in content
 * area of popup object.
 *
 * @param obj The Popup object
 * @param wrap wrapping type of type Elm_Wrap_Type
 *
 * @ingroup Popup
 * @see elm_popup_content_text_wrap_type_get()
 */
EAPI void elm_popup_content_text_wrap_type_set(Evas_Object *obj, Elm_Wrap_Type wrap) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the wrapping type of content text packed in content area of
 * popup object.
 *
 * @param obj The Popup object
 * @return wrap type of the content text
 *
 * @ingroup Popup
 * @see elm_popup_content_text_wrap_type_set
 */
EAPI Elm_Wrap_Type elm_popup_content_text_wrap_type_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Sets the orientation of the popup in the parent region
 *
 * @param obj The popup object
 * @param orient  the orientation of the popup
 *
 * Sets the position in which popup will appear in its parent
 *
 * @ingroup Popup
 * @see @ref Elm_Popup_Orient for possible values.
 */
EAPI void elm_popup_orient_set(Evas_Object *obj, Elm_Popup_Orient orient) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the orientation of Popup
 *
 * @param obj The popup object
 * @return the orientation of the popup
 *
 * @ingroup Popup
 * @see elm_popup_orient_set()
 * @see Elm_Popup_Orient
 */
EAPI Elm_Popup_Orient elm_popup_orient_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Sets a timeout to hide popup automatically
 *
 * @param obj The popup object
 * @param timeout The timeout in seconds
 *
 * This function sets a timeout and starts the timer controlling when the
 * popup is hidden. Since calling evas_object_show() on a popup restarts
 * the timer controlling when it is hidden, setting this before the
 * popup is shown will in effect mean starting the timer when the popup is
 * shown. Smart signal "timeout" is called afterwards which can be handled
 * if needed.
 *
 * @note Set a value <= 0.0 to disable a running timer.
 *
 * @note If the value > 0.0 and the popup is previously visible, the
 * timer will be started with this value, canceling any running timer.
 * @ingroup Popup
 */
EAPI void elm_popup_timeout_set(Evas_Object *obj, double timeout) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the timeout value set to the popup (in seconds)
 *
 * @param obj The popup object
 * @return the timeout value
 *
 * @ingroup Popup
 * @see elm_popup_timeout_set()
 */
EAPI double elm_popup_timeout_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Sets whether events should be passed to by a click outside.
 *
 * @param obj The popup object
 * @param allow EINA_TRUE Events are passed to lower objects, else not
 *
 * Enabling allow event will remove the Blocked event area and events will
 * pass to the lower layer objects otherwise they are blocked.
 *
 * @ingroup Popup
 * @see elm_popup_allow_events_get()
 * @note The default value is EINA_FALSE.
 */
EAPI void elm_popup_allow_events_set(Evas_Object *obj, Eina_Bool allow);

/**
 * @brief Returns value indicating whether allow event is enabled or not
 *
 * @param obj The popup object
 * @return EINA_FALSE if Blocked event area is present else EINA_TRUE
 *
 * @ingroup Popup
 * @see elm_popup_allow_events_set()
 * @note By default the Blocked event area is present
 */
EAPI Eina_Bool elm_popup_allow_events_get(const Evas_Object *obj);
