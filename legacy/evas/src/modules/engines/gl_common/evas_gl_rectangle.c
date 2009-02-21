#include "evas_gl_private.h"

static void
evas_gl_common_rect_draw_internal(Evas_GL_Context *gc, int x, int y, int w, int h)
{
   int r, g, b, a;
   RGBA_Draw_Context *dc = gc->dc;

   a = (dc->col.col >> 24) & 0xff;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   evas_gl_common_context_color_set(gc, r, g, b, a);
   if (a < 255) evas_gl_common_context_blend_set(gc, 1);
   else evas_gl_common_context_blend_set(gc, 0);
   if (dc->clip.use)
     evas_gl_common_context_clip_set(gc, 1,
				     dc->clip.x, dc->clip.y,
				     dc->clip.w, dc->clip.h);
   else
     evas_gl_common_context_clip_set(gc, 0,
				     0, 0, 0, 0);
   evas_gl_common_context_texture_set(gc, NULL, 0, 0, 0);
   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_BACK);
   glBegin(GL_QUADS);
   glVertex2i(x, y);
   glVertex2i(x + w, y);
   glVertex2i(x + w, y + h);
   glVertex2i(x, y + h);
   glEnd();
}

void
evas_gl_common_rect_draw(Evas_GL_Context *gc, int x, int y, int w, int h)
{
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch;
   int          i;
   
   if ((w <= 0) || (h <= 0)) return;
   if (!(RECTS_INTERSECT(x, y, w, h, 0, 0, gc->w, gc->h)))
     return;
   /* save out clip info */
   c = gc->dc->clip.use; cx = gc->dc->clip.x; cy = gc->dc->clip.y; cw = gc->dc->clip.w; ch = gc->dc->clip.h;
   evas_common_draw_context_clip_clip(gc->dc, 0, 0, gc->w, gc->h);
   /* no cutouts - cut right to the chase */
   if (!gc->dc->cutout.rects)
     {
        evas_gl_common_rect_draw_internal(gc, x, y, w, h);
     }
   else
     {
        evas_common_draw_context_clip_clip(gc->dc, x, y, w, h);
        /* our clip is 0 size.. abort */
        if ((gc->dc->clip.w > 0) && (gc->dc->clip.h > 0))
          {
             rects = evas_common_draw_context_apply_cutouts(gc->dc);
             for (i = 0; i < rects->active; ++i)
               {
                  r = rects->rects + i;
                  evas_common_draw_context_set_clip(gc->dc, r->x, r->y, r->w, r->h);
                  evas_gl_common_rect_draw_internal(gc, x, y, w, h);
               }
             evas_common_draw_context_apply_clear_cutouts(rects);
          }
     }
   /* restore clip info */
   gc->dc->clip.use = c; gc->dc->clip.x = cx; gc->dc->clip.y = cy; gc->dc->clip.w = cw; gc->dc->clip.h = ch;
}
