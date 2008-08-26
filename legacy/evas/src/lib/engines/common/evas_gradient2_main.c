#include "evas_common.h"
#include "evas_blend_private.h"
#include "evas_private.h"
#include <math.h>

static void _evas_common_gradient2_stops_free(RGBA_Gradient2 *gr);
static void _evas_common_gradient2_stops_scale(RGBA_Gradient2 *gr);

static void _evas_common_gradient2_map_argb(RGBA_Draw_Context *dc, RGBA_Gradient2 *gr, int len);
static void _evas_common_gradient2_map_ahsv(RGBA_Draw_Context *dc, RGBA_Gradient2 *gr, int len);

static  int grad_initialised = 0;

EAPI void
evas_common_gradient2_init(void)
{
   RGBA_Gradient2_Type  *geom;

   if (grad_initialised)
	return;
   geom = evas_common_gradient2_type_linear_get();
   if (geom)
	geom->init();
   geom = evas_common_gradient2_type_radial_get();
   if (geom)
	geom->init();
   grad_initialised = 1;
}

void
evas_common_gradient2_shutdown(void)
{
   RGBA_Gradient2_Type  *geom;

   if (!grad_initialised)
	return;
   geom = evas_common_gradient2_type_linear_get();
   if (geom)
	geom->shutdown();
   geom = evas_common_gradient2_type_radial_get();
   if (geom)
	geom->shutdown();
   grad_initialised = 0;
}

static void
_evas_common_gradient2_stops_free(RGBA_Gradient2 *gr)
{
   if (!gr) return;
   if (gr->stops.stops)
     {
	Evas_Object_List *l;

	while (gr->stops.stops)
	  {
	   l = gr->stops.stops;
	   gr->stops.stops = evas_object_list_remove(gr->stops.stops, gr->stops.stops);
	   free(l);
	  }
	gr->stops.stops = NULL;
	gr->stops.nstops = 0;
     }
}

EAPI void
evas_common_gradient2_free(RGBA_Gradient2 *gr)
{
   if (!gr) return;
   gr->references--;
   if (gr->references > 0) return;
   evas_common_gradient2_clear(gr);
   if (gr->stops.cdata) free(gr->stops.cdata);
   if (gr->stops.adata) free(gr->stops.adata);
   if (gr->type.geometer && gr->type.gdata)
	gr->type.geometer->geom_free(gr->type.gdata);
   if (gr->map.data) free(gr->map.data);
   free(gr);
}

EAPI void
evas_common_gradient2_clear(RGBA_Gradient2 *gr)
{
   if (!gr) return;

   _evas_common_gradient2_stops_free(gr);
   gr->has_alpha = 0;
}

EAPI void
evas_common_gradient2_color_np_stop_insert(RGBA_Gradient2 *gr, int r, int g, int b, int a, float pos)
{
   RGBA_Gradient2_Color_Np_Stop *gc;
   Evas_Object_List *l;

   if (!gr) return;
   if (!gr->stops.stops)
     {
	RGBA_Gradient2_Color_Np_Stop *gc1;

	gc = malloc(sizeof(RGBA_Gradient2_Color_Np_Stop));
	if (!gc) return;
	gc1 = malloc(sizeof(RGBA_Gradient2_Color_Np_Stop));
	if (!gc1) { free(gc);  return; }
	gc->r = gc->g = gc->b = gc->a = 255;  gc->pos = 0.0;  gc->dist = 0;
	gc1->r = gc1->g = gc1->b = gc1->a = 255;  gc1->pos = 1.0;  gc1->dist = 0;

	gr->stops.stops = evas_object_list_append(gr->stops.stops, gc);
	gr->stops.stops = evas_object_list_append(gr->stops.stops, gc1);
	gr->stops.nstops = 2;
	gr->stops.len = 0;
     }

   if (r < 0) r = 0;  if (r > 255) r = 255;
   if (g < 0) g = 0;  if (g > 255) g = 255;
   if (b < 0) b = 0;  if (b > 255) b = 255;
   if (a < 0) a = 0;  if (a > 255) a = 255;
   if (pos < 0.0) pos = 0.0;
   if (pos > 1.0) pos = 1.0;

   if (pos == 0.0)
     {
	gc = (RGBA_Gradient2_Color_Np_Stop *)gr->stops.stops;
	gc->r = r;  gc->g = g;  gc->b = b;  gc->a = a;  gc->dist = 0;
        if (a < 255) gr->has_alpha = 1;
	return;
     }
   if (pos == 1.0)
     {
	gc = (RGBA_Gradient2_Color_Np_Stop *)(gr->stops.stops->last);
	gc->r = r;  gc->g = g;  gc->b = b;  gc->a = a;  gc->dist = 0;
	if (a < 255) gr->has_alpha = 1;
	return;
     }

   l = gr->stops.stops->next;
   while (l)
     {
	gc = (RGBA_Gradient2_Color_Np_Stop *)l;
	if (pos <= gc->pos)
	  {
	    if (pos == gc->pos)
	      {
		gc->r = r;  gc->g = g;  gc->b = b;  gc->a = a;  gc->dist = 0;
		if (a < 255) gr->has_alpha = 1;
		return;
	      }
	    break;
	  }
	l = l->next;
     }

   gc = malloc(sizeof(RGBA_Gradient2_Color_Np_Stop));
   if (!gc) return;
   gc->r = r;
   gc->g = g;
   gc->b = b;
   gc->a = a;
   gc->pos = pos;
   gc->dist = 0;

   gr->stops.stops = evas_object_list_prepend_relative(gr->stops.stops, gc, l);
   gr->stops.nstops++;
   if (a < 255)
	gr->has_alpha = 1;
}

EAPI void
evas_common_gradient2_fill_transform_set(RGBA_Gradient2 *gr, Evas_Common_Transform *t)
{
   if (!gr) return;
   if (!t)
     {
	gr->fill.transform.mxx = 1;  gr->fill.transform.mxy = 0;  gr->fill.transform.mxz = 0;
	gr->fill.transform.myx = 0;  gr->fill.transform.myy = 1;  gr->fill.transform.myz = 0;
	gr->fill.transform.mzx = 1;  gr->fill.transform.mzy = 0;  gr->fill.transform.mzz = 1;
	return;
     }
   gr->fill.transform.mxx = t->mxx;  gr->fill.transform.mxy = t->mxy;  gr->fill.transform.mxz = t->mxz;
   gr->fill.transform.myx = t->myx;  gr->fill.transform.myy = t->myy;  gr->fill.transform.myz = t->myz;
   gr->fill.transform.mzx = t->mzx;  gr->fill.transform.mzy = t->mzy;  gr->fill.transform.mzz = t->mzz;
}

EAPI void
evas_common_gradient2_fill_spread_set(RGBA_Gradient2 *gr, int spread)
{
   if (!gr) return;
   gr->fill.spread = spread;
}

EAPI void
evas_common_gradient2_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
			   int x, int y, int w, int h, RGBA_Gradient2 *gr)
{
   Gfx_Func_Gradient2_Fill   gfunc;
   RGBA_Gfx_Func            bfunc;
   int             len;
   int             xin, yin, xoff, yoff;
   int             clx, cly, clw, clh;
   DATA32          *pdst, *dst_end, *buf, *map;
   RGBA_Image      *argb_buf = NULL, *alpha_buf = NULL;
   DATA8           *mask = NULL;
   void            *gdata;
   int             direct_copy = 0, buf_step = 0;

   if (!dst || !dc || !gr || !dst || !dst->image.data)
	return;
   if (!gr->map.data || !gr->type.geometer)
	return;
   if ((w < 1) || (h < 1))
	return;
   clx = 0;  cly = 0;  clw = dst->cache_entry.w;  clh = dst->cache_entry.h;
   if ((clw < 1) || (clh < 1))  return;

   if (dc->clip.use)
     RECTS_CLIP_TO_RECT(clx,cly,clw,clh, dc->clip.x,dc->clip.y,dc->clip.w,dc->clip.h);
   if ((clw < 1) || (clh < 1))  return;

   xin = x;  yin = y;
   RECTS_CLIP_TO_RECT(x,y,w,h, clx,cly,clw,clh);
   if ((w < 1) || (h < 1))  return;

   xoff = (x - xin);
   yoff = (y - yin);

   if (!gr->type.geometer->has_mask(gr, dc->render_op))
     {
	if ((dc->render_op == _EVAS_RENDER_FILL) ||
             (dc->render_op == _EVAS_RENDER_COPY))
	  {
	     direct_copy = 1;  buf_step = dst->cache_entry.w;
	     if (gr->type.geometer->has_alpha(gr, dc->render_op))
	        dst->cache_entry.flags.alpha = 1;
	  }
	else if ((dc->render_op == _EVAS_RENDER_BLEND) &&
	          !gr->type.geometer->has_alpha(gr, dc->render_op))
	  {
	     direct_copy = 1;  buf_step = dst->cache_entry.w;
	  }
     }
   if (!direct_copy)
     {
	argb_buf = evas_common_image_line_buffer_obtain(w);
	if (!argb_buf)
	   return;
	argb_buf->cache_entry.flags.alpha = gr->type.geometer->has_alpha(gr, dc->render_op) ? 1 : 0;

	if (gr->type.geometer->has_mask(gr, dc->render_op))
	  {
	    alpha_buf = evas_common_image_alpha_line_buffer_obtain(w);
	    if (!alpha_buf)
	      {
		evas_common_image_line_buffer_release(argb_buf);
		return;
	      }
	    bfunc = evas_common_gfx_func_composite_pixel_mask_span_get(argb_buf, dst, w, dc->render_op);
	  }
	else
	   bfunc = evas_common_gfx_func_composite_pixel_span_get(argb_buf, dst, w, dc->render_op);
     }

   gfunc = gr->type.geometer->get_fill_func(gr, dc->render_op);
   gdata = gr->type.gdata;
   if (!gdata)
     {
	if (!direct_copy)
	  {
	    evas_common_image_line_buffer_release(argb_buf);
	    if (alpha_buf)
		evas_common_image_alpha_line_buffer_release(alpha_buf);
	  }
	return;
     }

   map = gr->map.data;
   len = gr->map.len;
   pdst = dst->image.data + (y * dst->cache_entry.w) + x;
   dst_end = pdst + (h * dst->cache_entry.w);
   if (!direct_copy)
     {
	buf = argb_buf->image.data;
	if (alpha_buf)
	   mask = (DATA8 *)alpha_buf->image.data;
     }
   else
	buf = pdst;

   while (pdst < dst_end)
     {
#ifdef EVAS_SLI
	if ((y % dc->sli.h) == dc->sli.y)
#endif
	  {
	     gfunc(map, len, buf, mask, w, xoff, yoff, gdata);
	     if (!direct_copy)
	       bfunc(buf, mask, 0, pdst, w);
	  }
	buf += buf_step;
	pdst += dst->cache_entry.w;
	yoff++;
#ifdef EVAS_SLI
	y++;
#endif
     }

   if (!direct_copy)
     {
	evas_common_image_line_buffer_release(argb_buf);
	if (alpha_buf)
	   evas_common_image_alpha_line_buffer_release(alpha_buf);
     }
   evas_common_cpu_end_opt();
}

static void
_evas_common_gradient2_stops_scale(RGBA_Gradient2 *gr)
{
   Evas_Object_List  *l;
   RGBA_Gradient2_Color_Np_Stop  *gc, *gc_next;
   double  scale;
   int  len;

   if (!gr || !gr->stops.stops) return;

   scale = 1.0;
   gc = (RGBA_Gradient2_Color_Np_Stop *)gr->stops.stops;
   l = gr->stops.stops->next;
   while (l)
     {
	double dp;

	gc_next = (RGBA_Gradient2_Color_Np_Stop *)l;
	dp = gc_next->pos - gc->pos;
	if (dp > 0.000030517)
	    scale = MIN(scale, dp);
	gc = gc_next;
	l = l->next;
    }

   scale = 2.0 / scale;
   len = 1;
   gc = (RGBA_Gradient2_Color_Np_Stop *)gr->stops.stops;
   l = gr->stops.stops->next;
   while (l)
     {
	int dist;

	gc_next = (RGBA_Gradient2_Color_Np_Stop *)l;
	dist = 0.5 + (scale * (gc_next->pos - gc->pos));
	if (dist < 1)
	   dist = 1;
	len += dist;
	gc->dist = dist;
	gc = gc_next;
	l = l->next;
     }

   if (len > 65535)
	len = 65535;
   gr->stops.len = len;
}

static void
_evas_common_gradient2_map_argb(RGBA_Draw_Context *dc, RGBA_Gradient2 *gr, int len)
{
   if (!gr || !dc)
	return;
   if ((len < 1) || (len > 65535))
     {
	if (gr->map.data)
	   free(gr->map.data);
	gr->map.data = NULL;
	gr->map.len = 0;
	return;
     }
   if ((len != gr->map.len) || (!gr->map.data))
	gr->map.data = realloc(gr->map.data, len * sizeof(DATA32));
   if (!gr->map.data)
     { gr->map.len = 0; return; }
   gr->map.len = len;
   gr->map.has_alpha = gr->has_alpha;

   if (!gr->stops.stops) return;

   _evas_common_gradient2_stops_scale(gr);

     {
	Evas_Object_List  *lc;
	RGBA_Gradient2_Color_Np_Stop  *gc, *gc_next;
	DATA32  *pmap, *map_end;
	DATA8   *pamap = NULL;
	int   i, dii;
	int   r, g, b, a;
	int   next_r, next_g, next_b, next_a;
	int   rr, drr, gg, dgg, bb, dbb, aa, daa;

	gr->stops.cdata = realloc(gr->stops.cdata, gr->stops.len * sizeof(DATA32));
	if (!gr->stops.cdata)  return;
	pmap = gr->stops.cdata;  map_end = pmap + gr->stops.len;

	if (gr->has_alpha)
	  {
	    gr->stops.adata = realloc(gr->stops.adata, gr->stops.len * sizeof(DATA8));
	    if (!gr->stops.adata)
	      { free(gr->stops.cdata);  gr->stops.cdata = NULL;  return; }
	    pamap = gr->stops.adata;
	  }

	gc = (RGBA_Gradient2_Color_Np_Stop *)gr->stops.stops;
	r = gc->r;  g = gc->g;  b = gc->b;  a = gc->a;
	lc = gr->stops.stops->next;

	while (pmap < map_end)
	  {
	    if (lc)
	      {
		i = gc->dist;
		dii = 65536 / i;
		gc_next = (RGBA_Gradient2_Color_Np_Stop *)lc;
		next_r = gc_next->r;  next_g = gc_next->g;  next_b = gc_next->b;  next_a = gc_next->a;
		rr = r << 16;  drr = ((next_r - r) * dii);
		gg = g << 16;  dgg = ((next_g - g) * dii);
		bb = b << 16;  dbb = ((next_b - b) * dii);
		aa = a << 16;  daa = ((next_a - a) * dii);
		while (i--)
        	  {
		    r = rr >> 16;  r += (rr - (r << 16)) >> 15;
		    g = gg >> 16;  g += (gg - (g << 16)) >> 15;
		    b = bb >> 16;  b += (bb - (b << 16)) >> 15;
		    *pmap++ = 0xff000000 + RGB_JOIN(r,g,b);
		    if (pamap)
		      {
			a = aa >> 16;  a += (aa - (a << 16)) >> 15;
			*pamap++ = a;
			aa += daa;
		      }
		    rr += drr;  gg += dgg;  bb += dbb;
		  }
		gc = gc_next;
		r = next_r;  g = next_g;  b = next_b;  a = next_a;
		lc = lc->next;
              }
	    else
	      {
		*pmap++ = 0xff000000 + RGB_JOIN(gc->r,gc->g,gc->b);
		if (pamap) *pamap++ = gc->a;
	      }
	  }
     }

   if (gr->stops.cdata && gr->stops.adata)
     {
	evas_common_scale_rgba_a8_span(gr->stops.cdata, gr->stops.adata, gr->stops.len,
					dc->mul.col, gr->map.data, gr->map.len, 1);
	return;
     }

   evas_common_scale_rgba_span(gr->stops.cdata, NULL, gr->stops.len,
				dc->mul.col, gr->map.data, gr->map.len, 1);
   gr->map.has_alpha |= (!!(255 - (dc->mul.col >> 24)));
}

static void
_evas_common_gradient2_map_ahsv(RGBA_Draw_Context *dc, RGBA_Gradient2 *gr, int len)
{
   if (!gr || !dc)
	return;
   if ((len < 1) || (len > 65535))
     {
	if (gr->map.data)
	   free(gr->map.data);
	gr->map.data = NULL;
	gr->map.len = 0;
	return;
     }
   if ((len != gr->map.len) || (!gr->map.data))
	gr->map.data = realloc(gr->map.data, len * sizeof(DATA32));
   if (!gr->map.data)
     { gr->map.len = 0; return; }
   gr->map.len = len;
   gr->map.has_alpha = gr->has_alpha;

   if (!gr->stops.stops) return;

   _evas_common_gradient2_stops_scale(gr);

     {
	Evas_Object_List  *lc;
	RGBA_Gradient2_Color_Np_Stop  *gc, *gc_next;
	DATA32  *pmap, *map_end;
	DATA8   *pamap = NULL;
	int   i, dii;
	int   h, s, v;
	int   next_h, next_s, next_v;
	int   hh, dhh, ss, dss, vv, dvv, aa, daa;
	int   r, g, b, a;
	int   next_r, next_g, next_b, next_a;

	gr->stops.cdata = realloc(gr->stops.cdata, gr->stops.len * sizeof(DATA32));
	if (!gr->stops.cdata)  return;
	pmap = gr->stops.cdata;  map_end = pmap + gr->stops.len;

	if (gr->has_alpha)
	  {
	    gr->stops.adata = realloc(gr->stops.adata, gr->stops.len * sizeof(DATA8));
	    if (!gr->stops.adata)
	      { free(gr->stops.cdata);  gr->stops.cdata = NULL;  return; }
	    pamap = gr->stops.adata;
	  }

	gc = (RGBA_Gradient2_Color_Np_Stop *)gr->stops.stops;
	r = gc->r;  g = gc->g;  b = gc->b;  a = gc->a;
	evas_common_convert_color_rgb_to_hsv_int(r, g, b, &h, &s, &v);
	lc = gr->stops.stops->next;

	while (pmap < map_end)
	  {
	    if (lc)
	      {
		i = gc->dist;
		dii = 65536 / i;
		gc_next = (RGBA_Gradient2_Color_Np_Stop *)lc;

		next_r = gc_next->r;  next_g = gc_next->g;  next_b = gc_next->b;  next_a = gc_next->a;
		evas_common_convert_color_rgb_to_hsv_int(next_r, next_g, next_b,
							 &next_h, &next_s, &next_v);
		hh = h << 16;  dhh = ((next_h - h) * dii);
		ss = s << 16;  dss = ((next_s - s) * dii);
		vv = v << 16;  dvv = ((next_v - v) * dii);
		aa = a << 16;  daa = ((next_a - a) * dii);
		while (i--)
        	  {
		    h = hh >> 16;  h += (hh - (h << 16)) >> 15;
		    s = ss >> 16;  s += (ss - (s << 16)) >> 15;
		    v = vv >> 16;  v += (vv - (v << 16)) >> 15;
		    evas_common_convert_color_hsv_to_rgb_int(h, s, v, &r, &g, &b);
		    *pmap++ = 0xff000000 + RGB_JOIN(r,g,b);
		    if (pamap)
		      {
			a = aa >> 16;  a += (aa - (a << 16)) >> 15;
			*pamap++ = a;
			aa += daa;
		      }
		    hh += dhh;  ss += dss;  vv += dvv;
        	  }
		gc = gc_next;
		h = next_h;  s = next_s;  v = next_v;  a = next_a;
		lc = lc->next;
              }
	    else
	      {
		*pmap++ = 0xff000000 + RGB_JOIN(gc->r,gc->g,gc->b);
		if (pamap) *pamap++ = gc->a;
	      }
	  }
     }

   if (gr->stops.cdata && gr->stops.adata)
     {
	evas_common_scale_hsva_a8_span(gr->stops.cdata, gr->stops.adata, gr->stops.len,
					   dc->mul.col, gr->map.data, gr->map.len, 1);
	return;
     }
   evas_common_scale_hsva_span(gr->stops.cdata, NULL, gr->stops.len,
				dc->mul.col, gr->map.data, gr->map.len, 1);
   gr->map.has_alpha |= (!!(255 - (dc->mul.col >> 24)));
}

EAPI void
evas_common_gradient2_map(RGBA_Draw_Context *dc, RGBA_Gradient2 *gr, int len)
{
   if (!gr || !dc) return;
   if (dc->interpolation.color_space == _EVAS_COLOR_SPACE_AHSV)
     {
	_evas_common_gradient2_map_ahsv(dc, gr, len);
	return;
     }
   _evas_common_gradient2_map_argb(dc, gr, len);
}
