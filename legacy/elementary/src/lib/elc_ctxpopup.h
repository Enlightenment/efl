/**
 * @defgroup Ctxpopup Ctxpopup
 * @ingroup Elementary
 *
 * @image html ctxpopup_inheritance_tree.png
 * @image latex ctxpopup_inheritance_tree.eps
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
 * This widget inherits from the Layout one, so that all the
 * functions acting on it also work for context popup objects
 * (@since 1.8).
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "dismissed" - the ctxpopup was dismissed
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

#define ELM_OBJ_CTXPOPUP_CLASS elm_obj_ctxpopup_class_get()

const Eo_Class *elm_obj_ctxpopup_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_CTXPOPUP_BASE_ID;

enum
{
   ELM_OBJ_CTXPOPUP_SUB_ID_HOVER_PARENT_SET,
   ELM_OBJ_CTXPOPUP_SUB_ID_HOVER_PARENT_GET,
   ELM_OBJ_CTXPOPUP_SUB_ID_CLEAR,
   ELM_OBJ_CTXPOPUP_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_CTXPOPUP_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_CTXPOPUP_SUB_ID_ITEM_APPEND,
   ELM_OBJ_CTXPOPUP_SUB_ID_DIRECTION_PRIORITY_SET,
   ELM_OBJ_CTXPOPUP_SUB_ID_DIRECTION_PRIORITY_GET,
   ELM_OBJ_CTXPOPUP_SUB_ID_DIRECTION_GET,
   ELM_OBJ_CTXPOPUP_SUB_ID_DISMISS,
   ELM_OBJ_CTXPOPUP_SUB_ID_LAST
};

#define ELM_OBJ_CTXPOPUP_ID(sub_id) (ELM_OBJ_CTXPOPUP_BASE_ID + sub_id)


/**
 * @def ctxpopup_hover_parent_set
 * @since 1.8
 *
 * @brief Set the Ctxpopup's parent
 *
 * @param[in] parent
 *
 * @see elm_ctxpopup_hover_parent_set
 */
#define elm_obj_ctxpopup_hover_parent_set(parent) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_HOVER_PARENT_SET), EO_TYPECHECK(Evas_Object *, parent)

/**
 * @def ctxpopup_hover_parent_get
 * @since 1.8
 *
 * @brief Get the Ctxpopup's parent
 *
 * @param[out] ret
 *
 * @see elm_ctxpopup_hover_parent_get
 */
#define elm_obj_ctxpopup_hover_parent_get(ret) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_HOVER_PARENT_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def ctxpopup_clear
 * @since 1.8
 *
 * @brief Clear all items in the given ctxpopup object.
 *
 *
 * @see elm_ctxpopup_clear
 */
#define elm_obj_ctxpopup_clear() ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_CLEAR)

/**
 * @def ctxpopup_horizontal_set
 * @since 1.8
 *
 * @brief Change the ctxpopup's orientation to horizontal or vertical.
 *
 * @param[in] horizontal
 *
 * @see elm_ctxpopup_horizontal_set
 */
#define elm_obj_ctxpopup_horizontal_set(horizontal) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def ctxpopup_horizontal_get
 * @since 1.8
 *
 * @brief Get the value of current ctxpopup object's orientation.
 *
 * @param[out] ret
 *
 * @see elm_ctxpopup_horizontal_get
 */
#define elm_obj_ctxpopup_horizontal_get(ret) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def ctxpopup_item_append
 * @since 1.8
 *
 * @brief Add a new item to a ctxpopup object.
 *
 * @param[in] label
 * @param[in] icon
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_ctxpopup_item_append
 */
#define elm_obj_ctxpopup_item_append(label, icon, func, data, ret) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Object *, icon), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def ctxpopup_direction_priority_set
 * @since 1.8
 *
 * @brief Set the direction priority of a ctxpopup.
 *
 * @param[in] first
 * @param[in] second
 * @param[in] third
 * @param[in] fourth
 *
 * @see elm_ctxpopup_direction_priority_set
 */
#define elm_obj_ctxpopup_direction_priority_set(first, second, third, fourth) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_DIRECTION_PRIORITY_SET), EO_TYPECHECK(Elm_Ctxpopup_Direction, first), EO_TYPECHECK(Elm_Ctxpopup_Direction, second), EO_TYPECHECK(Elm_Ctxpopup_Direction, third), EO_TYPECHECK(Elm_Ctxpopup_Direction, fourth)

/**
 * @def ctxpopup_direction_priority_get
 * @since 1.8
 *
 * @brief Get the direction priority of a ctxpopup.
 *
 * @param[out] first
 * @param[out] second
 * @param[out] third
 * @param[out] fourth
 *
 * @see elm_ctxpopup_direction_priority_get
 */
#define elm_obj_ctxpopup_direction_priority_get(first, second, third, fourth) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_DIRECTION_PRIORITY_GET), EO_TYPECHECK(Elm_Ctxpopup_Direction *, first), EO_TYPECHECK(Elm_Ctxpopup_Direction *, second), EO_TYPECHECK(Elm_Ctxpopup_Direction *, third), EO_TYPECHECK(Elm_Ctxpopup_Direction *, fourth)

/**
 * @def ctxpopup_direction_get
 * @since 1.8
 *
 * @brief Get the current direction of a ctxpopup.
 *
 * @param[out] ret
 *
 * @see elm_ctxpopup_direction_get
 */
#define elm_obj_ctxpopup_direction_get(ret) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_DIRECTION_GET), EO_TYPECHECK(Elm_Ctxpopup_Direction *, ret)

/**
 * @def ctxpopup_dismiss
 * @since 1.8
 *
 * @brief Dismiss a ctxpopup object
 *
 *
 * @see elm_ctxpopup_dismiss
 */
#define elm_obj_ctxpopup_dismiss() ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_DISMISS)
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
