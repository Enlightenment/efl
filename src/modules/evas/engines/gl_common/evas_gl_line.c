#include "evas_gl_private.h"


void
evas_gl_common_line_draw(Evas_Engine_GL_Context *gc, int x1, int y1, int x2, int y2)
{
   RGBA_Draw_Context *dc;
   int r, g, b, a;
   int c, cx, cy, cw, ch;
   static int offset_hack = -1;

   if (offset_hack == -1) {
        if (getenv("EVAS_GL_LINE_NO_OFFSET_HACK")) offset_hack = 0;
        else offset_hack = 1;
   }

   dc = gc->dc;
   if (dc->mul.use)
     {
        a = (dc->mul.col >> 24) & 0xff;
        r = (dc->mul.col >> 16) & 0xff;
        g = (dc->mul.col >> 8 ) & 0xff;
        b = (dc->mul.col      ) & 0xff;
     }
   else
     {
        a = (dc->col.col >> 24) & 0xff;
        r = (dc->col.col >> 16) & 0xff;
        g = (dc->col.col >> 8 ) & 0xff;
        b = (dc->col.col      ) & 0xff;
     }

   c = gc->dc->clip.use;
   cx = gc->dc->clip.x; cy = gc->dc->clip.y;
   cw = gc->dc->clip.w; ch = gc->dc->clip.h;

   if (offset_hack)
     {
        /* Increment pixels since the gl line origin position is slightly different
           on some platform.  Carsten did this hack.. doesn't remember exactly why but
           it works most drivers. */
        if (x1 == x2)
          {
             if (gc->rot == 0)
                x1++;
          }

        if (y1 == y2)
          {
             if ((gc->rot == 90) || (gc->rot == 180))
                y1++;
          }

        x2++; y2++;
     }

   evas_gl_common_context_line_push(gc, x1, y1, x2, y2,
                                    c, cx, cy, cw, ch,
                                    r, g, b, a);
}
