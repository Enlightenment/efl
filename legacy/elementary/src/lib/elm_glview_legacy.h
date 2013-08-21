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
 * Get the gl api struct for gl rendering
 *
 * @param obj The glview object
 * @return The api object or NULL if it cannot be created
 *
 * @ingroup GLView
 */
EAPI Evas_GL_API *elm_glview_gl_api_get(const Evas_Object *obj);

/**
 * Set the mode of the GLView. Supports alpha, depth, stencil.
 *
 * @param obj The glview object
 * @param mode The mode Options OR'ed enabling Alpha, Depth, Stencil, Direct.
 * @return True if set properly.
 *
 * Direct is a hint for the elm_glview to render directly to the window
 * given that the right conditions are met. Otherwise it falls back
 * to rendering to an offscreen buffer before it gets composited to the
 * window.
 *
 * @ingroup GLView
 */
EAPI Eina_Bool    elm_glview_mode_set(Evas_Object *obj, Elm_GLView_Mode mode);

/**
 * Set the resize policy for the glview object.
 *
 * @param obj The glview object.
 * @param policy The scaling policy.
 *
 * By default, the resize policy is set to ELM_GLVIEW_RESIZE_POLICY_RECREATE.
 * When resize is called it destroys the previous surface and recreates the
 * newly specified size. If the policy is set to
 * ELM_GLVIEW_RESIZE_POLICY_SCALE, however, glview only scales the image
 * object and not the underlying GL Surface.
 *
 * @ingroup GLView
 */
EAPI Eina_Bool    elm_glview_resize_policy_set(Evas_Object *obj, Elm_GLView_Resize_Policy policy);

/**
 * Set the render policy for the glview object.
 *
 * @param obj The glview object.
 * @param policy The render policy.
 *
 * By default, the render policy is set to ELM_GLVIEW_RENDER_POLICY_ON_DEMAND.
 * This policy is set such that during the render loop, glview is only
 * redrawn if it needs to be redrawn. (i.e. when it is visible) If the policy
 * is set to ELM_GLVIEWW_RENDER_POLICY_ALWAYS, it redraws regardless of
 * whether it is visible or needs redrawing.
 *
 * @ingroup GLView
 */
EAPI Eina_Bool    elm_glview_render_policy_set(Evas_Object *obj, Elm_GLView_Render_Policy policy);

/**
 * Sets the size of the glview
 *
 * @param obj The glview object
 * @param w width of the glview object
 * @param h height of the glview object
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 * Get the size of the glview.
 *
 * @param obj The glview object
 * @param w width of the glview object
 * @param h height of the glview object
 *
 * Note that this function returns the actual image size of the
 * glview.  This means that when the scale policy is set to
 * ELM_GLVIEW_RESIZE_POLICY_SCALE, it'll return the non-scaled
 * size.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

/**
 * Set the init function that runs once in the main loop.
 *
 * @param obj The glview object.
 * @param func The init function to be registered.
 *
 * The registered init function gets called once during the render loop.
 * This function allows glview to hide all the rendering context/surface
 * details and have the user just call GL calls that they desire
 * for initialization GL calls.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_init_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);

/**
 * Set the render function that runs in the main loop.
 *
 * @param obj The glview object.
 * @param func The delete function to be registered.
 *
 * The registered del function gets called when GLView object is deleted.
 * This function allows glview to hide all the rendering context/surface
 * details and have the user just call GL calls that they desire
 * when delete happens.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_del_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);

/**
 * Set the resize function that gets called when resize happens.
 *
 * @param obj The glview object.
 * @param func The resize function to be registered.
 *
 * The resize function gets called during the render loop.
 * This function allows glview to hide all the rendering context/surface
 * details and have the user just call GL calls that they desire
 * when resize happens.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_resize_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);

/**
 * Set the render function that runs in the main loop.
 *
 * The render function gets called in the main loop but whether it runs
 * depends on the rendering policy and whether elm_glview_changed_set()
 * gets called.
 *
 * @param obj The glview object.
 * @param func The render function to be registered.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_render_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);

/**
 * Notifies that there has been changes in the GLView.
 *
 * @param obj The glview object.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_changed_set(Evas_Object *obj);
