/**
 * @defgroup Ctxpopup Ctxpopup
 * @ingroup Elementary
 *
 * @image html img/widget/ctxpopup/preview-00.png
 * @image latex img/widget/ctxpopup/preview-00.eps
 *
 * @brief Context popup widget.
 *
 * A ctxpopup is a widget that, when shown, pops up a list of items.
 * It automatically chooses an area inside its parent object's view
 * (set via elm_ctxpopup_add() and elm_ctxpopup_hover_parent_set()) to
 * optimally fit into it. In the default theme, it will also point an
 * arrow to it's top left position at the time one shows it. Ctxpopup
 * items have a label and/or an icon. It is intended for a small
 * number of items (hence the use of list, not genlist).
 *
 * @note Ctxpopup is a specialization of @ref Hover.
 *
 * Signals that you can add callbacks for are:
 * "dismissed" - the ctxpopup was dismissed
 *
 * Default content parts of the ctxpopup widget that you can use for are:
 * @li "default" - A content of the ctxpopup
 *
 * Default content parts of the ctxpopup items that you can use for are:
 * @li "icon" - An icon in the title area
 *
 * Default text parts of the ctxpopup items that you can use for are:
 * @li "default" - Title label in the title area
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_signal_callback_add
 * @li @ref elm_object_signal_callback_del
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_disabled_set
 * @li @ref elm_object_item_disabled_get
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 * @li @ref elm_object_item_part_content_set
 * @li @ref elm_object_item_part_content_get
 * @li @ref elm_object_item_signal_emit
 *
 * @ref tutorial_ctxpopup shows the usage of a good deal of the API.
 * @{
 */

typedef enum
{
   ELM_CTXPOPUP_DIRECTION_DOWN, /**< ctxpopup show appear below clicked area */
   ELM_CTXPOPUP_DIRECTION_RIGHT, /**< ctxpopup show appear to the right of the clicked area */
   ELM_CTXPOPUP_DIRECTION_LEFT, /**< ctxpopup show appear to the left of the clicked area */
   ELM_CTXPOPUP_DIRECTION_UP, /**< ctxpopup show appear above the clicked area */
   ELM_CTXPOPUP_DIRECTION_UNKNOWN, /**< ctxpopup does not determine it's direction yet*/
} Elm_Ctxpopup_Direction; /**< Direction in which to show the popup */

/**
 * @brief Add a new Ctxpopup object to the parent.
 *
 * @param parent Parent object
 * @return New object or @c NULL, if it cannot be created
 *
 * @ingroup Ctxpopup
 */
EAPI Evas_Object                 *elm_ctxpopup_add(Evas_Object *parent);

/**
 * @brief Set the Ctxpopup's parent
 *
 * @param obj The ctxpopup object
 * @param parent The parent to use
 *
 * Set the parent object.
 *
 * @note elm_ctxpopup_add() will automatically call this function
 * with its @c parent argument.
 *
 * @see elm_ctxpopup_add()
 * @see elm_hover_parent_set()
 *
 * @ingroup Ctxpopup
 */
EAPI void                         elm_ctxpopup_hover_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * @brief Get the Ctxpopup's parent
 *
 * @param obj The ctxpopup object
 *
 * @see elm_ctxpopup_hover_parent_set() for more information
 *
 * @ingroup Ctxpopup
 */
EAPI Evas_Object                 *elm_ctxpopup_hover_parent_get(const Evas_Object *obj);

/**
 * @brief Clear all items in the given ctxpopup object.
 *
 * @param obj Ctxpopup object
 *
 * @ingroup Ctxpopup
 */
EAPI void                         elm_ctxpopup_clear(Evas_Object *obj);

/**
 * @brief Change the ctxpopup's orientation to horizontal or vertical.
 *
 * @param obj Ctxpopup object
 * @param horizontal @c EINA_TRUE for horizontal mode, @c EINA_FALSE for vertical
 *
 * @ingroup Ctxpopup
 */
EAPI void                         elm_ctxpopup_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief Get the value of current ctxpopup object's orientation.
 *
 * @param obj Ctxpopup object
 * @return @c EINA_TRUE for horizontal mode, @c EINA_FALSE for vertical mode (or errors)
 *
 * @see elm_ctxpopup_horizontal_set()
 *
 * @ingroup Ctxpopup
 */
EAPI Eina_Bool                    elm_ctxpopup_horizontal_get(const Evas_Object *obj);

/**
 * @brief Add a new item to a ctxpopup object.
 *
 * @param obj Ctxpopup object
 * @param icon Icon to be set on new item
 * @param label The Label of the new item
 * @param func Convenience function called when item selected
 * @param data Data passed to @p func
 * @return A handle to the item added or @c NULL, on errors
 *
 * @warning Ctxpopup can't hold both an item list and a content at the same
 * time. When an item is added, any previous content will be removed.
 *
 * @see elm_object_content_set()
 *
 * @ingroup Ctxpopup
 */
EAPI Elm_Object_Item             *elm_ctxpopup_item_append(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Smart_Cb func, const void *data);

/**
 * @brief Set the direction priority of a ctxpopup.
 *
 * @param obj Ctxpopup object
 * @param first 1st priority of direction
 * @param second 2nd priority of direction
 * @param third 3th priority of direction
 * @param fourth 4th priority of direction
 *
 * This functions gives a chance to user to set the priority of ctxpopup
 * showing direction. This doesn't guarantee the ctxpopup will appear in the
 * requested direction.
 *
 * @see Elm_Ctxpopup_Direction
 *
 * @ingroup Ctxpopup
 */
EAPI void                         elm_ctxpopup_direction_priority_set(Evas_Object *obj, Elm_Ctxpopup_Direction first, Elm_Ctxpopup_Direction second, Elm_Ctxpopup_Direction third, Elm_Ctxpopup_Direction fourth);

/**
 * @brief Get the direction priority of a ctxpopup.
 *
 * @param obj Ctxpopup object
 * @param first 1st priority of direction to be returned
 * @param second 2nd priority of direction to be returned
 * @param third 3th priority of direction to be returned
 * @param fourth 4th priority of direction to be returned
 *
 * @see elm_ctxpopup_direction_priority_set() for more information.
 *
 * @ingroup Ctxpopup
 */
EAPI void                         elm_ctxpopup_direction_priority_get(Evas_Object *obj, Elm_Ctxpopup_Direction *first, Elm_Ctxpopup_Direction *second, Elm_Ctxpopup_Direction *third, Elm_Ctxpopup_Direction *fourth);

/**
 * @brief Get the current direction of a ctxpopup.
 *
 * @param obj Ctxpopup object
 * @return current direction of a ctxpopup
 *
 * @warning Once the ctxpopup showed up, the direction would be determined
 *
 * @ingroup Ctxpopup
 */
EAPI Elm_Ctxpopup_Direction       elm_ctxpopup_direction_get(const Evas_Object *obj);

/**
 * @brief Dismiss a ctxpopup object
 *
 * @param obj The ctxpopup object
 * Use this function to simulate clicking outside the ctxpopup to dismiss it.
 * In this way, the ctxpopup will be hidden and the "clicked" signal will be
 * emitted.
 */
EAPI void                         elm_ctxpopup_dismiss(Evas_Object *obj);

/**
 * @}
 */
