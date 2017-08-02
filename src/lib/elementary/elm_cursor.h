/**
 * @defgroup Elm_Cursors Cursors
 * @ingroup Elementary
 */

#ifdef EFL_EO_API_SUPPORT
#include "efl_ui_cursor.eo.h"
#endif

#ifndef EFL_NOLEGACY_API_SUPPORT
#include "efl_ui_cursor.eo.legacy.h"

/**
 * Unset cursor for object
 *
 * Unset cursor for object, and set the cursor to default if the mouse
 * was over this object.
 *
 * @param obj Target object
 * @see elm_object_cursor_set()
 *
 * @ingroup Elm_Cursors
 */
EAPI void elm_object_cursor_unset(Evas_Object *obj);

#endif

/**
 * @}
 */
