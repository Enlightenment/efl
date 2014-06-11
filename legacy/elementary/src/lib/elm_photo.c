#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_photo.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

#define MY_CLASS ELM_PHOTO_CLASS

#define MY_CLASS_NAME "Elm_Photo"
#define MY_CLASS_NAME_LEGACY "elm_photo"

static const char SIG_CLICKED[] = "clicked";
static const char SIG_DRAG_START[] = "drag,start";
static const char SIG_DRAG_END[] = "drag,end";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_DRAG_START, ""},
   {SIG_DRAG_END, ""},
   {NULL, NULL}
};

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   double scale;

   ELM_PHOTO_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->size <= 0) return;

   scale = (sd->size * elm_widget_scale_get(obj) * elm_config_scale_get());

   evas_object_size_hint_min_set(sd->icon, scale, scale);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   maxw = minw;
   maxh = minh;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

EOLIAN static Eina_Bool
_elm_photo_elm_widget_theme_apply(Eo *obj, Elm_Photo_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   edje_object_mirrored_set
     (wd->resize_obj, elm_widget_mirrored_get(obj));

   elm_widget_theme_object_set
     (obj, wd->resize_obj, "photo", "base",
     elm_widget_style_get(obj));

   elm_object_scale_set(sd->icon, elm_widget_scale_get(obj));

   edje_object_scale_set(wd->resize_obj,
                         elm_widget_scale_get(obj) * elm_config_scale_get());
   _sizing_eval(obj);

   return EINA_TRUE;
}

static void
_icon_move_resize_cb(void *data,
                     Evas *e EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;

   ELM_PHOTO_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->fill_inside)
     {
        Edje_Message_Int_Set *msg;
        Evas_Object *img = elm_image_object_get(sd->icon);

        evas_object_geometry_get(img, NULL, NULL, &w, &h);
        msg = alloca(sizeof(Edje_Message_Int_Set) + (sizeof(int)));
        msg->count = 2;
        msg->val[0] = (int)w;
        msg->val[1] = (int)h;

        edje_object_message_send
          (wd->resize_obj, EDJE_MESSAGE_INT_SET, 0, msg);
     }

   if (sd->thumb.file.path)
     elm_icon_thumb_set(sd->icon, sd->thumb.file.path, sd->thumb.file.key);
}

static void
_drag_done_cb(void *unused EINA_UNUSED,
              Evas_Object *obj)
{
   ELM_PHOTO_DATA_GET(obj, sd);

   elm_object_scroll_freeze_pop(obj);
   evas_object_smart_callback_call(obj, SIG_DRAG_END, NULL);
   sd->drag_started = EINA_FALSE;
}

static void
_mouse_move(void *data,
            Evas *e EINA_UNUSED,
            Evas_Object *icon,
            void *event)
{
   Evas_Event_Mouse_Move *move = event;

   ELM_PHOTO_DATA_GET(data, sd);

   /* Sanity */
   if (!sd->long_press_timer)
     {
        evas_object_event_callback_del
          (icon, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move);
        return;
     }

   /* if the event is held, stop waiting */
   if (move->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        /* Moved too far: No longpress for you! */
        ELM_SAFE_FREE(sd->long_press_timer, ecore_timer_del);
        evas_object_event_callback_del
          (icon, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move);
     }
}

static Eina_Bool
_long_press_cb(void *obj)
{
   Evas_Object *img;
   const char *file;

   ELM_PHOTO_DATA_GET(obj, sd);

   DBG("Long press: start drag!");
   sd->long_press_timer = NULL; /* clear: must return NULL now */
   evas_object_event_callback_del
     (sd->icon, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move);

   img = elm_image_object_get(sd->icon);
   file = NULL;
   evas_object_image_file_get(img, &file, NULL);
   if (file)
     {
        char buf[4096 + 7];

        /* FIXME: Deal with relative paths; use PATH_MAX */
        snprintf(buf, sizeof(buf), "file://%s", file);
        if (elm_drag_start
              (obj, ELM_SEL_FORMAT_IMAGE, buf, ELM_XDND_ACTION_MOVE,
                  NULL, NULL,
                  NULL, NULL,
                  NULL, NULL,
                  _drag_done_cb, NULL))
          {
             elm_object_scroll_freeze_push(obj);
             evas_object_smart_callback_call(obj, SIG_DRAG_START, NULL);
             sd->drag_started = EINA_TRUE;
          }
     }

   return EINA_FALSE; /* Don't call again */
}

static void
_mouse_down(void *data,
            Evas *e EINA_UNUSED,
            Evas_Object *icon,
            void *event_info EINA_UNUSED)
{
   Evas_Event_Mouse_Down *ev = event_info;

   ELM_PHOTO_DATA_GET(data, sd);

   if (ev->button != 1) return;

   ecore_timer_del(sd->long_press_timer);
   sd->long_press_timer = ecore_timer_add(_elm_config->longpress_timeout,
                                          _long_press_cb, data);
   evas_object_event_callback_add
     (icon, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, data);
}

static void
_mouse_up(void *data,
          Evas *e EINA_UNUSED,
          Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   Evas_Event_Mouse_Up *ev = event_info;
   ELM_PHOTO_DATA_GET(data, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   ELM_SAFE_FREE(sd->long_press_timer, ecore_timer_del);

   if (!sd->drag_started)
     evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static inline int
_icon_size_min_get(Evas_Object *icon)
{
   int size;

   elm_image_object_size_get(icon, &size, NULL);

   return (size < 32) ? 32 : size;
}

static void
_elm_photo_internal_image_follow(Evas_Object *obj)
{
   Evas_Object *img;

   ELM_PHOTO_DATA_GET(obj, sd);

   img = elm_image_object_get(sd->icon);

   evas_object_event_callback_add
     (img, EVAS_CALLBACK_MOVE, _icon_move_resize_cb, obj);
   evas_object_event_callback_add
     (img, EVAS_CALLBACK_RESIZE, _icon_move_resize_cb, obj);
}

static void
_on_thumb_done(void *data,
               Evas_Object *obj EINA_UNUSED,
               void *event EINA_UNUSED)
{
   _elm_photo_internal_image_follow(data);
}

EOLIAN static void
_elm_photo_evas_object_smart_add(Eo *obj, Elm_Photo_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   priv->icon = elm_icon_add(obj);
   evas_object_repeat_events_set(priv->icon, EINA_TRUE);

   elm_image_resizable_set(priv->icon, EINA_TRUE, EINA_TRUE);
   elm_image_smooth_set(priv->icon, EINA_TRUE);
   elm_image_fill_outside_set(priv->icon, !priv->fill_inside);
   elm_image_prescale_set(priv->icon, 0);

   elm_object_scale_set(priv->icon, elm_widget_scale_get(obj));

   evas_object_event_callback_add
     (priv->icon, EVAS_CALLBACK_MOUSE_UP, _mouse_up, obj);
   evas_object_event_callback_add
     (priv->icon, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, obj);

   evas_object_smart_callback_add
     (priv->icon, "thumb,done", _on_thumb_done, obj);

   _elm_photo_internal_image_follow(obj);

   _sizing_eval(obj);

   elm_widget_resize_object_set
       (obj, edje_object_add(evas_object_evas_get(obj)), EINA_TRUE);

   elm_widget_theme_object_set
     (obj, wd->resize_obj, "photo", "base", "default");

   edje_object_part_swallow
     (wd->resize_obj, "elm.swallow.content", priv->icon);

   elm_photo_file_set(obj, NULL);
}

EOLIAN static void
_elm_photo_evas_object_smart_del(Eo *obj, Elm_Photo_Data *sd)
{
   ecore_timer_del(sd->long_press_timer);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_photo_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_photo_eo_base_constructor(Eo *obj, Elm_Photo_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks),
         elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_IMAGE));
}

EOLIAN static Eina_Bool
_elm_photo_file_set(Eo *obj, Elm_Photo_Data *sd, const char *file)
{
   if (!file)
     {
        if (!elm_icon_standard_set(sd->icon, "no_photo")) return EINA_FALSE;
     }
   else
     {
        if (!elm_image_file_set(sd->icon, file, NULL)) return EINA_FALSE;
     }

   _sizing_eval(obj);

   return EINA_TRUE;
}

EOLIAN static void
_elm_photo_size_set(Eo *obj, Elm_Photo_Data *sd, int size)
{
   sd->size = (size > 0) ? size : 0;

   elm_image_prescale_set(sd->icon, sd->size);

   _sizing_eval(obj);
}

EOLIAN static int
_elm_photo_size_get(Eo *obj EINA_UNUSED, Elm_Photo_Data *sd)
{
   return sd->size;
}

EOLIAN static void
_elm_photo_fill_inside_set(Eo *obj, Elm_Photo_Data *sd, Eina_Bool fill)
{
   elm_image_fill_outside_set(sd->icon, !fill);
   sd->fill_inside = !!fill;

   _sizing_eval(obj);
}

EOLIAN static Eina_Bool
_elm_photo_fill_inside_get(Eo *obj EINA_UNUSED, Elm_Photo_Data *sd)
{
   return sd->fill_inside;
}

EOLIAN static void
_elm_photo_editable_set(Eo *obj EINA_UNUSED, Elm_Photo_Data *sd, Eina_Bool set)
{
   elm_image_editable_set(sd->icon, set);
}

EOLIAN static Eina_Bool
_elm_photo_editable_get(Eo *obj EINA_UNUSED, Elm_Photo_Data *sd)
{
   return elm_image_editable_get(sd->icon);
}

EOLIAN static void
_elm_photo_thumb_set(Eo *obj EINA_UNUSED, Elm_Photo_Data *sd, const char *file, const char *group)
{
   eina_stringshare_replace(&sd->thumb.file.path, file);
   eina_stringshare_replace(&sd->thumb.file.key, group);

   elm_icon_thumb_set(sd->icon, file, group);
}

EOLIAN static void
_elm_photo_aspect_fixed_set(Eo *obj EINA_UNUSED, Elm_Photo_Data *sd, Eina_Bool fixed)
{
   elm_image_aspect_fixed_set(sd->icon, fixed);
}

EOLIAN static Eina_Bool
_elm_photo_aspect_fixed_get(Eo *obj EINA_UNUSED, Elm_Photo_Data *sd)
{
   return elm_image_aspect_fixed_get(sd->icon);
}

static void
_elm_photo_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_photo.eo.c"
