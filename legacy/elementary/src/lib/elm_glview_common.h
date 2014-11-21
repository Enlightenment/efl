/**
 * @addtogroup GLView
 *
 * @{
 */

typedef void (*Elm_GLView_Func_Cb)(Evas_Object *obj);

/**
 * @brief Selects the target surface properties
 *
 * An OR combination of @c Elm_GLView_Mode values should be passed to
 * @ref elm_glview_mode_set when setting up a GL widget. These flags will
 * specify the properties of the rendering target surface; in particular,
 * the mode can request the surface to support alpha, depth and stencil buffers.
 *
 * @note @c ELM_GLVIEW_CLIENT_SIDE_ROTATION is a special value that indicates
 *       to EFL that the application will handle the view rotation when the
 *       device is rotated. This is needed only when the application requests
 *       direct rendering. Please refer to @ref Evas_GL
 *       for more information about direct rendering.
 *
 * @see elm_glview_mode_set
 * @see @ref elm_opengl_page
 */
typedef enum
{
   ELM_GLVIEW_NONE    = 0,
   // 0x1 is reserved for future use
   ELM_GLVIEW_ALPHA   = (1<<1), /**< Alpha channel enabled rendering mode */
   ELM_GLVIEW_DEPTH   = (1<<2), /**< Depth buffer enabled rendering mode (24 bits by default) */
   ELM_GLVIEW_STENCIL = (1<<3), /**< Stencil buffer enabled rendering mode (8 bits by default) */
   ELM_GLVIEW_DIRECT  = (1<<4), /**< Request direct rendering, unless there must be a fallback */
   ELM_GLVIEW_CLIENT_SIDE_ROTATION = (1<<5), /**< Client will handle GL view rotation if direct rendering is enabled */
   // Depth buffer sizes (3 bits)
   ELM_GLVIEW_DEPTH_8  = ELM_GLVIEW_DEPTH | (1 << 6), /**< Request min. 8 bits for the depth buffer */
   ELM_GLVIEW_DEPTH_16 = ELM_GLVIEW_DEPTH | (2 << 6), /**< Request min. 16 bits for the depth buffer */
   ELM_GLVIEW_DEPTH_24 = ELM_GLVIEW_DEPTH | (3 << 6), /**< Request min. 24 bits for the depth buffer (default) */
   ELM_GLVIEW_DEPTH_32 = ELM_GLVIEW_DEPTH | (4 << 6), /**< Request min. 32 bits for the depth buffer */
   // Stencil buffer sizes (3 bits)
   ELM_GLVIEW_STENCIL_1  = ELM_GLVIEW_STENCIL | (1 << 9), /**< Request min. 1 bits for the stencil buffer */
   ELM_GLVIEW_STENCIL_2  = ELM_GLVIEW_STENCIL | (2 << 9), /**< Request min. 2 bits for the stencil buffer */
   ELM_GLVIEW_STENCIL_4  = ELM_GLVIEW_STENCIL | (3 << 9), /**< Request min. 4 bits for the stencil buffer */
   ELM_GLVIEW_STENCIL_8  = ELM_GLVIEW_STENCIL | (4 << 9), /**< Request min. 8 bits for the stencil buffer (default) */
   ELM_GLVIEW_STENCIL_16 = ELM_GLVIEW_STENCIL | (5 << 9), /**< Request min. 16 bits for the stencil buffer */
   // MSAA params (2 bits)
   ELM_GLVIEW_MULTISAMPLE_LOW  = (1 << 12), /**< MSAA with minimum number of samples */
   ELM_GLVIEW_MULTISAMPLE_MED  = (2 << 12), /**< MSAA with half the number of maximum samples */
   ELM_GLVIEW_MULTISAMPLE_HIGH = (3 << 12)  /**< MSAA with maximum number of samples */
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
 * @}
 */
