#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/**
 * @file
 * @brief Definition of special values for user configuration.
 */

#define ELM_CONFIG_ICON_THEME_ELEMENTARY "_Elementary_Icon_Theme"

#if defined(ELEMENTARY_BUILD) || defined(ELM_INTERNAL_API_ARGESFSDFEFC)
#define EFL_UI_WIDGET_PROTECTED
#define EFL_CANVAS_OBJECT_PROTECTED
#define EFL_CANVAS_GROUP_PROTECTED
#define EFL_UI_FOCUS_OBJECT_PROTECTED
#endif
