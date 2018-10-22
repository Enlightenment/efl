#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_glview.h"

#define MY_CLASS ELM_GLVIEW_CLASS

#define MY_CLASS_NAME "Elm_Glview"
#define MY_CLASS_NAME_LEGACY "elm_glview"

static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";

/* smart callbacks coming from elm glview objects: */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_FOCUSED, ""},
   {SIG_UNFOCUSED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

EOLIAN static Eina_Bool
_elm_glview_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Glview_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   if (efl_ui_focus_object_focus_get(obj))
     evas_object_focus_set(wd->resize_obj, EINA_TRUE);
   else
     evas_object_focus_set(wd->resize_obj, EINA_FALSE);

   return EINA_TRUE;
}

static void
_glview_update_surface(Evas_Object *obj)
{
   Evas_Native_Surface ns = {};
   Evas_GL_Options_Bits opt;

   ELM_GLVIEW_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   if (!sd) return;

   evas_gl_make_current(sd->evasgl, NULL, NULL);

   if (sd->surface)
     {
        evas_object_image_native_surface_set(wd->resize_obj, NULL);
        evas_gl_surface_destroy(sd->evasgl, sd->surface);
     }

   evas_object_image_size_set(wd->resize_obj, sd->w, sd->h);

   opt = sd->config->options_bits;
   if ((opt & EVAS_GL_OPTIONS_DIRECT) &&
       (sd->render_policy != ELM_GLVIEW_RENDER_POLICY_ON_DEMAND))
     {
        if (!sd->warned_about_dr)
          {
             WRN("App requested direct rendering but render policy is not ON_DEMAND. "
                 "Disabling direct rendering...");
             sd->warned_about_dr = EINA_TRUE;
          }
        sd->config->options_bits &= ~(EVAS_GL_OPTIONS_DIRECT);
     }
   sd->surface = evas_gl_surface_create(sd->evasgl, sd->config, sd->w, sd->h);
   sd->config->options_bits = opt;
   evas_gl_native_surface_get(sd->evasgl, sd->surface, &ns);
   evas_object_image_native_surface_set(wd->resize_obj, &ns);
   elm_obj_glview_draw_request(obj);

   // fake a resize event so that clients can reconfigure their viewport
   sd->resized = EINA_TRUE;
}

EOLIAN static void
_elm_glview_efl_gfx_entity_size_set(Eo *obj, Elm_Glview_Data *sd, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);

   sd->resized = EINA_TRUE;

   if (sd->scale_policy == ELM_GLVIEW_RESIZE_POLICY_RECREATE)
     {
        if ((sz.w == 0) || (sz.h == 0))
          sz = EINA_SIZE2D(64, 64);

        sd->w = sz.w;
        sd->h = sz.h;

        _glview_update_surface(obj);
     }
}

static void
_render_cb(void *obj, const Efl_Event *event EINA_UNUSED)
{
   ELM_GLVIEW_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, );

   evas_object_render_op_set(wd->resize_obj, evas_object_render_op_get(obj));

   // Do a make current
   if (!evas_gl_make_current(sd->evasgl, sd->surface, sd->context))
     {
        ERR("Failed doing make current.");
        goto on_error;
     }

   // Call the init function if it hasn't been called already
   if (!sd->initialized)
     {
        //TODO:will be optimized
        efl_event_callback_legacy_call(obj, ELM_GLVIEW_EVENT_CREATED, NULL);
        if (sd->init_func) sd->init_func(obj);
        sd->initialized = EINA_TRUE;
     }

   if (sd->resized)
     {
        //TODO:will be optimized
        efl_event_callback_legacy_call(obj, ELM_GLVIEW_EVENT_RESIZED, NULL);
        if (sd->resize_func) sd->resize_func(obj);
        sd->resized = EINA_FALSE;
     }

   if (sd->render_policy == ELM_GLVIEW_RENDER_POLICY_ALWAYS)
     evas_sync(evas_object_evas_get(obj));
   // Call the render function
   if (sd->render_func) sd->render_func(obj);
   //TODO:will be optimized
   efl_event_callback_legacy_call(obj, ELM_GLVIEW_EVENT_RENDER, NULL);

   // Depending on the policy return true or false
   if (sd->render_policy == ELM_GLVIEW_RENDER_POLICY_ON_DEMAND)
     {
        return;
     }
   else if (sd->render_policy == ELM_GLVIEW_RENDER_POLICY_ALWAYS)
     {
        // Return false so it only runs once
        goto on_error;
     }
   else
     {
        ERR("Invalid Render Policy.");
        goto on_error;
     }

   return;

 on_error:
   efl_event_callback_del(efl_main_loop_get(),
                         EFL_LOOP_EVENT_IDLE_ENTER,
                         _render_cb,
                         obj);
   sd->render_idle_enterer = 0;
}

static void
_set_render_policy_callback(Evas_Object *obj)
{
   ELM_GLVIEW_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   switch (sd->render_policy)
     {
      case ELM_GLVIEW_RENDER_POLICY_ON_DEMAND:
         if (sd->render_idle_enterer)
           {
              evas_object_image_pixels_dirty_set(wd->resize_obj, EINA_TRUE);
              evas_object_image_data_update_add(wd->resize_obj, 0, 0, sd->w, sd->h);
           }
         // Delete idle_enterer if it for some reason is around
         efl_event_callback_del(efl_main_loop_get(),
                               EFL_LOOP_EVENT_IDLE_ENTER,
                               _render_cb,
                               obj);
         sd->render_idle_enterer = 0;

        // Set pixel getter callback
        evas_object_image_pixels_get_callback_set
          (wd->resize_obj,
          (Evas_Object_Image_Pixels_Get_Cb)_render_cb,
          obj);
        break;

      case ELM_GLVIEW_RENDER_POLICY_ALWAYS:
        if (evas_object_image_pixels_dirty_get(wd->resize_obj) && !sd->render_idle_enterer)
          sd->render_idle_enterer = efl_event_callback_priority_add(efl_main_loop_get(),
                                                                   EFL_LOOP_EVENT_IDLE_ENTER,
                                                                   EFL_CALLBACK_PRIORITY_BEFORE,
                                                                   _render_cb,
                                                                   obj);
        // Unset the pixel getter callback if set already
        evas_object_image_pixels_get_callback_set
          (wd->resize_obj, NULL, NULL);

        break;

      default:
        ERR("Invalid Render Policy.");
        return;
     }
}

EOLIAN static void
_elm_glview_efl_canvas_group_group_add(Eo *obj, Elm_Glview_Data *priv EINA_UNUSED)
{
   Evas_Object *img;

   elm_widget_sub_object_parent_add(obj);

   // Create image to render Evas_GL Surface
   img = evas_object_image_filled_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, img);
   evas_object_image_size_set(img, 1, 1);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
}

static void
_elm_glview_constructor(Eo *obj, Elm_Glview_Data *priv)
{
   // Evas_GL
   priv->evasgl = evas_gl_new(evas_object_evas_get(obj));
   if (!priv->evasgl)
     {
        ERR("Failed Creating an Evas GL Object.");
        return;
     }

   // Create a default config
   priv->config = evas_gl_config_new();
   if (!priv->config)
     {
        ERR("Failed Creating a Config Object.");
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

   // Set context version
   if (!priv->gles_version)
     priv->gles_version = EVAS_GL_GLES_2_X;
   priv->config->gles_version = priv->gles_version;

   // Create Context
   if (priv->gles_version == EVAS_GL_GLES_2_X)
     priv->context = evas_gl_context_create(priv->evasgl, NULL);
   else
     priv->context = evas_gl_context_version_create(priv->evasgl, NULL, priv->gles_version);
   if (!priv->context)
     {
        ERR("Error Creating an Evas_GL Context.");
        ELM_SAFE_FREE(priv->config, evas_gl_config_free);
        ELM_SAFE_FREE(priv->evasgl, evas_gl_free);
        return;
     }
}

EOLIAN static void
_elm_glview_efl_canvas_group_group_del(Eo *obj, Elm_Glview_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   // Call delete func if it's registered
   if (sd->del_func)
     {
        evas_gl_make_current(sd->evasgl, sd->surface, sd->context);
        sd->del_func(obj);
     }
   //TODO:will be optimised
   efl_event_callback_legacy_call(obj, ELM_GLVIEW_EVENT_DESTROYED, NULL);

   efl_event_callback_del(efl_main_loop_get(),
                         EFL_LOOP_EVENT_IDLE_ENTER,
                         _render_cb,
                         obj);
   evas_gl_make_current(sd->evasgl, NULL, NULL);

   if (sd->surface)
     {
        evas_object_image_native_surface_set(wd->resize_obj, NULL);
        evas_gl_surface_destroy(sd->evasgl, sd->surface);
     }
   if (sd->context) evas_gl_context_destroy(sd->evasgl, sd->context);
   if (sd->config) evas_gl_config_free(sd->config);
   if (sd->evasgl) evas_gl_free(sd->evasgl);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

static void
_cb_added(void *data EINA_UNUSED, const Efl_Event *ev)
{
   const Efl_Callback_Array_Item *event = ev->info;

   ELM_GLVIEW_DATA_GET(ev->object, sd);

   if (event->desc == ELM_GLVIEW_EVENT_CREATED)
     {
        sd->initialized = EINA_FALSE;
     }
   else if (event->desc == ELM_GLVIEW_EVENT_RENDER)
     {
        _set_render_policy_callback(ev->object);
     }
}

EAPI Evas_Object *
elm_glview_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent,
                         elm_obj_glview_version_constructor(efl_added, EVAS_GL_GLES_2_X));
}

EAPI Evas_Object *
elm_glview_version_add(Evas_Object *parent, Evas_GL_Context_Version version)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent,
                         elm_obj_glview_version_constructor(efl_added, version));
}

EOLIAN static void
_elm_glview_version_constructor(Eo *obj, Elm_Glview_Data *sd,
                                Evas_GL_Context_Version version)
{
   sd->gles_version =
     ((version > 0) && (version <= 3)) ? version : EVAS_GL_GLES_2_X;
   _elm_glview_constructor(obj, sd);

   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_ANIMATION);
   efl_event_callback_add(obj, EFL_EVENT_CALLBACK_ADD, _cb_added, NULL);
}

EOLIAN static Efl_Object*
_elm_glview_efl_object_constructor(Eo *obj, Elm_Glview_Data *pd)
{
   legacy_object_focus_handle(obj);
   return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_elm_glview_efl_object_finalize(Eo *obj, Elm_Glview_Data *sd)
{
   if (!sd->evasgl)
     {
        ERR("Failed");
        return NULL;
     }

   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static Evas_GL_API*
_elm_glview_gl_api_get(const Eo *obj EINA_UNUSED, Elm_Glview_Data *sd)
{
   return evas_gl_context_api_get(sd->evasgl, sd->context);
}

EOLIAN static Eina_Bool
_elm_glview_mode_set(Eo *obj, Elm_Glview_Data *sd, Elm_GLView_Mode mode)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   // Set the configs
   if (mode & ELM_GLVIEW_ALPHA) sd->config->color_format = EVAS_GL_RGBA_8888;
   else sd->config->color_format = EVAS_GL_RGB_888;

   if (mode & ELM_GLVIEW_DEPTH)
     {
        const int mask = 7 << 6;
        if ((mode & mask) == (ELM_GLVIEW_DEPTH_8 & mask))
          sd->config->depth_bits = EVAS_GL_DEPTH_BIT_8;
        else if ((mode & mask) == (ELM_GLVIEW_DEPTH_16 & mask))
          sd->config->depth_bits = EVAS_GL_DEPTH_BIT_16;
        else if ((mode & mask) == (ELM_GLVIEW_DEPTH_24 & mask))
          sd->config->depth_bits = EVAS_GL_DEPTH_BIT_24;
        else if ((mode & mask) == (ELM_GLVIEW_DEPTH_32 & mask))
          sd->config->depth_bits = EVAS_GL_DEPTH_BIT_32;
        else
          sd->config->depth_bits = EVAS_GL_DEPTH_BIT_24;
     }
   else
     sd->config->depth_bits = EVAS_GL_DEPTH_NONE;

   if (mode & ELM_GLVIEW_STENCIL)
     {
        const int mask = 7 << 9;
        if ((mode & mask) == (ELM_GLVIEW_STENCIL_1 & mask))
          sd->config->stencil_bits = EVAS_GL_STENCIL_BIT_1;
        else if ((mode & mask) == (ELM_GLVIEW_STENCIL_2 & mask))
          sd->config->stencil_bits = EVAS_GL_STENCIL_BIT_2;
        else if ((mode & mask) == (ELM_GLVIEW_STENCIL_4 & mask))
          sd->config->stencil_bits = EVAS_GL_STENCIL_BIT_4;
        else if ((mode & mask) == (ELM_GLVIEW_STENCIL_8 & mask))
          sd->config->stencil_bits = EVAS_GL_STENCIL_BIT_8;
        else if ((mode & mask) == (ELM_GLVIEW_STENCIL_16 & mask))
          sd->config->stencil_bits = EVAS_GL_STENCIL_BIT_16;
        else
          sd->config->stencil_bits = EVAS_GL_STENCIL_BIT_8;
     }
   else
     sd->config->stencil_bits = EVAS_GL_STENCIL_NONE;

   if (mode & ELM_GLVIEW_MULTISAMPLE_HIGH)
     {
        if ((mode & ELM_GLVIEW_MULTISAMPLE_HIGH) == ELM_GLVIEW_MULTISAMPLE_LOW)
          sd->config->multisample_bits = EVAS_GL_MULTISAMPLE_LOW;
        else if ((mode & ELM_GLVIEW_MULTISAMPLE_HIGH) == ELM_GLVIEW_MULTISAMPLE_MED)
          sd->config->multisample_bits = EVAS_GL_MULTISAMPLE_MED;
        else
          sd->config->multisample_bits = EVAS_GL_MULTISAMPLE_HIGH;
     }
   else
     sd->config->multisample_bits = EVAS_GL_MULTISAMPLE_NONE;

   sd->config->options_bits = EVAS_GL_OPTIONS_NONE;
   if (mode & ELM_GLVIEW_DIRECT)
     sd->config->options_bits = EVAS_GL_OPTIONS_DIRECT;
   if (mode & ELM_GLVIEW_CLIENT_SIDE_ROTATION)
     sd->config->options_bits |= EVAS_GL_OPTIONS_CLIENT_SIDE_ROTATION;

   // Check for Alpha Channel and enable it
   if (mode & ELM_GLVIEW_ALPHA)
     evas_object_image_alpha_set(wd->resize_obj, EINA_TRUE);
   else
     evas_object_image_alpha_set(wd->resize_obj, EINA_FALSE);

   sd->mode = mode;
   sd->warned_about_dr = EINA_FALSE;

   _glview_update_surface(obj);
   if (!sd->surface)
     {
        ERR("Failed to create a surface with the requested configuration.");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_glview_resize_policy_set(Eo *obj, Elm_Glview_Data *sd, Elm_GLView_Resize_Policy policy)
{
   if (policy == sd->scale_policy) return EINA_TRUE;
   switch (policy)
     {
      case ELM_GLVIEW_RESIZE_POLICY_RECREATE:
      case ELM_GLVIEW_RESIZE_POLICY_SCALE:
        sd->scale_policy = policy;
        _glview_update_surface(obj);
        elm_obj_glview_draw_request(obj);
        return EINA_TRUE;

      default:
        ERR("Invalid Scale Policy.");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_glview_render_policy_set(Eo *obj, Elm_Glview_Data *sd, Elm_GLView_Render_Policy policy)
{
   if ((policy != ELM_GLVIEW_RENDER_POLICY_ON_DEMAND) &&
       (policy != ELM_GLVIEW_RENDER_POLICY_ALWAYS))
     {
        ERR("Invalid Render Policy.");
        return EINA_FALSE;
     }

   if (sd->render_policy == policy) return EINA_TRUE;

   sd->warned_about_dr = EINA_FALSE;
   sd->render_policy = policy;
   _set_render_policy_callback(obj);

   return EINA_TRUE;
}

EOLIAN static void
_elm_glview_efl_gfx_view_view_size_set(Eo *obj, Elm_Glview_Data *sd, Eina_Size2D sz)
{
   if ((sz.w == sd->w) && (sz.h == sd->h)) return;

   sd->w = sz.w;
   sd->h = sz.h;

   _glview_update_surface(obj);
   elm_obj_glview_draw_request(obj);
}

EOLIAN static Eina_Size2D
_elm_glview_efl_gfx_view_view_size_get(const Eo *obj EINA_UNUSED, Elm_Glview_Data *sd)
{
   return EINA_SIZE2D(sd->w, sd->h);
}

EOLIAN static void
_elm_glview_draw_request(Eo *obj, Elm_Glview_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_image_pixels_dirty_set(wd->resize_obj, EINA_TRUE);
   evas_object_image_data_update_add(wd->resize_obj, 0, 0, sd->w, sd->h);
   if (sd->render_policy == ELM_GLVIEW_RENDER_POLICY_ALWAYS &&
       !sd->render_idle_enterer)
     sd->render_idle_enterer = efl_event_callback_priority_add(efl_main_loop_get(),
                                                              EFL_LOOP_EVENT_IDLE_ENTER,
                                                              EFL_CALLBACK_PRIORITY_BEFORE,
                                                              _render_cb, obj);
}

EOLIAN static Evas_GL *
_elm_glview_evas_gl_get(const Eo *obj EINA_UNUSED, Elm_Glview_Data *sd)
{
   return sd->evasgl;
}

EOLIAN static int
_elm_glview_rotation_get(const Eo *obj EINA_UNUSED, Elm_Glview_Data *sd)
{
   return evas_gl_rotation_get(sd->evasgl);
}

static void
_elm_glview_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Legacy deprecated functions */
EAPI void
elm_glview_changed_set(Evas_Object *obj)
{
   ELM_GLVIEW_CHECK(obj);

   elm_obj_glview_draw_request(obj);
}

EAPI void
elm_glview_size_get(const Elm_Glview *obj, int *w, int *h)
{
   Eina_Size2D sz;
   sz = efl_gfx_view_size_get(obj);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

EAPI void
elm_glview_size_set(Elm_Glview *obj, int w, int h)
{
   efl_gfx_view_size_set(obj, EINA_SIZE2D(w, h));
}

EAPI void
elm_glview_init_func_set(Elm_Glview *obj, Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   ELM_GLVIEW_DATA_GET(obj, sd);

   sd->initialized = EINA_FALSE;
   sd->init_func = func;
}

EAPI void
elm_glview_del_func_set(Elm_Glview *obj, Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   ELM_GLVIEW_DATA_GET(obj, sd);

   sd->del_func = func;
}

EAPI void
elm_glview_resize_func_set(Elm_Glview *obj, Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   ELM_GLVIEW_DATA_GET(obj, sd);

   sd->resize_func = func;
}

EAPI void
elm_glview_render_func_set(Elm_Glview *obj, Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   ELM_GLVIEW_DATA_GET(obj, sd);

   sd->render_func = func;
   _set_render_policy_callback(obj);
}

/* Internal EO APIs and hidden overrides */

#define ELM_GLVIEW_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_glview)

#include "elm_glview.eo.c"
