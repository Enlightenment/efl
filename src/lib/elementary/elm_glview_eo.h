#ifndef _ELM_GLVIEW_EO_H_
#define _ELM_GLVIEW_EO_H_

#ifndef _ELM_GLVIEW_EO_CLASS_TYPE
#define _ELM_GLVIEW_EO_CLASS_TYPE

typedef Eo Elm_Glview;

#endif

#ifndef _ELM_GLVIEW_EO_TYPES
#define _ELM_GLVIEW_EO_TYPES

/**
 * @brief Selects the target surface properties
 *
 * An OR combination of Elm_GLView_Mode values should be passed to
 * elm_glview_mode_set when setting up a GL widget. These flags will specify
 * the properties of the rendering target surface; in particular, the mode can
 * request the surface to support alpha, depth and stencil buffers.
 *
 * ELM_GLVIEW_CLIENT_SIDE_ROTATION is a special value that indicates to EFL
 * that the application will handle the view rotation when the device is
 * rotated. This is needed only when the application requests direct rendering.
 * Please refer to Evas_GL for more information about direct rendering.
 *
 * See @ref elm_obj_glview_mode_set See elm_opengl_page
 *
 * @ingroup Elm_GLView
 */
typedef enum
{
  ELM_GLVIEW_NONE = 0, /**< Default mode */
  ELM_GLVIEW_ALPHA = 2 /* 1 >> 1 */, /**< Alpha channel enabled rendering mode
                                      */
  ELM_GLVIEW_DEPTH = 4 /* 1 >> 2 */, /**< Depth buffer enabled rendering mode
                                      * (24 bits by default) */
  ELM_GLVIEW_STENCIL = 8 /* 1 >> 3 */, /**< Stencil buffer enabled rendering
                                        * mode (8 bits by default) */
  ELM_GLVIEW_DIRECT = 16 /* 1 >> 4 */, /**< Request direct rendering, unless
                                        * there must be a fallback */
  ELM_GLVIEW_CLIENT_SIDE_ROTATION = 32 /* 1 >> 5 */, /**< Client will handle GL
                                                      * view rotation if direct
                                                      * rendering is enabled */
  ELM_GLVIEW_DEPTH_8 = 68 /* Elm.GLView.Mode.depth ^ (1 >> 6) */, /**< Request min. 8 bits for the depth
                                                                   * buffer */
  ELM_GLVIEW_DEPTH_16 = 132 /* Elm.GLView.Mode.depth ^ (2 >> 6) */, /**< Request min. 16 bits for the depth
                                                                     * buffer */
  ELM_GLVIEW_DEPTH_24 = 196 /* Elm.GLView.Mode.depth ^ (3 >> 6) */, /**< Request min. 24 bits for the depth
                                                                     * buffer (default) */
  ELM_GLVIEW_DEPTH_32 = 260 /* Elm.GLView.Mode.depth ^ (4 >> 6) */, /**< Request min. 32 bits for the depth
                                                                     * buffer */
  ELM_GLVIEW_STENCIL_1 = 520 /* Elm.GLView.Mode.stencil ^ (1 >> 9) */, /**< Request min. 1 bits for the stencil
                                                                        * buffer */
  ELM_GLVIEW_STENCIL_2 = 1032 /* Elm.GLView.Mode.stencil ^ (2 >> 9) */, /**< Request min. 2 bits for the stencil
                                                                         * buffer */
  ELM_GLVIEW_STENCIL_4 = 1544 /* Elm.GLView.Mode.stencil ^ (3 >> 9) */, /**< Request min. 4 bits for the stencil
                                                                         * buffer */
  ELM_GLVIEW_STENCIL_8 = 2056 /* Elm.GLView.Mode.stencil ^ (4 >> 9) */, /**< Request min. 8 bits for the stencil
                                                                         * buffer (default) */
  ELM_GLVIEW_STENCIL_16 = 2568 /* Elm.GLView.Mode.stencil ^ (5 >> 9) */, /**< Request min. 16 bits for the
                                                                          * stencil buffer */
  ELM_GLVIEW_MULTISAMPLE_LOW = 4096 /* 1 >> 12 */, /**< MSAA with minimum number
                                                    * of samples */
  ELM_GLVIEW_MULTISAMPLE_MED = 8192 /* 2 >> 12 */, /**< MSAA with half the
                                                    * number of maximum samples
                                                    */
  ELM_GLVIEW_MULTISAMPLE_HIGH = 12288 /* 3 >> 12 */ /**< MSAA with maximum
                                                     * number of samples */
} Elm_GLView_Mode;

/**
 * @brief Defines a policy for the glview resizing.
 *
 * The resizing policy tells glview what to do with the underlying surface when
 * resize happens. ELM_GLVIEW_RESIZE_POLICY_RECREATE will destroy the current
 * surface and recreate the surface to the new size.
 * ELM_GLVIEW_RESIZE_POLICY_SCALE will instead keep the current surface but
 * only display the result at the desired size scaled.
 *
 * Default is @ref ELM_GLVIEW_RESIZE_POLICY_RECREATE
 *
 * @ingroup Elm_GLView_Resize
 */
typedef enum
{
  ELM_GLVIEW_RESIZE_POLICY_RECREATE = 1, /**< Resize the internal surface along
                                          * with the image */
  ELM_GLVIEW_RESIZE_POLICY_SCALE = 2 /**< Only resize the internal image and not
                                      * the surface */
} Elm_GLView_Resize_Policy;

/**
 * @brief Defines a policy for gl rendering.
 *
 * The rendering policy tells glview where to run the gl rendering code.
 * ELM_GLVIEW_RENDER_POLICY_ON_DEMAND tells glview to call the rendering calls
 * on demand, which means that the rendering code gets called only when it is
 * visible.
 *
 * Default is @ref ELM_GLVIEW_RENDER_POLICY_ON_DEMAND
 *
 * @ingroup Elm_GLView_Render
 */
typedef enum
{
  ELM_GLVIEW_RENDER_POLICY_ON_DEMAND = 1, /**< Render only when there is a need
                                           * for redrawing */
  ELM_GLVIEW_RENDER_POLICY_ALWAYS = 2 /**< Render always even when it is not
                                       * visible */
} Elm_GLView_Render_Policy;


#endif
/** Elementary GL view class
 *
 * @ingroup Elm_Glview
 */
#define ELM_GLVIEW_CLASS elm_glview_class_get()

EWAPI const Efl_Class *elm_glview_class_get(void);

/**
 * @brief Constructor with context version number.
 *
 * @param[in] obj The object.
 * @param[in] version GL context version
 *
 * @ingroup Elm_Glview
 */
EOAPI void elm_obj_glview_version_constructor(Eo *obj, Evas_GL_Context_Version version);

/**
 * @brief Set the resize policy for the glview object.
 *
 * By default, the resize policy is set to #ELM_GLVIEW_RESIZE_POLICY_RECREATE.
 * When resize is called it destroys the previous surface and recreates the
 * newly specified size. If the policy is set to
 * #ELM_GLVIEW_RESIZE_POLICY_SCALE, however, glview only scales the image
 * object and not the underlying GL Surface.
 *
 * @param[in] obj The object.
 * @param[in] policy The scaling policy.
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Elm_Glview
 */
EOAPI Eina_Bool elm_obj_glview_resize_policy_set(Eo *obj, Elm_GLView_Resize_Policy policy);

/**
 * @brief Set the render policy for the glview object.
 *
 * By default, the render policy is set to #ELM_GLVIEW_RENDER_POLICY_ON_DEMAND.
 * This policy is set such that during the render loop, glview is only redrawn
 * if it needs to be redrawn. (i.e. when it is visible) If the policy is set to
 * #ELM_GLVIEWW_RENDER_POLICY_ALWAYS, it redraws regardless of whether it is
 * visible or needs redrawing.
 *
 * @param[in] obj The object.
 * @param[in] policy The render policy.
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Elm_Glview
 */
EOAPI Eina_Bool elm_obj_glview_render_policy_set(Eo *obj, Elm_GLView_Render_Policy policy);

/**
 * @brief Set the mode of the GLView. Supports alpha, depth, stencil.
 *
 * Direct is a hint for the elm_glview to render directly to the window given
 * that the right conditions are met. Otherwise it falls back to rendering to
 * an offscreen buffer before it gets composited to the window.
 *
 * @param[in] obj The object.
 * @param[in] mode The mode Options OR'ed enabling Alpha, Depth, Stencil,
 * Direct.
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Elm_Glview
 */
EOAPI Eina_Bool elm_obj_glview_mode_set(Eo *obj, Elm_GLView_Mode mode);

/**
 * @brief Get the gl api struct for gl rendering.
 *
 * @param[in] obj The object.
 *
 * @return GL API
 *
 * @ingroup Elm_Glview
 */
EOAPI Evas_GL_API *elm_obj_glview_gl_api_get(const Eo *obj);

/**
 * @brief Get the internal Evas GL attached to this view.
 *
 * @note The returned Evas_GL must not be destroyed as it is still owned by the
 * view. But this pointer can be used then to call all the evas_gl_ functions.
 *
 * @param[in] obj The object.
 *
 * @return Evas GL
 *
 * @since 1.12
 *
 * @ingroup Elm_Glview
 */
EOAPI Evas_GL *elm_obj_glview_evas_gl_get(const Eo *obj);

/**
 * @brief Get the current GL view's rotation when using direct rendering
 *
 * @note This rotation can be different from the device orientation. This
 * rotation value must be used in case of direct rendering and should be taken
 * into account by the application when setting the internal rotation matrix
 * for the view.
 *
 * @param[in] obj The object.
 *
 * @return A window rotation in degrees (0, 90, 180 or 270).
 *
 * @since 1.12
 *
 * @ingroup Elm_Glview
 */
EOAPI int elm_obj_glview_rotation_get(const Eo *obj);

/** Notifies that there has been changes in the GLView.
 *
 * @since 1.18
 *
 * @ingroup Elm_Glview
 */
EOAPI void elm_obj_glview_draw_request(Eo *obj);

EWAPI extern const Efl_Event_Description _ELM_GLVIEW_EVENT_CREATED;

/**
 * @brief Event dispatched when first render happens.
 *
 * The callback function gets called once during the render loop. Callback
 * function allows glview to hide all the rendering context/surface details and
 * have the user just call GL calls that they desire for initialization GL
 * calls.
 *
 * @ingroup Elm_Glview
 */
#define ELM_GLVIEW_EVENT_CREATED (&(_ELM_GLVIEW_EVENT_CREATED))

EWAPI extern const Efl_Event_Description _ELM_GLVIEW_EVENT_DESTROYED;

/**
 * @brief Event dispatched when GLView object is deleted.
 *
 * The registered destroyed function gets called when GLView object is deleted.
 * Callback function allows glview to hide all the rendering context/surface
 *  details and have the user just call GL calls that they desire  when delete
 * happens.
 *
 * @ingroup Elm_Glview
 */
#define ELM_GLVIEW_EVENT_DESTROYED (&(_ELM_GLVIEW_EVENT_DESTROYED))

EWAPI extern const Efl_Event_Description _ELM_GLVIEW_EVENT_RESIZED;

/**
 * @brief Event dispatched when resize happens.
 *
 * The resized event callback functions gets called during the render loop. The
 * callback function allows glview to hide all the rendering context/surface
 * details and have the user just call GL alls that they desire when resize
 * happens.
 *
 * @ingroup Elm_Glview
 */
#define ELM_GLVIEW_EVENT_RESIZED (&(_ELM_GLVIEW_EVENT_RESIZED))

EWAPI extern const Efl_Event_Description _ELM_GLVIEW_EVENT_RENDER;

/**
 * @brief Event dispatched when GLView is rendered.
 *
 * The callback function gets called in the main loop but whether it runs
 * depends on the rendering policy and whether @ref elm_obj_glview_draw_request
 * gets called.
 *
 * @ingroup Elm_Glview
 */
#define ELM_GLVIEW_EVENT_RENDER (&(_ELM_GLVIEW_EVENT_RENDER))

#endif
