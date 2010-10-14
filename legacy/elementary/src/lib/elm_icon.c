#include <Elementary.h>
#include "elm_priv.h"

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
 * clicked - This is called when a user has clicked the icon
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *img;
   const char *stdicon;
   Elm_Icon_Lookup_Order lookup_order;
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

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);

static Eina_Bool _icon_standard_set(Widget_Data *wd, Evas_Object *obj, const char *name);
static Eina_Bool _icon_freedesktop_set(Widget_Data *wd, Evas_Object *obj, const char *name, int size);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->stdicon) eina_stringshare_del(wd->stdicon);
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
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   int w, h;

   _els_smart_icon_size_get(wd->img, &w, &h);
#ifdef ELM_EFREET
   if (wd->freedesktop.use && !((w - wd->freedesktop.requested_size) % 16))
     {
        /* This icon has been set to a freedesktop icon, and the requested
           appears to have a different size than the requested size, so try to
           request another, higher resolution, icon.
           FIXME: Find a better heuristic to determine if there should be
           an icon with a different resolution. */
        if (!_icon_freedesktop_set(wd, obj, wd->stdicon, w))
          wd->freedesktop.use = EINA_TRUE;
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
   evas_object_smart_callback_call(data, "clicked", event_info);
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

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "icon");
   elm_widget_type_set(obj, "icon");
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->lookup_order = ELM_ICON_LOOKUP_FDO_THEME;
   wd->img = _els_smart_icon_add(e);
   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_MOUSE_UP,
				  _mouse_up, obj);
   evas_object_repeat_events_set(wd->img, EINA_TRUE);
   elm_widget_resize_object_set(obj, wd->img);

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

   if ((!wd) || (!file)) return EINA_FALSE;
   if (wd->stdicon) eina_stringshare_del(wd->stdicon);
   wd->stdicon = NULL;
   if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
     ret = _els_smart_icon_file_edje_set(wd->img, file, group);
   else
     ret = _els_smart_icon_file_key_set(wd->img, file, group);
   _sizing_eval(obj);
   return ret;
}

static Eina_Bool
_icon_standard_set(Widget_Data *wd, Evas_Object *obj, const char *name)
{
   if (_elm_theme_object_icon_set(obj, wd->img, name, "default"))
     {
#ifdef ELM_EFREET
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
   char *path;

   elm_need_efreet();
   path = efreet_icon_path_find(getenv("E_ICON_THEME"), name, size);
   if (!path)
     {
        const char **itr;
        static const char *themes[] = {
           "default", "highcolor", "hicolor", "gnome", "Human", "oxygen", NULL
        };
        for (itr = themes; !path && *itr; itr++)
          path = efreet_icon_path_find(*itr, name, size);
     }
   if ((wd->freedesktop.use = !!path))
     {
        wd->freedesktop.requested_size = size;
        elm_icon_file_set(obj, path, NULL);
        free(path);
        return EINA_TRUE;
     }
#endif
   return EINA_FALSE;
}

/**
 * Set the theme, as standard, for a icon
 *
 * @param obj The icon object
 * @param name The theme name
 *
 * @return (1 = success, 0 = error)
 *
 * @ingroup Icon
 */
EAPI Eina_Bool
elm_icon_standard_set(Evas_Object *obj, const char *name)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   char *tmp;
   Eina_Bool ret = EINA_FALSE;

   if ((!wd) || (!name)) return EINA_FALSE;

   /* try locating the icon using the specified lookup order */
   switch (wd->lookup_order)
   {
   case ELM_ICON_LOOKUP_FDO:
      ret = _icon_freedesktop_set(wd, obj, name, 48) ||
            _icon_freedesktop_set(wd, obj, name, 32);
      break;
   case ELM_ICON_LOOKUP_THEME:
      ret = _icon_standard_set(wd, obj, name);
      break;
   case ELM_ICON_LOOKUP_THEME_FDO:
      ret = _icon_standard_set(wd, obj, name) ||
            _icon_freedesktop_set(wd, obj, name, 48) ||
            _icon_freedesktop_set(wd, obj, name, 32);
      break;
   case ELM_ICON_LOOKUP_FDO_THEME:
   default:
      ret = _icon_freedesktop_set(wd, obj, name, 48) ||
            _icon_freedesktop_set(wd, obj, name, 32) ||
            _icon_standard_set(wd, obj, name);
      break;
   }

   if (ret)
     {
        eina_stringshare_replace(&wd->stdicon, name);
        _sizing_eval(obj);
        return EINA_TRUE;
     }

   /* if that fails, see if icon name is in the format size/name. if so,
      try locating a fallback without the size specification */
   if (!(tmp = strchr(name, '/'))) return EINA_FALSE;
   ++tmp;
   if (*tmp) return elm_icon_standard_set(obj, tmp);

   /* give up */
   return EINA_FALSE;
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
elm_icon_order_lookup_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_ICON_LOOKUP_FDO_THEME;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_ICON_LOOKUP_FDO_THEME;
   return wd->lookup_order;
}

/**
 * Set the smooth effect for a icon
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


