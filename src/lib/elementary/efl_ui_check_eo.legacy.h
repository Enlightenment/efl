#ifndef _EFL_UI_CHECK_EO_LEGACY_H_
#define _EFL_UI_CHECK_EO_LEGACY_H_

#ifndef _EFL_UI_CHECK_EO_CLASS_TYPE
#define _EFL_UI_CHECK_EO_CLASS_TYPE

typedef Eo Efl_Ui_Check;

#endif

#ifndef _EFL_UI_CHECK_EO_TYPES
#define _EFL_UI_CHECK_EO_TYPES


#endif

/**
 * @brief The on/off state of the check object.
 *
 * @param[in] obj The object.
 * @param[in] value @c true if the check object is selected, @c false otherwise
 *
 * @ingroup Elm_Check_Group
 */
EAPI void elm_check_selected_set(Efl_Ui_Check *obj, Eina_Bool value);

/**
 * @brief The on/off state of the check object.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the check object is selected, @c false otherwise
 *
 * @ingroup Elm_Check_Group
 */
EAPI Eina_Bool elm_check_selected_get(const Efl_Ui_Check *obj);

#endif
