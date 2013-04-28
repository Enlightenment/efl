typedef void (*Elm_GLView_Func_Cb)(Evas_Object *obj);

/**
 * Defines mode of GLView
 *
 * @ingroup GLView
 */
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
 * @ingroup GLView
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
 * @ingroup GLView
 */
typedef enum
{
   ELM_GLVIEW_RENDER_POLICY_ON_DEMAND = 1, /**< Render only when there is a need for redrawing */
   ELM_GLVIEW_RENDER_POLICY_ALWAYS    = 2  /**< Render always even when it is not visible */
} Elm_GLView_Render_Policy;

