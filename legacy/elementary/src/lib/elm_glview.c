#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_glview.h"

EAPI Eo_Op ELM_OBJ_GLVIEW_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_GLVIEW_CLASS

#define MY_CLASS_NAME "Elm_Glview"
#define MY_CLASS_NAME_LEGACY "elm_glview"

static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";
static const char SIG_LANG_CHANGED[] = "language,changed";

/* smart callbacks coming from elm glview objects: */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_FOCUSED, ""},
   {SIG_UNFOCUSED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static void
_elm_glview_smart_on_focus(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_on_focus(&int_ret));
   if (!int_ret) return;

   if (elm_widget_focus_get(obj))
     evas_object_focus_set(wd->resize_obj, EINA_TRUE);
   else
     evas_object_focus_set(wd->resize_obj, EINA_FALSE);

   if (ret) *ret = EINA_TRUE;
}

static void
_glview_update_surface(Evas_Object *obj)
{
   ELM_GLVIEW_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   if (!sd) return;

   if (sd->surface)
     {
        evas_object_image_native_surface_set
          (wd->resize_obj, NULL);
        evas_gl_surface_destroy(sd->evasgl, sd->surface);
        sd->surface = NULL;
     }

   evas_object_image_size_set(wd->resize_obj, sd->w, sd->h);

   if (!sd->surface)
     {
        Evas_Native_Surface ns;

        sd->surface = evas_gl_surface_create
            (sd->evasgl, sd->config, sd->w, sd->h);
        evas_gl_native_surface_get(sd->evasgl, sd->surface, &ns);
        evas_object_image_native_surface_set
          (wd->resize_obj, &ns);
        elm_glview_changed_set(obj);
     }
}

static void
_elm_glview_smart_resize(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   Elm_Glview_Smart_Data * sd = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   sd->resized = EINA_TRUE;

   if (sd->scale_policy == ELM_GLVIEW_RESIZE_POLICY_RECREATE)
     {
        if ((w == 0) || (h == 0))
          {
             w = 64;
             h = 64;
          }

        if ((sd->w == w) && (sd->h == h)) return;

        sd->w = w;
        sd->h = h;

        _glview_update_surface(obj);
     }
}

static Eina_Bool
_render_cb(void *obj)
{
   ELM_GLVIEW_DATA_GET(obj, sd);

   // Do a make current
   if (!evas_gl_make_current(sd->evasgl, sd->surface, sd->context))
     {
        sd->render_idle_enterer = NULL;
        ERR("Failed doing make current.\n");
        return EINA_FALSE;
     }

   // Call the init function if it hasn't been called already
   if (!sd->initialized)
     {
        if (sd->init_func) sd->init_func(obj);
        sd->initialized = EINA_TRUE;
     }

   if (sd->resized)
     {
        if (sd->resize_func) sd->resize_func(obj);
        sd->resized = EINA_FALSE;
     }

   if (sd->render_policy == ELM_GLVIEW_RENDER_POLICY_ALWAYS)
     evas_sync(evas_object_evas_get(obj));
   // Call the render function
   if (sd->render_func) sd->render_func(obj);

   // Depending on the policy return true or false
   if (sd->render_policy == ELM_GLVIEW_RENDER_POLICY_ON_DEMAND)
     return EINA_TRUE;
   else if (sd->render_policy == ELM_GLVIEW_RENDER_POLICY_ALWAYS)
     {
        // Return false so it only runs once
        sd->render_idle_enterer = NULL;
        return EINA_FALSE;
     }
   else
     {
        ERR("Invalid Render Policy.\n");
        sd->render_idle_enterer = NULL;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_set_render_policy_callback(Evas_Object *obj)
{
   ELM_GLVIEW_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   switch (sd->render_policy)
     {
      case ELM_GLVIEW_RENDER_POLICY_ON_DEMAND:
         // Delete idle_enterer if it for some reason is around
         ELM_SAFE_FREE(sd->render_idle_enterer, ecore_idle_enterer_del);

        // Set pixel getter callback
        evas_object_image_pixels_get_callback_set
          (wd->resize_obj,
          (Evas_Object_Image_Pixels_Get_Cb)_render_cb,
          obj);
        break;

      case ELM_GLVIEW_RENDER_POLICY_ALWAYS:
        // Unset the pixel getter callback if set already
        evas_object_image_pixels_get_callback_set
          (wd->resize_obj, NULL, NULL);

        break;

      default:
        ERR("Invalid Render Policy.\n");
        return;
     }
}

static void
_elm_glview_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Glview_Smart_Data *priv = _pd;
   Evas_Object *img;

   elm_widget_sub_object_parent_add(obj);

   // Create image to render Evas_GL Surface
   img = evas_object_image_filled_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, img, EINA_TRUE);
   evas_object_image_size_set(img, 1, 1);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());

   // Evas_GL
   priv->evasgl = evas_gl_new(evas_object_evas_get(obj));
   if (!priv->evasgl)
     {
        ERR("Failed Creating an Evas GL Object.\n");

        return;
     }

   // Create a default config
   priv->config = evas_gl_config_new();
   if (!priv->config)
     {
        ERR("Failed Creating a Config Object.\n");

        evas_gl_free(priv->evasgl);
        priv->evasgl = NULL;
        return;
     }
   priv->config->color_format = EVAS_GL_RGB_888;

   // Initialize variables
   priv->scale_policy = ELM_GLVIEW_RESIZE_POLICY_RECREATE;
   priv->render_policy = ELM_GLVIEW_RENDER_POLICY_ON_DEMAND;

   // Initialize it to (64,64)  (It's an arbitrary value)
   priv->w = 64;
   priv->h = 64;

   // Create Context
   priv->context = evas_gl_context_create(priv->evasgl, NULL);
   if (!priv->context)
     {
        ERR("Error Creating an Evas_GL Context.\n");

        evas_gl_config_free(priv->config);
        evas_gl_free(priv->evasgl);
        priv->evasgl = NULL;
        return;
     }
}

static void
_elm_glview_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Glview_Smart_Data * sd = _pd;

   // Call delete func if it's registered
   if (sd->del_func)
     {
        evas_gl_make_current(sd->evasgl, sd->surface, sd->context);
        sd->del_func(obj);
     }

   ecore_idle_enterer_del(sd->render_idle_enterer);

   if (sd->surface) evas_gl_surface_destroy(sd->evasgl, sd->surface);
   if (sd->context) evas_gl_context_destroy(sd->evasgl, sd->context);
   if (sd->config) evas_gl_config_free(sd->config);
   if (sd->evasgl) evas_gl_free(sd->evasgl);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_glview_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   const Elm_Glview_Smart_Data *sd = _pd;
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));

   if (!sd->evasgl)
     {
        eo_error_set(obj);
        return;
     }
}

EAPI Evas_GL_API *
elm_glview_gl_api_get(const Evas_Object *obj)
{
   ELM_GLVIEW_CHECK(obj) NULL;
   Evas_GL_API *ret = NULL;
   eo_do((Eo *)obj, elm_obj_glview_gl_api_get(&ret));
   return ret;
}

static void
_gl_api_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_GL_API **ret = va_arg(*list, Evas_GL_API **);
   const Elm_Glview_Smart_Data *sd = _pd;
   *ret = evas_gl_api_get(sd->evasgl);
}

EAPI Eina_Bool
elm_glview_mode_set(Evas_Object *obj,
                    Elm_GLView_Mode mode)
{
   ELM_GLVIEW_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_glview_mode_set(mode, &ret));
   return ret;
}

static void
_mode_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_GLView_Mode mode = va_arg(*list, Elm_GLView_Mode);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Glview_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   // Set the configs
   if (mode & ELM_GLVIEW_ALPHA) sd->config->color_format = EVAS_GL_RGBA_8888;
   else sd->config->color_format = EVAS_GL_RGB_888;

   if (mode & ELM_GLVIEW_DEPTH) sd->config->depth_bits = EVAS_GL_DEPTH_BIT_24;
   else sd->config->depth_bits = EVAS_GL_DEPTH_NONE;

   if (mode & ELM_GLVIEW_STENCIL)
     sd->config->stencil_bits = EVAS_GL_STENCIL_BIT_8;
   else sd->config->stencil_bits = EVAS_GL_STENCIL_NONE;

   if (mode & ELM_GLVIEW_DIRECT)
     sd->config->options_bits = EVAS_GL_OPTIONS_DIRECT;
   else sd->config->options_bits = EVAS_GL_OPTIONS_NONE;

   // Check for Alpha Channel and enable it
   if (mode & ELM_GLVIEW_ALPHA)
     evas_object_image_alpha_set(wd->resize_obj, EINA_TRUE);
   else
     evas_object_image_alpha_set(wd->resize_obj, EINA_FALSE);

   sd->mode = mode;

   _glview_update_surface(obj);
   elm_glview_changed_set(obj);

   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_glview_resize_policy_set(Evas_Object *obj,
                             Elm_GLView_Resize_Policy policy)
{
   ELM_GLVIEW_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_glview_resize_policy_set(policy, &ret));
   return ret;
}

static void
_resize_policy_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_GLView_Resize_Policy policy = va_arg(*list, Elm_GLView_Resize_Policy);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_TRUE;
   Elm_Glview_Smart_Data *sd = _pd;

   if (policy == sd->scale_policy) return;
   switch (policy)
     {
      case ELM_GLVIEW_RESIZE_POLICY_RECREATE:
      case ELM_GLVIEW_RESIZE_POLICY_SCALE:
        sd->scale_policy = policy;
        _glview_update_surface(obj);
        elm_glview_changed_set(obj);
        return;

      default:
        ERR("Invalid Scale Policy.\n");
        if (ret) *ret = EINA_FALSE;
        return;
     }
}

EAPI Eina_Bool
elm_glview_render_policy_set(Evas_Object *obj,
                             Elm_GLView_Render_Policy policy)
{
   ELM_GLVIEW_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_glview_render_policy_set(policy, &ret));
   return ret;
}

static void
_render_policy_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_GLView_Render_Policy policy = va_arg(*list, Elm_GLView_Render_Policy);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Glview_Smart_Data *sd = _pd;
   if (ret) *ret = EINA_TRUE;

   if ((policy != ELM_GLVIEW_RENDER_POLICY_ON_DEMAND) &&
       (policy != ELM_GLVIEW_RENDER_POLICY_ALWAYS))
     {
        ERR("Invalid Render Policy.\n");
        if (ret) *ret = EINA_FALSE;
        return;
     }

   if (sd->render_policy == policy) return;

   sd->render_policy = policy;
   _set_render_policy_callback(obj);
}

EAPI void
elm_glview_size_set(Evas_Object *obj,
                    int w,
                    int h)
{
   ELM_GLVIEW_CHECK(obj);
   eo_do(obj, elm_obj_glview_size_set(w, h));
}

static void
_size_set(Eo *obj, void *_pd, va_list *list)
{
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);
   Elm_Glview_Smart_Data *sd = _pd;

   if ((w == sd->w) && (h == sd->h)) return;

   sd->w = w;
   sd->h = h;

   _glview_update_surface(obj);
   elm_glview_changed_set(obj);
}

EAPI void
elm_glview_size_get(const Evas_Object *obj,
                    int *w,
                    int *h)
{
   ELM_GLVIEW_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_glview_size_get(w, h));
}

static void
_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);
   Elm_Glview_Smart_Data *sd = _pd;

   if (w) *w = sd->w;
   if (h) *h = sd->h;
}

EAPI void
elm_glview_init_func_set(Evas_Object *obj,
                         Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   eo_do(obj, elm_obj_glview_init_func_set(func));
}

static void
_init_func_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_GLView_Func_Cb func = va_arg(*list, Elm_GLView_Func_Cb);
   Elm_Glview_Smart_Data *sd = _pd;

   sd->initialized = EINA_FALSE;
   sd->init_func = func;
}

EAPI void
elm_glview_del_func_set(Evas_Object *obj,
                        Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   eo_do(obj, elm_obj_glview_del_func_set(func));
}

static void
_del_func_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_GLView_Func_Cb func = va_arg(*list, Elm_GLView_Func_Cb);
   Elm_Glview_Smart_Data *sd = _pd;

   sd->del_func = func;
}

EAPI void
elm_glview_resize_func_set(Evas_Object *obj,
                           Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   eo_do(obj, elm_obj_glview_resize_func_set(func));
}

static void
_resize_func_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_GLView_Func_Cb func = va_arg(*list, Elm_GLView_Func_Cb);
   Elm_Glview_Smart_Data *sd = _pd;

   sd->resize_func = func;
}

EAPI void
elm_glview_render_func_set(Evas_Object *obj,
                           Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   eo_do(obj, elm_obj_glview_render_func_set(func));
}

static void
_render_func_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_GLView_Func_Cb func = va_arg(*list, Elm_GLView_Func_Cb);
   Elm_Glview_Smart_Data *sd = _pd;

   sd->render_func = func;
   _set_render_policy_callback(obj);
}

EAPI void
elm_glview_changed_set(Evas_Object *obj)
{
   ELM_GLVIEW_CHECK(obj);
   eo_do(obj, elm_obj_glview_changed_set());
}

static void
_changed_set(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Glview_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_image_pixels_dirty_set
     (wd->resize_obj, EINA_TRUE);
   if (sd->render_policy == ELM_GLVIEW_RENDER_POLICY_ALWAYS &&
       !sd->render_idle_enterer)
     sd->render_idle_enterer =
       ecore_idle_enterer_before_add((Ecore_Task_Cb)_render_cb, obj);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_glview_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_glview_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _elm_glview_smart_resize),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ON_FOCUS), _elm_glview_smart_on_focus),

        EO_OP_FUNC(ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_GL_API_GET), _gl_api_get),
        EO_OP_FUNC(ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_MODE_SET), _mode_set),
        EO_OP_FUNC(ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_RESIZE_POLICY_SET), _resize_policy_set),
        EO_OP_FUNC(ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_RENDER_POLICY_SET), _render_policy_set),
        EO_OP_FUNC(ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_SIZE_SET), _size_set),
        EO_OP_FUNC(ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_SIZE_GET), _size_get),
        EO_OP_FUNC(ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_INIT_FUNC_SET), _init_func_set),
        EO_OP_FUNC(ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_DEL_FUNC_SET), _del_func_set),
        EO_OP_FUNC(ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_RESIZE_FUNC_SET), _resize_func_set),
        EO_OP_FUNC(ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_RENDER_FUNC_SET), _render_func_set),
        EO_OP_FUNC(ELM_OBJ_GLVIEW_ID(ELM_OBJ_GLVIEW_SUB_ID_CHANGED_SET), _changed_set),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_GLVIEW_SUB_ID_GL_API_GET, "Get the gl api struct for gl rendering."),
     EO_OP_DESCRIPTION(ELM_OBJ_GLVIEW_SUB_ID_MODE_SET, "Set the mode of the GLView."),
     EO_OP_DESCRIPTION(ELM_OBJ_GLVIEW_SUB_ID_RESIZE_POLICY_SET, "Set the resize policy for the glview object."),
     EO_OP_DESCRIPTION(ELM_OBJ_GLVIEW_SUB_ID_RENDER_POLICY_SET, "Set the render policy for the glview object."),
     EO_OP_DESCRIPTION(ELM_OBJ_GLVIEW_SUB_ID_SIZE_SET, "Sets the size of the glview."),
     EO_OP_DESCRIPTION(ELM_OBJ_GLVIEW_SUB_ID_SIZE_GET, "Get the size of the glview."),
     EO_OP_DESCRIPTION(ELM_OBJ_GLVIEW_SUB_ID_INIT_FUNC_SET, "Set the init function that runs once in the main loop."),
     EO_OP_DESCRIPTION(ELM_OBJ_GLVIEW_SUB_ID_DEL_FUNC_SET, "Set the render function that runs in the main loop."),
     EO_OP_DESCRIPTION(ELM_OBJ_GLVIEW_SUB_ID_RESIZE_FUNC_SET, "Set the resize function that gets called when resize happens."),
     EO_OP_DESCRIPTION(ELM_OBJ_GLVIEW_SUB_ID_RENDER_FUNC_SET, "Set the render function that runs in the main loop."),
     EO_OP_DESCRIPTION(ELM_OBJ_GLVIEW_SUB_ID_CHANGED_SET, "Notifies that there has been changes in the GLView."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_GLVIEW_BASE_ID, op_desc, ELM_OBJ_GLVIEW_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Glview_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_glview_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, NULL);
