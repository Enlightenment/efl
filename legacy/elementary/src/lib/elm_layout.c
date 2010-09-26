#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Layout Layout
 *
 * This takes a standard Edje design file and wraps it very thinly
 * in a widget and handles swallowing widgets into swallow regions
 * in the Edje object, allowing Edje to be used as a design and
 * layout tool
 */

typedef struct _Widget_Data Widget_Data;
typedef struct _Subinfo Subinfo;

struct _Widget_Data
{
   Evas_Object *obj;
   Evas_Object *lay;
   Eina_List *subs;
   Eina_Bool needs_size_calc:1;
};

struct _Subinfo
{
   const char *swallow;
   Evas_Object *obj;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Widget_Data *wd);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   if (!wd) return;
   EINA_LIST_FREE(wd->subs, si)
     {
	eina_stringshare_del(si->swallow);
	free(si);
     }
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_scale_set(wd->lay, elm_widget_scale_get(obj) *
                         _elm_config->scale);
   _sizing_eval(wd);
}

static void
_changed_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->needs_size_calc)
     {
	_sizing_eval(wd);
	wd->needs_size_calc = 0;
     }
}

static void *
_elm_layout_list_data_get(const Eina_List *list)
{
   Subinfo *si = eina_list_data_get(list);
   return si->obj;
}

static Eina_Bool
_elm_layout_focus_cycle_hook(Evas_Object *obj, Elm_Focus_Direction dir, Eina_Bool circular)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *items;
   void *(*list_data_get) (const Eina_List *list);
   Evas_Object *last_focused;

   if ((!wd) || (!wd->subs))
     return EINA_FALSE;

   /* Focus chain (This block is diferent of elm_win cycle)*/
   /* TODO: Change this to use other chain */
   if (1)
     {
        items = wd->subs;
        list_data_get = _elm_layout_list_data_get;

        if (!items) return EINA_FALSE;
     }
   else
     {
        items = NULL;
        list_data_get = eina_list_data_get;

        if (!items) return EINA_FALSE;
     }

   last_focused = elm_widget_focus_cycle_next_get(obj, items,
                                                  list_data_get, dir,
                                                  circular);

   return !!last_focused;
}

static void
_sizing_eval(Widget_Data *wd)
{
   Evas_Coord minw = -1, minh = -1;
   edje_object_size_min_calc(wd->lay, &minw, &minh);
   evas_object_size_hint_min_set(wd->obj, minw, minh);
   evas_object_size_hint_max_set(wd->obj, -1, -1);
}

static void
_request_sizing_eval(Widget_Data *wd)
{
   if (wd->needs_size_calc) return;
   wd->needs_size_calc = 1;
   evas_object_smart_changed(wd->obj);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _request_sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   Eina_List *l;
   Subinfo *si;
   if (!wd) return;
   EINA_LIST_FOREACH(wd->subs, l, si)
     {
	if (si->obj == sub)
	  {
	     evas_object_event_callback_del_full(sub,
                                            EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints,
                                            wd);
	     wd->subs = eina_list_remove_list(wd->subs, l);
	     eina_stringshare_del(si->swallow);
	     free(si);
	     break;
	  }
     }
}

static void
_signal_size_eval(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _request_sizing_eval(data);
}

/**
 * Add a new layout to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Layout
 */
EAPI Evas_Object *
elm_layout_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   wd->obj = obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "layout");
   elm_widget_type_set(obj, "layout");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_changed_hook_set(obj, _changed_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_focus_cycle_hook_set(obj, _elm_layout_focus_cycle_hook);

   wd->lay = edje_object_add(e);
   elm_widget_resize_object_set(obj, wd->lay);
   edje_object_signal_callback_add(wd->lay, "size,eval", "elm",
                                   _signal_size_eval, wd);
   
   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _request_sizing_eval(wd);
   return obj;
}

/**
 * Set the file that will be used as layout
 *
 * @param obj The layout object
 * @param file The path to file (edj) that will be used as layout
 * @param group The group that the layout belongs in edje file
 *
 * @return (1 = success, 0 = error)
 *
 * @ingroup Layout
 */
EAPI Eina_Bool
elm_layout_file_set(Evas_Object *obj, const char *file, const char *group)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   Eina_Bool ret = edje_object_file_set(wd->lay, file, group);
   if (ret) _request_sizing_eval(wd);
   else DBG("failed to set edje file '%s', group '%s': %s",
            file, group,
            edje_load_error_str(edje_object_load_error_get(wd->lay)));
   return ret;
}

/**
 * Set the edje group from the elementary theme that will be used as layout
 *
 * @param obj The layout object
 * @param clas the clas of the group
 * @param group the group
 * @param style the style to used
 *
 * @return (1 = success, 0 = error)
 *
 * @ingroup Layout
 */
EAPI Eina_Bool
elm_layout_theme_set(Evas_Object *obj, const char *clas, const char *group, const char *style)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   Eina_Bool ret = _elm_theme_object_set(obj, wd->lay, clas, group, style);
   if (ret) _request_sizing_eval(wd);
   return ret;
}

/**
 * Set the layout content
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_layout_content_unset() function.
 *
 * @param obj The layout object
 * @param swallow The swallow group name in the edje file
 * @param content The content will be filled in this layout object
 *
 * @ingroup Layout
 */
EAPI void
elm_layout_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   const Eina_List *l;
   if (!wd) return;
   EINA_LIST_FOREACH(wd->subs, l, si)
     {
	if (!strcmp(swallow, si->swallow))
	  {
	     if (content == si->obj) return;
	     evas_object_del(si->obj);
	     break;
	  }
     }
   if (content)
     {
	elm_widget_sub_object_add(obj, content);
	evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, wd);
	edje_object_part_swallow(wd->lay, swallow, content);
	si = ELM_NEW(Subinfo);
	si->swallow = eina_stringshare_add(swallow);
	si->obj = content;
	wd->subs = eina_list_append(wd->subs, si);
     }
   _request_sizing_eval(wd);
}

/**
 * Unset the layout content
 *
 * Unparent and return the content object which was set for this widget
 *
 * @param obj The layout object
 * @param swallow The swallow group name in the edje file
 * @return The content that was being used
 *
 * @ingroup Layout
 */
EAPI Evas_Object *
elm_layout_content_unset(Evas_Object *obj, const char *swallow)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   const Eina_List *l;
   if (!wd) return NULL;
   EINA_LIST_FOREACH(wd->subs, l, si)
     {
	if (!strcmp(swallow, si->swallow))
	  {
	     Evas_Object *content;
	     if (!si->obj) return NULL;
	     content = si->obj; /* si will die in _sub_del due elm_widget_sub_object_del() */
	     elm_widget_sub_object_del(obj, content);
	     edje_object_part_unswallow(wd->lay, content);
	     return content;
	  }
     }
   return NULL;
}

/**
 * Get the edje layout
 *
 * @param obj The layout object
 * 
 * This returns the edje object. It is not expected to be used to then swallow
 * objects via edje_object_part_swallow() for example. Use 
 * elm_layout_content_set() instead so child object handling and sizing is
 * done properly. This is more intended for setting text, emitting signals,
 * hooking to signal callbacks etc.
 *
 * @return A Evas_Object with the edje layout settings loaded
 * with function elm_layout_file_set
 *
 * @ingroup Layout
 */
EAPI Evas_Object *
elm_layout_edje_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->lay;
}

/**
 * Get the edje layout
 * 
 * Manually forms a sizing re-evaluation when contents changed state so that
 * minimum size might have changed and needs re-evaluation. Also note that
 * a standard signal of "size,eval" "elm" emitted by the edje object will
 * cause this to happen too
 *
 * @param obj The layout object
 *
 * @ingroup Layout
 */
EAPI void
elm_layout_sizing_eval(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   EINA_SAFETY_ON_NULL_RETURN(wd);
   _request_sizing_eval(wd);
}
