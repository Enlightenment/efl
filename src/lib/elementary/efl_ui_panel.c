#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_UI_FOCUS_LAYER_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_panel_private.h"

#include "els_box.h"

#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_PANEL_CLASS

#define MY_CLASS_NAME "Efl.Ui.Panel"

static const char ACCESS_OUTLINE_PART[] = "access.outline";
static const char SIG_TOGGLED[] = "toggled";
static const char SIG_SCROLL[] = "scroll";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_TOGGLED, ""},
   {SIG_SCROLL, ""},
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};
static void _panel_toggle(void *, Evas_Object *, const char *,const char *);
static Eina_Bool _key_action_toggle(Evas_Object *obj, const char *params);
static void _drawer_open(Evas_Object *, Evas_Coord , Evas_Coord , Eina_Bool );
static void _drawer_close(Evas_Object *, Evas_Coord , Evas_Coord , Eina_Bool);

static const Elm_Action key_actions[] = {
   {"toggle", _key_action_toggle},
   {NULL, NULL}
};

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   EFL_UI_PANEL_DATA_GET(obj, sd);

   if ((sd->content) && (efl_isa(sd->content, EFL_UI_WIDGET_CLASS)))
     efl_ui_mirrored_set(sd->content, rtl);
   efl_ui_panel_orient_set(obj, efl_ui_panel_orient_get(obj));
}

EOLIAN static void
_efl_ui_panel_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Panel_Data *sd)
{
   Evas_Coord mw = 0, mh = 0;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->delete_me) return;

   if (sd->scrollable)
     {
        if (sd->hidden) _drawer_close(obj, wd->w, wd->h, EINA_FALSE);
        else _drawer_open(obj, wd->w, wd->h, EINA_FALSE);
     }

   evas_object_smart_calculate(sd->bx);
   edje_object_size_min_calc(wd->resize_obj, &mw, &mh);
   evas_object_size_hint_min_set(obj, mw, mh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static char *
_access_state_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   EFL_UI_PANEL_DATA_GET(data, sd);

   if (!sd->hidden) return strdup(E_("state: opened"));
   else return strdup(E_("state: closed"));

   return NULL;
}

static Evas_Object *
_access_object_get(const Evas_Object *obj, const char *part)
{
   Evas_Object *po, *ao, *o;
   EFL_UI_PANEL_DATA_GET(obj, sd);

   o = elm_layout_edje_get(sd->scr_ly);
   edje_object_freeze(o);
   po = (Evas_Object *)edje_object_part_object_get(o, part);
   edje_object_thaw(o);
   ao = evas_object_data_get(po, "_part_access_obj");

   return ao;
}

static void
_access_activate_cb(void *data,
                    Evas_Object *part_obj EINA_UNUSED,
                    Elm_Object_Item *item EINA_UNUSED)
{
   efl_ui_panel_hidden_set(data, EINA_TRUE);
}

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   Evas_Object *ao;
   EFL_UI_PANEL_DATA_GET(obj, sd);

   if (is_access)
     {
        ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
        if (!ao)
          {
             ao = _elm_access_edje_object_part_object_register
                (obj, elm_layout_edje_get(sd->scr_ly), ACCESS_OUTLINE_PART);
             _elm_access_text_set(_elm_access_info_get(ao),
                                  ELM_ACCESS_TYPE, E_("A panel is open"));
             _elm_access_text_set(_elm_access_info_get(ao),
                                  ELM_ACCESS_CONTEXT_INFO, E_("Double tap to close panel menu"));
             _elm_access_activate_callback_set
                (_elm_access_info_get(ao), _access_activate_cb, obj);
          }
     }
   else
     {
        _elm_access_edje_object_part_object_unregister
           (obj, elm_layout_edje_get(sd->scr_ly), ACCESS_OUTLINE_PART);
     }
}

static void
_orient_set_do(Evas_Object *obj)
{
   EFL_UI_PANEL_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_TOP:
         elm_widget_theme_element_set(obj, "top");
         break;
      case EFL_UI_PANEL_ORIENT_BOTTOM:
         elm_widget_theme_element_set(obj, "bottom");
         break;
      case EFL_UI_PANEL_ORIENT_LEFT:
         elm_widget_theme_element_set(obj, "left");
         break;
      case EFL_UI_PANEL_ORIENT_RIGHT:
         elm_widget_theme_element_set(obj, "right");
         break;
     }

   efl_ui_widget_theme_apply(obj);

   /* access */
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        Evas_Object *ao;
        ao = _elm_access_edje_object_part_object_register
            (obj, wd->resize_obj, "btn_icon");
        _elm_access_text_set(_elm_access_info_get(ao),
                             ELM_ACCESS_TYPE, E_("panel button"));
        _elm_access_callback_set
          (_elm_access_info_get(ao), ELM_ACCESS_STATE, _access_state_cb, obj);
     }
}

static void
_scrollable_layout_theme_set(Eo *obj, Efl_Ui_Panel_Data *sd)
{
   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_TOP:
         elm_widget_theme_element_set(sd->scr_ly, "scrollable/top");
         break;
      case EFL_UI_PANEL_ORIENT_BOTTOM:
         elm_widget_theme_element_set(sd->scr_ly, "scrollable/bottom");
         break;
      case EFL_UI_PANEL_ORIENT_LEFT:
         elm_widget_theme_element_set(sd->scr_ly, "scrollable/left");
         break;
      case EFL_UI_PANEL_ORIENT_RIGHT:
         elm_widget_theme_element_set(sd->scr_ly, "scrollable/right");
         break;
     }

   efl_ui_widget_theme_apply(sd->scr_ly);

   /* access */
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_obj_process(obj, EINA_TRUE);
}

EOLIAN static Eina_Error
_efl_ui_panel_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Panel_Data *sd)
{
   const char *str;
   Evas_Coord minw = 0, minh = 0;

   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_ERROR_GENERIC);

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   if (sd->scrollable)
     {
        const char *handler_size;
        elm_widget_theme_object_set(obj, sd->scr_edje, "panel", NULL,
                                    elm_widget_style_get(obj));
        handler_size = edje_object_data_get(sd->scr_edje, "handler_size");
        if (handler_size)
          sd->handler_size = (int) (elm_object_scale_get(obj)) * (atoi(handler_size));
     }
   else
     {
        str = edje_object_data_get
           (wd->resize_obj, "focus_highlight");
        if ((str) && (!strcmp(str, "on")))
          elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
        else
          elm_widget_highlight_in_theme_set(obj, EINA_FALSE);

        evas_object_hide(sd->event);
        elm_coords_finger_size_adjust(1, &minw, 1, &minh);
        evas_object_size_hint_min_set(sd->event, minw, minh);

        if (edje_object_part_exists(wd->resize_obj, "efl.swallow.event"))
          efl_content_set(efl_part(efl_super(obj, MY_CLASS), "efl.swallow.event"), sd->event);
     }

   if (efl_finalized_get(obj))
     elm_layout_sizing_eval(obj);

   return int_ret;
}

static void
_box_layout_cb(Evas_Object *o,
               Evas_Object_Box_Data *_pd,
               void *data EINA_UNUSED)
{
   _els_box_layout(o, _pd, EINA_TRUE, EINA_FALSE, EINA_FALSE);
}

static void
_handler_open(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   EFL_UI_PANEL_DATA_GET(obj, sd);

   if (sd->handler_size == 0) return;

   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_TOP:
         elm_interface_scrollable_region_bring_in
               (obj, 0, (h * sd->content_size_ratio) - sd->handler_size, w, h);
         break;
      case EFL_UI_PANEL_ORIENT_BOTTOM:
         elm_interface_scrollable_region_bring_in
               (obj, 0, sd->handler_size, w, h);
         break;
      case EFL_UI_PANEL_ORIENT_LEFT:
         elm_interface_scrollable_region_bring_in
               (obj, (w * sd->content_size_ratio) - sd->handler_size, 0, w, h);
         break;
      case EFL_UI_PANEL_ORIENT_RIGHT:
         elm_interface_scrollable_region_bring_in
               (obj, sd->handler_size, 0, w, h);
         break;
     }
}

static void
_drawer_open(Evas_Object *obj, Evas_Coord w, Evas_Coord h, Eina_Bool anim)
{
   EFL_UI_PANEL_DATA_GET(obj, sd);
   int x = 0, y = 0;

   if (sd->freeze)
     {
        elm_interface_scrollable_movement_block_set
              (obj, EFL_UI_SCROLL_BLOCK_NONE);
        sd->freeze = EINA_FALSE;
        elm_layout_signal_emit(sd->scr_ly, "efl,state,content,visible", "efl");
     }

   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_TOP:
         break;
      case EFL_UI_PANEL_ORIENT_LEFT:
         if (efl_ui_mirrored_get(obj))
           x = w * sd->content_size_ratio;
         break;

      case EFL_UI_PANEL_ORIENT_BOTTOM:
         y = h * sd->content_size_ratio;
         break;

      case EFL_UI_PANEL_ORIENT_RIGHT:
         if (!efl_ui_mirrored_get(obj))
           x = w * sd->content_size_ratio;
         break;
     }

   if (anim)
     elm_interface_scrollable_region_bring_in
           (obj, x, y, w, h);
   else
     elm_interface_scrollable_content_region_show
           (obj, x, y, w, h);
}

static void
_drawer_close(Evas_Object *obj, Evas_Coord w, Evas_Coord h, Eina_Bool anim)
{
   EFL_UI_PANEL_DATA_GET(obj, sd);
   int x = 0, y = 0, cx, cy;
   Eina_Bool horizontal = EINA_FALSE;

   elm_widget_tree_unfocusable_set(obj, EINA_TRUE);
   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_TOP:
         y = h * sd->content_size_ratio;
         break;

      case EFL_UI_PANEL_ORIENT_LEFT:
         if (!efl_ui_mirrored_get(obj))
           x = w * sd->content_size_ratio;
         horizontal = EINA_TRUE;
         break;

      case EFL_UI_PANEL_ORIENT_BOTTOM:
         break;

      case EFL_UI_PANEL_ORIENT_RIGHT:
         if (efl_ui_mirrored_get(obj))
           x = w * sd->content_size_ratio;
         horizontal = EINA_TRUE;
         break;
     }

   elm_interface_scrollable_content_pos_get(obj, &cx, &cy);

   if ((x == cx) && (y == cy))
     {
        if (!sd->freeze)
          {
             if (horizontal)
               elm_interface_scrollable_movement_block_set
                  (obj, EFL_UI_SCROLL_BLOCK_HORIZONTAL);
             else
               elm_interface_scrollable_movement_block_set
                  (obj, EFL_UI_SCROLL_BLOCK_VERTICAL);
             sd->freeze = EINA_TRUE;
             elm_layout_signal_emit(sd->scr_ly, "efl,state,content,hidden", "efl");
          }

        return;
     }

   if (anim)
     {
        if (sd->freeze)
          {
             elm_interface_scrollable_movement_block_set
                   (obj, EFL_UI_SCROLL_BLOCK_NONE);
             sd->freeze = EINA_FALSE;
             elm_layout_signal_emit(sd->scr_ly, "efl,state,content,visible", "efl");
          }
        elm_interface_scrollable_region_bring_in(obj, x, y, w, h);
     }
   else
     {
        elm_interface_scrollable_content_region_show(obj, x, y, w, h);
        if (!sd->freeze)
          {
             if (horizontal)
               elm_interface_scrollable_movement_block_set
                     (obj, EFL_UI_SCROLL_BLOCK_HORIZONTAL);
             else
               elm_interface_scrollable_movement_block_set
                     (obj, EFL_UI_SCROLL_BLOCK_VERTICAL);
             sd->freeze = EINA_TRUE;
             elm_layout_signal_emit(sd->scr_ly, "efl,state,content,hidden", "efl");
          }
     }
}

static void
_panel_toggle(void *data EINA_UNUSED,
              Evas_Object *obj,
              const char *emission EINA_UNUSED,
              const char *source EINA_UNUSED)
{
   EFL_UI_PANEL_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   int w, h;

   if (sd->scrollable)
     {
        if (elm_widget_disabled_get(obj)) return;

        evas_object_geometry_get(obj, NULL, NULL, &w, &h);
        if (sd->hidden)
          {
             sd->hidden = EINA_FALSE;
             _drawer_open(obj, w, h, EINA_TRUE);
          }
        else
          {
             sd->hidden = EINA_TRUE;
             _drawer_close(obj, w, h, EINA_TRUE);
          }
     }
   else
     {
        if (sd->hidden)
          {
             elm_layout_signal_emit(obj, "efl,action,show", "efl");
             sd->hidden = EINA_FALSE;
             evas_object_repeat_events_set(obj, EINA_FALSE);
          }
        else
          {
             elm_layout_signal_emit(obj, "efl,action,hide", "efl");
             sd->hidden = EINA_TRUE;
             evas_object_repeat_events_set(obj, EINA_TRUE);
          }

        //if the panel is hidden, make this thing unfocusable
        elm_widget_tree_unfocusable_set(obj, sd->hidden);
        edje_object_message_signal_process(wd->resize_obj);
     }

   efl_ui_focus_layer_enable_set(obj, !sd->hidden);
   efl_event_callback_call(obj, EFL_UI_PANEL_EVENT_TOGGLED, NULL);
}

static Eina_Bool
_state_sync(Evas_Object *obj)
{
   EFL_UI_PANEL_DATA_GET(obj, sd);
   Evas_Coord pos, panel_size, w, h, threshold;
   Eina_Bool horizontal = EINA_FALSE, reverse = EINA_FALSE;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   if (!evas_object_visible_get(sd->bx)) return EINA_TRUE;

   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_BOTTOM:
         reverse = EINA_TRUE;
      case EFL_UI_PANEL_ORIENT_TOP:
         break;

      case EFL_UI_PANEL_ORIENT_RIGHT:
         reverse = EINA_TRUE;
         EINA_FALLTHROUGH;
      case EFL_UI_PANEL_ORIENT_LEFT:
         horizontal = EINA_TRUE;
         break;
     }

   if (horizontal)
     {
         if (w <= 0) return EINA_TRUE;

         panel_size = w * sd->content_size_ratio;
         elm_interface_scrollable_content_pos_get(obj, &pos, NULL);
         reverse ^= efl_ui_mirrored_get(obj);
     }
   else
     {
         if (h <= 0) return EINA_TRUE;

         panel_size = h * sd->content_size_ratio;
         elm_interface_scrollable_content_pos_get(obj, NULL, &pos);
     }
   threshold = (sd->hidden) ? panel_size - (panel_size / 4) : (panel_size / 4);

   if (reverse)
     {
         if (pos > panel_size - threshold) sd->hidden = EINA_FALSE;
         else sd->hidden = EINA_TRUE;
     }
   else
     {
         if (pos < threshold) sd->hidden = EINA_FALSE;
         else sd->hidden = EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool
_timer_cb(void *data)
{
   EFL_UI_PANEL_DATA_GET(data, sd);
   Evas_Object *obj = data;
   Evas_Coord w, h;

   sd->timer = NULL;

   if (sd->freeze)
     {
        elm_interface_scrollable_movement_block_set
              (obj, EFL_UI_SCROLL_BLOCK_NONE);
        sd->freeze = EINA_FALSE;
        elm_layout_signal_emit(sd->scr_ly, "efl,state,content,visible", "efl");
        evas_object_geometry_get(obj, NULL, NULL, &w, &h);
        _handler_open(obj, w, h);
     }

   return ECORE_CALLBACK_CANCEL;
}

static void
_event_mouse_up(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   EFL_UI_PANEL_DATA_GET(data, sd);
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Coord x, y, up_x, up_y, minw = 0, minh = 0;
   evas_object_geometry_get(data, &x, &y, NULL, NULL);

   up_x = ev->canvas.x - x;
   up_y = ev->canvas.y - y;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);

   if ((!sd->hidden) && (up_x == sd->down_x) && (up_y == sd->down_y))
     efl_ui_panel_hidden_set(data, EINA_TRUE);
}

static void
_on_mouse_down(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj,
               void *event_info)
{
   Efl_Ui_Panel_Data *sd = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord finger_size = elm_config_finger_size_get();
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   Eina_Bool is_mirrored = efl_ui_mirrored_get(obj);

   sd->down_x = ev->canvas.x - x;
   sd->down_y = ev->canvas.y - y;

   // if freeze state & mouse down on the edge
   // then set timer for un-freeze
   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_TOP:
         if ((sd->freeze) && (sd->down_y >= 0) && (sd->down_y < finger_size))
           {
              ecore_timer_del(sd->timer);
              sd->timer = ecore_timer_add(0.2, _timer_cb, obj);
           }
         break;
	  case EFL_UI_PANEL_ORIENT_BOTTOM:
         if ((sd->freeze) && (sd->down_y <= h) && (sd->down_y > (h - finger_size)))
           {
              ecore_timer_del(sd->timer);
              sd->timer = ecore_timer_add(0.2, _timer_cb, obj);
           }
         break;
      case EFL_UI_PANEL_ORIENT_LEFT:
         if ((!is_mirrored && (sd->freeze) && (sd->down_x >= 0) && (sd->down_x < finger_size)) ||
              ((is_mirrored && (sd->freeze) && (sd->down_x <= w) && (sd->down_x > (w - finger_size)))))
           {
              ecore_timer_del(sd->timer);
              sd->timer = ecore_timer_add(0.2, _timer_cb, obj);
           }
         break;
	  case EFL_UI_PANEL_ORIENT_RIGHT:
         if ((is_mirrored && (sd->freeze) && (sd->down_x >= 0) && (sd->down_x < finger_size)) ||
              (!is_mirrored && (sd->freeze) && (sd->down_x <= w) && (sd->down_x > (w - finger_size))))
           {
              ecore_timer_del(sd->timer);
              sd->timer = ecore_timer_add(0.2, _timer_cb, obj);
           }
         break;
     }
}

static void
_on_mouse_move(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj,
               void *event_info)
{
   Efl_Ui_Panel_Data *sd = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord x, y, w, h, cur_x, cur_y, finger_size;
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   finger_size = elm_config_finger_size_get();
   Eina_Bool is_mirrored = efl_ui_mirrored_get(obj);

   cur_x = ev->cur.canvas.x - x;
   cur_y = ev->cur.canvas.y - y;

   // if mouse down on the edge (it means sd->timer is not null)
   //    and move more than finger size
   // then un-freeze
   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_TOP:
         if (sd->timer && ((cur_y - sd->down_y) > finger_size))
           {
              elm_interface_scrollable_movement_block_set
                 (obj, EFL_UI_SCROLL_BLOCK_NONE);
              sd->freeze = EINA_FALSE;
              elm_layout_signal_emit(sd->scr_ly, "efl,state,content,visible", "efl");
           }
         break;
      case EFL_UI_PANEL_ORIENT_BOTTOM:
         if (sd->timer && ((sd->down_y - cur_y) > finger_size))
           {
              elm_interface_scrollable_movement_block_set
                 (obj, EFL_UI_SCROLL_BLOCK_NONE);
              sd->freeze = EINA_FALSE;
              elm_layout_signal_emit(sd->scr_ly, "efl,state,content,visible", "efl");
           }
         break;
      case EFL_UI_PANEL_ORIENT_LEFT:
         if ((!is_mirrored && (sd->timer) && ((cur_x - sd->down_x) > finger_size)) ||
              ((is_mirrored) && (sd->timer) && ((sd->down_x - cur_x) > finger_size)))
           {
              elm_interface_scrollable_movement_block_set
                 (obj, EFL_UI_SCROLL_BLOCK_NONE);
              sd->freeze = EINA_FALSE;
              elm_layout_signal_emit(sd->scr_ly, "efl,state,content,visible", "efl");
           }
         break;
      case EFL_UI_PANEL_ORIENT_RIGHT:
         if ((is_mirrored && (sd->timer) && ((cur_x - sd->down_x) > finger_size)) ||
              (!is_mirrored && (sd->timer) && ((sd->down_x - cur_x) > finger_size)))
           {
              elm_interface_scrollable_movement_block_set
                 (obj, EFL_UI_SCROLL_BLOCK_NONE);
              sd->freeze = EINA_FALSE;
              elm_layout_signal_emit(sd->scr_ly, "efl,state,content,visible", "efl");
           }
         break;
     }

   if (!sd->freeze && sd->hidden)
     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
}

static void
_on_mouse_up(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj,
             void *event_info)
{
   Efl_Ui_Panel_Data *sd = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Coord w, h;
   Eina_Bool hidden;

   hidden = sd->hidden;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);

   if (_state_sync(obj)) return;

   if (sd->hidden) _drawer_close(obj, w, h, EINA_TRUE);
   else _drawer_open(obj, w, h, EINA_TRUE);

   if (sd->hidden != hidden)
     efl_event_callback_legacy_call(obj, EFL_UI_PANEL_EVENT_TOGGLED, NULL);

   if (!sd->freeze && sd->hidden)
     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
}

static Eina_Bool
_key_action_toggle(Evas_Object *obj, const char *params EINA_UNUSED)
{
   _panel_toggle(NULL, obj, NULL, NULL);
   return EINA_TRUE;
}

// _panel_elm_widget_widget_event
ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(panel, Efl_Ui_Panel_Data)

EOLIAN static Eina_Bool
_efl_ui_panel_efl_ui_widget_widget_input_event_handler(Eo *obj, Efl_Ui_Panel_Data *pd, const Efl_Event *eo_event, Evas_Object *src)
{
   if (src != obj) return EINA_FALSE;
   return _panel_efl_ui_widget_widget_input_event_handler(obj, pd, eo_event, src);
}

static Eina_Bool
_efl_ui_panel_efl_content_content_set(Eo *obj, Efl_Ui_Panel_Data *sd, Efl_Gfx_Entity *content)
{
   if (sd->content == content) return EINA_TRUE;
   if (sd->content)
     evas_object_box_remove_all(sd->bx, EINA_TRUE);
   sd->content = content;
   if (content)
     {
        evas_object_box_append(sd->bx, sd->content);
        evas_object_show(sd->content);
        if (sd->scrollable)
          elm_widget_sub_object_add(sd->scr_ly, sd->content);
        else
          elm_widget_sub_object_add(obj, sd->content);
     }
   efl_event_callback_call(obj, EFL_CONTENT_EVENT_CONTENT_CHANGED, content);
   if (efl_finalized_get(obj))
     elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Efl_Gfx_Entity*
_efl_ui_panel_efl_content_content_get(const Eo *obj EINA_UNUSED, Efl_Ui_Panel_Data *sd)
{
   return sd->content;
}

static Efl_Gfx_Entity*
_efl_ui_panel_efl_content_content_unset(Eo *obj EINA_UNUSED, Efl_Ui_Panel_Data *sd)
{
   Efl_Gfx_Entity *ret = NULL;

   if (!sd->content) return NULL;
   ret = sd->content;

   evas_object_box_remove_all(sd->bx, EINA_FALSE);
   if (sd->scrollable)
     _elm_widget_sub_object_redirect_to_top(sd->scr_ly, sd->content);
   sd->content = NULL;
   efl_event_callback_call(obj, EFL_CONTENT_EVENT_CONTENT_CHANGED, NULL);
   return ret;
}

EOLIAN static void
_efl_ui_panel_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Panel_Data *sd, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   efl_gfx_entity_position_set(sd->hit_rect, pos);
}

static void
_scrollable_layout_resize(Eo *obj, Efl_Ui_Panel_Data *sd, Evas_Coord w, Evas_Coord h)
{
   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_TOP:
      case EFL_UI_PANEL_ORIENT_BOTTOM:
         // vertical
         evas_object_resize(sd->scr_ly, w, (1 + sd->content_size_ratio) * h);
         evas_object_size_hint_min_set(sd->scr_panel, w, (sd->content_size_ratio * h));
         evas_object_size_hint_min_set(sd->scr_event, w, h);
         break;
      case EFL_UI_PANEL_ORIENT_LEFT:
      case EFL_UI_PANEL_ORIENT_RIGHT:
         // horizontal
         evas_object_resize(sd->scr_ly, (1 + sd->content_size_ratio) * w, h);
         evas_object_size_hint_min_set(sd->scr_panel, (sd->content_size_ratio * w), h);
         evas_object_size_hint_min_set(sd->scr_event, w, h);
         break;
     }
   if (efl_finalized_get(obj))
     elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_panel_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Panel_Data *sd, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);

   if (!sd->scrollable) return;

   efl_gfx_entity_size_set(sd->hit_rect, sz);
   _scrollable_layout_resize(obj, sd, sz.w, sz.h);
}

EOLIAN static void
_efl_ui_panel_efl_canvas_group_group_member_add(Eo *obj, Efl_Ui_Panel_Data *sd, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);

   if (sd->hit_rect) evas_object_raise(sd->hit_rect);
}

EOLIAN static void
_efl_ui_panel_efl_ui_widget_on_access_update(Eo *obj, Efl_Ui_Panel_Data *_pd, Eina_Bool is_access)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Panel_Data *sd = _pd;

   if (sd->scrollable)
     {
        _access_obj_process(obj, is_access);
        return;
     }

   if (is_access)
     _elm_access_edje_object_part_object_register
       (obj, wd->resize_obj, "btn_icon");
   else
     _elm_access_edje_object_part_object_unregister
       (obj, wd->resize_obj, "btn_icon");
}

EOLIAN static Eo *
_efl_ui_panel_efl_object_constructor(Eo *obj, Efl_Ui_Panel_Data *_pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   _pd->panel_edje = wd->resize_obj;

   efl_ui_widget_theme_apply(obj);

   _pd->bx = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_box_layout_set(_pd->bx, _box_layout_cb, _pd, NULL);
   evas_object_show(_pd->bx);

   elm_layout_signal_callback_add
     (obj, "efl,action,panel,toggle", "*", _panel_toggle, obj);

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   _pd->event = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(_pd->event, 0, 0, 0, 0);
   evas_object_pass_events_set(_pd->event, EINA_TRUE);
   elm_widget_sub_object_add(obj, _pd->event);

   /* just to bootstrap and have theme hook to work */
   if (!elm_layout_theme_set(obj, "panel", "top", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
   else
     {
        elm_layout_content_set(obj, "efl.content", _pd->bx);

        if (edje_object_part_exists
            (wd->resize_obj, "efl.swallow.event"))
          {
             Evas_Coord minw = 0, minh = 0;

             elm_coords_finger_size_adjust(1, &minw, 1, &minh);
             evas_object_size_hint_min_set(_pd->event, minw, minh);
             efl_content_set(efl_part(efl_super(obj, MY_CLASS), "efl.swallow.event"), _pd->event);
          }
     }

   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_PANEL);

   efl_ui_focus_layer_behaviour_set(obj, EINA_FALSE, EINA_FALSE);

   return obj;
}

EOLIAN static void
_efl_ui_panel_efl_object_destructor(Eo *obj, Efl_Ui_Panel_Data *sd)
{
   Evas_Object *child;
   Eina_List *l;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   sd->delete_me = EINA_TRUE;

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);

   /* let's make our panel object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH(wd->subobjs, l, child)
     {
        if (child == sd->bx)
          {
             wd->subobjs =
               eina_list_demote_list(wd->subobjs, l);
             break;
          }
     }

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_panel_orient_set(Eo *obj, Efl_Ui_Panel_Data *sd, Efl_Ui_Panel_Orient orient)
{
   if (sd->orient == orient) return;
   sd->orient = orient;

   if (sd->scrollable)
     {
        _scrollable_layout_theme_set(obj, sd);

        if (!sd->freeze)
          {
             switch (sd->orient)
               {
                  case EFL_UI_PANEL_ORIENT_TOP:
                  case EFL_UI_PANEL_ORIENT_BOTTOM:
                     elm_interface_scrollable_movement_block_set
                           (obj, EFL_UI_SCROLL_BLOCK_VERTICAL);
                     break;
                  case EFL_UI_PANEL_ORIENT_LEFT:
                  case EFL_UI_PANEL_ORIENT_RIGHT:
                     elm_interface_scrollable_movement_block_set
                           (obj, EFL_UI_SCROLL_BLOCK_HORIZONTAL);
                     break;
               }

             sd->freeze = EINA_TRUE;
             elm_layout_signal_emit(sd->scr_ly, "efl,state,content,hidden", "efl");
          }

        efl_ui_panel_scrollable_content_size_set(obj, sd->content_size_ratio);
     }
   else
     _orient_set_do(obj);

   if (efl_finalized_get(obj))
     elm_layout_sizing_eval(obj);
}

EOLIAN static Efl_Ui_Panel_Orient
_efl_ui_panel_orient_get(const Eo *obj EINA_UNUSED, Efl_Ui_Panel_Data *sd)
{
   return sd->orient;
}

EOLIAN static void
_efl_ui_panel_hidden_set(Eo *obj, Efl_Ui_Panel_Data *sd, Eina_Bool hidden)
{
   if (sd->hidden == !!hidden)
     {
        if (sd->scrollable && sd->hidden && !sd->freeze)
          {
             Evas_Coord w, h;
             evas_object_geometry_get(obj, NULL, NULL, &w, &h);
             _drawer_close(obj, w, h, EINA_TRUE);
          }
        return;
     }

   _panel_toggle(NULL, obj, NULL, NULL);
}

EOLIAN static Eina_Bool
_efl_ui_panel_hidden_get(const Eo *obj EINA_UNUSED, Efl_Ui_Panel_Data *sd)
{
   return sd->hidden;
}

EOLIAN static void
_efl_ui_panel_toggle(Eo *obj, Efl_Ui_Panel_Data *_pd EINA_UNUSED)
{
   _panel_toggle(NULL, obj, NULL, NULL);
}

EOLIAN static Eina_Rect
_efl_ui_panel_efl_ui_widget_interest_region_get(const Eo *obj, Efl_Ui_Panel_Data *sd)
{
   Eina_Rect r = {};

   elm_interface_scrollable_content_pos_get(obj, &r.x, &r.y);
   evas_object_geometry_get(obj, NULL, NULL, &r.w, &r.h);
   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_TOP:
      case EFL_UI_PANEL_ORIENT_BOTTOM:
         r.h *= sd->content_size_ratio;
         break;
      case EFL_UI_PANEL_ORIENT_LEFT:
      case EFL_UI_PANEL_ORIENT_RIGHT:
         r.w *= sd->content_size_ratio;
         break;
     }
   if (r.w < 1) r.w = 1;
   if (r.h < 1) r.h = 1;

   return r;
}

static void
_anim_stop_cb(Evas_Object *obj, void *data EINA_UNUSED)
{
   EFL_UI_PANEL_DATA_GET(obj, sd);
   Evas_Object *ao;
   Evas_Coord pos, w, h, panel_size = 0;
   Eina_Bool open = EINA_FALSE, horizontal = EINA_FALSE, reverse = EINA_FALSE;

   if (elm_widget_disabled_get(obj)) return;

   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_BOTTOM:
         reverse = EINA_TRUE;
      case EFL_UI_PANEL_ORIENT_TOP:
         break;

      case EFL_UI_PANEL_ORIENT_RIGHT:
         reverse = EINA_TRUE;
         EINA_FALLTHROUGH;
      case EFL_UI_PANEL_ORIENT_LEFT:
         horizontal = EINA_TRUE;
         break;
     }

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (horizontal)
     {
         if (w <= 0) return;

         panel_size = w * sd->content_size_ratio;
         elm_interface_scrollable_content_pos_get(obj, &pos, NULL);
         reverse ^= efl_ui_mirrored_get(obj);
     }
   else
     {
         if (h <= 0) return;

         panel_size = h * sd->content_size_ratio;
         elm_interface_scrollable_content_pos_get(obj, NULL, &pos);
     }

   if (pos == 0) open = !reverse;
   else if (pos == panel_size) open = reverse;
   else return;

   if (open)
     {
        elm_interface_scrollable_single_direction_set
              (obj, ELM_SCROLLER_SINGLE_DIRECTION_HARD);

        //focus & access
        elm_object_tree_focus_allow_set(obj, EINA_TRUE);
        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
          {
             ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
             evas_object_show(ao);
             _elm_access_highlight_set(ao);
          }
        else
          elm_object_focus_set(obj, EINA_TRUE);
     }
   else
     {
        if (horizontal)
          elm_interface_scrollable_movement_block_set
                (obj, EFL_UI_SCROLL_BLOCK_HORIZONTAL);
        else
          elm_interface_scrollable_movement_block_set
                (obj, EFL_UI_SCROLL_BLOCK_VERTICAL);
        sd->freeze = EINA_TRUE;
        elm_layout_signal_emit(sd->scr_ly, "efl,state,content,hidden", "efl");

        elm_interface_scrollable_single_direction_set
              (obj, ELM_SCROLLER_SINGLE_DIRECTION_NONE);

        //focus & access
        elm_object_tree_focus_allow_set(obj, EINA_FALSE);
        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
          {
             ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
             evas_object_hide(ao);
          }
     }
}

static void
_scroll_cb(Evas_Object *obj, void *data EINA_UNUSED)
{
   EFL_UI_PANEL_DATA_GET(obj, sd);
   Efl_Ui_Panel_Scroll_Info event;
   Evas_Coord x, y, w, h;

   if (elm_widget_disabled_get(obj)) return;
   // in the case of
   // freeze_set(FALSE) -> mouse_up -> freeze_set(TRUE) -> scroll
   if (sd->freeze)
     {
        elm_interface_scrollable_movement_block_set
              (obj, EFL_UI_SCROLL_BLOCK_NONE);
        sd->freeze = EINA_FALSE;
        elm_layout_signal_emit(sd->scr_ly, "efl,state,content,visible", "efl");
     }

   elm_interface_scrollable_content_pos_get(obj, &x, &y);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   switch (sd->orient)
     {
      case EFL_UI_PANEL_ORIENT_TOP:
         event.rel_x = 1;
         event.rel_y = 1 - ((double) y / (double) ((sd->content_size_ratio) * h));
        break;
      case EFL_UI_PANEL_ORIENT_BOTTOM:
         event.rel_x = 1;
         event.rel_y = (double) y / (double) ((sd->content_size_ratio) * h);
        break;
      case EFL_UI_PANEL_ORIENT_LEFT:
        if (!efl_ui_mirrored_get(obj))
          {
             event.rel_x = 1 - ((double) x / (double) ((sd->content_size_ratio) * w));
             event.rel_y = 1;
          }
        else
          {
             event.rel_x = (double) x / (double) ((sd->content_size_ratio) * w);
             event.rel_y = 1;
           }
        break;
      case EFL_UI_PANEL_ORIENT_RIGHT:
        if (efl_ui_mirrored_get(obj))
          {
             event.rel_x = 1 - ((double) x / (double) ((sd->content_size_ratio) * w));
             event.rel_y = 1;
          }
        else
          {
             event.rel_x = (double) x / (double) ((sd->content_size_ratio) * w);
             event.rel_y = 1;
          }
        break;
     }
   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SCROLL, (void *) &event);
}

EOLIAN static void
_efl_ui_panel_efl_ui_widget_disabled_set(Eo *obj, Efl_Ui_Panel_Data *sd, Eina_Bool disabled)
{
   efl_ui_widget_disabled_set(efl_super(obj, MY_CLASS), disabled);

   if (sd->scrollable)
     {
        if (disabled && sd->callback_added)
          {
             switch (sd->orient)
               {
                  case ELM_PANEL_ORIENT_BOTTOM:
                  case ELM_PANEL_ORIENT_TOP:
                     elm_interface_scrollable_movement_block_set
                        (obj, EFL_UI_SCROLL_BLOCK_VERTICAL);
                     break;

                  case ELM_PANEL_ORIENT_RIGHT:
                  case ELM_PANEL_ORIENT_LEFT:
                     elm_interface_scrollable_movement_block_set
                        (obj, EFL_UI_SCROLL_BLOCK_HORIZONTAL);
                     break;
               }

             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                            _on_mouse_down);
             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                            _on_mouse_move);
             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_UP,
                                            _on_mouse_up);
             evas_object_event_callback_del(sd->scr_event, EVAS_CALLBACK_MOUSE_UP,
                                            _event_mouse_up);

             sd->callback_added = EINA_FALSE;
          }
        else if (!disabled && !sd->callback_added)
          {
             switch (sd->orient)
               {
                  case ELM_PANEL_ORIENT_BOTTOM:
                  case ELM_PANEL_ORIENT_TOP:
                     elm_interface_scrollable_movement_block_set
                        (obj, EFL_UI_SCROLL_BLOCK_HORIZONTAL);
                     break;

                  case ELM_PANEL_ORIENT_RIGHT:
                  case ELM_PANEL_ORIENT_LEFT:
                     elm_interface_scrollable_movement_block_set
                        (obj, EFL_UI_SCROLL_BLOCK_VERTICAL);
                     break;
               }

             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                            _on_mouse_down, sd);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                            _on_mouse_move, sd);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                            _on_mouse_up, sd);
             evas_object_event_callback_add(sd->scr_event, EVAS_CALLBACK_MOUSE_UP,
                                            _event_mouse_up, obj);

             sd->callback_added = EINA_TRUE;
          }
     }
}

EOLIAN static double
_efl_ui_panel_scrollable_content_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Panel_Data *sd)
{
   return sd->content_size_ratio;
}

EOLIAN static void
_efl_ui_panel_scrollable_content_size_set(Eo *obj, Efl_Ui_Panel_Data *sd, double ratio)
{
   if (ratio < 0) ratio = 0;
   else if (ratio > 1.0) ratio = 1.0;

   sd->content_size_ratio = ratio;

   if (sd->scrollable)
     {
        Evas_Coord w, h;
        evas_object_geometry_get(obj, NULL, NULL, &w, &h);

        _scrollable_layout_resize(obj, sd, w, h);
     }
}

EOLIAN static Eina_Bool
_efl_ui_panel_scrollable_get(const Eo *obj EINA_UNUSED, Efl_Ui_Panel_Data *sd)
{
   return sd->scrollable;
}

EOLIAN static void
_efl_ui_panel_scrollable_set(Eo *obj, Efl_Ui_Panel_Data *sd, Eina_Bool scrollable)
{
   scrollable = !!scrollable;
   if (sd->scrollable == scrollable) return;
   sd->scrollable = scrollable;

   if (scrollable)
     {
        elm_layout_content_unset(obj, "efl.content");

        elm_widget_resize_object_set(obj, NULL);
        elm_widget_sub_object_add(obj, sd->panel_edje);

        if (!sd->scr_edje)
          {
             const char *handler_size;

             sd->scr_edje = edje_object_add(evas_object_evas_get(obj));
             elm_widget_theme_object_set(obj, sd->scr_edje, "panel", NULL,
                                         elm_widget_style_get(obj));
             evas_object_size_hint_weight_set
                (sd->scr_edje, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set
                (sd->scr_edje, EVAS_HINT_FILL, EVAS_HINT_FILL);

             handler_size = edje_object_data_get(sd->scr_edje, "handler_size");
             if (handler_size)
               sd->handler_size = (int) (elm_object_scale_get(obj)) * (atoi(handler_size));
          }

        elm_widget_resize_object_set(obj, sd->scr_edje);

        if (!sd->hit_rect)
          {
             sd->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
             evas_object_smart_member_add(sd->hit_rect, obj);
             elm_widget_sub_object_add(obj, sd->hit_rect);
             evas_object_color_set(sd->hit_rect, 0, 0, 0, 0);
             evas_object_show(sd->hit_rect);
             evas_object_repeat_events_set(sd->hit_rect, EINA_TRUE);

             elm_interface_scrollable_objects_set(obj, sd->scr_edje, sd->hit_rect);
             elm_interface_scrollable_animate_stop_cb_set(obj, _anim_stop_cb);
             elm_interface_scrollable_scroll_cb_set(obj, _scroll_cb);
          }

        if (!sd->scr_ly)
          {
             sd->scr_ly = efl_add(EFL_UI_LAYOUT_CLASS, obj,
                                  elm_layout_theme_set(efl_added,
                                                       "panel",
                                                       "scrollable/left",
                                                       elm_widget_style_get(obj)));

             evas_object_smart_member_add(sd->scr_ly, obj);
             elm_widget_sub_object_add(obj, sd->scr_ly);
             _scrollable_layout_theme_set(obj, sd);

             sd->scr_panel = evas_object_rectangle_add(evas_object_evas_get(obj));
             evas_object_color_set(sd->scr_panel, 0, 0, 0, 0);
             elm_widget_sub_object_add(obj, sd->scr_panel);
             if (!elm_layout_content_set(sd->scr_ly, "efl.panel_area", sd->scr_panel))
               elm_layout_content_set(sd->scr_ly, "panel_area", sd->scr_panel);

             sd->scr_event = evas_object_rectangle_add(evas_object_evas_get(obj));
             evas_object_color_set(sd->scr_event, 0, 0, 0, 0);
             elm_widget_sub_object_add(obj, sd->scr_event);
             if (!elm_layout_content_set(sd->scr_ly, "efl.event_area", sd->scr_event))
               elm_layout_content_set(sd->scr_ly, "event_area", sd->scr_event);
          }

        elm_interface_scrollable_content_set(obj, sd->scr_ly);
        sd->freeze = EINA_TRUE;
        efl_content_set(efl_part(sd->scr_ly, "efl.content"), sd->bx);
        if (sd->content) elm_widget_sub_object_add(sd->scr_ly, sd->content);

        switch (sd->orient)
          {
           case EFL_UI_PANEL_ORIENT_TOP:
           case EFL_UI_PANEL_ORIENT_BOTTOM:
              elm_interface_scrollable_movement_block_set
                    (obj, EFL_UI_SCROLL_BLOCK_VERTICAL);
              break;
           case EFL_UI_PANEL_ORIENT_LEFT:
           case EFL_UI_PANEL_ORIENT_RIGHT:
              elm_interface_scrollable_movement_block_set
                    (obj, EFL_UI_SCROLL_BLOCK_HORIZONTAL);
              break;
          }

        elm_interface_scrollable_single_direction_set
              (obj, ELM_SCROLLER_SINGLE_DIRECTION_NONE);

        if (!elm_widget_disabled_get(obj) && !sd->callback_added)
          {
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                            _on_mouse_down, sd);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                            _on_mouse_move, sd);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                            _on_mouse_up, sd);
             evas_object_event_callback_add(sd->scr_event, EVAS_CALLBACK_MOUSE_UP,
                                            _event_mouse_up, obj);

             sd->callback_added = EINA_TRUE;
          }

     }
   else
     {
        elm_interface_scrollable_content_set(obj, NULL);

        if (sd->callback_added)
          {
             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down);
             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move);
             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_UP, _on_mouse_up);
             evas_object_event_callback_del(sd->scr_event, EVAS_CALLBACK_MOUSE_UP,
                                            _event_mouse_up);

             sd->callback_added = EINA_FALSE;
          }

        elm_widget_resize_object_set(obj, NULL);
        elm_widget_sub_object_add(obj, sd->scr_edje);

        elm_widget_resize_object_set(obj, sd->panel_edje);

        elm_layout_content_unset(sd->scr_ly, "efl.content");
        elm_layout_content_set(obj, "efl.content", sd->bx);
        if (sd->content) elm_widget_sub_object_add(obj, sd->content);
     }
}

EOLIAN static void
_efl_ui_panel_efl_ui_i18n_mirrored_set(Eo *obj, Efl_Ui_Panel_Data *sd, Eina_Bool mirrored)
{
   if (sd->scrollable)
     efl_ui_mirrored_set(efl_cast(obj, ELM_INTERFACE_SCROLLABLE_MIXIN), mirrored);
   else
     efl_ui_mirrored_set(efl_cast(obj, EFL_UI_WIDGET_CLASS), mirrored);
}

EOLIAN const Efl_Access_Action_Data *
_efl_ui_panel_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Efl_Ui_Panel_Data *pd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "toggle", "toggle", NULL, _key_action_toggle},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_PANEL_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_panel)

#include "efl_ui_panel.eo.c"
