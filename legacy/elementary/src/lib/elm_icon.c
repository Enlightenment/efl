#include <Elementary.h>
#include "elm_priv.h"

#ifdef ELM_EFREET
#define NON_EXISTING (void *)-1
static const char *icon_theme = NULL;
#endif

/**
 * @defgroup Icon Icon
 *
 * A standard icon that may be provided by the theme (delete, edit,
 * arrows etc.) or a custom file (PNG, JPG, EDJE etc.) used for an
 * icon. The Icon may scale or not and of course... support alpha
 * channels.
 *
 * Signals that you can add callbacks for are:
 *
 * "clicked" - This is called when a user has clicked the icon
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *img;
   const char *stdicon;
   Elm_Icon_Lookup_Order lookup_order;

#ifdef HAVE_ELEMENTARY_ETHUMB
   struct {
      int id;

      struct {
         const char *path;
         const char *key;
      } file, thumb;

      Ethumb_Exists *exists;

      Ecore_Event_Handler *eeh;

      Ethumb_Thumb_Format format;

      Eina_Bool retry : 1;
   } thumb;
#endif

#ifdef ELM_EFREET
   struct {
        int requested_size;
        Eina_Bool use : 1;
   } freedesktop;
#endif
   Eina_Bool scale_up : 1;
   Eina_Bool scale_down : 1;
   Eina_Bool smooth : 1;
   Eina_Bool fill_outside : 1;
   Eina_Bool no_scale : 1;
};

#ifdef HAVE_ELEMENTARY_ETHUMB
static Eina_List *_elm_icon_retry = NULL;
static int _icon_pending_request = 0;

static void _icon_thumb_exists(Ethumb_Client *client __UNUSED__, Ethumb_Exists *thread, Eina_Bool exists, void *data);
#endif

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);

static Eina_Bool _icon_standard_set(Widget_Data *wd, Evas_Object *obj, const char *name);
static Eina_Bool _icon_freedesktop_set(Widget_Data *wd, Evas_Object *obj, const char *name, int size);

static const char SIG_CLICKED[] = "clicked";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CLICKED, ""},
   {NULL, NULL}
};


//FIXME: move this code to ecore
#ifdef _WIN32
static Eina_Bool
_path_is_absolute(const char *path)
{
   //TODO: Check if this works with all absolute paths in windows
   return ((isalpha (*path)) && (*(path + 1) == ':') && ((*(path + 2) == '\\') || (*(path + 2) == '/')));
}
#else
static Eina_Bool
_path_is_absolute(const char *path)
{
   return  (*path == '/');
}
#endif

#ifdef HAVE_ELEMENTARY_ETHUMB
static void
_icon_thumb_stop(Widget_Data *wd, void *ethumbd)
{
   if (wd->thumb.id > 0)
     {
        ethumb_client_generate_cancel(ethumbd, wd->thumb.id, NULL, NULL, NULL);
        wd->thumb.id = -1;
        _icon_pending_request--;
     }

   if (wd->thumb.exists)
     {
        ethumb_client_thumb_exists_cancel(wd->thumb.exists, _icon_thumb_exists, wd);
        wd->thumb.exists = NULL;
        _icon_pending_request--;
     }

   if (wd->thumb.retry)
     {
        _elm_icon_retry = eina_list_remove(_elm_icon_retry, wd);
        wd->thumb.retry = EINA_FALSE;
     }
}

static Eina_Bool
_icon_thumb_display(Widget_Data *wd)
{
   Eina_Bool ret = EINA_FALSE;

   if (wd->thumb.format == ETHUMB_THUMB_EET)
     {
        static const char *extensions[] = {
          ".avi", ".mp4", ".ogv", ".mov", ".mpg", ".wmv", NULL
        };
        const char **ext, *ptr;
        int prefix_size;
        Eina_Bool video = EINA_FALSE;

        prefix_size = eina_stringshare_strlen(wd->thumb.file.path) - 4;
        if (prefix_size >= 0)
          {
             ptr = wd->thumb.file.path + prefix_size;
             for (ext = extensions; *ext; ++ext)
               if (!strcasecmp(ptr, *ext))
                 {
                    video = EINA_TRUE;
                    break;
                 }
          }

        if (video)
          ret = _els_smart_icon_file_edje_set(wd->img, wd->thumb.thumb.path, "movie/thumb");
     }

   if (!ret)
     ret = _els_smart_icon_file_key_set(wd->img, wd->thumb.thumb.path, wd->thumb.thumb.key);

   return ret;
}

static Eina_Bool
_icon_thumb_retry(Widget_Data *wd)
{
   return _icon_thumb_display(wd);
}

static void
_icon_thumb_cleanup(Ethumb_Client *ethumbd)
{
   Eina_List *l, *ll;
   Widget_Data *wd;

   EINA_LIST_FOREACH_SAFE(_elm_icon_retry, l, ll, wd)
     if (_icon_thumb_retry(wd))
       {
          _elm_icon_retry = eina_list_remove_list(_elm_icon_retry, l);
          wd->thumb.retry = EINA_FALSE;
       }

   if (_icon_pending_request == 0)
     EINA_LIST_FREE(_elm_icon_retry, wd)
       _icon_thumb_stop(wd, ethumbd);
}

static void
_icon_thumb_finish(Widget_Data *wd, Ethumb_Client *ethumbd)
{
   const char *file = NULL, *group = NULL;
   Eina_Bool ret;

   _els_smart_icon_file_get(wd->img, &file, &group);
   file = eina_stringshare_ref(file);
   group = eina_stringshare_ref(group);

   ret = _icon_thumb_display(wd);

   if (!ret && file)
     {
        const char *p;

        if (!wd->thumb.retry)
          {
             _elm_icon_retry = eina_list_append(_elm_icon_retry, wd);
             wd->thumb.retry = EINA_TRUE;
          }

        /* Back to previous image */
        if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
          _els_smart_icon_file_edje_set(wd->img, file, group);
        else
          _els_smart_icon_file_key_set(wd->img, file, group);
     }

   _icon_thumb_cleanup(ethumbd);

   eina_stringshare_del(file);
   eina_stringshare_del(group);
}

static void
_icon_thumb_cb(void *data,
               Ethumb_Client *ethumbd,
               int id,
               const char *file __UNUSED__,
               const char *key __UNUSED__,
               const char *thumb_path,
               const char *thumb_key,
               Eina_Bool success)
{
   Widget_Data *wd = data;

   EINA_SAFETY_ON_FALSE_RETURN(wd->thumb.id == id);
   wd->thumb.id = -1;

   _icon_pending_request--;

   if (success)
     {
        eina_stringshare_replace(&wd->thumb.thumb.path, thumb_path);
        eina_stringshare_replace(&wd->thumb.thumb.key, thumb_key);
        wd->thumb.format = ethumb_client_format_get(ethumbd);

        _icon_thumb_finish(wd, ethumbd);
     }
   else
     {
        ERR("could not generate thumbnail for %s (key: %s)", file, key);
        _icon_thumb_cleanup(ethumbd);
     }
}

static void
_icon_thumb_exists(Ethumb_Client *client __UNUSED__, Ethumb_Exists *thread, Eina_Bool exists, void *data)
{
   Widget_Data *wd = data;
   Ethumb_Client *ethumbd;

   if (ethumb_client_thumb_exists_check(thread))
     return ;

   wd->thumb.exists = NULL;

   ethumbd = elm_thumb_ethumb_client_get();

   if (exists)
     {
        const char *thumb_path, *thumb_key;

        _icon_pending_request--;
        ethumb_client_thumb_path_get(ethumbd, &thumb_path, &thumb_key);
        eina_stringshare_replace(&wd->thumb.thumb.path, thumb_path);
        eina_stringshare_replace(&wd->thumb.thumb.key, thumb_key);
        wd->thumb.format = ethumb_client_format_get(ethumbd);

        _icon_thumb_finish(wd, ethumbd);
     }
   else if ((wd->thumb.id = ethumb_client_generate(ethumbd, _icon_thumb_cb, wd, NULL)) == -1)
     {
        /* Failed to generate thumbnail */
        _icon_pending_request--;
     }
}

static void
_icon_thumb_apply(Widget_Data *wd)
{
   Ethumb_Client *ethumbd;

   ethumbd = elm_thumb_ethumb_client_get();

   _icon_thumb_stop(wd, ethumbd);

   if (!wd->thumb.file.path) return ;

   _icon_pending_request++;
   ethumb_client_file_set(ethumbd, wd->thumb.file.path, wd->thumb.file.key);
   ethumb_client_size_set(ethumbd, 32, 32);
   wd->thumb.exists = ethumb_client_thumb_exists(ethumbd, _icon_thumb_exists, wd);
}

static Eina_Bool
_icon_thumb_apply_cb(void *data, int type __UNUSED__, void *ev __UNUSED__)
{
   Widget_Data *wd = data;

   _icon_thumb_apply(wd);
   return ECORE_CALLBACK_RENEW;
}
#endif

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
#ifdef HAVE_ELEMENTARY_ETHUMB
   Ethumb_Client *ethumbd;
#endif

   if (!wd) return;
   if (wd->stdicon) eina_stringshare_del(wd->stdicon);

#ifdef HAVE_ELEMENTARY_ETHUMB
   ethumbd = elm_thumb_ethumb_client_get();
   _icon_thumb_stop(wd, ethumbd);

   eina_stringshare_del(wd->thumb.file.path);
   eina_stringshare_del(wd->thumb.file.key);
   eina_stringshare_del(wd->thumb.thumb.path);
   eina_stringshare_del(wd->thumb.thumb.key);

   if (wd->thumb.eeh)
     ecore_event_handler_del(wd->thumb.eeh);
#endif

   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->stdicon)
     _elm_theme_object_icon_set(obj, wd->img, wd->stdicon, elm_widget_style_get(obj));
   _sizing_eval(obj);
}

static void
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Object *icon_edje;
   icon_edje = _els_smart_icon_edje_get(wd->img);
   if (!icon_edje) return;
   edje_object_signal_emit(icon_edje, emission, source);
}

static void
_signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Object *icon_edje;
   icon_edje = _els_smart_icon_edje_get(wd->img);
   if (!icon_edje) return;
   edje_object_signal_callback_add(icon_edje, emission, source, func_cb, data);
}

static void
_signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Object *icon_edje;
   icon_edje = _els_smart_icon_edje_get(wd->img);
   if (!icon_edje) return;
   edje_object_signal_callback_del_full(icon_edje, emission, source, func_cb,
                                        data);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   int w, h;

   _els_smart_icon_size_get(wd->img, &w, &h);
#ifdef ELM_EFREET
   if ((wd->freedesktop.use) && (!((w - wd->freedesktop.requested_size) % 16)))
     {
        /* This icon has been set to a freedesktop icon, and the requested
           appears to have a different size than the requested size, so try to
           request another, higher resolution, icon.
FIXME: Find a better heuristic to determine if there should be
an icon with a different resolution. */
        _icon_freedesktop_set(wd, obj, wd->stdicon, w);
     }
#endif
   _els_smart_icon_scale_up_set(wd->img, wd->scale_up);
   _els_smart_icon_scale_down_set(wd->img, wd->scale_down);
   _els_smart_icon_smooth_scale_set(wd->img, wd->smooth);
   _els_smart_icon_fill_inside_set(wd->img, !(wd->fill_outside));
   if (wd->no_scale) _els_smart_icon_scale_set(wd->img, 1.0);
   else
     {
        _els_smart_icon_scale_set(wd->img, elm_widget_scale_get(obj) *
                                  _elm_config->scale);
        _els_smart_icon_size_get(wd->img, &w, &h);
     }
   if (!wd->scale_down)
     {
        minw = w;
        minh = h;
     }
   if (!wd->scale_up)
     {
        maxw = w;
        maxh = h;
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   evas_object_smart_callback_call(data, SIG_CLICKED, event_info);
}

/**
 * Add a new icon to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Icon
 */
EAPI Evas_Object *
elm_icon_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "icon");
   elm_widget_type_set(obj, "icon");
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);

   wd->lookup_order = ELM_ICON_LOOKUP_THEME_FDO;
   wd->img = _els_smart_icon_add(e);
   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, obj);
   evas_object_repeat_events_set(wd->img, EINA_TRUE);
   elm_widget_resize_object_set(obj, wd->img);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

#ifdef HAVE_ELEMENTARY_ETHUMB
   wd->thumb.id = -1;
#endif

   wd->smooth = EINA_TRUE;
   wd->scale_up = EINA_TRUE;
   wd->scale_down = EINA_TRUE;

   _sizing_eval(obj);
   return obj;
}

/**
 * Set the file that will be used as icon
 *
 * @param obj The icon object
 * @param file The path to file that will be used as icon
 * @param group The group that the icon belongs in edje file
 *
 * @return (1 = success, 0 = error)
 *
 * @ingroup Icon
 */
EAPI Eina_Bool
elm_icon_file_set(Evas_Object *obj, const char *file, const char *group)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Bool ret;
   const char *p;

   if (!wd) return EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);
   if (wd->stdicon) eina_stringshare_del(wd->stdicon);
   wd->stdicon = NULL;
   if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
     ret = _els_smart_icon_file_edje_set(wd->img, file, group);
   else
     ret = _els_smart_icon_file_key_set(wd->img, file, group);
   _sizing_eval(obj);
   return ret;
}

/**
 * Get the file that will be used as icon
 *
 * @param obj The icon object
 * @param file The path to file that will be used as icon
 * @param group The group that the icon belongs in edje file
 *
 * @ingroup Icon
 */
EAPI void
elm_icon_file_get(const Evas_Object *obj, const char **file, const char **group)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _els_smart_icon_file_get(wd->img, file, group);
}

EAPI void
elm_icon_thumb_set(const Evas_Object *obj, const char *file, const char *group)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

#ifdef HAVE_ELEMENTARY_ETHUMB
   eina_stringshare_replace(&wd->thumb.file.path, file);
   eina_stringshare_replace(&wd->thumb.file.key, group);

   if (elm_thumb_ethumb_client_connected())
     {
        _icon_thumb_apply(wd);
        return ;
     }

   if (!wd->thumb.eeh)
     {
        wd->thumb.eeh = ecore_event_handler_add(ELM_ECORE_EVENT_ETHUMB_CONNECT, _icon_thumb_apply_cb, wd);
     }
#else
   (void) obj;
   (void) file;
   (void) group;
#endif
}

static Eina_Bool
_icon_standard_set(Widget_Data *wd, Evas_Object *obj, const char *name)
{
   if (_elm_theme_object_icon_set(obj, wd->img, name, "default"))
     {
#ifdef ELM_EFREET
        /* TODO: elm_unneed_efreet() */
        wd->freedesktop.use = EINA_FALSE;
#endif
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_icon_file_set(Widget_Data *wd, Evas_Object *obj, const char *path)
{
   if (elm_icon_file_set(obj, path, NULL))
     {
#ifdef ELM_EFREET
        /* TODO: elm_unneed_efreet() */
        wd->freedesktop.use = EINA_FALSE;
#endif
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_icon_freedesktop_set(Widget_Data *wd, Evas_Object *obj, const char *name, int size)
{
#ifdef ELM_EFREET
   const char *path;

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
   wd->freedesktop.use = !!path;
   if (wd->freedesktop.use)
     {
        wd->freedesktop.requested_size = size;
        elm_icon_file_set(obj, path, NULL);
        return EINA_TRUE;
     }
#endif
   return EINA_FALSE;
}

static inline int
_icon_size_min_get(Evas_Object *icon)
{
   int size;
   _els_smart_icon_size_get(icon, &size, NULL);
   return (size < 32) ? 32 : size;
}

/**
 * Set the theme, as standard, for an icon.
 * If theme was not found and it is the absolute path of an image file, this
 * image will be used.
 *
 * @param obj The icon object
 * @param name The theme name
 *
 * @return (1 = success, 0 = error)
 *
 * @ingroup Icon
 */
static Eina_Bool
_elm_icon_standard_set(Widget_Data *wd, Evas_Object *obj, const char *name)
{
   char *tmp;
   Eina_Bool ret;

   /* try locating the icon using the specified lookup order */
   switch (wd->lookup_order)
     {
      case ELM_ICON_LOOKUP_FDO:
         ret = _icon_freedesktop_set(wd, obj, name, _icon_size_min_get(wd->img));
         break;
      case ELM_ICON_LOOKUP_THEME:
         ret = _icon_standard_set(wd, obj, name);
         break;
      case ELM_ICON_LOOKUP_THEME_FDO:
         ret = _icon_standard_set(wd, obj, name) ||
            _icon_freedesktop_set(wd, obj, name, _icon_size_min_get(wd->img));
         break;
      case ELM_ICON_LOOKUP_FDO_THEME:
      default:
         ret = _icon_freedesktop_set(wd, obj, name, _icon_size_min_get(wd->img)) ||
            _icon_standard_set(wd, obj, name);
         break;
     }

   if (ret)
     {
        eina_stringshare_replace(&wd->stdicon, name);
        _sizing_eval(obj);
        return EINA_TRUE;
     }

   if (_path_is_absolute(name))
     return _icon_file_set(wd, obj, name);

   /* if that fails, see if icon name is in the format size/name. if so,
      try locating a fallback without the size specification */
   if (!(tmp = strchr(name, '/'))) return EINA_FALSE;
   ++tmp;
   if (*tmp) return elm_icon_standard_set(obj, tmp);
   /* give up */
   return EINA_FALSE;
}

static void
_elm_icon_standard_resize(void *data,
                          Evas *e __UNUSED__,
                          Evas_Object *obj,
                          void *event_info __UNUSED__)
{
   Widget_Data *wd = data;

   if (!_elm_icon_standard_set(wd, obj, wd->stdicon))
     evas_object_event_callback_del_full(obj, EVAS_CALLBACK_RESIZE,
                                         _elm_icon_standard_resize, wd);
}

EAPI Eina_Bool
elm_icon_standard_set(Evas_Object *obj, const char *name)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!name)) return EINA_FALSE;

   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_RESIZE,
                                       _elm_icon_standard_resize, wd);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE,
                                  _elm_icon_standard_resize, wd);

   return _elm_icon_standard_set(wd, obj, name);
}

/**
 * Get the theme, as standard, for an icon
 *
 * @param obj The icon object
 * @return The theme name
 *
 * @ingroup Icon
 */
EAPI const char *
elm_icon_standard_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->stdicon;
}

/**
 * Sets icon lookup order, used by elm_icon_standard_set().
 *
 * @param obj The icon object
 * @param order The icon lookup order
 *
 * @ingroup Icon
 */
EAPI void
elm_icon_order_lookup_set(Evas_Object *obj, Elm_Icon_Lookup_Order order)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd) wd->lookup_order = order;
}

/**
 * Gets the icon lookup order.
 *
 * @param obj The icon object
 * @return The icon lookup order
 *
 * @ingroup Icon
 */
EAPI Elm_Icon_Lookup_Order
elm_icon_order_lookup_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_ICON_LOOKUP_THEME_FDO;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_ICON_LOOKUP_THEME_FDO;
   return wd->lookup_order;
}

/**
 * Set the smooth effect for an icon
 *
 * @param obj The icon object
 * @param smooth A bool to set (or no) smooth effect
 * (1 = smooth, 0 = not smooth)
 *
 * @ingroup Icon
 */
EAPI void
elm_icon_smooth_set(Evas_Object *obj, Eina_Bool smooth)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->smooth = smooth;
   _sizing_eval(obj);
}

/**
 * Get the smooth effect for an icon
 *
 * @param obj The icon object
 * @return If setted smooth effect
 *
 * @ingroup Icon
 */
EAPI Eina_Bool
elm_icon_smooth_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return wd->smooth;
}

/**
 * Set if the object is scalable
 *
 * @param obj The icon object
 * @param no_scale A bool to set scale (or no)
 * (1 = no_scale, 0 = scale)
 *
 * @ingroup Icon
 */
EAPI void
elm_icon_no_scale_set(Evas_Object *obj, Eina_Bool no_scale)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->no_scale = no_scale;
   _sizing_eval(obj);
}

/**
 * Get if the object isn't scalable
 *
 * @param obj The icon object
 * @return If isn't scalable
 *
 * @ingroup Icon
 */
EAPI Eina_Bool
elm_icon_no_scale_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->no_scale;
}

/**
 * Set if the object is (up/down) scalable
 *
 * @param obj The icon object
 * @param scale_up A bool to set if the object is scalable up
 * @param scale_down A bool to set if the object is scalable down
 *
 * @ingroup Icon
 */
EAPI void
elm_icon_scale_set(Evas_Object *obj, Eina_Bool scale_up, Eina_Bool scale_down)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->scale_up = scale_up;
   wd->scale_down = scale_down;
   _sizing_eval(obj);
}

/**
 * Get if the object is (up/down) scalable
 *
 * @param obj The icon object
 * @param scale_up A bool to set if the object is scalable up
 * @param scale_down A bool to set if the object is scalable down
 *
 * @ingroup Icon
 */
EAPI void
elm_icon_scale_get(const Evas_Object *obj, Eina_Bool *scale_up, Eina_Bool *scale_down)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (scale_up) *scale_up = wd->scale_up;
   if (scale_down) *scale_down = wd->scale_down;
}

/**
 * Set if the object is filled outside
 *
 * @param obj The icon object
 * @param fill_outside A bool to set if the object is filled outside
 * (1 = filled, 0 = no filled)
 *
 * @ingroup Icon
 */
EAPI void
elm_icon_fill_outside_set(Evas_Object *obj, Eina_Bool fill_outside)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->fill_outside = fill_outside;
   _sizing_eval(obj);
}

/**
 * Get if the object is filled outside
 *
 * @param obj The icon object
 * @return If the object is filled outside
 *
 * @ingroup Icon
 */
EAPI Eina_Bool
elm_icon_fill_outside_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return wd->fill_outside;
}

/**
 * Set the prescale size for the icon
 *
 * @param obj The icon object
 * @param size The prescale size
 *
 * @ingroup Icon
 */
EAPI void
elm_icon_prescale_set(Evas_Object *obj, int size)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   _els_smart_icon_scale_size_set(wd->img, size);
}

/**
 * Get the prescale size for the icon
 *
 * @param obj The icon object
 * @return The prescale size
 *
 * @ingroup Icon
 */
EAPI int
elm_icon_prescale_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return 0;
   return _els_smart_icon_scale_size_get(wd->img);
}
