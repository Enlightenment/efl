#define ELM_OBJ_SPINNER_CLASS elm_obj_spinner_class_get()

const Eo_Class *elm_obj_spinner_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_SPINNER_BASE_ID;

enum
{
   ELM_OBJ_SPINNER_SUB_ID_LABEL_FORMAT_SET,
   ELM_OBJ_SPINNER_SUB_ID_LABEL_FORMAT_GET,
   ELM_OBJ_SPINNER_SUB_ID_MIN_MAX_SET,
   ELM_OBJ_SPINNER_SUB_ID_MIN_MAX_GET,
   ELM_OBJ_SPINNER_SUB_ID_STEP_SET,
   ELM_OBJ_SPINNER_SUB_ID_STEP_GET,
   ELM_OBJ_SPINNER_SUB_ID_VALUE_SET,
   ELM_OBJ_SPINNER_SUB_ID_VALUE_GET,
   ELM_OBJ_SPINNER_SUB_ID_WRAP_SET,
   ELM_OBJ_SPINNER_SUB_ID_WRAP_GET,
   ELM_OBJ_SPINNER_SUB_ID_SPECIAL_VALUE_ADD,
   ELM_OBJ_SPINNER_SUB_ID_EDITABLE_SET,
   ELM_OBJ_SPINNER_SUB_ID_EDITABLE_GET,
   ELM_OBJ_SPINNER_SUB_ID_INTERVAL_SET,
   ELM_OBJ_SPINNER_SUB_ID_INTERVAL_GET,
   ELM_OBJ_SPINNER_SUB_ID_BASE_SET,
   ELM_OBJ_SPINNER_SUB_ID_BASE_GET,
   ELM_OBJ_SPINNER_SUB_ID_ROUND_SET,
   ELM_OBJ_SPINNER_SUB_ID_ROUND_GET,
   ELM_OBJ_SPINNER_SUB_ID_LAST
};

#define ELM_OBJ_SPINNER_ID(sub_id) (ELM_OBJ_SPINNER_BASE_ID + sub_id)


/**
 * @def elm_obj_spinner_label_format_set
 * @since 1.8
 *
 * Set the format string of the displayed label.
 *
 * @param[in] fmt
 *
 * @see elm_spinner_label_format_set
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_label_format_set(fmt) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_LABEL_FORMAT_SET), EO_TYPECHECK(const char *, fmt)

/**
 * @def elm_obj_spinner_label_format_get
 * @since 1.8
 *
 * Get the label format of the spinner.
 *
 * @param[out] ret
 *
 * @see elm_spinner_label_format_get
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_label_format_get(ret) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_LABEL_FORMAT_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_spinner_min_max_set
 * @since 1.8
 *
 * Set the minimum and maximum values for the spinner.
 *
 * @param[in] min
 * @param[in] max
 *
 * @see elm_spinner_min_max_set
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_min_max_set(min, max) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_MIN_MAX_SET), EO_TYPECHECK(double, min), EO_TYPECHECK(double, max)

/**
 * @def elm_obj_spinner_min_max_get
 * @since 1.8
 *
 * Get the minimum and maximum values of the spinner.
 *
 * @param[out] min
 * @param[out] max
 *
 * @see elm_spinner_min_max_get
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_min_max_get(min, max) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_MIN_MAX_GET), EO_TYPECHECK(double *, min), EO_TYPECHECK(double *, max)

/**
 * @def elm_obj_spinner_step_set
 * @since 1.8
 *
 * Set the step used to increment or decrement the spinner value.
 *
 * @param[in] step
 *
 * @see elm_spinner_step_set
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_step_set(step) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_STEP_SET), EO_TYPECHECK(double, step)

/**
 * @def elm_obj_spinner_step_get
 * @since 1.8
 *
 * Get the step used to increment or decrement the spinner value.
 *
 * @param[out] ret
 *
 * @see elm_spinner_step_get
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_step_get(ret) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_STEP_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_spinner_value_set
 * @since 1.8
 *
 * Set the value the spinner displays.
 *
 * @param[in] val
 *
 * @see elm_spinner_value_set
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_value_set(val) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_VALUE_SET), EO_TYPECHECK(double, val)

/**
 * @def elm_obj_spinner_value_get
 * @since 1.8
 *
 * Get the value displayed by the spinner.
 *
 * @param[out] ret
 *
 * @see elm_spinner_value_get
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_value_get(ret) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_VALUE_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_spinner_wrap_set
 * @since 1.8
 *
 * Set whether the spinner should wrap when it reaches its
 *
 * @param[in] wrap
 *
 * @see elm_spinner_wrap_set
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_wrap_set(wrap) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_WRAP_SET), EO_TYPECHECK(Eina_Bool, wrap)

/**
 * @def elm_obj_spinner_wrap_get
 * @since 1.8
 *
 * Get whether the spinner should wrap when it reaches its
 *
 * @param[out] ret
 *
 * @see elm_spinner_wrap_get
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_wrap_get(ret) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_WRAP_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_spinner_special_value_add
 * @since 1.8
 *
 * Set a special string to display in the place of the numerical value.
 *
 * @param[in] value
 * @param[in] label
 *
 * @see elm_spinner_special_value_add
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_special_value_add(value, label) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_SPECIAL_VALUE_ADD), EO_TYPECHECK(double, value), EO_TYPECHECK(const char *, label)

/**
 * @def elm_obj_spinner_editable_set
 * @since 1.8
 *
 * Set whether the spinner can be directly edited by the user or not.
 *
 * @param[in] editable
 *
 * @see elm_spinner_editable_set
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_editable_set(editable) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_EDITABLE_SET), EO_TYPECHECK(Eina_Bool, editable)

/**
 * @def elm_obj_spinner_editable_get
 * @since 1.8
 *
 * Get whether the spinner can be directly edited by the user or not.
 *
 * @param[out] ret
 *
 * @see elm_spinner_editable_get
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_editable_get(ret) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_EDITABLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_spinner_interval_set
 * @since 1.8
 *
 * Set the interval on time updates for an user mouse button hold
 *
 * @param[in] interval
 *
 * @see elm_spinner_interval_set
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_interval_set(interval) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_INTERVAL_SET), EO_TYPECHECK(double, interval)

/**
 * @def elm_obj_spinner_interval_get
 * @since 1.8
 *
 * Get the interval on time updates for an user mouse button hold
 *
 * @param[out] ret
 *
 * @see elm_spinner_interval_get
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_interval_get(ret) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_INTERVAL_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_spinner_base_set
 * @since 1.8
 *
 * Set the base for rounding
 *
 * @param[in] base
 *
 * @see elm_spinner_base_set
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_base_set(base) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_BASE_SET), EO_TYPECHECK(double, base)

/**
 * @def elm_obj_spinner_base_get
 * @since 1.8
 *
 * Get the base for rounding
 *
 * @param[out] ret
 *
 * @see elm_spinner_base_get
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_base_get(ret) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_BASE_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_spinner_round_set
 * @since 1.8
 *
 * Set the round value for rounding
 *
 * @param[in] rnd
 *
 * @see elm_spinner_round_set
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_round_set(rnd) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_ROUND_SET), EO_TYPECHECK(int, rnd)

/**
 * @def elm_obj_spinner_round_get
 * @since 1.8
 *
 * Get the round value for rounding
 *
 * @param[out] ret
 *
 * @see elm_spinner_round_get
 *
 * @ingroup Spinner
 */
#define elm_obj_spinner_round_get(ret) ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_ROUND_GET), EO_TYPECHECK(int *, ret)
