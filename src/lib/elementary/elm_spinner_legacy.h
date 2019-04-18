/**
 * Add a new spinner widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new spinner widget handle or @c NULL, on errors.
 *
 * This function inserts a new spinner widget on the canvas.
 *
 * @ingroup Elm_Spinner
 *
 */
EAPI Evas_Object *elm_spinner_add(Evas_Object *parent);

/**
 * Delete the special string display in the place of the numerical value.
 *
 * @param obj The spinner object.
 * @param value The replaced value.
 *
 * It will remove a previously added special value. After this, the spinner
 * will display the value itself instead of a label.
 *
 * @see elm_spinner_special_value_add() for more details.
 *
 * @ingroup Elm_Spinner
 * @since 1.8
 */
EAPI void elm_spinner_special_value_del(Evas_Object *obj, double value);

/**
 * Get the special string display in the place of the numerical value.
 *
 * @param obj The spinner object.
 * @param value The replaced value.
 * @return The used label.
 *
 * @see elm_spinner_special_value_add() for more details.
 *
 * @ingroup Elm_Spinner
 * @since 1.8
 */
EAPI const char *elm_spinner_special_value_get(Evas_Object *obj, double value);

/**
 * @brief Control the minimum and maximum values for the spinner.
 *
 * Define the allowed range of values to be selected by the user.
 *
 * If actual value is less than @c min, it will be updated to @c min. If it is
 * bigger then @c max, will be updated to @c max. Actual value can be get with
 * @ref elm_obj_spinner_value_get.
 *
 * By default, min is equal to 0, and max is equal to 100.
 *
 * @warning Maximum must be greater than minimum.
 *
 * @param[in] min The minimum value.
 * @param[in] max The maximum value.
 *
 * @ingroup Elm_Spinner
 */
EAPI void elm_spinner_min_max_set(Evas_Object *obj, double min, double max);

/**
 * @brief Control the minimum and maximum values for the spinner.
 *
 * Define the allowed range of values to be selected by the user.
 *
 * If actual value is less than @c min, it will be updated to @c min. If it is
 * bigger then @c max, will be updated to @c max. Actual value can be get with
 * @ref elm_obj_spinner_value_get.
 *
 * By default, min is equal to 0, and max is equal to 100.
 *
 * @warning Maximum must be greater than minimum.
 *
 * @param[out] min The minimum value.
 * @param[out] max The maximum value.
 *
 * @ingroup Elm_Spinner
 */
EAPI void elm_spinner_min_max_get(const Evas_Object *obj, double *min, double *max);

/**
 * @brief Control the step used to increment or decrement the spinner value.
 *
 * This value will be incremented or decremented to the displayed value. It
 * will be incremented while the user keep right or top arrow pressed, and will
 * be decremented while the user keep left or bottom arrow pressed.
 *
 * The interval to increment / decrement can be set with
 * @ref elm_obj_spinner_interval_set.
 *
 * By default step value is equal to 1.
 *
 * @param[in] step The step value.
 *
 * @ingroup Elm_Spinner
 */
EAPI void elm_spinner_step_set(Evas_Object *obj, double step);

/**
 * @brief Control the step used to increment or decrement the spinner value.
 *
 * This value will be incremented or decremented to the displayed value. It
 * will be incremented while the user keep right or top arrow pressed, and will
 * be decremented while the user keep left or bottom arrow pressed.
 *
 * The interval to increment / decrement can be set with
 * @ref elm_obj_spinner_interval_set.
 *
 * By default step value is equal to 1.
 *
 * @return The step value.
 *
 * @ingroup Elm_Spinner
 */
EAPI double elm_spinner_step_get(const Evas_Object *obj);

/**
 * @brief Control the value the spinner displays.
 *
 * Value will be presented on the label following format specified with
 * elm_spinner_format_set().
 *
 * Warning The value must to be between min and max values. This values are set
 * by elm_spinner_min_max_set().
 *
 * @param[in] val The value to be displayed.
 *
 * @ingroup Elm_Spinner
 */
EAPI void elm_spinner_value_set(Evas_Object *obj, double val);

/**
 * @brief Control the value the spinner displays.
 *
 * Value will be presented on the label following format specified with
 * elm_spinner_format_set().
 *
 * Warning The value must to be between min and max values. This values are set
 * by elm_spinner_min_max_set().
 *
 * @return The value to be displayed.
 *
 * @ingroup Elm_Spinner
 */
EAPI double elm_spinner_value_get(const Evas_Object *obj);

#include "elm_spinner_eo.legacy.h"
