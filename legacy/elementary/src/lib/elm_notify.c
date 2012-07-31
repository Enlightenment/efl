#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_notify.h"

EAPI const char ELM_NOTIFY_SMART_NAME[] = "elm_notify";

static const char SIG_BLOCK_CLICKED[] = "block,clicked";
static const char SIG_TIMEOUT[] = "timeout";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_BLOCK_CLICKED, ""},
   {SIG_TIMEOUT, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_NOTIFY_SMART_NAME, _elm_notify, Elm_Notify_Smart_Class,
  Elm_Container_Smart_Class, elm_container_smart_class_get, NULL);

/**
 * Return Notification orientation with RTL
 *
 * This function switches-sides of notification area when in RTL mode.
 *
 * @param obj notification object.
 *
 * @param orient Original notification orientation.
 *
 * @return notification orientation with respect to the object RTL mode.
 *
 * @internal
 **/
static Elm_Notify_Orient
_notify_orientation_rtl_fix(Evas_Object *obj,
                            Elm_Notify_Orient orient)
{
   if (elm_widget_mirrored_get(obj))
     {
        switch (orient)
          {
           case ELM_NOTIFY_ORIENT_LEFT:
             orient = ELM_NOTIFY_ORIENT_RIGHT;
             break;

           case ELM_NOTIFY_ORIENT_RIGHT:
             orient = ELM_NOTIFY_ORIENT_LEFT;
             break;

           case ELM_NOTIFY_ORIENT_TOP_LEFT:
             orient = ELM_NOTIFY_ORIENT_TOP_RIGHT;
             break;

           case ELM_NOTIFY_ORIENT_TOP_RIGHT:
             orient = ELM_NOTIFY_ORIENT_TOP_LEFT;
             break;

           case ELM_NOTIFY_ORIENT_BOTTOM_LEFT:
             orient = ELM_NOTIFY_ORIENT_BOTTOM_RIGHT;
             break;

           case ELM_NOTIFY_ORIENT_BOTTOM_RIGHT:
             orient = ELM_NOTIFY_ORIENT_BOTTOM_LEFT;
             break;

           default:
             break;
          }
     }

   return orient;
}

static void
_notify_theme_apply(Evas_Object *obj)
{
   const char *style = elm_widget_style_get(obj);

   ELM_NOTIFY_DATA_GET(obj, sd);

   switch (sd->orient)
     {
      case ELM_NOTIFY_ORIENT_TOP:
        elm_widget_theme_object_set(obj, sd->notify, "notify", "top", style);
        break;

      case ELM_NOTIFY_ORIENT_CENTER:
        elm_widget_theme_object_set
          (obj, sd->notify, "notify", "center", style);
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM:
        elm_widget_theme_object_set
          (obj, sd->notify, "notify", "bottom", style);
        break;

      case ELM_NOTIFY_ORIENT_LEFT:
        elm_widget_theme_object_set(obj, sd->notify, "notify", "left", style);
        break;

      case ELM_NOTIFY_ORIENT_RIGHT:
        elm_widget_theme_object_set(obj, sd->notify, "notify", "right", style);
        break;

      case ELM_NOTIFY_ORIENT_TOP_LEFT:
        elm_widget_theme_object_set
          (obj, sd->notify, "notify", "top_left", style);
        break;

      case ELM_NOTIFY_ORIENT_TOP_RIGHT:
        elm_widget_theme_object_set
          (obj, sd->notify, "notify", "top_right", style);
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM_LEFT:
        elm_widget_theme_object_set
          (obj, sd->notify, "notify", "bottom_left", style);
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM_RIGHT:
        elm_widget_theme_object_set
          (obj, sd->notify, "notify", "bottom_right", style);
        break;

      case ELM_NOTIFY_ORIENT_LAST:
        break;
     }
}

/**
 * Moves notification to orientation.
 *
 * This fucntion moves notification to orientation
 * according to object RTL orientation.
 *
 * @param obj notification object.
 *
 * @param orient notification orientation.
 *
 * @internal
 **/
static void
_notify_move_to_orientation(Evas_Object *obj)
{
   int offx;
   int offy;
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord x, y, w, h;

   ELM_NOTIFY_DATA_GET(obj, sd);

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   edje_object_size_min_get(sd->notify, &minw, &minh);
   edje_object_size_min_restricted_calc(sd->notify, &minw, &minh, minw, minh);
   offx = (w - minw) / 2;
   offy = (h - minh) / 2;

   switch (_notify_orientation_rtl_fix(obj, sd->orient))
     {
      case ELM_NOTIFY_ORIENT_TOP:
        evas_object_move(sd->notify, x + offx, y);
        break;

      case ELM_NOTIFY_ORIENT_CENTER:
        evas_object_move(sd->notify, x + offx, y + offy);
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM:
        evas_object_move(sd->notify, x + offx, y + h - minh);
        break;

      case ELM_NOTIFY_ORIENT_LEFT:
        evas_object_move(sd->notify, x, y + offy);
        break;

      case ELM_NOTIFY_ORIENT_RIGHT:
        evas_object_move(sd->notify, x + w - minw, y + offy);
        break;

      case ELM_NOTIFY_ORIENT_TOP_LEFT:
        evas_object_move(sd->notify, x, y);
        break;

      case ELM_NOTIFY_ORIENT_TOP_RIGHT:
        evas_object_move(sd->notify, x + w - minw, y);
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM_LEFT:
        evas_object_move(sd->notify, x, y + h - minh);
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM_RIGHT:
        evas_object_move(sd->notify, x + w - minw, y + h - minh);
        break;

      case ELM_NOTIFY_ORIENT_LAST:
        break;
     }
}

static void
_block_events_theme_apply(Evas_Object *obj)
{
   ELM_NOTIFY_DATA_GET(obj, sd);

   const char *style = elm_widget_style_get(obj);

   elm_layout_theme_set(sd->block_events, "notify", "block_events", style);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   ELM_NOTIFY_DATA_GET(obj, sd);
   edje_object_mirrored_set(sd->notify, rtl);
   _notify_move_to_orientation(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord x, y, w, h;

   ELM_NOTIFY_DATA_GET(obj, sd);

   if (!sd->parent) return;
   evas_object_geometry_get(sd->parent, &x, &y, &w, &h);
   evas_object_move(obj, x, y);
   evas_object_resize(obj, w, h);
}

static Eina_Bool
_elm_notify_smart_theme(Evas_Object *obj)
{
   ELM_NOTIFY_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_notify_parent_sc)->theme(obj)) return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   _notify_theme_apply(obj);
   if (sd->block_events) _block_events_theme_apply(obj);

   edje_object_scale_set
     (sd->notify, elm_widget_scale_get(obj) * elm_config_scale_get());

   _sizing_eval(obj);

   return EINA_TRUE;
}

static void
_calc(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord x, y, w, h;

   ELM_NOTIFY_DATA_GET(obj, sd);

   _sizing_eval(obj);

   evas_object_geometry_get(obj, &x, &y, &w, &h);

   edje_object_size_min_get(sd->notify, &minw, &minh);
   edje_object_size_min_restricted_calc(sd->notify, &minw, &minh, minw, minh);

   if (sd->content)
     {
        _notify_move_to_orientation(obj);
        evas_object_resize(sd->notify, minw, minh);
     }
}

static void
_changed_size_hints_cb(void *data,
                       Evas *e __UNUSED__,
                       Evas_Object *obj __UNUSED__,
                       void *event_info __UNUSED__)
{
   _calc(data);
}

static void
_content_resize_cb(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   _calc(data);
}

static Eina_Bool
_elm_notify_smart_sub_object_del(Evas_Object *obj,
                                 Evas_Object *sobj)
{
   ELM_NOTIFY_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_notify_parent_sc)->sub_object_del(obj, sobj))
     return EINA_FALSE;

   if (sobj == sd->content)
     {
        evas_object_event_callback_del_full
          (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _changed_size_hints_cb, obj);
        evas_object_event_callback_del_full
          (sobj, EVAS_CALLBACK_RESIZE, _content_resize_cb, obj);
        sd->content = NULL;
     }

   return EINA_TRUE;
}

static void
_block_area_clicked_cb(void *data,
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_BLOCK_CLICKED, NULL);
}

static void
_restack_cb(void *data __UNUSED__,
            Evas *e __UNUSED__,
            Evas_Object *obj,
            void *event_info __UNUSED__)
{
   ELM_NOTIFY_DATA_GET(obj, sd);

   evas_object_layer_set(sd->notify, evas_object_layer_get(obj));
}

static void
_elm_notify_smart_resize(Evas_Object *obj,
                         Evas_Coord w,
                         Evas_Coord h)
{
   ELM_WIDGET_CLASS(_elm_notify_parent_sc)->base.resize(obj, w, h);

   _calc(obj);
}

static void
_elm_notify_smart_move(Evas_Object *obj,
                       Evas_Coord x,
                       Evas_Coord y)
{
   ELM_WIDGET_CLASS(_elm_notify_parent_sc)->base.move(obj, x, y);

   _calc(obj);
}

static Eina_Bool
_timer_cb(void *data)
{
   Evas_Object *obj = data;

   ELM_NOTIFY_DATA_GET(obj, sd);

   if (!evas_object_visible_get(obj)) goto end;

   evas_object_hide(obj);
   evas_object_smart_callback_call(obj, SIG_TIMEOUT, NULL);

end:
   sd->timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_timer_init(Evas_Object *obj,
            Elm_Notify_Smart_Data *sd)
{
   if (sd->timer)
     {
        ecore_timer_del(sd->timer);
        sd->timer = NULL;
     }
   if (sd->timeout > 0.0)
     sd->timer = ecore_timer_add(sd->timeout, _timer_cb, obj);
}

static void
_elm_notify_smart_show(Evas_Object *obj)
{
   ELM_NOTIFY_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_notify_parent_sc)->base.show(obj);

   evas_object_show(sd->notify);
   if (!sd->allow_events) evas_object_show(sd->block_events);
   _timer_init(obj, sd);
   elm_object_focus_set(obj, EINA_TRUE);
}

static void
_elm_notify_smart_hide(Evas_Object *obj)
{
   ELM_NOTIFY_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_notify_parent_sc)->base.hide(obj);

   evas_object_hide(sd->notify);
   if (!sd->allow_events) evas_object_hide(sd->block_events);
   if (sd->timer)
     {
        ecore_timer_del(sd->timer);
        sd->timer = NULL;
     }
}

static void
_parent_del_cb(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   elm_notify_parent_set(data, NULL);
   evas_object_hide(data);
}

static void
_parent_hide_cb(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   evas_object_hide(data);
}

static Eina_Bool
_elm_notify_smart_focus_next(const Evas_Object *obj,
                             Elm_Focus_Direction dir,
                             Evas_Object **next)
{
   Evas_Object *cur;

   ELM_NOTIFY_DATA_GET(obj, sd);

   if (!sd->content)
     return EINA_FALSE;

   cur = sd->content;

   /* Try to cycle focus on content */
   return elm_widget_focus_next_get(cur, dir, next);
}

static Eina_Bool
_elm_notify_smart_focus_direction(const Evas_Object *obj,
                                  const Evas_Object *base,
                                  double degree,
                                  Evas_Object **direction,
                                  double *weight)
{
   Evas_Object *cur;

   ELM_NOTIFY_DATA_GET(obj, sd);

   if (!sd->content)
     return EINA_FALSE;

   cur = sd->content;

   return elm_widget_focus_direction_get(cur, base, degree, direction, weight);
}

static Eina_Bool
_elm_notify_smart_content_set(Evas_Object *obj,
                              const char *part,
                              Evas_Object *content)
{
   ELM_NOTIFY_DATA_GET(obj, sd);

   if (part && strcmp(part, "default")) return EINA_FALSE;
   if (sd->content == content) return EINA_TRUE;
   if (sd->content) evas_object_del(sd->content);
   sd->content = content;

   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_event_callback_add
          (content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _changed_size_hints_cb, obj);
        evas_object_event_callback_add
          (content, EVAS_CALLBACK_RESIZE, _content_resize_cb, obj);
        edje_object_part_swallow(sd->notify, "elm.swallow.content", content);
     }

   _sizing_eval(obj);
   _calc(obj);

   return EINA_TRUE;
}

static Evas_Object *
_elm_notify_smart_content_get(const Evas_Object *obj,
                              const char *part)
{
   ELM_NOTIFY_DATA_GET(obj, sd);

   if (part && strcmp(part, "default")) return NULL;

   return sd->content;
}

static Evas_Object *
_elm_notify_smart_content_unset(Evas_Object *obj,
                                const char *part)
{
   Evas_Object *content;

   ELM_NOTIFY_DATA_GET(obj, sd);

   if (part && strcmp(part, "default")) return NULL;
   if (!sd->content) return NULL;

   content = sd->content;
   elm_widget_sub_object_del(obj, sd->content);
   edje_object_part_unswallow(sd->notify, content);

   return content;
}

static void
_elm_notify_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Notify_Smart_Data);

   ELM_WIDGET_CLASS(_elm_notify_parent_sc)->base.add(obj);

   priv->allow_events = EINA_TRUE;

   priv->notify = edje_object_add(evas_object_evas_get(obj));
   priv->orient = -1;

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESTACK, _restack_cb, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_notify_orient_set(obj, ELM_NOTIFY_ORIENT_TOP);
}

static void
_elm_notify_smart_del(Evas_Object *obj)
{
   ELM_NOTIFY_DATA_GET(obj, sd);

   elm_notify_parent_set(obj, NULL);
   elm_notify_allow_events_set(obj, EINA_FALSE);
   if (sd->timer)
     {
        ecore_timer_del(sd->timer);
        sd->timer = NULL;
     }

   ELM_WIDGET_CLASS(_elm_notify_parent_sc)->base.del(obj);
}

static void
_elm_notify_smart_parent_set(Evas_Object *obj,
                             Evas_Object *parent)
{
   elm_notify_parent_set(obj, parent);

   _sizing_eval(obj);
}

static void
_elm_notify_smart_set_user(Elm_Notify_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_notify_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_notify_smart_del;

   ELM_WIDGET_CLASS(sc)->base.resize = _elm_notify_smart_resize;
   ELM_WIDGET_CLASS(sc)->base.move = _elm_notify_smart_move;
   ELM_WIDGET_CLASS(sc)->base.show = _elm_notify_smart_show;
   ELM_WIDGET_CLASS(sc)->base.hide = _elm_notify_smart_hide;

   ELM_WIDGET_CLASS(sc)->parent_set = _elm_notify_smart_parent_set;
   ELM_WIDGET_CLASS(sc)->theme = _elm_notify_smart_theme;
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_notify_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->focus_direction = _elm_notify_smart_focus_direction;

   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_notify_smart_sub_object_del;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_notify_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_get = _elm_notify_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_notify_smart_content_unset;
}

EAPI const Elm_Notify_Smart_Class *
elm_notify_smart_class_get(void)
{
   static Elm_Notify_Smart_Class _sc =
     ELM_NOTIFY_SMART_CLASS_INIT_NAME_VERSION(ELM_NOTIFY_SMART_NAME);
   static const Elm_Notify_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_notify_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_notify_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_notify_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_notify_parent_set(Evas_Object *obj,
                      Evas_Object *parent)
{
   ELM_NOTIFY_CHECK(obj);
   ELM_NOTIFY_DATA_GET(obj, sd);

   if (sd->parent)
     {
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _changed_size_hints_cb, obj);
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_RESIZE, _changed_size_hints_cb, obj);
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_MOVE, _changed_size_hints_cb, obj);
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_DEL, _parent_del_cb, obj);
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_HIDE, _parent_hide_cb, obj);
        sd->parent = NULL;
     }

   if (parent)
     {
        sd->parent = parent;
        evas_object_event_callback_add
          (parent, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _changed_size_hints_cb, obj);
        evas_object_event_callback_add
          (parent, EVAS_CALLBACK_RESIZE, _changed_size_hints_cb, obj);
        evas_object_event_callback_add
          (parent, EVAS_CALLBACK_MOVE, _changed_size_hints_cb, obj);
        evas_object_event_callback_add
          (parent, EVAS_CALLBACK_DEL, _parent_del_cb, obj);
        evas_object_event_callback_add
          (parent, EVAS_CALLBACK_HIDE, _parent_hide_cb, obj);
        _sizing_eval(obj);
     }

   _calc(obj);
}

EAPI Evas_Object *
elm_notify_parent_get(const Evas_Object *obj)
{
   ELM_NOTIFY_CHECK(obj) NULL;
   ELM_NOTIFY_DATA_GET(obj, sd);

   return sd->parent;
}

EAPI void
elm_notify_orient_set(Evas_Object *obj,
                      Elm_Notify_Orient orient)
{
   ELM_NOTIFY_CHECK(obj);
   ELM_NOTIFY_DATA_GET(obj, sd);

   if (sd->orient == orient) return;
   sd->orient = orient;

   _notify_theme_apply(obj);
   _calc(obj);
}

EAPI Elm_Notify_Orient
elm_notify_orient_get(const Evas_Object *obj)
{
   ELM_NOTIFY_CHECK(obj) - 1;
   ELM_NOTIFY_DATA_GET(obj, sd);

   return sd->orient;
}

EAPI void
elm_notify_timeout_set(Evas_Object *obj,
                       double timeout)
{
   ELM_NOTIFY_CHECK(obj);
   ELM_NOTIFY_DATA_GET(obj, sd);

   sd->timeout = timeout;
   _timer_init(obj, sd);
}

EAPI double
elm_notify_timeout_get(const Evas_Object *obj)
{
   ELM_NOTIFY_CHECK(obj) 0.0;
   ELM_NOTIFY_DATA_GET(obj, sd);

   return sd->timeout;
}

EAPI void
elm_notify_allow_events_set(Evas_Object *obj,
                            Eina_Bool allow)
{
   ELM_NOTIFY_CHECK(obj);
   ELM_NOTIFY_DATA_GET(obj, sd);

   if (allow == sd->allow_events) return;
   sd->allow_events = allow;
   if (!allow)
     {
        sd->block_events = elm_layout_add(obj);
        _block_events_theme_apply(obj);
        elm_widget_resize_object_set(obj, sd->block_events);
        elm_layout_signal_callback_add
          (sd->block_events, "elm,action,click", "elm",
          _block_area_clicked_cb, obj);
     }
   else
     evas_object_del(sd->block_events);
}

EAPI Eina_Bool
elm_notify_allow_events_get(const Evas_Object *obj)
{
   ELM_NOTIFY_CHECK(obj) EINA_FALSE;
   ELM_NOTIFY_DATA_GET(obj, sd);

   return sd->allow_events;
}
