#ifndef _ELM_CTXPOPUP_EO_LEGACY_H_
#define _ELM_CTXPOPUP_EO_LEGACY_H_

#ifndef _ELM_CTXPOPUP_EO_CLASS_TYPE
#define _ELM_CTXPOPUP_EO_CLASS_TYPE

typedef Eo Elm_Ctxpopup;

#endif

#ifndef _ELM_CTXPOPUP_EO_TYPES
#define _ELM_CTXPOPUP_EO_TYPES

/** Direction in which to show the popup.
 *
 * @ingroup Elm_Ctxpopup
 */
typedef enum
{
  ELM_CTXPOPUP_DIRECTION_DOWN = 0, /**< Ctxpopup show appear below clicked area.
                                    */
  ELM_CTXPOPUP_DIRECTION_RIGHT, /**< Ctxpopup show appear to the right of the
                                 * clicked area. */
  ELM_CTXPOPUP_DIRECTION_LEFT, /**< Ctxpopup show appear to the left of the
                                * clicked area. */
  ELM_CTXPOPUP_DIRECTION_UP, /**< Ctxpopup show appear above the clicked area.
                              */
  ELM_CTXPOPUP_DIRECTION_UNKNOWN /**< Ctxpopup does not determine it's direction
                                  * yet. */
} Elm_Ctxpopup_Direction;


#endif

/**
 * @brief Get the selected item in the widget.
 *
 * @param[in] obj The object.
 *
 * @return The selected item or @c null.
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI Elm_Widget_Item *elm_ctxpopup_selected_item_get(const Elm_Ctxpopup *obj);

/**
 * @brief Get the first item in the widget.
 *
 * @param[in] obj The object.
 *
 * @return The first item or @c null.
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI Elm_Widget_Item *elm_ctxpopup_first_item_get(const Elm_Ctxpopup *obj);

/**
 * @brief Get the last item in the widget.
 *
 * @param[in] obj The object.
 *
 * @return The last item or @c null.
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI Elm_Widget_Item *elm_ctxpopup_last_item_get(const Elm_Ctxpopup *obj);

/**
 * @brief Returns a list of the widget item.
 *
 * @param[in] obj The object.
 *
 * @return const list to widget items
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI const Eina_List *elm_ctxpopup_items_get(const Elm_Ctxpopup *obj);

/**
 * @brief Change the ctxpopup's orientation to horizontal or vertical.
 *
 * @param[in] obj The object.
 * @param[in] horizontal @c true for horizontal mode, @c false for vertical.
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI void elm_ctxpopup_horizontal_set(Elm_Ctxpopup *obj, Eina_Bool horizontal);

/**
 * @brief Get the value of current ctxpopup object's orientation.
 *
 * See also @ref elm_ctxpopup_horizontal_set.
 *
 * @param[in] obj The object.
 *
 * @return @c true for horizontal mode, @c false for vertical.
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI Eina_Bool elm_ctxpopup_horizontal_get(const Elm_Ctxpopup *obj);

/**
 * @brief Set ctxpopup auto hide mode triggered by ctxpopup policy.
 *
 * Use this function when user wants ctxpopup not to hide automatically. By
 * default, ctxpopup is dismissed whenever mouse clicked its background area,
 * language is changed, and its parent geometry is updated(changed). Not to
 * hide ctxpopup automatically, disable auto hide function by calling this API,
 * then ctxpopup won't be dismissed in those scenarios.
 *
 * Default value of disabled is @c false.
 *
 * See also @ref elm_ctxpopup_auto_hide_disabled_get.
 *
 * @param[in] obj The object.
 * @param[in] disabled Auto hide enable/disable.
 *
 * @since 1.9
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI void elm_ctxpopup_auto_hide_disabled_set(Elm_Ctxpopup *obj, Eina_Bool disabled);

/**
 * @brief Get ctxpopup auto hide mode triggered by ctxpopup policy.
 *
 * See also @ref elm_ctxpopup_auto_hide_disabled_set for more information.
 *
 * @param[in] obj The object.
 *
 * @return Auto hide enable/disable.
 *
 * @since 1.9
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI Eina_Bool elm_ctxpopup_auto_hide_disabled_get(const Elm_Ctxpopup *obj);

/**
 * @brief Set the Ctxpopup's parent
 *
 * Set the parent object.
 *
 * @note @ref elm_ctxpopup_add will automatically call this function with its
 * @c parent argument.
 *
 * See also @ref elm_ctxpopup_add, @ref elm_hover_parent_set.
 *
 * @param[in] obj The object.
 * @param[in] parent The parent to use.
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI void elm_ctxpopup_hover_parent_set(Elm_Ctxpopup *obj, Efl_Canvas_Object *parent);

/**
 * @brief Get the Ctxpopup's parent
 *
 * See also @ref elm_ctxpopup_hover_parent_set for more information
 *
 * @param[in] obj The object.
 *
 * @return The parent to use.
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI Efl_Canvas_Object *elm_ctxpopup_hover_parent_get(const Elm_Ctxpopup *obj);

/**
 * @brief Set the direction priority of a ctxpopup.
 *
 * This functions gives a chance to user to set the priority of ctxpopup
 * showing direction. This doesn't guarantee the ctxpopup will appear in the
 * requested direction.
 *
 * See also @ref Elm_Ctxpopup_Direction.
 *
 * @param[in] obj The object.
 * @param[in] first 1st priority of direction
 * @param[in] second 2nd priority of direction
 * @param[in] third 3th priority of direction
 * @param[in] fourth 4th priority of direction
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI void elm_ctxpopup_direction_priority_set(Elm_Ctxpopup *obj, Elm_Ctxpopup_Direction first, Elm_Ctxpopup_Direction second, Elm_Ctxpopup_Direction third, Elm_Ctxpopup_Direction fourth);

/**
 * @brief Get the direction priority of a ctxpopup.
 *
 * See also @ref elm_ctxpopup_direction_priority_set for more information.
 *
 * @param[in] obj The object.
 * @param[out] first 1st priority of direction
 * @param[out] second 2nd priority of direction
 * @param[out] third 3th priority of direction
 * @param[out] fourth 4th priority of direction
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI void elm_ctxpopup_direction_priority_get(const Elm_Ctxpopup *obj, Elm_Ctxpopup_Direction *first, Elm_Ctxpopup_Direction *second, Elm_Ctxpopup_Direction *third, Elm_Ctxpopup_Direction *fourth);

/**
 * @brief Get the current direction of a ctxpopup.
 *
 * @warning Once the ctxpopup showed up, the direction would be determined
 *
 * @param[in] obj The object.
 *
 * @return Direction
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI Elm_Ctxpopup_Direction elm_ctxpopup_direction_get(const Elm_Ctxpopup *obj);

/**
 * @brief Dismiss a ctxpopup object
 *
 * Use this function to simulate clicking outside of the ctxpopup to dismiss
 * it. In this way, the ctxpopup will be hidden and the "clicked" signal will
 * be emitted.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI void elm_ctxpopup_dismiss(Elm_Ctxpopup *obj);

/** Clear all items in the given ctxpopup object.
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI void elm_ctxpopup_clear(Elm_Ctxpopup *obj);

/**
 * @brief Insert a new item to a ctxpopup object before item @c before.
 *
 * See also elm_object_content_set.
 *
 * @param[in] obj The object.
 * @param[in] before The ctxpopup item to insert before.
 * @param[in] label The Label of the new item
 * @param[in] icon Icon to be set on new item
 * @param[in] func Convenience function called when item selected
 * @param[in] data Data passed to @c func
 *
 * @return A handle to the item added or @c null, on errors.
 *
 * @since 1.21
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI Elm_Widget_Item *elm_ctxpopup_item_insert_before(Elm_Ctxpopup *obj, Elm_Widget_Item *before, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

/**
 * @brief Insert a new item to a ctxpopup object after item @c after.
 *
 * See also elm_object_content_set.
 *
 * @param[in] obj The object.
 * @param[in] after The ctxpopup item to insert after.
 * @param[in] label The Label of the new item
 * @param[in] icon Icon to be set on new item
 * @param[in] func Convenience function called when item selected
 * @param[in] data Data passed to @c func
 *
 * @return A handle to the item added or @c null, on errors.
 *
 * @since 1.21
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI Elm_Widget_Item *elm_ctxpopup_item_insert_after(Elm_Ctxpopup *obj, Elm_Widget_Item *after, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

/**
 * @brief Add a new item to a ctxpopup object.
 *
 * Warning:Ctxpopup can't hold both an item list and a content at the same
 * time. When an item is added, any previous content will be removed.
 *
 * See also @ref elm_object_content_set.
 *
 * @param[in] obj The object.
 * @param[in] label The Label of the new item
 * @param[in] icon Icon to be set on new item
 * @param[in] func Convenience function called when item selected
 * @param[in] data Data passed to @c func
 *
 * @return A handle to the item added or @c null, on errors.
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI Elm_Widget_Item *elm_ctxpopup_item_append(Elm_Ctxpopup *obj, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

/**
 * @brief Prepend a new item to a ctxpopup object.
 *
 * @warning Ctxpopup can't hold both an item list and a content at the same
 * time. When an item is added, any previous content will be removed.
 *
 * See also @ref elm_object_content_set.
 *
 * @param[in] obj The object.
 * @param[in] label The Label of the new item
 * @param[in] icon Icon to be set on new item
 * @param[in] func Convenience function called when item selected
 * @param[in] data Data passed to @c func
 *
 * @return A handle to the item added or @c null, on errors.
 *
 * @since 1.11
 *
 * @ingroup Elm_Ctxpopup_Group
 */
EAPI Elm_Widget_Item *elm_ctxpopup_item_prepend(Elm_Ctxpopup *obj, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

#endif
