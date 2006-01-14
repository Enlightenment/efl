#include "evas_gl_private.h"

void
evas_gl_common_swap_rect(Evas_GL_Context *gc, int x, int y, int w, int h)
{
   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_FRONT);
   evas_gl_common_context_blend_set(gc, 0);
   evas_gl_common_context_clip_set(gc, 0, 0, 0, 0, 0);
   evas_gl_common_context_dither_set(gc, 0);
   y = gc->h - y - h;
   glRasterPos2i(x, gc->h - y);
   glCopyPixels(x, y, w, h, GL_COLOR);
}
