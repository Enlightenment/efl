#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *frm;
   Evas_Object *img;
   int size;
   Eina_Bool fill;
   Ecore_Timer *longtimer;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info);

static const char SIG_CLICKED[] = "clicked";
static const char SIG_DRAG_START[] = "drag,start";
static const char SIG_DRAG_END[] = "drag,end";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CLICKED, ""},
   {SIG_DRAG_START, ""},
   {SIG_DRAG_END, ""},
   {NULL, NULL}
};


static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->frm, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(wd->frm, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->frm, "photo", "base",
                         elm_widget_style_get(obj));
   edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->img);
   edje_object_scale_set(wd->frm, elm_widget_scale_get(obj) *
                         _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   if (!wd) return;
   if (wd->size > 0)
     {
        double scale = 0.0;

        scale = (wd->size * elm_widget_scale_get(obj) * _elm_config->scale);
        evas_object_size_hint_min_set(wd->img, scale, scale);
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
   if (wd->fill)
     {
        Edje_Message_Int_Set *msg;
        Evas_Object *icon = _els_smart_icon_object_get(wd->img);

        evas_object_geometry_get(icon, NULL, NULL, &w, &h);
        msg = alloca(sizeof(Edje_Message_Int_Set) + (sizeof(int)));
        msg->count=2;
        msg->val[0] = (int)w;
        msg->val[1] = (int)h;

        edje_object_message_send(wd->frm, EDJE_MESSAGE_INT_SET, 0, msg);
     }
}


static void
_drag_done_cb(void *unused __UNUSED__, Evas_Object *obj)
{
   elm_object_scroll_freeze_pop(obj);
   evas_object_smart_callback_call(obj, SIG_DRAG_END, NULL);
}

static Eina_Bool
_longpress(void *objv)
{
   Widget_Data *wd = elm_widget_data_get(objv);
   Evas_Object *tmp;
   const char *file;
   char *buf;

   DBG("Long press: start drag!");
   wd->longtimer = NULL; /* clear: must return NULL now */
   evas_object_event_callback_del(objv, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move);

   tmp = _els_smart_icon_object_get(wd->img);
   file = NULL;
   evas_object_image_file_get(tmp,&file,NULL);
   if (file)
     {
        /* FIXME: Deal with relative paths */
        buf = malloc(strlen(file) + strlen("file://") + 1);
        sprintf(buf, "%s%s","file://",file);
        elm_drag_start(objv, ELM_SEL_FORMAT_IMAGE, buf, _drag_done_cb, NULL);
        free(buf);
     }
   elm_object_scroll_freeze_push(objv);

   evas_object_smart_callback_call(objv, SIG_DRAG_START, NULL);

   return 0; /* Don't call again */
}

static void
_mouse_move(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Move *move = event;

   /* Sanity */
   if (!wd->longtimer)
     {
        evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move);
        return;
     }

   /* if the event is held, stop waiting */
   if (move->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        /* Moved too far: No longpress for you! */
        ecore_timer_del(wd->longtimer);
        wd->longtimer = NULL;
        evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                       _mouse_move);
     }
}

static void
_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (wd->longtimer) ecore_timer_del(wd->longtimer);

   /* FIXME: Hard coded timeout */
   wd->longtimer = ecore_timer_add(0.7, _longpress, data);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move, data);
}

static void
_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if ((wd) && (wd->longtimer))
     {
        ecore_timer_del(wd->longtimer);
        wd->longtimer = NULL;
     }

   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

EAPI Evas_Object *
elm_photo_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Object *icon;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "photo");
   elm_widget_type_set(obj, "photo");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

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
   _els_smart_icon_scale_set(wd->img,
                             elm_widget_scale_get(obj) * _elm_config->scale);
   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, obj);
   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down, obj);
   evas_object_repeat_events_set(wd->img, 1);
   edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->img);
   evas_object_show(wd->img);
   elm_widget_sub_object_add(obj, wd->img);

   wd->longtimer = NULL;

   icon = _els_smart_icon_object_get(wd->img);
   evas_object_event_callback_add(icon, EVAS_CALLBACK_MOVE,
                                  _icon_move_resize, obj);
   evas_object_event_callback_add(icon, EVAS_CALLBACK_RESIZE,
                                  _icon_move_resize, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

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

EAPI void
elm_photo_size_set(Evas_Object *obj, int size)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->size = (size > 0) ? size : 0;

   _els_smart_icon_scale_size_set(wd->img, wd->size);

   _sizing_eval(obj);
}

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

EAPI void
elm_photo_editable_set(Evas_Object *obj, Eina_Bool set)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;;
   _els_smart_icon_edit_set(wd->img, set, obj);
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-3f0^-2{2(0W1st0 :*/
