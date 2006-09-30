#include "evas_gl_private.h"

void
evas_gl_common_rect_draw(Evas_GL_Context *gc, int x, int y, int w, int h)
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
