#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_icon.h"
#include "elm_widget_image.h"

#define NON_EXISTING (void *)-1
static const char *icon_theme = NULL;

EAPI Eo_Op ELM_OBJ_ICON_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_ICON_CLASS
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
   int size;

   elm_image_object_size_get(icon, &size, NULL);

   return (size < 16) ? 16 : size;
}

static void
_icon_thumb_stop(Elm_Icon_Smart_Data *sd,
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
_icon_thumb_display(Elm_Icon_Smart_Data *sd)
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
     evas_object_smart_callback_call
       (sd->obj, SIG_THUMB_DONE, NULL);
   else
     evas_object_smart_callback_call
       (sd->obj, SIG_THUMB_ERROR, NULL);

   return ret;
}

static Eina_Bool
_icon_thumb_retry(Elm_Icon_Smart_Data *sd)
{
   return _icon_thumb_display(sd);
}

static void
_icon_thumb_cleanup(Ethumb_Client *ethumbd)
{
   Eina_List *l, *ll;
   Elm_Icon_Smart_Data *sd;

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
_icon_thumb_finish(Elm_Icon_Smart_Data *sd,
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
   Elm_Icon_Smart_Data *sd = data;

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
   Elm_Icon_Smart_Data *sd = data;

   if (EINA_UNLIKELY(!sd->thumb.request))
     {
        ERR("Something odd happened with a thumbnail request");
        return;
     }

   _icon_pending_request--;
   sd->thumb.request = NULL;

   ERR("could not generate thumbnail for %s (key: %s)",
       sd->thumb.file.path, sd->thumb.file.key);

   evas_object_smart_callback_call(sd->obj, SIG_THUMB_ERROR, NULL);

   _icon_thumb_cleanup(client);
}

static void
_icon_thumb_apply(Elm_Icon_Smart_Data *sd)
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
                      const char *name,
                      int size)
{
   const char *path;

   ELM_ICON_DATA_GET(obj, sd);

   elm_need_efreet();

   if (icon_theme == NON_EXISTING) return EINA_FALSE;

   if (!icon_theme)
     {
        Efreet_Icon_Theme *theme;
        /* TODO: Listen for EFREET_EVENT_ICON_CACHE_UPDATE */
        theme = efreet_icon_theme_find(getenv("E_ICON_THEME"));
        if (!theme)
          {
             const char **itr;
             static const char *themes[] = {
                "gnome", "Human", "oxygen", "hicolor", NULL
             };
             for (itr = themes; *itr; itr++)
               {
                  theme = efreet_icon_theme_find(*itr);
                  if (theme) break;
               }
          }

        if (!theme)
          {
             icon_theme = NON_EXISTING;
             return EINA_FALSE;
          }
        else
          icon_theme = eina_stringshare_add(theme->name.internal);
     }
   path = efreet_icon_path_find(icon_theme, name, size);
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
_elm_icon_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   int w, h;

   Elm_Icon_Smart_Data *sd = _pd;

   if (sd->in_eval) return;

   sd->in_eval++;
   elm_image_object_size_get(obj, &w, &h);

   if (sd->freedesktop.use && sd->stdicon)
     {
        int size;
        /* This icon has been set to a freedesktop icon, and the requested
           appears to have a different size than the requested size, so try to
           request another, higher resolution, icon.
           FIXME: Find a better heuristic to determine if there should be
           an icon with a different resolution. */
        size = ((w / 16) + 1) * 16;
        _icon_freedesktop_set(obj, sd->stdicon, size);
     }

   eo_do_super(obj, MY_CLASS, elm_obj_image_sizing_eval());

   sd->in_eval--;
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
_edje_signals_free(Elm_Icon_Smart_Data *sd)
{
   Edje_Signal_Data *esd;
   Elm_Image_Data *id = eo_data_scope_get(sd->obj, ELM_OBJ_IMAGE_CLASS);

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

static void
_elm_icon_smart_file_set(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *pclip;

   Elm_Icon_Smart_Data *sd = _pd;
   Elm_Image_Data *id = eo_data_scope_get(obj, ELM_OBJ_IMAGE_CLASS);

   const char *file = va_arg(*list, const char *);
   const char *key = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN(file);

   _edje_signals_free(sd);

   if (!sd->freedesktop.use)
     ELM_SAFE_FREE(sd->stdicon, eina_stringshare_del);

   if (!sd->is_video)
     {
        eo_do_super(obj, MY_CLASS, elm_obj_image_file_set(file, key, &int_ret));
        if (ret) *ret = int_ret;
        return;
     }

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
     }

   id->edje = EINA_TRUE;
   if (!edje_object_file_set(id->img, file, key))
     {
        ERR("failed to set edje file '%s', group '%s': %s", file, key,
            edje_load_error_str
              (edje_object_load_error_get(id->img)));
        return;
     }

   evas_object_move(id->img, id->img_x,
                    id->img_y);
   evas_object_resize(id->img, id->img_w,
                      id->img_h);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_icon_smart_memfile_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Icon_Smart_Data *sd = _pd;

   const void *img = va_arg(*list, const void *);
   size_t size = va_arg(*list, size_t);
   const char *format = va_arg(*list, const char *);
   const char *key = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN(img);
   EINA_SAFETY_ON_TRUE_RETURN(!size);
   ELM_SAFE_FREE(sd->stdicon, eina_stringshare_del);

   _edje_signals_free(sd);

   eo_do_super(obj, MY_CLASS, elm_obj_image_memfile_set(img, size, format, key, &int_ret));
   if (ret) *ret = int_ret;
}

static void
_elm_icon_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Elm_Icon_Smart_Data *sd = _pd;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   if (sd->stdicon)
     _elm_theme_object_icon_set(obj, sd->stdicon, elm_widget_style_get(obj));

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   if (ret) *ret = EINA_TRUE;
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
_icon_file_set(Elm_Icon_Smart_Data *sd,
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
_elm_icon_standard_set(Evas_Object *obj,
                       const char *name,
                       Eina_Bool *fdo)
{
   char *tmp;
   Eina_Bool ret = EINA_FALSE;

   ELM_ICON_DATA_GET(obj, sd);

   /* try locating the icon using the specified lookup order */
   switch (sd->lookup_order)
     {
      case ELM_ICON_LOOKUP_FDO:
        ret = _icon_freedesktop_set(obj, name, _icon_size_min_get(obj));
        if (ret && fdo) *fdo = EINA_TRUE;
        break;

      case ELM_ICON_LOOKUP_THEME:
        ret = _icon_standard_set(obj, name);
        break;

      case ELM_ICON_LOOKUP_THEME_FDO:
        ret = _icon_standard_set(obj, name);
        if (!ret)
          {
             ret = _icon_freedesktop_set(obj, name, _icon_size_min_get(obj));
             if (ret && fdo) *fdo = EINA_TRUE;
          }
        break;

      case ELM_ICON_LOOKUP_FDO_THEME:
      default:
        ret = _icon_freedesktop_set(obj, name, _icon_size_min_get(obj));
        if (!ret)
          ret = _icon_standard_set(obj, name);
        else if (fdo)
          *fdo = EINA_TRUE;
        break;
     }

   if (ret)
     {
        eina_stringshare_replace(&sd->stdicon, name);
        eo_do(obj, elm_obj_image_sizing_eval());
        return EINA_TRUE;
     }

   if (_path_is_absolute(name))
     return _icon_file_set(sd, obj, name);

   /* if that fails, see if icon name is in the format size/name. if so,
      try locating a fallback without the size specification */
   if (!(tmp = strchr(name, '/'))) return EINA_FALSE;
   ++tmp;
   if (*tmp) return elm_icon_standard_set(obj, tmp);
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

   if (!_elm_icon_standard_set(obj, sd->stdicon, &fdo) || (!fdo))
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

static void
_elm_icon_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Icon_Smart_Data *priv = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->lookup_order = ELM_ICON_LOOKUP_THEME_FDO;

   priv->thumb.request = NULL;
}

static void
_elm_icon_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Icon_Smart_Data *sd = _pd;

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

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

/* WARNING: to be deprecated */
void
_elm_icon_signal_emit(Evas_Object *obj,
                      const char *emission,
                      const char *source)
{

   Elm_Image_Data *id = eo_data_scope_get(obj, ELM_OBJ_IMAGE_CLASS);

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
   Elm_Image_Data *id = eo_data_scope_get(obj, ELM_OBJ_IMAGE_CLASS);

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
   Elm_Image_Data *id = eo_data_scope_get(obj, ELM_OBJ_IMAGE_CLASS);

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
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Icon_Smart_Data *sd = _pd;
   sd->obj = obj;

   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI Eina_Bool
elm_icon_memfile_set(Evas_Object *obj,
                     const void *img,
                     size_t size,
                     const char *format,
                     const char *key)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_image_memfile_set(img, size, format, key, &ret));
   return ret;
}

EAPI Eina_Bool
elm_icon_file_set(Evas_Object *obj,
                  const char *file,
                  const char *group)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_image_file_set(file, group, &ret));
   return ret;
}

EAPI void
elm_icon_file_get(const Evas_Object *obj,
                  const char **file,
                  const char **group)
{
   ELM_ICON_CHECK(obj);

   elm_image_file_get(obj, file, group);
}

EAPI void
elm_icon_thumb_set(Evas_Object *obj,
                   const char *file,
                   const char *group)
{
   ELM_ICON_CHECK(obj);
   eo_do(obj, elm_obj_icon_thumb_set(file, group));
}

static void
_thumb_set(Eo *obj, void *_pd, va_list *list)
{
   const char *file = va_arg(*list, const char *);
   const char *group = va_arg(*list, const char *);
   Elm_Icon_Smart_Data *sd = _pd;

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

EAPI Eina_Bool
elm_icon_standard_set(Evas_Object *obj,
                      const char *name)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_icon_standard_set(name, &ret));
   return ret;
}

static void
_standard_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool fdo = EINA_FALSE;
   const char *name = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   if (!name)
     {
        if (ret) *ret = EINA_FALSE;
        return;
     }

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _elm_icon_standard_resize_cb, obj);

   Eina_Bool int_ret = _elm_icon_standard_set(obj, name, &fdo);

   if (fdo)
     evas_object_event_callback_add
       (obj, EVAS_CALLBACK_RESIZE, _elm_icon_standard_resize_cb, obj);

   if (ret) *ret = int_ret;
}

EAPI const char *
elm_icon_standard_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) NULL;

   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_icon_standard_get(&ret));
   return ret;
}

static void
_standard_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{

   Elm_Icon_Smart_Data *sd = _pd;
   const char **ret = va_arg(*list, const char **);

   *ret = sd->stdicon;
}

EAPI void
elm_icon_order_lookup_set(Evas_Object *obj,
                          Elm_Icon_Lookup_Order order)
{
   ELM_ICON_CHECK(obj);

   eo_do(obj, elm_obj_icon_order_lookup_set(order));
}

static void
_order_lookup_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{

   Elm_Icon_Smart_Data *sd = _pd;
   Elm_Icon_Lookup_Order order = va_arg(*list, Elm_Icon_Lookup_Order);

   sd->lookup_order = order;
}

EAPI Elm_Icon_Lookup_Order
elm_icon_order_lookup_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) ELM_ICON_LOOKUP_THEME_FDO;
   Elm_Icon_Lookup_Order ret = ELM_ICON_LOOKUP_THEME_FDO;
   eo_do((Eo *) obj, elm_obj_icon_order_lookup_get(&ret));
   return ret;
}

static void
_order_lookup_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{

   Elm_Icon_Smart_Data *sd = _pd;
   Elm_Icon_Lookup_Order *ret = va_arg(*list, Elm_Icon_Lookup_Order *);

   *ret = sd->lookup_order;
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

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_icon_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_icon_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_icon_smart_theme),

        EO_OP_FUNC(ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_FILE_SET), _elm_icon_smart_file_set),
        EO_OP_FUNC(ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_MEMFILE_SET), _elm_icon_smart_memfile_set),
        EO_OP_FUNC(ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_SIZING_EVAL), _elm_icon_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_THUMB_SET), _thumb_set),
        EO_OP_FUNC(ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_STANDARD_SET), _standard_set),
        EO_OP_FUNC(ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_STANDARD_GET), _standard_get),
        EO_OP_FUNC(ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_SET), _order_lookup_set),
        EO_OP_FUNC(ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_GET), _order_lookup_get),

        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_ICON_SUB_ID_THUMB_SET, "Set the file that will be used, but use a generated thumbnail."),
     EO_OP_DESCRIPTION(ELM_OBJ_ICON_SUB_ID_STANDARD_SET, "Set the icon by icon standards names."),
     EO_OP_DESCRIPTION(ELM_OBJ_ICON_SUB_ID_STANDARD_GET, "Get the icon name set by icon standard names."),
     EO_OP_DESCRIPTION(ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_SET, "Sets the icon lookup order used by elm_icon_standard_set()."),
     EO_OP_DESCRIPTION(ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_GET, "Get the icon lookup order."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_ICON_BASE_ID, op_desc, ELM_OBJ_ICON_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Icon_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_icon_class_get, &class_desc, ELM_OBJ_IMAGE_CLASS, NULL);
