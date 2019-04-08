
// 66.74 % of time
static void
FUNC_NAME(RGBA_Image *src, RGBA_Image *dst,
          int clip_x, int clip_y, int clip_w, int clip_h,
          DATA32 mul_col, int render_op,
          RGBA_Map_Point *p,
          int smooth, int anti_alias, int level EINA_UNUSED, // level unused for now - for future use
          RGBA_Image *mask_ie, int mask_x, int mask_y)
{
   int i;
   int cx, cy, cw, ch;
   int ytop, ybottom, ystart, yend, y, sw, shp, swp, direct;
   Line *spans;
   DATA32 *buf = NULL, *sp;
   RGBA_Gfx_Func func = NULL, func2 = NULL;
   Eina_Bool havea = EINA_FALSE;
   Eina_Bool sa, ssa, da;
   Eina_Bool saa;  //Source alpha overriding with anti-alias flag.
   int havecol = 4;

   cx = clip_x;
   cy = clip_y;
   cw = clip_w;
   ch = clip_h;

   // find y top line and collect point color info
   ytop = p[0].y;
   if ((p[0].col >> 24) < 0xff) havea = EINA_TRUE;
   if (p[0].col == 0xffffffff) havecol--;
   for (i = 1; i < 4; i++)
     {
        if (p[i].y < ytop) ytop = p[i].y;
        if ((p[i].col >> 24) < 0xff) havea = EINA_TRUE;
        if (p[i].col == 0xffffffff) havecol--;
     }

   // find y bottom line
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

   sa = src->cache_entry.flags.alpha;
   ssa = src->cache_entry.flags.alpha_sparse;
   da = dst->cache_entry.flags.alpha;

   // limit u,v coords of points to be within the source image
   for (i = 0; i < 4; i++)
     {
        if (p[i].u < 0) p[i].u = 0;
        else if (p[i].u > (int)(src->cache_entry.w << FP))
          p[i].u = src->cache_entry.w << FP;

        if (p[i].v < 0) p[i].v = 0;
        else if (p[i].v > (int)(src->cache_entry.h << FP))
          p[i].v = src->cache_entry.h << FP;
     }

   // allocate some spans to hold out span list
   spans = alloca((yend - ystart + 3) * sizeof(Line));
   memset(spans, 0, (yend - ystart + 3) * sizeof(Line));

   // calculate the spans list
   _calc_spans(p, spans, ystart, yend, cx, cy, cw, ch);

   // calculate anti alias edges
   if (anti_alias) _calc_aa_edges(spans, ystart, yend);

   // walk through spans and render

   // if operation is solid, bypass buf and draw func and draw direct to dst
   direct = 0;


   /* FIXME: even if anti-alias is enabled, only edges may require the
      pixels composition. we can optimize it. */

   if ((!sa) && (!da) &&
       (mul_col == 0xffffffff) && (!havea) && (!anti_alias) && (!mask_ie))
     {
        direct = 1;
     }
   else
     {
        buf = alloca(cw * sizeof(DATA32));
        if (havea) sa = EINA_TRUE;

        saa = (anti_alias | sa);

        if (!mask_ie)
          {
             if (mul_col != 0xffffffff)
               func = evas_common_gfx_func_composite_pixel_color_span_get(saa, ssa, mul_col, da, cw, render_op);
             else
               func = evas_common_gfx_func_composite_pixel_span_get(saa, ssa, da, cw, render_op);
          }
        else
          {
             func = evas_common_gfx_func_composite_pixel_mask_span_get(saa, ssa, da, cw, render_op);
             if (mul_col != 0xffffffff)
               func2 = evas_common_gfx_func_composite_pixel_color_span_get(saa, ssa, mul_col, da, cw, EVAS_RENDER_COPY);
          }
        if (sa) src->cache_entry.flags.alpha = EINA_TRUE;
     }
   if (havecol == 0)
     {
#undef COLMUL
#include "evas_map_image_core.c"
     }
   else
     {
#define COLMUL 1
#include "evas_map_image_core.c"
     }
}

static void
FUNC_NAME_DO(RGBA_Image *src, RGBA_Image *dst,
             RGBA_Draw_Context *dc,
             const RGBA_Map_Spans *ms,
             int smooth, int anti_alias, int level EINA_UNUSED) // level unused for now - for future use
{
   Line *spans;
   DATA32 *buf = NULL, *sp;
   RGBA_Gfx_Func func = NULL, func2 = NULL;
   int cx, cy, cw, ch;
   DATA32 mul_col;
   int ystart, yend, y, sw, shp, swp, direct;
   int havecol;
   int i;
   Eina_Bool sa, ssa, da;
   Eina_Bool saa;  //Source alpha overriding with anti-alias flag.

   RGBA_Image *mask_ie = dc->clip.mask;
   int mask_x = dc->clip.mask_x;
   int mask_y = dc->clip.mask_y;

   cx = dc->clip.x;
   cy = dc->clip.y;
   cw = dc->clip.w;
   ch = dc->clip.h;

   mul_col = dc->mul.use ? dc->mul.col : 0xffffffff;

   if (ms->ystart < cy) ystart = cy;
   else ystart = ms->ystart;
   if (ms->yend >= (cy + ch)) yend = (cy + ch) - 1;
   else yend = ms->yend;

   // get some source image information
   sp = src->image.data;
   sw = src->cache_entry.w;
   swp = sw << (FP + FPI);
   shp = src->cache_entry.h << (FP + FPI);
   havecol = ms->havecol;
   direct = ms->direct;

   sa = src->cache_entry.flags.alpha;
   ssa = src->cache_entry.flags.alpha_sparse;
   da = dst->cache_entry.flags.alpha;

   // allocate some s to hold out span list
   spans = alloca((yend - ystart + 3) * sizeof(Line));
   memcpy(spans, &ms->spans[ystart - ms->ystart],
          (yend - ystart + 3) * sizeof(Line));
   _clip_spans(spans, ystart, yend, cx, cw, EINA_TRUE);

   // if operation is solid, bypass buf and draw func and draw direct to dst
   if (!direct)
     {
        buf = alloca(cw * sizeof(DATA32));
        if (ms->havea) sa = EINA_TRUE;

        saa = (anti_alias | sa);

        if (!mask_ie)
          {
             if (mul_col != 0xffffffff)
               func = evas_common_gfx_func_composite_pixel_color_span_get(saa, ssa, dc->mul.col, da, cw, dc->render_op);
             else
               func = evas_common_gfx_func_composite_pixel_span_get(saa, ssa, da, cw, dc->render_op);
          }
        else
          {
             func = evas_common_gfx_func_composite_pixel_mask_span_get(saa, ssa, da, cw, dc->render_op);
             if (mul_col != 0xffffffff)
               func2 = evas_common_gfx_func_composite_pixel_color_span_get(saa, ssa, dc->mul.col, da, cw, EVAS_RENDER_COPY);
          }
        if (sa) src->cache_entry.flags.alpha = EINA_TRUE;
     }

   if (havecol == 0)
     {
#undef COLMUL
#include "evas_map_image_core.c"
     }
   else
     {
#define COLMUL 1
#include "evas_map_image_core.c"
     }
}
