#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_icon.h"

#ifdef ELM_EFREET
#define NON_EXISTING (void *)-1
static const char *icon_theme = NULL;
#endif

EAPI const char ELM_ICON_SMART_NAME[] = "elm_icon";

#ifdef HAVE_ELEMENTARY_ETHUMB
static Eina_List *_elm_icon_retry = NULL;
static int _icon_pending_request = 0;
#endif

static const char SIG_THUMB_DONE[] = "thumb,done";
static const char SIG_THUMB_ERROR[] = "thumb,error";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_THUMB_DONE, ""},
   {SIG_THUMB_ERROR, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_ICON_SMART_NAME, _elm_icon, Elm_Icon_Smart_Class,
  Elm_Image_Smart_Class, elm_image_smart_class_get, _smart_callbacks);

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

#ifdef HAVE_ELEMENTARY_ETHUMB
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
            (ELM_WIDGET_DATA(sd)->obj, sd->thumb.thumb.path,
            sd->thumb.thumb.key);

        sd->is_video = EINA_FALSE;
     }

   if (!ret)
     ret = elm_image_file_set
         (ELM_WIDGET_DATA(sd)->obj, sd->thumb.thumb.path, sd->thumb.thumb.key);

   if (ret)
     evas_object_smart_callback_call
       (ELM_WIDGET_DATA(sd)->obj, SIG_THUMB_DONE, NULL);
   else
     evas_object_smart_callback_call
       (ELM_WIDGET_DATA(sd)->obj, SIG_THUMB_ERROR, NULL);

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
     EINA_LIST_FREE (_elm_icon_retry, sd)
       _icon_thumb_stop(sd, ethumbd);
}

static void
_icon_thumb_finish(Elm_Icon_Smart_Data *sd,
                   Ethumb_Client *ethumbd)
{
   const char *file = NULL, *group = NULL;
   Eina_Bool ret;

   elm_image_file_get(ELM_WIDGET_DATA(sd)->obj, &file, &group);
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
        elm_image_file_set(ELM_WIDGET_DATA(sd)->obj, file, group);
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

   evas_object_smart_callback_call(data, SIG_THUMB_ERROR, NULL);

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

   min_size = _icon_size_min_get(ELM_WIDGET_DATA(sd)->obj);
   ethumb_client_size_set(ethumbd, min_size, min_size);

   sd->thumb.request = ethumb_client_thumb_async_get
       (ethumbd, _icon_thumb_done, _icon_thumb_error, sd);
}

static Eina_Bool
_icon_thumb_apply_cb(void *data,
                     int type __UNUSED__,
                     void *ev __UNUSED__)
{
   Elm_Icon_Smart_Data *sd = data;

   _icon_thumb_apply(sd);

   return ECORE_CALLBACK_RENEW;
}

#endif

static Eina_Bool
#ifdef ELM_EFREET
_icon_freedesktop_set(Evas_Object *obj,
                      const char *name,
                      int size)
#else
_icon_freedesktop_set(Evas_Object * obj __UNUSED__,
                      const char *name __UNUSED__,
                      int size __UNUSED__)
#endif
{
#ifdef ELM_EFREET
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
#endif
   return EINA_FALSE;
}

static void
_elm_icon_smart_sizing_eval(Evas_Object *obj)
{
   int w, h;

   ELM_ICON_DATA_GET(obj, sd);

   if (sd->in_eval) return;

   sd->in_eval++;
   elm_image_object_size_get(obj, &w, &h);

#ifdef ELM_EFREET
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
#endif

   _elm_icon_parent_sc->sizing_eval(obj);

   sd->in_eval--;
}

static void
_edje_signals_free(Elm_Icon_Smart_Data *sd)
{
   Edje_Signal_Data *esd;

   EINA_LIST_FREE (sd->edje_signals, esd)
     {
        eina_stringshare_del(esd->emission);
        eina_stringshare_del(esd->source);
        free(esd);
     }
}

static Eina_Bool
_elm_icon_smart_file_set(Evas_Object *obj,
                         const char *file,
                         const char *key)
{
   Evas_Object *pclip;

   ELM_ICON_DATA_GET(obj, sd);

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);

   _edje_signals_free(sd);

#ifdef ELM_EFREET
   if (!sd->freedesktop.use)
     {
        if (sd->stdicon) eina_stringshare_del(sd->stdicon);
        sd->stdicon = NULL;
     }
#endif

   if (!sd->is_video) return _elm_icon_parent_sc->file_set(obj, file, key);

   /* parent's edje file setting path replicated here (we got .eet
    * extension, so bypassing it) */
   if (ELM_IMAGE_DATA(sd)->prev_img)
     evas_object_del(ELM_IMAGE_DATA(sd)->prev_img);
   ELM_IMAGE_DATA(sd)->prev_img = NULL;

   if (!ELM_IMAGE_DATA(sd)->edje)
     {
        pclip = evas_object_clip_get(ELM_IMAGE_DATA(sd)->img);
        if (ELM_IMAGE_DATA(sd)->img) evas_object_del(ELM_IMAGE_DATA(sd)->img);

        /* Edje object instead */
        ELM_IMAGE_DATA(sd)->img = edje_object_add(evas_object_evas_get(obj));
        evas_object_smart_member_add(ELM_IMAGE_DATA(sd)->img, obj);
        if (ELM_IMAGE_DATA(sd)->show)
          evas_object_show(ELM_IMAGE_DATA(sd)->img);
        evas_object_clip_set(ELM_IMAGE_DATA(sd)->img, pclip);
     }

   ELM_IMAGE_DATA(sd)->edje = EINA_TRUE;
   if (!edje_object_file_set(ELM_IMAGE_DATA(sd)->img, file, key))
     {
        ERR("failed to set edje file '%s', group '%s': %s", file, key,
            edje_load_error_str
              (edje_object_load_error_get(ELM_IMAGE_DATA(sd)->img)));
        return EINA_FALSE;
     }

   evas_object_move(ELM_IMAGE_DATA(sd)->img, ELM_IMAGE_DATA(sd)->img_x,
                    ELM_IMAGE_DATA(sd)->img_y);
   evas_object_resize(ELM_IMAGE_DATA(sd)->img, ELM_IMAGE_DATA(sd)->img_w,
                      ELM_IMAGE_DATA(sd)->img_h);

   return EINA_TRUE;
}

static Eina_Bool
_elm_icon_smart_memfile_set(Evas_Object *obj,
                            const void *img,
                            size_t size,
                            const char *format,
                            const char *key)
{
   ELM_ICON_DATA_GET(obj, sd);

   Eina_Bool ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(img, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!size, EINA_FALSE);
   eina_stringshare_del(sd->stdicon);
   sd->stdicon = NULL;

   _edje_signals_free(sd);

   ret = _elm_icon_parent_sc->memfile_set(obj, img, size, format, key);

   return ret;
}

static Eina_Bool
_elm_icon_smart_theme(Evas_Object *obj)
{
   ELM_ICON_DATA_GET(obj, sd);

   if (sd->stdicon)
     _elm_theme_object_icon_set(obj, sd->stdicon, elm_widget_style_get(obj));

   if (!ELM_WIDGET_CLASS(_elm_icon_parent_sc)->theme(obj))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_icon_standard_set(Evas_Object *obj,
                   const char *name)
{
   ELM_ICON_DATA_GET(obj, sd);

   if (_elm_theme_object_icon_set(obj, name, "default"))
     {
#ifdef ELM_EFREET
        /* TODO: elm_unneed_efreet() */
        sd->freedesktop.use = EINA_FALSE;
#endif
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool
#ifdef ELM_EFREET
_icon_file_set(Elm_Icon_Smart_Data *sd,
               Evas_Object *obj,
               const char *path)
#else
_icon_file_set(Elm_Icon_Smart_Data * sd __UNUSED__,
               Evas_Object * obj,
               const char *path)
#endif
{
   if (elm_image_file_set(obj, path, NULL))
     {
#ifdef ELM_EFREET
        /* TODO: elm_unneed_efreet() */
        sd->freedesktop.use = EINA_FALSE;
#endif
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
   Eina_Bool ret;

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
        _elm_icon_smart_sizing_eval(obj);
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
                             Evas *e __UNUSED__,
                             Evas_Object *obj,
                             void *event_info __UNUSED__)
{
   Elm_Icon_Smart_Data *sd = data;
   const char *refup = eina_stringshare_ref(sd->stdicon);
   Eina_Bool fdo = EINA_FALSE;

   if (!_elm_icon_standard_set(obj, sd->stdicon, &fdo) || (!fdo))
     evas_object_event_callback_del_full
       (obj, EVAS_CALLBACK_RESIZE, _elm_icon_standard_resize_cb, sd);
   eina_stringshare_del(refup);
}

#ifdef HAVE_ELEMENTARY_ETHUMB
static void
_elm_icon_thumb_resize_cb(void *data,
                          Evas *e __UNUSED__,
                          Evas_Object *obj,
                          void *event_info __UNUSED__)
{
   Elm_Icon_Smart_Data *sd = data;

   if (sd->thumb.file.path)
     elm_icon_thumb_set(obj, sd->thumb.file.path, sd->thumb.file.key);
}

#endif

static void
_elm_icon_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Icon_Smart_Data);

   ELM_WIDGET_CLASS(_elm_icon_parent_sc)->base.add(obj);

   priv->lookup_order = ELM_ICON_LOOKUP_THEME_FDO;

#ifdef HAVE_ELEMENTARY_ETHUMB
   priv->thumb.request = NULL;
#endif
}

static void
_elm_icon_smart_del(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ETHUMB
   Ethumb_Client *ethumbd;
#endif

   ELM_ICON_DATA_GET(obj, sd);

   if (sd->stdicon) eina_stringshare_del(sd->stdicon);

#ifdef HAVE_ELEMENTARY_ETHUMB
   ethumbd = elm_thumb_ethumb_client_get();
   _icon_thumb_stop(sd, ethumbd);

   eina_stringshare_del(sd->thumb.file.path);
   eina_stringshare_del(sd->thumb.file.key);
   eina_stringshare_del(sd->thumb.thumb.path);
   eina_stringshare_del(sd->thumb.thumb.key);

   if (sd->thumb.eeh)
     ecore_event_handler_del(sd->thumb.eeh);
#endif

   _edje_signals_free(sd);

   ELM_WIDGET_CLASS(_elm_icon_parent_sc)->base.del(obj);
}

/* WARNING: to be deprecated */
void
_elm_icon_signal_emit(Evas_Object *obj,
                      const char *emission,
                      const char *source)
{
   ELM_ICON_DATA_GET(obj, sd);

   if (!ELM_IMAGE_DATA(sd)->edje) return;

   edje_object_signal_emit(ELM_IMAGE_DATA(sd)->img, emission, source);
}

static void
_edje_signal_callback(void *data,
                      Evas_Object *obj __UNUSED__,
                      const char *emission,
                      const char *source)
{
   Edje_Signal_Data *esd = data;

   esd->func(esd->data, esd->obj, emission, source);
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

   if (!ELM_IMAGE_DATA(sd)->edje) return;

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
     (ELM_IMAGE_DATA(sd)->img, emission, source, _edje_signal_callback, esd);
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

   if (!ELM_IMAGE_DATA(sd)->edje) return NULL;

   EINA_LIST_FOREACH(sd->edje_signals, l, esd)
     {
        if ((esd->func == func_cb) && (!strcmp(esd->emission, emission)) &&
            (!strcmp(esd->source, source)))
          {
             sd->edje_signals = eina_list_remove_list(sd->edje_signals, l);
             eina_stringshare_del(esd->emission);
             eina_stringshare_del(esd->source);
             data = esd->data;
             free(esd);

             edje_object_signal_callback_del_full
               (ELM_IMAGE_DATA(sd)->img, emission, source,
               _edje_signal_callback, esd);

             return data; /* stop at 1st match */
          }
     }

   return data;
}

static void
_elm_icon_smart_set_user(Elm_Icon_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_icon_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_icon_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_icon_smart_theme;

   ELM_IMAGE_CLASS(sc)->file_set = _elm_icon_smart_file_set;
   ELM_IMAGE_CLASS(sc)->memfile_set = _elm_icon_smart_memfile_set;
   ELM_IMAGE_CLASS(sc)->sizing_eval = _elm_icon_smart_sizing_eval;
}

EAPI const Elm_Icon_Smart_Class *
elm_icon_smart_class_get(void)
{
   static Elm_Icon_Smart_Class _sc =
     ELM_ICON_SMART_CLASS_INIT_NAME_VERSION(ELM_ICON_SMART_NAME);
   static const Elm_Icon_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_icon_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_icon_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_icon_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Eina_Bool
elm_icon_memfile_set(Evas_Object *obj,
                     const void *img,
                     size_t size,
                     const char *format,
                     const char *key)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   return elm_image_memfile_set(obj, img, size, format, key);
}

EAPI Eina_Bool
elm_icon_file_set(Evas_Object *obj,
                  const char *file,
                  const char *group)
{
   ELM_ICON_CHECK(obj) EINA_FALSE;

   return elm_image_file_set(obj, file, group);
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

#ifdef HAVE_ELEMENTARY_ETHUMB
   ELM_ICON_DATA_GET(obj, sd);

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _elm_icon_standard_resize_cb, sd);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _elm_icon_thumb_resize_cb, sd);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _elm_icon_thumb_resize_cb, sd);

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
            (ELM_ECORE_EVENT_ETHUMB_CONNECT, _icon_thumb_apply_cb, sd);
     }
#else
   (void)obj;
   (void)file;
   (void)group;
#endif
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
   ELM_ICON_DATA_GET(obj, sd);

   Eina_Bool fdo = EINA_FALSE;
   Eina_Bool ret;

   if (!name) return EINA_FALSE;

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _elm_icon_standard_resize_cb, sd);

   ret = _elm_icon_standard_set(obj, name, &fdo);

   if (fdo)
     evas_object_event_callback_add
       (obj, EVAS_CALLBACK_RESIZE, _elm_icon_standard_resize_cb, sd);

   return ret;
}

EAPI const char *
elm_icon_standard_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) NULL;
   ELM_ICON_DATA_GET(obj, sd);

   return sd->stdicon;
}

EAPI void
elm_icon_order_lookup_set(Evas_Object *obj,
                          Elm_Icon_Lookup_Order order)
{
   ELM_ICON_CHECK(obj);
   ELM_ICON_DATA_GET(obj, sd);

   sd->lookup_order = order;
}

EAPI Elm_Icon_Lookup_Order
elm_icon_order_lookup_get(const Evas_Object *obj)
{
   ELM_ICON_CHECK(obj) ELM_ICON_LOOKUP_THEME_FDO;
   ELM_ICON_DATA_GET(obj, sd);

   return sd->lookup_order;
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
