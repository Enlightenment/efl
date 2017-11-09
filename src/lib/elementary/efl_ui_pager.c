#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pager.h"
#include "efl_page_transition.h"

#define MY_CLASS EFL_UI_PAGER_CLASS

#define CURRENT_PAGE_SET_ENABLED 0

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
   EFL_UI_PAGER_DATA_GET(obj, sd);

   double t = 0;
   Eina_Bool prev = EINA_FALSE, next = EINA_FALSE;

   sd->job = NULL;

   if (sd->dir == EFL_UI_DIR_HORIZONTAL)
     t = ((double)sd->mouse_down.x - (double)sd->mouse_x) / (double)sd->w;
   else
     t = ((double)sd->mouse_down.y - (double)sd->mouse_y) / (double)sd->h;

   if (t > 1.0) t = 1.0;
   else if (t < -1.0) t = -1.0;

   if (sd->move == t) return;

   sd->move = t;

   if (t < 0) prev = EINA_TRUE;
   else if (t > 0) next = EINA_TRUE;

   if (sd->prev_block && prev) return;
   else if (sd->next_block && next) return;

   if (!sd->loop && ((next && sd->mouse_down.page == (sd->cnt - 1))
                     || (prev && sd->mouse_down.page == 0))) return;

   _efl_ui_pager_update(sd);
}

static Eina_Bool
_animator(void *data, double pos)
{
   Evas_Object *obj = data;
   double p;

   EFL_UI_PAGER_DATA_GET(obj, sd);

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   if (sd->move < 0.0)
     {
        if (sd->move > -0.5)
          sd->move = sd->move * (1 - p);
        else
          sd->move = (-1) - (-1 - sd->move) * (1 - p);
     }
   else
     {
        if (sd->move < 0.5)
          sd->move = sd->move * (1 - p);
        else
          sd->move = 1 - (1 - sd->move) * (1 - p);
     }

   _efl_ui_pager_update(sd);

   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   if (sd->move == 1.0 || sd->move == -1.0)
     {
        efl_page_transition_curr_page_change(sd->transition, sd->move);
        sd->current_page = (sd->current_page + (int) sd->move + sd->cnt) % sd->cnt;
        //TODO Call "page changed" callback
        sd->move = 0.0;
     }

   sd->animator = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Object *pc = data;
   EFL_UI_PAGER_DATA_GET(pc, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   if (sd->move != 0.0) return; //FIXME

   ELM_SAFE_FREE(sd->animator, ecore_animator_del);

   sd->move_started = EINA_FALSE;
   sd->mouse_down.enabled = EINA_TRUE;

   sd->mouse_x = ev->canvas.x - sd->x;
   sd->mouse_y = ev->canvas.y - sd->y;

   sd->mouse_down.x = sd->mouse_x;
   sd->mouse_down.y = sd->mouse_y;

   sd->mouse_down.page = sd->current_page;
}

static void
_mouse_move_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *pc = data;
   EFL_UI_PAGER_DATA_GET(pc, sd);

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!sd->mouse_down.enabled) return;

   sd->mouse_x = ev->cur.canvas.x - sd->x;
   sd->mouse_y = ev->cur.canvas.y - sd->y;

   if (!sd->move_started)
     {
        Evas_Coord dx, dy;
        dx = sd->mouse_x - sd->mouse_down.x;
        dy = sd->mouse_y - sd->mouse_down.y;

        if (((dx * dx) + (dy * dy)) <=
            (_elm_config->finger_size * _elm_config->finger_size / 4))
          return;

        sd->move_started = EINA_TRUE;
     }

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   ecore_job_del(sd->job);
   sd->job = ecore_job_add(_job, pc);
}

static void
_mouse_up_cb(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Object *pc = data;
   EFL_UI_PAGER_DATA_GET(pc, sd);
   double time;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!sd->mouse_down.enabled) return;

   sd->mouse_down.enabled = EINA_FALSE;

   ELM_SAFE_FREE(sd->job, ecore_job_del);

   if (!sd->loop && (((sd->move > 0) && (sd->mouse_down.page == (sd->cnt - 1)))
                     || ((sd->move < 0) && (sd->mouse_down.page == 0))))
     {
        sd->move = 0.0;
        return;
     }

   if (sd->move == 0.0) return;
   if (sd->move == 1.0 || sd->move == -1.0)
     {
        efl_page_transition_curr_page_change(sd->transition, sd->move);
        sd->current_page = (sd->current_page + (int) sd->move + sd->cnt) % sd->cnt;
        //TODO Call "page changed" callback
        sd->move = 0.0;
        return;
     }

   if (sd->move < 0.0)
     {
        if (sd->move > -0.5) time = (-1) * sd->move;
        else time = 1 + sd->move;
     }
   else
     {
        if (sd->move < 0.5) time = sd->move;
        else time = 1 - sd->move;
     }

   if (time < 0.01) time = 0.01;
   else if (time > 0.99) time = 0.99;

   ecore_animator_del(sd->animator);
   sd->animator = ecore_animator_timeline_add(time, _animator, pc);
}

//FIXME sub_object_parent_add? destruction
static void
_event_handler_create(Eo *obj, Efl_Ui_Pager_Data *sd)
{
   sd->event = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                       evas_object_evas_get(obj));
   evas_object_color_set(sd->event, 0, 0, 0, 0);

   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_cb, obj);
   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up_cb, obj);
   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_MOVE,
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
   pd->loop = EINA_FALSE;

   _event_handler_create(obj, pd);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   return obj;
}

EOLIAN static void
_efl_ui_pager_efl_gfx_size_set(Eo *obj,
                               Efl_Ui_Pager_Data *sd,
                               Eina_Size2D sz)
{
   if ((sd->w == sz.w) && (sd->h == sz.h)) return;

   efl_gfx_size_set(efl_super(obj, MY_CLASS), sz);

   sd->w = sz.w;
   sd->h = sz.h;

   efl_gfx_size_set(sd->event, sz);
}

EOLIAN static void
_efl_ui_pager_efl_gfx_position_set(Eo *obj,
                                   Efl_Ui_Pager_Data *sd,
                                   Eina_Position2D pos)
{
   if ((sd->x == pos.x) && (sd->y == pos.y)) return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), pos);

   sd->x = pos.x;
   sd->y = pos.y;

   efl_gfx_position_set(sd->event, pos);
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

#if CURRENT_PAGE_SET_ENABLED
static Eina_Bool
_change_animator(void *data, double pos)
{
   Eo *obj = data;
   EFL_UI_PAGER_DATA_GET(obj, sd);
   double p, d;

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   d = sd->change.src + sd->change.delta * pos;

   sd->page = d;
   sd->ratio = d - (int)d;

   if (pos == 1.0)
     sd->page = sd->change.dst;

   _efl_ui_pager_update(sd);

   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   sd->change.jump = EINA_FALSE;
   sd->change.animator = NULL;

   return ECORE_CALLBACK_CANCEL;
}
#endif

EOLIAN static void
_efl_ui_pager_current_page_set(Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *sd EINA_UNUSED,
                               int index EINA_UNUSED)
{
#if CURRENT_PAGE_SET_ENABLED
   double time;

   sd->change.src = sd->page + sd->ratio;
   sd->change.dst = index;

   sd->change.jump = jump;
   if (sd->ratio != 0) sd->change.jump = EINA_FALSE;

   sd->change.delta = index - sd->change.src;
   if (sd->change.jump) sd->change.delta = 1.0;
   if (sd->change.delta == 0) return;

   time = sd->change.delta;
   if (sd->change.delta < 0) time *= (-1);
   time /= sd->cnt;

   ecore_animator_del(sd->change.animator);
   sd->change.animator = ecore_animator_timeline_add(time, _change_animator, obj);
#endif
}

EOLIAN static int
_efl_ui_pager_current_page_get(Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *sd)
{
   return sd->current_page;
}

EOLIAN static Efl_Page_Transition *
_efl_ui_pager_transition_get(Eo *obj EINA_UNUSED,
                             Efl_Ui_Pager_Data *sd)
{
   return sd->transition;
}

EOLIAN static void
_efl_ui_pager_transition_set(Eo *obj EINA_UNUSED,
                             Efl_Ui_Pager_Data *sd,
                             Efl_Page_Transition *transition)
{
   efl_page_transition_bind(transition, obj);
   sd->transition = transition;
}

EOLIAN static Efl_Ui_Dir
_efl_ui_pager_efl_ui_direction_direction_get(Eo *obj EINA_UNUSED,
                                             Efl_Ui_Pager_Data *sd)
{
   return sd->dir;
}

EOLIAN static void
_efl_ui_pager_efl_ui_direction_direction_set(Eo *obj EINA_UNUSED,
                                             Efl_Ui_Pager_Data *sd,
                                             Efl_Ui_Dir dir)
{
   if (dir != EFL_UI_DIR_VERTICAL &&
       dir != EFL_UI_DIR_HORIZONTAL)
     return;

   sd->dir = dir;
}

EOLIAN Eina_Size2D
_efl_ui_pager_page_size_get(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *sd)
{
   Eina_Size2D sz;
   sz.w = sd->page_spec.w;
   sz.h = sd->page_spec.h;

   return sz;
}

EOLIAN static void
_efl_ui_pager_page_size_set(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd,
                            Eina_Size2D sz)
{
   pd->page_spec.w = sz.w;
   pd->page_spec.h = sz.h;

   efl_page_transition_page_size_set(pd->transition, sz.w, sz.h);
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
                               Efl_Ui_Pager_Data *sd,
                               Eina_Bool *prev,
                               Eina_Bool *next)
{
   *prev = sd->prev_block;
   *next = sd->next_block;
}

EOLIAN static void
_efl_ui_pager_scroll_block_set(Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *sd,
                               Eina_Bool prev,
                               Eina_Bool next)
{
   sd->prev_block = prev;
   sd->next_block = next;
}

EOLIAN static void
_efl_ui_pager_loop_set(Eo *obj EINA_UNUSED,
                       Efl_Ui_Pager_Data *pd,
                       Eina_Bool loop)
{
   pd->loop = loop;

   efl_page_transition_loop_set(pd->transition, loop);
}

EOLIAN static Eina_Bool
_efl_ui_pager_loop_get(Eo *obj EINA_UNUSED,
                       Efl_Ui_Pager_Data *sd)
{
   return sd->loop;
}



#include "efl_ui_pager.eo.c"
