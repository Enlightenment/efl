//#undef SCALE_USING_MMX
{
   if (smooth)
     {
        for (y = ystart; y <= yend; y++)
          {
             int x, w, ww;
             FPc u, v, u2, v2, ud, vd, dv;
             DATA32 *d, *s;
#ifdef COLMUL
             FPc cv, cd; // col
# ifdef SCALE_USING_MMX
             FPc cc;
#endif //SCALE_USING_MMX
             DATA32 c1, c2; // col
#endif //COLMUL
             Line *line;
#ifdef SCALE_USING_MMX
             pxor_r2r(mm0, mm0);
             MOV_A2R(ALPHA_255, mm5)
#endif //SCALE_USING_MMX
             line = &(spans[y - ystart]);
             for (i = 0; i < 2; i++)
               {
                  Span *span;
                  span = &(line->span[i]);

                  //The polygon shape won't be completed type
                  if (span->x[0] < 0) break;

                  x = span->x[0];

                  w = (span->x[1] - x);
                  if (w <= 0) continue;

                  dv = (span->o2 - span->o1);
                  if (dv <= 0) continue;

                  ww = w;

                  //correct elaborate u point
                  u = span->u[0] << FPI;
                  if (u < 0) u = 0;
                  else if (u > swp) u = swp;
                  u2 = span->u[1] << FPI;
                  if (u2 < 0) u2 = 0;
                  else if (u2 > swp) u2 = swp;
                  ud = (u2 - u) / w;
                  ud = ((long long)ud * (w << FP)) / dv;
                  u -= (ud * (span->o1 - (span->x[0] << FP))) / FP1;
                  if (ud < 0) u += ud;
                  if (u < 0) u = 0;
                  else if (u >= swp) u = swp - 1;

                  //correct elaborate v point
                  v = span->v[0] << FPI;
                  if (v < 0) v = 0;
                  else if (v > shp) v = shp;
                  v2 = span->v[1] << FPI;
                  if (v2 < 0) v2 = 0;
                  else if (v2 > shp) v2 = shp;
                  vd = (v2 - v) / w;
                  vd = ((long long)vd * (w << FP)) / dv;
                  v -= (vd * (span->o1 - (span->x[0] << FP))) / FP1;
                  if (vd < 0) v += vd;
                  if (v < 0) v = 0;
                  else if (v >= shp) v = shp - 1;

                  if (direct)
                    d = dst->image.data + (y * dst->cache_entry.w) + x;
                  else
                    d = buf;
#define SMOOTH 1
#ifdef COLMUL
                  c1 = span->col[0]; // col
                  c2 = span->col[1]; // col
                  cv = 0; // col
                  cd = (255 << 16) / w; // col

                  if (c1 == c2)
                    {
                       if (c1 == 0xffffffff)
                         {
#endif //COLMUL
#define COLSAME 1
#include "evas_map_image_loop.c"
#undef COLSAME
#ifdef COLMUL
                         }
                       else if ((c1 == 0x0000ff) && (!src->cache_entry.flags.alpha))
                         {
                            // all black line
# define COLBLACK 1
# define COLSAME 1
# include "evas_map_image_loop.c"
# undef COLSAME
# undef COLBLACK
                         }
                       else if (c1 == 0x000000)
                         {
                            // skip span
                         }
                       else
                         {
                            // generic loop
# define COLSAME 1
# include "evas_map_image_loop.c"
# undef COLSAME
                         }
                    }
                  else
                    {
# include "evas_map_image_loop.c"
                    }
#endif //COLMUL
                  if (!direct)
                    {
                       d = dst->image.data;
                       d += (y * dst->cache_entry.w) + x;
                       if (!mask_ie)
                         func(buf, NULL, mul_col, d, w);
                       else
                         {
                            DATA8 *mask = mask_ie->image.data8
                               + (y - mask_y) * mask_ie->cache_entry.w
                               + (x - mask_x);
                            if (mul_col != 0xffffffff) func2(buf, NULL, mul_col, buf, w);
                            func(buf, mask, 0, d, w);
                         }
                    }
               }
          }
     }
   else
     {
        for (y = ystart; y <= yend; y++)
          {
             int x, w, ww;
             FPc u, v, u2, v2, ud, vd;
             DATA32 *d, *s;
#ifdef COLMUL
             FPc cv, cd; // col
             DATA32 c1, c2; // col
#endif //COLMUL
             Line *line;
             line = &(spans[y - ystart]);
             for (i = 0; i < 2; i++)
               {
                  Span *span;
                  span = &(line->span[i]);

                  //The polygon shape won't be completed type
                  if (span->x[0] < 0) break;

                  x = span->x[0];

                  w = (span->x[1] - x);
                  if (w <= 0) continue;

                  ww = w;

                  //correct elaborate u point
                  u = span->u[0] << FPI;
                  if (u < 0) u = 0;
                  else if (u > swp) u = swp;
                  u2 = span->u[1] << FPI;
                  if (u2 < 0) u2 = 0;
                  else if (u2 > swp) u2 = swp;
                  ud = (u2 - u) / w;
                  if (ud < 0) u += ud;
                  if (u < 0) u = 0;
                  else if (u >= swp) u = swp - 1;

                  //correct elaborate v point
                  v = span->v[0] << FPI;
                  if (v < 0) v = 0;
                  else if (v > shp) v = shp;
                  v2 = span->v[1] << FPI;
                  if (v2 < 0) v2 = 0;
                  else if (v2 > shp) v2 = shp;
                  vd = (v2 - v) / w;
                  if (vd < 0) v += vd;
                  if (v < 0) v = 0;
                  else if (v >= shp) v = shp - 1;

                  if (direct)
                    d = dst->image.data + (y * dst->cache_entry.w) + x;
                  else
                    d = buf;
#undef SMOOTH
#ifdef COLMUL
                  c1 = span->col[0]; // col
                  c2 = span->col[1]; // col
                  cv = 0; // col
                  cd = (255 << 16) / w; // col

                  if (c1 == c2)
                    {
                       if (c1 == 0xffffffff)
                         {
#endif //COLMUL
#define COLSAME 1
#include "evas_map_image_loop.c"
#undef COLSAME
#ifdef COLMUL
                         }
                       else if ((c1 == 0x0000ff) && (!src->cache_entry.flags.alpha))
                         {
                            // all black line
# define COLBLACK 1
# define COLSAME 1
# include "evas_map_image_loop.c"
# undef COLSAME
# undef COLBLACK
                         }
                       else if (c1 == 0x000000)
                         {
                            // skip span
                         }
                       else
                         {
                            // generic loop
# define COLSAME 1
# include "evas_map_image_loop.c"
# undef COLSAME
                         }
                    }
                  else
                    {
                       // generic loop
# include "evas_map_image_loop.c"
                    }
#endif //COLMUL
                  if (!direct)
                    {
                       d = dst->image.data;
                       d += (y * dst->cache_entry.w) + x;
                       if (!mask_ie)
                         func(buf, NULL, mul_col, d, w);
                       else
                         {
                            DATA8 *mask = mask_ie->image.data8
                               + (y - mask_y) * mask_ie->cache_entry.w
                               + (x - mask_x);
                            if (mul_col != 0xffffffff) func2(buf, NULL, mul_col, buf, w);
                            func(buf, mask, 0, d, w);
                         }
                    }
               }
          }
     }
}
