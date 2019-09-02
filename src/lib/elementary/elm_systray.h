#include "elm_systray_common.h"

#ifdef EFL_BETA_API_SUPPORT
# ifndef EFL_NOLEGACY_API_SUPPORT
#  include "elm_systray_eo.legacy.h"
# endif

/**
 * Add a new systray element
 *
 * @param parent The parent object
 * @return A new systray object which will be displayed on a window manager that provides appindicator support.
 *
 * @ingroup Elm_Systray
 */
EAPI Elm_Systray                 *elm_systray_add(Evas_Object *win);

#endif
