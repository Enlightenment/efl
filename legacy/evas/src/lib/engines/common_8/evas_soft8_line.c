#include "evas_common_soft8.h"
#include "evas_soft8_scanline_fill.c"

/*
 * All functions except by evas_common_soft8_line_draw() expect x0 <= x1.
 */

static inline int
_in_range(int value, int min, int max)
{
   return min <= value && value <= max;
}

static inline int
_is_xy_inside_clip(int x, int y, const struct RGBA_Draw_Context_clip clip)
{
   if (!clip.use)
      return 1;

   if (!_in_range(x, clip.x, clip.x + clip.w - 1))
      return 0;

   if (!_in_range(y, clip.y, clip.y + clip.h - 1))
      return 0;

   return 1;
}

static inline int
_is_x_inside_clip(int x, const struct RGBA_Draw_Context_clip clip)
{
   if (!clip.use)
      return 1;

   return _in_range(x, clip.x, clip.x + clip.w - 1);
}

static inline int
_is_y_inside_clip(int y, const struct RGBA_Draw_Context_clip clip)
{
   if (!clip.use)
      return 1;

   return _in_range(y, clip.y, clip.y + clip.h - 1);
}

static inline int
_is_xy_inside_rect(int x, int y, int w, int h)
{
   return _in_range(x, 0, w - 1) && _in_range(y, 0, h - 1);
}

static inline int
_is_empty_clip(const struct RGBA_Draw_Context_clip clip)
{
   return clip.w < 1 || clip.h < 1;
}

static void
_soft8_line_point(Soft8_Image * dst, RGBA_Draw_Context * dc, int x, int y)
{
   DATA8 gry8, *dst_itr;
   DATA8 alpha;

   if (!_is_xy_inside_rect(x, y, dst->cache_entry.w, dst->cache_entry.h))
      return;

   if (!_is_xy_inside_clip(x, y, dc->clip))
      return;

   dst_itr = dst->pixels + (dst->stride * y) + x;
   alpha = A_VAL(&dc->col.col);
   gry8 = GRY_8_FROM_RGB(&dc->col.col);

   if (alpha == 0xff)
      _soft8_pt_fill_solid_solid(dst_itr, gry8);
   else if (alpha > 0)
     {
        alpha++;
        _soft8_pt_fill_transp_solid(dst_itr, gry8, alpha);
     }
}

static void
_soft8_line_horiz(Soft8_Image * dst, RGBA_Draw_Context * dc, int x0, int x1,
                  int y)
{
   DATA8 gry8, *dst_itr;
   DATA8 alpha;
   int w;

   if (!_is_y_inside_clip(y, dc->clip))
      return;

   if (x0 < dc->clip.x)
      x0 = dc->clip.x;

   if (x1 >= dc->clip.x + dc->clip.w)
      x1 = dc->clip.x + dc->clip.w - 1;

   w = x1 - x0;
   if (w < 1)
      return;

   dst_itr = dst->pixels + (dst->stride * y) + x0;
   alpha = A_VAL(&dc->col.col);
   gry8 = GRY_8_FROM_RGB(&dc->col.col);

   if (alpha == 0xff)
      _soft8_scanline_fill_solid_solid(dst_itr, w, gry8);
   else if (alpha > 0)
     {
        alpha++;
        _soft8_scanline_fill_transp_solid(dst_itr, w, gry8, alpha);
     }
}

static void
_soft8_line_vert(Soft8_Image * dst, RGBA_Draw_Context * dc, int x, int y0,
                 int y1)
{
   DATA8 gry8, *dst_itr;
   DATA8 alpha;
   int h;

   if (!_is_x_inside_clip(x, dc->clip))
      return;

   if (y1 < y0)
     {
        int t;
        t = y0;
        y0 = y1;
        y1 = t;
     }

   if (y0 < dc->clip.y)
      y0 = dc->clip.y;

   if (y1 >= dc->clip.y + dc->clip.h)
      y1 = dc->clip.y + dc->clip.h - 1;

   h = y1 - y0;
   if (h < 1)
      return;

   dst_itr = dst->pixels + (dst->stride * y0) + x;
   alpha = A_VAL(&dc->col.col);
   gry8 = GRY_8_FROM_RGB(&dc->col.col);

   if (alpha == 0xff)
     {
        for (; h > 0; h--, dst_itr += dst->stride)
           _soft8_pt_fill_solid_solid(dst_itr, gry8);
     }
   else if (alpha > 0)
     {
        alpha++;

        for (; h > 0; h--, dst_itr += dst->stride)
           _soft8_pt_fill_transp_solid(dst_itr, gry8, alpha);
     }
}

static inline void
_soft8_line_45deg_adjust_boundaries(const struct RGBA_Draw_Context_clip clip,
                                    int *p_x0, int *p_y0, int *p_x1, int *p_y1)
{
   int diff, dy, x0, y0, x1, y1;

   x0 = *p_x0;
   y0 = *p_y0;
   x1 = *p_x1;
   y1 = *p_y1;

   dy = y1 - y0;

   diff = clip.x - x0;
   if (diff > 0)
     {
        x0 = clip.x;
        y0 += (dy > 0) ? diff : -diff;
     }

   diff = x1 - (clip.x + clip.w);
   if (diff > 0)
     {
        x1 = clip.x + clip.w;
        y1 += (dy > 0) ? -diff : diff;
     }

   if (dy > 0)
     {
        diff = clip.y - y0;
        if (diff > 0)
          {
             y0 = clip.y;
             x0 += diff;
          }

        diff = y1 - (clip.y + clip.h);
        if (diff > 0)
          {
             y1 = clip.y + clip.h;
             x1 -= diff;
          }
     }
   else
     {
        diff = clip.y - y1;
        if (diff > 0)
          {
             y1 = clip.y;
             x1 -= diff;
          }

        diff = y0 - (clip.y + clip.h - 1);
        if (diff > 0)
          {
             y0 = clip.y + clip.h - 1;
             x0 += diff;
          }
     }

   *p_x0 = x0;
   *p_y0 = y0;
   *p_x1 = x1;
   *p_y1 = y1;
}

static void
_soft8_line_45deg(Soft8_Image * dst, RGBA_Draw_Context * dc, int x0, int y0,
                  int x1, int y1)
{
   int dy, step_dst_itr, len;
   DATA8 alpha;
   DATA8 *dst_itr, gry8;

   alpha = A_VAL(&dc->col.col);
   if (alpha < 1)
      return;

   gry8 = GRY_8_FROM_RGB(&dc->col.col);

   dy = y1 - y0;
   step_dst_itr = 1 + ((dy > 0) ? dst->stride : -dst->stride);

   _soft8_line_45deg_adjust_boundaries(dc->clip, &x0, &y0, &x1, &y1);

   len = (dy > 0) ? (y1 - y0) : (y0 - y1);
   if (len < 1)
      return;

   dst_itr = dst->pixels + dst->stride * y0 + x0;
   if (alpha == 0xff)
     {
        for (; len > 0; len--, dst_itr += step_dst_itr)
           _soft8_pt_fill_solid_solid(dst_itr, gry8);
     }
   else
     {
        alpha++;
        for (; len > 0; len--, dst_itr += step_dst_itr)
           _soft8_pt_fill_transp_solid(dst_itr, gry8, alpha);
     }
}

EFL_ALWAYS_INLINE void
_soft8_line_aliased_pt(DATA8 * dst_itr, DATA8 gry8, DATA8 alpha)
{
   if (alpha == 32)
      _soft8_pt_fill_solid_solid(dst_itr, gry8);
   else
      _soft8_pt_fill_transp_solid(dst_itr, gry8, alpha);
}

static void
_soft8_line_aliased(Soft8_Image * dst, RGBA_Draw_Context * dc, int x0, int y0,
                    int x1, int y1)
{
   int dx, dy, step_y, step_dst_itr;
   DATA8 gry8;
   DATA8 alpha;

   alpha = A_VAL(&dc->col.col);
   if (alpha == 0)
      return;
   alpha++;

   gry8 = GRY_8_FROM_RGB(&dc->col.col);

   dx = x1 - x0;
   dy = y1 - y0;

   if (dy >= 0)
     {
        step_y = 1;
        step_dst_itr = dst->stride;
     }
   else
     {
        dy = -dy;
        step_y = -1;
        step_dst_itr = -dst->stride;
     }

   if (dx > dy)
     {
        DATA8 *dst_itr;
        int e, x, y;

        e = -(dx / 2);
        y = y0;
        dst_itr = dst->pixels + dst->stride * y0 + x0;
        for (x = x0; x <= x1; x++, dst_itr++)
          {
             if (_is_xy_inside_clip(x, y, dc->clip))
                _soft8_line_aliased_pt(dst_itr, gry8, alpha);

             e += dy;
             if (e >= 0)
               {
                  dst_itr += step_dst_itr;
                  y += step_y;
                  e -= dx;
               }
          }
     }
   else
     {
        DATA8 *dst_itr;
        int e, x, y;

        e = -(dy / 2);
        x = x0;
        dst_itr = dst->pixels + dst->stride * y0 + x0;
        for (y = y0; y != y1; y += step_y, dst_itr += step_dst_itr)
          {
             if (_is_xy_inside_clip(x, y, dc->clip))
                _soft8_line_aliased_pt(dst_itr, gry8, alpha);

             e += dx;
             if (e >= 0)
               {
                  dst_itr++;
                  x++;
                  e -= dy;
               }
          }
     }
}

void
evas_common_soft8_line_draw(Soft8_Image * dst, RGBA_Draw_Context * dc, int x0, int y0,
                int x1, int y1)
{
   struct RGBA_Draw_Context_clip c_bkp, c_tmp;
   int dx, dy;
   int x, y, w, h;

   c_tmp.use = 1;
   c_tmp.x = 0;
   c_tmp.y = 0;
   c_tmp.w = dst->cache_entry.w;
   c_tmp.h = dst->cache_entry.h;

   /* save out clip info */
   c_bkp = dc->clip;
   if (c_bkp.use)
     {
        RECTS_CLIP_TO_RECT(c_tmp.x, c_tmp.y, c_tmp.w, c_tmp.h,
                           c_bkp.x, c_bkp.y, c_bkp.w, c_bkp.h);
        if (_is_empty_clip(c_tmp))
           return;
     }

   x = MIN(x0, x1);
   y = MIN(y0, y1);
   w = MAX(x0, x1) - x + 1;
   h = MAX(y0, y1) - y + 1;

   RECTS_CLIP_TO_RECT(c_tmp.x, c_tmp.y, c_tmp.w, c_tmp.h, x, y, w, h);
   if (_is_empty_clip(c_tmp))
      return;

   /* Check if the line doesn't cross the clip area */
   if (x0 < c_tmp.x && x1 < c_tmp.x)
      return;
   if (x0 >= c_tmp.x + c_tmp.w && x1 >= c_tmp.x + c_tmp.w)
      return;
   if (y0 < c_tmp.y && y1 < c_tmp.y)
      return;
   if (y0 >= c_tmp.y + c_tmp.h && y1 >= c_tmp.y + c_tmp.h)
      return;

   dc->clip = c_tmp;
   dx = x1 - x0;
   dy = y1 - y0;

   if (dx < 0)
     {
        int t;

        t = x0;
        x0 = x1;
        x1 = t;

        t = y0;
        y0 = y1;
        y1 = t;
     }

   if (dx == 0 && dy == 0)
      _soft8_line_point(dst, dc, x0, y0);
   else if (dx == 0)
      _soft8_line_vert(dst, dc, x0, y0, y1);
   else if (dy == 0)
      _soft8_line_horiz(dst, dc, x0, x1, y0);
   else if (dy == dx || dy == -dx)
      _soft8_line_45deg(dst, dc, x0, y0, x1, y1);
   else
      _soft8_line_aliased(dst, dc, x0, y0, x1, y1);

   /* restore clip info */
   dc->clip = c_bkp;
}
