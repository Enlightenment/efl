#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Photo Photo
 *
 * For displaying the photo of a person (contact). Simple yet
 * with a very specific purpose. 
 * 
 * Signals that you can add callbacks for are:
 *
 * clicked - This is called when a user has clicked the photo
 *
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *frm;
   Evas_Object *img;
   int size;
   Eina_Bool fill;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);

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
   _elm_theme_object_set(obj, wd->frm, "photo", "base", elm_widget_style_get(obj));
   edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->img);
   edje_object_scale_set(wd->frm, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;

   if(wd->size > 0) {
	   evas_object_size_hint_min_set(wd->img,
			   wd->size * elm_widget_scale_get(obj) * _elm_config->scale,
			   wd->size * elm_widget_scale_get(obj) * _elm_config->scale);
	   edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->img);
	   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
	   edje_object_size_min_restricted_calc(wd->frm, &minw, &minh, minw, minh);
	   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
	   maxw = minw;
	   maxh = minh;
	   evas_object_size_hint_min_set(obj, minw, minh);
	   evas_object_size_hint_max_set(obj, maxw, maxh);
   }
}

static void
_icon_move_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Coord w, h;
	Widget_Data *wd = elm_widget_data_get(data);
	if (!wd) return;

	if(wd->fill) {
		Evas_Object *icon = _els_smart_icon_object_get(wd->img);
		evas_object_geometry_get(icon, NULL, NULL, &w, &h);
		Edje_Message_Int_Set *msg = alloca(sizeof(Edje_Message_Int_Set) + (sizeof(int)));
		msg->count=2;
		msg->val[0] = (int)w;
		msg->val[1] = (int)h;

		edje_object_message_send(wd->frm, EDJE_MESSAGE_INT_SET, 0, msg);
	}
}


static void
_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "clicked", NULL);
}

/**
 * Add a new photo to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Photo
 */
EAPI Evas_Object *
elm_photo_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Object *icon;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "photo");
   elm_widget_type_set(obj, "photo");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, 0);

   wd->frm = edje_object_add(e);
   _elm_theme_object_set(obj, wd->frm, "photo", "base", "default");
   elm_widget_resize_object_set(obj, wd->frm);

   wd->img = _els_smart_icon_add(e);
   _els_smart_icon_scale_up_set(wd->img, 1);
   _els_smart_icon_scale_down_set(wd->img, 1);
   _els_smart_icon_smooth_scale_set(wd->img, 1);
   _els_smart_icon_fill_inside_set(wd->img, 0);
   _els_smart_icon_scale_size_set(wd->img, 0);
   wd->fill = EINA_FALSE;
   _els_smart_icon_scale_set(wd->img, elm_widget_scale_get(obj) * _elm_config->scale);
   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_MOUSE_UP,
				  _mouse_up, obj);
   evas_object_repeat_events_set(wd->img, 1);
   edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->img);
   evas_object_show(wd->img);
   elm_widget_sub_object_add(obj, wd->img);


   icon = _els_smart_icon_object_get(wd->img);
   evas_object_event_callback_add(icon, EVAS_CALLBACK_MOVE,
				  _icon_move_resize, obj);
   evas_object_event_callback_add(icon, EVAS_CALLBACK_RESIZE,
   				  _icon_move_resize, obj);
   _sizing_eval(obj);
   return obj;
}

/**
 * Set the file that will be used as photo
 *
 * @param obj The photo object
 * @param file The path to file that will be used as photo
 *
 * @return (1 = sucess, 0 = error)
 *
 * @ingroup Photo
 */
EAPI Eina_Bool
elm_photo_file_set(Evas_Object *obj, const char *file)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   if (!_els_smart_icon_file_key_set(wd->img, file, NULL))
     return EINA_FALSE;

   _sizing_eval(obj);
   return EINA_TRUE;
}

/**
 * Set the size that will be used on the photo
 *
 * @param obj The photo object
 * @param size The size that the photo will be
 *
 * @ingroup Photo
 */
EAPI void
elm_photo_size_set(Evas_Object *obj, int size)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->size = size;

   if(size >= 0)
	   _els_smart_icon_scale_size_set(wd->img, size);
   else
	   _els_smart_icon_scale_size_set(wd->img, 0);

   _sizing_eval(obj);
}

/**
 * Set if the photo should be completely visible or not.
 *
 * @param obj The photo object
 * @param fill if true the photo will be completely visible
 *
 * @ingroup Photo
 */
EAPI void
elm_photo_fill_inside_set(Evas_Object *obj, Eina_Bool fill)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _els_smart_icon_fill_inside_set(wd->img, fill);
   wd->fill = fill;
   _sizing_eval(obj);
}

