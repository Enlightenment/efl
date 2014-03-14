#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_notify.h"
#include "elm_widget_container.h"

EAPI Eo_Op ELM_OBJ_NOTIFY_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_NOTIFY_CLASS

#define MY_CLASS_NAME "Elm_Notify"
#define MY_CLASS_NAME_LEGACY "elm_notify"

static const char SIG_BLOCK_CLICKED[] = "block,clicked";
static const char SIG_TIMEOUT[] = "timeout";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_BLOCK_CLICKED, ""},
   {SIG_TIMEOUT, ""},
   {NULL, NULL}
};

static void
_notify_theme_apply(Evas_Object *obj)
{
   const char *style = elm_widget_style_get(obj);
   double ax, ay;

   ELM_NOTIFY_DATA_GET(obj, sd);

   ax = sd->horizontal_align;
   ay = sd->vertical_align;
   if ((elm_widget_mirrored_get(obj)) && (ax != ELM_NOTIFY_ALIGN_FILL)) ax = 1.0 - ax;

   if (ay == 0.0)
     elm_widget_theme_object_set(obj, sd->notify, "notify", "top", style);
   else if (ay == 1.0)
     elm_widget_theme_object_set(obj, sd->notify, "notify", "bottom", style);
   else if (ax == 0.0)
     elm_widget_theme_object_set(obj, sd->notify, "notify", "left", style);
   else if (ax == 1.0)
     elm_widget_theme_object_set(obj, sd->notify, "notify", "right", style);
   else
     elm_widget_theme_object_set(obj, sd->notify, "notify", "center", style);
}

/**
 * Moves notification to orientation.
 *
 * This function moves notification to orientation
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
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord x, y, w, h;
   double ax, ay;

   ELM_NOTIFY_DATA_GET(obj, sd);

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   edje_object_size_min_get(sd->notify, &minw, &minh);
   edje_object_size_min_restricted_calc(sd->notify, &minw, &minh, minw, minh);

   ax = sd->horizontal_align;
   ay = sd->vertical_align;
   if ((elm_widget_mirrored_get(obj)) && (ax != ELM_NOTIFY_ALIGN_FILL)) ax = 1.0 - ax;

   if (ax == ELM_NOTIFY_ALIGN_FILL) minw = w;
   if (ay == ELM_NOTIFY_ALIGN_FILL) minh = h;

   x = x + ((w - minw) * ax);
   y = y + ((h - minh) * ay);

   evas_object_move(sd->notify, x, y);
}

static void
_block_events_theme_apply(Evas_Object *obj)
{
   ELM_NOTIFY_DATA_GET(obj, sd);

   const char *style = elm_widget_style_get(obj);

   if (!elm_layout_theme_set
       (sd->block_events, "notify", "block_events", style))
     CRI("Failed to set layout!");
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

static void
_elm_notify_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Elm_Notify_Smart_Data *sd = _pd;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   _notify_theme_apply(obj);
   if (sd->block_events) _block_events_theme_apply(obj);

   edje_object_scale_set
     (sd->notify, elm_widget_scale_get(obj) * elm_config_scale_get());

   _sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
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

   if (sd->horizontal_align == ELM_NOTIFY_ALIGN_FILL) minw = w;
   if (sd->vertical_align == ELM_NOTIFY_ALIGN_FILL) minh = h;

   if (sd->content)
     {
        _notify_move_to_orientation(obj);
        evas_object_resize(sd->notify, minw, minh);
     }
}

static void
_changed_size_hints_cb(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   _calc(data);
}

static void
_elm_notify_smart_sub_object_del(Eo *obj, void *_pd, va_list *list)
{
   Elm_Notify_Smart_Data *sd = _pd;
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_del(sobj, &int_ret));
   if (!int_ret) return;

   if (sobj == sd->content)
     {
        evas_object_event_callback_del_full
          (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _changed_size_hints_cb, obj);
        sd->content = NULL;
     }

   if (ret) *ret = EINA_TRUE;
}

static void
_block_area_clicked_cb(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   evas_object_smart_callback_call(data, SIG_BLOCK_CLICKED, NULL);
}

static void
_restack_cb(void *data EINA_UNUSED,
            Evas *e EINA_UNUSED,
            Evas_Object *obj,
            void *event_info EINA_UNUSED)
{
   ELM_NOTIFY_DATA_GET(obj, sd);

   evas_object_layer_set(sd->notify, evas_object_layer_get(obj));
}

static void
_elm_notify_smart_resize(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   _calc(obj);
}

static void
_elm_notify_smart_move(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   _calc(obj);
}

static Eina_Bool
_timer_cb(void *data)
{
   const char *hide_signal;
   Evas_Object *obj = data;

   ELM_NOTIFY_DATA_GET(obj, sd);

   sd->timer = NULL;
   if (!evas_object_visible_get(obj)) goto end;

   hide_signal = edje_object_data_get(sd->notify, "hide_finished_signal");
   if ((hide_signal) && (!strcmp(hide_signal, "on")))
     {
        sd->in_timeout = EINA_TRUE;
        edje_object_signal_emit(sd->notify, "elm,state,hide", "elm");
     }
   else //for backport supporting: edc without emitting hide finished signal
     {
        evas_object_hide(obj);
     }
   evas_object_smart_callback_call(obj, SIG_TIMEOUT, NULL);

end:
   return ECORE_CALLBACK_CANCEL;
}

static void
_timer_init(Evas_Object *obj,
            Elm_Notify_Smart_Data *sd)
{
   ecore_timer_del(sd->timer);
   if (sd->timeout > 0.0)
     sd->timer = ecore_timer_add(sd->timeout, _timer_cb, obj);
   else
     sd->timer = NULL;
}

static void
_elm_notify_smart_show(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Notify_Smart_Data *sd = _pd;

   sd->had_hidden = EINA_FALSE;
   sd->in_timeout = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());

   evas_object_show(sd->notify);
   if (!sd->allow_events) evas_object_show(sd->block_events);
   _timer_init(obj, sd);
   elm_object_focus_set(obj, EINA_TRUE);
}

static void
_elm_notify_smart_hide(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   const char *hide_signal;
   Elm_Notify_Smart_Data *sd = _pd;

   if (sd->had_hidden && !sd->in_timeout)
     return;
   eo_do_super(obj, MY_CLASS, evas_obj_smart_hide());

   hide_signal = edje_object_data_get(sd->notify, "hide_finished_signal");
   if ((hide_signal) && (!strcmp(hide_signal, "on")))
     {
        if (!sd->in_timeout)
          edje_object_signal_emit(sd->notify, "elm,state,hide", "elm");
     }
   else //for backport supporting: edc without emitting hide finished signal
     {
        evas_object_hide(sd->notify);
        if (sd->allow_events) evas_object_hide(sd->block_events);
     }
   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
}

static void
_parent_del_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   elm_notify_parent_set(data, NULL);
   evas_object_hide(data);
}

static void
_parent_hide_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   evas_object_hide(data);
}

static void
_elm_notify_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_notify_smart_focus_next(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object *cur;
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;
   Elm_Notify_Smart_Data *sd = _pd;

   if (!sd->content) return;

   cur = sd->content;

   /* Try to cycle focus on content */
   int_ret = elm_widget_focus_next_get(cur, dir, next);
   if (ret) *ret = int_ret;
}

static void
_elm_notify_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_notify_smart_focus_direction(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object *cur;

   Evas_Object *base = va_arg(*list, Evas_Object *);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   Elm_Notify_Smart_Data *sd = _pd;

   if (!sd->content) return;

   cur = sd->content;

   int_ret = elm_widget_focus_direction_get(cur, base, degree, direction, weight);
   if (ret) *ret = int_ret;
}

static void
_elm_notify_smart_content_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Notify_Smart_Data *sd = _pd;

   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   if (part && strcmp(part, "default")) return;
   if (sd->content == content) goto end;

   evas_object_del(sd->content);
   sd->content = content;

   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_event_callback_add
          (content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _changed_size_hints_cb, obj);
        edje_object_part_swallow(sd->notify, "elm.swallow.content", content);
     }

   _sizing_eval(obj);
   _calc(obj);

end:
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_notify_smart_content_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Notify_Smart_Data *sd = _pd;
   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   *ret = NULL;

   if (part && strcmp(part, "default")) return;

   *ret = sd->content;
}

static void
_elm_notify_smart_content_unset(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *content;

   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   if (ret) *ret = NULL;

   Elm_Notify_Smart_Data *sd = _pd;

   if (part && strcmp(part, "default")) return;
   if (!sd->content) return;

   content = sd->content;
   elm_widget_sub_object_del(obj, sd->content);
   edje_object_part_unswallow(sd->notify, content);

   if (ret) *ret = content;
}

static void
_hide_finished_cb(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  const char *emission EINA_UNUSED,
                  const char *source EINA_UNUSED)
{
   ELM_NOTIFY_DATA_GET(data, sd);
   sd->had_hidden = EINA_TRUE;
   evas_object_hide(sd->notify);
   if (!sd->allow_events) evas_object_hide(sd->block_events);
}

static void
_elm_notify_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Notify_Smart_Data *priv = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->allow_events = EINA_TRUE;

   priv->notify = edje_object_add(evas_object_evas_get(obj));
   priv->horizontal_align = 0.5;
   priv->vertical_align = 0.0;

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESTACK, _restack_cb, obj);
   edje_object_signal_callback_add
      (priv->notify, "elm,action,hide,finished", "elm", _hide_finished_cb, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_notify_align_set(obj, 0.5, 0.0);
}

static void
_elm_notify_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Notify_Smart_Data *sd = _pd;

   edje_object_signal_callback_del_full
      (sd->notify, "elm,action,hide,finished", "elm", _hide_finished_cb, obj);
   elm_notify_parent_set(obj, NULL);
   elm_notify_allow_events_set(obj, EINA_FALSE);
   ecore_timer_del(sd->timer);

   ELM_SAFE_FREE(sd->notify, evas_object_del);
   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_notify_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY));
}

EAPI void
elm_notify_parent_set(Evas_Object *obj,
                      Evas_Object *parent)
{
   ELM_NOTIFY_CHECK(obj);
   eo_do(obj, elm_obj_widget_parent_set(parent));
}

static void
_elm_notify_smart_parent_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *parent = va_arg(*list, Evas_Object *);
   Elm_Notify_Smart_Data *sd = _pd;

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
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_widget_parent_get(&ret));
   return ret;
}

static void
_elm_notify_smart_parent_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Notify_Smart_Data *sd = _pd;
   *ret = sd->parent;
}

EINA_DEPRECATED EAPI void
elm_notify_orient_set(Evas_Object *obj,
                      Elm_Notify_Orient orient)
{
   double horizontal = 0, vertical = 0;

   switch (orient)
     {
      case ELM_NOTIFY_ORIENT_TOP:
         horizontal = 0.5; vertical = 0.0;
        break;

      case ELM_NOTIFY_ORIENT_CENTER:
         horizontal = 0.5; vertical = 0.5;
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM:
         horizontal = 0.5; vertical = 1.0;
        break;

      case ELM_NOTIFY_ORIENT_LEFT:
         horizontal = 0.0; vertical = 0.5;
        break;

      case ELM_NOTIFY_ORIENT_RIGHT:
         horizontal = 1.0; vertical = 0.5;
        break;

      case ELM_NOTIFY_ORIENT_TOP_LEFT:
         horizontal = 0.0; vertical = 0.0;
        break;

      case ELM_NOTIFY_ORIENT_TOP_RIGHT:
         horizontal = 1.0; vertical = 0.0;
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM_LEFT:
         horizontal = 0.0; vertical = 1.0;
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM_RIGHT:
         horizontal = 1.0; vertical = 1.0;
        break;

      case ELM_NOTIFY_ORIENT_LAST:
        break;
     }
   elm_notify_align_set(obj, horizontal, vertical);
}

EINA_DEPRECATED EAPI Elm_Notify_Orient
elm_notify_orient_get(const Evas_Object *obj)
{
   Elm_Notify_Orient orient;
   double horizontal, vertical;

   elm_notify_align_get(obj, &horizontal, &vertical);

   if ((horizontal == 0.5) && (vertical == 0.0))
     orient = ELM_NOTIFY_ORIENT_TOP;
   else if ((horizontal == 0.5) && (vertical == 0.5))
     orient = ELM_NOTIFY_ORIENT_CENTER;
   else if ((horizontal == 0.5) && (vertical == 1.0))
     orient = ELM_NOTIFY_ORIENT_BOTTOM;
   else if ((horizontal == 0.0) && (vertical == 0.5))
     orient = ELM_NOTIFY_ORIENT_LEFT;
   else if ((horizontal == 1.0) && (vertical == 0.5))
     orient = ELM_NOTIFY_ORIENT_RIGHT;
   else if ((horizontal == 0.0) && (vertical == 0.0))
     orient = ELM_NOTIFY_ORIENT_TOP_LEFT;
   else if ((horizontal == 1.0) && (vertical == 0.0))
     orient = ELM_NOTIFY_ORIENT_TOP_RIGHT;
   else if ((horizontal == 0.0) && (vertical == 1.0))
     orient = ELM_NOTIFY_ORIENT_BOTTOM_LEFT;
   else if ((horizontal == 1.0) && (vertical == 1.0))
     orient = ELM_NOTIFY_ORIENT_BOTTOM_RIGHT;
   else
     orient = ELM_NOTIFY_ORIENT_TOP;

   return orient;
}

EAPI void
elm_notify_timeout_set(Evas_Object *obj,
                       double timeout)
{
   ELM_NOTIFY_CHECK(obj);
   eo_do(obj, elm_obj_notify_timeout_set(timeout));
}

static void
_timeout_set(Eo *obj, void *_pd, va_list *list)
{
   double timeout = va_arg(*list, double);
   Elm_Notify_Smart_Data *sd = _pd;

   sd->timeout = timeout;
   _timer_init(obj, sd);
}

EAPI double
elm_notify_timeout_get(const Evas_Object *obj)
{
   ELM_NOTIFY_CHECK(obj) 0.0;
   double ret = 0.0;
   eo_do((Eo *) obj, elm_obj_notify_timeout_get(&ret));
   return ret;
}

static void
_timeout_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Notify_Smart_Data *sd = _pd;
   *ret = sd->timeout;
}

EAPI void
elm_notify_allow_events_set(Evas_Object *obj,
                            Eina_Bool allow)
{
   ELM_NOTIFY_CHECK(obj);
   eo_do(obj, elm_obj_notify_allow_events_set(allow));
}

static void
_allow_events_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool allow = va_arg(*list, int);
   Elm_Notify_Smart_Data *sd = _pd;

   if (allow == sd->allow_events) return;
   sd->allow_events = allow;
   if (!allow)
     {
        sd->block_events = elm_layout_add(obj);
        _block_events_theme_apply(obj);
        elm_widget_resize_object_set(obj, sd->block_events, EINA_TRUE);
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
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_notify_allow_events_get(&ret));
   return ret;
}

static void
_allow_events_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Notify_Smart_Data *sd = _pd;
   *ret = sd->allow_events;
}

EAPI void
elm_notify_align_set(Evas_Object *obj, double horizontal, double vertical)
{
   ELM_NOTIFY_CHECK(obj);
   eo_do(obj, elm_obj_notify_align_set(horizontal, vertical));
}

static void
_align_set(Eo *obj, void *_pd, va_list *list)
{
   double horizontal = va_arg(*list, double);
   double vertical = va_arg(*list, double);
   Elm_Notify_Smart_Data *sd = _pd;

   sd->horizontal_align = horizontal;
   sd->vertical_align = vertical;

   _notify_theme_apply(obj);
   _calc(obj);
}

EAPI void
elm_notify_align_get(const Evas_Object *obj, double *horizontal, double *vertical)
{
   ELM_NOTIFY_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_notify_align_get(horizontal, vertical));
}

static void
_align_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *horizontal = va_arg(*list, double *);
   double *vertical = va_arg(*list, double *);
   Elm_Notify_Smart_Data *sd = _pd;

   if (horizontal)
     *horizontal = sd->horizontal_align;
   if (vertical)
     *vertical = sd->vertical_align;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_notify_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_notify_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _elm_notify_smart_resize),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _elm_notify_smart_move),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW), _elm_notify_smart_show),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE), _elm_notify_smart_hide),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_PARENT_SET), _elm_notify_smart_parent_set),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_PARENT_GET), _elm_notify_smart_parent_get),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_notify_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_notify_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_notify_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_notify_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION), _elm_notify_smart_focus_direction),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_notify_smart_sub_object_del),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_notify_smart_content_set),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET), _elm_notify_smart_content_get),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET), _elm_notify_smart_content_unset),

        EO_OP_FUNC(ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_TIMEOUT_SET), _timeout_set),
        EO_OP_FUNC(ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_TIMEOUT_GET), _timeout_get),
        EO_OP_FUNC(ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_ALLOW_EVENTS_SET), _allow_events_set),
        EO_OP_FUNC(ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_ALLOW_EVENTS_GET), _allow_events_get),
        EO_OP_FUNC(ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_ALIGN_SET), _align_set),
        EO_OP_FUNC(ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_ALIGN_GET), _align_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_NOTIFY_SUB_ID_TIMEOUT_SET, "Set the time interval after which the notify window is going to be hidden."),
     EO_OP_DESCRIPTION(ELM_OBJ_NOTIFY_SUB_ID_TIMEOUT_GET, "Return the timeout value (in seconds)."),
     EO_OP_DESCRIPTION(ELM_OBJ_NOTIFY_SUB_ID_ALLOW_EVENTS_SET, "Sets whether events should be passed to by a click outside its area."),
     EO_OP_DESCRIPTION(ELM_OBJ_NOTIFY_SUB_ID_ALLOW_EVENTS_GET, "Return true if events are allowed below the notify object."),
     EO_OP_DESCRIPTION(ELM_OBJ_NOTIFY_SUB_ID_ALIGN_SET, "Set the alignment."),
     EO_OP_DESCRIPTION(ELM_OBJ_NOTIFY_SUB_ID_ALIGN_GET, "Return the alignment."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_NOTIFY_BASE_ID, op_desc, ELM_OBJ_NOTIFY_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Notify_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_notify_class_get, &class_desc, ELM_OBJ_CONTAINER_CLASS, NULL);
