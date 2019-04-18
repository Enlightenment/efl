#ifndef _EFL_UI_BG_LEGACY_EO_H_
#define _EFL_UI_BG_LEGACY_EO_H_

#ifndef _EFL_UI_BG_LEGACY_EO_CLASS_TYPE
#define _EFL_UI_BG_LEGACY_EO_CLASS_TYPE

typedef Eo Efl_Ui_Bg_Legacy;

#endif

#ifndef _EFL_UI_BG_LEGACY_EO_TYPES
#define _EFL_UI_BG_LEGACY_EO_TYPES


#endif
#ifdef EFL_BETA_API_SUPPORT
/**
 * @brief The bg (background) widget is used for setting (solid) background
 * decorations
 *
 * for a window (unless it has transparency enabled) or for any container
 * object. It works just like an image, but has some properties useful for
 * backgrounds, such as setting it to tiled, centered, scaled or stretched.
 *
 * @ingroup Efl_Ui_Bg_Legacy
 */
#define EFL_UI_BG_LEGACY_CLASS efl_ui_bg_legacy_class_get()

EWAPI const Efl_Class *efl_ui_bg_legacy_class_get(void);
#endif /* EFL_BETA_API_SUPPORT */

#endif
