/*
  * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
  */
//#undef SCALE_USING_MMX
{
   if (smooth)
     {
        for (y = ystart; y <= yend; y++)
          {
             int x, w, ww;
             FPc u, v, ud, vd, dv;
             DATA32 *d, *s, *so[4], val1, val2;
#ifdef COLMUL             
             FPc cv, cd, cc; // col
             DATA32 c1, c2; // col
#endif             
             Line *line;
             
#ifdef SCALE_USING_MMX
             pxor_r2r(mm0, mm0);
             MOV_A2R(ALPHA_255, mm5)
#endif
               
             line = &(spans[y - ystart]);
             for (i = 0; i < 2; i++)
               {
                  Span *span;
                  
                  span = &(line->span[i]);
                  if (span->x1 >= 0)
                    {
                       long long tl;
                       
                       x = span->x1;
                       w = (span->x2 - x);
                       if (w <= 0) continue;
                       dv = (span->o2 - span->o1);
                       if (dv <= 0) continue;
                       
                       ww = w;
                       u = span->u[0] << FPI;
                       v = span->v[0] << FPI;
                       ud = ((span->u[1] << FPI) - u) / w;
                       vd = ((span->v[1] << FPI) - v) / w;
                       tl = (long long)ud * (w << FP);
                       tl = tl / dv;
                       ud = tl;
                       u -= (ud * (span->o1 - (span->x1 << FP))) / FP1;
                       
                       tl = (long long)vd * (w << FP);
                       tl = tl / dv;
                       vd = tl;
                       v -= (vd * (span->o1 - (span->x1 << FP))) / FP1;
                       
                       if (ud < 0) u -= 1;
                       if (vd < 0) v -= 1;
                       
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
#endif                         
#include "evas_map_image_loop.c"
#ifdef COLMUL             
                              }
                            else if ((c1 == 0x0000ff) && (!src->cache_entry.flags.alpha))
                              {
                                 // all black line
# define COLBLACK 1
# include "evas_map_image_loop.c"
# undef COLBLACK                                 
                              }
                            else if (c1 == 0x000000)
                              {
                                 // skip span
                              }
                            else
                              {
                                 // generic loop
# include "evas_map_image_loop.c"
                              }
                         }
                       else
                         {
# include "evas_map_image_loop.c"
                         }
#endif                         
                       if (!direct)
                         {
                            d = dst->image.data;
                            d += (y * dst->cache_entry.w) + x;
                            func(buf, NULL, dc->mul.col, d, w);
                         }
                    }
                  else break;
               }
          }
     }
   else
     {
        for (y = ystart; y <= yend; y++)
          {
             int x, w, ww;
             FPc u, v, ud, vd;
             DATA32 *d, *s;
#ifdef COLMUL
             FPc cv, cd, cc; // col
             DATA32 c1, c2; // col
#endif             
             Line *line;
             
             line = &(spans[y - ystart]);
             for (i = 0; i < 2; i++)
               {
                  Span *span;
                  
                  span = &(line->span[i]);
                  if (span->x1 >= 0)
                    {
                       x = span->x1;
                       w = (span->x2 - x);
                       
                       if (w <= 0) continue;
                       ww = w;
                       u = span->u[0] << FPI;
                       v = span->v[0] << FPI;
                       ud = ((span->u[1] << FPI) - u) / w;
                       vd = ((span->v[1] << FPI) - v) / w;
                       if (ud < 0) u -= 1;
                       if (vd < 0) v -= 1;
                       
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
#endif                                 
#include "evas_map_image_loop.c"
#ifdef COLMUL
                              }
                            else if ((c1 == 0x0000ff) && (!src->cache_entry.flags.alpha))
                              {
                                 // all black line
# define COLBLACK 1
# include "evas_map_image_loop.c"
# undef COLBLACK                                 
                              }
                            else if (c1 == 0x000000)
                              {
                                 // skip span
                              }
                            else
                              {
                                 // generic loop
# include "evas_map_image_loop.c"
                              }
                         }
                       else
                         {
                            // generic loop
# include "evas_map_image_loop.c"
                         }
#endif                       
                       if (!direct)
                         {
                            d = dst->image.data;
                            d += (y * dst->cache_entry.w) + x;
                            func(buf, NULL, dc->mul.col, d, w);
                         }
                    }
                  else break;
               }
          }
     }
}
