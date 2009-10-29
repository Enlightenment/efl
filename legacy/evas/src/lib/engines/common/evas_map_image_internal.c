/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
// 66.74 % of time
static void
FUNC_NAME(RGBA_Image *src, RGBA_Image *dst,
          RGBA_Draw_Context *dc,
          RGBA_Map_Point *p, 
          int smooth, int level)
{
   int i;
   int c, cx, cy, cw, ch;
   int ytop, ybottom, ystart, yend, y, sw, sh, shp, swp, direct;
   Line *spans;
   DATA32 *buf, *sp;
   RGBA_Gfx_Func func;

   // get the clip
   c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
   if (!c)
     {
        cx = 0;
        cy = 0;
        cw = dst->cache_entry.w;
        ch = dst->cache_entry.h;
     }
   
   // find y yop line and y bottom line
   ytop = p[0].y;
   for (i = 1; i < 4; i++)
     {
        if (p[i].y < ytop) ytop = p[i].y;
     }
   ybottom = p[0].y;
   for (i = 1; i < 4; i++)
     {
        if (p[i].y > ybottom) ybottom = p[i].y;
     }
   
   // convert to screen space from fixed point
   ytop = ytop >> FP;
   ybottom = ybottom >> FP;
   
   // if its outside the clip vertical bounds - don't bother
   if ((ytop >= (cy + ch)) || (ybottom < cy)) return;
   
   // limit to the clip vertical bounds
   if (ytop < cy) ystart = cy;
   else ystart = ytop;
   if (ybottom >= (cy + ch)) yend = (cy + ch) - 1;
   else yend = ybottom;

   // get some source image information
   sp = src->image.data;
   sw = src->cache_entry.w;
   swp = sw << (FP + FPI);
   shp = src->cache_entry.h << (FP + FPI);

   // limit u,v coords of points to be within the source image
   for (i = 0; i < 4; i++)
     {
        if (p[i].u < 0) p[i].u = 0;
        else if (p[i].u > (src->cache_entry.w << FP))
          p[i].u = src->cache_entry.w << FP;
        
        if (p[i].v < 0) p[i].v = 0;
        else if (p[i].v > (src->cache_entry.h << FP))
          p[i].v = src->cache_entry.h << FP;
     }
   
   // allocate some spans to hold out span list
   spans = alloca((yend - ystart + 1) * sizeof(Line));
   if (!spans) return;
   memset(spans, 0, (yend - ystart + 1) * sizeof(Line));

   // calculate the spans list
   _calc_spans(p, spans, ystart, yend, cx, cy, cw, ch);
   
   // walk through spans and render
   
   // if operation is solid, bypass buf and draw func and draw direct to dst
   direct = 0;
   if ((!src->cache_entry.flags.alpha) && (!dst->cache_entry.flags.alpha) &&
       (!dc->mul.use))
     direct = 1;
   else
     {
        buf = alloca(cw * sizeof(DATA32));
        if (!buf) return;
        
        if (dc->mul.use)
          func = evas_common_gfx_func_composite_pixel_color_span_get(src, dc->mul.col, dst, cw, dc->render_op);
        else
          func = evas_common_gfx_func_composite_pixel_span_get(src, dst, cw, dc->render_op);
     }
   if (smooth)
     {
        for (y = ystart; y <= yend; y++)
          {
             int x, w, ww;
             FPc u, v, ud, vd, dv;
             DATA32 *d, *s, *so[4], val1, val2;
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
                       
                       while (ww > 0)
                         {
                            FPc u1, v1, u2, v2;
                            FPc rv, ru;
                            DATA32 val1, val2, val3, val4;
                            
                            u1 = u;
                            if (u1 < 0) u1 = 0;
                            else if (u1 >= swp) u1 = swp - 1;
                            
                            v1 = v;
                            if (v1 < 0) v1 = 0;
                            else if (v1 >= shp) v1 = shp - 1;
                            
                            u2 = u1 + FPFPI1;
                            if (u2 >= swp) u2 = swp - 1;
                            
                            v2 = v1 + FPFPI1;
                            if (v2 >= shp) v2 = shp - 1;
                            
                            ru = (u >> (FP + FPI - 8)) & 0xff;
                            rv = (v >> (FP + FPI - 8)) & 0xff;
                            
                            s = sp + ((v1 >> (FP + FPI)) * sw) + 
                              (u1 >> (FP + FPI));
                            val1 = *s;
                            s = sp + ((v1 >> (FP + FPI)) * sw) + 
                              (u2 >> (FP + FPI));
                            val2 = *s;
                            
                            s = sp + ((v2 >> (FP + FPI)) * sw) + 
                              (u1 >> (FP + FPI));
                            val3 = *s;
                            s = sp + ((v2 >> (FP + FPI)) * sw) + 
                              (u2 >> (FP + FPI));
                            val4 = *s;
#ifdef SCALE_USING_MMX
                            MOV_A2R(rv, mm4);
                            MOV_A2R(ru, mm6);
                            MOV_P2R(val1, mm1, mm0);
                            if (val1 | val2)
                              {
                                 MOV_P2R(val2, mm2, mm0);
                                 INTERP_256_R2R(mm6, mm2, mm1, mm5);
                              }
                            MOV_P2R(val3, mm2, mm0);
                            if (val3 | val4)
                              {
                                 MOV_P2R(val4, mm3, mm0);
                                 INTERP_256_R2R(mm6, mm3, mm2, mm5);
                              }
                            INTERP_256_R2R(mm4, mm2, mm1, mm5);
                            MOV_R2P(mm1, *d, mm0);
                            d++;
#else
                            val1 = INTERP_256(ru, val2, val1);
                            val3 = INTERP_256(ru, val4, val3);
                            *d++ = INTERP_256(rv, val3, val1);
#endif                            
                            u += ud;
                            v += vd;
                            ww--;
                         }
                       
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
                       
                       while (ww > 0)
                         {
                            s = sp + ((v >> (FP + FPI)) * sw) + 
                              (u >> (FP + FPI));
                            *d++ = *s;
                            u += ud;
                            v += vd;
                            ww--;
                         }
                       
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
