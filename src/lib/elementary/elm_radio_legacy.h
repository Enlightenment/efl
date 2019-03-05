typedef Eo Elm_Radio;

/**
 * @brief Add a new radio to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Radio
 */
EAPI Evas_Object                 *elm_radio_add(Evas_Object *parent);

/**
 * @brief Set the value of the radio group.
 *
 * This sets the value of the radio group and will also set the value if
 * pointed to, to the value supplied, but will not call any callbacks.
 *
 * @param[in] value The value to use for the group
 *
 * @ingroup Elm_Radio
 */
EAPI void                       elm_radio_value_set(Evas_Object *obj, int value);

/**
 * @brief Get the value of the radio group
 *
 * @return The value to use for the group
 *
 * @ingroup Elm_Radio
 */
EAPI int                        elm_radio_value_get(const Evas_Object *obj);

#include "efl_ui_radio_eo.legacy.h"
