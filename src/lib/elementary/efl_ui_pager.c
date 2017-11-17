#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pager.h"
#include "efl_page_transition.h"

#define MY_CLASS EFL_UI_PAGER_CLASS


static void
_efl_ui_pager_update(Efl_Ui_Pager_Data *pd)
{
   if (pd->cnt == 0) return;

   efl_page_transition_update(pd->transition, pd->move);
}

static void
_job(void *data)
{
   Evas_Object *obj = data;
   EFL_UI_PAGER_DATA_GET(obj, pd);

   double t = 0;
   Eina_Bool prev = EINA_FALSE, next = EINA_FALSE;

   pd->job = NULL;

   if (pd->dir == EFL_UI_DIR_HORIZONTAL)
     t = ((double)pd->mouse_down.x - (double)pd->mouse_x) / (double)pd->w;
   else
     t = ((double)pd->mouse_down.y - (double)pd->mouse_y) / (double)pd->h;

   if (t > 1.0) t = 1.0;
   else if (t < -1.0) t = -1.0;

   if (pd->prev_block && (t < 0)) return;
   if (pd->next_block && (t > 0)) return;

   if (EINA_DBL_EQ(pd->move, t)) return;

   pd->move = t;

   if (t < 0) prev = EINA_TRUE;
   else if (t > 0) next = EINA_TRUE;

   if (pd->prev_block && prev) return;
   else if (pd->next_block && next) return;

   if ((pd->loop == EFL_UI_PAGER_LOOP_DISABLED)
       && ((next && pd->mouse_down.page == (pd->cnt - 1))
           || (prev && pd->mouse_down.page == 0))) return;

   _efl_ui_pager_update(pd);
}

static Eina_Bool
_animator(void *data, double pos)
{
   Eo *obj = data;
   double p;

   EFL_UI_PAGER_DATA_GET(obj, pd);

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   if (pd->move < 0.0)
     {
        if (pd->move > -0.5)
          pd->move = pd->move * (1 - p);
        else
          pd->move = (-1) - (-1 - pd->move) * (1 - p);
     }
   else
     {
        if (pd->move < 0.5)
          pd->move = pd->move * (1 - p);
        else
          pd->move = 1 - (1 - pd->move) * (1 - p);
     }

   _efl_ui_pager_update(pd);

   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   if (EINA_DBL_EQ(pd->move, 1.0) || EINA_DBL_EQ(pd->move, -1.0))
     {
        efl_page_transition_curr_page_change(pd->transition, pd->move);
        pd->current_page = (pd->current_page + (int) pd->move + pd->cnt) % pd->cnt;
        //TODO Call "page changed" callback
        pd->move = 0.0;
     }

   pd->animator = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down_cb(void *data,
               const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Evas_Object *pc = data;
   EFL_UI_PAGER_DATA_GET(pc, pd);
   Eina_Position2D pos;

   if (efl_input_pointer_button_get(ev) != 1) return;
   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;

   if (pd->move != 0.0) return; //FIXME

   ELM_SAFE_FREE(pd->animator, ecore_animator_del);

   pd->move_started = EINA_FALSE;
   pd->mouse_down.enabled = EINA_TRUE;

   pos = efl_input_pointer_position_get(ev);
   pd->mouse_x = pos.x - pd->x;
   pd->mouse_y = pos.y - pd->y;

   pd->mouse_down.x = pd->mouse_x;
   pd->mouse_down.y = pd->mouse_y;

   pd->mouse_down.page = pd->current_page;
}

static void
_mouse_move_cb(void *data,
               const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Evas_Object *pc = data;
   EFL_UI_PAGER_DATA_GET(pc, pd);
   Eina_Position2D pos;

   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;
   if (!pd->mouse_down.enabled) return;

   pos = efl_input_pointer_position_get(ev);
   pd->mouse_x = pos.x - pd->x;
   pd->mouse_y = pos.y - pd->y;

   if (!pd->move_started)
     {
        Evas_Coord dx, dy;
        dx = pd->mouse_x - pd->mouse_down.x;
        dy = pd->mouse_y - pd->mouse_down.y;

        if (((dx * dx) + (dy * dy)) <=
            (_elm_config->finger_size * _elm_config->finger_size / 4))
          return;

        pd->move_started = EINA_TRUE;
     }

   ecore_job_del(pd->job);
   pd->job = ecore_job_add(_job, pc);
}

static void
_mouse_up_cb(void *data,
             const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Evas_Object *pc = data;
   EFL_UI_PAGER_DATA_GET(pc, pd);
   double time;

   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;
   if (!pd->mouse_down.enabled) return;

   pd->mouse_down.enabled = EINA_FALSE;

   ELM_SAFE_FREE(pd->job, ecore_job_del);

   if ((pd->loop == EFL_UI_PAGER_LOOP_DISABLED)
       && (((pd->move > 0) && (pd->mouse_down.page == (pd->cnt - 1)))
           || ((pd->move < 0) && (pd->mouse_down.page == 0))))
     {
        pd->move = 0.0;
        return;
     }

   if (EINA_DBL_EQ(pd->move, 0.0)) return;
   if (EINA_DBL_EQ(pd->move, 1.0) || EINA_DBL_EQ(pd->move, -1.0))
     {
        efl_page_transition_curr_page_change(pd->transition, pd->move);
        pd->current_page = (pd->current_page + (int) pd->move + pd->cnt) % pd->cnt;
        //TODO Call "page changed" callback
        pd->move = 0.0;
        return;
     }

   if (pd->move < 0.0)
     {
        if (pd->move > -0.5) time = (-1) * pd->move;
        else time = 1 + pd->move;
     }
   else
     {
        if (pd->move < 0.5) time = pd->move;
        else time = 1 - pd->move;
     }

   if (time < 0.01) time = 0.01;
   else if (time > 0.99) time = 0.99;

   ecore_animator_del(pd->animator);
   pd->animator = ecore_animator_timeline_add(time, _animator, pc);
}

//FIXME sub_object_parent_add? destruction
static void
_event_handler_create(Eo *obj, Efl_Ui_Pager_Data *pd)
{
   pd->event = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                       evas_object_evas_get(obj));
   evas_object_color_set(pd->event, 0, 0, 0, 0);

   efl_event_callback_add(pd->event, EFL_EVENT_POINTER_DOWN,
                          _mouse_down_cb, obj);
   efl_event_callback_add(pd->event, EFL_EVENT_POINTER_UP,
                          _mouse_up_cb, obj);
   efl_event_callback_add(pd->event, EFL_EVENT_POINTER_MOVE,
                          _mouse_move_cb, obj);
}

EOLIAN static Eo *
_efl_ui_pager_efl_object_constructor(Eo *obj,
                                     Efl_Ui_Pager_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   elm_widget_sub_object_parent_add(obj);

   pd->cnt = 0;
   pd->current_page = -1;
   pd->move = 0.0;
   pd->dir = EFL_UI_DIR_HORIZONTAL;
   pd->loop = EFL_UI_PAGER_LOOP_DISABLED;

   _event_handler_create(obj, pd);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   return obj;
}

EOLIAN static void
_efl_ui_pager_efl_gfx_size_set(Eo *obj,
                               Efl_Ui_Pager_Data *pd,
                               Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   if ((pd->w == sz.w) && (pd->h == sz.h)) return;

   efl_gfx_size_set(efl_super(obj, MY_CLASS), sz);

   pd->w = sz.w;
   pd->h = sz.h;

   efl_gfx_size_set(pd->event, sz);
}

EOLIAN static void
_efl_ui_pager_efl_gfx_position_set(Eo *obj,
                                   Efl_Ui_Pager_Data *pd,
                                   Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   if ((pd->x == pos.x) && (pd->y == pos.y)) return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), pos);

   pd->x = pos.x;
   pd->y = pos.y;

   efl_gfx_position_set(pd->event, pos);
}

EOLIAN static int
_efl_ui_pager_efl_container_content_count(Eo *obj EINA_UNUSED,
                                          Efl_Ui_Pager_Data *pd)
{
   return pd->cnt;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_end(Eo *obj,
                                       Efl_Ui_Pager_Data *pd,
                                       Efl_Gfx *subobj)
{
   efl_parent_set(subobj, obj);

   pd->content_list = eina_list_append(pd->content_list, subobj);
   efl_gfx_stack_above(pd->event, subobj);

   pd->packed_page.index = pd->cnt;
   pd->packed_page.obj = subobj;

   if (pd->cnt == 0) pd->current_page = 0;
   pd->cnt += 1;

   efl_page_transition_update(pd->transition, pd->move);

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx *
_efl_ui_pager_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED,
                                               Efl_Ui_Pager_Data *pd,
                                               int index)
{
   return eina_list_nth(pd->content_list, index);
}

EOLIAN static int
_efl_ui_pager_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                             Efl_Ui_Pager_Data *pd,
                                             const Efl_Gfx *subobj)
{
   return eina_list_data_idx(pd->content_list, (void *)subobj);
}

static Eina_Bool
_change_animator(void *data, double pos)
{
   Eo *obj = data;
   EFL_UI_PAGER_DATA_GET(obj, pd);
   double p, d, move;
   int page;
   Eina_Bool changed = EINA_FALSE;

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   d = pd->change.src + pd->change.delta * p;
   page = d;
   move = d - page;

   if ((pd->change.delta < 0) && (move > 0))
     {
        page += 1;
        move -= 1.0;
     }

   if (pd->current_page != page)
     {
        if (pd->change.delta < 0)
          {
             page += 1;
             move = -1.0;
          }
        else
          {
             page -= 1;
             move = 1.0;
          }
        changed = EINA_TRUE;
     }

   pd->move = move;
   _efl_ui_pager_update(pd);

   if (changed)
     {
        efl_page_transition_curr_page_change(pd->transition, pd->move);
        pd->current_page = (page + (int) move + pd->cnt) % pd->cnt;
        pd->move = 0.0;
     }

   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   pd->change.animator = NULL;

   return ECORE_CALLBACK_CANCEL;
}

EOLIAN static void
_efl_ui_pager_current_page_set(Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *pd,
                               int index)
{
   double time;

   pd->change.src = pd->current_page + pd->move;
   pd->change.dst = index;

   pd->change.delta = pd->change.dst - pd->change.src;

   if (pd->change.delta == 0) return;

   time = pd->change.delta;
   if (pd->change.delta < 0) time *= (-1);
   time /= pd->cnt;

   ecore_animator_del(pd->change.animator);
   pd->change.animator = ecore_animator_timeline_add(time, _change_animator, obj);
}

EOLIAN static int
_efl_ui_pager_current_page_get(Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *pd)
{
   return pd->current_page;
}

EOLIAN static Efl_Page_Transition *
_efl_ui_pager_transition_get(Eo *obj EINA_UNUSED,
                             Efl_Ui_Pager_Data *pd)
{
   return pd->transition;
}

EOLIAN static void
_efl_ui_pager_transition_set(Eo *obj EINA_UNUSED,
                             Efl_Ui_Pager_Data *pd,
                             Efl_Page_Transition *transition)
{
   efl_page_transition_bind(transition, obj);
   pd->transition = transition;
}

EOLIAN static Efl_Ui_Dir
_efl_ui_pager_efl_ui_direction_direction_get(Eo *obj EINA_UNUSED,
                                             Efl_Ui_Pager_Data *pd)
{
   return pd->dir;
}

EOLIAN static void
_efl_ui_pager_efl_ui_direction_direction_set(Eo *obj EINA_UNUSED,
                                             Efl_Ui_Pager_Data *pd,
                                             Efl_Ui_Dir dir)
{
   if (dir != EFL_UI_DIR_VERTICAL &&
       dir != EFL_UI_DIR_HORIZONTAL)
     return;

   pd->dir = dir;
}

EOLIAN Eina_Size2D
_efl_ui_pager_page_size_get(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd)
{
   return pd->page_spec.sz;
}

EOLIAN static void
_efl_ui_pager_page_size_set(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd,
                            Eina_Size2D sz)
{
   if (sz.w < 0 || sz.h < 0) return;

   pd->page_spec.sz = sz;

   efl_page_transition_page_size_set(pd->transition, sz);
}

EOLIAN static int
_efl_ui_pager_padding_get(Eo *obj EINA_UNUSED,
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

   efl_page_transition_padding_size_set(pd->transition, padding);
}

EOLIAN static void
_efl_ui_pager_scroll_block_get(Eo *obj EINA_UNUSED,
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

EOLIAN static void
_efl_ui_pager_loop_set(Eo *obj EINA_UNUSED,
                       Efl_Ui_Pager_Data *pd,
                       Efl_Ui_Pager_Loop loop)
{
   pd->loop = loop;

   efl_page_transition_loop_set(pd->transition, loop);
}

EOLIAN static Efl_Ui_Pager_Loop
_efl_ui_pager_loop_get(Eo *obj EINA_UNUSED,
                       Efl_Ui_Pager_Data *pd)
{
   return pd->loop;
}



#include "efl_ui_pager.eo.c"
