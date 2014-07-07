#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_panel.h"

#include "els_box.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

#define MY_CLASS ELM_PANEL_CLASS

#define MY_CLASS_NAME "Elm_Panel"
#define MY_CLASS_NAME_LEGACY "elm_panel"

static const char ACCESS_OUTLINE_PART[] = "access.outline";

static const char SIG_SCROLL[] = "scroll";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_SCROLL, ""},
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static Eina_Bool _key_action_toggle(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"toggle", _key_action_toggle},
   {NULL, NULL}
};

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   ELM_PANEL_DATA_GET(obj, sd);

   if ((sd->content) && (eo_isa(sd->content, ELM_WIDGET_CLASS)))
     elm_widget_mirrored_set(sd->content, rtl);
   elm_panel_orient_set(obj, elm_panel_orient_get(obj));
}

EOLIAN static void
_elm_panel_elm_layout_sizing_eval(Eo *obj, Elm_Panel_Data *sd)
{
   Evas_Coord mw = -1, mh = -1;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->delete_me) return;

   evas_object_smart_calculate(sd->bx);
   edje_object_size_min_calc(wd->resize_obj, &mw, &mh);
   evas_object_size_hint_min_set(obj, mw, mh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static char *
_access_state_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   ELM_PANEL_DATA_GET(data, sd);

   if (!sd->hidden) return strdup(E_("state: opened"));
   else return strdup(E_("state: closed"));

   return NULL;
}

static Evas_Object *
_access_object_get(const Evas_Object *obj, const char *part)
{
   Evas_Object *po, *ao;
   ELM_PANEL_DATA_GET(obj, sd);

   po = (Evas_Object *)edje_object_part_object_get
      (elm_layout_edje_get(sd->scr_ly), part);
   ao = evas_object_data_get(po, "_part_access_obj");

   return ao;
}

static void
_access_activate_cb(void *data,
                    Evas_Object *part_obj EINA_UNUSED,
                    Elm_Object_Item *item EINA_UNUSED)
{
   elm_panel_hidden_set(data, EINA_TRUE);
}

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   Evas_Object *ao;
   ELM_PANEL_DATA_GET(obj, sd);

   if (is_access)
     {
        ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
        if (!ao)
          {
             ao = _elm_access_edje_object_part_object_register
                (obj, elm_layout_edje_get(sd->scr_ly), ACCESS_OUTLINE_PART);
             _elm_access_text_set(_elm_access_info_get(ao),
                                  ELM_ACCESS_TYPE, E_("A drawer is open"));
             _elm_access_text_set(_elm_access_info_get(ao),
                                  ELM_ACCESS_CONTEXT_INFO, E_("Double tap to close drawer menu"));
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
   ELM_PANEL_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
        if (!elm_layout_theme_set
              (obj, "panel", "top", elm_widget_style_get(obj)))
          CRI("Failed to set layout!");
        break;

      case ELM_PANEL_ORIENT_BOTTOM:
        if (!elm_layout_theme_set
              (obj, "panel", "bottom", elm_widget_style_get(obj)))
          CRI("Failed to set layout!");
        break;

      case ELM_PANEL_ORIENT_LEFT:
        if (!elm_widget_mirrored_get(obj))
          {
             if (!elm_layout_theme_set
                   (obj, "panel", "left", elm_widget_style_get(obj)))
               CRI("Failed to set layout!");
          }
        else
          {
             if (!elm_layout_theme_set
                   (obj, "panel", "right", elm_widget_style_get(obj)))
               CRI("Failed to set layout!");
          }
        break;

      case ELM_PANEL_ORIENT_RIGHT:
        if (!elm_widget_mirrored_get(obj))
          {
             if (!elm_layout_theme_set
                   (obj, "panel", "right", elm_widget_style_get(obj)))
               CRI("Failed to set layout!");
          }
        else
          {
             if (!elm_layout_theme_set
                   (obj, "panel", "left", elm_widget_style_get(obj)))
               CRI("Failed to set layout!");
          }
        break;
     }

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
_scrollable_layout_theme_set(Eo *obj, Elm_Panel_Data *sd)
{
   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
         if (!elm_layout_theme_set(sd->scr_ly, "scroller", "panel/top",
                                   elm_widget_style_get(obj)))
           CRI("Failed to set layout!");
         break;
      case ELM_PANEL_ORIENT_BOTTOM:
         if (!elm_layout_theme_set(sd->scr_ly, "scroller", "panel/bottom",
                                   elm_widget_style_get(obj)))
           CRI("Failed to set layout!");
         break;
      case ELM_PANEL_ORIENT_LEFT:
         if (!elm_layout_theme_set(sd->scr_ly, "scroller", "panel/left",
                                   elm_widget_style_get(obj)))
           CRI("Failed to set layout!");
         break;
      case ELM_PANEL_ORIENT_RIGHT:
         if (!elm_layout_theme_set(sd->scr_ly, "scroller", "panel/right",
                                   elm_widget_style_get(obj)))
           CRI("Failed to set layout!");
         break;
     }

   /* access */
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_obj_process(obj, EINA_TRUE);
}

EOLIAN static Eina_Bool
_elm_panel_elm_widget_theme_apply(Eo *obj, Elm_Panel_Data *sd)
{
   const char *str;
   Evas_Coord minw = 0, minh = 0;

   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   if (sd->scrollable)
     {
        const char *handler_size;
        elm_widget_theme_object_set(obj, sd->scr_edje, "scroller", "panel",
                                    elm_widget_style_get(obj));
        _scrollable_layout_theme_set(obj, sd);

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

        _orient_set_do(obj);

        evas_object_hide(sd->event);
        elm_coords_finger_size_adjust(1, &minw, 1, &minh);
        evas_object_size_hint_min_set(sd->event, minw, minh);

        if (edje_object_part_exists
            (wd->resize_obj, "elm.swallow.event"))
          elm_layout_content_set(obj, "elm.swallow.event", sd->event);
     }

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_panel_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Panel_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_panel_elm_widget_focus_next(Eo *obj, Elm_Panel_Data *sd, Elm_Focus_Direction dir, Evas_Object **next)
{
   Evas_Object *cur;
   Eina_List *items = NULL;
   Eina_Bool ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!sd->content) return EINA_FALSE;

   if (sd->scrollable)
     {
        if (sd->hidden) return EINA_FALSE;

        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
          {
             Evas_Object *ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
             if (ao) items = eina_list_append(items, ao);
             items = eina_list_append(items, sd->content);

             ret = elm_widget_focus_list_next_get
                (obj, items, eina_list_data_get, dir, next);
             eina_list_free(items);

             return ret;
          }

        return elm_widget_focus_next_get(sd->content, dir, next);
     }

   cur = sd->content;

   /* Try to Focus cycle in subitem */
   if (!sd->hidden) return elm_widget_focus_next_get(cur, dir, next);

   /* access */
   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
     {
        Evas_Object *ao, *po;
        po = (Evas_Object *)edje_object_part_object_get
               (wd->resize_obj, "btn_icon");
        ao = evas_object_data_get(po, "_part_access_obj");
        _elm_access_highlight_set(ao);
     }

   /* Return */
   *next = (Evas_Object *)obj;
   return !elm_widget_focus_get(obj);
}

static void
_box_layout_cb(Evas_Object *o,
               Evas_Object_Box_Data *priv,
               void *data EINA_UNUSED)
{
   _els_box_layout(o, priv, EINA_TRUE, EINA_FALSE, EINA_FALSE);
}

static void
_handler_open(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   ELM_PANEL_DATA_GET(obj, sd);

   if (sd->handler_size == 0) return;

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
         eo_do(obj, elm_interface_scrollable_region_bring_in
               (0, (h * sd->content_size_ratio) - sd->handler_size, w, h));
         break;
      case ELM_PANEL_ORIENT_BOTTOM:
         eo_do(obj, elm_interface_scrollable_region_bring_in
               (0, sd->handler_size, w, h));
         break;
      case ELM_PANEL_ORIENT_LEFT:
         eo_do(obj, elm_interface_scrollable_region_bring_in
               ((w * sd->content_size_ratio) - sd->handler_size, 0, w, h));
         break;
      case ELM_PANEL_ORIENT_RIGHT:
         eo_do(obj, elm_interface_scrollable_region_bring_in
               (sd->handler_size, 0, w, h));
         break;
     }
}

static void
_drawer_open(Evas_Object *obj, Evas_Coord w, Evas_Coord h, Eina_Bool anim)
{
   ELM_PANEL_DATA_GET(obj, sd);
   int x = 0, y = 0;

   if (sd->freeze)
     {
        eo_do(obj, elm_interface_scrollable_movement_block_set
				(ELM_SCROLLER_MOVEMENT_NO_BLOCK));
        sd->freeze = EINA_FALSE;
        elm_layout_signal_emit(sd->scr_ly, "elm,state,content,visible", "elm");
     }

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
      case ELM_PANEL_ORIENT_LEFT:
         break;

      case ELM_PANEL_ORIENT_BOTTOM:
         y = h * sd->content_size_ratio;
         break;

      case ELM_PANEL_ORIENT_RIGHT:
         x = w * sd->content_size_ratio;
         break;
     }

   if (anim)
     eo_do(obj, elm_interface_scrollable_region_bring_in
           (x, y, w, h));
   else
     eo_do(obj, elm_interface_scrollable_content_region_show
           (x, y, w, h));
}

static void
_drawer_close(Evas_Object *obj, Evas_Coord w, Evas_Coord h, Eina_Bool anim)
{
   ELM_PANEL_DATA_GET(obj, sd);
   int x = 0, y = 0;
   Eina_Bool horizontal = EINA_FALSE;

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
         y = h * sd->content_size_ratio;
         break;

      case ELM_PANEL_ORIENT_LEFT:
         x = w * sd->content_size_ratio;
         horizontal = EINA_TRUE;
         break;

      case ELM_PANEL_ORIENT_BOTTOM:
         break;
      case ELM_PANEL_ORIENT_RIGHT:
         horizontal = EINA_TRUE;
         break;
     }

   if (anim)
     {
        if (sd->freeze)
          {
             eo_do(obj, elm_interface_scrollable_movement_block_set
                   (ELM_SCROLLER_MOVEMENT_NO_BLOCK));
             sd->freeze = EINA_FALSE;
             elm_layout_signal_emit(sd->scr_ly, "elm,state,content,visible", "elm");
          }
        eo_do(obj, elm_interface_scrollable_region_bring_in(x, y, w, h));
     }
   else
     {
        eo_do(obj, elm_interface_scrollable_content_region_show(x, y, w, h));
        if (!sd->freeze)
          {
             if (horizontal)
               eo_do(obj, elm_interface_scrollable_movement_block_set
                     (ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL));
             else
               eo_do(obj, elm_interface_scrollable_movement_block_set
                     (ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL));
             sd->freeze = EINA_TRUE;
             elm_layout_signal_emit(sd->scr_ly, "elm,state,content,hidden", "elm");
          }
     }
}

static void
_panel_toggle(void *data EINA_UNUSED,
              Evas_Object *obj,
              const char *emission EINA_UNUSED,
              const char *source EINA_UNUSED)
{
   ELM_PANEL_DATA_GET(obj, sd);
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
             elm_layout_signal_emit(obj, "elm,action,show", "elm");
             sd->hidden = EINA_FALSE;
             evas_object_repeat_events_set(obj, EINA_FALSE);
          }
        else
          {
             elm_layout_signal_emit(obj, "elm,action,hide", "elm");
             sd->hidden = EINA_TRUE;
             evas_object_repeat_events_set(obj, EINA_TRUE);
             if (sd->content && elm_widget_focus_get(sd->content))
               {
                  elm_widget_focused_object_clear(obj);
                  elm_widget_focus_steal(obj);
               }
          }

        edje_object_message_signal_process(wd->resize_obj);
     }
}

static Eina_Bool
_state_sync(Evas_Object *obj)
{
   ELM_PANEL_DATA_GET(obj, sd);
   Evas_Object *ao;
   Evas_Coord pos, panel_size, w, h;
   Eina_Bool open = EINA_FALSE, horizontal = EINA_FALSE;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
         panel_size = h * sd->content_size_ratio;
         eo_do(obj, elm_interface_scrollable_content_pos_get(NULL, &pos));

         if (pos == 0) open = EINA_TRUE;
         else if (pos == panel_size) open = EINA_FALSE;
         else return EINA_FALSE;
         break;

      case ELM_PANEL_ORIENT_BOTTOM:
         panel_size = h * sd->content_size_ratio;
         eo_do(obj, elm_interface_scrollable_content_pos_get(NULL, &pos));

         if (pos == panel_size) open = EINA_TRUE;
         else if (pos == 0) open = EINA_FALSE;
         else return EINA_FALSE;
         break;

      case ELM_PANEL_ORIENT_LEFT:
         panel_size = w * sd->content_size_ratio;
         eo_do(obj, elm_interface_scrollable_content_pos_get(&pos, NULL));
         horizontal = EINA_TRUE;

         if (pos == 0) open = EINA_TRUE;
         else if (pos == panel_size) open = EINA_FALSE;
         else return EINA_FALSE;
         break;

      case ELM_PANEL_ORIENT_RIGHT:
         panel_size = w * sd->content_size_ratio;
         eo_do(obj, elm_interface_scrollable_content_pos_get(&pos, NULL));
         horizontal = EINA_TRUE;

         if (pos == panel_size) open = EINA_TRUE;
         else if (pos == 0) open = EINA_FALSE;
         else return EINA_FALSE;
         break;
     }

   if (open)
     {
        if (sd->hidden) sd->hidden = EINA_FALSE;
        eo_do(obj, elm_interface_scrollable_single_direction_set
              (ELM_SCROLLER_SINGLE_DIRECTION_HARD));

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
        if (!sd->hidden) sd->hidden = EINA_TRUE;

        if (horizontal)
          eo_do(obj, elm_interface_scrollable_movement_block_set
                (ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL));
        else
          eo_do(obj, elm_interface_scrollable_movement_block_set
                (ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL));
        sd->freeze = EINA_TRUE;
        elm_layout_signal_emit(sd->scr_ly, "elm,state,content,hidden", "elm");

        eo_do(obj, elm_interface_scrollable_single_direction_set
              (ELM_SCROLLER_SINGLE_DIRECTION_NONE));

        //focus & access
        elm_object_tree_focus_allow_set(obj, EINA_FALSE);
        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
          {
             ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
             evas_object_hide(ao);
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_timer_cb(void *data)
{
   ELM_PANEL_DATA_GET(data, sd);
   Evas_Object *obj = data;
   Evas_Coord w, h;

   sd->timer = NULL;

   if (sd->freeze)
     {
        eo_do(obj, elm_interface_scrollable_movement_block_set
              (ELM_SCROLLER_MOVEMENT_NO_BLOCK));
        sd->freeze = EINA_FALSE;
        elm_layout_signal_emit(sd->scr_ly, "elm,state,content,visible", "elm");
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
   ELM_PANEL_DATA_GET(data, sd);
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Coord x, y, up_x, up_y, minw = 0, minh = 0;
   evas_object_geometry_get(data, &x, &y, NULL, NULL);

   up_x = ev->output.x - x;
   up_y = ev->output.y - y;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);

   if ((!sd->hidden) && (up_x == sd->down_x) && (up_y == sd->down_y))
     elm_panel_hidden_set(data, EINA_TRUE);
}

static void
_on_mouse_down(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj,
               void *event_info)
{
   Elm_Panel_Data *sd = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord finger_size = elm_config_finger_size_get();
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(obj, &x, &y, &w, &h);

   sd->down_x = ev->output.x - x;
   sd->down_y = ev->output.y - y;

   // if freeze state & mouse down on the edge
   // then set timer for un-freeze
   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
         if ((sd->freeze) && (sd->down_y >= 0) && (sd->down_y < finger_size))
           {
              ecore_timer_del(sd->timer);
              sd->timer = ecore_timer_add(0.2, _timer_cb, obj);
           }
         break;
      case ELM_PANEL_ORIENT_BOTTOM:
         if ((sd->freeze) && (sd->down_y <= h) && (sd->down_y > (h - finger_size)))
           {
              ecore_timer_del(sd->timer);
              sd->timer = ecore_timer_add(0.2, _timer_cb, obj);
           }
         break;
      case ELM_PANEL_ORIENT_LEFT:
         if ((sd->freeze) && (sd->down_x >= 0) && (sd->down_x < finger_size))
           {
              ecore_timer_del(sd->timer);
              sd->timer = ecore_timer_add(0.2, _timer_cb, obj);
           }
         break;
      case ELM_PANEL_ORIENT_RIGHT:
         if ((sd->freeze) && (sd->down_x <= w) && (sd->down_x > (w - finger_size)))
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
   Elm_Panel_Data *sd = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord x, y, w, h, cur_x, cur_y, finger_size;
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   finger_size = elm_config_finger_size_get();

   cur_x = ev->cur.canvas.x - x;
   cur_y = ev->cur.canvas.y - y;

   // if mouse down on the edge (it means sd->timer is not null)
   //    and move more than finger size
   // then un-freeze
   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
         if (sd->timer && ((cur_y - sd->down_y) > finger_size))
           {
              eo_do(obj, elm_interface_scrollable_freeze_set(EINA_FALSE));
              sd->freeze = EINA_FALSE;
              elm_layout_signal_emit(sd->scr_ly, "elm,state,content,visible", "elm");
           }
         break;
      case ELM_PANEL_ORIENT_BOTTOM:
         if (sd->timer && ((sd->down_y - cur_y) > finger_size))
           {
              eo_do(obj, elm_interface_scrollable_freeze_set(EINA_FALSE));
              sd->freeze = EINA_FALSE;
              elm_layout_signal_emit(sd->scr_ly, "elm,state,content,visible", "elm");
           }
         break;
      case ELM_PANEL_ORIENT_LEFT:
         if (sd->timer && ((cur_x - sd->down_x) > finger_size))
           {
              eo_do(obj, elm_interface_scrollable_freeze_set(EINA_FALSE));
              sd->freeze = EINA_FALSE;
              elm_layout_signal_emit(sd->scr_ly, "elm,state,content,visible", "elm");
           }
         break;
      case ELM_PANEL_ORIENT_RIGHT:
         if (sd->timer && ((sd->down_x - cur_x) > finger_size))
           {
              eo_do(obj, elm_interface_scrollable_freeze_set(EINA_FALSE));
              sd->freeze = EINA_FALSE;
              elm_layout_signal_emit(sd->scr_ly, "elm,state,content,visible", "elm");
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
   Elm_Panel_Data *sd = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Coord panel_size, threshold, pos, w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);

   if (_state_sync(obj)) return;

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
         panel_size = h * sd->content_size_ratio;
         threshold = panel_size / 4;
         eo_do(obj, elm_interface_scrollable_content_pos_get(NULL, &pos));

         if (sd->hidden)
           {
              if (pos < (panel_size - threshold)) _drawer_open(obj, w, h, EINA_TRUE);
              else _drawer_close(obj, w, h, EINA_TRUE);
           }
         else
           {
              if (pos < threshold) _drawer_open(obj, w, h, EINA_TRUE);
              else _drawer_close(obj, w, h, EINA_TRUE);
           }
         break;

      case ELM_PANEL_ORIENT_BOTTOM:
         panel_size = h * sd->content_size_ratio;
         threshold = panel_size / 4;
         eo_do(obj, elm_interface_scrollable_content_pos_get(NULL, &pos));

         if (sd->hidden)
           {
              if (pos > threshold) _drawer_open(obj, w, h, EINA_TRUE);
              else _drawer_close(obj, w, h, EINA_TRUE);
           }
         else
           {
              if (pos > (panel_size - threshold)) _drawer_open(obj, w, h, EINA_TRUE);
              else _drawer_close(obj, w, h, EINA_TRUE);
           }
         break;

      case ELM_PANEL_ORIENT_LEFT:
         panel_size = w * sd->content_size_ratio;
         threshold = panel_size / 4;
         eo_do(obj, elm_interface_scrollable_content_pos_get(&pos, NULL));

         if (sd->hidden)
           {
              if (pos < (panel_size - threshold)) _drawer_open(obj, w, h, EINA_TRUE);
              else _drawer_close(obj, w, h, EINA_TRUE);
           }
         else
           {
              if (pos < threshold) _drawer_open(obj, w, h, EINA_TRUE);
              else _drawer_close(obj, w, h, EINA_TRUE);
           }
         break;

      case ELM_PANEL_ORIENT_RIGHT:
         panel_size = w * sd->content_size_ratio;
         threshold = panel_size / 4;
         eo_do(obj, elm_interface_scrollable_content_pos_get(&pos, NULL));

         if (sd->hidden)
           {
              if (pos > threshold) _drawer_open(obj, w, h, EINA_TRUE);
              else _drawer_close(obj, w, h, EINA_TRUE);
           }
         else
           {
              if (pos > (panel_size - threshold)) _drawer_open(obj, w, h, EINA_TRUE);
              else _drawer_close(obj, w, h, EINA_TRUE);
           }
         break;
     }

   if (!sd->freeze && sd->hidden)
     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
}

static Eina_Bool
_key_action_toggle(Evas_Object *obj, const char *params EINA_UNUSED)
{
   _panel_toggle(NULL, obj, NULL, NULL);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_panel_elm_widget_event(Eo *obj, Elm_Panel_Data *_pd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (src != obj) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, ev, key_actions)) return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_panel_elm_container_content_set(Eo *obj, Elm_Panel_Data *sd, const char *part, Evas_Object *content)
{
   if (part && strcmp(part, "default"))
     {
        Eina_Bool int_ret = EINA_TRUE;
        eo_do_super(obj, MY_CLASS, int_ret = elm_obj_container_content_set(part, content));
        return int_ret;
     }

   if (sd->content == content) return EINA_TRUE;
   if (sd->content)
     evas_object_box_remove_all(sd->bx, EINA_TRUE);
   sd->content = content;
   if (content)
     {
        evas_object_box_append(sd->bx, sd->content);
        evas_object_show(sd->content);
     }

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

EOLIAN static Evas_Object*
_elm_panel_elm_container_content_get(Eo *obj, Elm_Panel_Data *sd, const char *part)
{
   if (part && strcmp(part, "default"))
     {
        Evas_Object *ret = NULL;
        eo_do_super(obj, MY_CLASS, ret = elm_obj_container_content_get(part));
        return ret;
     }

   return sd->content;
}

EOLIAN static Evas_Object*
_elm_panel_elm_container_content_unset(Eo *obj, Elm_Panel_Data *sd, const char *part)
{
   Evas_Object *ret = NULL;

   if (part && strcmp(part, "default"))
     {
        eo_do_super(obj, MY_CLASS, ret = elm_obj_container_content_unset(part));
        return ret;
     }

   if (!sd->content) return NULL;
   ret = sd->content;

   evas_object_box_remove_all(sd->bx, EINA_FALSE);
   sd->content = NULL;

   return ret;
}

EOLIAN static void
_elm_panel_evas_object_smart_add(Eo *obj, Elm_Panel_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->panel_edje = wd->resize_obj;

   eo_do(obj, elm_obj_widget_theme_apply());

   priv->bx = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_size_hint_align_set(priv->bx, 0.5, 0.5);
   evas_object_box_layout_set(priv->bx, _box_layout_cb, priv, NULL);
   evas_object_show(priv->bx);

   elm_layout_signal_callback_add
     (obj, "elm,action,panel,toggle", "*", _panel_toggle, obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   priv->event = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(priv->event, 0, 0, 0, 0);
   evas_object_pass_events_set(priv->event, EINA_TRUE);
   elm_widget_sub_object_add(obj, priv->event);

   /* just to bootstrap and have theme hook to work */
   if (!elm_layout_theme_set(obj, "panel", "top", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
   else
     {
        elm_layout_content_set(obj, "elm.swallow.content", priv->bx);

        if (edje_object_part_exists
            (wd->resize_obj, "elm.swallow.event"))
          {
             Evas_Coord minw = 0, minh = 0;

             elm_coords_finger_size_adjust(1, &minw, 1, &minh);
             evas_object_size_hint_min_set(priv->event, minw, minh);
             elm_layout_content_set(obj, "elm.swallow.event", priv->event);
          }
     }

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_panel_evas_object_smart_del(Eo *obj, Elm_Panel_Data *sd)
{
   Evas_Object *child;
   Eina_List *l;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   sd->delete_me = EINA_TRUE;

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);

   /* let's make our box object the *last* to be processed, since it
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

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static void
_elm_panel_evas_object_smart_move(Eo *obj, Elm_Panel_Data *sd, Evas_Coord x, Evas_Coord y)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   evas_object_move(sd->hit_rect, x, y);
}

static Eina_Bool
_elm_panel_anim_cb(void *data)
{
   Evas_Object *obj = data;
   ELM_PANEL_DATA_GET(obj, sd);
   int w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   if (sd->hidden) _drawer_close(obj, w, h, EINA_FALSE);
   else _drawer_open(obj, w, h, EINA_FALSE);

   return ECORE_CALLBACK_CANCEL;
}

EOLIAN static void
_elm_panel_evas_object_smart_resize(Eo *obj, Elm_Panel_Data *sd, Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   if (!sd->scrollable) return;

   evas_object_resize(sd->hit_rect, w, h);

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
      case ELM_PANEL_ORIENT_BOTTOM:
         // vertical
         evas_object_resize(sd->scr_ly, w, (1 + sd->content_size_ratio) * h);
         evas_object_size_hint_min_set(sd->scr_panel, w, (sd->content_size_ratio * h));
         evas_object_size_hint_min_set(sd->scr_event, w, h);
         break;
      case ELM_PANEL_ORIENT_LEFT:
      case ELM_PANEL_ORIENT_RIGHT:
         // horizontal
         evas_object_resize(sd->scr_ly, (1 + sd->content_size_ratio) * w, h);
         evas_object_size_hint_min_set(sd->scr_panel, (sd->content_size_ratio * w), h);
         evas_object_size_hint_min_set(sd->scr_event, w, h);
         break;
     }

   ecore_animator_add(_elm_panel_anim_cb, obj);
}

EOLIAN static void
_elm_panel_evas_object_smart_member_add(Eo *obj, Elm_Panel_Data *sd, Evas_Object *member)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_member_add(member));

   if (sd->hit_rect) evas_object_raise(sd->hit_rect);
}

EOLIAN static void
_elm_panel_elm_widget_access(Eo *obj, Elm_Panel_Data *_pd, Eina_Bool is_access)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Elm_Panel_Data *sd = _pd;

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

EAPI Evas_Object *
elm_panel_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   wd->highlight_root = EINA_TRUE;

   return obj;
}

EOLIAN static void
_elm_panel_eo_base_constructor(Eo *obj, Elm_Panel_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks),
         elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_PANEL));
}

EOLIAN static void
_elm_panel_orient_set(Eo *obj, Elm_Panel_Data *sd, Elm_Panel_Orient orient)
{
   if (sd->orient == orient) return;
   sd->orient = orient;

   if (sd->scrollable) _scrollable_layout_theme_set(obj, sd);
   else _orient_set_do(obj);

   elm_layout_sizing_eval(obj);
}

EOLIAN static Elm_Panel_Orient
_elm_panel_orient_get(Eo *obj EINA_UNUSED, Elm_Panel_Data *sd)
{
   return sd->orient;
}

EOLIAN static void
_elm_panel_hidden_set(Eo *obj, Elm_Panel_Data *sd, Eina_Bool hidden)
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
_elm_panel_hidden_get(Eo *obj EINA_UNUSED, Elm_Panel_Data *sd)
{
   return sd->hidden;
}

EOLIAN static void
_elm_panel_toggle(Eo *obj, Elm_Panel_Data *_pd EINA_UNUSED)
{
   _panel_toggle(NULL, obj, NULL, NULL);
}

EOLIAN static Eina_Bool
_elm_panel_elm_widget_on_focus_region(Eo *obj,
                                      Elm_Panel_Data *sd,
                                      Evas_Coord *x,
                                      Evas_Coord *y,
                                      Evas_Coord *w,
                                      Evas_Coord *h)
{
   eo_do(obj, elm_interface_scrollable_content_pos_get(x, y));
   evas_object_geometry_get(obj, NULL, NULL, w, h);
   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
      case ELM_PANEL_ORIENT_BOTTOM:
         *h = *h * sd->content_size_ratio;
         break;
      case ELM_PANEL_ORIENT_LEFT:
      case ELM_PANEL_ORIENT_RIGHT:
         *w = *w * sd->content_size_ratio;
         break;
     }
   return EINA_TRUE;
}

static void
_anim_stop_cb(Evas_Object *obj, void *data EINA_UNUSED)
{
   if (elm_widget_disabled_get(obj)) return;
   _state_sync(obj);
}

static void
_scroll_cb(Evas_Object *obj, void *data EINA_UNUSED)
{
   ELM_PANEL_DATA_GET(obj, sd);
   Elm_Panel_Scroll_Info event;
   Evas_Coord x, y, w, h;

   if (elm_widget_disabled_get(obj)) return;
   // in the case of
   // freeze_set(FALSE) -> mouse_up -> freeze_set(TRUE) -> scroll
   if (sd->freeze)
     {
        eo_do(obj, elm_interface_scrollable_movement_block_set
              (ELM_SCROLLER_MOVEMENT_NO_BLOCK));
        sd->freeze = EINA_FALSE;
        elm_layout_signal_emit(sd->scr_ly, "elm,state,content,visible", "elm");
     }

   eo_do(obj, elm_interface_scrollable_content_pos_get(&x, &y));
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
         event.rel_x = 1;
         event.rel_y = 1 - ((double) y / (double) ((sd->content_size_ratio) * h));
        break;
      case ELM_PANEL_ORIENT_BOTTOM:
         event.rel_x = 1;
         event.rel_y = (double) y / (double) ((sd->content_size_ratio) * h);
        break;
      case ELM_PANEL_ORIENT_LEFT:
         event.rel_x = 1 - ((double) x / (double) ((sd->content_size_ratio) * w));
         event.rel_y = 1;
        break;
      case ELM_PANEL_ORIENT_RIGHT:
         event.rel_x = (double) x / (double) ((sd->content_size_ratio) * w);
         event.rel_y = 1;
        break;
     }
   evas_object_smart_callback_call(obj, SIG_SCROLL, (void *) &event);
}

EOLIAN static Eina_Bool
_elm_panel_elm_widget_disable(Eo *obj, Elm_Panel_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_disable());
   if (!int_ret) return EINA_FALSE;

   if (sd->scrollable)
     {
        if (elm_widget_disabled_get(obj))
          {
             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                            _on_mouse_down);
             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                            _on_mouse_move);
             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_UP,
                                            _on_mouse_up);
             evas_object_event_callback_del(sd->scr_event, EVAS_CALLBACK_MOUSE_UP,
                                            _event_mouse_up);
          }
        else
          {
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                            _on_mouse_down, sd);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                            _on_mouse_move, sd);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                            _on_mouse_up, sd);
             evas_object_event_callback_add(sd->scr_event, EVAS_CALLBACK_MOUSE_UP,
                                            _event_mouse_up, obj);
          }
     }

   return EINA_TRUE;
}

EOLIAN static void
_elm_panel_scrollable_content_size_set(Eo *obj EINA_UNUSED, Elm_Panel_Data *sd, double ratio)
{
   sd->content_size_ratio = ratio;
}

EOLIAN static void
_elm_panel_scrollable_set(Eo *obj, Elm_Panel_Data *sd, Eina_Bool scrollable)
{
   scrollable = !!scrollable;
   if (sd->scrollable == scrollable) return;
   sd->scrollable = scrollable;

   if (scrollable)
     {
        elm_layout_content_unset(obj, "elm.swallow.content");

        elm_widget_resize_object_set(obj, NULL, EINA_TRUE);
        elm_widget_sub_object_add(obj, sd->panel_edje);

        if (!sd->scr_edje)
          {
             const char *handler_size;

             sd->scr_edje = edje_object_add(evas_object_evas_get(obj));
             elm_widget_theme_object_set(obj, sd->scr_edje, "scroller", "panel",
                                         elm_widget_style_get(obj));
             evas_object_size_hint_weight_set
                (sd->scr_edje, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set
                (sd->scr_edje, EVAS_HINT_FILL, EVAS_HINT_FILL);

             handler_size = edje_object_data_get(sd->scr_edje, "handler_size");
             if (handler_size)
               sd->handler_size = (int) (elm_object_scale_get(obj)) * (atoi(handler_size));
          }

        elm_widget_resize_object_set(obj, sd->scr_edje, EINA_TRUE);

        if (!sd->hit_rect)
          {
             sd->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
             evas_object_smart_member_add(sd->hit_rect, obj);
             elm_widget_sub_object_add(obj, sd->hit_rect);
             evas_object_color_set(sd->hit_rect, 0, 0, 0, 0);
             evas_object_show(sd->hit_rect);
             evas_object_repeat_events_set(sd->hit_rect, EINA_TRUE);

             eo_do(obj,
                   elm_interface_scrollable_objects_set(sd->scr_edje, sd->hit_rect),
                   elm_interface_scrollable_animate_stop_cb_set(_anim_stop_cb),
                   elm_interface_scrollable_scroll_cb_set(_scroll_cb));
          }

        if (!sd->scr_ly)
          {
             sd->scr_ly = elm_layout_add(obj);
             evas_object_smart_member_add(sd->scr_ly, obj);
             elm_widget_sub_object_add(obj, sd->scr_ly);
             _scrollable_layout_theme_set(obj, sd);

             sd->scr_panel = evas_object_rectangle_add(evas_object_evas_get(obj));
             evas_object_color_set(sd->scr_panel, 0, 0, 0, 0);
             elm_widget_sub_object_add(obj, sd->scr_panel);
             elm_layout_content_set(sd->scr_ly, "panel_area", sd->scr_panel);

             sd->scr_event = evas_object_rectangle_add(evas_object_evas_get(obj));
             evas_object_color_set(sd->scr_event, 0, 0, 0, 0);
             elm_widget_sub_object_add(obj, sd->scr_event);
             elm_layout_content_set(sd->scr_ly, "event_area", sd->scr_event);
          }

        eo_do(obj,
              elm_interface_scrollable_content_set(sd->scr_ly));
        sd->freeze = EINA_TRUE;
        elm_layout_content_set(sd->scr_ly, "elm.swallow.content", sd->bx);

        switch (sd->orient)
          {
           case ELM_PANEL_ORIENT_TOP:
           case ELM_PANEL_ORIENT_BOTTOM:
              eo_do(obj, elm_interface_scrollable_movement_block_set
                    (ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL));
              break;
           case ELM_PANEL_ORIENT_LEFT:
           case ELM_PANEL_ORIENT_RIGHT:
              eo_do(obj, elm_interface_scrollable_movement_block_set
                    (ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL));
              break;
          }

        eo_do(obj, elm_interface_scrollable_single_direction_set
              (ELM_SCROLLER_SINGLE_DIRECTION_NONE));

        if (!elm_widget_disabled_get(obj))
          {
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                            _on_mouse_down, sd);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                            _on_mouse_move, sd);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                            _on_mouse_up, sd);
             evas_object_event_callback_add(sd->scr_event, EVAS_CALLBACK_MOUSE_UP,
                                            _event_mouse_up, obj);
          }

     }
   else
     {
        eo_do(obj, elm_interface_scrollable_content_set(NULL));

        evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down);
        evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move);
        evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_UP, _on_mouse_up);
        evas_object_event_callback_del(sd->scr_event, EVAS_CALLBACK_MOUSE_UP,
                                       _event_mouse_up);

        elm_widget_resize_object_set(obj, NULL, EINA_TRUE);
        elm_widget_sub_object_add(obj, sd->scr_edje);

        elm_widget_resize_object_set(obj, sd->panel_edje, EINA_TRUE);

        elm_layout_content_unset(sd->scr_ly, "elm.swallow.content");
        elm_layout_content_set(obj, "elm.swallow.content", sd->bx);
     }
}

static void
_elm_panel_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_panel.eo.c"
