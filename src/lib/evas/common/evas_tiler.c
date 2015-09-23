#include "evas_common_private.h"
#include "region.h"

EAPI void
evas_common_tilebuf_init(void)
{
}

EAPI Tilebuf *
evas_common_tilebuf_new(int w, int h)
{
   Tilebuf *tb = malloc(sizeof(Tilebuf));
   tb->outbuf_w = w;
   tb->outbuf_h = h;
   tb->region = region_new(tb->outbuf_w, tb->outbuf_h);
   return tb;
}

EAPI void
evas_common_tilebuf_free(Tilebuf *tb)
{
   region_free(tb->region);
   free(tb);
}

EAPI void
evas_common_tilebuf_set_tile_size(Tilebuf *tb EINA_UNUSED, int tw EINA_UNUSED, int th EINA_UNUSED)
{
}

EAPI void
evas_common_tilebuf_get_tile_size(Tilebuf *tb EINA_UNUSED, int *tw, int *th)
{
   if (tw) *tw = 1;
   if (th) *th = 1;
}

EAPI void
evas_common_tilebuf_tile_strict_set(Tilebuf *tb EINA_UNUSED, Eina_Bool strict EINA_UNUSED)
{
}

EAPI int
evas_common_tilebuf_add_redraw(Tilebuf *tb, int x, int y, int w, int h)
{
   region_rect_add(tb->region, x, y, w, h);
   return 1;
}

EAPI int
evas_common_tilebuf_del_redraw(Tilebuf *tb, int x, int y, int w, int h)
{
   region_rect_del(tb->region, x, y, w, h);
   return 1;
}

EAPI int
evas_common_tilebuf_add_motion_vector(Tilebuf *tb EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED, int dx EINA_UNUSED, int dy EINA_UNUSED, int alpha EINA_UNUSED)
{
   return 0;
}

EAPI void
evas_common_tilebuf_clear(Tilebuf *tb)
{
   region_free(tb->region);
   tb->region = region_new(tb->outbuf_w, tb->outbuf_h);
}

EAPI Tilebuf_Rect *
evas_common_tilebuf_get_render_rects(Tilebuf *tb)
{
   Tilebuf_Rect *rects = NULL, *r, *rend, *rbuf;
   Box *rects2, *rs;
   int n;

   rects2 = region_rects(tb->region);
   if (!rects2) return NULL;
   n = region_rects_num(tb->region);
   if (n <= 0) return NULL;

   rbuf = malloc(n * sizeof(Tilebuf_Rect));
   if (!rbuf) return NULL;

   rend = rbuf + n;
   rs = rects2;
   for (r = rbuf; r < rend; r++)
     {
        EINA_INLIST_GET(r)->next = NULL;
        EINA_INLIST_GET(r)->prev = NULL;
        EINA_INLIST_GET(r)->last = NULL;
        r->x = rs->x1;
        r->y = rs->y1;
        r->w = rs->x2 - rs->x1;
        r->h = rs->y2 - rs->y1;
        rs++;
        rects = (Tilebuf_Rect *)
          eina_inlist_append(EINA_INLIST_GET(rects),
                             EINA_INLIST_GET(r));
     }
   return rects;
}

EAPI void
evas_common_tilebuf_free_render_rects(Tilebuf_Rect *rects)
{
   free(rects);
}
