#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pager.h"
#include "efl_page_transition.h"

#define MY_CLASS EFL_UI_PAGER_CLASS


#define INTERSECT(x1, w1, x2, w2) \
   (!(((x1) + (w1) <= (x2)) || ((x2) + (w2) <= (x1))))
#define RECT_INTERSECT(x1, y1, w1, h1, x2, y2, w2, h2) \
   ((INTERSECT(x1, w1, x2, w2)) && (INTERSECT(y1, h1, y2, h2)))

#define DEBUG 0

static void
_efl_ui_pager_update(Eo *obj)
{
   //FIXME need or need not update when an item is appended
   EFL_UI_PAGER_DATA_GET(obj, pd);

   if (pd->cnt == 0) return;

   efl_page_transition_update(pd->transition);

   efl_event_callback_call(obj, EFL_UI_PAGER_EVENT_UPDATED, NULL);
}

static void
_job(void *data)
{
   Evas_Object *obj = data;
   EFL_UI_PAGER_DATA_GET(obj, sd);

   double t = 0;
   Eina_Bool prev = EINA_FALSE, next = EINA_FALSE;

   sd->job = NULL;

   if (sd->orientation == EFL_ORIENT_HORIZONTAL)
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

#if 0
   //FIXME loop is not handled yet
   if (!sd->loop && ((next && sd->mouse_down.page == (sd->cnt - 1))
                     || (prev && sd->mouse_down.page == 0))) return;
#endif

   _efl_ui_pager_update(obj);
}

static Eina_Bool
_animator(void *data, double pos)
{
   Evas_Object *obj = data;
   Eina_List *list;
   Page_Info *pi;
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

   _efl_ui_pager_update(obj);

   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

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

   ELM_SAFE_FREE(sd->animator, ecore_animator_del);

   sd->move_started = EINA_FALSE;
   sd->mouse_down.enabled = EINA_TRUE;

   sd->mouse_x = ev->canvas.x - sd->x;
   sd->mouse_y = ev->canvas.y - sd->y;

   sd->mouse_down.x = sd->mouse_x;
   sd->mouse_down.y = sd->mouse_y;
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

   if (sd->move == 1.0 || sd->move == -1.0)
     return;

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

EOLIAN static Eo *
_efl_ui_pager_efl_object_constructor(Eo *obj,
                                     Efl_Ui_Pager_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

static void
_page_info_set(Eo *obj, Efl_Ui_Pager_Data *pd)
{
   Page_Info *page_info;
   Eina_List *list;
   Page_Info *pi;
   int i, tmp;

#if DEBUG
   int rgba[5][4] = {
      {255, 0, 0, 255},
      {0, 255, 0, 255},
      {0, 0, 255, 255},
      {255, 255, 0, 255},
      {255, 0, 255, 255}
   };
#endif
   for (i = 0; i < pd->page_info_num; i++)
     {
        page_info = (Page_Info *)malloc(sizeof(Page_Info));
        page_info->id = i;
        page_info->pos = i - 2;

#if DEBUG
        page_info->obj = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                                efl_provider_find(obj, EVAS_CANVAS_CLASS));
        efl_gfx_visible_set(page_info->obj, EINA_TRUE);

        efl_gfx_color_set(page_info->obj,
                          rgba[i][0],
                          rgba[i][1],
                          rgba[i][2],
                          rgba[i][3]);
#else
        page_info->obj = efl_add(EFL_UI_BOX_CLASS, obj);
        efl_gfx_visible_set(page_info->obj, EINA_TRUE);
#endif

        pd->page_infos = eina_list_append(pd->page_infos, page_info);

        if (i == 0 || i == 4)
          efl_canvas_object_clip_set(page_info->obj, pd->viewport.backclip);
        else
          efl_canvas_object_clip_set(page_info->obj, pd->viewport.foreclip);
     }

   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
        tmp = (pi->id + 1) % pd->page_info_num;
        pi->next = eina_list_nth(pd->page_infos, tmp);

        tmp = (pi->id - 1 + pd->page_info_num) % pd->page_info_num;
        pi->prev = eina_list_nth(pd->page_infos, tmp);
     }
}

static void
_event_handler_create(Eo *obj, Efl_Ui_Pager_Data *sd)
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

EOLIAN static void
_efl_ui_pager_efl_canvas_group_group_add(Eo *obj,
                                         Efl_Ui_Pager_Data *pd)
{
   pd->obj = obj;
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   pd->cnt = 0;
   pd->page_info_num = 5; //TEMP
   pd->move = 0.0;
   pd->orientation = EFL_ORIENT_HORIZONTAL;

   pd->viewport.foreclip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(pd->viewport.foreclip, EINA_TRUE);
   efl_gfx_visible_set(pd->viewport.foreclip, EINA_TRUE);

   pd->viewport.backclip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(pd->viewport.backclip, EINA_TRUE);

   pd->hidden_clip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(pd->hidden_clip, EINA_TRUE);

   _page_info_set(obj, pd);
   _event_handler_create(obj, pd);
}

static void
_page_info_job(void *data)
{
   //calculate the number of pages
   Efl_Ui_Pager_Data *pd = data;

   pd->page_info_job = NULL;

   efl_page_transition_init(pd->transition);

   _efl_ui_pager_update(pd->obj);
}

EOLIAN static void
_efl_ui_pager_efl_gfx_size_set(Eo *obj,
                               Efl_Ui_Pager_Data *sd,
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

   if (sd->page_info_job) ecore_job_del(sd->page_info_job);
   sd->page_info_job = ecore_job_add(_page_info_job, sd);
}

EOLIAN static void
_efl_ui_pager_efl_gfx_position_set(Eo *obj,
                                   Efl_Ui_Pager_Data *sd,
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

   if (sd->page_info_job) ecore_job_del(sd->page_info_job);
   sd->page_info_job = ecore_job_add(_page_info_job, sd);
}

EOLIAN static int
_efl_ui_pager_efl_container_content_count(Eo *obj EINA_UNUSED,
                                          Efl_Ui_Pager_Data *pd)
{
   return eina_list_count(pd->content_list);
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_end(Eo *obj,
                                       Efl_Ui_Pager_Data *pd,
                                       Efl_Gfx *subobj)
{
   Page_Info *pi;

   efl_parent_set(subobj, obj);

   pd->content_list = eina_list_append(pd->content_list, subobj);
   efl_gfx_stack_raise(pd->event);

   pi = eina_list_nth(pd->page_infos, pd->cnt);
   if (pi)
     {
        efl_pack(pi->obj, subobj);
        evas_object_size_hint_weight_set(subobj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(subobj, EVAS_HINT_FILL, EVAS_HINT_FILL);
     }
   pd->cnt += 1;
   _efl_ui_pager_update(obj);

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
   EFL_UI_PAGER_DATA_GET(obj, sd);
   double p, d;

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   d = sd->change.src + sd->change.delta * pos;
#if 0
   sd->page = d;
   sd->ratio = d - (int)d;

   if (pos == 1.0)
     sd->page = sd->change.dst;
#endif
   _efl_ui_pager_update(obj);

   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   sd->change.jump = EINA_FALSE;
   sd->change.animator = NULL;

   return ECORE_CALLBACK_CANCEL;
}

EOLIAN static void
_efl_ui_pager_current_page_set(Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *sd EINA_UNUSED,
                               int index EINA_UNUSED)
{
#if 0
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
   return sd->page;
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

EOLIAN static Efl_Orient
_efl_ui_pager_efl_orientation_orientation_get(Eo *obj EINA_UNUSED,
                                              Efl_Ui_Pager_Data *sd)
{
   return sd->orientation;
}

EOLIAN static void
_efl_ui_pager_efl_orientation_orientation_set(Eo *obj EINA_UNUSED,
                                              Efl_Ui_Pager_Data *sd,
                                              Efl_Orient dir)
{
   if (dir != EFL_ORIENT_VERTICAL &&
       dir != EFL_ORIENT_HORIZONTAL)
     return;

   sd->orientation = dir;
}

EOLIAN static void
_efl_ui_pager_page_size_get(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *sd,
                            int *width,
                            int *height)
{
   *width = sd->page_spec.w;
   *height = sd->page_spec.h;
}

EOLIAN static void
_efl_ui_pager_page_size_set(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *sd,
                            int width,
                            int height)
{
   sd->page_spec.w = width;
   sd->page_spec.h = height;

   if (sd->page_info_job) ecore_job_del(sd->page_info_job);
   sd->page_info_job = ecore_job_add(_page_info_job, sd);
}

EOLIAN static int
_efl_ui_pager_padding_get(Eo *obj EINA_UNUSED,
                          Efl_Ui_Pager_Data *sd)
{
   return sd->page_spec.padding;
}

EOLIAN static void
_efl_ui_pager_padding_set(Eo *obj EINA_UNUSED,
                          Efl_Ui_Pager_Data *sd,
                          int padding)
{
   sd->page_spec.padding = padding;

   if (sd->page_info_job) ecore_job_del(sd->page_info_job);
   sd->page_info_job = ecore_job_add(_page_info_job, sd);
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
                       Efl_Ui_Pager_Data *sd,
                       Eina_Bool loop_enabled)
{
   sd->loop = loop_enabled;
}

EOLIAN static Eina_Bool
_efl_ui_pager_loop_get(Eo *obj EINA_UNUSED,
                       Efl_Ui_Pager_Data *sd)
{
   return sd->loop;
}

#define EFL_UI_PAGER_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(efl_ui_pager)

#include "efl_ui_pager.eo.c"
