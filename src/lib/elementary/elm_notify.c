#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_WIDGET_PROTECTED
#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_notify.h"
#include "elm_widget_container.h"

#include "elm_notify_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_NOTIFY_CLASS

#define MY_CLASS_NAME "Elm_Notify"
#define MY_CLASS_NAME_LEGACY "elm_notify"

static Efl_Ui_Theme_Apply
_notify_theme_apply(Evas_Object *obj)
{
   const char *style = elm_widget_style_get(obj);
   const char *position;
   double ax, ay;

   ELM_NOTIFY_DATA_GET(obj, sd);

   ax = sd->horizontal_align;
   ay = sd->vertical_align;

   if (ay == 0.0)
     {
        if (ax == 0.0)
          position = "top_left";
        else if (ax == 1.0)
          position = "top_right";
        else
          position = "top";
     }
   else if (ay == 1.0)
     {
        if (ax == 0.0)
          position = "bottom_left";
        else if (ax == 1.0)
          position = "bottom_right";
        else
          position = "bottom";
     }
   else
     {
        if (ax == 0.0)
          position = "left";
        else if (ax == 1.0)
          position = "right";
        else
          position = "center";
     }

   return elm_widget_theme_object_set(obj, sd->notify, "notify", position, style);
}

/**
 * Moves notification to orientation.
 *
 * This function moves notification to orientation
 * according to object RTL orientation.
 *
 * @param obj notification object.
 *
 * @param x, y, w, h geometry of object.
 *
 * @internal
 **/
static void
_notify_move_to_orientation(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Coord minw = -1, minh = -1;
   double ax, ay;

   ELM_NOTIFY_DATA_GET(obj, sd);

   edje_object_size_min_get(sd->notify, &minw, &minh);
   edje_object_size_min_restricted_calc(sd->notify, &minw, &minh, minw, minh);

   ax = sd->horizontal_align;
   ay = sd->vertical_align;
   if ((efl_ui_mirrored_get(obj)) && (ax != ELM_NOTIFY_ALIGN_FILL)) ax = 1.0 - ax;

   if (ax == ELM_NOTIFY_ALIGN_FILL) minw = w;
   if (ay == ELM_NOTIFY_ALIGN_FILL) minh = h;

   x = x + ((w - minw) * ax);
   y = y + ((h - minh) * ay);

   evas_object_move(sd->notify, x, y);
   evas_object_resize(sd->notify, minw, minh);
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
   Evas_Coord x, y, w, h;

   ELM_NOTIFY_DATA_GET(obj, sd);
   edje_object_mirrored_set(sd->notify, rtl);
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   _notify_move_to_orientation(obj, x, y, w, h);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord x, y, w, h;

   ELM_NOTIFY_DATA_GET(obj, sd);

   if (!sd->parent) return;
   evas_object_geometry_get(sd->parent, &x, &y, &w, &h);
   if (efl_isa(sd->parent, EFL_UI_WIN_CLASS))
     {
        x = 0;
        y = 0;
     }
   evas_object_move(obj, x, y);
   evas_object_resize(obj, w, h);
}

EOLIAN static Efl_Ui_Theme_Apply
_elm_notify_efl_ui_widget_theme_apply(Eo *obj, Elm_Notify_Data *sd)
{
   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   int_ret &= _notify_theme_apply(obj);
   if (sd->block_events) _block_events_theme_apply(obj);

   edje_object_scale_set
     (sd->notify, efl_gfx_entity_scale_get(obj) * elm_config_scale_get());

   _sizing_eval(obj);

   return int_ret;
}

/* Legacy compat. Note that notify has no text parts in the default theme... */
static void
_elm_notify_text_set(Eo *obj EINA_UNUSED, Elm_Notify_Data *sd, const char *part, const char *label)
{
   edje_object_part_text_set(sd->notify, part, label);
}

/* Legacy compat. Note that notify has no text parts in the default theme... */
static const char*
_elm_notify_text_get(Eo *obj EINA_UNUSED, Elm_Notify_Data *sd, const char *part)
{
   return edje_object_part_text_get(sd->notify, part);
}

static void
_calc(Evas_Object *obj)
{
   Evas_Coord x, y, w, h;

   ELM_NOTIFY_DATA_GET(obj, sd);

   _sizing_eval(obj);

   evas_object_geometry_get(obj, &x, &y, &w, &h);

   if (sd->content)
     {
        _notify_move_to_orientation(obj, x, y, w, h);
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

EOLIAN static Eina_Bool
_elm_notify_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Notify_Data *sd, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;
   int_ret = elm_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   if (sobj == sd->content)
     {
        evas_object_event_callback_del_full
          (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _changed_size_hints_cb, obj);
        sd->content = NULL;
     }

   return EINA_TRUE;
}

static void
_block_area_clicked_cb(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, ELM_NOTIFY_EVENT_BLOCK_CLICKED, NULL);
}

EOLIAN static void
_elm_notify_efl_gfx_entity_size_set(Eo *obj, Elm_Notify_Data *sd, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);

   if (!sd->parent && sd->content)
     {
        Eina_Position2D pos;

        pos = efl_gfx_entity_position_get(obj);
        _notify_move_to_orientation(obj, pos.x, pos.y, sz.w, sz.h);
     }
}

EOLIAN static void
_elm_notify_efl_gfx_entity_position_set(Eo *obj, Elm_Notify_Data *sd, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);

   if (!sd->parent && sd->content)
     {
        Evas_Coord w, h;

        evas_object_geometry_get(obj, NULL, NULL, &w, &h);
        _notify_move_to_orientation(obj, pos.x, pos.y, w, h);
     }
}

static Eina_Bool
_timer_cb(void *data)
{
   Evas_Object *obj = data;

   ELM_NOTIFY_DATA_GET(obj, sd);

   sd->timer = NULL;
   if (!evas_object_visible_get(obj)) goto end;

   evas_object_hide(obj);
   sd->in_timeout = EINA_TRUE;
   efl_event_callback_legacy_call(obj, ELM_NOTIFY_EVENT_TIMEOUT, NULL);

end:
   return ECORE_CALLBACK_CANCEL;
}

static void
_timer_init(Evas_Object *obj,
            Elm_Notify_Data *sd)
{
   ecore_timer_del(sd->timer);
   if (sd->timeout > 0.0)
     sd->timer = ecore_timer_add(sd->timeout, _timer_cb, obj);
   else
     sd->timer = NULL;
}

static void
_elm_notify_show(Eo *obj, Elm_Notify_Data *sd)
{
   sd->had_hidden = EINA_FALSE;
   sd->in_timeout = EINA_FALSE;
   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), EINA_TRUE);

   evas_object_show(sd->notify);
   if (!sd->allow_events) evas_object_show(sd->block_events);
   _timer_init(obj, sd);
   elm_object_focus_set(obj, EINA_TRUE);
}

static void
_elm_notify_hide(Eo *obj EINA_UNUSED, Elm_Notify_Data *sd)
{
   const char *hide_signal;

   if (sd->had_hidden && !sd->in_timeout)
     return;

   hide_signal = edje_object_data_get(sd->notify, "hide_finished_signal");
   if (eina_streq(hide_signal, "on"))
     {
        if (!sd->in_timeout)
          {
             elm_layout_signal_emit(sd->block_events, "elm,state,hide", "elm");
             edje_object_signal_emit(sd->notify, "elm,state,hide", "elm");
          }
     }
   else //for backport supporting: edc without emitting hide finished signal
     {
        efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), EINA_FALSE);
        evas_object_hide(sd->notify);
        if (sd->allow_events) evas_object_hide(sd->block_events);
     }
   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
}

EOLIAN static void
_elm_notify_efl_gfx_entity_visible_set(Eo *obj, Elm_Notify_Data *sd, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   if (vis) _elm_notify_show(obj, sd);
   else _elm_notify_hide(obj, sd);
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

static Eina_Bool
_elm_notify_content_set(Eo *obj, Elm_Notify_Data *sd, const char *part, Evas_Object *content)
{
   if (part && strcmp(part, "default")) return EINA_FALSE;
   if (sd->content == content) return EINA_TRUE;

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

   _calc(obj);

   return EINA_TRUE;
}

static Evas_Object*
_elm_notify_content_get(const Eo *obj EINA_UNUSED, Elm_Notify_Data *sd, const char *part)
{
   if (part && strcmp(part, "default")) return NULL;

   return sd->content;
}

static Evas_Object*
_elm_notify_content_unset(Eo *obj, Elm_Notify_Data *sd, const char *part)
{
   Evas_Object *content;

   if (part && strcmp(part, "default")) return NULL;
   if (!sd->content) return NULL;

   content = sd->content;
   _elm_widget_sub_object_redirect_to_top(obj, sd->content);
   edje_object_part_unswallow(sd->notify, content);

   return content;
}

EOLIAN static Eina_Bool
_elm_notify_efl_content_content_set(Eo *obj, Elm_Notify_Data *sd, Evas_Object *content)
{
   return _elm_notify_content_set(obj, sd, NULL, content);
}

EOLIAN static Evas_Object*
_elm_notify_efl_content_content_get(const Eo *obj EINA_UNUSED, Elm_Notify_Data *sd)
{
   return _elm_notify_content_get(obj, sd, NULL);
}

EOLIAN static Evas_Object*
_elm_notify_efl_content_content_unset(Eo *obj, Elm_Notify_Data *sd)
{
   return _elm_notify_content_unset(obj, sd, NULL);
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
   efl_gfx_entity_visible_set(efl_super(data, MY_CLASS), EINA_FALSE);
   efl_event_callback_legacy_call(data, ELM_NOTIFY_EVENT_DISMISSED, NULL);
}

EOLIAN static void
_elm_notify_efl_canvas_group_group_add(Eo *obj, Elm_Notify_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->allow_events = EINA_TRUE;

   priv->notify = edje_object_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->notify, obj);

   edje_object_signal_callback_add
      (priv->notify, "elm,action,hide,finished", "elm", _hide_finished_cb, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_notify_align_set(obj, 0.5, 0.0);
}

EOLIAN static void
_elm_notify_efl_canvas_group_group_del(Eo *obj, Elm_Notify_Data *sd)
{
   edje_object_signal_callback_del_full
      (sd->notify, "elm,action,hide,finished", "elm", _hide_finished_cb, obj);
   elm_notify_parent_set(obj, NULL);
   // Allow events set with TRUE will lead to the destruction of sd->block_events.
   elm_notify_allow_events_set(obj, EINA_TRUE);
   ecore_timer_del(sd->timer);

   ELM_SAFE_FREE(sd->notify, evas_object_del);
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EAPI Evas_Object *
elm_notify_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_notify_efl_object_constructor(Eo *obj, Elm_Notify_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_NOTIFICATION);

   return obj;
}

EAPI void
elm_notify_parent_set(Evas_Object *obj,
                      Evas_Object *parent)
{
   ELM_NOTIFY_CHECK(obj);
   efl_ui_widget_parent_set(obj, parent);
}

EOLIAN static void
_elm_notify_efl_ui_widget_widget_parent_set(Eo *obj, Elm_Notify_Data *sd, Evas_Object *parent)
{
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
     }

   _calc(obj);
}

EAPI Evas_Object *
elm_notify_parent_get(const Evas_Object *obj)
{
   ELM_NOTIFY_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   ret = efl_ui_widget_parent_get((Eo *) obj);
   return ret;
}

EOLIAN static Evas_Object*
_elm_notify_efl_ui_widget_widget_parent_get(const Eo *obj EINA_UNUSED, Elm_Notify_Data *sd)
{
   return sd->parent;
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

EOLIAN static void
_elm_notify_timeout_set(Eo *obj, Elm_Notify_Data *sd, double timeout)
{
   sd->timeout = timeout;
   _timer_init(obj, sd);
}

EOLIAN static double
_elm_notify_timeout_get(const Eo *obj EINA_UNUSED, Elm_Notify_Data *sd)
{
   return sd->timeout;
}

EOLIAN static void
_elm_notify_allow_events_set(Eo *obj, Elm_Notify_Data *sd, Eina_Bool allow)
{
   if (allow == sd->allow_events) return;
   sd->allow_events = allow;
   if (!allow)
     {
        sd->block_events = elm_layout_add(obj);
        _block_events_theme_apply(obj);
        elm_widget_resize_object_set(obj, sd->block_events);
        evas_object_stack_above(sd->notify, sd->block_events);
        elm_layout_signal_callback_add
          (sd->block_events, "elm,action,click", "elm",
          _block_area_clicked_cb, obj);
     }
   else
     {
        evas_object_del(sd->block_events);
        sd->block_events = NULL;
     }
}

EOLIAN static Eina_Bool
_elm_notify_allow_events_get(const Eo *obj EINA_UNUSED, Elm_Notify_Data *sd)
{
   return sd->allow_events;
}

EOLIAN static void
_elm_notify_align_set(Eo *obj, Elm_Notify_Data *sd, double horizontal, double vertical)
{
   sd->horizontal_align = horizontal;
   sd->vertical_align = vertical;

   _notify_theme_apply(obj);
   _calc(obj);
}

EOLIAN static void
_elm_notify_dismiss(Eo *obj EINA_UNUSED, Elm_Notify_Data *sd)
{
   elm_layout_signal_emit(sd->block_events, "elm,state,hide", "elm");
   edje_object_signal_emit(sd->notify, "elm,state,hide", "elm");
}


EOLIAN static void
_elm_notify_align_get(const Eo *obj EINA_UNUSED, Elm_Notify_Data *sd, double *horizontal, double *vertical)
{
   if (horizontal)
     *horizontal = sd->horizontal_align;
   if (vertical)
     *vertical = sd->vertical_align;
}

static void
_elm_notify_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(elm_notify, ELM_NOTIFY, Elm_Notify_Data)
ELM_PART_OVERRIDE_TEXT_SET(elm_notify, ELM_NOTIFY, Elm_Notify_Data)
ELM_PART_OVERRIDE_TEXT_GET(elm_notify, ELM_NOTIFY, Elm_Notify_Data)
ELM_PART_OVERRIDE_CONTENT_SET(elm_notify, ELM_NOTIFY, Elm_Notify_Data)
ELM_PART_OVERRIDE_CONTENT_GET(elm_notify, ELM_NOTIFY, Elm_Notify_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(elm_notify, ELM_NOTIFY, Elm_Notify_Data)
ELM_PART_CONTENT_DEFAULT_GET(elm_notify, "default")

EOLIAN static const char *
_elm_notify_part_efl_ui_translatable_translatable_text_get(const Eo *obj, void *_pd EINA_UNUSED, const char **domain)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return elm_widget_part_translatable_text_get(pd->obj, pd->part, domain);
}

EOLIAN static void
_elm_notify_part_efl_ui_translatable_translatable_text_set(Eo *obj, void *_pd EINA_UNUSED, const char *label, const char *domain)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   elm_widget_part_translatable_text_set(pd->obj, pd->part, label, domain);
}

#include "elm_notify_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define ELM_NOTIFY_EXTRA_OPS \
   ELM_PART_CONTENT_DEFAULT_OPS(elm_notify), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_notify)

#include "elm_notify.eo.c"
