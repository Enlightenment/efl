#include "evas_engine.h"
#include "Evas_Engine_GL_Glew.h"


/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   HWND                 win;
   HDC                  dc;
   HGLRC                context;
   Evas_GL_Glew_Window *window;
   int                  end;
};

static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_GL_Glew *info;

   info = calloc(1, sizeof(Evas_Engine_Info_GL_Glew));
   if (!info) return NULL;

   info->magic.magic = rand();

   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_GL_Glew *in;

   in = (Evas_Engine_Info_GL_Glew *)info;
   free(in);
}

static int
eng_setup(Evas *e, void *in)
{
   Render_Engine            *re;
   Evas_Engine_Info_GL_Glew *info;

   info = (Evas_Engine_Info_GL_Glew *)in;
   if (!e->engine.data.output)
     {
	re = calloc(1, sizeof(Render_Engine));
	if (!re) return 0;

        if (!evas_glew_init(info->info.window, &re->dc, &re->context))
          {
	     free(re);
	     e->engine.data.output = NULL;
	     return 0;
          }

        re->win = info->info.window;
	e->engine.data.output = re;
	re->window = eng_window_new(info->info.window,
                                    re->dc,
                                    re->context,
                                    info->info.depth,
                                    e->output.w,
                                    e->output.h);
	if (!re->window)
	  {
	     free(re);
	     e->engine.data.output = NULL;
	     return 0;
	  }

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
     }
   else
     {

        if (!evas_glew_init(info->info.window, &re->dc, &re->context))
          {
	     free(re);
	     e->engine.data.output = NULL;
	     return 0;
          }

        re->win = info->info.window;
	re = e->engine.data.output;
	eng_window_free(re->window);
	re->window = eng_window_new(info->info.window,
                                    re->dc,
                                    re->context,
                                    info->info.depth,
                                    e->output.w,
                                    e->output.h);
     }
   if (!e->engine.data.output) return 0;

   if (!e->engine.data.context)
     e->engine.data.context =
       e->engine.func->context_new(e->engine.data.output);

   return 1;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_free(re->window);
   evas_glew_shutdown(re->win, re->dc, re->context);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->window->width = w;
   re->window->height = h;
   evas_gl_common_context_resize(re->window->gl_context, w, h);
}

static void
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_context_resize(re->window->gl_context, re->window->width, re->window->height);
   /* simple bounding box */
   if (!re->window->draw.redraw)
     {
#if 0
	re->window->draw.x1 = x;
	re->window->draw.y1 = y;
	re->window->draw.x2 = x + w - 1;
	re->window->draw.y2 = y + h - 1;
#else
	re->window->draw.x1 = 0;
	re->window->draw.y1 = 0;
	re->window->draw.x2 = re->window->width - 1;
	re->window->draw.y2 = re->window->height - 1;
#endif
     }
   else
     {
	if (x < re->window->draw.x1) re->window->draw.x1 = x;
	if (y < re->window->draw.y1) re->window->draw.y1 = y;
	if ((x + w - 1) > re->window->draw.x2) re->window->draw.x2 = x + w - 1;
	if ((y + h - 1) > re->window->draw.y2) re->window->draw.y2 = y + h - 1;
     }
   re->window->draw.redraw = 1;
}

static void
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->window->draw.redraw = 0;
}

#define SLOW_GL_COPY_RECT 1

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* get the upate rect surface - return engine data as dummy */
   if (!re->window->draw.redraw)
     {
//	printf("GL: NO updates!\n");
	return NULL;
     }
//   printf("GL: update....!\n");
#ifdef SLOW_GL_COPY_RECT
   /* if any update - just return the whole canvas - works with swap
    * buffers then */
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = re->window->width;
   if (h) *h = re->window->height;
   if (cx) *cx = 0;
   if (cy) *cy = 0;
   if (cw) *cw = re->window->width;
   if (ch) *ch = re->window->height;
#else
   /* 1 update - INCREDIBLY SLOW if combined with swap_rect in flush. a gl
    * problem where there just is no hardware path for somethnig that
    * obviously SHOULD be there */
   /* only 1 update to minimise gl context games and rendering multiple update
    * regions as evas does with other engines
    */
   if (x) *x = re->window->draw.x1;
   if (y) *y = re->window->draw.y1;
   if (w) *w = re->window->draw.x2 - re->window->draw.x1 + 1;
   if (h) *h = re->window->draw.y2 - re->window->draw.y1 + 1;
   if (cx) *cx = re->window->draw.x1;
   if (cy) *cy = re->window->draw.y1;
   if (cw) *cw = re->window->draw.x2 - re->window->draw.x1 + 1;
   if (ch) *ch = re->window->draw.y2 - re->window->draw.y1 + 1;
#endif
   return re;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* put back update surface.. in this case just unflag redraw */
   re->window->draw.redraw = 0;
   re->window->draw.drew = 1;
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!re->window->draw.drew) return;

   re->window->draw.drew = 0;
   eng_window_use(re->window);

#ifdef SLOW_GL_COPY_RECT
   SwapBuffers(re->dc);
#else
   /* SLOW AS ALL HELL */
   evas_gl_common_swap_rect(re->window->gl_context,
			    re->window->draw.x1, re->window->draw.y1,
			    re->window->draw.x2 - re->window->draw.x1 + 1,
			    re->window->draw.y2 - re->window->draw.y1 + 1);
#endif
}

static void
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
eng_context_cutout_add(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
#ifndef EVAS_GL_COMMON_NOCUTOUTS
   re->window->gl_context->dc = context;
   evas_common_draw_context_add_cutout(context, x, y, w, h);
#endif
}

static void
eng_context_cutout_clear(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
#ifndef EVAS_GL_COMMON_NOCUTOUTS
   re->window->gl_context->dc = context;
   evas_common_draw_context_clear_cutouts(context);
#endif
}

static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->window);
   re->window->gl_context->dc = context;
   evas_gl_common_rect_draw(re->window->gl_context, x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->window->gl_context->dc = context;
   evas_gl_common_line_draw(re->window->gl_context, x1, y1, x2, y2);
}

static void *
eng_polygon_point_add(void *data, void *context, void *polygon, int x, int y)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_poly_point_add(polygon, x, y);

}

static void *
eng_polygon_points_clear(void *data, void *context, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_poly_points_clear(polygon);
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->window->gl_context->dc = context;
   evas_gl_common_poly_draw(re->window->gl_context, polygon);
}

static void *
eng_gradient_new(void *data)
{
   return evas_gl_common_gradient_new();
}

static void
eng_gradient_color_stop_add(void *data, void *gradient, int r, int g, int b, int a, int delta)
{
   evas_gl_common_gradient_color_stop_add(gradient, r, g, b, a, delta);
}

static void
eng_gradient_alpha_stop_add(void *data, void *gradient, int a, int delta)
{
   evas_gl_common_gradient_alpha_stop_add(gradient, a, delta);
}

static void
eng_gradient_clear(void *data, void *gradient)
{
   evas_gl_common_gradient_clear(gradient);
}

static void
eng_gradient_color_data_set(void *data, void *gradient, void *map, int len, int has_alpha)
{
   evas_gl_common_gradient_color_data_set(gradient, map, len, has_alpha);
}

static void
eng_gradient_alpha_data_set(void *data, void *gradient, void *alpha_map, int len)
{
   evas_gl_common_gradient_alpha_data_set(gradient, alpha_map, len);
}

static void
eng_gradient_free(void *data, void *gradient)
{
   evas_gl_common_gradient_free(gradient);
}

static void
eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h)
{
   evas_gl_common_gradient_fill_set(gradient, x, y, w, h);
}

static void
eng_gradient_fill_angle_set(void *data, void *gradient, double angle)
{
   evas_gl_common_gradient_fill_angle_set(gradient, angle);
}

static void
eng_gradient_fill_spread_set(void *data, void *gradient, int spread)
{
   evas_gl_common_gradient_fill_spread_set(gradient, spread);
}

static void
eng_gradient_angle_set(void *data, void *gradient, double angle)
{
   evas_gl_common_gradient_map_angle_set(gradient, angle);
}

static void
eng_gradient_offset_set(void *data, void *gradient, float offset)
{
   evas_gl_common_gradient_map_offset_set(gradient, offset);
}

static void
eng_gradient_direction_set(void *data, void *gradient, int direction)
{
   evas_gl_common_gradient_map_direction_set(gradient, direction);
}

static void
eng_gradient_type_set(void *data, void *gradient, char *name, char *params)
{
   evas_gl_common_gradient_type_set(gradient, name, params);
}

static int
eng_gradient_is_opaque(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re = (Render_Engine *)data;

   re->window->gl_context->dc = context;
   return evas_gl_common_gradient_is_opaque(re->window->gl_context, gradient, x, y, w, h);
}

static int
eng_gradient_is_visible(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re = (Render_Engine *)data;

   re->window->gl_context->dc = context;
   return evas_gl_common_gradient_is_visible(re->window->gl_context, gradient, x, y, w, h);
}

static void
eng_gradient_render_pre(void *data, void *context, void *gradient)
{
   Render_Engine *re = (Render_Engine *)data;

   re->window->gl_context->dc = context;
   evas_gl_common_gradient_render_pre(re->window->gl_context, gradient);
}

static void
eng_gradient_render_post(void *data, void *gradient)
{
   evas_gl_common_gradient_render_post(gradient);
}

static void
eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->window);
   re->window->gl_context->dc = context;
   evas_gl_common_gradient_draw(re->window->gl_context, gradient, x, y, w, h);
}

static int
eng_image_alpha_get(void *data, void *image)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image) return 1;
   im = image;
   /* FIXME: can move to gl_common */
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (im->im->cache_entry.flags.alpha) return 1;
      default:
	break;
     }
   return 0;
}

static int
eng_image_colorspace_get(void *data, void *image)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image) return EVAS_COLORSPACE_ARGB8888;
   im = image;
   return im->cs.space;
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   eng_window_use(re->window);
   im = image;
   /* FIXME: can move to gl_common */
   if (im->cs.space != EVAS_COLORSPACE_ARGB8888) return im;
   if ((has_alpha) && (im->im->cache_entry.flags.alpha)) return image;
   else if ((!has_alpha) && (!im->im->cache_entry.flags.alpha)) return image;
   if (im->references > 1)
    {
	Evas_GL_Image *im_new;

        im_new = evas_gl_common_image_new_from_copied_data(im->gc, im->im->cache_entry.w, im->im->cache_entry.h, im->im->image.data,
							   eng_image_alpha_get(data, image),
							   eng_image_colorspace_get(data, image));
	if (!im_new) return im;
	evas_gl_common_image_free(im);
	im = im_new;
     }
   else
     evas_gl_common_image_dirty(im);
   im->im->cache_entry.flags.alpha = has_alpha ? 1 : 0;
   return image;
}

static void *
eng_image_border_set(void *data, void *image, int l, int r, int t, int b)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return image;
}

static void
eng_image_border_get(void *data, void *image, int *l, int *r, int *t, int *b)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static char *
eng_image_comment_get(void *data, void *image, char *key)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   im = image;
   return im->im->info.comment;
}

static char *
eng_image_format_get(void *data, void *image)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   im = image;
   return NULL;
}

static void
eng_image_colorspace_set(void *data, void *image, int cspace)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image) return;
   im = image;
   /* FIXME: can move to gl_common */
   if (im->cs.space == cspace) return;
   evas_cache_image_colorspace(&im->im->cache_entry, cspace);
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
	if (im->tex) evas_gl_common_texture_free(im->tex);
	im->tex = NULL;
	if (im->cs.data)
	  {
	     if (!im->cs.no_free) free(im->cs.data);
	  }
	im->cs.data = calloc(1, im->im->cache_entry.h * sizeof(unsigned char *) * 2);
	im->cs.no_free = 0;
	break;
      default:
	abort();
	break;
     }
   im->cs.space = cspace;
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

   re = (Render_Engine *)data;
   *error = 0;
   eng_window_use(re->window);
   return evas_gl_common_image_load(re->window->gl_context, file, key, lo);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->window);
   return evas_gl_common_image_new_from_data(re->window->gl_context, w, h, image_data, alpha, cspace);
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->window);
   return evas_gl_common_image_new_from_copied_data(re->window->gl_context, w, h, image_data, alpha, cspace);
}

static void
eng_image_free(void *data, void *image)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return;
   eng_window_use(re->window);
   evas_gl_common_image_free(image);
}

static void
eng_image_size_get(void *data, void *image, int *w, int *h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image)
     {
	*w = 0;
	*h = 0;
	return;
     }
   if (w) *w = ((Evas_GL_Image *)image)->im->cache_entry.w;
   if (h) *h = ((Evas_GL_Image *)image)->im->cache_entry.h;

}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;
   Evas_GL_Image *im, *im_old;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   eng_window_use(re->window);
   im_old = image;
   if ((eng_image_colorspace_get(data, image) == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (eng_image_colorspace_get(data, image) == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;
   if ((im_old) && (im_old->im->cache_entry.w == w) && (im_old->im->cache_entry.h == h))

     return image;
   if (im_old)
     {
	im = evas_gl_common_image_new(re->window->gl_context, w, h,
				      eng_image_alpha_get(data, image),
				      eng_image_colorspace_get(data, image));
/*
	evas_common_load_image_data_from_file(im_old->im);
	if (im_old->im->image->data)
	  {
	     evas_common_blit_rectangle(im_old->im, im->im, 0, 0, w, h, 0, 0);
	     evas_common_cpu_end_opt();
	  }
 */
	evas_gl_common_image_free(im_old);
     }
   else
     im = evas_gl_common_image_new(re->window->gl_context, w, h, 1, EVAS_COLORSPACE_ARGB8888);
   return im;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   evas_gl_common_image_dirty(image);
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image)
     {
	*image_data = NULL;
	return NULL;
     }
   im = image;
   eng_window_use(re->window);
   evas_cache_image_load_data(&im->im->cache_entry);
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (to_write)
	  {
	     if (im->references > 1)
	       {
		  Evas_GL_Image *im_new;

		  im_new = evas_gl_common_image_new_from_copied_data(im->gc, im->im->cache_entry.w, im->im->cache_entry.h, im->im->image.data,
								     eng_image_alpha_get(data, image),
								     eng_image_colorspace_get(data, image));
		  if (!im_new)
		    {
		       *image_data = NULL;
		       return im;
		    }
		  evas_gl_common_image_free(im);
		  im = im_new;
	       }
	     else
	       evas_gl_common_image_dirty(im);
	  }
	*image_data = im->im->image.data;
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	*image_data = im->cs.data;
	break;
      default:
	abort();
	break;
     }
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im, *im2;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   im = image;
   eng_window_use(re->window);
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (image_data != im->im->image.data)
	  {
	     int w, h;

	     w = im->im->cache_entry.w;
	     h = im->im->cache_entry.h;
	     im2 = eng_image_new_from_data(data, w, h, image_data,
					   eng_image_alpha_get(data, image),
					   eng_image_colorspace_get(data, image));
	     if (!im2) return im;
	     evas_gl_common_image_free(im);
	     im = im2;
	  }
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
        if (image_data != im->cs.data)
	  {
	     if (im->cs.data)
	       {
		  if (!im->cs.no_free) free(im->cs.data);
	       }
	     im->cs.data = image_data;
	  }
	break;
      default:
	abort();
	break;
     }
   /* hmmm - but if we wrote... why bother? */
   evas_gl_common_image_dirty(im);
   return im;
}

static void
eng_image_data_preload_request(void *data, void *image, const void *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return ;
   im = (RGBA_Image*) gim->im;
   if (!im) return ;
   evas_cache_image_preload_data(&im->cache_entry, target);
}

static void
eng_image_data_preload_cancel(void *data, void *image, const void *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return ;
   im = (RGBA_Image*) gim->im;
   if (!im) return ;
   evas_cache_image_preload_cancel(&im->cache_entry, target);
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return;
   eng_window_use(re->window);
   re->window->gl_context->dc = context;
   evas_gl_common_image_draw(re->window->gl_context, image,
			     src_x, src_y, src_w, src_h,
			     dst_x, dst_y, dst_w, dst_h,
			     smooth);
}

static void
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, const char *text)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
     {
	static RGBA_Image *im = NULL;

	if (!im)
          im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
        if (!im) return ;
        evas_cache_image_surface_alloc(&im->cache_entry, re->window->width, re->window->height);

	evas_common_draw_context_font_ext_set(context,
					      re->window->gl_context,
					      evas_gl_font_texture_new,
					      evas_gl_font_texture_free,
					      evas_gl_font_texture_draw);
	evas_common_font_draw(im, context, font, x, y, text);
	evas_common_draw_context_font_ext_set(context,
					      NULL,
					      NULL,
					      NULL,
					      NULL);
     }
}


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
   ORD(context_cutout_add);
   ORD(context_cutout_clear);
   ORD(output_flush);
   ORD(output_idle_flush);
   ORD(rectangle_draw);
   ORD(line_draw);
   ORD(polygon_point_add);
   ORD(polygon_points_clear);
   ORD(polygon_draw);
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
   "gl_glew",
   "none"
};
