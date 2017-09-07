#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pagescroller.h"
#include "efl_page_transition.h"

#define MY_CLASS EFL_UI_PAGESCROLLER_CLASS



EOLIAN static void
_efl_ui_pagescroller_update(Eo *obj,
                            Efl_Ui_Pagescroller_Data *pd)
{
   //FIXME need or need not update when an item is appended

   if (pd->cnt == 0) return;

   Eina_List *l;
   Evas_Object *subobj, *o1 = NULL, *o2 = NULL;
   int p1 = -1, p2 = -1;

   // All the pages go to background - backclip is essential!
   // in order to hide pages regardless of their visibility
   // geometry isn't enough - think about cube effect
   EINA_LIST_FOREACH(pd->content_list, l, subobj)
     {
        efl_gfx_size_set(subobj, pd->w, pd->h);
        evas_object_clip_set(subobj, pd->hidden_clip);
     }

   // Only the current and the next page go into the viewport
   p1 = pd->page;
   o1 = eina_list_nth(pd->content_list, p1);
   evas_object_clip_set(o1, pd->viewport.foreclip);

   if (pd->ratio != 0)
     {
        if (pd->change.jump)
          p2 = pd->change.dst;
        else
          p2 = (p1 + 1 + pd->cnt) % pd->cnt;

        o2 = eina_list_nth(pd->content_list, p2);
        evas_object_clip_set(o2, pd->viewport.backclip);
     }

   // Transition effect
   //efl_page_transition_update(pd->transition, o1, (-1 * pd->ratio));
   //efl_page_transition_update(pd->transition, o2, pd->ratio);

   efl_event_callback_call(obj, EFL_UI_PAGESCROLLER_EVENT_UPDATED, NULL);
}

static void
efl_ui_pagescroller_update(Eo *obj)
{
   EFL_UI_PAGESCROLLER_DATA_GET(obj, sd);
   _efl_ui_pagescroller_update(obj, sd);
}

static void
_job(void *data)
{
   Evas_Object *obj = data;
   EFL_UI_PAGESCROLLER_DATA_GET(obj, sd);

   double t = 0;
   Eina_Bool prev = EINA_FALSE, next = EINA_FALSE;

   sd->job = NULL;

   // calculate t based on mouse position
   if (sd->orientation == EFL_ORIENT_HORIZONTAL)
     t = ((double)sd->mouse_down.x - (double)sd->mouse_x) / (double)sd->w;
   else
     t = ((double)sd->mouse_down.y - (double)sd->mouse_y) / (double)sd->h;

   if (t > 1.0) t = 1.0;
   else if (t < -1.0) t = -1.0;

   if (t < 0) prev = EINA_TRUE;
   else if (t > 0) next = EINA_TRUE;

   //if (sd->prev_block && t < 0) return;
   //else if (sd->next_block && t > 0) return;
   if (sd->prev_block && prev) return;
   else if (sd->next_block && next) return;

   t += sd->mouse_down.page + sd->mouse_down.ratio;
   if (t < 0.0) t += 3.0;
   else if (t >= 3.0) t -= 3.0;

   //if (!sd->loop && (t < 0.0 || t > (sd->cnt - 1))) return;
   ERR("sd->loop %d next %d page %d", sd->loop, next, (int)t);
   if (!sd->loop && ((next && sd->mouse_down.page == (sd->cnt - 1))
                     || (prev && sd->mouse_down.page == 0))) return;

   sd->page = t;
   sd->ratio = t - (int)t;

   efl_ui_pagescroller_update(obj);
}

static Eina_Bool
_animator(void *data, double pos)
{
   Evas_Object *obj = data;
   EFL_UI_PAGESCROLLER_DATA_GET(obj, sd);
   double p;

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   if (sd->ratio < 0.5)
     sd->ratio = sd->ratio * (1 - p);
   else
     sd->ratio = 1 - (1 - sd->ratio) * (1 - p);

   efl_ui_pagescroller_update(obj);

   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   if (sd->ratio == 1.0)
     {
        sd->page = (sd->page + 1) % sd->cnt;
        sd->ratio = 0.0;
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
   EFL_UI_PAGESCROLLER_DATA_GET(pc, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   ELM_SAFE_FREE(sd->animator, ecore_animator_del);

   sd->move_started = EINA_FALSE;
   sd->mouse_down.enabled = EINA_TRUE;

   sd->mouse_x = ev->canvas.x - sd->x;
   sd->mouse_y = ev->canvas.y - sd->y;

   sd->mouse_down.x = sd->mouse_x;
   sd->mouse_down.y = sd->mouse_y;
   sd->mouse_down.page = sd->page;
   sd->mouse_down.ratio = sd->ratio;
}

static void
_mouse_move_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *pc = data;
   EFL_UI_PAGESCROLLER_DATA_GET(pc, sd);

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
   EFL_UI_PAGESCROLLER_DATA_GET(pc, sd);

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!sd->mouse_down.enabled) return;

   sd->mouse_down.enabled = EINA_FALSE;

   ELM_SAFE_FREE(sd->job, ecore_job_del);

   double time;
   if (sd->ratio < 0.5) //target = first;
      time = sd->ratio;
   else //target = second;
      time = 1 - sd->ratio;

   if (time < 0.01) time = 0.01;
   else if (time > 0.99) time = 0.99;

   //animation
   ecore_animator_del(sd->animator);
   sd->animator = ecore_animator_timeline_add(time, _animator, pc);
}
static void
_event_handler_create(Eo *obj, Efl_Ui_Pagescroller_Data *sd)
{
   sd->event = evas_object_rectangle_add(evas_object_evas_get(obj));

   evas_object_color_set(sd->event, 0, 0, 0, 0);
   efl_gfx_position_set(sd->event, sd->x, sd->y);
   efl_gfx_size_set(sd->event, sd->w, sd->h);
   efl_gfx_visible_set(sd->event, EINA_TRUE);

   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_cb, obj);
   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up_cb, obj);
   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move_cb, obj);
}

EOLIAN static Eo *
_efl_ui_pagescroller_efl_object_constructor(Eo *obj,
                                            Efl_Ui_Pagescroller_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static void
_efl_ui_pagescroller_efl_canvas_group_group_add(Eo *obj,
                                                Efl_Ui_Pagescroller_Data *pd)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   pd->cnt = 0;
   pd->page = 0;
   pd->ratio = 0.0;
   pd->orientation = EFL_ORIENT_HORIZONTAL;

   pd->viewport.foreclip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(pd->viewport.foreclip, EINA_TRUE);

   pd->viewport.backclip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(pd->viewport.backclip, EINA_TRUE);

   pd->hidden_clip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(pd->hidden_clip, EINA_TRUE);

   _event_handler_create(obj, pd);
}

EOLIAN static void
_efl_ui_pagescroller_efl_gfx_size_set(Eo *obj,
                                      Efl_Ui_Pagescroller_Data *sd,
                                      Evas_Coord w,
                                      Evas_Coord h)
{
   if ((sd->w == w) && (sd->h == h)) return;

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);

   sd->w = w;
   sd->h = h;

   efl_gfx_size_set(sd->event, w, h);
   efl_gfx_size_set(sd->viewport.foreclip, w, h);
   efl_gfx_size_set(sd->viewport.backclip, w, h);

   efl_ui_pagescroller_update(obj);
}

EOLIAN static void
_efl_ui_pagescroller_efl_gfx_position_set(Eo *obj,
                                          Efl_Ui_Pagescroller_Data *sd,
                                          Evas_Coord x,
                                          Evas_Coord y)
{
   if ((sd->x == x) && (sd->y == y)) return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);

   sd->x = x;
   sd->y = y;

   efl_gfx_position_set(sd->event, x, y);
   efl_gfx_position_set(sd->viewport.foreclip, x, y);
   efl_gfx_position_set(sd->viewport.backclip, x, y);
}

EOLIAN static int
_efl_ui_pagescroller_efl_container_content_count(Eo *obj EINA_UNUSED,
                                                 Efl_Ui_Pagescroller_Data *pd)
{
   return eina_list_count(pd->content_list);
}

EOLIAN static Eina_Bool
_efl_ui_pagescroller_efl_pack_linear_pack_end(Eo *obj,
                                              Efl_Ui_Pagescroller_Data *pd,
                                              Efl_Gfx *subobj)
{
   efl_parent_set(subobj, obj);
   pd->cnt += 1;

   pd->content_list = eina_list_append(pd->content_list, subobj);
   efl_gfx_stack_raise(pd->event);

   efl_ui_pagescroller_update(obj);

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx *
_efl_ui_pagescroller_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED,
                                                      Efl_Ui_Pagescroller_Data *pd,
                                                      int index)
{
   return eina_list_nth(pd->content_list, index);
}

EOLIAN static int
_efl_ui_pagescroller_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                                    Efl_Ui_Pagescroller_Data *pd,
                                                    const Efl_Gfx *subobj)
{
   return eina_list_data_idx(pd->content_list, (void *)subobj);
}

static Eina_Bool
_change_animator(void *data, double pos)
{
   Eo *obj = data;
   EFL_UI_PAGESCROLLER_DATA_GET(obj, sd);
   double p, d;

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   d = sd->change.src + sd->change.delta * pos;

   sd->page = d;
   sd->ratio = d - (int)d;

   if (pos == 1.0)
     sd->page = sd->change.dst;

   efl_ui_pagescroller_update(obj);

   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   sd->change.jump = EINA_FALSE;
   sd->change.animator = NULL;

   return ECORE_CALLBACK_CANCEL;
}

EOLIAN static void
_efl_ui_pagescroller_current_page_set(Eo *obj,
                                      Efl_Ui_Pagescroller_Data *sd,
                                      int index,
                                      Eina_Bool animation,
                                      Eina_Bool jump)
{
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
}

EOLIAN static int
_efl_ui_pagescroller_current_page_get(Eo *obj,
                                      Efl_Ui_Pagescroller_Data *sd)
{
   return sd->page;
}

EOLIAN static Efl_Page_Transition *
_efl_ui_pagescroller_transition_get(Eo *obj EINA_UNUSED,
                                    Efl_Ui_Pagescroller_Data *sd)
{
   return sd->transition;
}

EOLIAN static void
_efl_ui_pagescroller_transition_set(Eo *obj EINA_UNUSED,
                                    Efl_Ui_Pagescroller_Data *sd,
                                    Efl_Page_Transition *transition)
{
   sd->transition = transition;
}

EOLIAN static Efl_Orient
_efl_ui_pagescroller_efl_orientation_orientation_get(Eo *obj,
                                                     Efl_Ui_Pagescroller_Data *sd)
{
   return sd->orientation;
}

EOLIAN static void
_efl_ui_pagescroller_efl_orientation_orientation_set(Eo *obj,
                                                     Efl_Ui_Pagescroller_Data *sd,
                                                     Efl_Orient dir)
{
   if (dir != EFL_ORIENT_VERTICAL &&
       dir != EFL_ORIENT_HORIZONTAL)
     return;

   sd->orientation = dir;
}

EOLIAN static void
_efl_ui_pagescroller_scroll_block_get(Eo *obj,
                                      Efl_Ui_Pagescroller_Data *sd,
                                      Eina_Bool *prev,
                                      Eina_Bool *next)
{
   *prev = sd->prev_block;
   *next = sd->next_block;
}

EOLIAN static void
_efl_ui_pagescroller_scroll_block_set(Eo *obj,
                                      Efl_Ui_Pagescroller_Data *sd,
                                      Eina_Bool prev,
                                      Eina_Bool next)
{
   sd->prev_block = prev;
   sd->next_block = next;
}

EOLIAN static void
_efl_ui_pagescroller_loop_set(Eo *obj,
                              Efl_Ui_Pagescroller_Data *sd,
                              Eina_Bool loop_enabled)
{
   sd->loop = loop_enabled;
}

EOLIAN static Eina_Bool
_efl_ui_pagescroller_loop_get(Eo *obj,
                              Efl_Ui_Pagescroller_Data *sd)
{
   return sd->loop;
}

#define EFL_UI_PAGESCROLLER_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(efl_ui_pagescroller)

#include "efl_ui_pagescroller.eo.c"
