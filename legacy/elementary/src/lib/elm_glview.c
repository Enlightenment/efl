#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_glview.h"

EAPI const char ELM_GLVIEW_SMART_NAME[] = "elm_glview";

static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";

/* smart callbacks coming from elm glview objects: */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_FOCUSED, ""},
   {SIG_UNFOCUSED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_GLVIEW_SMART_NAME, _elm_glview, Elm_Glview_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, _smart_callbacks);

static Eina_Bool
_elm_glview_smart_on_focus(Evas_Object *obj)
{
   ELM_GLVIEW_DATA_GET(obj, sd);

   if (elm_widget_focus_get(obj))
     {
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_TRUE);
        evas_object_smart_callback_call(obj, SIG_FOCUSED, NULL);
     }
   else
     {
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_FALSE);
        evas_object_smart_callback_call(obj, SIG_UNFOCUSED, NULL);
     }

   return EINA_TRUE;
}

static void
_glview_update_surface(Evas_Object *obj)
{
   ELM_GLVIEW_DATA_GET(obj, sd);
   if (!sd) return;

   if (sd->surface)
     {
        evas_object_image_native_surface_set
          (ELM_WIDGET_DATA(sd)->resize_obj, NULL);
        evas_gl_surface_destroy(sd->evasgl, sd->surface);
        sd->surface = NULL;
     }

   evas_object_image_size_set(ELM_WIDGET_DATA(sd)->resize_obj, sd->w, sd->h);

   if (!sd->surface)
     {
        Evas_Native_Surface ns;

        sd->surface = evas_gl_surface_create
            (sd->evasgl, sd->config, sd->w, sd->h);
        evas_gl_native_surface_get(sd->evasgl, sd->surface, &ns);
        evas_object_image_native_surface_set
          (ELM_WIDGET_DATA(sd)->resize_obj, &ns);
        elm_glview_changed_set(obj);
     }
}

static void
_elm_glview_smart_resize(Evas_Object *obj,
                         Evas_Coord w,
                         Evas_Coord h)
{
   ELM_GLVIEW_DATA_GET(obj, sd);

   _elm_glview_parent_sc->base.resize(obj, w, h);

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

   switch (sd->render_policy)
     {
      case ELM_GLVIEW_RENDER_POLICY_ON_DEMAND:
        // Delete idle_enterer if it for some reason is around
        if (sd->render_idle_enterer)
          {
             ecore_idle_enterer_del(sd->render_idle_enterer);
             sd->render_idle_enterer = NULL;
          }

        // Set pixel getter callback
        evas_object_image_pixels_get_callback_set
          (ELM_WIDGET_DATA(sd)->resize_obj,
          (Evas_Object_Image_Pixels_Get_Cb)_render_cb,
          obj);
        break;

      case ELM_GLVIEW_RENDER_POLICY_ALWAYS:
        // Unset the pixel getter callback if set already
        evas_object_image_pixels_get_callback_set
          (ELM_WIDGET_DATA(sd)->resize_obj, NULL, NULL);

        break;

      default:
        ERR("Invalid Render Policy.\n");
        return;
     }
}

static void
_elm_glview_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Glview_Smart_Data);

   // Create image to render Evas_GL Surface
   ELM_WIDGET_DATA(priv)->resize_obj =
     evas_object_image_filled_add(evas_object_evas_get(obj));
   evas_object_image_size_set(ELM_WIDGET_DATA(priv)->resize_obj, 1, 1);

   _elm_glview_parent_sc->base.add(obj);

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
        evas_object_del(obj);

        evas_gl_free(priv->evasgl);
        priv->evasgl = NULL;
        return;
     }
   priv->config->color_format = EVAS_GL_RGB_888;

   // Initialize variables
   priv->mode = 0;
   priv->scale_policy = ELM_GLVIEW_RESIZE_POLICY_RECREATE;
   priv->render_policy = ELM_GLVIEW_RENDER_POLICY_ON_DEMAND;
   priv->surface = NULL;

   // Initialize it to (64,64)  (It's an arbitrary value)
   priv->w = 64;
   priv->h = 64;

   // Initialize the rest of the values
   priv->init_func = NULL;
   priv->del_func = NULL;
   priv->render_func = NULL;
   priv->render_idle_enterer = NULL;
   priv->initialized = EINA_FALSE;
   priv->resized = EINA_FALSE;

   // Create Context
   priv->context = evas_gl_context_create(priv->evasgl, NULL);
   if (!priv->context)
     {
        ERR("Error Creating an Evas_GL Context.\n");
        evas_object_del(obj);

        evas_gl_config_free(priv->config);
        evas_gl_free(priv->evasgl);
        priv->evasgl = NULL;
        return;
     }
}

static void
_elm_glview_smart_del(Evas_Object *obj)
{
   ELM_GLVIEW_DATA_GET(obj, sd);

   // Call delete func if it's registered
   if (sd->del_func)
     {
        evas_gl_make_current(sd->evasgl, sd->surface, sd->context);
        sd->del_func(obj);
     }

   if (sd->render_idle_enterer)
     ecore_idle_enterer_del(sd->render_idle_enterer);

   if (sd->surface) evas_gl_surface_destroy(sd->evasgl, sd->surface);
   if (sd->context) evas_gl_context_destroy(sd->evasgl, sd->context);
   if (sd->config) evas_gl_config_free(sd->config);
   if (sd->evasgl) evas_gl_free(sd->evasgl);

   _elm_glview_parent_sc->base.del(obj); /* handles freeing sd */
}

static void
_elm_glview_smart_set_user(Elm_Glview_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_glview_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_glview_smart_del;
   ELM_WIDGET_CLASS(sc)->base.resize = _elm_glview_smart_resize;

   ELM_WIDGET_CLASS(sc)->on_focus = _elm_glview_smart_on_focus;
}

EAPI const Elm_Glview_Smart_Class *
elm_glview_smart_class_get(void)
{
   static Elm_Glview_Smart_Class _sc =
     ELM_GLVIEW_SMART_CLASS_INIT_NAME_VERSION(ELM_GLVIEW_SMART_NAME);
   static const Elm_Glview_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_glview_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_glview_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_glview_smart_class_new(), parent);
   if (!obj) return NULL;

   ELM_GLVIEW_DATA_GET(obj, sd);

   if (!sd->evasgl)
     return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Evas_GL_API *
elm_glview_gl_api_get(const Evas_Object *obj)
{
   ELM_GLVIEW_CHECK(obj) NULL;
   ELM_GLVIEW_DATA_GET(obj, sd);

   return evas_gl_api_get(sd->evasgl);
}

EAPI Eina_Bool
elm_glview_mode_set(Evas_Object *obj,
                    Elm_GLView_Mode mode)
{
   ELM_GLVIEW_CHECK(obj) EINA_FALSE;
   ELM_GLVIEW_DATA_GET(obj, sd);

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
     evas_object_image_alpha_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_TRUE);
   else
     evas_object_image_alpha_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_FALSE);

   sd->mode = mode;

   elm_glview_changed_set(obj);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_glview_resize_policy_set(Evas_Object *obj,
                             Elm_GLView_Resize_Policy policy)
{
   ELM_GLVIEW_CHECK(obj) EINA_FALSE;
   ELM_GLVIEW_DATA_GET(obj, sd);

   if (policy == sd->scale_policy) return EINA_TRUE;
   switch (policy)
     {
      case ELM_GLVIEW_RESIZE_POLICY_RECREATE:
      case ELM_GLVIEW_RESIZE_POLICY_SCALE:
        sd->scale_policy = policy;
        _glview_update_surface(obj);
        elm_glview_changed_set(obj);
        return EINA_TRUE;

      default:
        ERR("Invalid Scale Policy.\n");
        return EINA_FALSE;
     }
}

EAPI Eina_Bool
elm_glview_render_policy_set(Evas_Object *obj,
                             Elm_GLView_Render_Policy policy)
{
   ELM_GLVIEW_CHECK(obj) EINA_FALSE;
   ELM_GLVIEW_DATA_GET(obj, sd);

   if ((policy != ELM_GLVIEW_RENDER_POLICY_ON_DEMAND) &&
       (policy != ELM_GLVIEW_RENDER_POLICY_ALWAYS))
     {
        ERR("Invalid Render Policy.\n");
        return EINA_FALSE;
     }

   if (sd->render_policy == policy) return EINA_TRUE;

   sd->render_policy = policy;
   _set_render_policy_callback(obj);
   _glview_update_surface(obj);

   return EINA_TRUE;
}

EAPI void
elm_glview_size_set(Evas_Object *obj,
                    int w,
                    int h)
{
   ELM_GLVIEW_CHECK(obj);
   ELM_GLVIEW_DATA_GET(obj, sd);

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
   ELM_GLVIEW_DATA_GET(obj, sd);

   if (w) *w = sd->w;
   if (h) *h = sd->h;
}

EAPI void
elm_glview_init_func_set(Evas_Object *obj,
                         Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   ELM_GLVIEW_DATA_GET(obj, sd);

   sd->initialized = EINA_FALSE;
   sd->init_func = func;
}

EAPI void
elm_glview_del_func_set(Evas_Object *obj,
                        Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   ELM_GLVIEW_DATA_GET(obj, sd);

   sd->del_func = func;
}

EAPI void
elm_glview_resize_func_set(Evas_Object *obj,
                           Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   ELM_GLVIEW_DATA_GET(obj, sd);

   sd->resize_func = func;
}

EAPI void
elm_glview_render_func_set(Evas_Object *obj,
                           Elm_GLView_Func_Cb func)
{
   ELM_GLVIEW_CHECK(obj);
   ELM_GLVIEW_DATA_GET(obj, sd);

   sd->render_func = func;
   _set_render_policy_callback(obj);
}

EAPI void
elm_glview_changed_set(Evas_Object *obj)
{
   ELM_GLVIEW_CHECK(obj);
   ELM_GLVIEW_DATA_GET(obj, sd);

   evas_object_image_pixels_dirty_set
     (ELM_WIDGET_DATA(sd)->resize_obj, EINA_TRUE);
   if (sd->render_policy == ELM_GLVIEW_RENDER_POLICY_ALWAYS &&
       !sd->render_idle_enterer)
     sd->render_idle_enterer =
       ecore_idle_enterer_before_add((Ecore_Task_Cb)_render_cb, obj);
}
