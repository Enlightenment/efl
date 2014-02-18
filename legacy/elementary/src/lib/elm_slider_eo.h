#define ELM_OBJ_SLIDER_CLASS elm_obj_slider_class_get()

const Eo_Class *elm_obj_slider_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_SLIDER_BASE_ID;

typedef char *(*slider_func_type)(double);
typedef void (*slider_freefunc_type)(char *);

enum
{
   ELM_OBJ_SLIDER_SUB_ID_SPAN_SIZE_SET,
   ELM_OBJ_SLIDER_SUB_ID_SPAN_SIZE_GET,
   ELM_OBJ_SLIDER_SUB_ID_UNIT_FORMAT_SET,
   ELM_OBJ_SLIDER_SUB_ID_UNIT_FORMAT_GET,
   ELM_OBJ_SLIDER_SUB_ID_INDICATOR_FORMAT_SET,
   ELM_OBJ_SLIDER_SUB_ID_INDICATOR_FORMAT_GET,
   ELM_OBJ_SLIDER_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_SLIDER_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_SLIDER_SUB_ID_MIN_MAX_SET,
   ELM_OBJ_SLIDER_SUB_ID_MIN_MAX_GET,
   ELM_OBJ_SLIDER_SUB_ID_VALUE_SET,
   ELM_OBJ_SLIDER_SUB_ID_VALUE_GET,
   ELM_OBJ_SLIDER_SUB_ID_INVERTED_SET,
   ELM_OBJ_SLIDER_SUB_ID_INVERTED_GET,
   ELM_OBJ_SLIDER_SUB_ID_INDICATOR_FORMAT_FUNCTION_SET,
   ELM_OBJ_SLIDER_SUB_ID_UNITS_FORMAT_FUNCTION_SET,
   ELM_OBJ_SLIDER_SUB_ID_INDICATOR_SHOW_SET,
   ELM_OBJ_SLIDER_SUB_ID_INDICATOR_SHOW_GET,
   ELM_OBJ_SLIDER_SUB_ID_STEP_SET,
   ELM_OBJ_SLIDER_SUB_ID_STEP_GET,
   ELM_OBJ_SLIDER_SUB_ID_INDICATOR_SHOW_ON_FOCUS_SET,
   ELM_OBJ_SLIDER_SUB_ID_INDICATOR_SHOW_ON_FOCUS_GET,
   ELM_OBJ_SLIDER_SUB_ID_LAST
};

#define ELM_OBJ_SLIDER_ID(sub_id) (ELM_OBJ_SLIDER_BASE_ID + sub_id)


/**
 * @def elm_obj_slider_span_size_set
 * @since 1.8
 *
 * Set the (exact) length of the bar region of a given slider widget.
 *
 * @param[in] size
 *
 * @see elm_slider_span_size_set
 *
 * @ingroup Slider
 */
#define elm_obj_slider_span_size_set(size) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_SPAN_SIZE_SET), EO_TYPECHECK(Evas_Coord, size)

/**
 * @def elm_obj_slider_span_size_get
 * @since 1.8
 *
 * Get the length set for the bar region of a given slider widget
 *
 * @param[out] ret
 *
 * @see elm_slider_span_size_get
 *
 * @ingroup Slider
 */
#define elm_obj_slider_span_size_get(ret) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_SPAN_SIZE_GET), EO_TYPECHECK(Evas_Coord *, ret)

/**
 * @def elm_obj_slider_unit_format_set
 * @since 1.8
 *
 * Set the format string for the unit label.
 *
 * @param[in] units
 *
 * @see elm_slider_unit_format_set
 *
 * @ingroup Slider
 */
#define elm_obj_slider_unit_format_set(units) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_UNIT_FORMAT_SET), EO_TYPECHECK(const char *, units)

/**
 * @def elm_obj_slider_unit_format_get
 * @since 1.8
 *
 * Get the unit label format of the slider.
 *
 * @param[out] ret
 *
 * @see elm_slider_unit_format_get
 *
 * @ingroup Slider
 */
#define elm_obj_slider_unit_format_get(ret) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_UNIT_FORMAT_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_slider_indicator_format_set
 * @since 1.8
 *
 * Set the format string for the indicator label.
 *
 * @param[in] indicator
 *
 * @see elm_slider_indicator_format_set
 *
 * @ingroup Slider
 */
#define elm_obj_slider_indicator_format_set(indicator) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_INDICATOR_FORMAT_SET), EO_TYPECHECK(const char *, indicator)

/**
 * @def elm_obj_slider_indicator_format_get
 * @since 1.8
 *
 * Get the indicator label format of the slider.
 *
 * @param[out] ret
 *
 * @see elm_slider_indicator_format_get
 *
 * @ingroup Slider
 */
#define elm_obj_slider_indicator_format_get(ret) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_INDICATOR_FORMAT_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_slider_horizontal_set
 * @since 1.8
 *
 * Set the orientation of a given slider widget.
 *
 * @param[in] horizontal
 *
 * @see elm_slider_horizontal_set
 *
 * @ingroup Slider
 */
#define elm_obj_slider_horizontal_set(horizontal) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_slider_horizontal_get
 * @since 1.8
 *
 * Retrieve the orientation of a given slider widget
 *
 * @param[out] ret
 *
 * @see elm_slider_horizontal_get
 *
 * @ingroup Slider
 */
#define elm_obj_slider_horizontal_get(ret) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_slider_min_max_set
 * @since 1.8
 *
 * Set the minimum and maximum values for the slider.
 *
 * @param[in] min
 * @param[in] max
 *
 * @see elm_slider_min_max_set
 *
 * @ingroup Slider
 */
#define elm_obj_slider_min_max_set(min, max) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_MIN_MAX_SET), EO_TYPECHECK(double, min), EO_TYPECHECK(double, max)

/**
 * @def elm_obj_slider_min_max_get
 * @since 1.8
 *
 * Get the minimum and maximum values of the slider.
 *
 * @param[out] min
 * @param[out] max
 *
 * @see elm_slider_min_max_get
 *
 * @ingroup Slider
 */
#define elm_obj_slider_min_max_get(min, max) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_MIN_MAX_GET), EO_TYPECHECK(double *, min), EO_TYPECHECK(double *, max)

/**
 * @def elm_obj_slider_value_set
 * @since 1.8
 *
 * Set the value the slider displays.
 *
 * @param[in] val
 *
 * @see elm_slider_value_set
 *
 * @ingroup Slider
 */
#define elm_obj_slider_value_set(val) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_VALUE_SET), EO_TYPECHECK(double, val)

/**
 * @def elm_obj_slider_value_get
 * @since 1.8
 *
 * Get the value displayed by the slider.
 *
 * @param[out] ret
 *
 * @see elm_slider_value_get
 *
 * @ingroup Slider
 */
#define elm_obj_slider_value_get(ret) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_VALUE_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_slider_inverted_set
 * @since 1.8
 *
 * Invert a given slider widget's displaying values order
 *
 * @param[in] inverted
 *
 * @see elm_slider_inverted_set
 *
 * @ingroup Slider
 */
#define elm_obj_slider_inverted_set(inverted) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_INVERTED_SET), EO_TYPECHECK(Eina_Bool, inverted)

/**
 * @def elm_obj_slider_inverted_get
 * @since 1.8
 *
 * Get whether a given slider widget's displaying values are
 * inverted or not.
 *
 * @param[out] ret
 *
 * @see elm_slider_inverted_get
 *
 * @ingroup Slider
 */
#define elm_obj_slider_inverted_get(ret) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_INVERTED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_slider_indicator_format_function_set
 * @since 1.8
 *
 * Set the format function pointer for the indicator label
 *
 * @param[in] func
 * @param[in] free_func
 *
 * @see elm_slider_indicator_format_function_set
 *
 * @ingroup Slider
 */
#define elm_obj_slider_indicator_format_function_set(func, free_func) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_INDICATOR_FORMAT_FUNCTION_SET), EO_TYPECHECK(slider_func_type, func), EO_TYPECHECK(slider_freefunc_type, free_func)

/**
 * @def elm_obj_slider_units_format_function_set
 * @since 1.8
 *
 * Set the format function pointer for the units label
 *
 * @param[in] func
 * @param[in] free_func
 *
 * @see elm_slider_units_format_function_set
 *
 * @ingroup Slider
 */
#define elm_obj_slider_units_format_function_set(func, free_func) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_UNITS_FORMAT_FUNCTION_SET), EO_TYPECHECK(slider_func_type, func), EO_TYPECHECK(slider_freefunc_type, free_func)

/**
 * @def elm_obj_slider_indicator_show_set
 * @since 1.9
 *
 * Set whether to enlarge slider indicator (augmented knob) or not.
 *
 * @param[in] show
 *
 * @see elm_slider_indicator_show_set
 *
 * @ingroup Slider
 */
#define elm_obj_slider_indicator_show_set(show) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_INDICATOR_SHOW_SET), EO_TYPECHECK(Eina_Bool, show)

/**
 * @def elm_obj_slider_indicator_show_get
 * @since 1.8
 *
 * Get whether a given slider widget's enlarging indicator or not.
 *
 * @param[out] ret
 *
 * @see elm_slider_indicator_show_get
 *
 * @ingroup Slider
 */
#define elm_obj_slider_indicator_show_get(ret) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_INDICATOR_SHOW_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_slider_step_set
 * @since 1.8
 *
 * Set the step by which slider indicator will move.
 *
 * This value is used when draggable object is moved automatically i.e., in case
 * of key event when up/down/left/right key is pressed or in case when
 * accessibility is set and flick event is used to inc/dec slider values.
 * By default step value is equal to 0.05.
 *
 * @param[in] step
 *
 * @see elm_slider_step_set
 *
 * @ingroup Slider
 */

#define elm_obj_slider_step_set(step) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_STEP_SET), EO_TYPECHECK(double, step)


/**
 * @def elm_obj_slider_step_get
 * @since 1.8
 *
 * Get the step by which slider indicator moves.
 *
 * @param[out] ret
 *
 * @see elm_slider_step_get
 *
 * @ingroup Slider
 */
#define elm_obj_slider_step_get(ret) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_STEP_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_slider_indicator_show_on_focus_set
 * @since 1.9
 *
 * show indicator of slider on focus
 *
 * @param[in] flag
 *
 * @see elm_slider_indicator_show_on_focus_set
 *
 * @ingroup Slider
 */
#define elm_obj_slider_indicator_show_on_focus_set(flag) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_INDICATOR_SHOW_ON_FOCUS_SET), EO_TYPECHECK(Eina_Bool, flag)

/**
 * @def elm_obj_slider_indicator_show_on_focus_get
 * @since 1.9
 *
 * Get whether the indicator of the slider is set or not
 *
 * @param[out] ret
 *
 * @see elm_slider_indicator_show_on_focus_get
 *
 * @ingroup Slider
 */
#define elm_obj_slider_indicator_show_on_focus_get(ret) ELM_OBJ_SLIDER_ID(ELM_OBJ_SLIDER_SUB_ID_INDICATOR_SHOW_ON_FOCUS_GET), EO_TYPECHECK(Eina_Bool*, ret)
