#define ELM_OBJ_CHECK_CLASS elm_obj_check_class_get()

const Eo_Class *elm_obj_check_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_CHECK_BASE_ID;

enum
{
      ELM_OBJ_CHECK_SUB_ID_STATE_SET,
      ELM_OBJ_CHECK_SUB_ID_STATE_GET,
      ELM_OBJ_CHECK_SUB_ID_STATE_POINTER_SET,
      ELM_OBJ_CHECK_SUB_ID_LAST
};

#define ELM_OBJ_CHECK_ID(sub_id) (ELM_OBJ_CHECK_BASE_ID + sub_id)


/**
 * @def elm_obj_check_state_set
 * @since 1.8
 *
 * @brief Set the on/off state of the check object
 *
 * @param[in] state
 *
 * @see elm_check_state_set
 *
 * @ingroup Check
 */
#define elm_obj_check_state_set(state) ELM_OBJ_CHECK_ID(ELM_OBJ_CHECK_SUB_ID_STATE_SET), EO_TYPECHECK(Eina_Bool, state)

/**
 * @def elm_obj_check_state_get
 * @since 1.8
 *
 * @brief Get the state of the check object
 *
 * @param[out] ret
 *
 * @see elm_check_state_get
 *
 * @ingroup Check
 */
#define elm_obj_check_state_get(ret) ELM_OBJ_CHECK_ID(ELM_OBJ_CHECK_SUB_ID_STATE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_check_state_pointer_set
 * @since 1.8
 *
 * @brief Set a convenience pointer to a boolean to change
 *
 * @param[in] statep
 *
 * @see elm_check_state_pointer_set
 *
 * @ingroup Check
 */
#define elm_obj_check_state_pointer_set(statep) ELM_OBJ_CHECK_ID(ELM_OBJ_CHECK_SUB_ID_STATE_POINTER_SET), EO_TYPECHECK(Eina_Bool *, statep)
