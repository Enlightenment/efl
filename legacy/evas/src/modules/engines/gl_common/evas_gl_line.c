#include "evas_gl_private.h"

void
evas_gl_common_line_draw(Evas_Engine_GL_Context *gc, int x1, int y1, int x2, int y2)
{
   RGBA_Draw_Context *dc;
   int r, g, b, a;
   int c, cx, cy, cw, ch;

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
        r = g = b = a = 255;
     }

   glFlush();

   c = gc->dc->clip.use;
   cx = gc->dc->clip.x; cy = gc->dc->clip.y;
   cw = gc->dc->clip.w; ch = gc->dc->clip.h;

   evas_gl_common_context_line_push(gc, x1, y1, x2, y2,
                                    c, cx, cy, cw, ch,
                                    r, g, b, a);
}
