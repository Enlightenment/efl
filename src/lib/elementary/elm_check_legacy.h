typedef Eo Elm_Check;

/**
 * @brief Add a new Check object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Check
 */
EAPI Evas_Object *                elm_check_add(Evas_Object *parent);

/**
 * @brief Get the state of the check object
 *
 * @param obj The check object
 *
 * @ingroup Elm_Check
 */
EAPI Eina_Bool                   elm_check_state_get(const Evas_Object *obj);

/**
 * @brief Set the on/off state of the check object
 *
 * This sets the state of the check. If set with
 * @.state_pointer.set, the state of that variable is also
 * changed. Calling this doesn't cause the "changed" signal to
 * be emitted.
 *
 * @param obj The check object
 * @param state The state to use (1 == on, 0 == off)
 *
 * @ingroup Elm_Check
 */
EAPI void                        elm_check_state_set(Evas_Object *obj, Eina_Bool state);

/**
 * @brief Set a convenience pointer to a boolean to change
 *
 * This sets a pointer to a boolean, that, in addition to the check
 * objects state will also be modified directly. To stop setting the
 * object pointed to simply use null as the "statep" parameter.
 * If "statep" is not null, then when this is called, the check
 * objects state will also be modified to reflect the value of the
 * boolean "statep" points to, just like calling @.state.set.
 *
 * @param obj The check object
 * @param statep pointer to the boolean to modify
 *
 * @ingroup Elm_Check
 */
EAPI void                        elm_check_state_pointer_set(Evas_Object *obj, Eina_Bool *statep);

#ifdef EFL_EO_API_SUPPORT
# include "efl_ui_check.eo.legacy.h"
#endif
