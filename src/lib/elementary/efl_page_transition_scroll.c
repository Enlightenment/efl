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

#define DEBUG 1

EOLIAN static void
_efl_page_transition_scroll_efl_page_transition_bind(Eo *obj,
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
   double d;
   int n;
#if 0
   d = (double)pd->w / (double)pd->page_spec.w;
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
   Eina_List *list;
   Page_Info *pi;

   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
         pi->x = pd->x + (pd->w / 2)
            + pi->pos * (pd->page_spec.w + pd->page_spec.padding)
            - (pd->page_spec.w / 2);
         pi->y = pd->y + (pd->h / 2) - (pd->page_spec.h / 2);
         pi->w = pd->page_spec.w;
         pi->h = pd->page_spec.h;

         if (RECT_INTERSECT(pd->x, pd->y, pd->w, pd->h,
                            pi->x, pi->y, pi->w, pi->h))
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
             if (pi->vis_page && !pi->filled)
               {
                  tmp_id = (pd->current_page + pi->pos + pd->cnt) % pd->cnt;
                  tmp = eina_list_nth(pd->content_list, tmp_id);

                  efl_pack(pi->obj, tmp);
                  efl_gfx_visible_set(tmp, EINA_TRUE);

                  pi->content_num = tmp_id;
                  pi->content = tmp;
                  pi->filled = EINA_TRUE;
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

             if (!pi->vis_page && tpi->vis_page)
               {
                  if (RECT_INTERSECT(pi->tx, pi->ty, pi->tw, pi->th,
                                     pd->x, pd->y, pd->w, pd->h))
                    {
                       if (!pi->visible)
                         {
                            tmp_id = (pd->current_page + pi->pos + pd->cnt) % pd->cnt;
                            tmp = eina_list_nth(pd->content_list, tmp_id);

                            if (pi->filled) efl_pack_unpack(pi->obj, pi->content);
                            efl_gfx_visible_set(pi->content, EINA_FALSE);

                            efl_pack(pi->obj, tmp);
                            efl_gfx_visible_set(tmp, EINA_TRUE);

                            pi->content_num = tmp_id;
                            pi->content = tmp;
                            pi->filled = EINA_TRUE;
                            pi->visible = EINA_TRUE;
                            efl_canvas_object_clip_set(pi->obj, pd->viewport.foreclip);
                         }
                    }
                  else
                    {
                       if (pi->visible)
                         {
                            pi->visible = EINA_FALSE;
                            efl_canvas_object_clip_set(pi->obj, pd->viewport.backclip);
                         }
                    }
               }
             else if (pi->vis_page && !tpi->vis_page)
               {
                  if (!RECT_INTERSECT(pi->tx, pi->ty, pi->tw, pi->th,
                                      pd->x, pd->y, pd->w, pd->h))
                    {
                       if (pi->visible)
                         {
                            pi->visible = EINA_FALSE;
                            efl_canvas_object_clip_set(pi->obj, pd->viewport.backclip);
                         }
                    }
                  else
                    {
                       if (!pi->visible)
                         {
                            pi->visible = EINA_TRUE;
                            efl_canvas_object_clip_set(pi->obj, pd->viewport.foreclip);
                         }
                    }
               }
          }
     }
}

EOLIAN static void
_efl_page_transition_scroll_efl_page_transition_finish(Eo *obj EINA_UNUSED,
                                                       Efl_Page_Transition_Scroll_Data *_pd EINA_UNUSED)
{
   Evas_Object *pager = _pd->pager;

   if (!pager) return;

   EFL_UI_PAGER_DATA_GET(pager, pd);

   Eina_List *list;
   Page_Info *pi, *tpi;
#if DEBUG
   EINA_LIST_FOREACH(pd->page_infos, list, pi)
     {
        ERR("[id: %d] content %d, filled %d visible %d",
            pi->id, pi->content_num, pi->filled, pi->visible);
     }
   printf("\n");
#endif
   if (pd->move == 1.0)
     {
        EINA_LIST_FOREACH(pd->page_infos, list, pi)
          {
             pi->id = (pi->id - 1 + pd->page_info_num) % pd->page_info_num;
             pi->pos = pi->id - 2;

             pi->x = pi->tx;
             pi->y = pi->ty;
             pi->w = pi->tw;
             pi->h = pi->th;

             if (RECT_INTERSECT(pd->x, pd->y, pd->w, pd->h,
                                pi->x, pi->y, pi->w, pi->h))
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
     }
   else if (pd->move == -1.0)
     {
        EINA_LIST_FOREACH(pd->page_infos, list, pi)
          {
             pi->id = (pi->id + 1) % pd->page_info_num;
             pi->pos = pi->id - 2;

             pi->x = pi->tx;
             pi->y = pi->ty;
             pi->w = pi->tw;
             pi->h = pi->th;

             if (RECT_INTERSECT(pd->x, pd->y, pd->w, pd->h,
                                pi->x, pi->y, pi->w, pi->h))
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
     }

   pd->move = 0.0;
}

#include "efl_page_transition_scroll.eo.c"
