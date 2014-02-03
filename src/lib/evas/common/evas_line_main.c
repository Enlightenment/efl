#include "evas_common_private.h"
#include "evas_blend_private.h"

#define IN_RANGE(x, y, w, h) \
  ( x > 0 && y > 0 &&((unsigned)(x) < (unsigned)(w)) && ((unsigned)(y) < (unsigned)(h)) )

#define IN_RECT(x, y, rx, ry, rw, rh)                   \
  ( ((unsigned)((x) - (rx)) < (unsigned)(rw)) &&        \
    ((unsigned)((y) - (ry)) < (unsigned)(rh)) )

#define EXCHANGE_POINTS(x0, y0, x1, y1)         \
  {                                             \
     int _tmp = y0;                             \
                                                \
     y0 = y1;                                   \
     y1 = _tmp;                                 \
                                                \
     _tmp = x0;                                 \
     x0 = x1;                                   \
     x1 = _tmp;                                 \
  }

EAPI void
evas_common_line_init(void)
{
}

static void
_evas_draw_point(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y)
{
   RGBA_Gfx_Pt_Func pfunc;

   if (!dst->image.data) return;
   if (!IN_RANGE(x, y, dst->cache_entry.w, dst->cache_entry.h))
	return;
   if ((dc->clip.use) && (!IN_RECT(x, y, dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h)))
	return;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE   
   pixman_op_t op = PIXMAN_OP_SRC;
   
   if (dc->render_op == _EVAS_RENDER_BLEND)
     op = PIXMAN_OP_OVER;

   if ((dst->pixman.im) && (dc->col.pixman_color_image))
     pixman_image_composite(op, dc->col.pixman_color_image, NULL, 
                            dst->pixman.im, x, y, 0, 0, x, y, 1, 1);
   else
# endif     
#endif
     {
        pfunc = evas_common_gfx_func_composite_color_pt_get(dc->col.col, dst, dc->render_op);
        if (pfunc)
          pfunc(0, 255, dc->col.col, dst->image.data + (dst->cache_entry.w * y) + x);
     }
}

EAPI void
evas_common_line_point_draw(RGBA_Image *dst, int clip_x, int clip_y, int clip_w, int clip_h, DATA32 color, int render_op, int x, int y)
{
   Eina_Bool no_cuse;
   RGBA_Gfx_Pt_Func pfunc;

   if (!dst->image.data) return;
   no_cuse = ((clip_x == 0) && (clip_y == 0) &&
              ((clip_w == (int)dst->cache_entry.w) &&
               (clip_h == (int)dst->cache_entry.h)));

   if (!IN_RANGE(x, y, dst->cache_entry.w, dst->cache_entry.h)) return;
   if ((!no_cuse) && (!IN_RECT(x, y, clip_x, clip_y, clip_w, clip_h)))
     return;

   pfunc = evas_common_gfx_func_composite_color_pt_get(color, dst, render_op);
   if (pfunc)
     pfunc(0, 255, color, dst->image.data + (dst->cache_entry.w * y) + x);
}

/*
   these functions use the dc->clip data as bounding
   data. they assume that such data has already been cut
   back to lie in the dst image rect and the object's
   (line) bounding rect.
*/
static void
_evas_draw_simple_line(RGBA_Image *dst, RGBA_Draw_Context *dc, int x0, int y0, int x1, int y1)
{
   int     dx, dy, len, lx, ty, rx, by;
   int     clx, cly, clw, clh;
   int     dstw;
   DATA32  *p, color;
   RGBA_Gfx_Pt_Func pfunc;
   RGBA_Gfx_Func    sfunc;

#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
   pixman_op_t op = PIXMAN_OP_SRC; // _EVAS_RENDER_COPY
   if (dc->render_op == _EVAS_RENDER_BLEND)
     op = PIXMAN_OP_OVER;
# endif
#endif

   dstw = dst->cache_entry.w;
   color = dc->col.col;

   if (y0 > y1)
      EXCHANGE_POINTS(x0, y0, x1, y1)
   if (x0 > x1)
      EXCHANGE_POINTS(x0, y0, x1, y1)

   dx = x1 - x0;
   dy = y1 - y0;

   clx = dc->clip.x;
   cly = dc->clip.y;
   clw = dc->clip.w;
   clh = dc->clip.h;

   lx = clx;
   rx = clx + clw - 1;
   ty = cly;
   by = cly + clh - 1;

   if (dy == 0)
     {
        if ((y0 >= ty) && (y0 <= by))
          {
             if (dx < 0)
               {
                  int  tmp = x1;

                  x1 = x0;
                  x0 = tmp;
               }

             if (x0 < lx) x0 = lx;
             if (x1 > rx) x1 = rx;

             len = x1 - x0 + 1;
             p = dst->image.data + (dstw * y0) + x0;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE                  
             if ((dst->pixman.im) && (dc->col.pixman_color_image))
               pixman_image_composite(op, dc->col.pixman_color_image,
                                      NULL, dst->pixman.im,
                                      x0, y0, 0, 0, x0, y0, len, 1);
             else
# endif
#endif
               {
                  sfunc = evas_common_gfx_func_composite_color_span_get(color, dst, len, dc->render_op);
                  if (sfunc)
                    sfunc(NULL, NULL, color, p, len);
               }
          }
        return;
     }

   pfunc = evas_common_gfx_func_composite_color_pt_get(color, dst, dc->render_op);
   if (!pfunc) return;

   if (dx == 0)
     {
        if ((x0 >= lx) && (x0 <= rx))
          {
             if (y0 < ty) y0 = ty;
             if (y1 > by) y1 = by;

             len = y1 - y0 + 1;
             p = dst->image.data + (dstw * y0) + x0;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE             
             if ((dst->pixman.im) && (dc->col.pixman_color_image))
               pixman_image_composite(op, dc->col.pixman_color_image,
                                      NULL, dst->pixman.im,
                                      x0, y0, 0, 0, x0, y0, 1, len);
             else
# endif
#endif
               {
                  while (len--)
                    {
                       pfunc(0, 255, color, p);
                       p += dstw;
                    }
               }
          }
        return;
     }

   if ((dy == dx) || (dy == -dx))
     {
        int   p0_in, p1_in;

        p0_in = (IN_RECT(x0, y0, clx, cly, clw, clh) ? 1 : 0);
        p1_in = (IN_RECT(x1, y1, clx, cly, clw, clh) ? 1 : 0);

        if (dy > 0)
          {
             if (!p0_in)
               {
                  x0 = x0 + (ty - y0);
                  y0 = ty;
                  if (x0 > rx) return;
                  if (x0 < lx)
                    {
                       y0 = y0 + (lx - x0);
                       x0 = lx;
                       if ((y0 < ty) || (y0 > by)) return;
                    }
               }
             if (!p1_in)
               {
                  x1 = x0 + (by - y0);
                  y1 = by;
                  if (x1 < lx) return;
                  if (x1 > rx)
                    {
                       y1 = y0 + (rx - x0);
                       x1 = rx;
                       if ((y1 < ty) || (y1 > by)) return;
                    }
               }
          }
        else
          {
             if (!p0_in)
               {
                  x0 = x0 - (by - y0);
                  y0 = by;
                  if (x0 > rx) return;
                  if (x0 < lx)
                    {
                       y0 = y0 - (lx - x0);
                       x0 = lx;
                       if ((y0 < ty) || (y0 > by)) return;
                    }
               }
             if (!p1_in)
               {
                  x1 = x0 - (ty - y0);
                  y1 = ty;
                  if (x1 < lx) return;
                  if (x1 > rx)
                    {
                       y1 = y0 - (rx - x0);
                       x1 = rx;
                       if ((y1 < ty) || (y1 > by)) return;
                    }
               }
          }
        if (y1 > y0)
          {
             p = dst->image.data + (dstw * y0) + x0;
             len = y1 - y0 + 1;
             if (dx > 0)  dstw++;
             else  dstw--;
          }
        else
          {
             len = y0 - y1 + 1;
             p = dst->image.data + (dstw * y1) + x1;
             if (dx > 0)  dstw--;
             else  dstw++;
          }
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE        
        int pixman_x_position = x0;
        int pixman_y_position = y0;
        int x_unit = dstw - dst->cache_entry.w;
# endif
#endif
        while (len--)
          {
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE                  
             if ((dst->pixman.im) && (dc->col.pixman_color_image))
               pixman_image_composite(op, dc->col.pixman_color_image,
                                      NULL, dst->pixman.im,
                                      pixman_x_position,
                                      pixman_y_position,
                                      0, 0, pixman_x_position,
                                      pixman_y_position, 1, 1);
             else
# endif
#endif
               pfunc(0, 255, color, p);
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE             
             pixman_x_position += x_unit;
             pixman_y_position += 1;
# endif
#endif
             p += dstw;
          }
     }
}

#define SETUP_LINE_SHALLOW                                              \
  if (x0 > x1)                                                          \
    {									\
       EXCHANGE_POINTS(x0, y0, x1, y1);                                 \
       dx = -dx;							\
       dy = -dy;							\
    }									\
                                                                        \
  px = x0;								\
  py = y0;								\
                                                                        \
  p0_in = (IN_RANGE(x0 , y0 , clw, clh) ? 1 : 0);                       \
  p1_in = (IN_RANGE(x1 , y1 , clw, clh) ? 1 : 0);                       \
                                                                        \
  dely = 1;								\
  dh = dstw;								\
  if (dy < 0)								\
    {									\
       dely = -1;							\
       dh = -dstw;							\
    }									\
                                                                        \
  dyy = ((dy) << 16) / (dx);						\
                                                                        \
  if (!p0_in)								\
    {									\
       dxx = ((dx) << 16) / (dy);					\
       if (px < 0)							\
         {								\
            x = -px;  px = 0;						\
            yy = x * dyy;						\
            y = yy >> 16;						\
            if (!a_a)							\
              y += (yy - (y << 16)) >> 15;                              \
            py += y;							\
            if ((dely > 0) && (py >= clh))                              \
              return;							\
            else if ((dely < 0) && (py < -1))                           \
              return;							\
         }								\
                                                                        \
       y = 0;								\
       if ((dely > 0) && (py < 0))                                      \
         y = (-1 - py);                                                 \
       else if ((dely < 0) && (py >= clh))                              \
         y = (clh - 1 - py);                                            \
                                                                        \
       xx = y * dxx;							\
       x = xx >> 16;							\
       if (!a_a)							\
         x += (xx - (x << 16)) >> 15;                                   \
       px += x;								\
       if (px >= clw) return;                                           \
                                                                        \
       yy = x * dyy;							\
       y = yy >> 16;							\
       if (!a_a)							\
         y += (yy - (y << 16)) >> 15;                                   \
       py += y;                                                         \
       if ((dely > 0) && (py >= clh))                                   \
         return;							\
       else if ((dely < 0) && (py < -1))                                \
         return;							\
    }									\
                                                                        \
  p = data + (dstw * py) + px;                                          \
                                                                        \
  x = px - x0;                                                          \
  yy = x * dyy;                                                         \
  prev_y = (yy >> 16);                                                  \
                                                                        \
  rx = MIN(x1 + 1, clw);						\
  by = clh - 1;


#define SETUP_LINE_STEEP                                                \
  if (y0 > y1)								\
    {									\
       EXCHANGE_POINTS(x0, y0, x1, y1);                                 \
       dx = -dx;                                                        \
       dy = -dy;                                                        \
    }									\
                                                                        \
  px = x0;                                                              \
  py = y0;                                                              \
                                                                        \
  p0_in = (IN_RANGE(x0 , y0 , clw, clh) ? 1 : 0);                       \
  p1_in = (IN_RANGE(x1 , y1 , clw, clh) ? 1 : 0);                       \
                                                                        \
  delx = 1;								\
  if (dx < 0)								\
    delx = -1;								\
                                                                        \
  dxx = ((dx) << 16) / (dy);						\
                                                                        \
  if (!p0_in)								\
    {									\
       dyy = ((dy) << 16) / (dx);                                       \
                                                                        \
       if (py < 0)                                                      \
         {                                                              \
	   y = -py;  py = 0;						\
	   xx = y * dxx;                                                \
	   x = xx >> 16;                                                \
	   if (!a_a)							\
             x += (xx - (x << 16)) >> 15;                               \
	   px += x;                                                     \
	   if ((delx > 0) && (px >= clw))				\
             return;							\
	   else if ((delx < 0) && (px < -1))                            \
             return;							\
         }                                                              \
                                                                        \
       x = 0;								\
       if ((delx > 0) && (px < -1))					\
         x = (-1 - px);							\
       else if ((delx < 0) && (px >= clw))				\
         x = (clw - 1 - px);						\
                                                                        \
       yy = x * dyy;							\
       y = yy >> 16;							\
       if (!a_a)                                                        \
         y += (yy - (y << 16)) >> 15;                                   \
       py += y;								\
       if (py >= clh) return;						\
                                                                        \
       xx = y * dxx;							\
       x = xx >> 16;							\
       if (!a_a)                                                        \
         x += (xx - (x << 16)) >> 15;                                   \
       px += x;								\
       if ((delx > 0) && (px >= clw))                                   \
         return;                                                        \
       else if ((delx < 0) && (px < -1))				\
         return;                                                        \
    }									\
                                                                        \
  p = data + (dstw * py) + px;                                          \
                                                                        \
  y = py - y0;								\
  xx = y * dxx;								\
  prev_x = (xx >> 16);							\
                                                                        \
  by = MIN(y1 + 1, clh);						\
  rx = clw - 1;

static void
_draw_render_thread_simple_line(RGBA_Image *dst, int clip_x, int clip_y, int clip_w, int clip_h, DATA32 color, int render_op, int x0, int y0, int x1, int y1)
{
   int     dx, dy, len, lx, ty, rx, by;
   int     clx, cly, clw, clh;
   int     dstw;
   DATA32  *p;
   RGBA_Gfx_Pt_Func pfunc;
   RGBA_Gfx_Func    sfunc;

   dstw = dst->cache_entry.w;

   if (y0 > y1)
      EXCHANGE_POINTS(x0, y0, x1, y1)
   if (x0 > x1)
      EXCHANGE_POINTS(x0, y0, x1, y1)

   dx = x1 - x0;
   dy = y1 - y0;

   clx = clip_x;
   cly = clip_y;
   clw = clip_w;
   clh = clip_h;

   lx = clx;
   rx = clx + clw - 1;
   ty = cly;
   by = cly + clh - 1;

   if (dy == 0)
     {
        if ((y0 >= ty) && (y0 <= by))
          {
             if (dx < 0)
               {
                  int  tmp = x1;

                  x1 = x0;
                  x0 = tmp;
               }

             if (x0 < lx) x0 = lx;
             if (x1 > rx) x1 = rx;

             len = x1 - x0 + 1;
             p = dst->image.data + (dstw * y0) + x0;
             sfunc = evas_common_gfx_func_composite_color_span_get(color, dst, len, render_op);
             if (sfunc) sfunc(NULL, NULL, color, p, len);
          }
        return;
     }

   pfunc = evas_common_gfx_func_composite_color_pt_get(color, dst, render_op);
   if (!pfunc) return;

   if (dx == 0)
     {
        if ((x0 >= lx) && (x0 <= rx))
          {
             if (y0 < ty) y0 = ty;
             if (y1 > by) y1 = by;

             len = y1 - y0 + 1;
             p = dst->image.data + (dstw * y0) + x0;
             while (len--)
               {
                  pfunc(0, 255, color, p);
                  p += dstw;
               }
          }
        return;
     }

   if ((dy == dx) || (dy == -dx))
     {
        int   p0_in, p1_in;

        p0_in = (IN_RECT(x0, y0, clx, cly, clw, clh) ? 1 : 0);
        p1_in = (IN_RECT(x1, y1, clx, cly, clw, clh) ? 1 : 0);

        if (dy > 0)
          {
             if (!p0_in)
               {
                  x0 = x0 + (ty - y0);
                  y0 = ty;
                  if (x0 > rx) return;
                  if (x0 < lx)
                    {
                       y0 = y0 + (lx - x0);
                       x0 = lx;
                       if ((y0 < ty) || (y0 > by)) return;
                    }
               }
             if (!p1_in)
               {
                  x1 = x0 + (by - y0);
                  y1 = by;
                  if (x1 < lx) return;
                  if (x1 > rx)
                    {
                       y1 = y0 + (rx - x0);
                       x1 = rx;
                       if ((y1 < ty) || (y1 > by)) return;
                    }
               }
          }
        else
          {
             if (!p0_in)
               {
                  x0 = x0 - (by - y0);
                  y0 = by;
                  if (x0 > rx) return;
                  if (x0 < lx)
                    {
                       y0 = y0 - (lx - x0);
                       x0 = lx;
                       if ((y0 < ty) || (y0 > by)) return;
                    }
               }
             if (!p1_in)
               {
                  x1 = x0 - (ty - y0);
                  y1 = ty;
                  if (x1 < lx) return;
                  if (x1 > rx)
                    {
                       y1 = y0 - (rx - x0);
                       x1 = rx;
                       if ((y1 < ty) || (y1 > by)) return;
                    }
               }
          }
        if (y1 > y0)
          {
             p = dst->image.data + (dstw * y0) + x0;
             len = y1 - y0 + 1;
             if (dx > 0)  dstw++;
             else  dstw--;
          }
        else
          {
             len = y0 - y1 + 1;
             p = dst->image.data + (dstw * y1) + x1;
             if (dx > 0)  dstw--;
             else  dstw++;
          }
        while (len--)
          {
             pfunc(0, 255, color, p);
             p += dstw;
          }
     }
}

#define SETUP_LINE_SHALLOW                                             \
  if (x0 > x1)                                                         \
    {									\
       EXCHANGE_POINTS(x0, y0, x1, y1);                                \
       dx = -dx;							\
       dy = -dy;							\
    }									\
                                                                       \
  px = x0;								\
  py = y0;								\
                                                                       \
  p0_in = (IN_RANGE(x0 , y0 , clw, clh) ? 1 : 0);                      \
  p1_in = (IN_RANGE(x1 , y1 , clw, clh) ? 1 : 0);                      \
                                                                       \
  dely = 1;								\
  dh = dstw;								\
  if (dy < 0)								\
    {									\
       dely = -1;							\
       dh = -dstw;							\
    }									\
                                                                       \
  dyy = ((dy) << 16) / (dx);						\
                                                                       \
  if (!p0_in)								\
    {									\
       dxx = ((dx) << 16) / (dy);					\
       if (px < 0)							\
         {								\
            x = -px;  px = 0;						\
            yy = x * dyy;						\
            y = yy >> 16;						\
            if (!a_a)							\
              y += (yy - (y << 16)) >> 15;                             \
            py += y;							\
            if ((dely > 0) && (py >= clh))                             \
              return;							\
            else if ((dely < 0) && (py < -1))                          \
              return;							\
         }								\
                                                                       \
       y = 0;								\
       if ((dely > 0) && (py < 0))                                     \
         y = (-1 - py);                                                \
       else if ((dely < 0) && (py >= clh))                             \
         y = (clh - 1 - py);                                           \
                                                                       \
       xx = y * dxx;							\
       x = xx >> 16;							\
       if (!a_a)							\
         x += (xx - (x << 16)) >> 15;                                  \
       px += x;							\
       if (px >= clw) return;                                          \
                                                                       \
       yy = x * dyy;							\
       y = yy >> 16;							\
       if (!a_a)							\
         y += (yy - (y << 16)) >> 15;                                  \
       py += y;                                                        \
       if ((dely > 0) && (py >= clh))                                  \
         return;							\
       else if ((dely < 0) && (py < -1))                               \
         return;							\
    }									\
                                                                       \
  p = data + (dstw * py) + px;                                         \
                                                                       \
  x = px - x0;                                                         \
  yy = x * dyy;                                                        \
  prev_y = (yy >> 16);                                                 \
                                                                       \
  rx = MIN(x1 + 1, clw);						\
  by = clh - 1;


#define SETUP_LINE_STEEP                                               \
  if (y0 > y1)								\
    {									\
       EXCHANGE_POINTS(x0, y0, x1, y1);                                \
       dx = -dx;                                                       \
       dy = -dy;                                                       \
    }									\
                                                                       \
  px = x0;                                                             \
  py = y0;                                                             \
                                                                       \
  p0_in = (IN_RANGE(x0 , y0 , clw, clh) ? 1 : 0);                      \
  p1_in = (IN_RANGE(x1 , y1 , clw, clh) ? 1 : 0);                      \
                                                                       \
  delx = 1;								\
  if (dx < 0)								\
    delx = -1;								\
                                                                       \
  dxx = ((dx) << 16) / (dy);						\
                                                                       \
  if (!p0_in)								\
    {									\
       dyy = ((dy) << 16) / (dx);                                      \
                                                                       \
       if (py < 0)                                                     \
         {                                                             \
	   y = -py;  py = 0;						\
	   xx = y * dxx;                                                \
	   x = xx >> 16;                                                \
	   if (!a_a)							\
             x += (xx - (x << 16)) >> 15;                              \
	   px += x;                                                     \
	   if ((delx > 0) && (px >= clw))				\
             return;							\
	   else if ((delx < 0) && (px < -1))                            \
             return;							\
         }                                                             \
                                                                       \
       x = 0;								\
       if ((delx > 0) && (px < -1))					\
         x = (-1 - px);						\
       else if ((delx < 0) && (px >= clw))				\
         x = (clw - 1 - px);						\
                                                                       \
       yy = x * dyy;							\
       y = yy >> 16;							\
       if (!a_a)                                                       \
         y += (yy - (y << 16)) >> 15;                                  \
       py += y;							\
       if (py >= clh) return;						\
                                                                       \
       xx = y * dxx;							\
       x = xx >> 16;							\
       if (!a_a)                                                       \
         x += (xx - (x << 16)) >> 15;                                  \
       px += x;							\
       if ((delx > 0) && (px >= clw))                                  \
         return;                                                       \
       else if ((delx < 0) && (px < -1))				\
         return;                                                       \
    }									\
                                                                       \
  p = data + (dstw * py) + px;                                         \
                                                                       \
  y = py - y0;								\
  xx = y * dxx;							\
  prev_x = (xx >> 16);							\
                                                                       \
  by = MIN(y1 + 1, clh);						\
  rx = clw - 1;

EAPI void
evas_common_line_draw_line(RGBA_Image *dst, int clip_x, int clip_y, int clip_w, int clip_h, DATA32 color, int render_op, int x0, int y0, int x1, int y1)
{
   int     px, py, x, y, prev_x, prev_y;
   int     dx, dy, rx, by, p0_in, p1_in, dh, a_a = 0;
   int     delx, dely, xx, yy, dxx, dyy;
   int     clx, cly, clw, clh;
   int     dstw;
   DATA32  *p, *data;
   RGBA_Gfx_Pt_Func pfunc;

   dx = x1 - x0;
   dy = y1 - y0;

   if ( (dx == 0) || (dy == 0) || (dx == dy) || (dx == -dy) )
     {
	_draw_render_thread_simple_line
          (dst, clip_x, clip_y, clip_w, clip_h,
           color, render_op,
           x0, y0, x1, y1);
	return;
     }

   pfunc = evas_common_gfx_func_composite_color_pt_get(color, dst, render_op);
   if (!pfunc) return;

   clx = clip_x;
   cly = clip_y;
   clw = clip_w;
   clh = clip_h;

   data = dst->image.data;
   dstw = dst->cache_entry.w;

   data += (dstw * cly) + clx;
   x0 -= clx;
   y0 -= cly;
   x1 -= clx;
   y1 -= cly;

   /* shallow: x-parametric */
   if ((dy < dx) || (dy < -dx))
     {
	SETUP_LINE_SHALLOW;

	while (px < rx)
	  {
	    y = (yy >> 16);
	    y += ((yy - (y << 16)) >> 15);
	    if (prev_y != y)
	      {
		prev_y = y;
		p += dh;
		py += dely;
	      }
	    if (!p1_in)
	      {
		if ((py < 0) && (dely < 0)) return;
		if ((py > by) && (dely > 0)) return;
	      }
            if (!p0_in)
              {
                 if (py < 0) goto next_x;
              }
            if (IN_RANGE(px, py, clw, clh)) pfunc(0, 255, color, p);

next_x:
            yy += dyy;
            px++;
            p++;
	  }
	return;
     }

   /* steep: y-parametric */

   SETUP_LINE_STEEP;

   while (py < by)
     {
	x = (xx >> 16);
	x += ((xx - (x << 16)) >> 15);
	if (prev_x != x)
	  {
             prev_x = x;
             px += delx;
             p += delx;
	  }
	if (!p1_in)
	  {
	    if ((px < 0) && (delx < 0)) return;
	    if ((px > rx) && (delx > 0)) return;
	  }
        if (!p0_in)
          {
             if (px < 0) goto next_y;
          }
        if (IN_RANGE(px, py, clw, clh)) pfunc(0, 255, color, p);

next_y:
	xx += dxx;
	py++;
	p += dstw;
     }
}

static void
_evas_draw_line(RGBA_Image *dst, RGBA_Draw_Context *dc, int x0, int y0, int x1, int y1)
{
   int     px, py, x, y, prev_x, prev_y;
   int     dx, dy, rx, by, p0_in, p1_in, dh, a_a = 0;
   int     delx, dely, xx, yy, dxx, dyy;
   int     clx, cly, clw, clh;
   int     dstw;
   DATA32  *p, *data, color;
   RGBA_Gfx_Pt_Func pfunc;

   dx = x1 - x0;
   dy = y1 - y0;

#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
   int pix_x;
   int pix_y;
   int pix_x_unit;
   int pix_y_unit;

   pixman_op_t op = PIXMAN_OP_SRC; // _EVAS_RENDER_COPY
   if (dc->render_op == _EVAS_RENDER_BLEND)
     op = PIXMAN_OP_OVER;
   pix_x = x0;
   pix_y = y0;

   if (dx < 0)
     pix_x_unit = -1;
   else
     pix_x_unit = 1;

   if (dy < 0)
     pix_y_unit = -1;
   else
     pix_y_unit = 1;
# endif
#endif

   if ( (dx == 0) || (dy == 0) || (dx == dy) || (dx == -dy) )
     {
	_evas_draw_simple_line(dst, dc, x0, y0, x1, y1);
	return;
     }

   color = dc->col.col;
   pfunc = evas_common_gfx_func_composite_color_pt_get(color, dst, dc->render_op);
   if (!pfunc) return;

   clx = dc->clip.x;
   cly = dc->clip.y;
   clw = dc->clip.w;
   clh = dc->clip.h;

   data = dst->image.data;
   dstw = dst->cache_entry.w;

   data += (dstw * cly) + clx;
   x0 -= clx;
   y0 -= cly;
   x1 -= clx;
   y1 -= cly;

   /* shallow: x-parametric */
   if ((dy < dx) || (dy < -dx))
     {
	SETUP_LINE_SHALLOW;

	while (px < rx)
	  {
	    y = (yy >> 16);
	    y += ((yy - (y << 16)) >> 15);
	    if (prev_y != y)
	      {
		prev_y = y;
		p += dh;
		py += dely;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
                 pix_y += pix_y_unit;
# endif
#endif
	      }
	    if (!p1_in)
	      {
		if ((py < 0) && (dely < 0)) return;
		if ((py > by) && (dely > 0)) return;
	      }
            if (!p0_in)
              {
                 if (py < 0) goto next_x;
              }
            if (IN_RANGE(px, py, clw, clh))
              {
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
                 if ((dst->pixman.im) && (dc->col.pixman_color_image))
                   pixman_image_composite(op, dc->col.pixman_color_image,
                                          NULL, dst->pixman.im,
                                          pix_x, pix_y, 0, 0,
                                          pix_x, pix_y, 1, 1);
                 else
# endif
#endif
                   pfunc(0, 255, color, p);
              }

next_x:
            yy += dyy;
            px++;
            p++;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
            pix_x += pix_x_unit;
# endif
#endif
	  }
	return;
     }

   /* steep: y-parametric */

   SETUP_LINE_STEEP;

   while (py < by)
     {
	x = (xx >> 16);
	x += ((xx - (x << 16)) >> 15);
	if (prev_x != x)
	  {
             prev_x = x;
             px += delx;
             p += delx;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
             pix_x += pix_x_unit;
# endif
#endif
	  }
	if (!p1_in)
	  {
	    if ((px < 0) && (delx < 0)) return;
	    if ((px > rx) && (delx > 0)) return;
	  }
        if (!p0_in)
          {
             if (px < 0) goto next_y;
          }
        if (IN_RANGE(px, py, clw, clh))
          {
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE                  
             if ((dst->pixman.im) && (dc->col.pixman_color_image))
               pixman_image_composite(op, dc->col.pixman_color_image,
                                      NULL, dst->pixman.im,
                                      pix_x, pix_y, 0, 0,
                                      pix_x, pix_y, 1, 1);
             else
# endif                    
#endif
               pfunc(0, 255, color, p);
          }
next_y:
	xx += dxx;
	py++;
	p += dstw;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE        
        pix_y += pix_y_unit;
# endif        
#endif
     }
}

EAPI void
evas_common_line_draw_line_aa(RGBA_Image *dst, int clip_x, int clip_y, int clip_w, int clip_h, DATA32 color, int render_op, int x0, int y0, int x1, int y1)
{
   int     px, py, x, y, prev_x, prev_y;
   int     dx, dy, rx, by, p0_in, p1_in, dh, a_a = 1;
   int     delx, dely, xx, yy, dxx, dyy;
   int     clx, cly, clw, clh;
   int     dstw;
   DATA32  *p, *data;
   RGBA_Gfx_Pt_Func pfunc;

   dx = x1 - x0;
   dy = y1 - y0;

   if (y0 > y1)
     EXCHANGE_POINTS(x0, y0, x1, y1);

   dx = x1 - x0;
   dy = y1 - y0;

   if ((dx == 0) || (dy == 0) || (dx == dy) || (dx == -dy))
     {
	_draw_render_thread_simple_line
          (dst, clip_x, clip_y, clip_w, clip_h,
           color, render_op,
           x0, y0, x1, y1);
	return;
     }

   pfunc = evas_common_gfx_func_composite_mask_color_pt_get(color, dst, render_op);
   if (!pfunc) return;

   clx = clip_x;
   cly = clip_y;
   clw = clip_w;
   clh = clip_h;

   data = evas_cache_image_pixels(&dst->cache_entry);
   dstw = dst->cache_entry.w;

   data += (dstw * cly) + clx;
   x0 -= clx;
   y0 -= cly;
   x1 -= clx;
   y1 -= cly;

   /* shallow: x-parametric */
   if ((dy < dx) || (dy < -dx))
     {
	SETUP_LINE_SHALLOW;

	while (px < rx)
	  {
	    DATA8   aa;

	    y = (yy >> 16);
	    if (prev_y != y)
	      {
                 prev_y = y;
                 p += dh;
                 py += dely;
	      }
	    if (!p1_in)
	      {
		if ((py < 0) && (dely < 0)) return;
		if ((py > by) && (dely > 0)) return;
	      }
            if (!p0_in)
              {
                 if (py < 0) goto next_x;
              }
	    if (px < clw)
	      {
                 aa = ((yy - (y << 16)) >> 8);

                 if ((py) < clh) pfunc(0, 255 - aa, color, p);
                 if ((py + 1) < clh) pfunc(0, aa, color, p + dstw);
              }

next_x:
             yy += dyy;
             px++;
             p++;
	  }
	return;
     }

   /* steep: y-parametric */
   SETUP_LINE_STEEP;

   while (py < by)
     {
	DATA8   aa;

	x = (xx >> 16);
	if (prev_x != x)
	  {
             prev_x = x;
             px += delx;
             p += delx;
	  }
	if (!p1_in)
	  {
             if ((px < 0) && (delx < 0)) return;
             if ((px > rx) && (delx > 0)) return;
	  }
        if (!p0_in)
          {
             if (px < 0) goto next_y;
          }
	if (py < clh)
	  {
             aa = ((xx - (x << 16)) >> 8);
             if ((px) < clw) pfunc(0, 255 - aa, color, p);
             if ((px + 1) < clw) pfunc(0, aa, color, p + 1);
          }

     next_y:
	xx += dxx;
	py++;
	p += dstw;
     }
}

static void
_evas_draw_line_aa(RGBA_Image *dst, RGBA_Draw_Context *dc, int x0, int y0, int x1, int y1)
{
   int     px, py, x, y, prev_x, prev_y;
   int     dx, dy, rx, by, p0_in, p1_in, dh, a_a = 1;
   int     delx, dely, xx, yy, dxx, dyy;
   int     clx, cly, clw, clh;
   int     dstw;
   DATA32  *p, *data, color;
   RGBA_Gfx_Pt_Func pfunc;

   dx = x1 - x0;
   dy = y1 - y0;

#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE   
   int pix_x;
   int pix_y;
   int pix_x_unit;
   int pix_y_unit;

   pixman_image_t *aa_mask_image;
   int alpha_data_buffer;
   
   pixman_op_t op = PIXMAN_OP_SRC; // _EVAS_RENDER_COPY
   if (dc->render_op == _EVAS_RENDER_BLEND)
     op = PIXMAN_OP_OVER;
   pix_x = x0;
   pix_y = y0;

   if (dx < 0)
     pix_x_unit = -1;
   else
     pix_x_unit = 1;

   if (dy < 0)
     pix_y_unit = -1;
   else
     pix_y_unit = 1;
# endif
#endif
   if (y0 > y1)
     EXCHANGE_POINTS(x0, y0, x1, y1);

   dx = x1 - x0;
   dy = y1 - y0;

   if ((dx == 0) || (dy == 0) || (dx == dy) || (dx == -dy))
     {
	_evas_draw_simple_line(dst, dc, x0, y0, x1, y1);
	return;
     }

   color = dc->col.col;
   pfunc = evas_common_gfx_func_composite_mask_color_pt_get(color, dst, dc->render_op);
   if (!pfunc) return;

   clx = dc->clip.x;
   cly = dc->clip.y;
   clw = dc->clip.w;
   clh = dc->clip.h;

   data = evas_cache_image_pixels(&dst->cache_entry);
   dstw = dst->cache_entry.w;

   data += (dstw * cly) + clx;
   x0 -= clx;
   y0 -= cly;
   x1 -= clx;
   y1 -= cly;

   /* shallow: x-parametric */
   if ((dy < dx) || (dy < -dx))
     {
	SETUP_LINE_SHALLOW;

	while (px < rx)
	  {
	    DATA8   aa;

	    y = (yy >> 16);
	    if (prev_y != y)
	      {
                 prev_y = y;
                 p += dh;
                 py += dely;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE                 
                 pix_y += pix_y_unit;
# endif                 
#endif
	      }
	    if (!p1_in)
	      {
		if ((py < 0) && (dely < 0)) return;
		if ((py > by) && (dely > 0)) return;
	      }
            if (!p0_in)
              {
                 if (py < 0) goto next_x;
              }
	    if (px < clw)
	      {
                 aa = ((yy - (y << 16)) >> 8);
                 if ((py) < clh)
                   {
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE                       
                      alpha_data_buffer = 255 - aa;
                      aa_mask_image = pixman_image_create_bits(PIXMAN_a8, 1, 1,
                                                               (uint32_t *)&alpha_data_buffer, 4);
                      
                      if ((dst->pixman.im) && (dc->col.pixman_color_image))
                        pixman_image_composite(op, dc->col.pixman_color_image,
                                               aa_mask_image, dst->pixman.im,
                                               pix_x, pix_y, 0, 0,
                                               pix_x, pix_y, 1, 1);
                      else
# endif
#endif
                        pfunc(0, 255 - aa, color, p);
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE                       
                      pixman_image_unref(aa_mask_image);
# endif
#endif
                    }
                 if ((py + 1) < clh)
                   {
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE                       
                      alpha_data_buffer = aa;
                      aa_mask_image = pixman_image_create_bits(PIXMAN_a8, 1, 1,
                                                               (uint32_t *)&alpha_data_buffer, 4);
                      
                      if ((dst->pixman.im) && (dc->col.pixman_color_image))
                        pixman_image_composite(op, dc->col.pixman_color_image,
                                               aa_mask_image, dst->pixman.im,
                                               pix_x, pix_y + 1, 0, 0,
                                               pix_x, pix_y + 1, 1, 1);
                      else
# endif
#endif
                        pfunc(0, aa, color, p + dstw);
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE                       
                      pixman_image_unref(aa_mask_image);
# endif
#endif
                   }
              }
             
next_x:
             yy += dyy;
             px++;
             p++;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE                       
             pix_x += pix_x_unit;
# endif
#endif
	  }
	return;
     }
   
   /* steep: y-parametric */
   SETUP_LINE_STEEP;

   while (py < by)
     {
	DATA8   aa;
        
	x = (xx >> 16);
	if (prev_x != x)
	  {
             prev_x = x;
             px += delx;
             p += delx;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
             pix_x += pix_x_unit;
# endif
#endif
	  }
	if (!p1_in)
	  {
             if ((px < 0) && (delx < 0)) return;
             if ((px > rx) && (delx > 0)) return;
	  }
        if (!p0_in)
          {
             if (px < 0) goto next_y;
          }
	if (py < clh)
	  {
             aa = ((xx - (x << 16)) >> 8);
             if ((px) < clw)
               {
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
                  alpha_data_buffer = 255 - aa;
                  aa_mask_image = pixman_image_create_bits(PIXMAN_a8, 1, 1, (uint32_t *)&alpha_data_buffer, 4);
                  
                  if ((dst->pixman.im) && (dc->col.pixman_color_image))
                    pixman_image_composite(op, dc->col.pixman_color_image,
                                           aa_mask_image, dst->pixman.im,
                                           pix_x, pix_y, 0, 0,
                                           pix_x, pix_y, 1, 1);
                  else
# endif
#endif
                    pfunc(0, 255 - aa, color, p);
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
                  pixman_image_unref(aa_mask_image);
# endif
#endif
                  
               }
             if ((px + 1) < clw)
               {
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
                  alpha_data_buffer = aa;
                  aa_mask_image = pixman_image_create_bits(PIXMAN_a8, 1, 1, 
                                                           (uint32_t *)&alpha_data_buffer, 4);

                  if ((dst->pixman.im) && (dc->col.pixman_color_image))
                       pixman_image_composite(op, dc->col.pixman_color_image,
                                              aa_mask_image, dst->pixman.im,
                                              pix_x + 1, pix_y, 0, 0,
                                              pix_x + 1, pix_y, 1, 1);
                  else
# endif
#endif
                    pfunc(0, aa, color, p + 1);
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
                  pixman_image_unref(aa_mask_image);
# endif
#endif
               }
          }
     next_y:
	xx += dxx;
	py++;
	p += dstw;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_LINE
        pix_y += pix_y_unit;
# endif
#endif
     }
}

EAPI void
evas_common_line_draw_cb(RGBA_Image *dst, RGBA_Draw_Context *dc, int x0, int y0, int x1, int y1, Evas_Common_Line_Draw_Cb cb)
{
   int  x, y, w, h;
   int  clx, cly, clw, clh;
   int  cuse, cx, cy, cw, ch;

   /* No cutout ? FIXME ? */

   if ((x0 == x1) && (y0 == y1))
     {
	_evas_draw_point(dst, dc, x0, y0);
	return;
     }

   clx = cly = 0;
   clw = dst->cache_entry.w;
   clh = dst->cache_entry.h;

   /* save out clip info */
   cuse = dc->clip.use;
   cx = dc->clip.x;
   cy = dc->clip.y;
   cw = dc->clip.w;
   ch = dc->clip.h;

   if (cuse)
     {
	RECTS_CLIP_TO_RECT(clx, cly, clw, clh, cx, cy, cw, ch);
	if ((clw < 1) || (clh < 1))
	   return;
     }

   x = MIN(x0, x1);
   y = MIN(y0, y1);
   w = MAX(x0, x1) - x + 1;
   h = MAX(y0, y1) - y + 1;

   RECTS_CLIP_TO_RECT(clx, cly, clw, clh, x, y, w, h);
   if ((clw < 1) || (clh < 1))
	return;

   dc->clip.use = 1;
   dc->clip.x = clx;
   dc->clip.y = cly;
   dc->clip.w = clw;
   dc->clip.h = clh;

   cb(dst, dc, x0, y0, x1, y1);

   /* restore clip info */
   dc->clip.use = cuse;
   dc->clip.x = cx;
   dc->clip.y = cy;
   dc->clip.w = cw;
   dc->clip.h = ch;
}

EAPI void
evas_common_line_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x0, int y0, int x1, int y1)
{
   Evas_Common_Line_Draw_Cb cb;

   if (dc->anti_alias) cb = _evas_draw_line_aa;
   else cb = _evas_draw_line;

   evas_common_line_draw_cb(dst, dc, x0, y0, x1, y1, cb);
}
