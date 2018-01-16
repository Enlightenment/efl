#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_page_transition_scroll.h"
#include "efl_page_transition.h"
#include "efl_ui_widget_pager.h"

#define MY_CLASS EFL_PAGE_TRANSITION_SCROLL_CLASS

#define DEBUG 0



static void
_page_info_allocate(Efl_Page_Transition_Scroll_Data *pd,
                    Efl_Page_Transition_Data *spd)
{
   Eina_List *list;
   Page_Info *pi;
   int i, tmp;

   for (i = 0; i < pd->page_info_num; i++)
     {
        pi = calloc(1, sizeof(*pi));
        if (i == 0) pd->head = pi;
        else if (i == (pd->page_info_num - 1)) pd->tail = pi;
        pi->id = i;
        pi->pos = i - (pd->side_page_num + 1);
        pi->content_num = -1;
        pi->content = NULL;

        pi->geometry = eina_rectangle_new(0, 0, 0, 0);
        pi->temp = eina_rectangle_new(0, 0, 0, 0);

        pi->obj = efl_add(EFL_UI_BOX_CLASS, spd->pager.obj);
        efl_canvas_group_member_add(spd->pager.group, pi->obj);
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
_content_show(Efl_Page_Transition_Scroll_Data *pd,
              Efl_Page_Transition_Data *spd)
{
   Eina_List *list;
   Page_Info *pi;
   int tmp_id;
   Eo *tmp;
   int curr_page, cnt;

   curr_page = efl_ui_pager_current_page_get(spd->pager.obj);
   cnt = efl_content_count(spd->pager.obj);

   //FIXME make logic efficient: don't have to empty all and fill all the time
   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
        if (pi->content)
          {
             efl_pack_unpack(pi->obj, pi->content);
             efl_canvas_object_clip_set(pi->content, pd->backclip);

             pi->content_num = -1;
             pi->content = NULL;
             pi->visible = EINA_FALSE;
          }
        efl_canvas_object_clip_set(pi->obj, pd->backclip);
     }

   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
        if (pi->vis_page)
          {
             tmp_id = (curr_page + pi->pos + cnt) % cnt;

             if ((spd->loop == EFL_UI_PAGER_LOOP_DISABLED)
                 && ((pi->pos) * (tmp_id - curr_page) < 0)) continue;

             tmp = efl_pack_content_get(spd->pager.obj, tmp_id);
             if (tmp)
               {
                  efl_canvas_object_clip_set(pi->obj, pd->foreclip);

                  efl_pack(pi->obj, tmp);
                  efl_canvas_object_clip_set(tmp, pd->foreclip);

                  pi->content_num = tmp_id;
                  pi->content = tmp;
                  pi->visible = EINA_TRUE;
               }
          }
     }
}

//FIXME use ecore_job
static void
_page_info_geometry_change(Efl_Page_Transition_Scroll_Data *pd,
                           Efl_Page_Transition_Data *spd)
{
   Eina_List *list;
   Page_Info *pi;
   int content_w;

   content_w = spd->page_spec.sz.w
      + ((spd->page_spec.sz.w + spd->page_spec.padding) * pd->side_page_num * 2);

   if (content_w < spd->pager.w)
     {
        pd->viewport->x = spd->pager.x + ((spd->pager.w - content_w) / 2);
        pd->viewport->y = spd->pager.y;
        pd->viewport->w = content_w;
        pd->viewport->h = spd->pager.h;
     }
   else
     {
        pd->viewport->x = spd->pager.x;
        pd->viewport->y = spd->pager.y;
        pd->viewport->w = spd->pager.w;
        pd->viewport->h = spd->pager.h;
     }

   efl_gfx_geometry_set(pd->foreclip,
                        EINA_RECT(pd->viewport->x, pd->viewport->y,
                                  pd->viewport->w, pd->viewport->h));

   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
        pi->geometry->x = spd->pager.x + (spd->pager.w / 2)
           + pi->pos * (spd->page_spec.sz.w + spd->page_spec.padding)
           - (spd->page_spec.sz.w / 2);
        pi->geometry->y = spd->pager.y + (spd->pager.h / 2) - (spd->page_spec.sz.h / 2);
        pi->geometry->w = spd->page_spec.sz.w;
        pi->geometry->h = spd->page_spec.sz.h;

        if (eina_rectangles_intersect(pi->geometry, pd->viewport) &&
            ((pi->id != 0) && (pi->id != (pd->page_info_num - 1))))
          {
             pi->vis_page = EINA_TRUE;
             pi->visible = EINA_TRUE;
             efl_canvas_object_clip_set(pi->obj, pd->foreclip);
          }
        else
          {
             pi->vis_page = EINA_FALSE;
             pi->visible = EINA_FALSE;
             efl_canvas_object_clip_set(pi->obj, pd->backclip);
          }
#if DEBUG
        if (pi->vis_page)
          ERR("content %d: %d %d %d %d",
              pi->content_num,
              pi->geometry->x, pi->geometry->y,
              pi->geometry->w, pi->geometry->h);
#endif
        efl_gfx_geometry_set(pi->obj,
                             EINA_RECT(pi->geometry->x, pi->geometry->y,
                                       pi->geometry->w, pi->geometry->h));
     }

   if (efl_content_count(spd->pager.obj) > 0) _content_show(pd, spd);
}

static void
_resize_cb(void *data, const Efl_Event *ev)
{
   Efl_Page_Transition_Scroll *obj = data;
   EFL_PAGE_TRANSITION_SCROLL_DATA_GET(obj, pd);
   EFL_PAGE_TRANSITION_DATA_GET(obj, spd);

   Eina_Size2D sz;

   Efl_Ui_Pager *pager = ev->object;

   sz = efl_gfx_size_get(pager);

   spd->pager.w = sz.w;
   spd->pager.h = sz.h;

   _page_info_geometry_change(pd, spd);
}

static void
_move_cb(void *data, const Efl_Event *ev)
{
   Efl_Page_Transition_Scroll *obj = data;
   EFL_PAGE_TRANSITION_SCROLL_DATA_GET(obj, pd);
   EFL_PAGE_TRANSITION_DATA_GET(obj, spd);

   Eina_Position2D pos;

   Efl_Ui_Pager *pager = ev->object;

   pos = efl_gfx_position_get(pager);

   spd->pager.x = pos.x;
   spd->pager.y = pos.y;

   _page_info_geometry_change(pd, spd);
}

EOLIAN static void
_efl_page_transition_scroll_efl_page_transition_bind(Eo *obj,
                                                     Efl_Page_Transition_Scroll_Data *pd,
                                                     Eo *pager,
                                                     Efl_Canvas_Group *group)
{
   EFL_PAGE_TRANSITION_DATA_GET(obj, spd);

   efl_page_transition_bind(efl_super(obj, MY_CLASS), pager, group);

   efl_event_callback_add(spd->pager.obj, EFL_GFX_EVENT_RESIZE, _resize_cb, obj);
   efl_event_callback_add(spd->pager.obj, EFL_GFX_EVENT_MOVE, _move_cb, obj);

   pd->foreclip = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                          evas_object_evas_get(spd->pager.obj));
   evas_object_static_clip_set(pd->foreclip, EINA_TRUE);

   pd->backclip = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                          evas_object_evas_get(spd->pager.obj));
   evas_object_static_clip_set(pd->backclip, EINA_TRUE);
   efl_gfx_visible_set(pd->backclip, EINA_FALSE);

   _page_info_allocate(pd, spd);
}

EOLIAN static void
_efl_page_transition_scroll_page_size_set(Eo *obj,
                                          Efl_Page_Transition_Scroll_Data *pd,
                                          Eina_Size2D sz)
{
   EFL_PAGE_TRANSITION_DATA_GET(obj, spd);

   efl_page_transition_page_size_set(efl_super(obj, MY_CLASS), sz);
   _page_info_geometry_change(pd, spd);
}

EOLIAN static void
_efl_page_transition_scroll_padding_size_set(Eo *obj,
                                             Efl_Page_Transition_Scroll_Data *pd,
                                             int padding)
{
   EFL_PAGE_TRANSITION_DATA_GET(obj, spd);

   efl_page_transition_padding_size_set(efl_super(obj, MY_CLASS), padding);
   _page_info_geometry_change(pd, spd);
}

EOLIAN static void
_efl_page_transition_scroll_update(Eo *obj,
                                   Efl_Page_Transition_Scroll_Data *pd,
                                   double pos)
{
   EFL_PAGE_TRANSITION_DATA_GET(obj, spd);

   double t;
   int tmp_id, curr_page, cnt;
   Eo *tmp;
   Eina_List *list;
   Page_Info *pi, *tpi;

   t = pos;
   if (t < 0) t *= (-1);

   curr_page = efl_ui_pager_current_page_get(spd->pager.obj);
   cnt = efl_content_count(spd->pager.obj);

   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
        if (pos < 0)
          tpi = pi->next;
        else
          tpi = pi->prev;

        pi->temp->x = tpi->geometry->x * t + pi->geometry->x * (1 - t);
        pi->temp->y = tpi->geometry->y;
        pi->temp->w = tpi->geometry->w;
        pi->temp->h = tpi->geometry->h;

        efl_gfx_geometry_set(pi->obj,
                             EINA_RECT(pi->temp->x, pi->temp->y,
                                       pi->temp->w, pi->temp->h));

        if (!pi->vis_page && !tpi->vis_page) continue;

        if (!eina_rectangles_intersect(pi->temp, pd->viewport))
          {
             if (pi->content)
               {
                  efl_canvas_object_clip_set(pi->obj, pd->backclip);

                  efl_pack_unpack(pi->obj, pi->content);
                  efl_canvas_object_clip_set(pi->content, pd->backclip);

                  pi->content_num = -1;
                  pi->content = NULL;
                  pi->visible = EINA_FALSE;

               }
          }
        else
          {
             tmp_id = (curr_page + pi->pos + cnt) % cnt;
             if (pi->content)
               {
                  efl_pack_unpack(pi->obj, pi->content);
                  efl_canvas_object_clip_set(pi->content, pd->backclip);
               }

             if ((spd->loop == EFL_UI_PAGER_LOOP_DISABLED)
                 && ((pi->pos) * (tmp_id - curr_page) < 0)) continue;

             tmp = efl_pack_content_get(spd->pager.obj, tmp_id);
             if (tmp)
               {
                  efl_canvas_object_clip_set(pi->obj, pd->foreclip);

                  efl_pack(pi->obj, tmp);
                  efl_canvas_object_clip_set(tmp, pd->foreclip);

                  pi->content_num = tmp_id;
                  pi->content = tmp;
                  pi->visible = EINA_TRUE;
               }
          }
     }

#if DEBUG
   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
        ERR("[id: %d] vis_page %d visible %d content %d",
            pi->id, pi->vis_page, pi->visible, pi->content_num);
     }
   printf("\n");
#endif
}

EOLIAN static void
_efl_page_transition_scroll_curr_page_change(Eo *obj EINA_UNUSED,
                                             Efl_Page_Transition_Scroll_Data *pd,
                                             double pos)
{
   Eina_List *list;
   Page_Info *pi;

   if (EINA_DBL_EQ(pos, 1.0))
     {
        EINA_LIST_FOREACH(pd->page_infos, list, pi)
          {
             pi->id = (pi->id - 1 + pd->page_info_num) % pd->page_info_num;
             pi->pos = pi->id - (pd->side_page_num + 1);

             pi->temp->x = pi->prev->geometry->x;
             pi->temp->y = pi->prev->geometry->y;
             pi->temp->w = pi->prev->geometry->w;
             pi->temp->h = pi->prev->geometry->h;

             if (eina_rectangles_intersect(pi->temp, pd->viewport) &&
                 (pi->id != 0) && (pi->id != (pd->page_info_num - 1)))
               {
                  pi->vis_page = EINA_TRUE;
                  pi->visible = EINA_TRUE;
                  efl_canvas_object_clip_set(pi->obj, pd->foreclip);
               }
             else
               {
                  pi->vis_page = EINA_FALSE;
                  pi->visible = EINA_FALSE;
                  efl_canvas_object_clip_set(pi->obj, pd->backclip);
                  if (pi->content)
                    {
                       efl_pack_unpack(pi->obj, pi->content);
                       efl_canvas_object_clip_set(pi->content, pd->backclip);
                    }
               }
          }

        pd->head = pd->head->next;
        pd->tail = pd->tail->next;
     }
   else if (EINA_DBL_EQ(pos, -1.0))
     {
        EINA_LIST_FOREACH(pd->page_infos, list, pi)
          {
             pi->id = (pi->id + 1) % pd->page_info_num;
             pi->pos = pi->id - (pd->side_page_num + 1);

             pi->temp->x = pi->next->geometry->x;
             pi->temp->y = pi->next->geometry->y;
             pi->temp->w = pi->next->geometry->w;
             pi->temp->h = pi->next->geometry->h;

             if (eina_rectangles_intersect(pi->temp, pd->viewport) &&
                 (pi->id != 0) && (pi->id != (pd->page_info_num - 1)))
               {
                  pi->vis_page = EINA_TRUE;
                  pi->visible = EINA_TRUE;
                  efl_canvas_object_clip_set(pi->obj, pd->foreclip);
               }
             else
               {
                  pi->vis_page = EINA_FALSE;
                  pi->visible = EINA_FALSE;
                  efl_canvas_object_clip_set(pi->obj, pd->backclip);
               }
          }

        pd->head = pd->head->prev;
        pd->tail = pd->tail->prev;
     }

   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
        pi->geometry->x = pi->temp->x;
        pi->geometry->y = pi->temp->y;
        pi->geometry->w = pi->temp->w;
        pi->geometry->h = pi->temp->h;
     }
#if DEBUG
   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
        ERR("[id: %d] vis_page %d visible %d content %d",
            pi->id, pi->vis_page, pi->visible, pi->content_num);
     }
   printf("\n");
#endif
}

EOLIAN static void
_efl_page_transition_scroll_pack_end(Eo *obj EINA_UNUSED,
                                     Efl_Page_Transition_Scroll_Data *pd EINA_UNUSED,
                                     Efl_Gfx *subobj EINA_UNUSED)
{
   //TODO implement animation
}

EOLIAN static int
_efl_page_transition_scroll_side_page_num_get(Eo *obj EINA_UNUSED,
                                              Efl_Page_Transition_Scroll_Data *pd)
{
   return pd->side_page_num;
}

EOLIAN static void
_efl_page_transition_scroll_side_page_num_set(Eo *obj,
                                              Efl_Page_Transition_Scroll_Data *pd,
                                              int side_page_num)
{
   EFL_PAGE_TRANSITION_DATA_GET(obj, spd);
   Page_Info *pi;
   int delta, i;

   if (pd->side_page_num == side_page_num) return;

   delta = side_page_num - pd->side_page_num;
   pd->side_page_num = side_page_num;
   pd->page_info_num = (side_page_num * 2) + 3;

   if (!spd->pager.obj) return;

   if (delta > 0)
     {
        for (i = 0; i < delta; i++)
          {
             pi = calloc(1, sizeof(*pi));
             pi->obj = efl_add(EFL_UI_BOX_CLASS, spd->pager.obj);
             efl_canvas_group_member_add(spd->pager.group, pi->obj);
             pi->content_num = -1;
             pi->content = NULL;
             pi->geometry = eina_rectangle_new(0, 0, 0, 0);
             pi->temp = eina_rectangle_new(0, 0, 0, 0);
             pd->page_infos = eina_list_prepend_relative(pd->page_infos, pi, pd->head);

             pi->next = pd->head;
             pd->head->prev = pi;
             pi->prev = pd->tail;
             pd->tail->next = pi;
             pd->head = pi;

             efl_gfx_stack_below(pi->obj, pi->next->obj);

             pi = calloc(1, sizeof(*pi));
             pi->obj = efl_add(EFL_UI_BOX_CLASS, spd->pager.obj);
             efl_canvas_group_member_add(spd->pager.group, pi->obj);
             pi->content_num = -1;
             pi->content = NULL;
             pi->geometry = eina_rectangle_new(0, 0, 0, 0);
             pi->temp = eina_rectangle_new(0, 0, 0, 0);
             pd->page_infos = eina_list_append_relative(pd->page_infos, pi, pd->tail);

             pi->next = pd->head;
             pd->head->prev = pi;
             pi->prev = pd->tail;
             pd->tail->next = pi;
             pd->tail = pi;

             efl_gfx_stack_above(pi->obj, pi->prev->obj);
          }
     }
   else
     {
        for (i = 0; i > delta; i--)
          {
             pi = pd->head;
             pd->head = pi->next;

             efl_canvas_object_clip_set(pi->content, pd->backclip);
             efl_pack_unpack(pi->obj, pi->content);
             efl_del(pi->obj);
             eina_rectangle_free(pi->geometry);
             eina_rectangle_free(pi->temp);
             pi->prev->next = pi->next;
             pi->next->prev = pi->prev;
             pd->page_infos = eina_list_remove(pd->page_infos, pi);
             free(pi);

             pi = pd->tail;
             pd->tail = pi->prev;

             efl_canvas_object_clip_set(pi->content, pd->backclip);
             efl_pack_unpack(pi->obj, pi->content);
             efl_del(pi->obj);
             eina_rectangle_free(pi->geometry);
             eina_rectangle_free(pi->temp);
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

   _page_info_geometry_change(pd, spd);
}

EOLIAN static void
_efl_page_transition_scroll_loop_set(Eo *obj,
                                     Efl_Page_Transition_Scroll_Data *pd,
                                     Efl_Ui_Pager_Loop loop)
{
   EFL_PAGE_TRANSITION_DATA_GET(obj, spd);

   if (spd->loop == loop) return;

   efl_page_transition_loop_set(efl_super(obj, MY_CLASS), loop);

   _content_show(pd, spd);
}

EOLIAN static Eo *
_efl_page_transition_scroll_efl_object_constructor(Eo *obj,
                                                   Efl_Page_Transition_Scroll_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->side_page_num = 1;
   pd->page_info_num = pd->side_page_num * 2 + 3;
   pd->viewport = eina_rectangle_new(0, 0, 0, 0);

   return obj;
}

EOLIAN static void
_efl_page_transition_scroll_efl_object_destructor(Eo *obj,
                                                  Efl_Page_Transition_Scroll_Data *pd)
{
   Page_Info *pi;

   eina_rectangle_free(pd->viewport);

   if (pd->page_infos)
     {
        EINA_LIST_FREE(pd->page_infos, pi)
          {
             efl_del(pi->obj);
             eina_rectangle_free(pi->geometry);
             eina_rectangle_free(pi->temp);
             free(pi);
          }
     }

   efl_destructor(efl_super(obj, MY_CLASS));
}


#define EFL_PAGE_TRANSITION_SCROLL_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_page_transition_update, \
                      _efl_page_transition_scroll_update), \
   EFL_OBJECT_OP_FUNC(efl_page_transition_curr_page_change, \
                      _efl_page_transition_scroll_curr_page_change), \
   EFL_OBJECT_OP_FUNC(efl_page_transition_page_size_set, \
                      _efl_page_transition_scroll_page_size_set), \
   EFL_OBJECT_OP_FUNC(efl_page_transition_padding_size_set, \
                      _efl_page_transition_scroll_padding_size_set), \
   EFL_OBJECT_OP_FUNC(efl_page_transition_pack_end, \
                      _efl_page_transition_scroll_pack_end), \
   EFL_OBJECT_OP_FUNC(efl_page_transition_loop_set, \
                      _efl_page_transition_scroll_loop_set)

#include "efl_page_transition_scroll.eo.c"
