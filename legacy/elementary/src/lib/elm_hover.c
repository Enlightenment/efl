#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Hover Hover
 *
 * A Hover object will over over the @p Parent object at the @p Target
 * location.  Anything in the background will be given a darker coloring
 * to indicate that the hover object is on top.
 *
 * NOTE: The hover object will take up the entire space of @p Target object.
 */

typedef struct _Widget_Data Widget_Data;
typedef struct _Subinfo Subinfo;

struct _Widget_Data
{
   Evas_Object *hov, *cov;
   Evas_Object *offset, *size;
   Evas_Object *parent, *target;
   Eina_List *subs;
};

struct _Subinfo
{
   const char *swallow;
   Evas_Object *obj;
};

static const char *widtype = NULL;
static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _reval_content(Evas_Object *obj);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _hov_show_do(Evas_Object *obj);
static void _hov_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _hov_resize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _hov_show(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _hov_hide(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   if (!wd) return;
   if (evas_object_visible_get(obj))
     evas_object_smart_callback_call(obj, "clicked", NULL);
   elm_hover_target_set(obj, NULL);
   elm_hover_parent_set(obj, NULL);
   evas_object_event_callback_del_full(wd->hov, EVAS_CALLBACK_MOVE, _hov_move, obj);
   evas_object_event_callback_del_full(wd->hov, EVAS_CALLBACK_RESIZE, _hov_resize, obj);
   evas_object_event_callback_del_full(wd->hov, EVAS_CALLBACK_SHOW, _hov_show, obj);
   evas_object_event_callback_del_full(wd->hov, EVAS_CALLBACK_HIDE, _hov_hide, obj);

   EINA_LIST_FREE(wd->subs, si)
     {
	eina_stringshare_del(si->swallow);
	free(si);
     }
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   // FIXME: hover contents doesnt seem to propagate resizes properly
   _elm_theme_object_set(obj, wd->cov, "hover", "base", elm_widget_style_get(obj));
   edje_object_scale_set(wd->cov, elm_widget_scale_get(obj) *
                         _elm_config->scale);
   _reval_content(obj);
   _sizing_eval(obj);
   if (evas_object_visible_get(wd->cov)) _hov_show_do(obj);
}


static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord x = 0, y = 0, w = 0, h = 0, x2 = 0, y2 = 0, w2 = 0, h2 = 0;
   if (!wd) return;
   if (wd->parent) evas_object_geometry_get(wd->parent, &x, &y, &w, &h);
   if (wd->hov) evas_object_geometry_get(wd->hov, &x2, &y2, &w2, &h2);
   evas_object_move(wd->cov, x, y);
   evas_object_resize(wd->cov, w, h);
   evas_object_size_hint_min_set(wd->offset, x2 - x, y2 - y);
   evas_object_size_hint_min_set(wd->size, w2, h2);
   edje_object_part_swallow(wd->cov, "elm.swallow.offset", wd->offset);
   edje_object_part_swallow(wd->cov, "elm.swallow.size", wd->size);
}

static void
_reval_content(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *l;
   const Subinfo *si;
   if (!wd) return;
   EINA_LIST_FOREACH(wd->subs, l, si)
     edje_object_part_swallow(wd->cov, si->swallow, si->obj);
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
	     wd->subs = eina_list_remove_list(wd->subs, l);
	     eina_stringshare_del(si->swallow);
	     free(si);
	     break;
	  }
     }
}

static void
_hov_show_do(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *l;
   const Subinfo *si;
   if (!wd) return;
   if (wd->cov)
     {
	evas_object_show(wd->cov);
	edje_object_signal_emit(wd->cov, "elm,action,show", "elm");
     }
   EINA_LIST_FOREACH(wd->subs, l, si)
     {
	char buf[1024];

	if (!strncmp(si->swallow, "elm.swallow.slot.", 17))
	  {
	     snprintf(buf, sizeof(buf), "elm,action,slot,%s,show",
		      si->swallow + 17);
	     edje_object_signal_emit(wd->cov, buf, "elm");
	  }
     }
}

static void
_hov_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_hov_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_hov_show(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _hov_show_do(data);
}

static void
_hov_hide(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   const Eina_List *l;
   const Subinfo *si;
   if (!wd) return;
   if (wd->cov)
     {
	edje_object_signal_emit(wd->cov, "elm,action,hide", "elm");
	evas_object_hide(wd->cov);
     }
   EINA_LIST_FOREACH(wd->subs, l, si)
     {
	char buf[1024];

	if (!strncmp(si->swallow, "elm.swallow.slot.", 17))
	  {
	     snprintf(buf, sizeof(buf), "elm,action,slot,%s,hide",
		      si->swallow + 17);
	     edje_object_signal_emit(wd->cov, buf, "elm");
	  }
     }
}

static void
_target_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->target = NULL;
}

static void
_signal_dismiss(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_hide(data);
   evas_object_smart_callback_call(data, "clicked", NULL);
}

static void
_parent_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_parent_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_parent_show(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
}

static void
_parent_hide(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd) evas_object_hide(wd->cov);
}

static void
_parent_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   elm_hover_parent_set(data, NULL);
   _sizing_eval(data);
}

/**
 * Adds a hover object to @p parent
 *
 * @param parent The parent object
 * @return The hover object or NULL if one could not be created
 *
 * @ingroup Hover
 */
EAPI Evas_Object *
elm_hover_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "hover");
   elm_widget_type_set(obj, "hover");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_del_hook_set(obj, _del_hook);

   wd->hov = evas_object_rectangle_add(e);
   evas_object_pass_events_set(wd->hov, 1);
   evas_object_color_set(wd->hov, 0, 0, 0, 0);
   elm_widget_resize_object_set(obj, wd->hov);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_MOVE, _hov_move, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_RESIZE, _hov_resize, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_SHOW, _hov_show, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_HIDE, _hov_hide, obj);

   wd->cov = edje_object_add(e);
   _elm_theme_object_set(obj, wd->cov, "hover", "base", "default");
   elm_widget_sub_object_add(obj, wd->cov);
   edje_object_signal_callback_add(wd->cov, "elm,action,dismiss", "",
                                   _signal_dismiss, obj);

   wd->offset = evas_object_rectangle_add(e);
   evas_object_pass_events_set(wd->offset, 1);
   evas_object_color_set(wd->offset, 0, 0, 0, 0);
   elm_widget_sub_object_add(obj, wd->offset);

   wd->size = evas_object_rectangle_add(e);
   evas_object_pass_events_set(wd->size, 1);
   evas_object_color_set(wd->size, 0, 0, 0, 0);
   elm_widget_sub_object_add(obj, wd->size);

   edje_object_part_swallow(wd->cov, "elm.swallow.offset", wd->offset);
   edje_object_part_swallow(wd->cov, "elm.swallow.size", wd->size);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Sets the target object for the hover.
 *
 * @param obj The hover object
 * @param target The object to center the hover onto. The hover
 * will take up the entire space that the target object fills.
 *
 * @ingroup Hover
 */
EAPI void
elm_hover_target_set(Evas_Object *obj, Evas_Object *target)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (wd->target)
     evas_object_event_callback_del_full(wd->target, EVAS_CALLBACK_DEL,
                                    _target_del, obj);
   wd->target = target;
   if (wd->target)
     {
	evas_object_event_callback_add(wd->target, EVAS_CALLBACK_DEL,
                                       _target_del, obj);
	elm_widget_hover_object_set(target, obj);
	_sizing_eval(obj);
     }
}


/**
 * Sets the parent object for the hover.
 *
 * @param obj The hover object
 * @param parent The object to locate the hover over.
 *
 * @ingroup Hover
 */
EAPI void
elm_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->parent)
     {
	evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_MOVE,
                                       _parent_move, obj);
	evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_RESIZE,
                                       _parent_resize, obj);
	evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_SHOW,
                                       _parent_show, obj);
	evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_HIDE,
                                       _parent_hide, obj);
	evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_DEL,
                                       _parent_del, obj);
     }
   wd->parent = parent;
   if (wd->parent)
     {
	evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_MOVE,
                                       _parent_move, obj);
	evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_RESIZE,
                                       _parent_resize, obj);
	evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_SHOW,
                                       _parent_show, obj);
	evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_HIDE,
                                       _parent_hide, obj);
	evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_DEL,
                                       _parent_del, obj);
//	elm_widget_sub_object_add(parent, obj);
     }
   _sizing_eval(obj);
}

/**
 * Gets the target object for the hover.
 *
 * @param obj The hover object
 * @return The target object of the hover.
 * 
 * @ingroup Hover
 */
EAPI Evas_Object *
elm_hover_target_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   return wd->target;
}

/**
 * Gets the parent object for the hover.
 *
 * @param obj The hover object
 * @return The parent object to locate the hover over.
 *
 * @ingroup Hover
 */
EAPI Evas_Object *
elm_hover_parent_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   return wd->parent;
}

/**
 * Sets the content of the hover object and the direction in which
 * it will pop out.
 *
 * @param obj The hover object
 * @param swallow The direction that the object will display in. Multiple
 * objects can have the same swallow location. Objects placed in the same
 * swallow will be placed starting at the middle of the hover and ending
 * farther from the middle.
 * Accepted values are "left" "right" "top" "bottom" "middle"
 * @param content The content to place at @p swallow
 *
 * @ingroup Hover
 */
EAPI void
elm_hover_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   const Eina_List *l;
   char buf[1024];
   if (!wd) return;
   snprintf(buf, sizeof(buf), "elm.swallow.slot.%s", swallow);
   EINA_LIST_FOREACH(wd->subs, l, si)
     {
	if (!strcmp(buf, si->swallow))
	  {
	     if (content == si->obj) return;
	     elm_widget_sub_object_del(obj, si->obj);
	     break;
	  }
     }
   if (content)
     {
	elm_widget_sub_object_add(obj, content);
	edje_object_part_swallow(wd->cov, buf, content);
	si = ELM_NEW(Subinfo);
	si->swallow = eina_stringshare_add(buf);
	si->obj = content;
	wd->subs = eina_list_append(wd->subs, si);
	_sizing_eval(obj);
     }
}

/**
 * Returns the best swallow location for content in the hover.
 *
 * @param obj The hover object
 * @return The edje location to place content into the hover.
 * See also elm_hover_content_set()
 *
 * @ingroup Hover
 */
EAPI const char *
elm_hover_best_content_location_get(const Evas_Object *obj, Elm_Hover_Axis pref_axis)
{
   ELM_CHECK_WIDTYPE(obj, widtype) "left";
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord x = 0, y = 0, w = 0, h = 0, x2 = 0, y2 = 0, w2 = 0, h2 = 0;
   Evas_Coord spc_l, spc_r, spc_t, spc_b;
   if (!wd) return "left";
   if (wd->parent) evas_object_geometry_get(wd->parent, &x, &y, &w, &h);
   if (wd->target) evas_object_geometry_get(wd->target, &x2, &y2, &w2, &h2);
   spc_l = x2 - x;
   spc_r = (x + w) - (x2 + w2);
   if (spc_l < 0) spc_l = 0;
   if (spc_r < 0) spc_r = 0;
   spc_t = y2 - y;
   spc_b = (y + h) - (y2 + h2);
   if (spc_t < 0) spc_t = 0;
   if (spc_b < 0) spc_b = 0;
   if (pref_axis == ELM_HOVER_AXIS_HORIZONTAL)
     {
	if (spc_l < spc_r) return "right";
	else return "left";
     }
   else if (pref_axis == ELM_HOVER_AXIS_VERTICAL)
     {
	if (spc_t < spc_b) return "bottom";
	else return "top";
     }
   if (spc_l < spc_r)
     {
	if (spc_t > spc_r) return "top";
	else if (spc_b > spc_r) return "bottom";
	return "right";
     }
   if (spc_t > spc_r) return "top";
   else if (spc_b > spc_r) return "bottom";
   return "left";
}
