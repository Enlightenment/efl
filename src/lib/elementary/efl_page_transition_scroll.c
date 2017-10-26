#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_page_transition_scroll.h"
#include "efl_ui_widget_pager.h"

//FIXME use ELM_RECTS_INTERSECT
#define INTERSECT(x1, w1, x2, w2) \
   (!(((x1) + (w1) <= (x2)) || ((x2) + (w2) <= (x1))))
#define RECT_INTERSECT(x1, y1, w1, h1, x2, y2, w2, h2) \
   ((INTERSECT(x1, w1, x2, w2)) && (INTERSECT(y1, h1, y2, h2)))

#define DEBUG 0

EOLIAN static void
_efl_page_transition_scroll_efl_page_transition_bind(Eo *obj EINA_UNUSED,
                                                     Efl_Page_Transition_Scroll_Data *_pd,
                                                     Eo *pager)
{
   _pd->pager = pager;
}

EOLIAN static void
_efl_page_transition_scroll_efl_page_transition_init(Eo *obj,
                                                     Efl_Page_Transition_Scroll_Data *_pd)
{
   EFL_UI_PAGER_DATA_GET(_pd->pager, pd);
   Eina_List *list;
   Page_Info *pi;
   int content_w;

#if 0
   double d;
   int n;

   d = (double) pd->w / ((double) pd->page_spec.w + (double) pd->page_spec.padding);
   n = (int) d;
   d -= n;

   if ((n % 2) == 0)
     {
        n -= 1;
        d += 1.0;
     }
   if (d > 0) n += 2;

   pd->page_info_num = n;
#endif

   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
         pi->x = pd->x + (pd->w / 2)
            + pi->pos * (pd->page_spec.w + pd->page_spec.padding)
            - (pd->page_spec.w / 2);
         pi->y = pd->y + (pd->h / 2) - (pd->page_spec.h / 2);
         pi->w = pd->page_spec.w;
         pi->h = pd->page_spec.h;

         if (RECT_INTERSECT(pd->x, pd->y, pd->w, pd->h,
                            pi->x, pi->y, pi->w, pi->h) &&
             (pi->id != 0) && (pi->id != (pd->page_info_num - 1)))
           {
              pi->vis_page = EINA_TRUE;
              pi->visible = EINA_TRUE;
              efl_canvas_object_clip_set(pi->obj, pd->viewport.foreclip);
           }
         else
           {
              pi->vis_page = EINA_FALSE;
              pi->visible = EINA_FALSE;
              efl_canvas_object_clip_set(pi->obj, pd->viewport.backclip);
           }
     }

   content_w = pd->page_spec.w
      + ((pd->page_spec.w + pd->page_spec.padding) * pd->side_page_num * 2);

   if (content_w < pd->w)
     {
        pd->viewport.x = pd->x + ((pd->w - content_w) / 2);
        pd->viewport.y = pd->y;
        pd->viewport.w = content_w;
        pd->viewport.h = pd->h;
     }
   else
     {
        pd->viewport.x = pd->x;
        pd->viewport.y = pd->y;
        pd->viewport.w = pd->w;
        pd->viewport.h = pd->h;
     }

   efl_gfx_position_set(pd->viewport.foreclip,
                        EINA_POSITION2D(pd->viewport.x, pd->viewport.y));
   efl_gfx_size_set(pd->viewport.foreclip,
                    EINA_SIZE2D(pd->viewport.w, pd->viewport.h));

   //FIXME need to call update function?
   //      or can we substitute the code with size_set and position_set
   //      and do something more for dynamic viewport size change?
   efl_page_transition_update(obj);
}

EOLIAN static void
_efl_page_transition_scroll_efl_page_transition_update(Eo *obj EINA_UNUSED,
                                                       Efl_Page_Transition_Scroll_Data *_pd EINA_UNUSED)
{
   Evas_Object *pager = _pd->pager;

   if (!pager) return;

   EFL_UI_PAGER_DATA_GET(pager, pd);

   int tmp_id;
   Eo *tmp;
   Eina_List *list;
   Page_Info *pi, *tpi;

   double t = pd->move;
   if (t < 0) t *= (-1);

   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
        if (pd->move == 0.0)
          {
             if (!pi->vis_page && !!pi->content)
               {
                  efl_canvas_object_clip_set(pi->obj, pd->viewport.backclip);
                  efl_pack_unpack(pi->obj, pi->content);
                  efl_canvas_object_clip_set(pi->content, pd->viewport.backclip);

                  pi->content_num = -1;
                  pi->content = NULL;
                  pi->visible = EINA_FALSE;
               }
             else if (pi->vis_page && !pi->content)
               {
                  efl_canvas_object_clip_set(pi->obj, pd->viewport.foreclip);
                  tmp_id = (pd->current_page + pi->pos + pd->cnt) % pd->cnt;
                  tmp = eina_list_nth(pd->content_list, tmp_id);

                  efl_pack(pi->obj, tmp);
                  efl_canvas_object_clip_set(tmp, pd->viewport.foreclip);

                  pi->content_num = tmp_id;
                  pi->content = tmp;
                  pi->visible = EINA_TRUE;
               }

             efl_gfx_position_set(pi->obj, EINA_POSITION2D(pi->x, pi->y));
             efl_gfx_size_set(pi->obj, EINA_SIZE2D(pi->w, pi->h));
          }
        else
          {
             if (pd->move < 0)
               tpi = pi->next;
             else
               tpi = pi->prev;

             pi->tx = tpi->x * t + pi->x * (1 - t);
             pi->ty = tpi->y * t + pi->y * (1 - t);
             pi->tw = tpi->w * t + pi->w * (1 - t);
             pi->th = tpi->h * t + pi->h * (1 - t);

             efl_gfx_position_set(pi->obj, EINA_POSITION2D(pi->tx, pi->ty));
             efl_gfx_size_set(pi->obj, EINA_SIZE2D(pi->tw, pi->th));

             if ((!pi->vis_page && tpi->vis_page) || (pi->vis_page && !tpi->vis_page))
               {
                  if (!RECT_INTERSECT(pi->tx, pi->ty, pi->tw, pi->th,
                                      pd->viewport.x, pd->viewport.y,
                                      pd->viewport.w, pd->viewport.h))
                    {
                       if (pi->visible)
                         {
                            efl_canvas_object_clip_set(pi->obj, pd->viewport.backclip);

                            if (!!pi->content) efl_pack_unpack(pi->obj, pi->content);
                            efl_canvas_object_clip_set(pi->content, pd->viewport.backclip);

                            pi->content_num = -1;
                            pi->content = NULL;
                            pi->visible = EINA_FALSE;
                         }
                    }
                  else
                    {
                       if (!pi->visible)
                         {
                            efl_canvas_object_clip_set(pi->obj, pd->viewport.foreclip);

                            if (!!pi->content) efl_pack_unpack(pi->obj, pi->content);
                            efl_canvas_object_clip_set(pi->content, pd->viewport.backclip);

                            tmp_id = (pd->current_page + pi->pos + pd->cnt) % pd->cnt;
                            tmp = eina_list_nth(pd->content_list, tmp_id);

                            efl_pack(pi->obj, tmp);
                            efl_canvas_object_clip_set(tmp, pd->viewport.foreclip);

                            pi->content_num = tmp_id;
                            pi->content = tmp;
                            pi->visible = EINA_TRUE;
                         }
                    }
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
_efl_page_transition_scroll_efl_page_transition_finish(Eo *obj EINA_UNUSED,
                                                       Efl_Page_Transition_Scroll_Data *_pd EINA_UNUSED)
{
   Evas_Object *pager = _pd->pager;

   if (!pager) return;

   EFL_UI_PAGER_DATA_GET(pager, pd);

   Eina_List *list;
   Page_Info *pi;

   if (pd->move == 1.0)
     {
        EINA_LIST_FOREACH(pd->page_infos, list, pi)
          {
             pi->id = (pi->id - 1 + pd->page_info_num) % pd->page_info_num;
             pi->pos = pi->id - (pd->side_page_num + 1);

             pi->x = pi->tx;
             pi->y = pi->ty;
             pi->w = pi->tw;
             pi->h = pi->th;

             if (RECT_INTERSECT(pi->x, pi->y, pi->w, pi->h,
                                pd->viewport.x, pd->viewport.y,
                                pd->viewport.w, pd->viewport.h) &&
                 (pi->id != 0) && (pi->id != (pd->page_info_num - 1)))
               {
                  pi->vis_page = EINA_TRUE;
                  pi->visible = EINA_TRUE;
                  efl_canvas_object_clip_set(pi->obj, pd->viewport.foreclip);
               }
             else
               {
                  pi->vis_page = EINA_FALSE;
                  pi->visible = EINA_FALSE;
                  efl_canvas_object_clip_set(pi->obj, pd->viewport.backclip);
               }
          }
        pd->current_page = (pd->current_page + 1) % pd->cnt;
        pd->head = pd->head->next;
        pd->tail = pd->tail->next;
     }
   else if (pd->move == -1.0)
     {
        EINA_LIST_FOREACH(pd->page_infos, list, pi)
          {
             pi->id = (pi->id + 1) % pd->page_info_num;
             pi->pos = pi->id - (pd->side_page_num + 1);

             pi->x = pi->tx;
             pi->y = pi->ty;
             pi->w = pi->tw;
             pi->h = pi->th;

             if (RECT_INTERSECT(pi->x, pi->y, pi->w, pi->h,
                                pd->viewport.x, pd->viewport.y,
                                pd->viewport.w, pd->viewport.h) &&
                 (pi->id != 0) && (pi->id != (pd->page_info_num - 1)))
               {
                  pi->vis_page = EINA_TRUE;
                  pi->visible = EINA_TRUE;
                  efl_canvas_object_clip_set(pi->obj, pd->viewport.foreclip);
               }
             else
               {
                  pi->vis_page = EINA_FALSE;
                  pi->visible = EINA_FALSE;
                  efl_canvas_object_clip_set(pi->obj, pd->viewport.backclip);
               }
          }
        pd->current_page = (pd->current_page - 1 + pd->cnt) % pd->cnt;
        pd->head = pd->head->prev;
        pd->tail = pd->tail->prev;
     }

#if DEBUG
   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
        ERR("[id: %d] vis_page %d visible %d content %d",
            pi->id, pi->vis_page, pi->visible, pi->content_num);
     }
   printf("\n");
#endif

   pd->move = 0.0;
}

#include "efl_page_transition_scroll.eo.c"
