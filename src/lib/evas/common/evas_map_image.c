#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_blend_private.h"
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif
#ifdef BUILD_NEON
#include <arm_neon.h>
#endif

#ifdef BUILD_MMX
# undef SCALE_USING_MMX
# define SCALE_USING_MMX
#endif

#define FPI 8
#define FPI1 (1 << (FPI))
#define FPIH (1 << (FPI - 1))

#define FPFPI1 (1 << (FP + FPI))

typedef struct _Line Line;
typedef struct _Span Span;

struct _Span
{
   int x[2];
   FPc o1, o2, z1, z2;
   FPc  u[2], v[2];
   DATA32 col[2];
};

struct _Line
{
   Span span[2];
   int aa_cov[2];
   int aa_len[2];
};

static inline FPc
_interp(int x1, int x2, int p, FPc u1, FPc u2)
{
   FPc u;

   x2 -= x1;
   if (x2 == 0) x2 = 1;
   p -= x1;
   u = u2 - u1;
   u = ((u * p) / x2);
   // FIXME: do z persp
   return u1 + u;
}

static inline DATA32
_interp_col(int x1, int x2, int p, DATA32 col1, DATA32 col2)
{
   x2 -= x1;
   if (x2 == 0) x2 = 1;
   p -= x1;
   p = ((p << 8) / x2);
   // FIXME: do z persp
   return INTERP_256(p, col2, col1);
}

static inline void
_interpolated_clip_span(Span *s, int c1, int c2, Eina_Bool interp_col)
{
   if (s->x[0] < c1)
     {
        s->u[0] = _interp(s->x[0], s->x[1], c1, s->u[0], s->u[1]);
        s->v[0] = _interp(s->x[0], s->x[1], c1, s->v[0], s->v[1]);
        if (interp_col)
          s->col[0] = _interp_col(s->x[0], s->x[1], c1, s->col[0], s->col[1]);
        s->x[0] = c1;
        s->o1 = c1 << FP;
        // FIXME: do s->z1
     }
   if (s->x[1] > c2)
     {
        s->u[1] = _interp(s->x[0], s->x[1], c2, s->u[0], s->u[1]);
        s->v[1] = _interp(s->x[0], s->x[1], c2, s->v[0], s->v[1]);
        if (interp_col)
          s->col[1] = _interp_col(s->x[0], s->x[1], c2, s->col[0], s->col[1]);
        s->x[1] = c2;
        s->o2 = c2 << FP;
        // FIXME: do s->z2
     }
}

#include "evas_map_image_aa.c"

// 12.63 % of time - this can improve
static void
_calc_spans(RGBA_Map_Point *p, Line *spans, int ystart, int yend, int cx, int cy EINA_UNUSED, int cw, int ch EINA_UNUSED)
{
   int i, y, yp;
   int py[4];
   int edge[4][4], edge_num, order[4];
   FPc uv[4][2], u, v, x, t, edge_h;
   DATA32 col[4];
   Eina_Bool interp_col = EINA_FALSE;
   Eina_Bool swapped;

   for (i = 0; i < 4; i++) py[i] = (p[i].y >> FP);

   //Horizontal Line?
   if ((py[0] == py[1]) && (py[0] == py[2]) && (py[0] == py[3]))
     {
        int leftp, rightp;

        leftp = rightp = 0;

        for (i = 1; i < 4; i++)
          {
             if (p[i].x < p[leftp].x) leftp = i;
             if (p[i].x > p[rightp].x) rightp = i;
             if (p[i].col != 0xffffffff) interp_col = EINA_TRUE;
          }
        for (y = ystart; y <= yend; y++)
          {
             yp = y - ystart;
             if (y == py[0])
               {
                  i = 0;
                  spans[yp].span[i].x[0] = p[leftp].x >> FP;
                  spans[yp].span[i].o1 = p[leftp].x;
                  spans[yp].span[i].u[0] = p[leftp].u;
                  spans[yp].span[i].v[0] = p[leftp].v;
                  spans[yp].span[i].col[0] = p[leftp].col;
                  spans[yp].span[i].x[1] = p[rightp].x >> FP;
                  spans[yp].span[i].o2 = p[rightp].x;
                  spans[yp].span[i].u[1] = p[rightp].u;
                  spans[yp].span[i].v[1] = p[rightp].v;
                  spans[yp].span[i].col[1] = p[rightp].col;
                  //Outside of the clipper
                  if ((spans[yp].span[i].x[0] > (cx + cw)) ||
                      (spans[yp].span[i].x[1] < cx))
                    spans[yp].span[i].x[0] = -1;
                  else
                    {
                       _interpolated_clip_span(&(spans[yp].span[i]), cx,
                                              (cx + cw), interp_col);
                       i++;
                       spans[yp].span[i].x[0] = -1;
                    }
               }
             //The polygon shape seems not be completed definitely.
             else
               spans[yp].span[0].x[0] = -1;
          }
        return;
     }
   for (y = ystart; y <= yend; y++)
     {
        edge_num = 0;

        //Find edges that intersects with current scanline.
        for (i = 0; i < 4; i++)
          {
             if ((py[i] <= y) && (py[(i + 1) % 4] > y))
               {
                  edge[edge_num][0] = i;
                  edge[edge_num][1] = (i + 1) % 4;
                  edge_num++;
               }
             else if ((py[(i + 1) % 4] <= y) && (py[i] > y))
               {
                  edge[edge_num][0] = (i + 1) % 4;
                  edge[edge_num][1] = i;
                  edge_num++;
               }
             if (p[i].col != 0xffffffff) interp_col = EINA_TRUE;
          }

        // calculate line x points for each edge
        for (i = 0; i < edge_num; i++)
          {
             int e1 = edge[i][0];
             int e2 = edge[i][1];
             FPc t256;

             // compute x point that proportionated to the y point offset
             edge_h = (p[e2].y - p[e1].y) >> FP;  //edge height
             if (edge_h < 1) edge_h = 1;
             t = (((y << FP) + (FP1 / 2) - 1) - p[e1].y) >> FP;
             x = p[e2].x - p[e1].x;  //edge width

             FPc temp = (x * t);

             // TODO: prevent data overflow. We can remove this exception if FPc type is more than integer.
             if (temp < 0) temp = (((x >> FP) * t) / edge_h) << FP;
             else temp /= edge_h;

             x = p[e1].x + temp; // intersected x point

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
             //compute texture u coordinate
             u = p[e2].u - p[e1].u;
             u = p[e1].u + ((u * t) / edge_h);

             //compute texture v coordinate
             v = p[e2].v - p[e1].v;
             v = p[e1].v + ((v * t) / edge_h);

             // FIXME: 3d accuracy for color too
             t256 = (t << 8) / edge_h; // maybe * 255?
             col[i] = INTERP_256(t256, p[e2].col, p[e1].col);

             // FIXME: store z persp
             uv[i][0] = u;
             uv[i][1] = v;
             edge[i][2] = x >> FP;
             edge[i][3] = x;
             // also fill in order
             order[i] = i;
          }

        // sort edges from left to right - bubble. its a small list!
        do
          {
             swapped = EINA_FALSE;
             for (i = 0; i < (edge_num - 1); i++)
               {
                  if (edge[order[i]][2] > edge[order[i + 1]][2])
                    {
                       t = order[i];
                       order[i] = order[i + 1];
                       order[i + 1] = t;
                       swapped = EINA_TRUE;
                    }
               }
          }
        while (swapped);

        yp = y - ystart;

        if (edge_num == 2)
          {
             i = 0;
             spans[yp].span[i].x[0] = edge[order[0]][2];
             spans[yp].span[i].o1 = edge[order[0]][3];
             spans[yp].span[i].u[0] = uv[order[0]][0];
             spans[yp].span[i].v[0] = uv[order[0]][1];
             spans[yp].span[i].col[0] = col[order[0]];

             spans[yp].span[i].x[1] = edge[order[1]][2];
             spans[yp].span[i].o2 = edge[order[1]][3];
             spans[yp].span[i].u[1] = uv[order[1]][0];
             spans[yp].span[i].v[1] = uv[order[1]][1];
             spans[yp].span[i].col[1] = col[order[1]];

             //Outside of the clipper
             if ((spans[yp].span[i].x[0] > (cx + cw)) ||
                 (spans[yp].span[i].x[1] < cx))
               spans[yp].span[i].x[0] = -1;
             else
               {
                  _interpolated_clip_span(&(spans[yp].span[i]), cx, (cx + cw),
                                          interp_col);
                  i++;
                  spans[yp].span[i].x[0] = -1;
               }
          }
        else if (edge_num == 4)
          {
             i = 0;
             spans[yp].span[i].x[0] = edge[order[0]][2];
             spans[yp].span[i].u[0] = uv[order[0]][0];
             spans[yp].span[i].v[0] = uv[order[0]][1];
             spans[yp].span[i].col[0] = col[order[0]];

             spans[yp].span[i].x[1] = edge[order[1]][2];
             spans[yp].span[i].u[1] = uv[order[1]][0];
             spans[yp].span[i].v[1] = uv[order[1]][1];
             spans[yp].span[i].col[1] = col[order[1]];

             //Outside of the clipper
             if ((spans[yp].span[i].x[0] > (cx + cw)) ||
                 (spans[yp].span[i].x[1] < cx))
               spans[yp].span[i].x[0] = -1;
             else
               {
                  _interpolated_clip_span(&(spans[yp].span[i]), cx, (cx + cw),
                                          interp_col);
                  i++;
               }

             spans[yp].span[i].x[0] = edge[order[2]][2];
             spans[yp].span[i].u[0] = uv[order[2]][0];
             spans[yp].span[i].v[0] = uv[order[2]][1];
             spans[yp].span[i].col[0] = col[order[2]];

             spans[yp].span[i].x[1] = edge[order[3]][2];
             spans[yp].span[i].u[1] = uv[order[3]][0];
             spans[yp].span[i].v[1] = uv[order[3]][1];
             spans[yp].span[i].col[1] = col[order[3]];

             //Outside of the clipper
             if ((spans[yp].span[i].x[0] > (cx + cw)) ||
                 (spans[yp].span[i].x[1] < cx))
               spans[yp].span[i].x[0] = -1;
             else
               {
                  int l = cx;

                  if (i > 0) l = spans[yp].span[i - 1].x[1];
                  _interpolated_clip_span(&(spans[yp].span[i]), l, (cx + cw),
                                          interp_col);
               }
          }
        //The polygon shape seems not be completed definitely.
        else
          spans[yp].span[0].x[0] = -1;
     }
}

/* FIXME: Account for 10% during pipe rendering, should be improved
 * Could be computing the interpolation once somehow.
 */
static void
_clip_spans(Line *spans, int ystart, int yend,
            int cx, int cw, Eina_Bool interp_col)
{
   int y, yp;

   for (y = ystart, yp = 0; y <= yend; y++, yp++)
     {
        if (spans[yp].span[0].x[0] > -1)
          {
             if ((spans[yp].span[0].x[0] >= (cx + cw)) ||
                 (spans[yp].span[0].x[1] < cx))
               {
                  spans[yp].span[0].x[0] = -1;
               }
             else
               {
                  _interpolated_clip_span(&(spans[yp].span[0]), cx, (cx + cw),
                                          interp_col);

                  if ((spans[yp].span[1].x[0] >= (cx + cw)) ||
                      (spans[yp].span[1].x[1] < cx))
                    {
                       spans[yp].span[1].x[0] = -1;
                    }
                  else
                    {
                       _interpolated_clip_span(&(spans[yp].span[1]),
                                               spans[yp].span[0].x[1],
                                               cx + cw, interp_col);
                    }
               }
          }
     }
}

typedef struct _RGBA_Map_Spans RGBA_Map_Spans;
typedef struct _RGBA_Map_Cutout RGBA_Map_Cutout;

struct _RGBA_Map_Spans
{
   Line *spans;
   int size;
   int ystart;
   int yend;

   int havecol;
   Eina_Bool havea;
   Eina_Bool direct;
};

struct _RGBA_Map_Cutout
{
   int count;

   Cutout_Rects *rects;
   RGBA_Map_Spans spans[1];
};

EAPI void
evas_common_map_rgba_clean(RGBA_Map *m)
{
   RGBA_Map_Cutout *spans = m->engine_data;

   if (spans)
     {
        int i;

        if (spans->rects)
          evas_common_draw_context_apply_clear_cutouts(spans->rects);
        for (i = 0; i < spans->count; i++)
          free(spans->spans[i].spans);
        free(spans);
     }

   m->engine_data = NULL;
}

static void
_rgba_map_cutout_resize(RGBA_Map *m, int count)
{
   RGBA_Map_Cutout *old = m->engine_data;
   RGBA_Map_Cutout *r;
   int size;
   int i;

   if (count == 0)
     goto empty;

   if (old && old->count == count)
     {
        return;
     }

   size = sizeof (RGBA_Map_Cutout) + sizeof (RGBA_Map_Spans) * (count - 1);

   if (old)
     {
        for (i = 0; i < old->count; i++)
          {
             free(old->spans[i].spans);
             old->spans[i].spans = NULL;
          }
     }

   r = realloc(old, size);
   if (!r)
     goto empty;

   memset(r, 0, size);
   m->engine_data = r;
   r->count = count;
   return;

 empty:
   evas_common_map_rgba_clean(m);
   return;
}

static void
_evas_common_map_rgba_span(RGBA_Map_Spans *span,
                           RGBA_Image *src, RGBA_Image *dst,
                           RGBA_Draw_Context *dc,
                           RGBA_Map_Point *p,
                           int cx, int cy, int cw, int ch)
{
   int ytop, ybottom, sw;
   unsigned int i;

   span->havecol = 4;
   span->havea = 0;
   span->direct = 0;

   // find y top line and y bottom line
   ytop = p[0].y;
   if ((p[0].col >> 24) < 0xff) span->havea = 1;
   if (p[0].col == 0xffffffff) span->havecol--;
   for (i = 1; i < 4; i++)
     {
        if (p[i].y < ytop) ytop = p[i].y;
        if ((p[i].col >> 24) < 0xff) span->havea = 1;
        if (p[i].col == 0xffffffff) span->havecol--;
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
   if (ytop < cy) span->ystart = cy;
   else span->ystart = ytop;
   if (ybottom >= (cy + ch)) span->yend = (cy + ch) - 1;
   else span->yend = ybottom;

   // get some source image information
   sw = src->cache_entry.w;

   // limit u,v coords of points to be within the source image
   for (i = 0; i < 4; i++)
     {
        if (p[i].u < 0) p[i].u = 0;
        else if (p[i].u > (int)(sw << FP))
          p[i].u = src->cache_entry.w << FP;
        
        if (p[i].v < 0) p[i].v = 0;
        else if (p[i].v > (int)(sw << FP))
          p[i].v = src->cache_entry.h << FP;
     }
   
   // allocate some spans to hold out span list
   if (span->size < (span->yend - span->ystart + 1))
     {
        free(span->spans);
        span->size = (span->yend - span->ystart + 1);
        span->spans = calloc(1, span->size * sizeof(Line));
     }
   if (!span->spans) return;

   // calculate the spans list
   _calc_spans(p, span->spans, span->ystart, span->yend, cx, cy, cw, ch);

   // if operation is solid, bypass buf and draw func and draw direct to dst
   if ((!src->cache_entry.flags.alpha) && (!dst->cache_entry.flags.alpha) &&
       (!dc->mul.use) && (!span->havea))
     {
        span->direct = 1;
     }
}

EAPI Eina_Bool
evas_common_map_rgba_prepare(RGBA_Image *src, RGBA_Image *dst,
                             RGBA_Draw_Context *dc,
                             RGBA_Map *m)
{
   RGBA_Map_Cutout *spans;
   Cutout_Rects *rects = NULL;
   Cutout_Rect *r;
   int i;

   if ((!dc->cutout.rects) && (!dc->clip.use))
     {
	evas_common_draw_context_clip_clip(dc, 0, 0,
                                           dst->cache_entry.w, dst->cache_entry.h);
	if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
	  {
             _rgba_map_cutout_resize(m, 0);
             return EINA_FALSE;
	  }

        _rgba_map_cutout_resize(m, 1);
        if (!m->engine_data) return EINA_FALSE;

        spans = m->engine_data;

        _evas_common_map_rgba_span(&spans->spans[0], src, dst, dc, m->pts,
                                   0, 0,
                                   dst->cache_entry.w, dst->cache_entry.h);
        return EINA_TRUE;
     }

   evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     {
        _rgba_map_cutout_resize(m, 0);
        return EINA_FALSE;
     }

   spans = m->engine_data;
   if (spans)
     {
        rects = spans->rects;
        spans->rects = NULL;
     }
   rects = evas_common_draw_context_apply_cutouts(dc, rects);
   _rgba_map_cutout_resize(m, rects->active);

   spans = m->engine_data;
   if (!spans)
     {
        evas_common_draw_context_apply_clear_cutouts(rects);
        return EINA_FALSE;
     }

   spans->rects = rects;
   for (i = 0; i < spans->rects->active; ++i)
     {
       r = spans->rects->rects + i;

       _evas_common_map_rgba_span(&spans->spans[i], src, dst, dc, m->pts,
				  r->x, r->y, r->w, r->h);
     }

   return EINA_TRUE;
}

#ifdef BUILD_MMX
# undef FUNC_NAME
# undef FUNC_NAME_DO
# define FUNC_NAME _evas_common_map_rgba_internal_mmx
# define FUNC_NAME_DO evas_common_map_rgba_internal_mmx_do
# undef SCALE_USING_MMX
# define SCALE_USING_MMX
# include "evas_map_image_internal.c"
#endif

#undef FUNC_NAME
#undef FUNC_NAME_DO
#define FUNC_NAME _evas_common_map_rgba_internal
#define FUNC_NAME_DO evas_common_map_rgba_internal_do
#undef SCALE_USING_MMX
#include "evas_map_image_internal.c"

# ifdef BUILD_NEON
#  undef FUNC_NAME
#  undef FUNC_NAME_DO
#  define FUNC_NAME _evas_common_map_rgba_internal_neon
#  define FUNC_NAME_DO evas_common_map_rgba_internal_neon_do
#  undef SCALE_USING_NEON
#  define SCALE_USING_NEON
#  undef SCALE_USING_MMX
#  include "evas_map_image_internal.c"
#  undef SCALE_USING_NEON
#endif


#ifdef BUILD_MMX
void evas_common_map_rgba_internal_mmx(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Map_Point *p, int smooth, int level)
{
   int clip_x, clip_y, clip_w, clip_h;
   DATA32 mul_col;

   if (dc->clip.use)
     {
	clip_x = dc->clip.x;
	clip_y = dc->clip.y;
	clip_w = dc->clip.w;
	clip_h = dc->clip.h;
     }
   else
     {
	clip_x = clip_y = 0;
	clip_w = dst->cache_entry.w;
	clip_h = dst->cache_entry.h;
     }

   mul_col = dc->mul.use ? dc->mul.col : 0xffffffff;

   _evas_common_map_rgba_internal_mmx(src, dst,
                                      clip_x, clip_y, clip_w, clip_h,
                                      mul_col, dc->render_op,
                                      p, smooth, dc->anti_alias, level,
                                      dc->clip.mask, dc->clip.mask_x, dc->clip.mask_y);
}
#endif

void evas_common_map_rgba_internal(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Map_Point *p, int smooth, int level)
{
   int clip_x, clip_y, clip_w, clip_h;
   DATA32 mul_col;

   if (dc->clip.use)
     {
	clip_x = dc->clip.x;
	clip_y = dc->clip.y;
	clip_w = dc->clip.w;
	clip_h = dc->clip.h;
     }
   else
     {
	clip_x = clip_y = 0;
	clip_w = dst->cache_entry.w;
	clip_h = dst->cache_entry.h;
     }

   mul_col = dc->mul.use ? dc->mul.col : 0xffffffff;

   _evas_common_map_rgba_internal(src, dst,
                                  clip_x, clip_y, clip_w, clip_h,
                                  mul_col, dc->render_op,
                                  p, smooth, dc->anti_alias, level,
                                  dc->clip.mask, dc->clip.mask_x, dc->clip.mask_y);
}

#ifdef BUILD_NEON
void evas_common_map_rgba_internal_neon(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Map_Point *p, int smooth, int level)
{
   int clip_x, clip_y, clip_w, clip_h;
   DATA32 mul_col;

   if (dc->clip.use)
     {
	clip_x = dc->clip.x;
	clip_y = dc->clip.y;
	clip_w = dc->clip.w;
	clip_h = dc->clip.h;
     }
   else
     {
	clip_x = clip_y = 0;
	clip_w = dst->cache_entry.w;
	clip_h = dst->cache_entry.h;
     }

   mul_col = dc->mul.use ? dc->mul.col : 0xffffffff;

   _evas_common_map_rgba_internal_neon(src, dst,
                                  clip_x, clip_y, clip_w, clip_h,
                                  mul_col, dc->render_op,
                                  p, smooth, dc->anti_alias, level,
                                  dc->clip.mask, dc->clip.mask_x, dc->clip.mask_y);
}
#endif

EAPI void
evas_common_map_rgba_cb(RGBA_Image *src, RGBA_Image *dst,
                        RGBA_Draw_Context *dc,
                        int npoints EINA_UNUSED, RGBA_Map_Point *p,
                        int smooth, int level,
                        Evas_Common_Map_RGBA_Cb cb)
{
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch;
   int          i;

   if (src->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_load_data(&src->cache_entry);
        else
#endif
          evas_cache_image_load_data(&src->cache_entry);
     }

   evas_common_image_colorspace_normalize(src);
   if ((!src->image.data) || (!dst->image.data)) return;

   if ((!dc->cutout.rects) && (!dc->clip.use))
     {
        cb(src, dst, dc, p, smooth, level);
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
   dc->cache.rects = evas_common_draw_context_apply_cutouts(dc, dc->cache.rects);
   for (i = 0; i < dc->cache.rects->active; ++i)
     {
        r = dc->cache.rects->rects + i;
        evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
        cb(src, dst, dc, p, smooth, level);
     }
   evas_common_draw_context_cache_update(dc);
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
}

EAPI Eina_Bool
evas_common_map_thread_rgba_cb(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Map *map, int smooth, int level, int offset, Evas_Common_Map_Thread_RGBA_Cb cb)
{
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch;
   int          i;
   Eina_Bool ret = EINA_FALSE;

   if (src->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_load_data(&src->cache_entry);
        else
#endif
          evas_cache_image_load_data(&src->cache_entry);
     }

   evas_common_image_colorspace_normalize(src);

   if ((!src->image.data) || (!dst->image.data)) return EINA_FALSE;

   if ((!dc->cutout.rects) && (!dc->clip.use))
     {
        return cb(src, dst, dc, map, smooth, level, offset);
     }

   /* save out clip info */
   c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     {
        dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
        return EINA_FALSE;
     }

   dc->cache.rects = evas_common_draw_context_apply_cutouts(dc, dc->cache.rects);
   for (i = 0; i < dc->cache.rects->active; ++i)
     {
        r = dc->cache.rects->rects + i;
        evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
        ret |= cb(src, dst, dc, map, smooth, level, offset);
     }
   evas_common_draw_context_cache_update(dc);
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;

   return ret;
}

EAPI void
evas_common_map_rgba(RGBA_Image *src, RGBA_Image *dst,
                     RGBA_Draw_Context *dc,
                     int npoints EINA_UNUSED, RGBA_Map_Point *p,
                     int smooth, int level)
{
   Evas_Common_Map_RGBA_Cb cb;
#ifdef BUILD_MMX
   int mmx, sse, sse2;

   evas_common_cpu_can_do(&mmx, &sse, &sse2);
   if (mmx)
     cb = evas_common_map_rgba_internal_mmx;
   else
#endif
#ifdef BUILD_NEON
   if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
     cb = evas_common_map_rgba_internal_neon;
   else
#endif
     cb = evas_common_map_rgba_internal;

   evas_common_map_rgba_cb(src, dst, dc, npoints, p, smooth, level, cb);
}

EAPI void
evas_common_map_rgba_draw(RGBA_Image *src, RGBA_Image *dst, int clip_x, int clip_y, int clip_w, int clip_h, DATA32 mul_col, int render_op, int npoints EINA_UNUSED, RGBA_Map_Point *p, int smooth, Eina_Bool anti_alias, int level, RGBA_Image *mask_ie, int mask_x, int mask_y)
{
#ifdef BUILD_MMX
   int mmx, sse, sse2;

   evas_common_cpu_can_do(&mmx, &sse, &sse2);
   if (mmx)
     _evas_common_map_rgba_internal_mmx(src, dst,
                                        clip_x, clip_y, clip_w, clip_h,
                                        mul_col, render_op,
                                        p, smooth, anti_alias, level,
                                        mask_ie, mask_x, mask_y);
   else
#endif
#ifdef BUILD_NEON
   if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
     _evas_common_map_rgba_internal_neon(src, dst,
                                    clip_x, clip_y, clip_w, clip_h,
                                    mul_col, render_op,
                                    p, smooth, anti_alias, level,
                                    mask_ie, mask_x, mask_y);
   else
#endif
     _evas_common_map_rgba_internal(src, dst,
                                    clip_x, clip_y, clip_w, clip_h,
                                    mul_col, render_op,
                                    p, smooth, anti_alias, level,
                                    mask_ie, mask_x, mask_y);
}

EAPI void
evas_common_map_rgba_do(const Eina_Rectangle *clip,
                        RGBA_Image *src, RGBA_Image *dst,
			RGBA_Draw_Context *dc,
			const RGBA_Map *m,
			int smooth, int level)
{
#ifdef BUILD_MMX
   int mmx, sse, sse2;
#endif
   const Cutout_Rects *rects;
   const RGBA_Map_Cutout *spans;
   Eina_Rectangle area;
   Cutout_Rect *r;
   int i;

#ifdef BUILD_MMX
   evas_common_cpu_can_do(&mmx, &sse, &sse2);
#endif   

   spans = m->engine_data;
   rects = spans->rects;
   if (rects->active == 0 &&
       spans->count == 1)
     {
        evas_common_draw_context_set_clip(dc, clip->x, clip->y, clip->w, clip->h);
#ifdef BUILD_MMX
        if (mmx)
          evas_common_map_rgba_internal_mmx_do(src, dst, dc,
                                               &spans->spans[0], smooth,
                                               dc->anti_alias, level);
        else
#endif
#ifdef BUILD_NEON
        if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
          evas_common_map_rgba_internal_neon_do(src, dst, dc,
                                           &spans->spans[0], smooth,
                                           dc->anti_alias, level);
        else
#endif
          evas_common_map_rgba_internal_do(src, dst, dc,
                                           &spans->spans[0], smooth,
                                           dc->anti_alias, level);
        return;
     }

   for (i = 0; i < rects->active; ++i)
     {
        r = rects->rects + i;

        EINA_RECTANGLE_SET(&area, r->x, r->y, r->w, r->h);
        if (!eina_rectangle_intersection(&area, clip)) continue ;
        evas_common_draw_context_set_clip(dc, area.x, area.y, area.w, area.h);
#ifdef BUILD_MMX
        if (mmx)
          {
             evas_common_map_rgba_internal_mmx_do(src, dst, dc,
                                                  &spans->spans[i], smooth,
                                                  dc->anti_alias, level);
          }
        else
#endif
#ifdef BUILD_NEON
        if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
          evas_common_map_rgba_internal_neon_do(src, dst, dc,
                                               &spans->spans[i], smooth,
                                               dc->anti_alias, level);
        else
#endif
          evas_common_map_rgba_internal_do(src, dst, dc,
                                           &spans->spans[i], smooth,
                                           dc->anti_alias, level);
     }
}
