/**
 * Add a new spinner widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new spinner widget handle or @c NULL, on errors.
 *
 * This function inserts a new spinner widget on the canvas.
 *
 * @ingroup Spinner
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
 * @ingroup Spinner
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
 * @ingroup Spinner
 * @since 1.8
 */
EAPI const char *elm_spinner_special_value_get(Evas_Object *obj, double value);

#include "elm_spinner.eo.legacy.h"