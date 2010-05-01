/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_blend_private.h"

#ifdef BUILD_SCALE_SMOOTH
# ifdef BUILD_MMX
#  undef SCALE_USING_MMX
#  define SCALE_USING_MMX
# endif
#endif

#define FPI 8
#define FPI1 (1 << (FPI))
#define FPIH (1 << (FPI - 1))

#define FPFPI1 (1 << (FP + FPI))

typedef struct _Line Line;
typedef struct _Span Span;

struct _Span
{
   int x1, x2;
   FPc o1, o2, z1, z2;
   FPc  u[2], v[2];
   DATA32 col[2];
};

struct _Line
{
   Span span[2];
};

static FPc
_interp(int x1, int x2, int p, FPc u1, FPc u2)
{
   FPc u;

   x2 -= x1;
   p -= x1;
   u = u2 - u1;
   u = (u * p) / (x2 + 1);
   // FIXME: do z persp
   return u1 + u;
}

static DATA32
_interp_col(int x1, int x2, int p, DATA32 col1, DATA32 col2)
{
   x2 -= x1;
   p -= x1;
   p = (p << 8) / (x2 + 1);
   // FIXME: do z persp
   return INTERP_256(p, col2, col1);
}

static void
_limit(Span *s, int c1, int c2, int nocol)
{
   if (s->x1 < c1)
     {
        s->u[0] = _interp(s->x1, s->x2, c1, s->u[0], s->u[1]);
        s->v[0] = _interp(s->x1, s->x2, c1, s->v[0], s->v[1]);
        if (!nocol)
          s->col[0] = _interp_col(s->x1, s->x2, c1, s->col[0], s->col[1]);
        s->x1 = c1;
        s->o1 = c1 << FP;
        // FIXME: do s->z1
     }
   if (s->x2 > c2)
     {
        s->u[1] = _interp(s->x1, s->x2, c2, s->u[0], s->u[1]);
        s->v[1] = _interp(s->x1, s->x2, c2, s->v[0], s->v[1]);
        if (!nocol)
          s->col[1] = _interp_col(s->x1, s->x2, c2, s->col[0], s->col[1]);
        s->x2 = c2;
        s->o2 = c2 << FP;
        // FIXME: do s->z2
     }
}

// 12.63 % of time - this can improve
static void
_calc_spans(RGBA_Map_Point *p, Line *spans, int ystart, int yend, int cx, int cy, int cw, int ch)
{
   int i, y, yp;
   int py[4];
   int edge[4][4], edge_num, swapped, order[4];
   FPc uv[4][2], u, v, x, h, t;
   DATA32 col[4];
   
#if 1 // maybe faster on x86?
   for (i = 0; i < 4; i++) py[i] = p[i].y >> FP;
# define PY(x) (py[x])   
#else
# define PY(x) (p[x].y >> FP) 
#endif
   
   if ((PY(0) == PY(1)) && (PY(0) == PY(2)) && (PY(0) == PY(3)))
     {
        int leftp, rightp;
        int nocol = 1;
        
        leftp = rightp = 0;
        for (i = 1; i < 4; i++)
          {
             if (p[i].x < p[leftp].x) leftp = i;
             if (p[i].x > p[rightp].x) rightp = i;
             if (p[i].col != 0xffffffff) nocol = 0;
          }
        for (y = ystart; y <= yend; y++)
          {
             yp = y - ystart;
             if (y == PY(0))
               {
                  i = 0;
                  spans[yp].span[i].x1 = p[leftp].x >> FP;
                  spans[yp].span[i].o1 = p[leftp].x;
                  spans[yp].span[i].u[0] = p[leftp].u;
                  spans[yp].span[i].v[0] = p[leftp].v;
                  spans[yp].span[i].col[0] = p[leftp].col;
                  spans[yp].span[i].x2 = p[rightp].x >> FP;
                  spans[yp].span[i].o2 = p[rightp].x;
                  spans[yp].span[i].u[1] = p[rightp].u;
                  spans[yp].span[i].v[1] = p[rightp].v;
                  spans[yp].span[i].col[1] = p[rightp].col;
                  if ((spans[yp].span[i].x1 >= (cx + cw)) ||
                      (spans[yp].span[i].x2 < cx))
                    spans[yp].span[i].x1 = -1;
                  else
                    {
                       _limit(&(spans[yp].span[i]), cx, cx + cw, nocol);
                       i++;
                       spans[yp].span[i].x1 = -1;
                    }
               }
             else
               spans[yp].span[0].x1 = -1;
          }
        return;
     }
   for (y = ystart; y <= yend; y++)
     {
        int nocol = 1;
        
        yp = y - ystart;
        edge_num = 0;
        for (i = 0; i < 4; i++)
          {
             if ((PY(i) <= y) && (PY((i + 1) % 4) > y))
               {
                  edge[edge_num][0] = i;
                  edge[edge_num][1] = (i + 1) % 4;
                  edge_num++;
               }
             else if ((PY((i + 1) % 4) <= y) && (PY(i) > y))
               {
                  edge[edge_num][0] = (i + 1) % 4;
                  edge[edge_num][1] = i;
                  edge_num++;
               }
             if (p[i].col != 0xffffffff) nocol = 0;
          }
        // calculate line x points for each edge
        for (i = 0; i < edge_num; i++)
          {
             int e1 = edge[i][0];
             int e2 = edge[i][1];
             FPc t256;
             
             h = (p[e2].y - p[e1].y) >> FP; // height of edge
             t = (((y << FP) + (FP1 - 1)) - p[e1].y) >> FP;
             x = p[e2].x - p[e1].x;
             x = p[e1].x + ((x * t) / h);

/*             
             // FIXME: 3d accuracy here
             // XXX t needs adjusting. above its a linear interp point
             // only.
             // 
             // // FIXME: do in fixed pt. reduce divides
             evas_common_cpu_end_opt();
             // 
             int foc = 512, z0 = 0, px = 320, py = 240; // FIXME: need from map points
             //
             float focf, hf;
             float z1, z2, y1, y2, dz, dy, zt, dydz, yt;
             
             focf = foc;
             hf = h;
             
             // adjust for fixed point and focal length and z0 for map
             z1 = (p[e1].z >> FP) - z0 + foc;
             z2 = (p[e2].z >> FP) - z0 + foc;
             // deltas
             dz = z1 - z2;
             
             if (dz != 0)
               {
                  int pt;
                  
                  // adjust for perspective point (being 0 0)
                  y1 = (p[e1].y >> FP) - py;
                  y2 = (p[e2].y >> FP) - py;
                  
                  // correct for x &y not being in world coords - screen coords
                  y1 = (y1 * z1) / focf;
                  y2 = (y2 * z2) / focf;
                  
                  // deltas
                  dy = y1 - y2;
                  
                  yt = y - py;
                  dydz = dy / dz;

                  zt = (y2 - (dydz * z2)) / ((yt / focf) - dydz);

                  pt = t;
                  t = ((z1 - zt) * hf) / dz;
               }
 */
             u = p[e2].u - p[e1].u;
             u = p[e1].u + ((u * t) / h);
             
             v = p[e2].v - p[e1].v;
             v = p[e1].v + ((v * t) / h);

             // FIXME: 3d accuracy for color too
             t256 = (t << 8) / h; // maybe * 255?
             col[i] = INTERP_256(t256, p[e2].col, p[e1].col);
             
             // FIXME: store z persp
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
             spans[yp].span[i].col[0] = col[order[0]];
             
             spans[yp].span[i].x2 = edge[order[1]][2];
             spans[yp].span[i].o2 = edge[order[1]][3];
             spans[yp].span[i].u[1] = uv[order[1]][0];
             spans[yp].span[i].v[1] = uv[order[1]][1];
             spans[yp].span[i].col[1] = col[order[1]];
             if ((spans[yp].span[i].x1 >= (cx + cw)) ||
                 (spans[yp].span[i].x2 < cx))
               spans[yp].span[i].x1 = -1;
             else
               {
                  _limit(&(spans[yp].span[i]), cx, cx + cw, nocol);
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
             spans[yp].span[i].col[0] = col[order[0]];
             
             spans[yp].span[i].x2 = edge[order[1]][2];
             spans[yp].span[i].u[1] = uv[order[1]][0];
             spans[yp].span[i].v[1] = uv[order[1]][1];
             spans[yp].span[i].col[1] = col[order[1]];
             if ((spans[yp].span[i].x1 >= (cx + cw)) ||
                 (spans[yp].span[i].x2 < cx))
               spans[yp].span[i].x1 = -1;
             else
               {
                  _limit(&(spans[yp].span[i]), cx, cx + cw, nocol);
                  i++;
               }
             spans[yp].span[i].x1 = edge[order[2]][2];
             spans[yp].span[i].u[0] = uv[order[2]][0];
             spans[yp].span[i].v[0] = uv[order[2]][1];
             spans[yp].span[i].col[0] = col[order[2]];
             
             spans[yp].span[i].x2 = edge[order[3]][2];
             spans[yp].span[i].u[1] = uv[order[3]][0];
             spans[yp].span[i].v[1] = uv[order[3]][1];
             spans[yp].span[i].col[1] = col[order[3]];
             if ((spans[yp].span[i].x1 >= (cx + cw)) ||
                 (spans[yp].span[i].x2 < cx))
               spans[yp].span[i].x1 = -1;
             else
               {
                  int l = cx;
                  
                  if (i > 0) l = spans[yp].span[i - 1].x2;
                  _limit(&(spans[yp].span[i]), l, cx + cw, nocol);
               }
          }
        else
          spans[yp].span[0].x1 = -1;
     }
}

#ifdef BUILD_SCALE_SMOOTH
# ifdef BUILD_MMX
#  undef FUNC_NAME
#  define FUNC_NAME evas_common_map4_rgba_internal_mmx
#  undef SCALE_USING_MMX
#  define SCALE_USING_MMX
#  include "evas_map_image_internal.c"
# endif
# ifdef BUILD_C
#  undef FUNC_NAME
#  define FUNC_NAME evas_common_map4_rgba_internal
#  undef SCALE_USING_MMX
#  include "evas_map_image_internal.c"
# endif
#endif

EAPI void
evas_common_map4_rgba(RGBA_Image *src, RGBA_Image *dst,
                      RGBA_Draw_Context *dc,
                      RGBA_Map_Point *p, 
                      int smooth, int level)
{
#ifdef BUILD_MMX
   int mmx, sse, sse2;
#endif
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch;
   int          i;
   
   if (src->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
     evas_cache_image_load_data(&src->cache_entry);
   evas_common_image_colorspace_normalize(src);
   if (!src->image.data) return;
#ifdef BUILD_MMX
   evas_common_cpu_can_do(&mmx, &sse, &sse2);
#endif   
   if ((!dc->cutout.rects) && (!dc->clip.use))
     {
#ifdef BUILD_MMX
        if (mmx)
          evas_common_map4_rgba_internal_mmx(src, dst, dc, p, smooth, level);
        else
#endif
#ifdef BUILD_C
          evas_common_map4_rgba_internal(src, dst, dc, p, smooth, level);
#endif
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
#ifdef BUILD_MMX
        if (mmx)
          evas_common_map4_rgba_internal_mmx(src, dst, dc, p, smooth, level);
        else
#endif
#ifdef BUILD_C
          evas_common_map4_rgba_internal(src, dst, dc, p, smooth, level);
#endif        
     }
   evas_common_draw_context_apply_clear_cutouts(rects);
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
}
