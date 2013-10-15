#define ELM_OBJ_PROGRESSBAR_CLASS elm_obj_progressbar_class_get()

const Eo_Class *elm_obj_progressbar_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_PROGRESSBAR_BASE_ID;

typedef char *(*progressbar_func_type)(double);
typedef void (*progressbar_freefunc_type)(char *);

enum
{
   ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE_SET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE_GET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE,
   ELM_OBJ_PROGRESSBAR_SUB_ID_VALUE_SET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_VALUE_GET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_SPAN_SIZE_SET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_SPAN_SIZE_GET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_SET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_GET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_FUNCTION_SET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_INVERTED_SET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_INVERTED_GET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_PART_VALUE_SET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_PART_VALUE_GET,
   ELM_OBJ_PROGRESSBAR_SUB_ID_LAST
};

#define ELM_OBJ_PROGRESSBAR_ID(sub_id) (ELM_OBJ_PROGRESSBAR_BASE_ID + sub_id)


/**
 * @def elm_obj_progressbar_pulse_set
 * @since 1.8
 *
 * Set whether a given progress bar widget is at "pulsing mode" or
 * not.
 *
 * @param[in] pulse
 *
 * @see elm_progressbar_pulse_set
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_pulse_set(pulse) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE_SET), EO_TYPECHECK(Eina_Bool, pulse)

/**
 * @def elm_obj_progressbar_pulse_get
 * @since 1.8
 *
 * Get whether a given progress bar widget is at "pulsing mode" or
 * not.
 *
 * @param[out] ret
 *
 * @see elm_progressbar_pulse_get
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_pulse_get(ret) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_progressbar_pulse
 * @since 1.8
 *
 * Start/stop a given progress bar "pulsing" animation, if its
 * under that mode
 *
 * @param[in] state
 *
 * @see elm_progressbar_pulse
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_pulse(state) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE), EO_TYPECHECK(Eina_Bool, state)

/**
 * @def elm_obj_progressbar_value_set
 * @since 1.8
 *
 * Set the progress value (in percentage) on a given progress bar
 *
 * @param[in] val
 *
 * @see elm_progressbar_value_set
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_value_set(val) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_VALUE_SET), EO_TYPECHECK(double, val)

/**
 * @def elm_obj_progressbar_value_get
 * @since 1.8
 *
 * Get the progress value (in percentage) on a given progress bar
 *
 * @param[out] ret
 *
 * @see elm_progressbar_value_get
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_value_get(ret) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_VALUE_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_progressbar_span_size_set
 * @since 1.8
 *
 * Set the (exact) length of the bar region of a given progress bar
 *
 * @param[in] size
 *
 * @see elm_progressbar_span_size_set
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_span_size_set(size) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_SPAN_SIZE_SET), EO_TYPECHECK(Evas_Coord, size)

/**
 * @def elm_obj_progressbar_span_size_get
 * @since 1.8
 *
 * Get the length set for the bar region of a given progress bar
 *
 * @param[out] ret
 *
 * @see elm_progressbar_span_size_get
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_span_size_get(ret) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_SPAN_SIZE_GET), EO_TYPECHECK(Evas_Coord *, ret)

/**
 * @def elm_obj_progressbar_unit_format_set
 * @since 1.8
 *
 * Set the format string for a given progress bar widget's units
 *
 * @param[in] units
 *
 * @see elm_progressbar_unit_format_set
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_unit_format_set(units) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_SET), EO_TYPECHECK(const char *, units)

/**
 * @def elm_obj_progressbar_unit_format_get
 * @since 1.8
 *
 * Retrieve the format string set for a given progress bar widget's
 *
 * @param[out] ret
 *
 * @see elm_progressbar_unit_format_get
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_unit_format_get(ret) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_progressbar_unit_format_function_set
 * @since 1.8
 *
 * Set the format function pointer for the units label
 *
 * @param[in] func
 * @param[in] free_func
 *
 * @see elm_progressbar_unit_format_function_set
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_unit_format_function_set(func, free_func) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_FUNCTION_SET), EO_TYPECHECK(progressbar_func_type, func), EO_TYPECHECK(progressbar_freefunc_type, free_func)

/**
 * @def elm_obj_progressbar_horizontal_set
 * @since 1.8
 *
 * Set the orientation of a given progress bar widget
 *
 * @param[in] horizontal
 *
 * @see elm_progressbar_horizontal_set
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_horizontal_set(horizontal) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_progressbar_horizontal_get
 * @since 1.8
 *
 * Retrieve the orientation of a given progress bar widget
 *
 * @param[out] ret
 *
 * @see elm_progressbar_horizontal_get
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_horizontal_get(ret) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_progressbar_inverted_set
 * @since 1.8
 *
 * Invert a given progress bar widget's displaying values order
 *
 * @param[in] inverted
 *
 * @see elm_progressbar_inverted_set
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_inverted_set(inverted) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_INVERTED_SET), EO_TYPECHECK(Eina_Bool, inverted)

/**
 * @def elm_obj_progressbar_inverted_get
 * @since 1.8
 *
 * Get whether a given progress bar widget's displaying values are
 * inverted or not
 *
 * @param[out] ret
 *
 * @see elm_progressbar_inverted_get
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_inverted_get(ret) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_INVERTED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_progressbar_part_value_set
 * @since 1.8
 *
 * Modified to support more than one progress status
 * Set the value of the progress status a particular part
 *
 * @param[in] part
 * @param[in] val
 *
 * @see elm_progressbar_part_value_set
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_part_value_set(part, val) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_PART_VALUE_SET), EO_TYPECHECK(const char*, part), EO_TYPECHECK(double, val)

/**
 * @def elm_obj_progressbar_part_value_get
 * @since 1.8
 *
 * Modified to support more than one progress status
 * Get the value of the progress status of a particular part
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see elm_progressbar_part_value_get
 *
 * @ingroup Progressbar
 */
#define elm_obj_progressbar_part_value_get(part, ret) ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_PART_VALUE_GET), EO_TYPECHECK(const char*, part), EO_TYPECHECK(double *, ret)
