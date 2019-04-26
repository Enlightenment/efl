#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pager.h"
#include "efl_page_transition.h"

#define MY_CLASS EFL_UI_PAGER_CLASS

static int
clamp_index(Efl_Ui_Pager_Data *pd, int index)
{
   if (index < ((int)eina_list_count(pd->content_list)) * -1)
     return -1;
   else if (index > (int)eina_list_count(pd->content_list) - 1)
     return 1;
   return 0;
}

static int
index_rollover(Efl_Ui_Pager_Data *pd, int index)
{
   int c = eina_list_count(pd->content_list);
   if (index < c * -1)
     return 0;
   else if (index > c - 1)
     return c - 1;
   else if (index < 0)
     return index + c;
   return index;
}

static void
_efl_ui_pager_update(Efl_Ui_Pager_Data *pd)
{
   if (pd->cnt == 0) return;

   if (pd->transition)
     efl_page_transition_update(pd->transition, pd->curr.pos);

   if (pd->indicator)
     efl_page_indicator_update(pd->indicator, pd->curr.pos);
}

static void
_job(void *data)
{
   Evas_Object *obj = data;
   EFL_UI_PAGER_DATA_GET(obj, pd);

   int prev_page, page_diff;
   double prev_pos, pos_diff;

   prev_page = pd->curr.page;
   prev_pos = pd->curr.pos;

   pd->job = NULL;

   pos_diff = ((double) pd->down.x - (double) pd->mouse_x) / (double) pd->w;
   pd->curr.pos = pd->down.pos + pos_diff;
   page_diff = (int) pd->curr.pos;

   //FIXME what if (prev_pos != 0.0)
   if ((pd->loop == EFL_UI_PAGER_LOOP_DISABLED) &&
       (((prev_page == 0) && (pd->curr.pos < 0)) ||
        ((prev_page == (pd->cnt - 1)) && (pd->curr.pos > 0))))
     {
        pd->curr.page = prev_page;
        pd->curr.pos = prev_pos;
        return;
     }

   pd->curr.page = (pd->down.page + page_diff + pd->cnt) % pd->cnt;
   pd->curr.pos -= page_diff;

   //FIXME what if (page_diff >= 2 || page_diff <= -2)
   if (page_diff != 0)
     {
        pd->down.x = pd->mouse_x;
        pd->down.y = pd->mouse_y;
        pd->down.page = pd->curr.page;
        pd->down.pos = pd->curr.pos;

        efl_page_transition_curr_page_change(pd->transition, page_diff);
     }

   _efl_ui_pager_update(pd);

   return;
}

static void
_page_set_animation(void *data, const Efl_Event *event)
{
   Efl_Ui_Pager_Data *pd = data;
   double p = ecore_loop_time_get() - pd->change.start_time;
   double d, temp_pos;
   int temp_page;

   if (p >= 1.0) p = 1.0;
   p = ecore_animator_pos_map(p, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   d = pd->change.src + pd->change.delta * p;
   temp_page = d;
   temp_pos = d - temp_page;

   if ((temp_page < pd->curr.page) && (fabs(pd->curr.page - d) < 1.0))
     {
        temp_page += 1;
        temp_pos -= 1.0;
     }

   if (pd->curr.page != temp_page)
     {
        if (pd->change.delta < 0)
          efl_page_transition_curr_page_change(pd->transition, -1);
        else
          efl_page_transition_curr_page_change(pd->transition, 1);
        temp_pos = 0.0;
     }

   pd->curr.page = temp_page;
   pd->curr.pos = temp_pos;

   ERR("page %d pos %lf", pd->curr.page, pd->curr.pos);

   _efl_ui_pager_update(pd);

   if (EINA_DBL_EQ(p, 1.0))
     efl_event_callback_del(event->object, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK,
                            _page_set_animation, pd);
}

static void
_mouse_up_animation(void *data, const Efl_Event *event)
{
   Efl_Ui_Pager_Data *pd = data;
   double p = ecore_loop_time_get() - pd->mouse_up_time;

   if (p >= 1.0) p = 1.0;
   p = ecore_animator_pos_map(p, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   if (pd->curr.pos < 0.0)
     {
        if (pd->curr.pos > -0.5)
          pd->curr.pos = pd->curr.pos * (1 - p);
        else
          pd->curr.pos = (-1) - (-1 - pd->curr.pos) * (1 - p);
     }
   else
     {
        if (pd->curr.pos < 0.5)
          pd->curr.pos = pd->curr.pos * (1 - p);
        else
          pd->curr.pos = 1 - (1 - pd->curr.pos) * (1 - p);
     }

   if (EINA_DBL_EQ(pd->curr.pos, 1.0))
     {
        efl_page_transition_curr_page_change(pd->transition, 1);
        pd->curr.page = (pd->curr.page + 1 + pd->cnt) % pd->cnt;
        pd->curr.pos = 0.0;
     }
   else if (EINA_DBL_EQ(pd->curr.pos, -1.0))
     {
        efl_page_transition_curr_page_change(pd->transition, -1);
        pd->curr.page = (pd->curr.page - 1 + pd->cnt) % pd->cnt;
        pd->curr.pos = 0.0;
     }

   _efl_ui_pager_update(pd);

   if (EINA_DBL_EQ(p, 1.0))
     efl_event_callback_del(event->object, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK,
                            _mouse_up_animation, pd);
}

static void
_mouse_down_cb(void *data,
               const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Eo *obj = data;
   EFL_UI_PAGER_DATA_GET(obj, pd);
   Eina_Position2D pos;

   if (efl_input_pointer_button_get(ev) != 1) return;
   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;

   if (pd->cnt == 0) return;

   efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _mouse_up_animation, pd);
   efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _page_set_animation, pd);

   pd->move_started = EINA_FALSE;

   pos = efl_input_pointer_position_get(ev);
   pd->mouse_x = pos.x - pd->x;
   pd->mouse_y = pos.y - pd->y;

   pd->down.enabled = EINA_TRUE;
   pd->down.x = pd->mouse_x;
   pd->down.y = pd->mouse_y;
   pd->down.page = pd->curr.page;
   pd->down.pos = pd->curr.pos;
}

static void
_mouse_move_cb(void *data,
               const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Eo *obj = data;
   EFL_UI_PAGER_DATA_GET(obj, pd);
   Eina_Position2D pos;

   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;
   if (!pd->down.enabled) return;

   pos = efl_input_pointer_position_get(ev);

   if (pd->prev_block && (pd->mouse_x < (pos.x - pd->x))) return;
   if (pd->next_block && (pd->mouse_x > (pos.x - pd->x))) return;

   pd->mouse_x = pos.x - pd->x;
   pd->mouse_y = pos.y - pd->y;

   if (!pd->move_started)
     {
        Evas_Coord dx, dy;
        dx = pd->mouse_x - pd->down.x;
        dy = pd->mouse_y - pd->down.y;

        if (((dx * dx) + (dy * dy)) <=
            (_elm_config->finger_size * _elm_config->finger_size / 4))
          return;

        pd->move_started = EINA_TRUE;
     }

   ecore_job_del(pd->job);
   pd->job = ecore_job_add(_job, obj);
}

static void
_mouse_up_cb(void *data,
             const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Eo *obj = data;
   EFL_UI_PAGER_DATA_GET(obj, pd);

   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;
   if (!pd->down.enabled) return;

   pd->down.enabled = EINA_FALSE;

   ELM_SAFE_FREE(pd->job, ecore_job_del);

   if (EINA_DBL_EQ(pd->curr.pos, 0.0)) return;

   pd->mouse_up_time = ecore_loop_time_get();

   efl_event_callback_add(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _mouse_up_animation, pd);
}

//FIXME sub_object_parent_add? destruction
static void
_event_handler_create(Eo *obj, Efl_Ui_Pager_Data *pd)
{
   pd->event = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                       evas_object_evas_get(obj));
   evas_object_color_set(pd->event, 0, 0, 0, 0);
   evas_object_repeat_events_set(pd->event, EINA_TRUE);

   efl_content_set(efl_part(obj, "efl.event"), pd->event);

   efl_event_callback_add(pd->event, EFL_EVENT_POINTER_DOWN,
                          _mouse_down_cb, obj);
   efl_event_callback_add(pd->event, EFL_EVENT_POINTER_UP,
                          _mouse_up_cb, obj);
   efl_event_callback_add(pd->event, EFL_EVENT_POINTER_MOVE,
                          _mouse_move_cb, obj);
}

static void
_event_handler_del(Eo *obj, Efl_Ui_Pager_Data *pd)
{
   efl_content_unset(efl_part(obj, "efl.event"));
   efl_del(pd->event);
   pd->event = NULL;
}

static void
_resize_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Pager_Data *pd = data;
   Eina_Size2D sz;

   sz = efl_gfx_entity_size_get(ev->object);

   pd->w = sz.w;
   pd->h = sz.h;

   if (pd->fill_width) pd->page_spec.sz.w = pd->w;
   if (pd->fill_height) pd->page_spec.sz.h = pd->h;

   if (pd->transition)
     efl_page_transition_page_size_set(pd->transition, pd->page_spec.sz);
   else
     {
        efl_gfx_entity_size_set(pd->foreclip, sz);
        efl_gfx_entity_size_set(pd->page_box, pd->page_spec.sz);
        efl_gfx_entity_position_set(pd->page_box,
                                    EINA_POSITION2D(pd->x + (pd->w / 2) - (pd->page_spec.sz.w / 2),
                                                    pd->y + (pd->h / 2) - (pd->page_spec.sz.h / 2)));
     }
}

static void
_move_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Pager_Data *pd = data;
   Eina_Position2D pos;

   pos = efl_gfx_entity_position_get(ev->object);

   pd->x = pos.x;
   pd->y = pos.y;

   if (!pd->transition)
     {
        efl_gfx_entity_position_set(pd->foreclip, pos);
        efl_gfx_entity_position_set(pd->page_box,
                             EINA_POSITION2D(pd->x + (pd->w / 2) - (pd->page_spec.sz.w / 2),
                                             pd->y + (pd->h / 2) - (pd->page_spec.sz.h / 2)));
     }
}

EOLIAN static Eo *
_efl_ui_pager_efl_object_constructor(Eo *obj,
                                     Efl_Ui_Pager_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "pager");

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

   pd->cnt = 0;
   pd->loop = EFL_UI_PAGER_LOOP_DISABLED;

   pd->curr.page = -1;
   pd->curr.pos = 0.0;

   pd->transition = NULL;
   pd->indicator = NULL;

   pd->fill_width = EINA_TRUE;
   pd->fill_height = EINA_TRUE;

   pd->page_spec.sz.w = -1;
   pd->page_spec.sz.h = -1;

   elm_widget_can_focus_set(obj, EINA_TRUE);

   pd->page_root = efl_add(EFL_CANVAS_GROUP_CLASS, evas_object_evas_get(obj));
   efl_content_set(efl_part(obj, "efl.page_root"), pd->page_root);

   efl_event_callback_add(pd->page_root, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _resize_cb, pd);
   efl_event_callback_add(pd->page_root, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _move_cb, pd);

   pd->page_box = efl_add(EFL_UI_BOX_CLASS, obj);
   efl_ui_widget_internal_set(pd->page_box, EINA_TRUE);
   efl_canvas_group_member_add(pd->page_root, pd->page_box);

   pd->foreclip = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                          evas_object_evas_get(obj));
   efl_canvas_group_member_add(pd->page_root, pd->foreclip);
   evas_object_static_clip_set(pd->foreclip, EINA_TRUE);
   efl_canvas_object_clipper_set(pd->page_box, pd->foreclip);

   pd->backclip = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                          evas_object_evas_get(obj));
   efl_canvas_group_member_add(pd->page_root, pd->backclip);
   evas_object_static_clip_set(pd->backclip, EINA_TRUE);
   efl_gfx_entity_visible_set(pd->backclip, EINA_FALSE);

   return obj;
}

EOLIAN static void
_efl_ui_pager_efl_object_invalidate(Eo *obj,
                                    Efl_Ui_Pager_Data *pd)
{
   efl_invalidate(efl_super(obj, MY_CLASS));

   /* Since the parent of foreclip and backclip is evas, foreclip and backclip
    * are not deleted automatically when pager is deleted.
    * Therefore, foreclip and backclip are deleted manually here. */
   efl_del(pd->foreclip);
   efl_del(pd->backclip);
}

EOLIAN static int
_efl_ui_pager_efl_container_content_count(Eo *obj EINA_UNUSED,
                                          Efl_Ui_Pager_Data *pd)
{
   return pd->cnt;
}

static Eina_Bool
_register_child(Eo *obj EINA_UNUSED, Efl_Ui_Pager_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (eina_list_data_find(pd->content_list, subobj))
     {
        ERR("Object already added!");
        return EINA_FALSE;
     }
   if (!efl_ui_widget_sub_object_add(obj, subobj))
     return EINA_FALSE;

   if (!pd->transition)
     efl_canvas_object_clipper_set(subobj, pd->backclip);

   return EINA_TRUE;
}

static void
_update_internals(Eo *obj EINA_UNUSED, Efl_Ui_Pager_Data *pd, Efl_Gfx_Entity *subobj EINA_UNUSED, int index)
{
   pd->cnt++;

   if (pd->curr.page >= index)
     pd->curr.page++;

   if (pd->transition)
     efl_page_transition_pack(pd->transition, index);

   if (pd->indicator)
     efl_page_transition_pack(pd->indicator, index);

   if (pd->cnt == 1)
     efl_ui_pager_current_page_set(obj, 0);
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_begin(Eo *obj EINA_UNUSED,
                                         Efl_Ui_Pager_Data *pd,
                                         Efl_Gfx_Entity *subobj)
{
   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   pd->content_list = eina_list_prepend(pd->content_list, subobj);
   _update_internals(obj, pd, subobj, 0);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_end(Eo *obj EINA_UNUSED,
                                       Efl_Ui_Pager_Data *pd,
                                       Efl_Gfx_Entity *subobj)
{
   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   pd->content_list = eina_list_append(pd->content_list, subobj);
   _update_internals(obj, pd, subobj, eina_list_count(pd->content_list) - 1);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_before(Eo *obj EINA_UNUSED,
                                          Efl_Ui_Pager_Data *pd,
                                          Efl_Gfx_Entity *subobj,
                                          const Efl_Gfx_Entity *existing)
{
   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   int index = eina_list_data_idx(pd->content_list, (void *)existing);
   if (index == -1) return EINA_FALSE;
   pd->content_list = eina_list_prepend_relative(pd->content_list, subobj, existing);
   _update_internals(obj, pd, subobj, index);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_after(Eo *obj EINA_UNUSED,
                                         Efl_Ui_Pager_Data *pd,
                                         Efl_Gfx_Entity *subobj,
                                         const Efl_Gfx_Entity *existing)
{
   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   int index = eina_list_data_idx(pd->content_list, (void *)existing);
   if (index == -1) return EINA_FALSE;
   pd->content_list = eina_list_append_relative(pd->content_list, subobj, existing);
   _update_internals(obj, pd, subobj, index + 1);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_at(Eo *obj,
                                      Efl_Ui_Pager_Data *pd,
                                      Efl_Gfx_Entity *subobj,
                                      int index)
{
   if (index == pd->cnt)
     {
        _efl_ui_pager_efl_pack_linear_pack_end(obj, pd, subobj);
     }
   else
     {
        Efl_Gfx_Entity *existing = NULL;

        if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
        int clamp = clamp_index(pd, index);
        int pass_index = -1;
        if (clamp == 0)
          {
             existing = eina_list_nth(pd->content_list, index_rollover(pd, index));
             pd->content_list = eina_list_prepend_relative(
                pd->content_list, subobj, existing);
          }
        else if (clamp == 1)
          {
             pd->content_list = eina_list_append(pd->content_list, subobj);
             pass_index = eina_list_count(pd->content_list);
          }
        else
          {
             pd->content_list = eina_list_prepend(pd->content_list, subobj);
             pass_index = 0;
          }
        _update_internals(obj, pd, subobj, pass_index);
     }

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_pager_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED,
                                               Efl_Ui_Pager_Data *pd,
                                               int index)
{
   return eina_list_nth(pd->content_list, index_rollover(pd, index));
}

EOLIAN static int
_efl_ui_pager_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                             Efl_Ui_Pager_Data *pd,
                                             const Efl_Gfx_Entity *subobj)
{
   return eina_list_data_idx(pd->content_list, (void *)subobj);
}

EOLIAN static void
_efl_ui_pager_current_page_set(Eo *obj,
                               Efl_Ui_Pager_Data *pd,
                               int index)
{
   if (index == pd->curr.page) return;

   efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _mouse_up_animation, pd);
   efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _page_set_animation, pd);

   if ((index < 0) || (index > (pd->cnt - 1)))
     {
        ERR("page set fail");
        return;
     }

   if (!pd->transition)
     {
        Eo *curr;

        curr = eina_list_nth(pd->content_list, pd->curr.page);
        if (curr)
          efl_pack_unpack(pd->page_box, curr);
        efl_canvas_object_clipper_set(curr, pd->backclip);

        pd->curr.page = index;
        curr = eina_list_nth(pd->content_list, pd->curr.page);
        efl_pack(pd->page_box, curr);

        if (pd->indicator)
          efl_page_indicator_update(pd->indicator, pd->curr.pos);

        return;
     }

   pd->change.src = pd->curr.page + pd->curr.pos;
   pd->change.delta = index - pd->change.src;

   if (pd->change.delta == 0) return;

   pd->change.start_time = ecore_loop_time_get();
   efl_event_callback_add(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _page_set_animation, pd);
}

EOLIAN static int
_efl_ui_pager_current_page_get(const Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *pd)
{
   return pd->curr.page;
}

EOLIAN static void
_efl_ui_pager_transition_set(Eo *obj,
                             Efl_Ui_Pager_Data *pd,
                             Efl_Page_Transition *transition)
{
   if (!EINA_DBL_EQ(pd->curr.pos, 0.0)) return;

   if (pd->transition == transition) return;

   if (pd->transition)
     efl_page_transition_bind(pd->transition, NULL, NULL);
   else
     {
        Eo *curr;

        curr = eina_list_nth(pd->content_list, pd->curr.page);
        efl_pack_unpack(pd->page_box, curr);
        efl_canvas_object_clipper_set(pd->page_box, pd->backclip);
     }

   pd->transition = transition;

   if (pd->transition)
     {
        if (!pd->event) _event_handler_create(obj, pd);
        efl_page_transition_bind(pd->transition, obj, pd->page_root);
     }
   else
     {
        Eina_List *list;
        Eo *curr;

        _event_handler_del(obj, pd);

        efl_canvas_object_clipper_set(pd->page_box, pd->foreclip);

        EINA_LIST_FOREACH(pd->content_list, list, curr)
          {
             efl_canvas_object_clipper_set(curr, pd->backclip);
          }

        curr = eina_list_nth(pd->content_list, pd->curr.page);
        efl_pack(pd->page_box, curr);
     }
}

EOLIAN static void
_efl_ui_pager_indicator_set(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd,
                            Efl_Page_Indicator *indicator)
{
   if (pd->indicator == indicator) return;

   if (pd->indicator)
     {
        efl_page_indicator_bind(pd->indicator, NULL, NULL);
        pd->indicator = NULL;
     }

   pd->indicator = indicator;

   if (!pd->indicator)
     {
        if (pd->idbox)
          {
             efl_del(pd->idbox);
             pd->idbox = NULL;
          }
        return;
     }

   if (!pd->idbox)
     {
        pd->idbox = efl_add(EFL_UI_BOX_CLASS, obj);
        efl_ui_widget_internal_set(pd->idbox, EINA_TRUE);
        efl_content_set(efl_part(obj, "efl.indicator"), pd->idbox);
     }

   efl_page_indicator_bind(pd->indicator, obj, pd->idbox);
}

EOLIAN Eina_Size2D
_efl_ui_pager_page_size_get(const Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd)
{
   return pd->page_spec.sz;
}

EOLIAN static void
_efl_ui_pager_page_size_set(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd,
                            Eina_Size2D sz)
{
   if (sz.w < -1 || sz.h < -1) return;

   if (sz.w == -1)
     {
        pd->fill_width = EINA_TRUE;
        pd->page_spec.sz.w = pd->w;
     }
   else
     {
        pd->fill_width = EINA_FALSE;
        pd->page_spec.sz.w = sz.w;
     }
   if (sz.h == -1)
     {
        pd->fill_height = EINA_TRUE;
        pd->page_spec.sz.h = pd->h;
     }
   else
     {
        pd->fill_height = EINA_FALSE;
        pd->page_spec.sz.h = sz.h;
     }

   if (pd->transition)
     efl_page_transition_page_size_set(pd->transition, pd->page_spec.sz);
   else
     {
        efl_gfx_entity_size_set(pd->page_box, pd->page_spec.sz);
        efl_gfx_entity_position_set(pd->page_box,
                                    EINA_POSITION2D(pd->x + (pd->w / 2) - (pd->page_spec.sz.w / 2),
                                                    pd->y + (pd->h / 2) - (pd->page_spec.sz.h / 2)));
     }
}

EOLIAN static int
_efl_ui_pager_padding_get(const Eo *obj EINA_UNUSED,
                          Efl_Ui_Pager_Data *pd)
{
   return pd->page_spec.padding;
}

EOLIAN static void
_efl_ui_pager_padding_set(Eo *obj EINA_UNUSED,
                          Efl_Ui_Pager_Data *pd,
                          int padding)
{
   pd->page_spec.padding = padding;

   if (pd->transition)
     efl_page_transition_padding_size_set(pd->transition, padding);
}

EOLIAN static void
_efl_ui_pager_scroll_block_get(const Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *pd,
                               Eina_Bool *prev,
                               Eina_Bool *next)
{
   if (prev) *prev = pd->prev_block;
   if (next) *next = pd->next_block;
}

EOLIAN static void
_efl_ui_pager_scroll_block_set(Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *pd,
                               Eina_Bool prev,
                               Eina_Bool next)
{
   pd->prev_block = prev;
   pd->next_block = next;
}

EOLIAN static Eina_Bool
_efl_ui_pager_loop_mode_set(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd,
                            Efl_Ui_Pager_Loop loop)
{
   if (pd->loop == loop) return EINA_TRUE;

   if (!pd->transition) return EINA_FALSE;

   if (efl_page_transition_loop_set(pd->transition, loop))
     {
        pd->loop = loop;
        return EINA_TRUE;
     }
   else return EINA_FALSE;
}

EOLIAN static Efl_Ui_Pager_Loop
_efl_ui_pager_loop_mode_get(const Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd)
{
   return pd->loop;
}

static void
_unpack_all(Efl_Ui_Pager_Data *pd,
            Eina_Bool clear)
{
   Eo *subobj;

   pd->cnt = 0;
   pd->curr.page = -1;
   pd->curr.pos = 0.0;

   if (pd->transition)
     {
        efl_page_transition_unpack_all(pd->transition);
     }
   else
     {
        subobj = eina_list_nth(pd->content_list, pd->curr.page);
        if (subobj)
          efl_pack_unpack(pd->page_box, subobj);
        pd->curr.page = -1;
     }

   if (clear)
     {
        EINA_LIST_FREE(pd->content_list, subobj)
           evas_object_del(subobj);
     }
   else
     {
        EINA_LIST_FREE(pd->content_list, subobj)
           efl_canvas_object_clipper_set(subobj, NULL);
     }

   if (pd->indicator)
     {
        efl_page_indicator_unpack_all(pd->indicator);
     }
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_pack_clear(Eo *obj EINA_UNUSED,
                                  Efl_Ui_Pager_Data *pd)
{
   _unpack_all(pd, EINA_TRUE);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_unpack_all(Eo *obj EINA_UNUSED,
                                  Efl_Ui_Pager_Data *pd)
{
   _unpack_all(pd, EINA_FALSE);

   return EINA_TRUE;
}

static void
_unpack(Eo *obj,
        Efl_Ui_Pager_Data *pd,
        Efl_Gfx_Entity *subobj,
        int index)
{
   int self_index = eina_list_data_idx(pd->content_list, subobj);
   int self_curr_page = pd->curr.page;
   pd->content_list = eina_list_remove(pd->content_list, subobj);
   pd->cnt--;

   if (((index == pd->curr.page) && ((index != 0) || (pd->cnt == 0))) ||
       (index < pd->curr.page))
     pd->curr.page--;

   if (pd->transition)
     {
        // if the number of pages is not enough after unpacking a page,
        // loop mode needs to be disabled
        if (pd->loop == EFL_UI_PAGER_LOOP_ENABLED)
          {
             _efl_ui_pager_loop_mode_set(obj, pd, EFL_UI_PAGER_LOOP_DISABLED);
             _efl_ui_pager_loop_mode_set(obj, pd, EFL_UI_PAGER_LOOP_ENABLED);
          }
        efl_page_transition_update(pd->transition, pd->curr.pos);
     }
   else
     {
        if (self_curr_page == self_index)
          {
             efl_pack_unpack(pd->page_box, subobj);
             self_curr_page = pd->curr.page;
             pd->curr.page = -1;
             efl_ui_pager_current_page_set(obj, self_curr_page);
          }
     }

   if (pd->indicator)
     efl_page_indicator_unpack(pd->indicator, index);
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_unpack(Eo *obj,
                              Efl_Ui_Pager_Data *pd,
                              Efl_Gfx_Entity *subobj)
{
   if (!subobj) return EINA_FALSE;

   int index = eina_list_data_idx(pd->content_list, subobj);
   if (index == -1) return EINA_FALSE;

   _unpack(obj, pd, subobj, index);

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_pager_efl_pack_linear_pack_unpack_at(Eo *obj,
                                             Efl_Ui_Pager_Data *pd,
                                             int index)
{
   Efl_Gfx_Entity *subobj = eina_list_nth(pd->content_list, index_rollover(pd, index_rollover(pd, index)));

   _unpack(obj, pd, subobj, index);

   return subobj;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_pack(Eo *obj, Efl_Ui_Pager_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   return efl_pack_begin(obj, subobj);
}

EOLIAN static Eina_Iterator*
_efl_ui_pager_efl_container_content_iterate(Eo *obj EINA_UNUSED, Efl_Ui_Pager_Data *pd)
{
  return eina_list_iterator_new(pd->content_list);
}


#include "efl_ui_pager.eo.c"
