#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Index Index
 *
 * XXX
 *
 * Signals that you can add callbacks for are:
 *
 * xxx - XXX.
 * 
 * xxx - XXX.
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *base;
   Evas_Object *event;
   Evas_Object *bx[2];
   const char *label;
   Eina_Bool horizontal : 1;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->label) eina_stringshare_del(wd->label);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->horizontal)
     _elm_theme_set(wd->base, "index", "horizontal", elm_widget_style_get(obj));
   else
     _elm_theme_set(wd->base, "index", "vertical", elm_widget_style_get(obj));
   edje_object_part_swallow(wd->base, "elm.swallow.event", wd->event);
   edje_object_part_swallow(wd->base, "elm.swallow.content", wd->bx[0]);
   if (edje_object_part_exists(wd->base, "elm.swallow.content.sub"))
     {
        if (!wd->bx[1])
          {
             wd->bx[1] = _els_smart_box_add(evas_object_evas_get(wd->base));
             _els_smart_box_orientation_set(wd->bx[1], 0);
             _els_smart_box_homogenous_set(wd->bx[1], 1);
             elm_widget_sub_object_add(obj, wd->bx[1]);
          }
        edje_object_part_swallow(wd->base, "elm.swallow.content.sub", wd->bx[1]);
        evas_object_show(wd->bx[1]);
     }
   else if (wd->bx[1])
     {
        evas_object_del(wd->bx[1]);
        wd->bx[1] = NULL;
     }
   edje_object_message_signal_process(wd->base);
   edje_object_scale_set(wd->base, elm_widget_scale_get(obj) * _elm_config->scale);
   // FIXME: content and sub lists - eval
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->base, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void 
_wheel(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev = event_info;
   Evas_Object *obj = o;
}

static void 
_mouse_down(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Object *obj = o;
}

static void 
_mouse_up(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Object *obj = o;
}

static void 
_mouse_move(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *obj = o;
}

/**
 * Add a new index to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Index
 */
EAPI Evas_Object *
elm_index_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas_Object *o;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "index");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->horizontal = EINA_FALSE;

   wd->base = edje_object_add(e);
   _elm_theme_set(wd->base, "index", "vertical", "default");
   elm_widget_resize_object_set(obj, wd->base);
   
   o = evas_object_rectangle_add(e);
   wd->event = o;
   evas_object_color_set(o, 0, 0, 0, 0);
   edje_object_part_swallow(wd->base, "elm.swallow.event", o);
   elm_widget_sub_object_add(obj, o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_WHEEL, _wheel, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _mouse_up, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, obj);
   evas_object_show(o);
   
   wd->bx[0] = _els_smart_box_add(e);
   _els_smart_box_orientation_set(wd->bx[0], 0);
   _els_smart_box_homogenous_set(wd->bx[0], 1);
   elm_widget_sub_object_add(obj, wd->bx[0]);
   edje_object_part_swallow(wd->base, "elm.swallow.content", wd->bx[0]);
   evas_object_show(wd->bx[0]);

   if (edje_object_part_exists(wd->base, "elm.swallow.content.sub"))
     {
        wd->bx[1] = _els_smart_box_add(e);
        _els_smart_box_orientation_set(wd->bx[1], 0);
        _els_smart_box_homogenous_set(wd->bx[1], 1);
        elm_widget_sub_object_add(obj, wd->bx[1]);
        edje_object_part_swallow(wd->base, "elm.swallow.content.sub", wd->bx[1]);
        evas_object_show(wd->bx[1]);
     }

   _sizing_eval(obj);
   return obj;
}

#if 0
/**
 * Set the label of the index
 *
 * @param obj The index object
 * @param label The text label string in UTF-8
 *
 * @ingroup Index
 */
EAPI void
elm_index_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   if (label)
     {
	wd->label = eina_stringshare_add(label);
	edje_object_signal_emit(wd->base, "elm,state,text,visible", "elm");
	edje_object_message_signal_process(wd->base);
     }
   else
     {
	wd->label = NULL;
	edje_object_signal_emit(wd->base, "elm,state,text,hidden", "elm");
	edje_object_message_signal_process(wd->base);
     }
   edje_object_part_text_set(wd->base, "elm.text", label);
   _sizing_eval(obj);
}

/**
 * Get the label of the index
 *
 * @param obj The index object
 * @return The text label string in UTF-8
 *
 * @ingroup Index
 */
EAPI const char *
elm_index_label_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return wd->label;
}
#endif

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_append(Evas_Object *obj, const char *letter, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_prepend(Evas_Object *obj, const char *letter, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_append_relative(Evas_Object *obj, const char *letter, const void *item, const void *relative)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!relative) elm_index_item_append(obj, letter, item);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_prepend_relative(Evas_Object *obj, const char *letter, const void *item, const void *relative)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!relative) elm_index_item_prepend(obj, letter, item);
}

