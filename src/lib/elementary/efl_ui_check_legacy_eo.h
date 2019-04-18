#ifndef _EFL_UI_CHECK_LEGACY_EO_H_
#define _EFL_UI_CHECK_LEGACY_EO_H_

#ifndef _EFL_UI_CHECK_LEGACY_EO_CLASS_TYPE
#define _EFL_UI_CHECK_LEGACY_EO_CLASS_TYPE

typedef Eo Efl_Ui_Check_Legacy;

#endif

#ifndef _EFL_UI_CHECK_LEGACY_EO_TYPES
#define _EFL_UI_CHECK_LEGACY_EO_TYPES


#endif
#ifdef EFL_BETA_API_SUPPORT
/**
 * @brief Check widget
 *
 * The check widget allows for toggling a value between true and false. Check
 * objects are a lot like radio objects in layout and functionality, except
 * they do not work as a group, but independently, and only toggle the value of
 * a boolean between false and true.
 *
 * @ingroup Efl_Ui_Check_Legacy
 */
#define EFL_UI_CHECK_LEGACY_CLASS efl_ui_check_legacy_class_get()

EWAPI const Efl_Class *efl_ui_check_legacy_class_get(void);
#endif /* EFL_BETA_API_SUPPORT */

#endif
