#include "evas_common.h"
#include <math.h>

void
evas_common_gradient_init(void)
{
}

RGBA_Gradient *
evas_common_gradient_new(void)
{
   RGBA_Gradient *gr;
   
   gr = calloc(1, sizeof(RGBA_Gradient));
   return gr;
}

void
evas_common_gradient_free(RGBA_Gradient *gr)
{
   evas_common_gradient_colors_clear(gr);
   free(gr);
}

void
evas_common_gradient_colors_clear(RGBA_Gradient *gr)
{
   if (gr->colors)
     {
	Evas_Object_List *l;

	while (gr->colors)
	  {
	     l = gr->colors;
	     gr->colors = evas_object_list_remove(gr->colors, gr->colors);
	     free(l);
	  }
	gr->colors = NULL;
     }
}

void
evas_common_gradient_color_add(RGBA_Gradient *gr, int r, int g, int b, int a, int dist)
{
   RGBA_Gradient_Color *gc;
   
   gc         = malloc(sizeof(RGBA_Gradient_Color));
   gc->r      = r;
   gc->g      = g;
   gc->b      = b;
   gc->a      = a;
   gc->dist   = dist;
   gr->colors = evas_object_list_append(gr->colors, gc);
}

void
evas_common_gradient_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, RGBA_Gradient *gr, double angle)
{
   Gfx_Func_Blend_Src_Dst func;
   int yy, xx, i, len;
   int xoff, yoff, divw, divh;
   DATA32 *ptr, *dptr, *buf, *map;
   int *hlut, *vlut;
   int o_x, o_y, o_w, o_h;
   RGBA_Image *tmp;

   o_x = x; o_y = y; o_w = w; o_h = h;
   
   if ((w <= 0) || (h <= 0)) return;
   
   if (!(RECTS_INTERSECT(x, y, w, h, 0, 0, dst->image->w, dst->image->h)))
     return;
   if (x < 0)
     {
	w += x;
	x = 0;
     }
   if (y < 0)
     {
	h += y;
	y = 0;
     }
   if ((w <= 0) || (h <= 0)) return;
   if ((x + w) > dst->image->w) w = dst->image->w - x;
   if ((y + h) > dst->image->h) h = dst->image->h - y;
   if ((w <= 0) || (h <= 0)) return;
   
   if (dc->clip.use)
     {
	if (dc->clip.x > x)
	  {
	     w += x - dc->clip.x;
	     x = dc->clip.x;
	  }
	if (dc->clip.y > y)
	  {
	     h += y - dc->clip.y;
	     y = dc->clip.y;
	  }
	if ((w <= 0) || (h <= 0)) return;
	if ((dc->clip.x + dc->clip.w) < (x + w)) 
	  w = dc->clip.x + dc->clip.w - x;
	if ((dc->clip.y + dc->clip.h) < (y + h)) 
	  h = dc->clip.y + dc->clip.h - y;
     }
   if ((w <= 0) || (h <= 0)) return;
   
   hlut = malloc(sizeof(int) * o_w);
   if (!hlut) return;
   vlut = malloc(sizeof(int) * o_h);
   if (!vlut)
     {
	free(hlut);
	return;
     }
   
   xoff = x - o_x;
   yoff = y - o_y;
   
   if (o_w > o_h) len = o_w * 16;
   else           len = o_h * 16;

   map = evas_common_gradient_map(gr, dc, len);
   if (!map) 
     {
	free(hlut);
	free(vlut);
	return;
     }
   tmp = evas_common_image_create(w, 1);
   if (!tmp)
     {
	free(hlut);
	free(vlut);
	free(map);
	return;
     }
   tmp->flags |= RGBA_IMAGE_HAS_ALPHA;

   evas_common_cpu_end_opt();
   xx = (int)(32 * sin(((angle + 180) * 2 * 3.141592654) / 360));
   yy = -(int)(32 * cos(((angle + 180) * 2 * 3.141592654) / 360));
   divw = ((o_w - 1) << 5);
   divh = ((o_h - 1) << 5);
   if (divw < 1) divw = 1;
   if (divh < 1) divh = 1;
   
   if (xx < 0)
     {
	for (i = 0; i < o_w; i++) hlut[i] = (-xx * (o_w - 1 - i) * len) / divw;
     }
   else
     {
	for (i = 0; i < o_w; i++) hlut[i] = (xx * i * len) / divw;
     }
   if (yy < 0)
     {
	for (i = 0; i < o_h; i++) vlut[i] = (-yy * (o_h - 1 - i) * len) / divh;
     }
   else
     {
	for (i = 0; i < o_h; i++) vlut[i] = (yy * i * len) / divh;
     }

   buf = tmp->image->data;
   func = evas_common_draw_func_blend_get(tmp, dst, w);
   ptr = dst->image->data + (y * dst->image->w) + x;
   for (yy = 0; yy < h; yy++)
     {
	dptr = buf;
	for (xx = 0; xx < w; xx++)
	  {
	     i = vlut[yoff + yy] + hlut[xoff + xx];
	     if      (i < 0)    i = 0;
	     else if (i >= len) i = len - 1;
	     *dptr = map[i];
	     dptr++;
	  }
	func(buf, ptr, w);
	ptr += dst->image->w;
     }
   free(hlut);
   free(vlut);
   free(map);
   evas_common_image_free(tmp);
}

DATA32 *
evas_common_gradient_map(RGBA_Gradient *gr, RGBA_Draw_Context *dc, int len)
{
   DATA32 *map, *pmap, v, vv;
   Evas_Object_List *l;
   RGBA_Gradient_Color *gc, *gc_next;
   int r, g, b, a, rr, gg, bb, aa, i, j, inc, v1, v2, total;
   
   if (!gr->colors) return NULL;
   if (!gr->colors->next) return NULL;
   
   total = 1;
   for (l = gr->colors; l->next; l = l->next)
     {
	gc = (RGBA_Gradient_Color *)l;
	total += gc->dist;
     }
   map = malloc(len * sizeof(DATA32));
   if (!map) return NULL;
   pmap = malloc(total * sizeof(DATA32));
   if (!pmap)
     {
	free(map);
	return NULL;
     }
   i = 0;
   for (l = gr->colors; l; l = l->next)
     {
	gc = (RGBA_Gradient_Color *)l;
	
	if (l->next)
	  {
	     gc_next = (RGBA_Gradient_Color *)l->next;
	     for (j = 0; j < gc->dist; j++)
	       {
		  v1 = (j << 16) / gc->dist;
		  v2 = 65536 - v1;
		  r = ((gc->r * v2) + (gc_next->r * v1)) >> 16;
		  g = ((gc->g * v2) + (gc_next->g * v1)) >> 16;
		  b = ((gc->b * v2) + (gc_next->b * v1)) >> 16;
		  a = ((gc->a * v2) + (gc_next->a * v1)) >> 16;
		  if (dc->mul.use)
		    {
		       r = ((r + 1) * (R_VAL(&dc->mul.col))) >> 8;
		       g = ((g + 1) * (G_VAL(&dc->mul.col))) >> 8;
		       b = ((b + 1) * (B_VAL(&dc->mul.col))) >> 8;
		       a = ((a + 1) * (A_VAL(&dc->mul.col))) >> 8;
		    }
		  pmap[i++] = (a << 24) | (r << 16) | (g << 8) | b;
	       }	     
	  }
	else
	  {
	     r = gc->r;
	     g = gc->g;
	     b = gc->b;
	     a = gc->a;
	     pmap[i++] = (a << 24) | (r << 16) | (g << 8) | b;	     
	  }
     }
   inc = ((total - 1) << 16) / (len);
   j = 0;
   for (i = 0; i < len; i++)
     {
	v = pmap[j >> 16];
	if   ((j >> 16) < total) vv = pmap[(j >> 16) + 1];
	else                     vv = pmap[(j >> 16)    ];
	v1 = j - ((j >> 16) << 16);
	v2 = 65536 - v1;
	b =  ((v)       ) & 0xff;
	g =  ((v) >> 8  ) & 0xff;
	r =  ((v) >> 16 ) & 0xff;
	a =  ((v) >> 24 ) & 0xff;
	bb = ((vv)      ) & 0xff;
	gg = ((vv) >> 8 ) & 0xff;
	rr = ((vv) >> 16) & 0xff;
	aa = ((vv) >> 24) & 0xff;
	r =  ((r * v2) + (rr * v1)) >> 16;
	g =  ((g * v2) + (gg * v1)) >> 16;
	b =  ((b * v2) + (bb * v1)) >> 16;
	a =  ((a * v2) + (aa * v1)) >> 16;
	map[i] = (a << 24) | (r << 16) | (g << 8) | b;
	j += inc;
     }
   free(pmap);
   return map;
}
