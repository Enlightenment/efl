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
   ELM_OBJ_CTXPOPUP_SUB_ID_AUTO_HIDE_DISABLED_SET,
   ELM_OBJ_CTXPOPUP_SUB_ID_AUTO_HIDE_DISABLED_GET,
   ELM_OBJ_CTXPOPUP_SUB_ID_LAST
};

#define ELM_OBJ_CTXPOPUP_ID(sub_id) (ELM_OBJ_CTXPOPUP_BASE_ID + sub_id)


/**
 * @def elm_obj_ctxpopup_hover_parent_set
 * @since 1.8
 *
 * @brief Set the Ctxpopup's parent
 *
 * @param[in] parent
 *
 * @see elm_ctxpopup_hover_parent_set
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_hover_parent_set(parent) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_HOVER_PARENT_SET), EO_TYPECHECK(Evas_Object *, parent)

/**
 * @def elm_obj_ctxpopup_hover_parent_get
 * @since 1.8
 *
 * @brief Get the Ctxpopup's parent
 *
 * @param[out] ret
 *
 * @see elm_ctxpopup_hover_parent_get
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_hover_parent_get(ret) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_HOVER_PARENT_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_ctxpopup_clear
 * @since 1.8
 *
 * @brief Clear all items in the given ctxpopup object.
 *
 *
 * @see elm_ctxpopup_clear
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_clear() ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_CLEAR)

/**
 * @def elm_obj_ctxpopup_horizontal_set
 * @since 1.8
 *
 * @brief Change the ctxpopup's orientation to horizontal or vertical.
 *
 * @param[in] horizontal
 *
 * @see elm_ctxpopup_horizontal_set
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_horizontal_set(horizontal) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_ctxpopup_horizontal_get
 * @since 1.8
 *
 * @brief Get the value of current ctxpopup object's orientation.
 *
 * @param[out] ret
 *
 * @see elm_ctxpopup_horizontal_get
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_horizontal_get(ret) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_ctxpopup_item_append
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
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_item_append(label, icon, func, data, ret) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Object *, icon), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_ctxpopup_direction_priority_set
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
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_direction_priority_set(first, second, third, fourth) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_DIRECTION_PRIORITY_SET), EO_TYPECHECK(Elm_Ctxpopup_Direction, first), EO_TYPECHECK(Elm_Ctxpopup_Direction, second), EO_TYPECHECK(Elm_Ctxpopup_Direction, third), EO_TYPECHECK(Elm_Ctxpopup_Direction, fourth)

/**
 * @def elm_obj_ctxpopup_direction_priority_get
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
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_direction_priority_get(first, second, third, fourth) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_DIRECTION_PRIORITY_GET), EO_TYPECHECK(Elm_Ctxpopup_Direction *, first), EO_TYPECHECK(Elm_Ctxpopup_Direction *, second), EO_TYPECHECK(Elm_Ctxpopup_Direction *, third), EO_TYPECHECK(Elm_Ctxpopup_Direction *, fourth)

/**
 * @def elm_obj_ctxpopup_direction_get
 * @since 1.8
 *
 * @brief Get the current direction of a ctxpopup.
 *
 * @param[out] ret
 *
 * @see elm_ctxpopup_direction_get
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_direction_get(ret) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_DIRECTION_GET), EO_TYPECHECK(Elm_Ctxpopup_Direction *, ret)

/**
 * @def elm_obj_ctxpopup_dismiss
 * @since 1.8
 *
 * @brief Dismiss a ctxpopup object
 *
 *
 * @see elm_ctxpopup_dismiss
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_dismiss() ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_DISMISS)

/**
 * @def elm_obj_ctxpopup_auto_hide_disabled_set
 * @since 1.9
 *
 * @brief Set whether ctxpopup hide automatically or not by ctxpopup policy
 *
 * @param[in] disabled
 *
 * @see elm_ctxpopup_auto_hide_disabled_get
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_auto_hide_disabled_set(disabled) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_AUTO_HIDE_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_ctxpopup_auto_hide_disabled_get
 * @since 1.9
 *
 * @brief Get whether ctxpopup hide automatically or not by ctxpopup policy
 *
 * @param[out] ret
 *
 * @see elm_ctxpopup_auto_hide_disabled_set
 *
 * @ingroup Ctxpopup
 */
#define elm_obj_ctxpopup_auto_hide_disabled_get(ret) ELM_OBJ_CTXPOPUP_ID(ELM_OBJ_CTXPOPUP_SUB_ID_AUTO_HIDE_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)