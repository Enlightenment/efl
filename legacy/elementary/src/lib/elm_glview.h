/**
 * @defgroup GLView GLView
 * @ingroup Elementary
 *
 * @image html glview_inheritance_tree.png
 * @image latex glview_inheritance_tree.eps
 *
 * A GLView widget allows for simple GL rendering in elementary environment.
 * GLView hides all the complicated evas_gl details so that the user only
 * has to deal with registering a few callback functions for rendering
 * to a surface using OpenGL APIs.
 *
 * Below is an illustrative example of how to use GLView and and OpenGL
 * to render in elementary environment.
 * @ref glview_example_01_page
 *
 */

typedef void (*Elm_GLView_Func_Cb)(Evas_Object *obj);

typedef enum _Elm_GLView_Mode
{
   ELM_GLVIEW_NONE    = 0,
   ELM_GLVIEW_ALPHA   = (1<<1), /**< Alpha channel enabled rendering mode */
   ELM_GLVIEW_DEPTH   = (1<<2), /**< Depth buffer enabled rendering mode */
   ELM_GLVIEW_STENCIL = (1<<3), /**< Stencil buffer enabled rendering mode */
   ELM_GLVIEW_DIRECT  = (1<<4)  /**< Direct rendering optimization hint */
} Elm_GLView_Mode;

/**
 * Defines a policy for the glview resizing.
 *
 * The resizing policy tells glview what to do with the underlying
 * surface when resize happens. ELM_GLVIEW_RESIZE_POLICY_RECREATE
 * will destroy the current surface and recreate the surface to the
 * new size.  ELM_GLVIEW_RESIZE_POLICY_SCALE will instead keep the
 * current surface but only display the result at the desired size
 * scaled.
 *
 * @note Default is ELM_GLVIEW_RESIZE_POLICY_RECREATE
 */
typedef enum
{
   ELM_GLVIEW_RESIZE_POLICY_RECREATE = 1, /**< Resize the internal surface along with the image */
   ELM_GLVIEW_RESIZE_POLICY_SCALE    = 2  /**< Only resize the internal image and not the surface */
} Elm_GLView_Resize_Policy;

/**
 * Defines a policy for gl rendering.
 *
 * The rendering policy tells glview where to run the gl rendering code.
 * ELM_GLVIEW_RENDER_POLICY_ON_DEMAND tells glview to call the rendering
 * calls on demand, which means that the rendering code gets called
 * only when it is visible.
 *
 * @note Default is ELM_GLVIEW_RENDER_POLICY_ON_DEMAND
 */
typedef enum
{
   ELM_GLVIEW_RENDER_POLICY_ON_DEMAND = 1, /**< Render only when there is a need for redrawing */
   ELM_GLVIEW_RENDER_POLICY_ALWAYS    = 2  /**< Render always even when it is not visible */
} Elm_GLView_Render_Policy;

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
 * Gets the size of the glview.
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
 * Gets the gl api struct for gl rendering
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

/**
 * @}
 */
