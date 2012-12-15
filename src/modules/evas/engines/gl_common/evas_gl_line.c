#include "evas_gl_private.h"

void
evas_gl_common_line_draw(Evas_Engine_GL_Context *gc, int x1, int y1, int x2, int y2)
{
   RGBA_Draw_Context *dc;
   int r, g, b, a;
   int c, cx, cy, cw, ch;
   static int offset_hack = -1;
   const int OFFSET_HACK_OFF = 0;
   const int OFFSET_HACK_DEFAULT = 1;
   const int OFFSET_HACK_ARM = 2;

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

   //I have no idea but shift lines/clips since the gl line origin position and
   //sissor area is slightly different by the gl driver.
   if (offset_hack == -1)
     {
        if (!getenv("EVAS_GL_LINE_OFFSET_HACK_DISABLE"))
          {
             const char *vendor_name;
             vendor_name = (char *) glGetString(GL_VENDOR);
             if (vendor_name && !strcmp(vendor_name, "ARM"))
               offset_hack = OFFSET_HACK_ARM;
             else
               offset_hack = OFFSET_HACK_DEFAULT;
          }
        else offset_hack = OFFSET_HACK_OFF;
     }

   if (offset_hack == OFFSET_HACK_DEFAULT)
     {
        if ((gc->rot == 0) || (gc->rot == 90))
          {
             x1++;
             x2++;
          }
        if ((gc->rot == 90) || (gc->rot == 180))
          {
             y1++;
             y2++;
          }
     }
   else if (offset_hack == OFFSET_HACK_ARM)
     {
        if ((gc->rot == 90) || (gc->rot == 180))
          {
             cx--;
             cw--;
          }
        if ((gc->rot == 180) || (gc->rot == 270))
          {
             cy--;
             ch--;
          }
     }

   evas_gl_common_context_line_push(gc, x1, y1, x2, y2,
                                    c, cx, cy, cw, ch,
                                    r, g, b, a);
}
