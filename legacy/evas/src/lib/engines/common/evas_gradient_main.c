/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <math.h>

#include "evas_common.h"
#include "evas_convert_main.h"
#include "evas_convert_color.h"


static void _get_word(char *in, char *key);
static void evas_common_gradient_map_argb(RGBA_Draw_Context *dc, RGBA_Gradient *gr, int len);
static void evas_common_gradient_map_ahsv(RGBA_Draw_Context *dc, RGBA_Gradient *gr, int len);

static  int grad_initialised = 0;

static void
_get_word(char *in, char *key)
{
   char  *p, *pp;
   int    l;

   if (!key) return;
   *key = 0;
   if (!in || !*in) return;

   p = in;
   while (*p && isspace(*p))
	p++;
   if (!*p) return;
   pp = p;
   while (*pp && !isspace(*pp))
	pp++;
   l = pp - p;
   if (l >= 255) return;
   *(p + l) = 0;
   strncpy(key, p, l + 1);
}

static void
_evas_common_gradient_stops_free(RGBA_Gradient *gr)
{
   if (!gr) return;
   if (gr->color.stops)
     {
	Evas_Object_List *l;

	while (gr->color.stops)
	  {
	   l = gr->color.stops;
	   gr->color.stops = evas_object_list_remove(gr->color.stops, gr->color.stops);
	   free(l);
	  }
	gr->color.stops = NULL;
	gr->color.nstops = 0;
     }
   if (gr->alpha.stops)
     {
	Evas_Object_List *l;

	while (gr->alpha.stops)
	  {
	   l = gr->alpha.stops;
	   gr->alpha.stops = evas_object_list_remove(gr->alpha.stops, gr->alpha.stops);
	   free(l);
	  }
	gr->alpha.stops = NULL;
	gr->alpha.nstops = 0;
     }
}


char *
evas_common_gradient_get_key_fval(char *in, char *key, float *val)
{
   char   *p, *pp, sval[256];

   if (!key || !val) return NULL;
   *key = 0;
   if (!in || !*in) return NULL;
   p = strchr(in, '=');
   if (!p || !*p)  return NULL;
   *p = 0;  p++;
   if (!*p)  return NULL;
   pp = strchr(p, ';');
   if (!pp || !*pp)  return NULL;
   _get_word(in, key);
   if (!*key)  return NULL;
   *pp = 0;
   _get_word(p, sval);
   if (!sval[0])  return NULL;
   *val = atof(sval);
   return (pp + 1);
}

EAPI void
evas_common_gradient_init(void)
{
   RGBA_Gradient_Type  *geom;

   if (grad_initialised)
	return;
   geom = evas_common_gradient_geometer_get("linear");
   if (geom)
	geom->init();
   geom = evas_common_gradient_geometer_get("radial");
   if (geom)
	geom->init();
   geom = evas_common_gradient_geometer_get("rectangular");
   if (geom)
	geom->init();
   geom = evas_common_gradient_geometer_get("angular");
   if (geom)
	geom->init();
   geom = evas_common_gradient_geometer_get("sinusoidal");
   if (geom)
	geom->init();
   grad_initialised = 1;
}

void
evas_common_gradient_shutdown(void)
{
   RGBA_Gradient_Type  *geom;

   if (!grad_initialised)
	return;
   geom = evas_common_gradient_geometer_get("linear");
   if (geom)
	geom->shutdown();
   geom = evas_common_gradient_geometer_get("radial");
   if (geom)
	geom->shutdown();
   geom = evas_common_gradient_geometer_get("rectangular");
   if (geom)
	geom->shutdown();
   geom = evas_common_gradient_geometer_get("angular");
   if (geom)
	geom->shutdown();
   geom = evas_common_gradient_geometer_get("sinusoidal");
   if (geom)
	geom->shutdown();
   grad_initialised = 0;
}

EAPI RGBA_Gradient *
evas_common_gradient_new(void)
{
   RGBA_Gradient *gr;

   gr = calloc(1, sizeof(RGBA_Gradient));
   gr->references = 1;
   return gr;
}

EAPI void
evas_common_gradient_free(RGBA_Gradient *gr)
{
   if (!gr) return;
   gr->references--;
   if (gr->references > 0) return;
   evas_common_gradient_clear(gr);
   if (gr->type.name) free(gr->type.name);
   if (gr->type.params) free(gr->type.params);
   if (gr->type.geometer && gr->type.gdata)
	gr->type.geometer->geom_free(gr->type.gdata);
   if (gr->map.data) free(gr->map.data);
   free(gr);
}

EAPI void
evas_common_gradient_clear(RGBA_Gradient *gr)
{
   if (!gr) return;

   _evas_common_gradient_stops_free(gr);

   if (gr->color.data && !gr->imported_data)
	free(gr->color.data);
   gr->color.data = NULL;
   gr->color.len = 0;
   if (gr->alpha.data && !gr->imported_data)
	free(gr->alpha.data);
   gr->alpha.data = NULL;
   gr->alpha.len = 0;

   gr->imported_data = 0;
   gr->has_alpha = 0;
}

EAPI void
evas_common_gradient_color_stop_add(RGBA_Gradient *gr, int r, int g, int b, int a, int dist)
{
   RGBA_Gradient_Color_Stop *gc, *gcm, *gc_last;

   if (!gr) return;
   if (gr->imported_data)
     {
	gr->color.data = NULL;
	gr->color.len = 0;
	gr->alpha.data = NULL;
	gr->alpha.len = 0;
	gr->imported_data = 0;
	gr->has_alpha = 0;
     }
   gc = malloc(sizeof(RGBA_Gradient_Color_Stop));
   if (!gc) return;
   if (dist < 1) dist = 1;
   if (dist > 32768) dist = 32768;
   if (r < 0) r = 0;  if (r > 255) r = 255;
   if (g < 0) g = 0;  if (g > 255) g = 255;
   if (b < 0) b = 0;  if (b > 255) b = 255;
   if (a < 0) a = 0;  if (a > 255) a = 255;
   gc->r = r;
   gc->g = g;
   gc->b = b;
   gc->a = a;
   gc->dist = dist;

   if (!gr->color.stops)
     {
	gr->color.stops = evas_object_list_append(gr->color.stops, gc);
	gr->color.nstops = 1;
	gr->color.len = 1;
	if (a < 255)
	   gr->has_alpha = 1;
	return;
     }
   gcm = malloc(sizeof(RGBA_Gradient_Color_Stop));
   if (!gcm) { free(gc); return; }
   gc_last = (RGBA_Gradient_Color_Stop *)(gr->color.stops->last);
   if ((dist + gc_last->dist + gr->color.len) > 65535)
	{ free(gc); free(gcm); return; }
   gcm->r = (gc_last->r + r) / 2;
   gcm->g = (gc_last->g + g) / 2;
   gcm->b = (gc_last->b + b) / 2;
   gcm->a = (gc_last->a + a) / 2;
   gcm->dist = dist;
   gr->color.stops = evas_object_list_append(gr->color.stops, gcm);
   gr->color.len += gc_last->dist;
   gr->color.stops = evas_object_list_append(gr->color.stops, gc);
   gr->color.len += dist;
   gr->color.nstops += 2;
   if (a < 255)
	gr->has_alpha = 1;
}

EAPI void
evas_common_gradient_alpha_stop_add(RGBA_Gradient *gr, int a, int dist)
{
   RGBA_Gradient_Alpha_Stop *ga, *gam, *ga_last;

   if (!gr) return;
   if (gr->imported_data)
     {
	gr->color.data = NULL;
	gr->color.len = 0;
	gr->alpha.data = NULL;
	gr->alpha.len = 0;
	gr->imported_data = 0;
	gr->has_alpha = 0;
     }
   ga = malloc(sizeof(RGBA_Gradient_Alpha_Stop));
   if (!ga) return;
   if (dist < 1) dist = 1;
   if (dist > 32768) dist = 32768;
   if (a < 0) a = 0;  if (a > 255) a = 255;
   ga->a = a;
   ga->dist = dist;

   if (!gr->alpha.stops)
     {
	gr->alpha.stops = evas_object_list_append(gr->alpha.stops, ga);
	gr->alpha.nstops = 1;
	gr->alpha.len = 1;
	if (a < 255)
	   gr->has_alpha = 1;
	return;
     }
   gam = malloc(sizeof(RGBA_Gradient_Alpha_Stop));
   if (!gam) { free(ga); return; }
   ga_last = (RGBA_Gradient_Alpha_Stop *)(gr->alpha.stops->last);
   if ((dist + ga_last->dist + gr->alpha.len) > 65535)
	{ free(ga); free(gam); return; }
   gam->a = (ga_last->a + a) / 2;
   gam->dist = dist;
   gr->alpha.stops = evas_object_list_append(gr->alpha.stops, gam);
   gr->alpha.len += ga_last->dist;
   gr->alpha.stops = evas_object_list_append(gr->alpha.stops, ga);
   gr->alpha.len += dist;
   gr->alpha.nstops += 2;
   if (a < 255)
	gr->has_alpha = 1;
}

EAPI void
evas_common_gradient_color_data_set(RGBA_Gradient *gr, DATA32 *data, int len, int alpha_flags)
{
   if (!gr) return;
   if (!gr->imported_data)
	evas_common_gradient_clear(gr);
   if (len < 1) data = NULL;
   if (!data) len = 0;
   gr->color.data = data;
   gr->color.len = len;
   gr->has_alpha = !!alpha_flags;
   gr->imported_data = 1;
}

EAPI void
evas_common_gradient_alpha_data_set(RGBA_Gradient *gr, DATA8 *data, int len)
{
   if (!gr) return;
   if (!gr->imported_data)
	evas_common_gradient_clear(gr);
   if (len < 1) data = NULL;
   if (!data) len = 0;
   gr->alpha.data = data;
   gr->alpha.len = len;
   gr->has_alpha = 1;
   gr->imported_data = 1;
}

EAPI void
evas_common_gradient_type_set(RGBA_Gradient *gr, const char *name, char *params)
{
   RGBA_Gradient_Type   *geometer;

   if (!gr) return;
   if (!name || !*name)
	name = "linear";

   geometer = evas_common_gradient_geometer_get(name);
   if (!geometer) return;
   if (gr->type.gdata && (geometer != gr->type.geometer))
     {
	if (gr->type.geometer)
	   gr->type.geometer->geom_free(gr->type.gdata);
	gr->type.gdata = NULL;
     }
   gr->type.geometer = geometer;

   if (gr->type.name) free(gr->type.name);
   gr->type.name = strdup(name);

   if (params && !*params) 
	params = NULL;
   if (gr->type.params) free(gr->type.params);
   gr->type.params = NULL;
   if (params) gr->type.params = strdup(params);

   gr->type.geometer->geom_init(gr);
}

EAPI void
evas_common_gradient_fill_set(RGBA_Gradient *gr, int x, int y, int w, int h)
{
   if (!gr) return;
   gr->fill.x = x;
   gr->fill.y = y;
   if ((w < 1) && (h < 1))
     { w = h = 1; }
   gr->fill.w = w;
   gr->fill.h = h;
}

EAPI void
evas_common_gradient_fill_angle_set(RGBA_Gradient *gr, float angle)
{
   if (!gr) return;
   gr->fill.angle = angle;
}

EAPI void
evas_common_gradient_fill_spread_set(RGBA_Gradient *gr, int spread)
{
   if (!gr) return;
   gr->fill.spread = spread;
}

EAPI void
evas_common_gradient_map_offset_set(RGBA_Gradient *gr, float offset)
{
   if (!gr) return;
   gr->map.offset = offset;
}

EAPI void
evas_common_gradient_map_direction_set(RGBA_Gradient *gr, int direction)
{
   if (!gr) return;
   gr->map.direction = (direction >= 0 ? 1 : -1);
}

EAPI void
evas_common_gradient_map_angle_set(RGBA_Gradient *gr, float angle)
{
   if (!gr) return;
   gr->map.angle = angle;
}

EAPI RGBA_Gradient_Type  *
evas_common_gradient_geometer_get(const char *name)
{
   RGBA_Gradient_Type  *geom = NULL;

   if (!name || !*name)
	name = "linear";
   if (!strcmp(name,"linear") || !strcmp(name,"linear.diag") || !strcmp(name,"linear.codiag"))
	geom = evas_common_gradient_linear_get();
   else if (!strcmp(name,"radial"))
	geom = evas_common_gradient_radial_get();
   else if (!strcmp(name,"angular"))
	geom = evas_common_gradient_angular_get();
   else if (!strcmp(name,"sinusoidal"))
	geom = evas_common_gradient_sinusoidal_get();
   else if (!strcmp(name,"rectangular"))
	geom = evas_common_gradient_rectangular_get();
   if (!geom)
	geom = evas_common_gradient_linear_get();
   return geom;
}

EAPI void
evas_common_gradient_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
			  int x, int y, int w, int h, RGBA_Gradient *gr)
{
   Gfx_Func_Gradient_Fill   gfunc;
   RGBA_Gfx_Func            bfunc;
   int             len;
   int             xin, yin, xoff, yoff;
   int             clx, cly, clw, clh;
   int             axx, axy, ayx, ayy;
   DATA32          *pdst, *dst_end, *buf, *map;
   RGBA_Image      *argb_buf = NULL, *alpha_buf = NULL;
   DATA8           *mask = NULL;
   void            *gdata;
   float           angle;
   int             direct_copy = 0, buf_step = 0;

   if (!dst || !dc || !gr || !dst || !dst->image.data)
     return;
   if (!gr->map.data || !gr->type.geometer)
     return;
   if ((gr->fill.w < 1) || (gr->fill.h < 1))
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

   xoff = (x - xin) - gr->fill.x;
   yoff = (y - yin) - gr->fill.y;

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

   gfunc = gr->type.geometer->get_fill_func(gr, dc->render_op, dc->anti_alias);
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

   /* I TOLD YOU! this here STOPS the gradeint bugs. it's a missing
    * emms() before doing floating point operations! the thread pipe code
    * just brought it out reliably. i swear i had seen this long before i
    * ever added the thread/pipe code.
    * 
    * now here is why it happens. NO drawing function... EXCEPT
    * evas_common_polygon_draw() and evas_common_gradient_draw() use floating
    * point for drawing (the poly stuff should really lose it actually), but
    * nicely nestled in the poly draw code is a evas_common_cpu_end_opt()
    * before it does any operations that would use floating point. the fact
    * is the gradient code was LUCKY before without the thread pipes to almost
    * all the time have another func do a evas_common_cpu_end_opt() before it
    * was called. that was no longer the case with the thread renderer and
    * it suffered. that is why on amd systems it seemed to work as i beileve
    * on amd cpu's the amms done by evas_common_cpu_end_opt() is not needed
    * to do floatingpoint ops again.
    * 
    * after a lot of futzing about - this was the culprit (well axx and axy
    * were garbage values eventually i found after much debugging and i traced
    * their garbageness back to here).
    */
   evas_common_cpu_end_opt();
   
   angle = (gr->fill.angle * M_PI) / 180.0;
   axx = (cos(angle) * 65536.0);
   ayy = axx;
   axy = (sin(angle) * 65536.0);
   ayx = -axy;

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
	if (((yoff + y) % dc->sli.h) == dc->sli.y)
#endif
	  {
	     gfunc(map, len, buf, mask, w, xoff, yoff, axx, axy, ayx, ayy, gdata);
	     evas_common_cpu_end_opt();
	     if (!direct_copy)
	       bfunc(buf, mask, 0, pdst, w);
	     evas_common_cpu_end_opt();
	  }
	buf += buf_step;
	pdst += dst->cache_entry.w;
	yoff++;
     }
   
   if (!direct_copy)
     {
	evas_common_image_line_buffer_release(argb_buf);
	if (alpha_buf)
	   evas_common_image_alpha_line_buffer_release(alpha_buf);
     }
}

static void
evas_common_gradient_map_argb(RGBA_Draw_Context *dc, RGBA_Gradient *gr, int len)
{
   DATA32   color;
   int      mul_use;

   if (!gr || !dc)
	return;
   if (len < 1)
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

   color = dc->mul.col;
   mul_use = dc->mul.use;
   if (dc->mul.col == 0xffffffff)
	mul_use = 0;

   if ((!gr->imported_data) && (!gr->color.stops) && (!gr->alpha.stops))
     {
	static DATA32  p = 0xffffffff;

	gr->color.data = &p;
	gr->color.len = 1;
	gr->imported_data = 1;
	gr->has_alpha = 0;
     }

   if (gr->color.stops)
     {
	Evas_Object_List  *lc;
	RGBA_Gradient_Color_Stop  *gc, *gc_next;
	DATA32  *pmap, *map_end;
	int   i, dii;
	int   r, g, b, a;
	int   next_r, next_g, next_b, next_a;
	int   rr, drr, gg, dgg, bb, dbb, aa, daa;
	int   mr = 256, mg = 256, mb = 256, ma = 256;

	gr->color.data = realloc(gr->color.data, gr->color.len * sizeof(DATA32));
	if (!gr->color.data)  return;

	gc = (RGBA_Gradient_Color_Stop *)gr->color.stops;
	r = gc->r;  g = gc->g;  b = gc->b;  a = gc->a;
	if (mul_use)
	  {
	    mr = 1 + ((color >> 16) & 0xff);  mg = 1 + ((color >> 8) & 0xff);
	    mb = 1 + ((color) & 0xff);  ma = 1 + (color >> 24);
	    if (ma < 256)
	       gr->map.has_alpha = 1;
	    r = (r * mr) >> 8;  g = (g * mg) >> 8;
	    b = (b * mb) >> 8;  a = (a * ma) >> 8;
          }
	lc = gr->color.stops->next;
	pmap = gr->color.data;  map_end = pmap + gr->color.len;

	while (pmap < map_end)
	  {
	    if (lc)
	      {
		i = gc->dist;
		dii = 65536 / i;
		gc_next = (RGBA_Gradient_Color_Stop *)lc;
		next_r = gc_next->r;  next_g = gc_next->g;
		next_b = gc_next->b;  next_a = gc_next->a;
		if (mul_use)
        	  {
		    next_r = (next_r * mr) >> 8;  next_g = (next_g * mg) >> 8;
		    next_b = (next_b * mb) >> 8;  next_a = (next_a * ma) >> 8;
        	  }
		rr = r << 16;  drr = ((next_r - r) * dii);
		gg = g << 16;  dgg = ((next_g - g) * dii);
		bb = b << 16;  dbb = ((next_b - b) * dii);
		aa = a << 16;  daa = ((next_a - a) * dii);
		while (i--)
        	  {
		    r = rr >> 16;  r += (rr - (r << 16)) >> 15;
		    g = gg >> 16;  g += (gg - (g << 16)) >> 15;
		    b = bb >> 16;  b += (bb - (b << 16)) >> 15;
		    a = aa >> 16;  a += (aa - (a << 16)) >> 15;
		    *pmap++ = ARGB_JOIN(a,r,g,b);
		    rr += drr;  gg += dgg;  bb += dbb;  aa += daa;
        	  }
		gc = gc_next;
		r = next_r; g = next_g; b = next_b; a = next_a;
		lc = lc->next;
              }
	    else
		*pmap++ = ARGB_JOIN(a,r,g,b);
	  }
     }

   if (gr->alpha.stops)
     {
	Evas_Object_List  *lc;
	RGBA_Gradient_Alpha_Stop  *ga, *ga_next;
	DATA8  *pamap, *amap_end;
	int   i, dii;
	int   a, next_a;
	int   aa, daa;

	gr->alpha.data = realloc(gr->alpha.data, gr->alpha.len * sizeof(DATA8));
	if (!gr->alpha.data)  return;

	ga = (RGBA_Gradient_Alpha_Stop *)gr->alpha.stops;
	a = ga->a;
	lc = gr->alpha.stops->next;
	pamap = gr->alpha.data;  amap_end = pamap + gr->alpha.len;

	while (pamap < amap_end)
	  {
	    if (lc)
	      {
		i = ga->dist;
		dii = 65536 / i;
		ga_next = (RGBA_Gradient_Alpha_Stop *)lc;
		next_a = ga_next->a;
		aa = a << 16;  daa = ((next_a - a) * dii);
		while (i--)
        	  {
		    a = aa >> 16;  a += (aa - (a << 16)) >> 15;
		    *pamap++ = a;
		    aa += daa;
        	  }
		ga = ga_next;
		a = next_a;
		lc = lc->next;
              }
	    else
		*pamap++ = a;
	  }
     }

   if (gr->color.data && gr->alpha.data)
     {
	if (!gr->imported_data)
	   color = 0xffffffff;
	if (gr->color.len == gr->alpha.len)
	  {
	    evas_common_scale_rgba_a8_span(gr->color.data, gr->alpha.data, gr->color.len,
					   color, gr->map.data, gr->map.len, gr->map.direction);
	    return;
	  }
	evas_common_scale_rgba_span(gr->color.data, NULL, gr->color.len,
				    color, gr->map.data, gr->map.len, gr->map.direction);
	evas_common_scale_clip_a8_span(NULL, gr->alpha.data, gr->alpha.len,
					0xffffffff, gr->map.data, gr->map.len, gr->map.direction);
	return;
     }

   if (gr->color.data)
     {
	if (!gr->imported_data)
	   color = 0xffffffff;
	evas_common_scale_rgba_span(gr->color.data, NULL, gr->color.len,
				    color, gr->map.data, gr->map.len, gr->map.direction);
	gr->map.has_alpha |= (!!(255 - (color >> 24)));
	return;
     }
	
   evas_common_scale_a8_span(NULL, gr->alpha.data, gr->alpha.len,
			     color, gr->map.data, gr->map.len, gr->map.direction);
}

static void
evas_common_gradient_map_ahsv(RGBA_Draw_Context *dc, RGBA_Gradient *gr, int len)
{
   DATA32   color;

   if (!gr || !dc)
	return;
   if (len < 1)
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

   color = dc->mul.col;
   if (!dc->mul.use)
	color = 0xffffffff;

   if ((!gr->imported_data) && (!gr->color.stops) && (!gr->alpha.stops))
     {
	static DATA32  p = 0xffffffff;

	gr->color.data = &p;
	gr->color.len = 1;
	gr->imported_data = 1;
	gr->has_alpha = 0;
     }

   if (gr->color.stops)
     {
	Evas_Object_List  *lc;
	RGBA_Gradient_Color_Stop  *gc, *gc_next;
	DATA32  *pmap, *map_end;
	int   i, dii;
	int   h, s, v;
	int   next_h, next_s, next_v;
	int   hh, dhh, ss, dss, vv, dvv, aa, daa;
	int   r, g, b, a;
	int   next_r, next_g, next_b, next_a;

	gr->color.data = realloc(gr->color.data, gr->color.len * sizeof(DATA32));
	if (!gr->color.data)  return;

	gc = (RGBA_Gradient_Color_Stop *)gr->color.stops;
	r = gc->r;  g = gc->g;  b = gc->b;  a = gc->a;
	evas_common_convert_color_rgb_to_hsv_int(r, g, b, &h, &s, &v);

	lc = gr->color.stops->next;
	pmap = gr->color.data;  map_end = pmap + gr->color.len;

	while (pmap < map_end)
	  {
	    if (lc)
	      {
		i = gc->dist;
		dii = 65536 / i;
		gc_next = (RGBA_Gradient_Color_Stop *)lc;

		next_r = gc_next->r;  next_g = gc_next->g;
		next_b = gc_next->b;  next_a = gc_next->a;
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
		    a = aa >> 16;  a += (aa - (a << 16)) >> 15;
		    evas_common_convert_color_hsv_to_rgb_int(h, s, v, &r, &g, &b);
		    *pmap++ = ARGB_JOIN(a,r,g,b);
		    hh += dhh;  ss += dss;  vv += dvv;  aa += daa;
        	  }
		gc = gc_next;
		h = next_h; s = next_s; v = next_v; a = next_a;
		lc = lc->next;
              }
	    else
		*pmap++ = ARGB_JOIN(gc->a,gc->r,gc->g,gc->b);
	  }
     }

   if (gr->alpha.stops)
     {
	Evas_Object_List  *lc;
	RGBA_Gradient_Alpha_Stop  *ga, *ga_next;
	DATA8  *pamap, *amap_end;
	int   i, dii;
	int   a, next_a;
	int   aa, daa;

	gr->alpha.data = realloc(gr->alpha.data, gr->alpha.len * sizeof(DATA8));
	if (!gr->alpha.data)  return;

	ga = (RGBA_Gradient_Alpha_Stop *)gr->alpha.stops;
	a = ga->a;
	lc = gr->alpha.stops->next;
	pamap = gr->alpha.data;  amap_end = pamap + gr->alpha.len;

	while (pamap < amap_end)
	  {
	    if (lc)
	      {
		i = ga->dist;
		dii = 65536 / i;
		ga_next = (RGBA_Gradient_Alpha_Stop *)lc;
		next_a = ga_next->a;
		aa = a << 16;  daa = ((next_a - a) * dii);
		while (i--)
        	  {
		    a = aa >> 16;  a += (aa - (a << 16)) >> 15;
		    *pamap++ = a;
		    aa += daa;
        	  }
		ga = ga_next;
		a = next_a;
		lc = lc->next;
              }
	    else
		*pamap++ = a;
	  }
     }

   if (gr->color.data && gr->alpha.data)
     {
	if (gr->color.len == gr->alpha.len)
	  {
	    evas_common_scale_hsva_a8_span(gr->color.data, gr->alpha.data, gr->color.len,
					   color, gr->map.data, gr->map.len, gr->map.direction);
	    return;
	  }
	evas_common_scale_hsva_span(gr->color.data, NULL, gr->color.len,
				    color, gr->map.data, gr->map.len, gr->map.direction);
	evas_common_scale_clip_a8_span(NULL, gr->alpha.data, gr->alpha.len,
					0xffffffff, gr->map.data, gr->map.len, gr->map.direction);
	return;
     }
   if (gr->color.data)
     {
	evas_common_scale_hsva_span(gr->color.data, NULL, gr->color.len,
				    color, gr->map.data, gr->map.len, gr->map.direction);
	gr->map.has_alpha |= (!!(255 - (color >> 24)));
	return;
     }
	
   evas_common_scale_a8_span(NULL, gr->alpha.data, gr->alpha.len,
			     color, gr->map.data, gr->map.len, gr->map.direction);
}

EAPI void
evas_common_gradient_map(RGBA_Draw_Context *dc, RGBA_Gradient *gr, int len)
{
   if (!gr || !dc) return;
   if (dc->interpolation.color_space == _EVAS_COLOR_SPACE_AHSV)
     {
	evas_common_gradient_map_ahsv(dc, gr, len);
	return;
     }
   evas_common_gradient_map_argb(dc, gr, len);
}
