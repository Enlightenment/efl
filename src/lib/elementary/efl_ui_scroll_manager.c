#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_scroll_manager.h"

#define MY_CLASS EFL_UI_SCROLL_MANAGER_CLASS
#define MY_CLASS_NAME "Efl_Ui_Scroll_Manager"

#define MY_SCROLLABLE_INTERFACE ELM_INTERFACE_SCROLLABLE_MIXIN
#define MY_PAN_CLASS ELM_PAN_CLASS
#define LEGACY 0


EOLIAN static void
_efl_ui_scroll_manager_scrollbar_mode_set(Eo *obj,
                                          Efl_Ui_Scroll_Manager_Data *sd,
                                          Efl_Ui_Scrollbar_Mode hmode,
                                          Efl_Ui_Scrollbar_Mode vmode)
{
#if LEGACY
   elm_interface_scrollable_policy_set(obj, hmode, vmode);
#else
#endif
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_scroll_manager_efl_ui_focus_user_manager_get(Eo *obj EINA_UNUSED,
                                                     Efl_Ui_Scroll_Manager_Data *sd)
{
   return efl_ui_focus_user_manager_get(sd->parent);
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_scroll_manager_efl_ui_focus_user_parent_get(Eo *obj EINA_UNUSED,
                                                    Efl_Ui_Scroll_Manager_Data *sd)
{
   return efl_ui_focus_user_parent_get(sd->parent);
}

EOLIAN static void
_efl_ui_scroll_manager_efl_gfx_size_set(Eo *obj EINA_UNUSED,
                                        Efl_Ui_Scroll_Manager_Data *sd,
                                        Evas_Coord w,
                                        Evas_Coord h)
{
   efl_gfx_size_set(sd->event_obj, w, h);
}

EOLIAN static void
_efl_ui_scroll_manager_efl_gfx_position_set(Eo *obj EINA_UNUSED,
                                            Efl_Ui_Scroll_Manager_Data *sd,
                                            Evas_Coord x,
                                            Evas_Coord y)
{
   efl_gfx_position_set(sd->event_obj, x, y);
}

#if LEGACY
static void
_viewport_resize_cb(Evas_Object *obj, Evas_Coord w EINA_UNUSED, Evas_Coord h EINA_UNUSED)
{
   Evas_Coord minw, minh;
   Efl_Ui_Scroll_Manager_Data *sd = efl_data_scope_get(obj, EFL_UI_SCROLL_MANAGER_CLASS);

   efl_gfx_size_hint_combined_min_get(sd->content.obj, &minw, &minh);
   efl_gfx_size_set(sd->content.obj, minw, minh);
}
#else
static void
_edje_object_resize_cb(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Coord w, h;
   efl_gfx_size_get(obj, &w, &h);

   sd->edje_obj.w = w;
   sd->edje_obj.h = h;
}

static void
_edje_object_move_cb(void *data,
                     Evas *e EINA_UNUSED,
                     Evas_Object *obj,
                     void *event_info EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Coord x, y;
   efl_gfx_position_get(obj, &x, &y);

   sd->edje_obj.x = x;
   sd->edje_obj.y = y;
}

static void
_mouse_down_cb(void *data,
               Evas *e,
               Evas_Object *obj,
               void *event_info)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Event_Mouse_Down *ev = event_info;

   sd->down.on = EINA_TRUE;
   sd->down.x = ev->canvas.x;
   sd->down.y = ev->canvas.y;

   sd->prev_x = sd->down.x;
   sd->prev_y = sd->down.y;
}

static void
_mouse_move_cb(void *data,
               Evas *e,
               Evas_Object *obj,
               void *event_info)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord cur_x, cur_y, dx, dy, x, y;

   if (!sd->down.on) return;

   cur_x = ev->cur.canvas.x;
   cur_y = ev->cur.canvas.y;
   dx = cur_x - sd->prev_x;
   dy = cur_y - sd->prev_y;

   elm_obj_pan_pos_get(sd->pan_obj, &x, &y);
   elm_obj_pan_pos_set(sd->pan_obj, (x - dx), (y - dy));

   sd->prev_x = cur_x;
   sd->prev_y = cur_y;
}

static void
_mouse_up_cb(void *data,
             Evas *e,
             Evas_Object *obj,
             void *event_info)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Event_Mouse_Up *ev = event_info;

   sd->down.on = EINA_FALSE;
}

static void
_edje_object_cb_add(Efl_Ui_Scroll_Manager_Data *sd)
{
   evas_object_event_callback_add
      (sd->edje_obj.obj, EVAS_CALLBACK_RESIZE, _edje_object_resize_cb, sd);
   evas_object_event_callback_add
      (sd->edje_obj.obj, EVAS_CALLBACK_MOVE, _edje_object_move_cb, sd);
}

static void
_edje_object_cb_del(Efl_Ui_Scroll_Manager_Data *sd)
{
   evas_object_event_callback_del_full
      (sd->edje_obj.obj, EVAS_CALLBACK_RESIZE, _edje_object_resize_cb, sd);
   evas_object_event_callback_del_full
      (sd->edje_obj.obj, EVAS_CALLBACK_MOVE, _edje_object_move_cb, sd);
}

static void
_event_object_cb_add(Efl_Ui_Scroll_Manager_Data *sd)
{
   evas_object_event_callback_add
      (sd->event_obj, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, sd);
   evas_object_event_callback_add
      (sd->event_obj, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, sd);
   evas_object_event_callback_add
      (sd->event_obj, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, sd);
}

static void
_event_object_cb_del(Efl_Ui_Scroll_Manager_Data *sd)
{
   evas_object_event_callback_del_full
      (sd->event_obj, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, sd);
   evas_object_event_callback_del_full
      (sd->event_obj, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, sd);
   evas_object_event_callback_del_full
      (sd->event_obj, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, sd);
}

static void
_pan_object_change_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Coord w, h;

   elm_obj_pan_content_size_get(sd->pan_obj, &w, &h);
   sd->content.w = w;
   sd->content.h = h;
   evas_object_size_hint_min_set(sd->edje_obj.obj, sd->content.w, sd->content.h);
}

static void
_pan_object_resize_cb(void *data,
                      Evas *e EINA_UNUSED,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Coord minw, minh;

   edje_object_calc_force(sd->edje_obj.obj);
   efl_gfx_size_hint_combined_min_get(sd->content.obj, &minw, &minh);
   efl_gfx_size_set(sd->content.obj, minw, minh);
}
#endif

EOLIAN static void
_efl_ui_scroll_manager_efl_canvas_group_group_add(Eo *obj,
                                                  Efl_Ui_Scroll_Manager_Data *sd)
{
   sd->parent = efl_parent_get(obj);
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->parent, wd);

#if LEGACY
   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   sd->event_obj = evas_object_rectangle_add(evas_object_evas_get(obj));
   efl_gfx_color_set(sd->event_obj, 0, 0, 0, 0);
   efl_gfx_visible_set(sd->event_obj, EINA_TRUE);

   elm_interface_scrollable_objects_set(obj, wd->resize_obj, sd->event_obj);

   elm_interface_scrollable_content_viewport_resize_cb_set(obj, _viewport_resize_cb);
#else
   efl_canvas_group_add(efl_super(obj, MY_SCROLLABLE_INTERFACE));

   sd->edje_obj.x = 0;
   sd->edje_obj.y = 0;
   sd->edje_obj.w = 0;
   sd->edje_obj.h = 0;

   if (sd->edje_obj.obj) _edje_object_cb_del(sd);
   sd->edje_obj.obj = wd->resize_obj;

   _edje_object_cb_add(sd);

   if (sd->event_obj) _event_object_cb_del(sd);

   sd->event_obj = evas_object_rectangle_add(evas_object_evas_get(obj));
   efl_gfx_color_set(sd->event_obj, 0, 0, 0, 0);
   efl_gfx_visible_set(sd->event_obj, EINA_TRUE);
   efl_canvas_object_repeat_events_set(sd->event_obj, EINA_TRUE);

   _event_object_cb_add(sd);
#endif
}

EOLIAN static void
_efl_ui_scroll_manager_content_set(Eo *obj,
                                   Efl_Ui_Scroll_Manager_Data *sd,
                                   Evas_Object *content)
{
   Evas_Coord w, h;
   if (sd->content.obj == content) return;

   sd->content.obj = content;
   efl_canvas_group_member_add(obj, content);
#if LEGACY
   elm_interface_scrollable_content_set(obj, content);
#else
   if (!sd->pan_obj)
     {
        sd->pan_obj = efl_add(MY_PAN_CLASS, evas_object_evas_get(obj));
        efl_event_callback_add(sd->pan_obj, ELM_PAN_EVENT_CHANGED,
                               _pan_object_change_cb, sd);
        evas_object_event_callback_add(sd->pan_obj, EVAS_CALLBACK_RESIZE,
                                       _pan_object_resize_cb, sd);
        edje_object_part_swallow(sd->edje_obj.obj, "elm.swallow.content",
                                 sd->pan_obj);
     }
   elm_obj_pan_content_set(sd->pan_obj, content);
   elm_obj_pan_content_size_get(sd->pan_obj, &w, &h);
   sd->content.w = w;
   sd->content.h = h;
#endif
}

EOLIAN static Eo *
_efl_ui_scroll_manager_efl_object_constructor(Eo *obj,
                                              Efl_Ui_Scroll_Manager_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}


#include "efl_ui_scroll_manager.eo.c"
