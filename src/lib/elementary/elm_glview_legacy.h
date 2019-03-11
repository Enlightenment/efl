/**
 * Add a new glview to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_GLView
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


/**
 * Notifies that there has been changes in the GLView.
 *
 * @param obj The GLView object
 *
 * @ingroup Elm_GLView
 */
EAPI void elm_glview_changed_set(Evas_Object *obj);

/**
 * Gets the size of the GLView.
 *
 * @param obj The GLView object
 * @param w pointer of int width
 * @param h pointer of int height
 *
 * @ingroup Elm_GLView
 */
EAPI void elm_glview_size_get(const Evas_Object *obj, int *w, int *h);

/**
 * Sets the size of the GLView.
 *
 * @param obj The GLView object
 * @param w width of GLView
 * @param h height of GLView
 *
 * @ingroup Elm_GLView
 */
EAPI void elm_glview_size_set(Evas_Object *obj, int w, int h);

/**
 * Set the init function that runs once in the main loop.
 * @param obj The GLView object
 * @param func The callback function
 *
 * @ingroup GLView
 */
EAPI void elm_glview_init_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);

/**
 * Set the delete function that runs in the main loop.
 *
 * @param obj The GLView object
 * @param func The callback function
 *
 * @ingroup Elm_GLView
 */
EAPI void elm_glview_del_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);

/**
 * Set the resize function that gets called when resize happens.
 *
 * @param obj The GLView object
 * @param func The callback function
 *
 * @ingroup Elm_GLView
 */
EAPI void elm_glview_resize_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);

/**
 * Set the render function that runs in the main loop.
 *
 * @param obj The GLView object
 * @param func The callback function
 *
 * @ingroup Elm_GLView
 */
EAPI void elm_glview_render_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);
#include "elm_glview_eo.legacy.h"
