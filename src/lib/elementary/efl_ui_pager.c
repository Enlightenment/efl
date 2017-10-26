#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pager.h"
#include "efl_page_transition.h"

#define MY_CLASS EFL_UI_PAGER_CLASS

#define CURRENT_PAGE_SET 0

#define INTERSECT(x1, w1, x2, w2) \
   (!(((x1) + (w1) <= (x2)) || ((x2) + (w2) <= (x1))))
#define RECT_INTERSECT(x1, y1, w1, h1, x2, y2, w2, h2) \
   ((INTERSECT(x1, w1, x2, w2)) && (INTERSECT(y1, h1, y2, h2)))


static void
_efl_ui_pager_update(Eo *obj)
{
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

   if (sd->move == 1.0 || sd->move == -1.0)
     efl_page_transition_finish(sd->transition);

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
     {
        efl_page_transition_finish(sd->transition);
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
   Eina_List *list;
   Page_Info *pi;
   int i, tmp;

   for (i = 0; i < pd->page_info_num; i++)
     {
        pi = (Page_Info *)malloc(sizeof(Page_Info));
        if (i == 0) pd->head = pi;
        else if (i == 4) pd->tail = pi;
        pi->id = i;
        pi->pos = i - (pd->side_page_num + 1);
        pi->content_num = -1;
        pi->content = NULL;
        pi->obj = efl_add(EFL_UI_BOX_CLASS, obj);
        pd->page_infos = eina_list_append(pd->page_infos, pi);
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

EOLIAN static void
_efl_ui_pager_efl_canvas_group_group_add(Eo *obj,
                                         Efl_Ui_Pager_Data *pd)
{
   pd->obj = obj;
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   pd->cnt = 0;
   pd->current_page = -1;
   pd->side_page_num = 1;
   pd->page_info_num = 5;
   pd->move = 0.0;
   pd->dir = EFL_UI_DIR_HORIZONTAL;

   pd->viewport.foreclip = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                                   evas_object_evas_get(obj));
   evas_object_static_clip_set(pd->viewport.foreclip, EINA_TRUE);

   pd->viewport.backclip = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                                   evas_object_evas_get(obj));
   evas_object_static_clip_set(pd->viewport.backclip, EINA_TRUE);
   efl_gfx_visible_set(pd->viewport.backclip, EINA_FALSE);

   _page_info_set(obj, pd);
   _event_handler_create(obj, pd);

   elm_widget_can_focus_set(obj, EINA_TRUE);
}

EOLIAN static void
_efl_ui_pager_efl_canvas_group_group_del(Eo *obj,
                                         Efl_Ui_Pager_Data *pd)
{
   Page_Info *pi;
   EINA_LIST_FREE(pd->page_infos, pi)
     {
        efl_del(pi->obj);
        free(pi);
     }

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

static void
_page_info_change(Eo *obj, int delta)
{
   EFL_UI_PAGER_DATA_GET(obj, pd);
   Page_Info *pi;
   int i;

   if (delta > 0)
     {
        for (i = 0; i < delta; i++)
          {
             pi = (Page_Info *)malloc(sizeof(Page_Info));
             pi->obj = efl_add(EFL_UI_BOX_CLASS, obj);
             pi->content_num = -1;
             pi->content = NULL;
             pd->page_infos = eina_list_prepend_relative(pd->page_infos, pi, pd->head);

             pi->next = pd->head;
             pd->head->prev = pi;
             pi->prev = pd->tail;
             pd->tail->next = pi;
             pd->head = pi;

             pi = (Page_Info *)malloc(sizeof(Page_Info));
             pi->obj = efl_add(EFL_UI_BOX_CLASS, obj);
             pi->content_num = -1;
             pi->content = NULL;
             pd->page_infos = eina_list_append_relative(pd->page_infos, pi, pd->tail);

             pi->next = pd->head;
             pd->head->prev = pi;
             pi->prev = pd->tail;
             pd->tail->next = pi;
             pd->tail = pi;
          }

        efl_gfx_stack_raise(pd->event); //FIXME
     }
   else
     {
        for (i = 0; i > delta; i--)
          {
             pi = pd->head;
             pd->head = pi->next;

             efl_canvas_object_clip_set(pi->content, pd->viewport.backclip);
             efl_pack_unpack(pi->obj, pi->content);
             efl_del(pi->obj);
             pi->prev->next = pi->next;
             pi->next->prev = pi->prev;
             pd->page_infos = eina_list_remove(pd->page_infos, pi);
             free(pi);

             pi = pd->tail;
             pd->tail = pi->prev;

             efl_canvas_object_clip_set(pi->content, pd->viewport.backclip);
             efl_pack_unpack(pi->obj, pi->content);
             efl_del(pi->obj);
             pi->prev->next = pi->next;
             pi->next->prev = pi->prev;
             pd->page_infos = eina_list_remove(pd->page_infos, pi);
             free(pi);
          }
     }

   pi = pd->head;
   for (i = 0; i < pd->page_info_num; i++)
     {
        pi->id = i;
        pi->pos = i - (pd->side_page_num + 1);
        pi = pi->next;
     }
}

static void
_page_info_job(void *data)
{
   Eo *obj = data;
   EFL_UI_PAGER_DATA_GET(obj, pd);

   pd->page_info_job = NULL;

   efl_page_transition_init(pd->transition);
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

   if (sd->page_info_job) ecore_job_del(sd->page_info_job);
   sd->page_info_job = ecore_job_add(_page_info_job, obj);
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

   if (sd->page_info_job) ecore_job_del(sd->page_info_job);
   sd->page_info_job = ecore_job_add(_page_info_job, obj);
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
   int id, pi_id, len, i;

   efl_parent_set(subobj, obj);

   pd->content_list = eina_list_append(pd->content_list, subobj);
   efl_gfx_stack_raise(pd->event); //FIXME
   id = pd->cnt;

   if (pd->cnt == 0) pd->current_page = 0;
   pd->cnt += 1;

   //FIXME 1.fix logic: current code works only when the current page num is 0
   //FIXME 2.use ecore job: don't need to unpack and pack everytime a page is packed
   len = (pd->side_page_num * 2) + 1;
   if (pd->cnt > len)
     {
        for (i = 1; i <= pd->side_page_num; i++)
          {
             pi = eina_list_nth(pd->page_infos, i);
             if (i == 1)
               {
                  efl_pack_unpack(pi->obj, pi->content);
                  efl_canvas_object_clip_set(pi->content, pd->viewport.backclip);
               }
             else
               {
                  pi->prev->content = pi->content;
                  pi->prev->content_num = pi->content_num;
                  efl_pack_unpack(pi->obj, pi->content);
                  efl_pack(pi->prev->obj, pi->content);
               }
             if (i == pd->side_page_num)
               {
                  pi->content = subobj;
                  pi->content_num = id;
                  efl_pack(pi->obj, pi->content);
                  efl_canvas_object_clip_set(pi->content, pd->viewport.foreclip);
               }
          }
     }
   else
     {
        pi_id = (id + pd->side_page_num) % len + 1;
        pi = eina_list_nth(pd->page_infos, pi_id);

        pi->content = subobj;
        pi->content_num = id;
        efl_pack(pi->obj, pi->content);
        efl_canvas_object_clip_set(pi->content, pd->viewport.foreclip);
     }

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

#if CURRENT_PAGE_SET
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

   _efl_ui_pager_update(obj);

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
#if CURRENT_PAGE_SET
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

EOLIAN static void
_efl_ui_pager_page_size_get(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *sd,
                            int *width,
                            int *height)
{
   if (width) *width = sd->page_spec.w;
   if (height) *height = sd->page_spec.h;
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
   sd->page_info_job = ecore_job_add(_page_info_job, obj);
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
   sd->page_info_job = ecore_job_add(_page_info_job, obj);
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

EOLIAN static int
_efl_ui_pager_side_page_num_get(Eo *obj EINA_UNUSED,
                                Efl_Ui_Pager_Data *sd)
{
   return sd->side_page_num;
}

EOLIAN static void
_efl_ui_pager_side_page_num_set(Eo *obj,
                                Efl_Ui_Pager_Data *sd,
                                int side_page_num)
{
   if (sd->side_page_num == side_page_num) return;

   int delta = side_page_num - sd->side_page_num;
   sd->side_page_num = side_page_num;
   sd->page_info_num = (side_page_num * 2) + 3;

   _page_info_change(obj, delta);

   if (sd->page_info_job) ecore_job_del(sd->page_info_job);
   sd->page_info_job = ecore_job_add(_page_info_job, obj);
}



#define EFL_UI_PAGER_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_pager)

#include "efl_ui_pager.eo.c"
