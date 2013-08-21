#define ELM_OBJ_GLVIEW_CLASS elm_obj_glview_class_get()

const Eo_Class *elm_obj_glview_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_GLVIEW_BASE_ID;

enum
{
   ELM_OBJ_GLVIEW_SUB_ID_GL_API_GET,
   ELM_OBJ_GLVIEW_SUB_ID_MODE_SET,
   ELM_OBJ_GLVIEW_SUB_ID_RESIZE_POLICY_SET,
   ELM_OBJ_GLVIEW_SUB_ID_RENDER_POLICY_SET,
   ELM_OBJ_GLVIEW_SUB_ID_SIZE_SET,
   ELM_OBJ_GLVIEW_SUB_ID_SIZE_GET,
   ELM_OBJ_GLVIEW_SUB_ID_INIT_FUNC_SET,
   ELM_OBJ_GLVIEW_SUB_ID_DEL_FUNC_SET,
   ELM_OBJ_GLVIEW_SUB_ID_RESIZE_FUNC_SET,
   ELM_OBJ_GLVIEW_SUB_ID_RENDER_FUNC_SET,
   ELM_OBJ_GLVIEW_SUB_ID_CHANGED_SET,
   ELM_OBJ_GLVIEW_SUB_ID_LAST
};

#define ELM_OBJ_GLVIEW_ID(sub_id) (ELM_OBJ_GLVIEW_BASE_ID + sub_id)


/**
 * @def elm_obj_glview_gl_api_get
 * @since 1.8
 *
 * Get the gl api struct for gl rendering
 *
 * @param[out] ret
 *
 * @see elm_glview_gl_api_get
 *
 * @ingroup GLView
 */
#define elm_obj_glview_gl_api_get(ret) ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_GL_API_GET), EO_TYPECHECK(Evas_GL_API **, ret)

/**
 * @def elm_obj_glview_mode_set
 * @since 1.8
 *
 * Set the mode of the GLView. Supports alpha, depth, stencil.
 *
 * @param[in] mode
 * @param[out] ret
 *
 * @see elm_glview_mode_set
 *
 * @ingroup GLView
 */
#define elm_obj_glview_mode_set(mode, ret) ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_MODE_SET), EO_TYPECHECK(Elm_GLView_Mode, mode), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_glview_resize_policy_set
 * @since 1.8
 *
 * Set the resize policy for the glview object.
 *
 * @param[in] policy
 * @param[out] ret
 *
 * @see elm_glview_resize_policy_set
 *
 * @ingroup GLView
 */
#define elm_obj_glview_resize_policy_set(policy, ret) ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_RESIZE_POLICY_SET), EO_TYPECHECK(Elm_GLView_Resize_Policy, policy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_glview_render_policy_set
 * @since 1.8
 *
 * Set the render policy for the glview object.
 *
 * @param[in] policy
 * @param[out] ret
 *
 * @see elm_glview_render_policy_set
 *
 * @ingroup GLView
 */
#define elm_obj_glview_render_policy_set(policy, ret) ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_RENDER_POLICY_SET), EO_TYPECHECK(Elm_GLView_Render_Policy, policy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_glview_size_set
 * @since 1.8
 *
 * Sets the size of the glview
 *
 * @param[in] w
 * @param[in] h
 *
 * @see elm_glview_size_set
 *
 * @ingroup GLView
 */
#define elm_obj_glview_size_set(w, h) ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_SIZE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def elm_obj_glview_size_get
 * @since 1.8
 *
 * Get the size of the glview.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see elm_glview_size_get
 *
 * @ingroup GLView
 */
#define elm_obj_glview_size_get(w, h) ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def elm_obj_glview_init_func_set
 * @since 1.8
 *
 * Set the init function that runs once in the main loop.
 *
 * @param[in] func
 *
 * @see elm_glview_init_func_set
 *
 * @ingroup GLView
 */
#define elm_obj_glview_init_func_set(func) ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_INIT_FUNC_SET), EO_TYPECHECK(Elm_GLView_Func_Cb, func)

/**
 * @def elm_obj_glview_del_func_set
 * @since 1.8
 *
 * Set the render function that runs in the main loop.
 *
 * @param[in] func
 *
 * @see elm_glview_del_func_set
 *
 * @ingroup GLView
 */
#define elm_obj_glview_del_func_set(func) ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_DEL_FUNC_SET), EO_TYPECHECK(Elm_GLView_Func_Cb, func)

/**
 * @def elm_obj_glview_resize_func_set
 * @since 1.8
 *
 * Set the resize function that gets called when resize happens.
 *
 * @param[in] func
 *
 * @see elm_glview_resize_func_set
 *
 * @ingroup GLView
 */
#define elm_obj_glview_resize_func_set(func) ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_RESIZE_FUNC_SET), EO_TYPECHECK(Elm_GLView_Func_Cb, func)

/**
 * @def elm_obj_glview_render_func_set
 * @since 1.8
 *
 * Set the render function that runs in the main loop.
 *
 * @param[in] func
 *
 * @see elm_glview_render_func_set
 *
 * @ingroup GLView
 */
#define elm_obj_glview_render_func_set(func) ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_RENDER_FUNC_SET), EO_TYPECHECK(Elm_GLView_Func_Cb, func)

/**
 * @def elm_obj_glview_changed_set
 * @since 1.8
 *
 * Notifies that there has been changes in the GLView.
 *
 *
 * @see elm_glview_changed_set
 *
 * @ingroup GLView
 */
#define elm_obj_glview_changed_set() ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_CHANGED_SET)
