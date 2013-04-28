#define ELM_OBJ_BUTTON_CLASS elm_obj_button_class_get()

const Eo_Class *elm_obj_button_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_BUTTON_BASE_ID;

enum
{
   ELM_OBJ_BUTTON_SUB_ID_ADMITS_AUTOREPEAT_GET,
   ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_SET,
   ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GET,
   ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_INITIAL_TIMEOUT_SET,
   ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_INITIAL_TIMEOUT_GET,
   ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GAP_TIMEOUT_SET,
   ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GAP_TIMEOUT_GET,
   ELM_OBJ_BUTTON_SUB_ID_LAST
};

#define ELM_OBJ_BUTTON_ID(sub_id) (ELM_OBJ_BUTTON_BASE_ID + sub_id)


/**
 * @def elm_obj_button_admits_autorepeat_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see elm_button_admits_autorepeat_get
 *
 * @ingroup Button
 */
#define elm_obj_button_admits_autorepeat_get(ret) ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_ADMITS_AUTOREPEAT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_button_autorepeat_set
 * @since 1.8
 *
 * Turn on/off the autorepeat event generated when the button is kept pressed
 *
 * @param[in] on
 *
 * @see elm_button_autorepeat_set
 *
 * @ingroup Button
 */
#define elm_obj_button_autorepeat_set(on) ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_SET), EO_TYPECHECK(Eina_Bool, on)

/**
 * @def elm_obj_button_autorepeat_get
 * @since 1.8
 *
 * Get whether the autorepeat feature is enabled
 *
 * @param[out] ret
 *
 * @see elm_button_autorepeat_get
 *
 * @ingroup Button
 */
#define elm_obj_button_autorepeat_get(ret) ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_button_autorepeat_initial_timeout_set
 * @since 1.8
 *
 * Set the initial timeout before the autorepeat event is generated
 *
 * @param[in] t
 *
 * @see elm_button_autorepeat_initial_timeout_set
 *
 * @ingroup Button
 */
#define elm_obj_button_autorepeat_initial_timeout_set(t) ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_INITIAL_TIMEOUT_SET), EO_TYPECHECK(double, t)

/**
 * @def elm_obj_button_autorepeat_initial_timeout_get
 * @since 1.8
 *
 * Get the initial timeout before the autorepeat event is generated
 *
 * @param[out] ret
 *
 * @see elm_button_autorepeat_initial_timeout_get
 *
 * @ingroup Button
 */
#define elm_obj_button_autorepeat_initial_timeout_get(ret) ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_INITIAL_TIMEOUT_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_button_autorepeat_gap_timeout_set
 * @since 1.8
 *
 * Set the interval between each generated autorepeat event
 *
 * @param[in] t
 *
 * @see elm_button_autorepeat_gap_timeout_set
 *
 * @ingroup Button
 */
#define elm_obj_button_autorepeat_gap_timeout_set(t) ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GAP_TIMEOUT_SET), EO_TYPECHECK(double, t)

/**
 * @def elm_obj_button_autorepeat_gap_timeout_get
 * @since 1.8
 *
 * Get the interval between each generated autorepeat event
 *
 * @param[out] ret
 *
 * @see elm_button_autorepeat_gap_timeout_get
 *
 * @ingroup Button
 */
#define elm_obj_button_autorepeat_gap_timeout_get(ret) ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GAP_TIMEOUT_GET), EO_TYPECHECK(double *, ret)
