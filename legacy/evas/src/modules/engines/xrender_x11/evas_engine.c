#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_X11.h"

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef struct _Render_Engine        Render_Engine;
typedef struct _Render_Engine_Update Render_Engine_Update;

struct _Render_Engine_Update
{
   int x, y, w, h;
   Xrender_Surface *surface;
};

struct _Render_Engine
{
   Display              *disp;
   Visual               *vis;
   Drawable              win;
   Pixmap                mask;
   unsigned char         destination_alpha : 1;
   
   Ximage_Info          *xinf;
   Xrender_Surface      *output;
   Xrender_Surface      *mask_output;
   
   Tilebuf              *tb;
   Tilebuf_Rect         *rects;
   Eina_Inlist          *cur_rect;
   int                   end : 1;
   
   Evas_List            *updates;
};

/* internal engine routines */

/* engine api this module provides */
static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_XRender_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_XRender_X11));
   if (!info) return NULL;
   info->magic.magic = rand();
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_XRender_X11 *in;

   in = (Evas_Engine_Info_XRender_X11 *)info;
   free(in);
}

static void
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_XRender_X11 *info;
   int resize = 1;

   info = (Evas_Engine_Info_XRender_X11 *)in;
   if (!e->engine.data.output)
     {
	re = calloc(1, sizeof(Render_Engine));
	evas_common_cpu_init();
	evas_common_blend_init();
	evas_common_image_init();
	evas_common_convert_init();
	evas_common_scale_init();
	evas_common_rectangle_init();
	evas_common_gradient_init();
	evas_common_polygon_init();
	evas_common_line_init();
	evas_common_font_init();
	evas_common_draw_init();
	evas_common_tilebuf_init();
	re->tb = evas_common_tilebuf_new(e->output.w, e->output.h);
	if (re->tb)
	  evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
	e->engine.data.output = re;
	resize = 0;
     }
   re = e->engine.data.output;
   if (!re) return;
   
   if (!e->engine.data.context) e->engine.data.context = e->engine.func->context_new(e->engine.data.output);
   
   re->disp = info->info.display;
   re->vis = info->info.visual;
   re->win = info->info.drawable;
   re->mask = info->info.mask;
   re->destination_alpha = info->info.destination_alpha;
   
   if (re->xinf) _xr_image_info_free(re->xinf);
   re->xinf = _xr_image_info_get(re->disp, re->win, re->vis);

   if (re->output) _xr_render_surface_free(re->output);
   if (re->mask_output) _xr_render_surface_free(re->mask_output);
   if (!re->mask)
     re->output = _xr_render_surface_adopt(re->xinf, re->win, e->output.w, e->output.h, re->destination_alpha);
   else
     re->output = _xr_render_surface_adopt(re->xinf, re->win, e->output.w, e->output.h, 0);
   if (re->mask)
     re->mask_output = _xr_render_surface_format_adopt(re->xinf, re->mask, 
						       e->output.w, e->output.h,
						       re->xinf->fmt1, 1);
   else
     re->mask_output = NULL;
   if (resize)
     {
	if (re->tb) evas_common_tilebuf_free(re->tb);
	if ((e->output.w > 0) && (e->output.h > 0))
	  re->tb = evas_common_tilebuf_new(e->output.w, e->output.h);
	else
	  re->tb = evas_common_tilebuf_new(1, 1);
        if (re->tb)
	  evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
     }
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_font_shutdown();
   evas_common_image_shutdown();
   while (re->updates)
     {
	Render_Engine_Update *reu;
	
	reu = re->updates->data;
	re->updates = evas_list_remove_list(re->updates, re->updates);
	_xr_render_surface_free(reu->surface);
	free(reu);
     }
   if (re->tb) evas_common_tilebuf_free(re->tb);
   if (re->output) _xr_render_surface_free(re->output);
   if (re->mask_output) _xr_render_surface_free(re->mask_output);
   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   if (re->xinf) _xr_image_info_free(re->xinf);
   free(re);
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re->output)
     {
	if ((re->output->w == w) && (re->output->h ==h)) return;
	if (re->output) _xr_render_surface_free(re->output);
     }
   re->output = _xr_render_surface_adopt(re->xinf, re->win, w, h, 0);
   if (re->mask_output)
     {
	if (re->mask_output) _xr_render_surface_free(re->mask_output);
	re->mask_output = _xr_render_surface_format_adopt(re->xinf, re->mask, 
							  w, h, 
							  re->xinf->fmt1, 1);
     }
   evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb) evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
}

static void
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_clear(re->tb);
}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;
   Tilebuf_Rect *rect;
   int ux, uy, uw, uh;

   re = (Render_Engine *)data;
   if (re->end)
     {
	re->end = 0;
	return NULL;
     }
   if (!re->rects)
     {
	re->rects = evas_common_tilebuf_get_render_rects(re->tb);
	re->cur_rect = EINA_INLIST_GET(re->rects);
     }
   if (!re->cur_rect) return NULL;
   rect = (Tilebuf_Rect *)re->cur_rect;
   ux = rect->x; uy = rect->y; uw = rect->w; uh = rect->h;
   re->cur_rect = re->cur_rect->next;
   if (!re->cur_rect)
     {
	evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	re->end = 1;
     }

   *x = ux; *y = uy; *w = uw; *h = uh;
   *cx = 0; *cy = 0; *cw = uw; *ch = uh;
   if ((re->destination_alpha) || (re->mask))
     {
	Xrender_Surface *surface;
	
	surface = _xr_render_surface_new(re->xinf, uw, uh, re->xinf->fmt32, 1);
	_xr_render_surface_solid_rectangle_set(surface, 0, 0, 0, 0, 0, 0, uw, uh);
	return surface;
     }
   return _xr_render_surface_new(re->xinf, uw, uh, re->xinf->fmtdef, 0);
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;
   Render_Engine_Update *reu;
   
   re = (Render_Engine *)data;
   reu = malloc(sizeof(Render_Engine_Update));
   if (!reu) return;
   reu->x = x;
   reu->y = y;
   reu->w = w;
   reu->h = h;
   reu->surface = (Xrender_Surface *)surface;
   re->updates = evas_list_append(re->updates, reu);
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   while (re->updates)
     {
	Render_Engine_Update *reu;
	
	reu = re->updates->data;
	re->updates = evas_list_remove_list(re->updates, re->updates);
	if (re->mask_output)
	  {
	     Xrender_Surface *tsurf;
	     
	     _xr_render_surface_copy(reu->surface, re->output, 0, 0,
				     reu->x, reu->y, reu->w, reu->h);
	     tsurf = _xr_render_surface_new(re->xinf, reu->w, reu->h, re->xinf->fmt1, 1);
	     if (tsurf)
	       {
		  _xr_render_surface_copy(reu->surface, tsurf, 0, 0,
					  0, 0, reu->w, reu->h);
		  _xr_render_surface_copy(tsurf, re->mask_output, 0, 0,
					  reu->x, reu->y, reu->w, reu->h);
		  _xr_render_surface_free(tsurf);
	       }
	  }
	else
	  {
	     _xr_render_surface_copy(reu->surface, re->output, 0, 0,
				     reu->x, reu->y, reu->w, reu->h);
	  }
	_xr_render_surface_free(reu->surface);
	free(reu);
     }
   XSync(re->disp, False);
   _xr_image_info_pool_flush(re->xinf, 0, 0);
}

static void
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   _xr_render_surface_rectangle_draw((Xrender_Surface *)surface,
				     (RGBA_Draw_Context *)context,
				     x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   _xr_render_surface_line_draw((Xrender_Surface *)surface, (RGBA_Draw_Context *)context, x1, y1, x2, y2);
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   _xre_poly_draw((Xrender_Surface *)surface, (RGBA_Draw_Context *)context, (RGBA_Polygon_Point *)polygon);
}


static void
eng_gradient2_color_np_stop_insert(void *data, void *gradient, int r, int g, int b, int a, float pos)
{
}

static void
eng_gradient2_clear(void *data, void *gradient)
{
}

static void
eng_gradient2_fill_transform_set(void *data, void *gradient, void *transform)
{
}

static void
eng_gradient2_fill_spread_set
(void *data, void *gradient, int spread)
{
}

static void *
eng_gradient2_linear_new(void *data)
{
   return NULL;
}

static void
eng_gradient2_linear_free(void *data, void *linear_gradient)
{
}

static void
eng_gradient2_linear_fill_set(void *data, void *linear_gradient, int x0, int y0, int x1, int y1)
{
}

static int
eng_gradient2_linear_is_opaque(void *data, void *context, void *linear_gradient, int x, int y, int w, int h)
{
   return 1;
}

static int
eng_gradient2_linear_is_visible(void *data, void *context, void *linear_gradient, int x, int y, int w, int h)
{
   return 1;
}

static void
eng_gradient2_linear_render_pre(void *data, void *context, void *linear_gradient)
{
}

static void
eng_gradient2_linear_render_post(void *data, void *linear_gradient)
{
}

static void
eng_gradient2_linear_draw(void *data, void *context, void *surface, void *linear_gradient, int x, int y, int w, int h)
{
}

static void *
eng_gradient2_radial_new(void *data)
{
   return NULL;
}

static void
eng_gradient2_radial_free(void *data, void *radial_gradient)
{
}

static void
eng_gradient2_radial_fill_set(void *data, void *radial_gradient, float cx, float cy, float rx, float ry)
{
}

static int
eng_gradient2_radial_is_opaque(void *data, void *context, void *radial_gradient, int x, int y, int w, int h)
{
   return 1;
}

static int
eng_gradient2_radial_is_visible(void *data, void *context, void *radial_gradient, int x, int y, int w, int h)
{
   return 1;
}

static void
eng_gradient2_radial_render_pre(void *data, void *context, void *radial_gradient)
{
}

static void
eng_gradient2_radial_render_post(void *data, void *radial_gradient)
{
}

static void
eng_gradient2_radial_draw(void *data, void *context, void *surface, void *radial_gradient, int x, int y, int w, int h)
{
}

static void *
eng_gradient_new(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   return _xre_gradient_new(re->xinf);
}

static void
eng_gradient_free(void *data, void *gradient)
{
   _xre_gradient_free(gradient);
}

static void
eng_gradient_color_stop_add(void *data, void *gradient, int r, int g, int b, int a, int delta)
{
   _xre_gradient_color_stop_add(gradient, r, g, b, a, delta);
}

static void
eng_gradient_alpha_stop_add(void *data, void *gradient, int a, int delta)
{
   _xre_gradient_alpha_stop_add(gradient, a, delta);
}

static void
eng_gradient_color_data_set(void *data, void *gradient, void *map, int len, int has_alpha)
{
   _xre_gradient_color_data_set(gradient, map, len, has_alpha);
}

static void
eng_gradient_alpha_data_set(void *data, void *gradient, void *alpha_map, int len)
{
   _xre_gradient_alpha_data_set(gradient, alpha_map, len);
}

static void
eng_gradient_clear(void *data, void *gradient)
{
   _xre_gradient_clear(gradient);
}

static void
eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h)
{
   _xre_gradient_fill_set(gradient, x, y, w, h);
}

static void
eng_gradient_fill_angle_set(void *data, void *gradient, double angle)
{
   _xre_gradient_fill_angle_set(gradient, angle);
}

static void
eng_gradient_fill_spread_set(void *data, void *gradient, int spread)
{
   _xre_gradient_fill_spread_set(gradient, spread);
}

static void
eng_gradient_angle_set(void *data, void *gradient, double angle)
{
   _xre_gradient_angle_set(gradient, angle);
}

static void
eng_gradient_offset_set(void *data, void *gradient, float offset)
{
   _xre_gradient_offset_set(gradient, offset);
}

static void
eng_gradient_direction_set(void *data, void *gradient, int direction)
{
   _xre_gradient_direction_set(gradient, direction);
}

static void
eng_gradient_type_set(void *data, void *gradient, char *name, char *params)
{
   _xre_gradient_type_set(gradient, name, params);
}

static int
eng_gradient_is_opaque(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   RGBA_Gradient  *grad;
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;

   if (!dc || !gradient) return 0;
   grad = ((XR_Gradient *)gradient)->grad;
   if(!grad || !grad->type.geometer)  return 0;
   return !(grad->type.geometer->has_alpha(grad, dc->render_op) |
              grad->type.geometer->has_mask(grad, dc->render_op));
}

static int
eng_gradient_is_visible(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   if (!context || !gradient)  return 0;
   return 1;
}

static void
eng_gradient_render_pre(void *data, void *context, void *gradient)
{
   int  len;
   RGBA_Gradient  *grad;

   if (!context || !gradient) return;
   grad = ((XR_Gradient *)gradient)->grad;
   if(!grad || !grad->type.geometer)  return;
   grad->type.geometer->geom_set(grad);
   len = grad->type.geometer->get_map_len(grad);
   evas_common_gradient_map(context, grad, len);
}

static void
eng_gradient_render_post(void *data, void *gradient)
{
}

static void
eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h)
{
   _xre_gradient_draw(surface, context, gradient, x, y, w, h);
}

static int
eng_image_alpha_get(void *data, void *image)
{
   if (!image) return 0;
   return _xre_image_alpha_get((XR_Image *)image);
}

static int
eng_image_colorspace_get(void *data, void *image)
{
   if (!image) return EVAS_COLORSPACE_ARGB8888;
   return ((XR_Image *)image)->cs.space;
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   XR_Image *im;
   
   im = (XR_Image *)image;
   if (!im) return im;
   if (im->cs.space != EVAS_COLORSPACE_ARGB8888) return im;
   if (((im->alpha) && (has_alpha)) || ((!im->alpha) && (!has_alpha))) 
     return im;
   if (im->references > 1)
     {
	XR_Image *old_im;
	
	old_im = im;
	im = _xre_image_copy(old_im);
	if (im)
	  {
	     im->alpha = old_im->alpha;
	     _xre_image_free(old_im);
	  }
	else
	  im = old_im;
     }
   else
     _xre_image_dirty(im);
   _xre_image_alpha_set(im, has_alpha);
   return im;
}

static void *
eng_image_border_set(void *data, void *image, int l, int r, int t, int b)
{
   if (!image) return image;
   _xre_image_border_set((XR_Image *)image, l, r, t, b);
   return image;
}

static void
eng_image_border_get(void *data, void *image, int *l, int *r, int *t, int *b)
{
   if (!image) return;
   _xre_image_border_get((XR_Image *)image, l, r, t, b);
}

static char *
eng_image_comment_get(void *data, void *image, char *key)
{
   if (!image) return NULL;
   return strdup(((XR_Image *)image)->comment);
}

static char *
eng_image_format_get(void *data, void *image)
{
   if (!image) return NULL;
   return ((XR_Image *)image)->format;
}

static void
eng_image_colorspace_set(void *data, void *image, int cspace)
{
   XR_Image *im;
      
   if (!image) return;
   im = (XR_Image *)image;
   if (im->cs.space == cspace) return;

   if (im->im) evas_cache_image_drop(&im->im->cache_entry);
   im->im = NULL;

   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (im->cs.data)
	  {
	     if (!im->cs.no_free) free(im->cs.data);
	     im->cs.data = NULL;
	     im->cs.no_free = 0;
	  }
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	if ((im->free_data) && (im->data)) free(im->data);
	im->data = NULL;
	if (im->cs.data)
	  {
	     if (!im->cs.no_free) free(im->cs.data);
	  }
	if (im->h > 0)
	  im->cs.data = calloc(1, im->h * sizeof(unsigned char *) * 2);
	im->cs.no_free = 0;
	break;
      default:
	abort();
	break;
     }
   im->cs.space = cspace;
   _xre_image_dirty(im);
   _xre_image_region_dirty(im, 0, 0, im->w, im->h);
}

static void
eng_image_native_set(void *data, void *image, void *native)
{
}

static void *
eng_image_native_get(void *data, void *image)
{
   return NULL;
}

static void *
eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re;
   XR_Image *im;
   
   re = (Render_Engine *)data;
   *error = 0;
   im = _xre_image_load(re->xinf, file, key, lo);
   return im;
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;
   XR_Image *im;
   
   re = (Render_Engine *)data;
   im = _xre_image_new_from_data(re->xinf, w, h, image_data, alpha, cspace);
   return im;
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;
   XR_Image *im;
   
   re = (Render_Engine *)data;
   im = _xre_image_new_from_copied_data(re->xinf, w, h, image_data, alpha, cspace);
   return im;
}

static void
eng_image_free(void *data, void *image)
{
   if (!image) return;
   _xre_image_free((XR_Image *)image);
}

static void
eng_image_size_get(void *data, void *image, int *w, int *h)
{
   if (!image) return;
   if (w) *w = ((XR_Image *)image)->w;
   if (h) *h = ((XR_Image *)image)->h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   XR_Image *im, *im_old;

   if (!image) return NULL;
   im_old = image;
   if ((im_old->cs.space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im_old->cs.space == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;
   if ((im_old) && (im_old->w == w) && (im_old->h == h))
     return image;
   if ((w <= 0) || (h <= 0))
     {
	_xre_image_free(im_old);
	return NULL;
     }
   if (im_old)
     {
	im = _xre_image_new_from_copied_data(im_old->xinf, w, h, NULL, im_old->alpha, im_old->cs.space);
	_xre_image_free(im_old);
	return im;
     }
   return image;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   if (!image) return image;
   _xre_image_dirty((XR_Image *)image);
   _xre_image_region_dirty((XR_Image *)image, x, y, w, h);
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   XR_Image *im;
   
   if (!image)
     {
	*image_data = NULL;
	return NULL;
     }
   im = (XR_Image *)image;
   if (im->im)
     evas_cache_image_load_data(&im->im->cache_entry);
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (to_write)
	  {
	     if (im->references > 1)
	       {
		  XR_Image *im_old;
		  
		  im_old = im;
		  im = _xre_image_copy(im_old);
		  if (im)
		    _xre_image_free(im_old);
		  else
		    im = im_old;
	       }
	     else
	       _xre_image_dirty(im);
	  }
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	break;
      default:
	abort();
	break;
     }
   if (image_data) *image_data = _xre_image_data_get(im);
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   XR_Image *im;
   
   if (!image) return image;
   im = (XR_Image *)image;
   
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (_xre_image_data_get(im) != image_data)
	  {
	     XR_Image *im_old;
	     
	     im_old = im;
	     image = _xre_image_data_find(image_data);
	     if (image != im_old)
	       {
		  if (!image)
		    {
		       image = _xre_image_new_from_data(im_old->xinf, im_old->w, im_old->h, image_data, im_old->alpha, EVAS_COLORSPACE_ARGB8888);
		       if (image)
			 {
			    ((XR_Image *)image)->alpha = im_old->alpha;
			    _xre_image_free(im_old);
			 }
		       else
			 image = im_old;
		    }
		  else
		    {
		       _xre_image_free(im_old);
		    }
	       }
	     else
	       {
		  _xre_image_free(image);
		  image = im_old;
	       }
	  }
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	if (_xre_image_data_get(im) != image_data)
	  {  
	     if (im->data)
	       {
		  if (im->free_data) free(im->data);
		  im->data = NULL;
	       }
             if (im->cs.data)
	       {
		  if (!im->cs.no_free) free(im->cs.data);
	       }
	     im->cs.data = image_data;
	     _xre_image_dirty(im);
	  }
        break;
      default:
	abort();
	break;
     }
   return image;
}

static void
eng_image_data_preload_request(void *data, void *image, const void *target)
{
   XR_Image *xim = image;
   RGBA_Image *im;

   if (!xim) return ;
   im = (RGBA_Image*) xim->im;
   if (!im) return ;
   evas_cache_image_preload_data(&im->cache_entry, target);
}

static void
eng_image_data_preload_cancel(void *data, void *image)
{
   XR_Image *xim = image;
   RGBA_Image *im;

   if (!xim) return ;
   im = (RGBA_Image*) xim->im;
   if (!im) return ;
   evas_cache_image_preload_cancel(&im->cache_entry);
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   if ((!image) || (!surface)) return;
   _xre_image_surface_gen((XR_Image *)image);
   if (((XR_Image *)image)->surface)
     _xr_render_surface_composite(((XR_Image *)image)->surface,
				  (Xrender_Surface *)surface,
				  (RGBA_Draw_Context *)context,
				  src_x, src_y, src_w, src_h,
				  dst_x, dst_y, dst_w, dst_h,
				  smooth);
}

static void
eng_image_cache_flush(void *data)
{
   int tmp_size;

   tmp_size = _xre_image_cache_get();
   pfunc.image_cache_flush(data);
   _xre_image_cache_set(0);
   _xre_image_cache_set(tmp_size);
}

static void
eng_image_cache_set(void *data, int bytes)
{
   pfunc.image_cache_set(data, bytes);
   _xre_image_cache_set(bytes);
}

static int
eng_image_cache_get(void *data)
{
   return pfunc.image_cache_get(data);
}

static void
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, const char *text)
{
   Render_Engine        *re;
   RGBA_Image           *im;

   re = (Render_Engine *)data;

   _xr_render_surface_clips_set((Xrender_Surface *)surface, (RGBA_Draw_Context *)context, x, y, w, h);

   im = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
                                             ((Xrender_Surface *)surface)->w,
                                             ((Xrender_Surface *)surface)->h,
                                             surface,
                                             0, EVAS_COLORSPACE_ARGB8888);
   evas_common_draw_context_font_ext_set(context,
                                         re->xinf,
                                         _xre_font_surface_new,
                                         _xre_font_surface_free,
                                         _xre_font_surface_draw);
   evas_common_font_draw(im, context, font, x, y, text);
   evas_common_draw_context_font_ext_set(context,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL);
   evas_common_cpu_end_opt();

   evas_cache_image_drop(&im->cache_entry);
}

/* module advertising code */
EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(output_free);
   ORD(output_resize);
   ORD(output_tile_size_set);
   ORD(output_redraws_rect_add);
   ORD(output_redraws_rect_del);
   ORD(output_redraws_clear);
   ORD(output_redraws_next_update_get);
   ORD(output_redraws_next_update_push);
   ORD(output_flush);
   ORD(output_idle_flush);
   ORD(rectangle_draw);
   ORD(line_draw);
   ORD(polygon_draw);

   ORD(gradient2_color_np_stop_insert);
   ORD(gradient2_clear);
   ORD(gradient2_fill_transform_set);
   ORD(gradient2_fill_spread_set);
   ORD(gradient2_linear_new);
   ORD(gradient2_linear_free);
   ORD(gradient2_linear_fill_set);
   ORD(gradient2_linear_is_opaque);
   ORD(gradient2_linear_is_visible);
   ORD(gradient2_linear_render_pre);
   ORD(gradient2_linear_render_post);
   ORD(gradient2_linear_draw);
   ORD(gradient2_radial_new);
   ORD(gradient2_radial_free);
   ORD(gradient2_radial_fill_set);
   ORD(gradient2_radial_is_opaque);
   ORD(gradient2_radial_is_visible);
   ORD(gradient2_radial_render_pre);
   ORD(gradient2_radial_render_post);
   ORD(gradient2_radial_draw);

   ORD(gradient_new);
   ORD(gradient_free);
   ORD(gradient_color_stop_add);
   ORD(gradient_alpha_stop_add);
   ORD(gradient_color_data_set);
   ORD(gradient_alpha_data_set);
   ORD(gradient_clear);
   ORD(gradient_fill_set);
   ORD(gradient_fill_angle_set);
   ORD(gradient_fill_spread_set);
   ORD(gradient_angle_set);
   ORD(gradient_offset_set);
   ORD(gradient_direction_set);
   ORD(gradient_type_set);
   ORD(gradient_is_opaque);
   ORD(gradient_is_visible);
   ORD(gradient_render_pre);
   ORD(gradient_render_post);
   ORD(gradient_draw);
   ORD(image_load);
   ORD(image_new_from_data);
   ORD(image_new_from_copied_data);
   ORD(image_free);
   ORD(image_size_get);
   ORD(image_size_set);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_data_preload_request);
   ORD(image_data_preload_cancel);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_border_set);
   ORD(image_border_get);
   ORD(image_draw);
   ORD(image_comment_get);
   ORD(image_format_get);
   ORD(image_colorspace_set);
   ORD(image_colorspace_get);
   ORD(image_native_set);
   ORD(image_native_get);
   ORD(image_cache_flush);
   ORD(image_cache_set);
   ORD(image_cache_get);
   ORD(font_draw);
   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

EAPI void
module_close(void)
{
}

EAPI Evas_Module_Api evas_modapi = 
{
   EVAS_MODULE_API_VERSION, 
     EVAS_MODULE_TYPE_ENGINE,
     "xrender_x11",
     "none"
};

