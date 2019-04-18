#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_icon.h"
#include "efl_ui_widget_image.h"
#include "elm_icon_eo.h"

#define NON_EXISTING (void *)-1

#define MY_CLASS ELM_ICON_CLASS
#define MY_CLASS_NAME "Elm_Icon"
#define MY_CLASS_NAME_LEGACY "elm_icon"

static Eina_List *_elm_icon_retry = NULL;
static int _icon_pending_request = 0;

static const char SIG_THUMB_DONE[] = "thumb,done";
static const char SIG_THUMB_ERROR[] = "thumb,error";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_THUMB_DONE, ""},
   {SIG_THUMB_ERROR, ""},
   {NULL, NULL}
};

/* FIXME: move this code to ecore */
#ifdef _WIN32
static Eina_Bool
_path_is_absolute(const char *path)
{
   //TODO: Check if this works with all absolute paths in windows
   return (isalpha(*path)) && (*(path + 1) == ':') &&
          ((*(path + 2) == '\\') || (*(path + 2) == '/'));
}

#else
static Eina_Bool
_path_is_absolute(const char *path)
{
   return *path == '/';
}

#endif

static inline int
_icon_size_min_get(Evas_Object *icon)
{
   int w, h;

   evas_object_geometry_get(icon, NULL, NULL, &w, &h);

   return MAX(16, MIN(w, h));
}

static void
_icon_thumb_stop(Elm_Icon_Data *sd,
                 void *ethumbd)
{
   if (sd->thumb.request)
     {
        ethumb_client_thumb_async_cancel(ethumbd, sd->thumb.request);
        sd->thumb.request = NULL;
        _icon_pending_request--;
     }

   if (sd->thumb.retry)
     {
        _elm_icon_retry = eina_list_remove(_elm_icon_retry, sd);
        sd->thumb.retry = EINA_FALSE;
     }
}

static Eina_Bool
_icon_thumb_display(Elm_Icon_Data *sd)
{
   Eina_Bool ret = EINA_FALSE;

   if (sd->thumb.format == ETHUMB_THUMB_EET)
     {
        int prefix_size;
        const char **ext, *ptr;
        static const char *extensions[] =
        {
           ".avi", ".mp4", ".ogv", ".mov", ".mpg", ".wmv", NULL
        };

        prefix_size = eina_stringshare_strlen(sd->thumb.file.path) - 4;
        if (prefix_size >= 0)
          {
             ptr = sd->thumb.file.path + prefix_size;
             for (ext = extensions; *ext; ++ext)
               if (!strcasecmp(ptr, *ext))
                 {
                    sd->is_video = EINA_TRUE;
                    break;
                 }
          }

        ret = elm_image_file_set
            (sd->obj, sd->thumb.thumb.path,
            sd->thumb.thumb.key);

        sd->is_video = EINA_FALSE;
     }

   if (!ret)
     ret = elm_image_file_set
         (sd->obj, sd->thumb.thumb.path, sd->thumb.thumb.key);

   if (ret)
     efl_event_callback_legacy_call
       (sd->obj, ELM_ICON_EVENT_THUMB_DONE, NULL);
   else
     efl_event_callback_legacy_call
       (sd->obj, ELM_ICON_EVENT_THUMB_ERROR, NULL);

   return ret;
}

static Eina_Bool
_icon_thumb_retry(Elm_Icon_Data *sd)
{
   return _icon_thumb_display(sd);
}

static void
_icon_thumb_cleanup(Ethumb_Client *ethumbd)
{
   Eina_List *l, *ll;
   Elm_Icon_Data *sd;

   EINA_LIST_FOREACH_SAFE(_elm_icon_retry, l, ll, sd)
     if (_icon_thumb_retry(sd))
       {
          _elm_icon_retry = eina_list_remove_list(_elm_icon_retry, l);
          sd->thumb.retry = EINA_FALSE;
       }

   if (_icon_pending_request == 0)
     EINA_LIST_FREE(_elm_icon_retry, sd)
       _icon_thumb_stop(sd, ethumbd);
}

static void
_icon_thumb_finish(Elm_Icon_Data *sd,
                   Ethumb_Client *ethumbd)
{
   const char *file = NULL, *group = NULL;
   Eina_Bool ret = EINA_FALSE;

   elm_image_file_get(sd->obj, &file, &group);
   file = eina_stringshare_ref(file);
   group = eina_stringshare_ref(group);

   ret = _icon_thumb_display(sd);

   if (!ret && file)
     {
        if (!sd->thumb.retry)
          {
             _elm_icon_retry = eina_list_append(_elm_icon_retry, sd);
             sd->thumb.retry = EINA_TRUE;
          }

        /* Back to previous image */
        elm_image_file_set(sd->obj, file, group);
     }

   _icon_thumb_cleanup(ethumbd);

   eina_stringshare_del(file);
   eina_stringshare_del(group);
}

static void
_icon_thumb_done(Ethumb_Client *client,
                 const char *thumb_path,
                 const char *thumb_key,
                 void *data)
{
   Elm_Icon_Data *sd = data;

   if (EINA_UNLIKELY(!sd->thumb.request))
     {
        ERR("Something odd happened with a thumbnail request");
        return;
     }

   _icon_pending_request--;
   sd->thumb.request = NULL;

   eina_stringshare_replace(&sd->thumb.thumb.path, thumb_path);
   eina_stringshare_replace(&sd->thumb.thumb.key, thumb_key);
   sd->thumb.format = ethumb_client_format_get(client);

   _icon_thumb_finish(sd, client);
}

static void
_icon_thumb_error(Ethumb_Client *client,
                  void *data)
{
   Elm_Icon_Data *sd = data;

   if (EINA_UNLIKELY(!sd->thumb.request))
     {
        ERR("Something odd happened with a thumbnail request");
        return;
     }

   _icon_pending_request--;
   sd->thumb.request = NULL;

   ERR("could not generate thumbnail for %s (key: %s)",
       sd->thumb.file.path, sd->thumb.file.key);

   efl_event_callback_legacy_call(sd->obj, ELM_ICON_EVENT_THUMB_ERROR, NULL);

   _icon_thumb_cleanup(client);
}

static void
_icon_thumb_apply(Elm_Icon_Data *sd)
{
   Ethumb_Client *ethumbd;
   int min_size;

   ethumbd = elm_thumb_ethumb_client_get();

   _icon_thumb_stop(sd, ethumbd);

   if (!sd->thumb.file.path) return;

   _icon_pending_request++;
   if (!ethumb_client_file_set
         (ethumbd, sd->thumb.file.path, sd->thumb.file.key)) return;

   min_size = _icon_size_min_get(sd->obj);
   ethumb_client_size_set(ethumbd, min_size, min_size);

   sd->thumb.request = ethumb_client_thumb_async_get
       (ethumbd, _icon_thumb_done, _icon_thumb_error, sd);
}

static Eina_Bool
_icon_thumb_apply_cb(void *data,
                     int type EINA_UNUSED,
                     void *ev EINA_UNUSED)
{
   ELM_ICON_DATA_GET(data, sd);

   _icon_thumb_apply(sd);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_icon_freedesktop_set(Evas_Object *obj,
                      const char *theme,
                      const char *name,
                      int size)
{
   const char *path;

   ELM_ICON_DATA_GET(obj, sd);

   elm_need_efreet();
   if (!theme)
     theme = elm_config_icon_theme_get();

   path = efreet_icon_path_find(theme, name, size);
   sd->freedesktop.use = !!path;
   if (sd->freedesktop.use)
     {
        sd->freedesktop.requested_size = size;
        elm_image_file_set(obj, path, NULL);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_edje_signal_callback(void *data,
                      Evas_Object *obj EINA_UNUSED,
                      const char *emission,
                      const char *source)
{
   Edje_Signal_Data *esd = data;

   esd->func(esd->data, esd->obj, emission, source);
}

static void
_edje_signals_free(Elm_Icon_Data *sd)
{
   Edje_Signal_Data *esd;
   Efl_Ui_Image_Data *id = efl_data_scope_get(sd->obj, EFL_UI_IMAGE_CLASS);

   EINA_LIST_FREE(sd->edje_signals, esd)
     {
        edje_object_signal_callback_del_full
           (id->img, esd->emission, esd->source,
            _edje_signal_callback, esd);
        eina_stringshare_del(esd->emission);
        eina_stringshare_del(esd->source);
        free(esd);
     }
}

EOLIAN static Eina_Error
_elm_icon_efl_file_load(Eo *obj, Elm_Icon_Data *sd)
{
   Evas_Object *pclip;
   const char *key;
   Eina_Error err;

   if (efl_file_loaded_get(obj)) return 0;
   err = efl_file_load(efl_super(obj, MY_CLASS));
   if (err) return err;
   
   Efl_Ui_Image_Data *id = efl_data_scope_get(obj, EFL_UI_IMAGE_CLASS);

   _edje_signals_free(sd);

   if (!sd->freedesktop.use)
     ELM_SAFE_FREE(sd->stdicon, eina_stringshare_del);

   if (!sd->is_video) return 0;

   /* parent's edje file setting path replicated here (we got .eet
    * extension, so bypassing it) */
   ELM_SAFE_FREE(id->prev_img, evas_object_del);

   if (!id->edje)
     {
        pclip = evas_object_clip_get(id->img);
        evas_object_del(id->img);

        /* Edje object instead */
        id->img = edje_object_add(evas_object_evas_get(obj));
        evas_object_smart_member_add(id->img, obj);
        if (id->show)
          evas_object_show(id->img);
        evas_object_clip_set(id->img, pclip);
        id->edje = EINA_TRUE;
     }
   key = efl_file_key_get(obj);
   efl_file_key_set(id->img, key);
   err = efl_file_mmap_set(id->img, efl_file_mmap_get(obj));
   if (!err) err = efl_file_load(id->img);
   if (err)
     {
        ERR("failed to set edje file '%s', group '%s': %s", efl_file_get(id->img), key,
            edje_load_error_str
              (edje_object_load_error_get(id->img)));
        return err;
     }

   evas_object_geometry_set(id->img, id->img_x, id->img_y, id->img_w, id->img_h);

   return 0;
}

EOLIAN static Eina_Error
_elm_icon_efl_ui_widget_theme_apply(Eo *obj, Elm_Icon_Data *sd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   if (sd->stdicon)
     _elm_theme_object_icon_set(obj, sd->stdicon, elm_widget_style_get(obj));

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   return int_ret;
}

static Eina_Bool
_icon_standard_set(Evas_Object *obj,
                   const char *name)
{
   ELM_ICON_DATA_GET(obj, sd);

   if (_elm_theme_object_icon_set(obj, name, "default"))
     {
        /* TODO: elm_unneed_efreet() */
        sd->freedesktop.use = EINA_FALSE;
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool
_icon_file_set(Elm_Icon_Data *sd,
               Evas_Object *obj,
               const char *path)
{
   if (elm_image_file_set(obj, path, NULL))
     {
        /* TODO: elm_unneed_efreet() */
        sd->freedesktop.use = EINA_FALSE;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_internal_elm_icon_standard_set(Evas_Object *obj,
                       const char *name,
                       Eina_Bool *fdo)
{
   char *tmp;
   Eina_Bool ret = EINA_FALSE;

   ELM_ICON_DATA_GET(obj, sd);

   /* try locating the icon using the specified theme */
   if (!strcmp(ELM_CONFIG_ICON_THEME_ELEMENTARY, elm_config_icon_theme_get()))
     {
        ret = _icon_standard_set(obj, name);
        if (ret && fdo) *fdo = EINA_FALSE;

        if (!ret)
          {
             ret = _icon_freedesktop_set(obj, "hicolor", name, _icon_size_min_get(obj));
             if (ret && fdo) *fdo = EINA_TRUE;
          }
     }
   else
     {
        ret = _icon_freedesktop_set(obj, NULL, name, _icon_size_min_get(obj));
        if (ret && fdo) *fdo = EINA_TRUE;
     }

   if (ret)
     {
        eina_stringshare_replace(&sd->stdicon, name);
        _efl_ui_image_sizing_eval(obj);
        return EINA_TRUE;
     }

   if (_path_is_absolute(name))
     {
        if (fdo)
          *fdo = EINA_FALSE;
        return _icon_file_set(sd, obj, name);
     }

   /* if that fails, see if icon name is in the format size/name. if so,
      try locating a fallback without the size specification */
   if (!(tmp = strchr(name, '/'))) return EINA_FALSE;
   ++tmp;
   if (*tmp) return _internal_elm_icon_standard_set(obj, tmp, fdo);
   /* give up */
   return EINA_FALSE;
}

static void
_elm_icon_standard_resize_cb(void *data,
                             Evas *e EINA_UNUSED,
                             Evas_Object *obj,
                             void *event_info EINA_UNUSED)
{
   ELM_ICON_DATA_GET(data, sd);
   const char *refup = eina_stringshare_ref(sd->stdicon);
   Eina_Bool fdo = EINA_FALSE;

   if (!_internal_elm_icon_standard_set(obj, sd->stdicon, &fdo) || (!fdo))
     evas_object_event_callback_del_full
       (obj, EVAS_CALLBACK_RESIZE, _elm_icon_standard_resize_cb, data);
   eina_stringshare_del(refup);
}

static void
_elm_icon_thumb_resize_cb(void *data,
                          Evas *e EINA_UNUSED,
                          Evas_Object *obj,
                          void *event_info EINA_UNUSED)
{
   ELM_ICON_DATA_GET(data, sd);

   if (sd->thumb.file.path)
     elm_icon_thumb_set(obj, sd->thumb.file.path, sd->thumb.file.key);
}

EOLIAN static void
_elm_icon_efl_canvas_group_group_add(Eo *obj, Elm_Icon_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   priv->thumb.request = NULL;
}

EOLIAN static void
_elm_icon_efl_canvas_group_group_del(Eo *obj, Elm_Icon_Data *sd)
{
   eina_stringshare_del(sd->stdicon);

   if (sd->thumb.request)
     {
        Ethumb_Client *ethumbd = elm_thumb_ethumb_client_get();
        if (ethumbd) _icon_thumb_stop(sd, ethumbd);
     }

   eina_stringshare_del(sd->thumb.file.path);
   eina_stringshare_del(sd->thumb.file.key);
   eina_stringshare_del(sd->thumb.thumb.path);
   eina_stringshare_del(sd->thumb.thumb.key);
   ecore_event_handler_del(sd->thumb.eeh);

   _edje_signals_free(sd);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

/* WARNING: to be deprecated */
void
_elm_icon_signal_emit(Evas_Object *obj,
                      const char *emission,
                      const char *source)
{

   Efl_Ui_Image_Data *id = efl_data_scope_get(obj, EFL_UI_IMAGE_CLASS);

   if (!id->edje) return;

   edje_object_signal_emit(id->img, emission, source);
}

/* WARNING: to be deprecated */
void
_elm_icon_signal_callback_add(Evas_Object *obj,
                              const char *emission,
                              const char *source,
                              Edje_Signal_Cb func_cb,
                              void *data)
{
   Edje_Signal_Data *esd;

   ELM_ICON_DATA_GET(obj, sd);
   Efl_Ui_Image_Data *id = efl_data_scope_get(obj, EFL_UI_IMAGE_CLASS);

   if (!id->edje) return;

   esd = ELM_NEW(Edje_Signal_Data);
   if (!esd) return;

   esd->obj = obj;
   esd->func = func_cb;
   esd->emission = eina_stringshare_add(emission);
   esd->source = eina_stringshare_add(source);
   esd->data = data;
   sd->edje_signals =
     eina_list_append(sd->edje_signals, esd);

   edje_object_signal_callback_add
     (id->img, emission, source, _edje_signal_callback, esd);
}

/* WARNING: to be deprecated */
void *
_elm_icon_signal_callback_del(Evas_Object *obj,
                              const char *emission,
                              const char *source,
                              Edje_Signal_Cb func_cb)
{
   Edje_Signal_Data *esd = NULL;
   void *data = NULL;
   Eina_List *l;

   ELM_ICON_DATA_GET(obj, sd);
   Efl_Ui_Image_Data *id = efl_data_scope_get(obj, EFL_UI_IMAGE_CLASS);

   if (!id->edje) return NULL;

   EINA_LIST_FOREACH(sd->edje_signals, l, esd)
     {
        if ((esd->func == func_cb) && (!strcmp(esd->emission, emission)) &&
            (!strcmp(esd->source, source)))
          {
             sd->edje_signals = eina_list_remove_list(sd->edje_signals, l);
             eina_stringshare_del(esd->emission);
             eina_stringshare_del(esd->source);
             data = esd->data;

             edje_object_signal_callback_del_full
               (id->img, emission, source,
               _edje_signal_callback, esd);

             free(esd);

             return data; /* stop at 1st match */
          }
     }

   return data;
}

EAPI Evas_Object *
elm_icon_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_icon_efl_object_constructor(Eo *obj, Elm_Icon_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   sd->obj = obj;

   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_ICON);

   return obj;
}

static void
_elm_icon_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Legacy deprecated functions */
EAPI Eina_Bool
elm_icon_memfile_set(Evas_Object *obj,
                     const void *img,
                     size_t size,
                     const char *format,
                     const char *key)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(img, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!size, EINA_FALSE);

   ELM_ICON_DATA_GET(obj, sd);
   ELM_SAFE_FREE(sd->stdicon, eina_stringshare_del);

   _edje_signals_free(sd);

   return elm_image_memfile_set(efl_super(obj, MY_CLASS), img, size, format, key);
}

EAPI Eina_Bool
elm_icon_file_set(Evas_Object *obj,
                  const char *file,
                  const char *group)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);

   return efl_file_simple_load(obj, file, group);
}

EAPI void
elm_icon_file_get(const Evas_Object *obj,
                  const char **file,
                  const char **group)
{
   ELM_ICON_CHECK(obj);

   elm_image_file_get(obj, file, group);
}

EAPI Eina_Bool
elm_icon_animated_available_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   return elm_image_animated_available_get(obj);
}

EAPI void
elm_icon_animated_set(Evas_Object *obj,
                      Eina_Bool anim)
{
   ELM_ICON_CHECK(obj);

   return elm_image_animated_set(obj, anim);
}

EAPI Eina_Bool
elm_icon_animated_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   return elm_image_animated_get(obj);
}

EAPI void
elm_icon_animated_play_set(Evas_Object *obj,
                           Eina_Bool play)
{
   ELM_ICON_CHECK(obj);

   elm_image_animated_play_set(obj, play);
}

EAPI Eina_Bool
elm_icon_animated_play_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   return elm_image_animated_play_get(obj);
}

EAPI void
elm_icon_smooth_set(Evas_Object *obj,
                    Eina_Bool smooth)
{
   ELM_ICON_CHECK(obj);

   elm_image_smooth_set(obj, smooth);
}

EAPI Eina_Bool
elm_icon_smooth_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   return elm_image_smooth_get(obj);
}

EAPI void
elm_icon_no_scale_set(Evas_Object *obj,
                      Eina_Bool no_scale)
{
   ELM_ICON_CHECK(obj);

   elm_image_no_scale_set(obj, no_scale);
}

EAPI Eina_Bool
elm_icon_no_scale_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   return elm_image_no_scale_get(obj);
}

EAPI void
elm_icon_resizable_set(Evas_Object *obj,
                       Eina_Bool size_up,
                       Eina_Bool size_down)
{
   ELM_ICON_CHECK(obj);

   elm_image_resizable_set(obj, size_up, size_down);
}

EAPI void
elm_icon_resizable_get(const Evas_Object *obj,
                       Eina_Bool *size_up,
                       Eina_Bool *size_down)
{
   ELM_ICON_CHECK(obj);

   elm_image_resizable_get(obj, size_up, size_down);
}

EAPI void
elm_icon_fill_outside_set(Evas_Object *obj,
                          Eina_Bool fill_outside)
{
   ELM_ICON_CHECK(obj);

   elm_image_fill_outside_set(obj, fill_outside);
}

EAPI Eina_Bool
elm_icon_fill_outside_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   return elm_image_fill_outside_get(obj);
}

EAPI void
elm_icon_size_get(const Evas_Object *obj,
                  int *w,
                  int *h)
{
   ELM_ICON_CHECK(obj);

   elm_image_object_size_get(obj, w, h);
}

EAPI void
elm_icon_prescale_set(Evas_Object *obj,
                      int size)
{
   ELM_ICON_CHECK(obj);

   elm_image_prescale_set(obj, size);
}

EAPI int
elm_icon_prescale_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) 0;

   return elm_image_prescale_get(obj);
}

EAPI Evas_Object *
elm_icon_object_get(Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) 0;

   return elm_image_object_get(obj);
}

EAPI void
elm_icon_preload_disabled_set(Evas_Object *obj,
                              Eina_Bool disabled)
{
   ELM_ICON_CHECK(obj);

   elm_image_preload_disabled_set(obj, disabled);
}

EAPI void
elm_icon_aspect_fixed_set(Evas_Object *obj,
                          Eina_Bool fixed)
{
   ELM_ICON_CHECK(obj);

   elm_image_aspect_fixed_set(obj, fixed);
}

EAPI Eina_Bool
elm_icon_aspect_fixed_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   return elm_image_aspect_fixed_get(obj);
}

EAPI void
elm_icon_thumb_set(Evas_Object *obj, const char *file, const char *group)
{
   ELM_ICON_CHECK(obj);
   ELM_ICON_DATA_GET(obj, sd);

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _elm_icon_standard_resize_cb, obj);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _elm_icon_thumb_resize_cb, obj);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _elm_icon_thumb_resize_cb, obj);

   eina_stringshare_replace(&sd->thumb.file.path, file);
   eina_stringshare_replace(&sd->thumb.file.key, group);

   if (elm_thumb_ethumb_client_connected_get())
     {
        _icon_thumb_apply(sd);
        return;
     }

   if (!sd->thumb.eeh)
     {
        sd->thumb.eeh = ecore_event_handler_add
            (ELM_ECORE_EVENT_ETHUMB_CONNECT, _icon_thumb_apply_cb, obj);
     }
}

EAPI Eina_Bool
elm_icon_standard_set(Evas_Object *obj, const char *name)
{
   Eina_Bool fdo = EINA_FALSE;

   ELM_ICON_CHECK(obj) EINA_FALSE;

   if (!name) return EINA_FALSE;

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _elm_icon_standard_resize_cb, obj);

   Eina_Bool int_ret = _internal_elm_icon_standard_set(obj, name, &fdo);

   if (fdo)
     evas_object_event_callback_add
       (obj, EVAS_CALLBACK_RESIZE, _elm_icon_standard_resize_cb, obj);

   return int_ret;
}

EAPI const char*
elm_icon_standard_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) NULL;
   ELM_ICON_DATA_GET(obj, sd);

   return sd->stdicon;
}

EAPI void
elm_icon_order_lookup_set(Evas_Object *obj EINA_UNUSED,
                           Elm_Icon_Lookup_Order order EINA_UNUSED)
{
   // this method's behaviour has been overridden by elm_config_icon_theme_set
}

EAPI Elm_Icon_Lookup_Order
elm_icon_order_lookup_get(const Evas_Object *obj EINA_UNUSED)
{
   return ELM_ICON_LOOKUP_FDO_THEME;
}

/* Internal EO APIs and hidden overrides */

#define ELM_ICON_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_icon)

#include "elm_icon_eo.c"
