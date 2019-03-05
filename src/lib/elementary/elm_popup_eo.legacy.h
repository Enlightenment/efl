#ifndef _ELM_POPUP_EO_LEGACY_H_
#define _ELM_POPUP_EO_LEGACY_H_

#ifndef _ELM_POPUP_EO_CLASS_TYPE
#define _ELM_POPUP_EO_CLASS_TYPE

typedef Eo Elm_Popup;

#endif

#ifndef _ELM_POPUP_EO_TYPES
#define _ELM_POPUP_EO_TYPES

/**
 * @brief Possible orient values for popup.
 *
 * These values should be used in conjunction to elm_popup_orient_set() to set
 * the position in which the popup should appear(relative to its parent) and in
 * conjunction with elm_popup_orient_get() to know where the popup is
 * appearing.
 *
 * @ingroup Elm_Popup
 */
typedef enum
{
  ELM_POPUP_ORIENT_TOP = 0, /**< Popup should appear in the top of parent,
                             * default. */
  ELM_POPUP_ORIENT_CENTER, /**< Popup should appear in the center of parent. */
  ELM_POPUP_ORIENT_BOTTOM, /**< Popup should appear in the bottom of parent. */
  ELM_POPUP_ORIENT_LEFT, /**< Popup should appear in the left of parent. */
  ELM_POPUP_ORIENT_RIGHT, /**< Popup should appear in the right of parent. */
  ELM_POPUP_ORIENT_TOP_LEFT, /**< Popup should appear in the top left of parent.
                              */
  ELM_POPUP_ORIENT_TOP_RIGHT, /**< Popup should appear in the top right of
                               * parent. */
  ELM_POPUP_ORIENT_BOTTOM_LEFT, /**< Popup should appear in the bottom left of
                                 * parent. */
  ELM_POPUP_ORIENT_BOTTOM_RIGHT, /**< Notify should appear in the bottom right
                                  * of parent. */
  ELM_POPUP_ORIENT_LAST /**< Sentinel value to indicate last enum field during
                         * iteration */
} Elm_Popup_Orient;


#endif

/**
 * @brief Set the alignment of the popup object.
 *
 * Sets the alignment in which the popup will appear in its parent.
 *
 * @param[in] obj The object.
 * @param[in] horizontal The horizontal alignment of the popup.
 * @param[in] vertical The vertical alignment of the popup.
 *
 * @since 1.9
 *
 * @ingroup Elm_Popup_Group
 */
EAPI void elm_popup_align_set(Elm_Popup *obj, double horizontal, double vertical);

/**
 * @brief Get the alignment of the popup object.
 *
 * @param[in] obj The object.
 * @param[out] horizontal The horizontal alignment of the popup.
 * @param[out] vertical The vertical alignment of the popup.
 *
 * @since 1.9
 *
 * @ingroup Elm_Popup_Group
 */
EAPI void elm_popup_align_get(const Elm_Popup *obj, double *horizontal, double *vertical);

/**
 * @brief Sets whether events should be passed to by a click outside.
 *
 * Enabling allow event will remove the Blocked event area and events will pass
 * to the lower layer objects otherwise they are blocked.
 *
 * @note The default value is @c false.
 *
 * @param[in] obj The object.
 * @param[in] allow If @c true, events are passed to lower objects.
 *
 * @ingroup Elm_Popup_Group
 */
EAPI void elm_popup_allow_events_set(Elm_Popup *obj, Eina_Bool allow);

/**
 * @brief Returns value indicating whether allow event is enabled or not.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, events are passed to lower objects.
 *
 * @ingroup Elm_Popup_Group
 */
EAPI Eina_Bool elm_popup_allow_events_get(const Elm_Popup *obj);

/**
 * @brief Control the wrapping type of content text packed in content area of
 * popup object.
 *
 * @param[in] obj The object.
 * @param[in] wrap Wrapping type of type Elm_Wrap_Type.
 *
 * @ingroup Elm_Popup_Group
 */
EAPI void elm_popup_content_text_wrap_type_set(Elm_Popup *obj, Elm_Wrap_Type wrap);

/**
 * @brief Control the wrapping type of content text packed in content area of
 * popup object.
 *
 * @param[in] obj The object.
 *
 * @return Wrapping type of type Elm_Wrap_Type.
 *
 * @ingroup Elm_Popup_Group
 */
EAPI Elm_Wrap_Type elm_popup_content_text_wrap_type_get(const Elm_Popup *obj);

/**
 * @brief Sets the orientation of the popup in the parent region.
 *
 * Sets the position in which popup will appear in its parent. By default,
 * #ELM_POPUP_ORIENT_CENTER is set.
 *
 * @param[in] obj The object.
 * @param[in] orient The orientation of the popup.
 *
 * @ingroup Elm_Popup_Group
 */
EAPI void elm_popup_orient_set(Elm_Popup *obj, Elm_Popup_Orient orient);

/**
 * @brief Returns the orientation of the popup.
 *
 * @param[in] obj The object.
 *
 * @return The orientation of the popup.
 *
 * @ingroup Elm_Popup_Group
 */
EAPI Elm_Popup_Orient elm_popup_orient_get(const Elm_Popup *obj);

/**
 * @brief Sets a timeout to hide popup automatically
 *
 * This function sets a timeout and starts the timer controlling when the popup
 * is hidden. Since calling @ref evas_object_show on a popup restarts the timer
 * controlling when it is hidden, setting this before the  popup is shown will
 * in effect mean starting the timer when the popup is shown. Smart signal
 * "timeout" is called afterwards which can be handled if needed.
 *
 * @note Set a value <= 0.0 to disable a running timer.
 *
 * @note If the value > 0.0 and the popup is previously visible, the timer will
 * be started with this value, canceling any running timer.
 *
 * @param[in] obj The object.
 * @param[in] timeout The timeout in seconds.
 *
 * @ingroup Elm_Popup_Group
 */
EAPI void elm_popup_timeout_set(Elm_Popup *obj, double timeout);

/**
 * @brief Returns the timeout value set to the popup (in seconds).
 *
 * @param[in] obj The object.
 *
 * @return The timeout in seconds.
 *
 * @ingroup Elm_Popup_Group
 */
EAPI double elm_popup_timeout_get(const Elm_Popup *obj);

/**
 * @brief Enable or disable scroller in popup content area
 *
 * Normally content area does not contain scroller.
 *
 * @param[in] obj The object.
 * @param[in] scroll @c true if it is to be scrollable, @c false otherwise.
 *
 * @since 1.15.1
 *
 * @ingroup Elm_Popup_Group
 */
EAPI void elm_popup_scrollable_set(Elm_Popup *obj, Eina_Bool scroll);

/**
 * @brief Get the scrollable state of popup content area
 *
 * Normally content area does not contain scroller.
 *
 * @param[in] obj The object.
 *
 * @return @c true if it is to be scrollable, @c false otherwise.
 *
 * @since 1.15.1
 *
 * @ingroup Elm_Popup_Group
 */
EAPI Eina_Bool elm_popup_scrollable_get(const Elm_Popup *obj);

/**
 * @brief Add a new item to a Popup object
 *
 * Both an item list and a content could not be set at the same time! once you
 * add an item, the previous content will be removed.
 *
 * @warning When the first item is appended to popup object, any previous
 * content of the content area is deleted. At a time, only one of content,
 * content-text and item(s) can be there in a popup content area.
 *
 * @param[in] obj The object.
 * @param[in] label The Label of the new item.
 * @param[in] icon Icon to be set on new item.
 * @param[in] func Convenience function called when item selected.
 * @param[in] data Data passed to @c func above.
 *
 * @return A handle to the item added or @c null on errors.
 *
 * @ingroup Elm_Popup_Group
 */
EAPI Elm_Widget_Item *elm_popup_item_append(Elm_Popup *obj, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

/** Dismiss a Popup object.
 *
 * @since 1.17
 *
 * @ingroup Elm_Popup_Group
 */
EAPI void elm_popup_dismiss(Elm_Popup *obj);

#endif
