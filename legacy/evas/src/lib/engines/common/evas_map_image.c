/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_blend_private.h"

#define FPI 8
#define FPI1 (1 << (FPI))
#define FPIH (1 << (FPI - 1))

#define FPFPI1 (1 << (FP + FPI))

typedef struct _Line Line;
typedef struct _Span Span;

struct _Span
{
   int x1, x2;
   FPc o1, o2;
   FPc  u[2], v[2];
};

struct _Line
{
   Span span[2];
};

static void
evas_common_map4_rgba_internal(RGBA_Image *src, RGBA_Image *dst,
                               RGBA_Draw_Context *dc,
                               RGBA_Map_Point *p, 
                               int smooth, int level);

//extern const DATA8 _evas_dither_44[4][4];
//extern const DATA8 _evas_dither_128128[128][128];
  
static FPc
_interp(int x1, int x2, int p, FPc u1, FPc u2)
{
   FPc u;

   x2 -= x1;
   p -= x1;
   u = u2 - u1;
   u = (u * p) / (x2 + 1);
   return u1 + u;
}

EAPI void
evas_common_map4_rgba(RGBA_Image *src, RGBA_Image *dst,
                      RGBA_Draw_Context *dc,
                      RGBA_Map_Point *p, 
                      int smooth, int level)
{
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch;
   int          i;
   
   if (src->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
     evas_cache_image_load_data(&src->cache_entry);
   evas_common_image_colorspace_normalize(src);
   if (!src->image.data) return;
   if (!dc->cutout.rects)
     {
        evas_common_map4_rgba_internal(src, dst, dc, p, smooth, level);
        return;
     }
   /* save out clip info */
   c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     {
        dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
        return;
     }
   rects = evas_common_draw_context_apply_cutouts(dc);
   for (i = 0; i < rects->active; ++i)
     {
        r = rects->rects + i;
        evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
        evas_common_map4_rgba_internal(src, dst, dc, p, smooth, level);
     }
   evas_common_draw_context_apply_clear_cutouts(rects);
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
}

// 12.63 % of time - this can improve
static void
_calc_spans(RGBA_Map_Point *p, Line *spans, int ystart, int yend, int cx, int cy, int cw, int ch)
{
   int i, y, yp;
   int py[4];
   int edge[4][4], edge_num, swapped, order[4];
   FPc uv[4][2], u, v, x, h, t;
   
#if 1 // maybe faster on x86?
   for (i = 0; i < 4; i++) py[i] = p[i].y >> FP;
# define PY(x) (py[x])   
#else
# define PY(x) (p[x].y >> FP) 
#endif
   
   if ((PY(0) == PY(1)) && (PY(0) == PY(2)) && (PY(0) == PY(3)))
     {
        // all on one line. eg:
        // 
        // |----------|
        // FIXME:
        // find min x point and max x point and span those
     }
   else
     {
        for (y = ystart; y <= yend; y++)
          {
        
             yp = y - ystart;
             edge_num = 0;
             //--------------------
             if ((PY(0) <= y) && (PY(1) > y)) // 0 above, 1, below
               {
                  edge[edge_num][0] = 0;
                  edge[edge_num][1] = 1;
                  edge_num++;
               }
             else if ((PY(1) <= y) && (PY(0) > y)) // 1 above, 0, below
               {
                  edge[edge_num][0] = 1;
                  edge[edge_num][1] = 0;
                  edge_num++;
               }
             //--------------------
             if ((PY(1) <= y) && (PY(2) > y)) // 1 above, 2, below
               {
                  edge[edge_num][0] = 1;
                  edge[edge_num][1] = 2;
                  edge_num++;
               }
             else if ((PY(2) <= y) && (PY(1) > y)) // 2 above, 1, below
               {
                  edge[edge_num][0] = 2;
                  edge[edge_num][1] = 1;
                  edge_num++;
               }
             //--------------------
             if ((PY(2) <= y) && (PY(3) > y)) // 2 above, 3, below
               {
                  edge[edge_num][0] = 2;
                  edge[edge_num][1] = 3;
                  edge_num++;
               }
             else if ((PY(3) <= y) && (PY(2) > y)) // 3 above, 2, below
               {
                  edge[edge_num][0] = 3;
                  edge[edge_num][1] = 2;
                  edge_num++;
               }
             //--------------------
             if ((PY(3) <= y) && (PY(0) > y)) // 3 above, 0, below
               {
                  edge[edge_num][0] = 3;
                  edge[edge_num][1] = 0;
                  edge_num++;
               }
             else if ((PY(0) <= y) && (PY(3) > y)) // 0 above, 3, below
               {
                  edge[edge_num][0] = 0;
                  edge[edge_num][1] = 3;
                  edge_num++;
               }
             // calculate line x points for each edge
             for (i = 0; i < edge_num; i++)
               {
                  int e1 = edge[i][0];
                  int e2 = edge[i][1];

                  h = (p[e2].y - p[e1].y) >> FP; // height of edge
                  t = (((y << FP) + (FP1 - 1)) - p[e1].y) >> FP;
                  x = p[e2].x - p[e1].x;
                  x = p[e1].x + ((x * t) / h);
                  
                  u = p[e2].u - p[e1].u;
                  u = p[e1].u + ((u * t) / h);
                  
                  v = p[e2].v - p[e1].v;
                  v = p[e1].v + ((v * t) / h);
                  
                  uv[i][1] = v;
                  uv[i][0] = u;
                  edge[i][2] = x >> FP;
                  edge[i][3] = x;
                  // also fill in order
                  order[i] = i;
               }
             // sort edges from left to right - bubble. its a small list!
             do
               {
                  swapped = 0;
                  for (i = 0; i < (edge_num - 1); i++)
                    {
                       if (edge[order[i]][2] > edge[order[i + 1]][2])
                         {
                            t = order[i];
                            order[i] = order[i + 1];
                            order[i + 1] = t;
                            swapped = 1;
                         }
                    }
               }
             while (swapped);
             if (edge_num == 2)
               {
                  i = 0;
                  spans[yp].span[i].x1 = edge[order[0]][2];
                  spans[yp].span[i].o1 = edge[order[0]][3];
                  spans[yp].span[i].u[0] = uv[order[0]][0];
                  spans[yp].span[i].v[0] = uv[order[0]][1];
                  spans[yp].span[i].x2 = edge[order[1]][2];
                  spans[yp].span[i].o2 = edge[order[1]][3];
                  spans[yp].span[i].u[1] = uv[order[1]][0];
                  spans[yp].span[i].v[1] = uv[order[1]][1];
                  if ((spans[yp].span[i].x1 >= (cx + cw)) ||
                      (spans[yp].span[i].x2 < cx))
                    {
                       spans[yp].span[i].x1 = -1;
                    }
                  else
                    {
                       if (spans[yp].span[i].x1 < cx)
                         {
                            spans[yp].span[i].u[0] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, 0, 
                                      spans[yp].span[i].u[0],
                                      spans[yp].span[i].u[1]);
                            spans[yp].span[i].v[0] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, 0, 
                                      spans[yp].span[i].v[0],
                                      spans[yp].span[i].v[1]);
                            spans[yp].span[i].x1 = cx;
                            spans[yp].span[i].o1 = cx << FP;
                         }
                       if (spans[yp].span[i].x2 >= (cx + cw))
                         {
                            spans[yp].span[i].u[1] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, (cx + cw),
                                      spans[yp].span[i].u[0],
                                      spans[yp].span[i].u[1]);
                            spans[yp].span[i].v[1] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, (cx + cw),
                                      spans[yp].span[i].v[0],
                                      spans[yp].span[i].v[1]);
                            spans[yp].span[i].x2 = (cx + cw);
                            spans[yp].span[i].o2 = (cx + cw) << FP;
                         }
                       i++;
                       spans[yp].span[i].x1 = -1;
                    }
               }
             else if (edge_num == 4)
               {
                  i = 0;
                  spans[yp].span[i].x1 = edge[order[0]][2];
                  spans[yp].span[i].u[0] = uv[order[0]][0];
                  spans[yp].span[i].v[0] = uv[order[0]][1];
                  spans[yp].span[i].x2 = edge[order[1]][2];
                  spans[yp].span[i].u[1] = uv[order[1]][0];
                  spans[yp].span[i].v[1] = uv[order[1]][1];
                  if ((spans[yp].span[i].x1 >= (cx + cw)) ||
                      (spans[yp].span[i].x2 < cx))
                    {
                       spans[yp].span[i].x1 = -1;
                    }
                  else
                    {
                       if (spans[yp].span[i].x1 < cx)
                         {
                            spans[yp].span[i].u[0] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, 0, 
                                      spans[yp].span[i].u[0],
                                      spans[yp].span[i].u[1]);
                            spans[yp].span[i].v[0] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, 0, 
                                      spans[yp].span[i].v[0],
                                      spans[yp].span[i].v[1]);
                            spans[yp].span[i].x1 = cx;
                            spans[yp].span[i].o1 = cx << FP;
                         }
                       if (spans[yp].span[i].x2 >= (cx + cw))
                         {
                            spans[yp].span[i].u[1] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, (cx + cw),
                                      spans[yp].span[i].u[0],
                                      spans[yp].span[i].u[1]);
                            spans[yp].span[i].v[1] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, (cx + cw),
                                      spans[yp].span[i].v[0],
                                      spans[yp].span[i].v[1]);
                            spans[yp].span[i].x2 = (cx + cw);
                            spans[yp].span[i].o2 = (cx + cw) << FP;
                         }
                       i++;
                    }
                  spans[yp].span[i].x1 = edge[order[2]][2];
                  spans[yp].span[i].u[0] = uv[order[2]][0];
                  spans[yp].span[i].v[0] = uv[order[2]][1];
                  spans[yp].span[i].x2 = edge[order[3]][2];
                  spans[yp].span[i].u[1] = uv[order[3]][0];
                  spans[yp].span[i].v[1] = uv[order[3]][1];
                  if ((spans[yp].span[i].x1 >= (cx + cw)) ||
                      (spans[yp].span[i].x2 < cx))
                    {
                       spans[yp].span[i].x1 = -1;
                    }
                  else
                    {
                       int l = cx;
                       
                       if (i > 0) l = spans[yp].span[i - 1].x2;
                       if (spans[yp].span[i].x1 < l)
                         {
                            spans[yp].span[i].u[0] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, l, 
                                      spans[yp].span[i].u[0],
                                      spans[yp].span[i].u[1]);
                            spans[yp].span[i].v[0] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, l, 
                                      spans[yp].span[i].v[0],
                                      spans[yp].span[i].v[1]);
                            spans[yp].span[i].x1 = l;
                            spans[yp].span[i].o1 = l << FP;
                         }
                       if (spans[yp].span[i].x2 >= (cx + cw))
                         {
                            spans[yp].span[i].u[1] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, (cx + cw),
                                      spans[yp].span[i].u[0],
                                      spans[yp].span[i].u[1]);
                            spans[yp].span[i].v[1] = 
                              _interp(spans[yp].span[i].x1,
                                      spans[yp].span[i].x2, (cx + cw),
                                      spans[yp].span[i].v[0],
                                      spans[yp].span[i].v[1]);
                            spans[yp].span[i].x2 = (cx + cw);
                            spans[yp].span[i].o2 = (cx + cw) << FP;
                         }
                    }
               }
             else
               {
                  spans[yp].span[0].x1 = -1;
               }
          }
     }
}

// 66.74 % of time
static void
evas_common_map4_rgba_internal(RGBA_Image *src, RGBA_Image *dst,
                               RGBA_Draw_Context *dc,
                               RGBA_Map_Point *p, 
                               int smooth, int level)
{
   int i;
   int c, cx, cy, cw, ch;
   int ytop, ybottom, ystart, yend, y, yp, sw, sh, shp, swp, direct;
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
             yp = y - ystart;
             
             for (i = 0; i < 2; i++)
               {
                  if (spans[yp].span[i].x1 >= 0)
                    {
                       long long tl;
                       
                       x = spans[yp].span[i].x1;
                       w = (spans[yp].span[i].x2 - x);
                       
                       if (w <= 0) continue;
                       
                       dv = (spans[yp].span[i].o2 - spans[yp].span[i].o1);
                       
                       ww = w;
                       u = spans[yp].span[i].u[0] << FPI;
                       v = spans[yp].span[i].v[0] << FPI;
                       ud = ((spans[yp].span[i].u[1] << FPI) - u) / w;
                       vd = ((spans[yp].span[i].v[1] << FPI) - v) / w;
                       tl = (long long)ud * (w << FP);
                       tl = tl / dv;
                       ud = tl;
                       u -= (ud * (spans[yp].span[i].o1 - (spans[yp].span[i].x1 << FP))) / FP1;
                       
                       tl = (long long)vd * (w << FP);
                       tl = tl / dv;
                       vd = tl;
                       v -= (vd * (spans[yp].span[i].o1 - (spans[yp].span[i].x1 << FP))) / FP1;
                       
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
                            DATA32 vala, valb, val1, val2;
                            
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
                            vala = INTERP_256(ru, val2, val1);
                            
                            s = sp + ((v2 >> (FP + FPI)) * sw) + 
                              (u1 >> (FP + FPI));
                            val1 = *s;
                            s = sp + ((v2 >> (FP + FPI)) * sw) + 
                              (u2 >> (FP + FPI));
                            val2 = *s;
                            valb = INTERP_256(ru, val2, val1);
                            
                            *d++ = INTERP_256(rv, valb, vala);
                            
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
             yp = y - ystart;
             
             for (i = 0; i < 2; i++)
               {
                  if (spans[yp].span[i].x1 >= 0)
                    {
                       x = spans[yp].span[i].x1;
                       w = (spans[yp].span[i].x2 - x);
                       
                       if (w <= 0) continue;
                       ww = w;
                       u = spans[yp].span[i].u[0] << FPI;
                       v = spans[yp].span[i].v[0] << FPI;
                       ud = ((spans[yp].span[i].u[1] << FPI) - u) / w;
                       vd = ((spans[yp].span[i].v[1] << FPI) - v) / w;
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
