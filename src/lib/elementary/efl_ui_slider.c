#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_ACCESS_VALUE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_slider_private.h"
#include "elm_widget_layout.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_SLIDER_CLASS
#define MY_CLASS_PFX efl_ui_slider
#define MY_CLASS_NAME "Efl.Ui.Slider"

#define SLIDER_DELAY_CHANGED_INTERVAL 0.2
#define SLIDER_STEP 0.05

static Eina_Bool _key_action_drag(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"drag", _key_action_drag},
   {NULL, NULL}
};

static Eina_Bool
_delay_change(void *data)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);

   sd->delay = NULL;
   efl_event_callback_call(data, EFL_UI_SLIDER_EVENT_DELAY_CHANGED, NULL);

   if (_elm_config->atspi_mode)
     efl_access_value_changed_signal_emit(data);

   return ECORE_CALLBACK_CANCEL;
}

static inline Eina_Bool
_is_horizontal(Efl_Ui_Dir dir)
{
   return efl_ui_dir_is_horizontal(dir, EINA_TRUE);
}

static inline Eina_Bool
_is_inverted(Efl_Ui_Dir dir)
{
   if ((dir == EFL_UI_DIR_LEFT) || (dir == EFL_UI_DIR_DOWN))
     return EINA_TRUE;

   return EINA_FALSE;
}

static void
_efl_ui_slider_val_fetch(Evas_Object *obj, Efl_Ui_Slider_Data *sd,  Eina_Bool user_event)
{
   Eina_Bool rtl;
   double posx = 0.0, posy = 0.0, pos = 0.0, val;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (elm_widget_is_legacy(obj))
     efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable.slider"),
                           &posx, &posy);
   else
     efl_ui_drag_value_get(efl_part(wd->resize_obj, "efl.dragable.slider"),
                           &posx, &posy);
   if (_is_horizontal(sd->dir)) pos = posx;
   else pos = posy;

   rtl = efl_ui_mirrored_get(obj);
   if ((!rtl && _is_inverted(sd->dir)) ||
       (rtl && ((sd->dir == EFL_UI_DIR_UP) ||
                (sd->dir == EFL_UI_DIR_RIGHT))))
     {
        pos = 1.0 - pos;
     }

   val = (pos * (sd->val_max - sd->val_min)) + sd->val_min;

   if (fabs(val - sd->val) > DBL_EPSILON)
     {
        sd->val = val;
        if (user_event)
          {
             efl_event_callback_call(obj, EFL_UI_SLIDER_EVENT_CHANGED, NULL);
             efl_event_callback_legacy_call(obj, EFL_UI_SLIDER_EVENT_CHANGED, NULL);
             ecore_timer_del(sd->delay);
             sd->delay = ecore_timer_add(SLIDER_DELAY_CHANGED_INTERVAL, _delay_change, obj);
          }
     }
}

static void
_efl_ui_slider_val_set(Evas_Object *obj, Efl_Ui_Slider_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Eina_Bool rtl;
   double pos;

   if (sd->val_max > sd->val_min)
     {
        pos = (sd->val - sd->val_min) / (sd->val_max - sd->val_min);
     }
   else
     {
        pos = 0.0;
     }

   if (pos < 0.0) pos = 0.0;
   else if (pos > 1.0)
     pos = 1.0;

   rtl = efl_ui_mirrored_get(obj);
   if ((!rtl && _is_inverted(sd->dir)) ||
       (rtl && ((sd->dir == EFL_UI_DIR_UP) ||
                (sd->dir == EFL_UI_DIR_RIGHT))))
     {
        pos = 1.0 - pos;
     }

   if (elm_widget_is_legacy(obj))
     efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                           pos, pos);
   else
     efl_ui_drag_value_set(efl_part(wd->resize_obj, "efl.dragable.slider"),
                           pos, pos);

   // emit accessibility event also if value was changed by API
   if (_elm_config->atspi_mode)
     efl_access_value_changed_signal_emit(obj);

   evas_object_smart_changed(obj);
}

static void
_efl_ui_slider_down_knob(Evas_Object *obj, Efl_Ui_Slider_Data *sd EINA_UNUSED, double button_x, double button_y)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (elm_widget_is_legacy(obj))
     efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                           button_x, button_y);
   else
     efl_ui_drag_value_set(efl_part(wd->resize_obj, "efl.dragable.slider"),
                           button_x, button_y);
}

static void
_efl_ui_slider_move_knob(Evas_Object *obj, Efl_Ui_Slider_Data *sd EINA_UNUSED, double button_x, double button_y)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (elm_widget_is_legacy(obj))
     efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                           button_x, button_y);
   else
     efl_ui_drag_value_set(efl_part(wd->resize_obj, "efl.dragable.slider"),
                           button_x, button_y);
}

static void
_slider_update(Evas_Object *obj, Eina_Bool user_event)
{
   efl_ui_slider_val_fetch(obj, user_event);
   evas_object_smart_changed(obj);
}

static void
_drag(void *data,
      Evas_Object *obj EINA_UNUSED,
      const char *emission EINA_UNUSED,
      const char *source EINA_UNUSED)
{
   _slider_update(data, EINA_TRUE);
}

static void
_drag_start(void *data,
            Evas_Object *obj EINA_UNUSED,
            const char *emission EINA_UNUSED,
            const char *source EINA_UNUSED)
{
   if (!efl_ui_focus_object_focus_get(data))
     elm_object_focus_set(data, EINA_TRUE);
   _slider_update(data, EINA_TRUE);
   efl_event_callback_call(data, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_START, NULL);
   elm_widget_scroll_freeze_push(data);
}

static void
_drag_stop(void *data,
           Evas_Object *obj EINA_UNUSED,
           const char *emission EINA_UNUSED,
           const char *source EINA_UNUSED)
{
   _slider_update(data, EINA_TRUE);
   efl_event_callback_call(data, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_STOP, NULL);
   elm_widget_scroll_freeze_pop(data);
}

static void
_drag_step(void *data,
           Evas_Object *obj EINA_UNUSED,
           const char *emission EINA_UNUSED,
           const char *source EINA_UNUSED)
{
   _slider_update(data, EINA_TRUE);
}

static void
_drag_up(void *data,
         Evas_Object *obj,
         const char *emission EINA_UNUSED,
         const char *source EINA_UNUSED)
{
   double step;

   EFL_UI_SLIDER_DATA_GET(data, sd);
   step = sd->step;

   if (_is_inverted(sd->dir)) step *= -1.0;

   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   if (elm_widget_is_legacy(obj))
     efl_ui_drag_step_move(efl_part(wd->resize_obj, "elm.dragable.slider"),
                           step, step);
   else
     efl_ui_drag_step_move(efl_part(wd->resize_obj, "efl.dragable.slider"),
                           step, step);
}

static void
_drag_down(void *data,
           Evas_Object *obj,
           const char *emission EINA_UNUSED,
           const char *source EINA_UNUSED)
{
   double step;

   EFL_UI_SLIDER_DATA_GET(data, sd);
   step = -sd->step;

   if (_is_inverted(sd->dir)) step *= -1.0;

   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   if (elm_widget_is_legacy(obj))
     efl_ui_drag_step_move(efl_part(wd->resize_obj, "elm.dragable.slider"),
                           step, step);
   else
     efl_ui_drag_step_move(efl_part(wd->resize_obj, "efl.dragable.slider"),
                           step, step);
}

static Eina_Bool
_key_action_drag(Evas_Object *obj, const char *params)
{
   EFL_UI_SLIDER_DATA_GET(obj, sd);
   const char *dir = params;

   if (!strcmp(dir, "left"))
     {
        if (!_is_horizontal(sd->dir))
          return EINA_FALSE;
        if (!_is_inverted(sd->dir))
          _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "right"))
     {
        if (!_is_horizontal(sd->dir))
          return EINA_FALSE;
        if (!_is_inverted(sd->dir))
          _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "up"))
     {
        if (_is_horizontal(sd->dir))
          return EINA_FALSE;
        if (_is_inverted(sd->dir))
          _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "down"))
     {
        if (_is_horizontal(sd->dir))
          return EINA_FALSE;
        if (_is_inverted(sd->dir))
          _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
     }
   else return EINA_FALSE;

   return EINA_TRUE;
}

// _slider_efl_ui_widget_widget_event
ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(slider, Efl_Ui_Slider_Data)

EOLIAN static Eina_Bool
_efl_ui_slider_efl_ui_widget_widget_event(Eo *obj, Efl_Ui_Slider_Data *sd, const Efl_Event *eo_event, Evas_Object *src)
{
   Eo *ev = eo_event->info;

   if (eo_event->desc == EFL_EVENT_KEY_DOWN)
     {
        if (!_slider_efl_ui_widget_widget_event(obj, sd, eo_event, src))
          return EINA_FALSE;
     }
   else if (eo_event->desc == EFL_EVENT_KEY_UP)
     {
        return EINA_FALSE;
     }
   else if (eo_event->desc == EFL_EVENT_POINTER_WHEEL)
     {
        if (efl_input_processed_get(ev)) return EINA_FALSE;
        if (efl_input_pointer_wheel_delta_get(ev) < 0)
          {
             if (_is_horizontal(sd->dir))
               _drag_up(obj, NULL, NULL, NULL);
             else
               _drag_down(obj, NULL, NULL, NULL);
          }
        else
          {
             if (_is_horizontal(sd->dir))
               _drag_down(obj, NULL, NULL, NULL);
             else
               _drag_up(obj, NULL, NULL, NULL);
          }
        efl_input_processed_set(ev, EINA_TRUE);
     }
   else return EINA_FALSE;

   _slider_update(obj, EINA_TRUE);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_slider_efl_ui_widget_on_access_activate(Eo *obj, Efl_Ui_Slider_Data *sd, Efl_Ui_Activate act)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (act == EFL_UI_ACTIVATE_DEFAULT) return EINA_FALSE;

   if ((act == EFL_UI_ACTIVATE_UP) ||
       (act == EFL_UI_ACTIVATE_RIGHT))
     {
        if (!_is_inverted(sd->dir))
          _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
     }
   else if ((act == EFL_UI_ACTIVATE_DOWN) ||
            (act == EFL_UI_ACTIVATE_LEFT))
     {
        if (!_is_inverted(sd->dir))
          _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
     }

   _slider_update(obj, EINA_TRUE);

   return EINA_TRUE;
}

static const char *
_theme_group_modify_pos_get(const char *cur_group, const char *search, size_t len)
{
   const char *pos = NULL;
   const char *temp_str = NULL;

   temp_str = cur_group + len - strlen(search);
   if (temp_str >= cur_group)
     {
        if (!strcmp(temp_str, search))
          pos = temp_str;
     }

   return pos;
}

static char *
_efl_ui_slider_theme_group_get(Evas_Object *obj, Efl_Ui_Slider_Data *sd)
{
   const char *pos = NULL;
   const char *cur_group = elm_widget_theme_element_get(obj);
   Eina_Strbuf *new_group = eina_strbuf_new();
   size_t len = 0;

   if (cur_group)
     {
        len = strlen(cur_group);
        pos = _theme_group_modify_pos_get(cur_group, "horizontal", len);
        if (!pos)
          pos = _theme_group_modify_pos_get(cur_group, "vertical", len);

        // TODO: change separator when it is decided.
        //       can skip when prev_group == cur_group
        if (!pos)
          {
             eina_strbuf_append(new_group, cur_group);
             eina_strbuf_append(new_group, "/");
          }
        else
          {
             eina_strbuf_append_length(new_group, cur_group, pos - cur_group);
          }
     }

   if (_is_horizontal(sd->dir))
     eina_strbuf_append(new_group, "horizontal");
   else
     eina_strbuf_append(new_group, "vertical");

   return eina_strbuf_release(new_group);
}

EOLIAN static Efl_Ui_Theme_Apply
_efl_ui_slider_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Slider_Data *sd)
{
   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_FAILED);
   char *group;

   group = _efl_ui_slider_theme_group_get(obj, sd);
   if (group)
     {
        elm_widget_theme_element_set(obj, group);
        free(group);
     }

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;

   if (elm_widget_is_legacy(obj))
     {
        if (_is_inverted(sd->dir))
          efl_layout_signal_emit(obj, "elm,state,inverted,on", "elm");
        else
          efl_layout_signal_emit(obj, "elm,state,inverted,off", "elm");
     }
   else
     {
        if (_is_inverted(sd->dir))
          efl_layout_signal_emit(obj, "efl,state,inverted,on", "efl");
        else
          efl_layout_signal_emit(obj, "efl,state,inverted,off", "efl");
     }

   efl_ui_slider_val_set(obj);

   efl_layout_signal_process(wd->resize_obj, EINA_FALSE);
   evas_object_smart_changed(obj);

   return int_ret;
}

EOLIAN static void
_efl_ui_slider_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Slider_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   efl_gfx_size_hint_restricted_min_set(obj, EINA_SIZE2D(minw, minh));
   efl_gfx_size_hint_max_set(obj, EINA_SIZE2D(-1, -1));
}

static void
_spacer_down_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);

   Evas_Event_Mouse_Down *ev = event_info;
   Eina_Rect sr;
   double button_x = 0.0, button_y = 0.0;

   sd->spacer_down = EINA_TRUE;
   sr = efl_gfx_entity_geometry_get(sd->spacer);
   sd->downx = ev->canvas.x - sr.x;
   sd->downy = ev->canvas.y - sr.y;
   if (_is_horizontal(sd->dir))
     {
        button_x = ((double)ev->canvas.x - (double)sr.x) / (double)sr.w;
        if (button_x > 1) button_x = 1;
        if (button_x < 0) button_x = 0;
     }
   else
     {
        button_y = ((double)ev->canvas.y - (double)sr.y) / (double)sr.h;
        if (button_y > 1) button_y = 1;
        if (button_y < 0) button_y = 0;
     }

   efl_ui_slider_down_knob(data, button_x, button_y);

   if (!efl_ui_focus_object_focus_get(data))
     elm_object_focus_set(data, EINA_TRUE);
   _slider_update(data, EINA_TRUE);
   efl_event_callback_call(data, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_START, NULL);
}

static void
_spacer_move_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);

   Eina_Rect sr;
   double button_x = 0.0, button_y = 0.0;
   Evas_Event_Mouse_Move *ev = event_info;

   if (sd->spacer_down)
     {
        Evas_Coord d = 0;

        sr = efl_gfx_entity_geometry_get(sd->spacer);
        if (_is_horizontal(sd->dir))
          d = abs(ev->cur.canvas.x - sr.x - sd->downx);
        else d = abs(ev->cur.canvas.y - sr.y - sd->downy);
        if (d > (_elm_config->thumbscroll_threshold - 1))
          {
             if (!sd->frozen)
               {
                  elm_widget_scroll_freeze_push(data);
                  sd->frozen = EINA_TRUE;
               }
             ev->event_flags &= ~EVAS_EVENT_FLAG_ON_HOLD;
          }

        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
          {
             if (sd->spacer_down) sd->spacer_down = EINA_FALSE;
             _slider_update(data, EINA_TRUE);
             efl_event_callback_call
               (data, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_STOP, NULL);
             if (sd->frozen)
               {
                  elm_widget_scroll_freeze_pop(data);
                  sd->frozen = EINA_FALSE;
               }
             return;
          }
        if (_is_horizontal(sd->dir))
          {
             button_x = ((double)ev->cur.canvas.x - (double)sr.x) / (double)sr.w;
             if (button_x > 1) button_x = 1;
             if (button_x < 0) button_x = 0;
          }
        else
          {
             button_y = ((double)ev->cur.canvas.y - (double)sr.y) / (double)sr.h;
             if (button_y > 1) button_y = 1;
             if (button_y < 0) button_y = 0;
          }

        efl_ui_slider_move_knob(data, button_x, button_y);
        _slider_update(data, EINA_TRUE);
     }
}

static void
_spacer_up_cb(void *data,
              Evas *e EINA_UNUSED,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);

   if (!sd->spacer_down) return;
   if (sd->spacer_down) sd->spacer_down = EINA_FALSE;

   _slider_update(data, EINA_TRUE);
   efl_event_callback_call(data, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_STOP, NULL);

   if (sd->frozen)
     {
        elm_widget_scroll_freeze_pop(data);
        sd->frozen = EINA_FALSE;
     }
}

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   const char *txt = elm_widget_access_info_get(obj);

   if (!txt) txt = elm_layout_text_get(obj, NULL);
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   char *ret;
   Eina_Strbuf *buf = eina_strbuf_new();

   if (elm_widget_disabled_get(obj))
     eina_strbuf_append(buf, " state: disabled");

   if (eina_strbuf_length_get(buf))
     {
        ret = eina_strbuf_string_steal(buf);
        eina_strbuf_free(buf);
        return ret;
     }

   eina_strbuf_free(buf);
   return NULL;
}

EOLIAN static Eo *
_efl_ui_slider_efl_object_constructor(Eo *obj, Efl_Ui_Slider_Data *priv)
{
   char *group;
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_SLIDER);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "slider");
   elm_widget_sub_object_parent_add(obj);

   group = _efl_ui_slider_theme_group_get(obj, priv);
   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                    elm_widget_theme_klass_get(obj),
                                    group,
                                    elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");

   free(group);

   priv->dir = EFL_UI_DIR_RIGHT;
   priv->val_max = 1.0;
   priv->step = SLIDER_STEP;

   efl_layout_signal_callback_add(obj, "drag", "*", _drag, obj);
   efl_layout_signal_callback_add(obj, "drag,start", "*", _drag_start, obj);
   efl_layout_signal_callback_add(obj, "drag,stop", "*", _drag_stop, obj);
   efl_layout_signal_callback_add(obj, "drag,step", "*", _drag_step, obj);
   efl_layout_signal_callback_add(obj, "drag,page", "*", _drag_stop, obj);

   priv->spacer = efl_add(EFL_CANVAS_RECTANGLE_CLASS, obj,
                          efl_gfx_color_set(efl_added, 0, 0, 0, 0),
                          efl_canvas_object_pass_events_set(efl_added, EINA_TRUE));

   if (elm_widget_is_legacy(obj))
     efl_content_set(efl_part(obj, "elm.swallow.bar"), priv->spacer);
   else
     efl_content_set(efl_part(obj, "efl.bar"), priv->spacer);

   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_DOWN, _spacer_down_cb, obj);
   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_MOVE, _spacer_move_cb, obj);
   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_UP, _spacer_up_cb, obj);

   efl_ui_widget_focus_allow_set(obj, EINA_TRUE);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("slider"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, NULL);

   evas_object_smart_changed(obj);

   return obj;
}

EOLIAN static void
_efl_ui_slider_efl_object_destructor(Eo *obj,
                                      Efl_Ui_Slider_Data *sd)
{
   ecore_timer_del(sd->delay);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_slider_efl_ui_direction_direction_set(Eo *obj, Efl_Ui_Slider_Data *sd, Efl_Ui_Dir dir)
{
   sd->dir = dir;

   efl_ui_widget_theme_apply(obj);
}

EOLIAN static Efl_Ui_Dir
_efl_ui_slider_efl_ui_direction_direction_get(const Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->dir;
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_range_min_max_set(Eo *obj, Efl_Ui_Slider_Data *sd, double min, double max)
{
   if ((sd->val_min == min) && (sd->val_max == max)) return;
   sd->val_min = min;
   sd->val_max = max;
   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   efl_ui_slider_val_set(obj);
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_range_min_max_get(const Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd, double *min, double *max)
{
   if (min) *min = sd->val_min;
   if (max) *max = sd->val_max;
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_range_value_set(Eo *obj, Efl_Ui_Slider_Data *sd, double val)
{
   if (sd->val == val) return;
   sd->val = val;

   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   efl_ui_slider_val_set(obj);
}

EOLIAN static double
_efl_ui_slider_efl_ui_range_range_value_get(const Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->val;
}

EOLIAN static double
_efl_ui_slider_efl_ui_range_range_step_get(const Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->step;
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_range_step_set(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd, double step)
{
   if (sd->step == step) return;

   if (step < 0.0) step = 0.0;
   else if (step > 1.0) step = 1.0;

   sd->step = step;
}

EOLIAN static Eina_Bool
_efl_ui_slider_efl_ui_focus_object_on_focus_update(Eo *obj, Efl_Ui_Slider_Data *sd EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));

   return int_ret;
}

// A11Y Accessibility

EOLIAN static void
_efl_ui_slider_efl_access_value_value_and_text_get(const Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd, double *value, const char **text)
{
   if (value) *value = sd->val;
   if (text) *text = NULL;
}

EOLIAN static Eina_Bool
_efl_ui_slider_efl_access_value_value_and_text_set(Eo *obj, Efl_Ui_Slider_Data *sd, double value, const char *text EINA_UNUSED)
{
   double oldval = sd->val;

   if (value < sd->val_min) value = sd->val_min;
   if (value > sd->val_max) value = sd->val_max;

   efl_event_callback_call(obj, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_START, NULL);
   sd->val = value;
   efl_ui_slider_val_set(obj);
   sd->val = oldval;
   _slider_update(obj, EINA_TRUE);
   efl_event_callback_call(obj, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_STOP, NULL);

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_slider_efl_access_value_range_get(const Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd, double *lower, double *upper, const char **descr)
{
   if (lower) *lower = sd->val_min;
   if (upper) *upper = sd->val_max;
   if (descr) *descr = NULL;
}

EOLIAN static double
_efl_ui_slider_efl_access_value_increment_get(const Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->step;
}

EOLIAN const Efl_Access_Action_Data *
_efl_ui_slider_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *pd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "drag,left", "drag", "left", _key_action_drag},
          { "drag,right", "drag", "right", _key_action_drag},
          { "drag,up", "drag", "up", _key_action_drag},
          { "drag,down", "drag", "down", _key_action_drag},
          { NULL, NULL, NULL, NULL}
   };
   return &atspi_actions[0];
}

// A11Y Accessibility - END
/* Internal EO APIs and hidden overrides */

EFL_VOID_FUNC_BODYV(efl_ui_slider_val_fetch, EFL_FUNC_CALL(user_event), Eina_Bool user_event)
EFL_VOID_FUNC_BODY(efl_ui_slider_val_set)
EFL_VOID_FUNC_BODYV(efl_ui_slider_down_knob, EFL_FUNC_CALL(button_x, button_y), double button_x, double button_y)
EFL_VOID_FUNC_BODYV(efl_ui_slider_move_knob, EFL_FUNC_CALL(button_x, button_y), double button_x, double button_y)

#define EFL_UI_SLIDER_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_slider), \
   EFL_UI_SLIDER_VAL_FETCH_OPS(efl_ui_slider), \
   EFL_UI_SLIDER_VAL_SET_OPS(efl_ui_slider), \
   EFL_UI_SLIDER_DOWN_KNOB_OPS(efl_ui_slider), \
   EFL_UI_SLIDER_MOVE_KNOB_OPS(efl_ui_slider), \

#include "efl_ui_slider.eo.c"
