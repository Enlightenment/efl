#include "evas_gl_private.h"

void
evas_gl_common_rect_draw(Evas_Engine_GL_Context *gc, int x, int y, int w, int h)
{
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch, cr, cg, cb, ca, i;

   if ((w <= 0) || (h <= 0)) return;
   if (!(RECTS_INTERSECT(x, y, w, h, 0, 0, gc->w, gc->h))) return;
   /* save out clip info */
   c = gc->dc->clip.use; cx = gc->dc->clip.x; cy = gc->dc->clip.y; cw = gc->dc->clip.w; ch = gc->dc->clip.h;

   ca = (gc->dc->col.col >> 24) & 0xff;
   if ((gc->dc->render_op != EVAS_RENDER_COPY) && (ca <= 0)) return;
   cr = (gc->dc->col.col >> 16) & 0xff;
   cg = (gc->dc->col.col >> 8 ) & 0xff;
   cb = (gc->dc->col.col      ) & 0xff;
   evas_common_draw_context_clip_clip(gc->dc, 0, 0, gc->shared->w, gc->shared->h);
   /* no cutouts - cut right to the chase */

   if (gc->dc->clip.use)
     {
        RECTS_CLIP_TO_RECT(x, y, w, h,
                           gc->dc->clip.x, gc->dc->clip.y,
                           gc->dc->clip.w, gc->dc->clip.h);
     }

   if (!gc->dc->cutout.rects)
     {
        evas_gl_common_context_rectangle_push(gc, x, y, w, h, cr, cg, cb, ca);
     }
   else
     {
        evas_common_draw_context_clip_clip(gc->dc, x, y, w, h);
        /* our clip is 0 size.. abort */
        if ((gc->dc->clip.w > 0) && (gc->dc->clip.h > 0))
          {
             _evas_gl_common_cutout_rects = evas_common_draw_context_apply_cutouts(gc->dc, _evas_gl_common_cutout_rects);
             for (i = 0; i < _evas_gl_common_cutout_rects->active; ++i)
               {
                  r = _evas_gl_common_cutout_rects->rects + i;
                  if ((r->w > 0) && (r->h > 0))
                    {
                       evas_gl_common_context_rectangle_push(gc, r->x, r->y, r->w, r->h, cr, cg, cb, ca);
                    }
               }
             evas_common_draw_context_cutouts_free(_evas_gl_common_cutout_rects);
          }
     }
   /* restore clip info */
   gc->dc->clip.use = c; gc->dc->clip.x = cx; gc->dc->clip.y = cy; gc->dc->clip.w = cw; gc->dc->clip.h = ch;
}
