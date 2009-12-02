#include <Elementary.h>
#include "elm_priv.h"

#if 0 // working on it

// fixme: on configure of widget smart obj - reconfigure front + back and maps
/**
 * @defgroup Flip Flip
 *
 * This holds 2 content objects, ont on the front and one on the back and
 * allows you to flip from front to back and vice-versa given various effects
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   struct {
      Evas_Object *content;
   } front, back;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, minw2 = -1, minh2 = -1;
   Evas_Coord maxw = -1, maxh = -1, maxw2 = -1, maxh2 = -1;
   
   if (wd->front.content)
     evas_object_size_hint_min_get(wd->front.content, &minw, &minh);
   if (wd->back.content)
     evas_object_size_hint_min_get(wd->back.content, &minw2, &minh2);
   if (wd->front.content)
     evas_object_size_hint_max_get(wd->front.content, &maxw, &maxh);
   if (wd->back.content)
     evas_object_size_hint_max_get(wd->back.content, &maxw2, &maxh2);
   
   if (minw2 > minw) minw = minw2;
   if (minh2 > minh) minh = minh2;
   if ((maxw2 >= 0) && (maxw2 < maxw)) maxw = maxw2;
   if ((maxh2 >= 0) && (maxh2 < maxh)) maxh = maxh2;
   
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;

   if (sub == wd->front.content)
     {
	evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
	wd->front.content = NULL;
	_sizing_eval(obj);
     }
   else if (sub == wd->back.content)
     {
	evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
	wd->back.content = NULL;
	_sizing_eval(obj);
     }
}

/**
 * Add a new flip to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Flip
 */
EAPI Evas_Object *
elm_flip_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "flip");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Set the flip front content
 *
 * @param obj The flip object
 * @param content The content will be filled in this flip object
 *
 * @ingroup Flip
 */
EAPI void
elm_flip_content_front_set(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((wd->front.content != content) && (wd->front.content))
     {
        elm_widget_sub_object_del(obj, wd->front.content);
        evas_object_smart_member_del(wd->front.content);
     }
   wd->front.content = content;
   if (content)
     {
	elm_widget_sub_object_add(content, obj);
        evas_object_smart_member_add(obj, content);
	evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	_sizing_eval(obj);
     }
   //XXX use clips
   if (!elm_flip_front_get(obj)) evas_object_hide(wd->front.content);
}

/**
 * Set the flip back content
 *
 * @param obj The flip object
 * @param content The content will be filled in this flip object
 *
 * @ingroup Flip
 */
EAPI void
elm_flip_content_back_set(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((wd->back.content != content) && (wd->back.content))
     {
        elm_widget_sub_object_del(obj, wd->back.content);
        evas_object_smart_member_del(wd->back.content);
     }
   wd->back.content = content;
   if (content)
     {
	elm_widget_sub_object_add(content, obj);
        evas_object_smart_member_add(obj, content);
	evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	_sizing_eval(obj);
     }
   //XXX use clips
   if (elm_flip_front_get(obj)) evas_object_hide(wd->back.content);
}

/**
 * Get flip visibility state
 *
 * @param obj The flip object
 * @return If front is showing or not currently
 *
 * @ingroup Flip
 */
EAPI Eina_Bool
elm_flip_front_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   //XXX retunr if front is showing or not
   return 1;
}

EAPI void
elm_flip_perspective_set(Evas_Object *obj, Evas_Coord foc, Evas_Coord x, Evas_Coord y)
{
   Widget_Data *wd = elm_widget_data_get(obj);
}

EAPI void
elm_flip_go(Evas_Object *obj, Elm_Flip_Mode mode)
{
   Widget_Data *wd = elm_widget_data_get(obj);
}
#endif
