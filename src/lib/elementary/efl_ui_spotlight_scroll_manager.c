#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"

typedef struct {
   Efl_Ui_Spotlight_Container * container;
   Efl_Gfx_Entity *foreclip, *backclip;
   Efl_Canvas_Rectangle *event;
   Eina_Size2D page_size;
   struct {
      Eina_Bool active;
      int to;
      double from, progress;
      double start_time;
      double max_time;
   } transition;
   struct {
      Eina_Bool active;
      int from;
      Eina_Position2D mouse_start;
      double start_time;
   } mouse_move;
   Eina_Bool animation;
   Eina_Bool scroll_block;
} Efl_Ui_Spotlight_Scroll_Manager_Data;

#define MY_CLASS EFL_UI_SPOTLIGHT_SCROLL_MANAGER_CLASS

static void _page_set_animation(void *data, const Efl_Event *event);

static void
_propagate_progress(Eo *obj, double pos)
{
   efl_event_callback_call(obj, EFL_UI_SPOTLIGHT_MANAGER_EVENT_POS_UPDATE, &pos);
}

static void
_apply_box_properties(Eo *obj, Efl_Ui_Spotlight_Scroll_Manager_Data *pd)
{
   Eina_Rect geometry = EINA_RECT_EMPTY();
   Eina_Rect group_pos = efl_gfx_entity_geometry_get(pd->container);
   double current_pos;

   if (pd->transition.active)
     current_pos = pd->transition.from + ((double)pd->transition.to - pd->transition.from)*pd->transition.progress;
   else
     current_pos = efl_pack_index_get(pd->container, efl_ui_spotlight_active_element_get(pd->container));

   efl_gfx_entity_geometry_set(pd->event, group_pos);
   efl_gfx_entity_geometry_set(pd->foreclip, group_pos);
   //first calculate the size
   geometry.h = pd->page_size.h;
   geometry.w = (pd->page_size.w);

   //calculate the position
   geometry.y = (group_pos.y + group_pos.h/2)-pd->page_size.h/2;

   for (int i = 0; i < efl_content_count(pd->container) ; ++i)
     {
        double diff = i - current_pos;
        Efl_Gfx_Entity *elem = efl_pack_content_get(pd->container, i);

        geometry.x = (int)((group_pos.x + group_pos.w/2)-(pd->page_size.w/2 - diff*pd->page_size.w));
        if (!eina_rectangles_intersect(&geometry.rect, &group_pos.rect))
          {
             efl_canvas_object_clipper_set(elem, pd->backclip);
          }
        else
          {
             efl_gfx_entity_geometry_set(elem, geometry);
             efl_canvas_object_clipper_set(elem, pd->foreclip);
          }
     }

   //Propagate progress
   _propagate_progress(obj, current_pos);
}

static void
_mouse_down_cb(void *data,
               const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Eo *obj = data;
   Efl_Ui_Spotlight_Scroll_Manager_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   if (efl_input_pointer_button_get(ev) != 1) return;
   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;

   if (efl_content_count(pd->container) == 0) return;

   if (pd->scroll_block) return;

   efl_event_callback_del(pd->container, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _page_set_animation, obj);

   pd->mouse_move.active = EINA_TRUE;
   pd->mouse_move.from = efl_pack_index_get(pd->container, efl_ui_spotlight_active_element_get(pd->container));
   pd->mouse_move.mouse_start = efl_input_pointer_position_get(ev);
   pd->mouse_move.start_time = ecore_time_get();

   pd->transition.from = pd->mouse_move.from;
   pd->transition.to = pd->transition.from + 1;
   pd->transition.progress = 0.0;
}

static void
_mouse_move_cb(void *data,
               const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Eo *obj = data;
   Efl_Ui_Spotlight_Scroll_Manager_Data *pd = efl_data_scope_get(obj, MY_CLASS);
   Eina_Position2D pos;
   int pos_y_diff;

   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;
   if (!pd->mouse_move.active) return;
   if (pd->scroll_block) return;

   pos = efl_input_pointer_position_get(ev);
   pos_y_diff = pd->mouse_move.mouse_start.x - pos.x;

   //Set input processed not to cause clicked event to content button.
   if (!efl_input_processed_get(ev))
     efl_input_processed_set(ev, EINA_TRUE);

   if (pd->transition.active ||
       EINA_POSITION2D_DISTANCE(pd->mouse_move.mouse_start, efl_input_pointer_position_get(ev)) > elm_config_finger_size_get() ||
       ecore_time_get() - pd->mouse_move.start_time > 0.1)
     {
        pd->transition.active = EINA_TRUE;
        pd->transition.progress = (double)pos_y_diff / (double)pd->page_size.w;
        _propagate_progress(data, pd->transition.from + pd->transition.progress);
        _apply_box_properties(obj, pd);
     }
}

static void
_mouse_up_cb(void *data,
             const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Eo *obj = data;
   Efl_Ui_Spotlight_Scroll_Manager_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;
   if (!pd->mouse_move.active) return;
   if (pd->scroll_block) return;

   double absolut_current_position = (double)pd->transition.from + pd->transition.progress;
   int result = round(absolut_current_position);

   Efl_Ui_Widget *new_content = efl_pack_content_get(pd->container, MIN(MAX(result, 0), efl_content_count(pd->container) - 1));
   efl_ui_spotlight_active_element_set(pd->container, new_content);

   //Set input processed not to cause clicked event to content button.
   if (EINA_POSITION2D_DISTANCE(pd->mouse_move.mouse_start, efl_input_pointer_position_get(ev)) > elm_config_finger_size_get())
     efl_input_processed_set(ev, EINA_TRUE);
}

EFL_CALLBACKS_ARRAY_DEFINE(mouse_listeners,
  {EFL_EVENT_POINTER_DOWN, _mouse_down_cb},
  {EFL_EVENT_POINTER_UP, _mouse_up_cb},
  {EFL_EVENT_POINTER_MOVE, _mouse_move_cb},
);

EOLIAN static void
_efl_ui_spotlight_scroll_manager_efl_ui_spotlight_manager_bind(Eo *obj, Efl_Ui_Spotlight_Scroll_Manager_Data *pd, Efl_Ui_Spotlight_Container *spotlight)
{
   if (spotlight)
     {
        pd->container = spotlight;

        pd->foreclip = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                               evas_object_evas_get(pd->container));
        evas_object_static_clip_set(pd->foreclip, EINA_TRUE);
        efl_canvas_group_member_add(spotlight, pd->foreclip);

        pd->backclip = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                               evas_object_evas_get(pd->container));
        evas_object_static_clip_set(pd->backclip, EINA_TRUE);
        efl_gfx_entity_visible_set(pd->backclip, EINA_FALSE);
        efl_canvas_group_member_add(spotlight, pd->backclip);

        pd->event = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                       evas_object_evas_get(obj));
        efl_canvas_object_repeat_events_set(pd->event, EINA_TRUE);
        efl_event_callback_array_add(pd->event, mouse_listeners(), obj);
        efl_canvas_group_member_add(spotlight, pd->event);
        efl_gfx_color_set(pd->event, 0, 0, 0, 0);

        for (int i = 0; i < efl_content_count(spotlight) ; ++i) {
           Efl_Gfx_Entity *elem = efl_pack_content_get(spotlight, i);
           efl_key_data_set(elem, "_elm_leaveme", spotlight);
           efl_canvas_object_clipper_set(elem, pd->backclip);
           efl_canvas_group_member_add(pd->container, elem);
           efl_gfx_entity_visible_set(elem, EINA_TRUE);
           efl_gfx_stack_above(pd->event, elem);
        }
        _apply_box_properties(obj, pd);
     }
}

EOLIAN static void
_efl_ui_spotlight_scroll_manager_efl_ui_spotlight_manager_content_add(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Scroll_Manager_Data *pd, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   efl_key_data_set(subobj, "_elm_leaveme", pd->container);
   efl_gfx_entity_visible_set(subobj, EINA_TRUE);
   efl_canvas_object_clipper_set(subobj, pd->backclip);
   efl_canvas_group_member_add(pd->container, subobj);
   efl_gfx_stack_above(pd->event, subobj);
   if (!pd->transition.active)
     _apply_box_properties(obj, pd);
}


EOLIAN static void
_efl_ui_spotlight_scroll_manager_efl_ui_spotlight_manager_content_del(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Scroll_Manager_Data *pd, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   efl_key_data_set(subobj, "_elm_leaveme", NULL);
   efl_canvas_object_clipper_set(subobj, NULL);
   efl_canvas_group_member_remove(pd->container, subobj);

   if (!pd->transition.active)
     _apply_box_properties(obj, pd);
}

static void
_page_set_animation(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Spotlight_Scroll_Manager_Data *pd = efl_data_scope_get(data, MY_CLASS);
   double p = (ecore_loop_time_get() - pd->transition.start_time) / pd->transition.max_time;

   if (p >= 1.0) p = 1.0;
   p = ecore_animator_pos_map(p, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   pd->transition.progress = p;

   if (EINA_DBL_EQ(p, 1.0))
     {
        efl_event_callback_del(pd->container, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK,
                               _page_set_animation, data);
        pd->transition.active = EINA_FALSE;
        pd->transition.progress = 0.0;
     }
   _apply_box_properties(data, pd);
}

static void
_animation_request_switch(Eo *obj, Efl_Ui_Spotlight_Scroll_Manager_Data *pd, int from, int to)
{
   //if there is already a transition ongoing, which is no mouse transition, but goes to the same position, then do nothing
   if (pd->transition.active && !pd->mouse_move.active && pd->transition.to == to)
    return;

   //In case there is no transition but from and to are the same, ensure that we reset the mouse_move state, and return.
   if (!pd->transition.active && from == to)
     {
        pd->mouse_move.active = EINA_FALSE;
        return;
     }

   efl_event_callback_del(pd->container, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _page_set_animation, obj);
   //if there is a ongoing transition, try to guess a better time, and try copy over the position where we are right now
   if (pd->transition.active || pd->mouse_move.active)
     {
        pd->transition.from = MIN(pd->transition.from, pd->transition.to) + pd->transition.progress;
        pd->transition.max_time = MIN(MAX(fabs(pd->transition.progress), 0.2), 0.5f);
        pd->mouse_move.active = EINA_FALSE;
     }
   else
     {
        pd->transition.from = from;
        pd->transition.max_time = 0.5;
        pd->transition.progress = 0.0;
     }

   pd->transition.start_time = ecore_loop_time_get();
   pd->transition.active = EINA_TRUE;
   pd->transition.to = to;
   efl_event_callback_add(pd->container, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _page_set_animation, obj);
}

EOLIAN static void
_efl_ui_spotlight_scroll_manager_efl_ui_spotlight_manager_switch_to(Eo *obj, Efl_Ui_Spotlight_Scroll_Manager_Data *pd, int from, int to, Efl_Ui_Spotlight_Manager_Switch_Reason reason EINA_UNUSED)
{
   if (pd->animation)
     {
        _animation_request_switch(obj, pd, from, to);
     }
   else
     {
        pd->mouse_move.active = EINA_FALSE;
        pd->transition.active = EINA_FALSE;
        pd->transition.progress = 0.0;
        _apply_box_properties(obj, pd);
     }
}

EOLIAN static void
_efl_ui_spotlight_scroll_manager_efl_ui_spotlight_manager_size_set(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Scroll_Manager_Data *pd, Eina_Size2D size)
{
   pd->page_size = size;
   if (!pd->transition.active)
     _apply_box_properties(obj, pd);
}

EOLIAN static void
_efl_ui_spotlight_scroll_manager_efl_ui_spotlight_manager_animated_transition_set(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Scroll_Manager_Data *pd, Eina_Bool animation)
{
   pd->animation = animation;
   if (pd->transition.active && !animation)
     {
        efl_event_callback_del(pd->container, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _page_set_animation, obj);
        _apply_box_properties(obj, pd);
        pd->transition.active = EINA_FALSE;
     }
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_scroll_manager_efl_ui_spotlight_manager_animated_transition_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Scroll_Manager_Data *pd)
{
   return pd->animation;
}


EOLIAN static void
_efl_ui_spotlight_scroll_manager_efl_object_invalidate(Eo *obj, Efl_Ui_Spotlight_Scroll_Manager_Data *pd EINA_UNUSED)
{
   efl_del(pd->event);
   efl_del(pd->backclip);
   efl_del(pd->foreclip);

   for (int i = 0; i < efl_content_count(pd->container); ++i)
     {
        efl_canvas_object_clipper_set(efl_pack_content_get(pd->container, i), NULL);
     }

   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_spotlight_scroll_manager_scroll_block_set(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Scroll_Manager_Data *pd, Eina_Bool scroll_block)
{
   if (pd->scroll_block == scroll_block) return;

   pd->scroll_block = scroll_block;
   if (scroll_block && pd->mouse_move.active)
     {
        pd->mouse_move.active = EINA_FALSE;
        pd->transition.active = EINA_FALSE;
        pd->transition.progress = 0.0;
        _apply_box_properties(obj, pd);
     }
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_scroll_manager_scroll_block_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Scroll_Manager_Data *pd)
{
   return pd->scroll_block;
}

#include "efl_ui_spotlight_scroll_manager.eo.c"
