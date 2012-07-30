#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_photo.h"

EAPI const char ELM_PHOTO_SMART_NAME[] = "elm_photo";

static const char SIG_CLICKED[] = "clicked";
static const char SIG_DRAG_START[] = "drag,start";
static const char SIG_DRAG_END[] = "drag,end";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_DRAG_START, ""},
   {SIG_DRAG_END, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_PHOTO_SMART_NAME, _elm_photo, Elm_Photo_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, _smart_callbacks);

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   double scale;

   ELM_PHOTO_DATA_GET(obj, sd);

   if (sd->size <= 0) return;

   scale = (sd->size * elm_widget_scale_get(obj) * elm_config_scale_get());

   evas_object_size_hint_min_set(sd->icon, scale, scale);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   maxw = minw;
   maxh = minh;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static Eina_Bool
_elm_photo_smart_theme(Evas_Object *obj)
{
   ELM_PHOTO_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_photo_parent_sc)->theme(obj))
     return EINA_FALSE;

   edje_object_mirrored_set
     (ELM_WIDGET_DATA(sd)->resize_obj, elm_widget_mirrored_get(obj));

   elm_widget_theme_object_set
     (obj, ELM_WIDGET_DATA(sd)->resize_obj, "photo", "base",
     elm_widget_style_get(obj));

   elm_object_scale_set(sd->icon, elm_widget_scale_get(obj));

   edje_object_scale_set(ELM_WIDGET_DATA(sd)->resize_obj,
                         elm_widget_scale_get(obj) * elm_config_scale_get());
   _sizing_eval(obj);

   return EINA_TRUE;
}

static void
_icon_move_resize_cb(void *data,
                     Evas *e __UNUSED__,
                     Evas_Object *obj __UNUSED__,
                     void *event_info __UNUSED__)
{
   Evas_Coord w, h;

   ELM_PHOTO_DATA_GET(data, sd);

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
          (ELM_WIDGET_DATA(sd)->resize_obj, EDJE_MESSAGE_INT_SET, 0, msg);
     }

#ifdef HAVE_ELEMENTARY_ETHUMB
   if (sd->thumb.file.path)
     elm_icon_thumb_set(sd->icon, sd->thumb.file.path, sd->thumb.file.key);
#endif
}

static void
_drag_done_cb(void *unused __UNUSED__,
              Evas_Object *obj)
{
   elm_object_scroll_freeze_pop(obj);
   evas_object_smart_callback_call(obj, SIG_DRAG_END, NULL);
}

static void
_mouse_move(void *data,
            Evas *e __UNUSED__,
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
        ecore_timer_del(sd->long_press_timer);
        sd->long_press_timer = NULL;
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
              (obj, ELM_SEL_FORMAT_IMAGE, buf, _drag_done_cb, NULL))
          {
             elm_object_scroll_freeze_push(obj);
             evas_object_smart_callback_call(obj, SIG_DRAG_START, NULL);
          }
     }

   return EINA_FALSE; /* Don't call again */
}

static void
_mouse_down(void *data,
            Evas *e __UNUSED__,
            Evas_Object *icon,
            void *event_info __UNUSED__)
{
   ELM_PHOTO_DATA_GET(data, sd);

   if (sd->long_press_timer) ecore_timer_del(sd->long_press_timer);

   /* FIXME: Hard coded timeout */
   sd->long_press_timer = ecore_timer_add(0.7, _long_press_cb, data);
   evas_object_event_callback_add
     (icon, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, data);
}

static void
_mouse_up(void *data,
          Evas *e __UNUSED__,
          Evas_Object *obj __UNUSED__,
          void *event_info __UNUSED__)
{
   ELM_PHOTO_DATA_GET(data, sd);

   if (sd->long_press_timer)
     {
        ecore_timer_del(sd->long_press_timer);
        sd->long_press_timer = NULL;
     }

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
               Evas_Object *obj __UNUSED__,
               void *event __UNUSED__)
{
   _elm_photo_internal_image_follow(data);
}

static void
_elm_photo_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Photo_Smart_Data);

   ELM_WIDGET_CLASS(_elm_photo_parent_sc)->base.add(obj);

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

   elm_widget_sub_object_add(obj, priv->icon);

   priv->long_press_timer = NULL;

   _elm_photo_internal_image_follow(obj);

   _sizing_eval(obj);

   elm_widget_resize_object_set
       (obj, edje_object_add(evas_object_evas_get(obj)));

   elm_widget_theme_object_set
     (obj, ELM_WIDGET_DATA(priv)->resize_obj, "photo", "base", "default");

   edje_object_part_swallow
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm.swallow.content", priv->icon);

   elm_photo_file_set(obj, NULL);
}

static void
_elm_photo_smart_del(Evas_Object *obj)
{
   ELM_PHOTO_DATA_GET(obj, sd);

   if (sd->long_press_timer) ecore_timer_del(sd->long_press_timer);
   sd->long_press_timer = NULL;

   ELM_WIDGET_CLASS(_elm_photo_parent_sc)->base.del(obj);
}

static void
_elm_photo_smart_set_user(Elm_Photo_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_photo_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_photo_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_photo_smart_theme;
}

EAPI const Elm_Photo_Smart_Class *
elm_photo_smart_class_get(void)
{
   static Elm_Photo_Smart_Class _sc =
     ELM_PHOTO_SMART_CLASS_INIT_NAME_VERSION(ELM_PHOTO_SMART_NAME);
   static const Elm_Photo_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_photo_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_photo_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_photo_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Eina_Bool
elm_photo_file_set(Evas_Object *obj,
                   const char *file)
{
   ELM_PHOTO_CHECK(obj) EINA_FALSE;
   ELM_PHOTO_DATA_GET(obj, sd);

   if (!file)
     {
        if (!elm_icon_standard_set(sd->icon, "no_photo"))
          return EINA_FALSE;
     }
   else
     {
        if (!elm_image_file_set(sd->icon, file, NULL))
          return EINA_FALSE;
     }

   _sizing_eval(obj);

   return EINA_TRUE;
}

EAPI void
elm_photo_size_set(Evas_Object *obj,
                   int size)
{
   ELM_PHOTO_CHECK(obj);
   ELM_PHOTO_DATA_GET(obj, sd);

   sd->size = (size > 0) ? size : 0;

   elm_image_prescale_set(sd->icon, sd->size);

   _sizing_eval(obj);
}

EAPI void
elm_photo_fill_inside_set(Evas_Object *obj,
                          Eina_Bool fill)
{
   ELM_PHOTO_CHECK(obj);
   ELM_PHOTO_DATA_GET(obj, sd);

   elm_image_fill_outside_set(sd->icon, !fill);
   sd->fill_inside = !!fill;

   _sizing_eval(obj);
}

EAPI void
elm_photo_editable_set(Evas_Object *obj,
                       Eina_Bool set)
{
   ELM_PHOTO_CHECK(obj);
   ELM_PHOTO_DATA_GET(obj, sd);

   elm_image_editable_set(sd->icon, set);
}

EAPI void
elm_photo_thumb_set(const Evas_Object *obj,
                    const char *file,
                    const char *group)
{
   ELM_PHOTO_CHECK(obj);

#ifdef HAVE_ELEMENTARY_ETHUMB
   ELM_PHOTO_DATA_GET(obj, sd);

   eina_stringshare_replace(&sd->thumb.file.path, file);
   eina_stringshare_replace(&sd->thumb.file.key, group);

   elm_icon_thumb_set(sd->icon, file, group);
#else
   (void)obj;
   (void)file;
   (void)group;
#endif
}

EAPI void
elm_photo_aspect_fixed_set(Evas_Object *obj,
                           Eina_Bool fixed)
{
   ELM_PHOTO_CHECK(obj);
   ELM_PHOTO_DATA_GET(obj, sd);

   return elm_image_aspect_fixed_set(sd->icon, fixed);
}

EAPI Eina_Bool
elm_photo_aspect_fixed_get(const Evas_Object *obj)
{
   ELM_PHOTO_CHECK(obj) EINA_FALSE;
   ELM_PHOTO_DATA_GET(obj, sd);

   return elm_image_aspect_fixed_get(sd->icon);
}
