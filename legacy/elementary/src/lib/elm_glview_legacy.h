/**
 * Add a new glview to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup GLView
 */
EAPI Evas_Object *elm_glview_add(Evas_Object *parent);

/**
 * Adds a new GLView to the parent, given an OpenGL-ES context version number.
 *
 * @param[in] parent The parent object
 * @param[in] version Requested GL ES version number (default is 2.x, 1.x may also be supported)
 * @return The new object or @c NULL if it cannot be created
 *
 * @since 1.12
 */
EAPI Evas_Object *elm_glview_version_add(Evas_Object *parent, Evas_GL_Context_Version version);

#include "elm_glview.eo.legacy.h"
