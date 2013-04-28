#define ELM_OBJ_RADIO_CLASS elm_obj_radio_class_get()

const Eo_Class *elm_obj_radio_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_RADIO_BASE_ID;

enum
{
   ELM_OBJ_RADIO_SUB_ID_GROUP_ADD,
   ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_SET,
   ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_GET,
   ELM_OBJ_RADIO_SUB_ID_VALUE_SET,
   ELM_OBJ_RADIO_SUB_ID_VALUE_GET,
   ELM_OBJ_RADIO_SUB_ID_VALUE_POINTER_SET,
   ELM_OBJ_RADIO_SUB_ID_SELECTED_OBJECT_GET,
   ELM_OBJ_RADIO_SUB_ID_LAST
};

#define ELM_OBJ_RADIO_ID(sub_id) (ELM_OBJ_RADIO_BASE_ID + sub_id)


/**
 * @def elm_obj_radio_group_add
 * @since 1.8
 *
 * @brief Add this radio to a group of other radio objects
 *
 * @param[in] group
 *
 * @see elm_radio_group_add
 *
 * @ingroup Radio
 */
#define elm_obj_radio_group_add(group) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_GROUP_ADD), EO_TYPECHECK(Evas_Object *, group)

/**
 * @def elm_obj_radio_state_value_set
 * @since 1.8
 *
 * @brief Set the integer value that this radio object represents
 *
 * @param[in] value
 *
 * @see elm_radio_state_value_set
 *
 * @ingroup Radio
 */
#define elm_obj_radio_state_value_set(value) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_SET), EO_TYPECHECK(int, value)

/**
 * @def elm_obj_radio_state_value_get
 * @since 1.8
 *
 * @brief Get the integer value that this radio object represents
 *
 * @param[out] ret
 *
 * @see elm_radio_state_value_get
 *
 * @ingroup Radio
 */
#define elm_obj_radio_state_value_get(ret) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_radio_value_set
 * @since 1.8
 *
 * @brief Set the value of the radio group.
 *
 * @param[in] value
 *
 * @see elm_radio_value_set
 *
 * @ingroup Radio
 */
#define elm_obj_radio_value_set(value) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_VALUE_SET), EO_TYPECHECK(int, value)

/**
 * @def elm_obj_radio_value_get
 * @since 1.8
 *
 * @brief Get the value of the radio group
 *
 * @param[out] ret
 *
 * @see elm_radio_value_get
 *
 * @ingroup Radio
 */
#define elm_obj_radio_value_get(ret) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_VALUE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_radio_value_pointer_set
 * @since 1.8
 *
 * @brief Set a convenience pointer to a integer to change when radio group
 *
 * @param[in] valuep
 *
 * @see elm_radio_value_pointer_set
 *
 * @ingroup Radio
 */
#define elm_obj_radio_value_pointer_set(valuep) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_VALUE_POINTER_SET), EO_TYPECHECK(int *, valuep)

/**
 * @def elm_obj_radio_selected_object_get
 * @since 1.8
 *
 * @brief Get the selected radio object.
 *
 * @param[out] ret
 *
 * @see elm_radio_selected_object_get
 *
 * @ingroup Radio
 */
#define elm_obj_radio_selected_object_get(ret) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_SELECTED_OBJECT_GET), EO_TYPECHECK(Evas_Object **, ret)
