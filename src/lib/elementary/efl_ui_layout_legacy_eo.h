#ifndef _EFL_UI_LAYOUT_LEGACY_EO_H_
#define _EFL_UI_LAYOUT_LEGACY_EO_H_

#ifndef _EFL_UI_LAYOUT_LEGACY_EO_CLASS_TYPE
#define _EFL_UI_LAYOUT_LEGACY_EO_CLASS_TYPE

typedef Eo Efl_Ui_Layout_Legacy;

#endif

#ifndef _EFL_UI_LAYOUT_LEGACY_EO_TYPES
#define _EFL_UI_LAYOUT_LEGACY_EO_TYPES


#endif
#ifdef EFL_BETA_API_SUPPORT
/** Elementary layout class
 *
 * @ingroup Efl_Ui_Layout_Legacy
 */
#define EFL_UI_LAYOUT_LEGACY_CLASS efl_ui_layout_legacy_class_get()

EWAPI const Efl_Class *efl_ui_layout_legacy_class_get(void);
#endif /* EFL_BETA_API_SUPPORT */

#endif
