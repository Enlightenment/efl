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
   int ytop, ybottom, ystart, yend, y, sw, shp, swp, direct;
   Line *spans;
   DATA32 *buf = NULL, *sp;
   RGBA_Gfx_Func func = NULL;
   int havea = 0;
   int havecol = 4;

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
   if ((p[0].col >> 24) < 0xff) havea = 1;
   if (p[0].col == 0xffffffff) havecol--;
   for (i = 1; i < 4; i++)
     {
        if (p[i].y < ytop) ytop = p[i].y;
        if ((p[i].col >> 24) < 0xff) havea = 1;
        if (p[i].col == 0xffffffff) havecol--;
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
       (!dc->mul.use) && (!havea))
     {
        direct = 1;
     }
   else
     {
        int pa;
        
        buf = alloca(cw * sizeof(DATA32));
        if (!buf) return;
        pa = src->cache_entry.flags.alpha;
        if (havea) src->cache_entry.flags.alpha = 1;
        if (dc->mul.use)
          func = evas_common_gfx_func_composite_pixel_color_span_get(src, dc->mul.col, dst, cw, dc->render_op);
        else
          func = evas_common_gfx_func_composite_pixel_span_get(src, dst, cw, dc->render_op);
        src->cache_entry.flags.alpha = pa;
     }
    
   if (!havecol)
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
