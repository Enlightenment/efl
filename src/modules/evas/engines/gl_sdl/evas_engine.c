#include "evas_common_private.h" /* Also includes international specific stuff */
#include "evas_private.h"
#include "evas_engine.h"

static void*                     _sdl_output_setup	(int w, int h, int fullscreen, int noframe);
                
int _evas_engine_GL_SDL_log_dom = -1;
/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

static void *
eng_info(Evas *e EINA_UNUSED)
{
   Evas_Engine_Info_GL_SDL *info;

   info = calloc(1, sizeof(Evas_Engine_Info_GL_SDL));
   if (!info) return NULL;
   info->magic.magic = rand();
   return info;
}

static void
eng_info_free(Evas *e EINA_UNUSED, void *info)
{
   Evas_Engine_Info_GL_SDL *in;
   in = (Evas_Engine_Info_GL_SDL *)info;
   free(in);
}

static int
eng_setup(Evas *eo_e, void *in)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Render_Engine *re;
   Evas_Engine_Info_GL_SDL *info;

   info = (Evas_Engine_Info_GL_SDL *)in;

   SDL_Init(SDL_INIT_NOPARACHUTE);

   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
     {
	ERR("SDL_Init failed with %s", SDL_GetError());
        SDL_Quit();
        return 0;
     }

   re = _sdl_output_setup(e->output.w, e->output.h,
			     info->flags.fullscreen,
			     info->flags.noframe);
   re->info = info;
   e->engine.data.output = re;
   if (!e->engine.data.output)
     return 0;

   e->engine.func = &func;
   e->engine.data.context = e->engine.func->context_new(e->engine.data.output);
   
   return 1;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   if (re)
     {
       evas_gl_common_context_free(re->gl_context);
       free(re);

       evas_common_font_shutdown();
       evas_common_image_shutdown();

       SDL_QuitSubSystem(SDL_INIT_VIDEO);
     }
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine	*re;
   SDL_Surface		*surface;

   re = (Render_Engine *)data;
   re->w = w;
   re->h = h;

   if(SDL_GetVideoSurface()->flags & SDL_RESIZABLE)
     {
	surface = SDL_SetVideoMode(w, h, 32, EVAS_SDL_GL_FLAG
		      | (re->info->flags.fullscreen ? SDL_FULLSCREEN : 0)
		      | (re->info->flags.noframe ? SDL_NOFRAME : 0));
	if (!surface)
	  {
	     ERR("Unable to change the resolution to : %ix%i", w, h);
	     SDL_Quit();
	     exit(-1);
	  }
     }

   evas_gl_common_context_resize(re->gl_context, w, h, re->gl_context->rot);
}

static void
eng_output_tile_size_set(void *data EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_context_resize(re->gl_context, re->w, re->h, re->gl_context->rot);
   /* smple bounding box */
   if (!re->draw.redraw)
     {
#if 0
	re->draw.x1 = x;
	re->draw.y1 = y;
	re->draw.x2 = x + w - 1;
	re->draw.y2 = y + h - 1;
#else
	re->draw.x1 = 0;
	re->draw.y1 = 0;
	re->draw.x2 = re->w - 1;
	re->draw.y2 = re->h - 1;
#endif
     }
   else
     {
	if (x < re->draw.x1) re->draw.x1 = x;
	if (y < re->draw.y1) re->draw.y1 = y;
	if ((x + w - 1) > re->draw.x2) re->draw.x2 = x + w - 1;
	if ((y + h - 1) > re->draw.y2) re->draw.y2 = y + h - 1;
     }
   re->draw.redraw = 1;
}

static void
eng_output_redraws_rect_del(void *data EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->draw.redraw = 0;
//   INF("GL: finish update cycle!");
}

/* at least the nvidia drivers are so abysmal that copying from the backbuffer
 * to the front using glCopyPixels() that you literally can WATCH it draw the
 * pixels slowly across the screen with a window update taking multiple
 * seconds - so workaround by doing a full buffer render as frankly GL isn't
 * up to doing anything that isn't done by quake (etc.)
 */
#define SLOW_GL_COPY_RECT 1
/* vsync games - not for now though */
//#define VSYNC_TO_SCREEN 1

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_context_flush(re->gl_context);
   evas_gl_common_context_newframe(re->gl_context);
   /* get the upate rect surface - return engine data as dummy */
   if (!re->draw.redraw)
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
   if (w) *w = re->w;
   if (h) *h = re->h;
   if (cx) *cx = 0;
   if (cy) *cy = 0;
   if (cw) *cw = re->w;
   if (ch) *ch = re->h;
#else
   /* 1 update - INCREDIBLY SLOW if combined with swap_rect in flush. a gl
    * problem where there just is no hardware path for somethnig that
    * obviously SHOULD be there */
   /* only 1 update to minimise gl context games and rendering multiple update
    * regions as evas does with other engines
    */
   if (x) *x = re->draw.x1;
   if (y) *y = re->draw.y1;
   if (w) *w = re->draw.x2 - re->draw.x1 + 1;
   if (h) *h = re->draw.y2 - re->draw.y1 + 1;
   if (cx) *cx = re->draw.x1;
   if (cy) *cy = re->draw.y1;
   if (cw) *cw = re->draw.x2 - re->draw.x1 + 1;
   if (ch) *ch = re->draw.y2 - re->draw.y1 + 1;
#endif
// clear buffer. only needed for dest alpha
//   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
//   glClear(GL_COLOR_BUFFER_BIT);
//x//   printf("frame -> new\n");
   return re->gl_context->def_surface;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED, Evas_Render_Mode render_mode)
{
   Render_Engine *re;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   re = (Render_Engine *)data;
   /* put back update surface.. in this case just unflag redraw */
   re->draw.redraw = 0;
   re->draw.drew = 1;
   evas_gl_common_context_flush(re->gl_context);
//x//   printf("frame -> push\n");
}

static void
eng_output_flush(void *data, Evas_Render_Mode render_mode)
{
   Render_Engine *re;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   re = (Render_Engine *)data;
   if (!re->draw.drew) return;
//x//   printf("frame -> flush\n");
   re->draw.drew = 0;

#if 0
#ifdef GL_GLES
//   glFlush();
   eglSwapBuffers(re->egl_disp, re->egl_surface[0]);
#else
   glXSwapBuffers(re->win->disp, re->win);
#endif   
#else
   SDL_GL_SwapBuffers();
#endif
}

static void
eng_output_idle_flush(void *data EINA_UNUSED)
{
}

static void
eng_output_dump(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
   evas_gl_common_image_all_unload(re->gl_context);
}

static void
eng_context_cutout_add(void *data EINA_UNUSED, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_add_cutout(context, x, y, w, h);
}

static void
eng_context_cutout_clear(void *data EINA_UNUSED, void *context)
{
   evas_common_draw_context_clear_cutouts(context);
}

static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h, Eina_Bool do_async EINA_UNUSED)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_context_target_surface_set(re->gl_context, surface);
   re->gl_context->dc = context;
   evas_gl_common_rect_draw(re->gl_context, x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2, Eina_Bool do_async EINA_UNUSED)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_context_target_surface_set(re->gl_context, surface);
   re->gl_context->dc = context;
   evas_gl_common_line_draw(re->gl_context, x1, y1, x2, y2);
}

static void *
eng_polygon_point_add(void *data EINA_UNUSED, void *context EINA_UNUSED, void *polygon, int x, int y)
{
   return evas_gl_common_poly_point_add(polygon, x, y);
}

static void *
eng_polygon_points_clear(void *data EINA_UNUSED, void *context EINA_UNUSED, void *polygon)
{
   return evas_gl_common_poly_points_clear(polygon);
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon, int x, int y, Eina_Bool do_async EINA_UNUSED)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_context_target_surface_set(re->gl_context, surface);
   re->gl_context->dc = context;
   evas_gl_common_poly_draw(re->gl_context, polygon, x, y);
}

static int
eng_image_alpha_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;

   return im ? im->alpha : 1;
}

static int
eng_image_colorspace_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;

   return im ? im->cs.space : EVAS_COLORSPACE_ARGB8888;
}

static void *
eng_image_alpha_set(void *data EINA_UNUSED, void *image, int has_alpha)
{
   Evas_GL_Image *im;

   if (!image) return NULL;
   im = image;
   if (im->native.data)
     {
        im->alpha = has_alpha;
        return image;
     }
   /* FIXME: can move to gl_common */
   if (im->cs.space != EVAS_COLORSPACE_ARGB8888) return im;
   if ((has_alpha) && (im->im->cache_entry.flags.alpha)) return image;
   else if ((!has_alpha) && (!im->im->cache_entry.flags.alpha)) return image;
   if (im->references > 1)
     {
        Evas_GL_Image *im_new;
        
        if (!im->im->image.data)
          evas_cache_image_load_data(&im->im->cache_entry);
        evas_gl_common_image_alloc_ensure(im);
        im_new = evas_gl_common_image_new_from_copied_data(im->gc, im->im->cache_entry.w, im->im->cache_entry.h, im->im->image.data,
                                                           eng_image_alpha_get(data, image),
                                                           eng_image_colorspace_get(data, image));
        if (!im_new) return im;
        evas_gl_common_image_free(im);
        im = im_new;
     }
   else
     evas_gl_common_image_dirty(im, 0, 0, 0, 0);
   im->im->cache_entry.flags.alpha = has_alpha ? 1 : 0;
   return image;
}

static void *
eng_image_border_set(void *data EINA_UNUSED, void *image, int l EINA_UNUSED, int r EINA_UNUSED, int t EINA_UNUSED, int b EINA_UNUSED)
{
   return image;
}

static void
eng_image_border_get(void *data EINA_UNUSED, void *image EINA_UNUSED, int *l EINA_UNUSED, int *r EINA_UNUSED, int *t EINA_UNUSED, int *b EINA_UNUSED)
{
}

static char *
eng_image_comment_get(void *data EINA_UNUSED, void *image, char *key EINA_UNUSED)
{
   Evas_GL_Image *im = image;

   if (im && im->im) return im->im->info.comment;
   return NULL;
}

static char *
eng_image_format_get(void *data EINA_UNUSED, void *image EINA_UNUSED)
{
   return NULL;
}

static void
eng_image_colorspace_set(void *data EINA_UNUSED, void *image, int cspace)
{
   Evas_GL_Image *im;

   if (!image) return;
   im = image;
   if (im->native.data) return;
   /* FIXME: can move to gl_common */
   if (im->cs.space == cspace) return;
   evas_gl_common_image_alloc_ensure(im);
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
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
        if (im->tex) evas_gl_common_texture_free(im->tex, EINA_TRUE);
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

/////////////////////////////////////////////////////////////////////////
//
//
typedef struct _Native Native;

struct _Native
{
   Evas_Native_Surface ns;
   
#ifdef GL_GLES
   EGLSurface  egl_surface;
#endif
};

static void *
eng_image_native_set(void *data EINA_UNUSED, void *image EINA_UNUSED, void *native EINA_UNUSED)
{
   return NULL;
}

static void *
eng_image_native_get(void *data EINA_UNUSED, void *image EINA_UNUSED)
{
   return NULL;
}

//
//
/////////////////////////////////////////////////////////////////////////

static void *
eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re = data;

   *error = EVAS_LOAD_ERROR_NONE;
   return evas_gl_common_image_load(re->gl_context, file, key, lo, error);
}

static void *
eng_image_mmap(void *data, Eina_File *f, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re = data;

   *error = EVAS_LOAD_ERROR_NONE;
   return evas_gl_common_image_mmap(re->gl_context, f, key, lo, error);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re = data;

   return evas_gl_common_image_new_from_data(re->gl_context, w, h, image_data, alpha, cspace);
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re = data;

   return evas_gl_common_image_new_from_copied_data(re->gl_context, w, h, image_data, alpha, cspace);
}

static void
eng_image_free(void *data EINA_UNUSED, void *image)
{
   if (image) evas_gl_common_image_free(image);
}

static void
eng_image_size_get(void *data EINA_UNUSED, void *image, int *w, int *h)
{
   if (!image)
     {
	*w = 0;
	*h = 0;
	return;
     }
   if (w) *w = ((Evas_GL_Image *)image)->w;
   if (h) *h = ((Evas_GL_Image *)image)->h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;
   Evas_GL_Image *im = image;
   Evas_GL_Image *im_old;
   
   re = (Render_Engine *)data;
   if (!im) return NULL;
   if (im->native.data)
     {
        im->w = w;
        im->h = h;
        return image;
     }
   if ((im->tex) && (im->tex->pt->dyn.img))
     {
        evas_gl_common_texture_free(im->tex, EINA_TRUE);
        im->tex = NULL;
        im->w = w;
        im->h = h;
        im->tex = evas_gl_common_texture_dynamic_new(im->gc, im);
        return image;
     }
   im_old = image;
   
   switch (eng_image_colorspace_get(data, image))
     {
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         w &= ~0x1;
         break;
     }

   evas_gl_common_image_alloc_ensure(im_old);
   if ((im_old->im) &&
       ((int)im_old->im->cache_entry.w == w) &&
       ((int)im_old->im->cache_entry.h == h))
     return image;
   if (im_old)
     {
   	im = evas_gl_common_image_new(re->gl_context, w, h,
   				      eng_image_alpha_get(data, image),
   				      eng_image_colorspace_get(data, image));
        evas_gl_common_image_free(im_old);
     }
   else
     im = evas_gl_common_image_new(re->gl_context, w, h, 1, EVAS_COLORSPACE_ARGB8888);
   return im;
}

static void *
eng_image_dirty_region(void *data EINA_UNUSED, void *image, int x, int y, int w, int h)
{
   Evas_GL_Image *im = image;

   if (!image) return NULL;
   if (im->native.data) return image;
   evas_gl_common_image_dirty(image, x, y, w, h);
   return image;
}

static void *
eng_image_data_get(void *data EINA_UNUSED, void *image, int to_write, DATA32 **image_data, int *err)
{
   Evas_GL_Image *im;
   int error;
   
   if (!image)
     {
	*image_data = NULL;
        if (err) *err = EVAS_LOAD_ERROR_GENERIC;
	return NULL;
     }
   im = image;
   if (im->native.data)
     {
        *image_data = NULL;
        if (err) *err = EVAS_LOAD_ERROR_NONE;
        return im;
     }
   error = evas_cache_image_load_data(&im->im->cache_entry);
   evas_gl_common_image_alloc_ensure(im);
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
                       if (err) *err = error;
                       return im;
   		    }
   		  evas_gl_common_image_free(im);
   		  im = im_new;
	       }
   	     else
   	       evas_gl_common_image_dirty(im, 0, 0, 0, 0);
	  }
	*image_data = im->im->image.data;
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
	*image_data = im->cs.data;
	break;
      default:
	abort();
	break;
     }
   if (err) *err = error;
   return im;
}

static void *
eng_image_data_put(void *data EINA_UNUSED, void *image, DATA32 *image_data)
{
   Evas_GL_Image *im, *im2;

   if (!image) return NULL;
   im = image;
   if (im->native.data) return image;
   evas_gl_common_image_alloc_ensure(im);
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
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
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
   evas_gl_common_image_dirty(im, 0, 0, 0, 0);
   return im;
}

static void
eng_image_data_preload_request(void *data EINA_UNUSED, void *image, const Eo *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;
   evas_cache_image_preload_data(&im->cache_entry, target, NULL, NULL, NULL);
}

static void
eng_image_data_preload_cancel(void *data EINA_UNUSED, void *image, const Eo *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;
   evas_cache_image_preload_cancel(&im->cache_entry, target);
}

static Eina_Bool
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth, Eina_Bool do_async EINA_UNUSED)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return EINA_FALSE;
   evas_gl_common_context_target_surface_set(re->gl_context, surface);
   re->gl_context->dc = context;
   evas_gl_common_image_draw(re->gl_context, image,
                             src_x, src_y, src_w, src_h,
                             dst_x, dst_y, dst_w, dst_h,
                             smooth);
   return EINA_FALSE;
}

static void
eng_image_scale_hint_set(void *data EINA_UNUSED, void *image, int hint)
{
   if (image) evas_gl_common_image_scale_hint_set(image, hint);
}

static Eina_Bool
eng_image_map_draw(void *data EINA_UNUSED, void *context, void *surface, void *image,  RGBA_Map *m, int smooth, int level, Eina_Bool do_async)
{
   Evas_GL_Image *gim = image;
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_context_target_surface_set(re->gl_context, surface);
   re->gl_context->dc = context;
   if (m->count != 4)
     {
        // FIXME: nash - you didn't fix this
        abort();
     }
   if ((m->pts[0].x == m->pts[3].x) &&
       (m->pts[1].x == m->pts[2].x) &&
       (m->pts[0].y == m->pts[1].y) &&
       (m->pts[3].y == m->pts[2].y) &&
       (m->pts[0].x <= m->pts[1].x) &&
       (m->pts[0].y <= m->pts[2].y) &&
       (m->pts[0].u == 0) &&
       (m->pts[0].v == 0) &&
       (m->pts[1].u == (gim->w << FP)) &&
       (m->pts[1].v == 0) &&
       (m->pts[2].u == (gim->w << FP)) &&
       (m->pts[2].v == (gim->h << FP)) &&
       (m->pts[3].u == 0) &&
       (m->pts[3].v == (gim->h << FP)) &&
       (m->pts[0].col == 0xffffffff) &&
       (m->pts[1].col == 0xffffffff) &&
       (m->pts[2].col == 0xffffffff) &&
       (m->pts[3].col == 0xffffffff))
     {
        int dx, dy, dw, dh;

        dx = m->pts[0].x >> FP;
        dy = m->pts[0].y >> FP;
        dw = (m->pts[2].x >> FP) - dx;
        dh = (m->pts[2].y >> FP) - dy;
        eng_image_draw(data, context, surface, image,
                       0, 0, gim->w, gim->h, dx, dy, dw, dh, smooth, do_async);
     }
   else
     {
        evas_gl_common_image_map_draw(re->gl_context, image, m->count, &m->pts[0],
                                      smooth, level);
     }

   return EINA_FALSE;
}

static void *
eng_image_map_surface_new(void *data EINA_UNUSED, int w, int h, int alpha)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return evas_gl_common_image_surface_new(re->gl_context, w, h, alpha);
}

static void
eng_image_map_surface_free(void *data EINA_UNUSED, void *surface)
{
   evas_gl_common_image_free(surface);
}

static int
eng_image_scale_hint_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   if (!gim) return EVAS_IMAGE_SCALE_HINT_NONE;
   return gim->scale_hint;
}

static Eina_Bool
eng_font_draw(void *data, void *context, void *surface, Evas_Font_Set *font EINA_UNUSED, int x, int y, int w EINA_UNUSED, int h EINA_UNUSED, int ow EINA_UNUSED, int oh EINA_UNUSED, Evas_Text_Props *intl_props, Eina_Bool do_async EINA_UNUSED)
{
   Render_Engine *re = data;

   evas_gl_common_context_target_surface_set(re->gl_context, surface);
   re->gl_context->dc = context;
     {
        // FIXME: put im into context so we can free it
	static RGBA_Image *im = NULL;
        
        if (!im)
          im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        im->cache_entry.w = re->w;
        im->cache_entry.h = re->h;
        evas_common_draw_context_font_ext_set(context,
   					      re->gl_context,
   					      evas_gl_font_texture_new,
   					      evas_gl_font_texture_free,
   					      evas_gl_font_texture_draw);
	evas_common_font_draw_prepare(intl_props);
	evas_common_font_draw(im, context, x, y, intl_props->glyphs);
	evas_common_draw_context_font_ext_set(context,
					      NULL,
					      NULL,
					      NULL,
					      NULL);
     }

   return EINA_FALSE;
}

static Eina_Bool
eng_canvas_alpha_get(void *data EINA_UNUSED, void *info EINA_UNUSED)
{
   // FIXME: support ARGB gl targets!!!
   return EINA_FALSE;
}

static int
eng_image_load_error_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im;
   
   if (!image) return EVAS_LOAD_ERROR_NONE;
   im = image;
   return im->im->cache_entry.load_error;
}


static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   if (!evas_gl_common_module_open()) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   if (_evas_engine_GL_SDL_log_dom < 0)
     _evas_engine_GL_SDL_log_dom = eina_log_domain_register
       ("evas-gl_sdl", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_GL_SDL_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
   #define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(canvas_alpha_get);
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
   ORD(output_dump);
   ORD(rectangle_draw);
   ORD(line_draw);
   ORD(polygon_point_add);
   ORD(polygon_points_clear);
   ORD(polygon_draw);

   ORD(image_load);
   ORD(image_mmap);
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
   
   ORD(image_scale_hint_set);
   ORD(image_scale_hint_get);
   
   ORD(image_map_draw);
   ORD(image_map_surface_new);
   ORD(image_map_surface_free);

//   ORD(image_content_hint_set);
//   ORD(image_content_hint_get);
   
//   ORD(image_cache_flush);
//   ORD(image_cache_set);
//   ORD(image_cache_get);
   
//   ORD(gl_surface_create);
//   ORD(gl_surface_destroy);
//   ORD(gl_context_create);
//   ORD(gl_context_destroy);
//   ORD(gl_make_current);
//   ORD(gl_proc_address_get);
//   ORD(gl_native_surface_get);
   
//   ORD(gl_api_get);
   
   ORD(image_load_error_get);
   
   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
    eina_log_domain_unregister(_evas_engine_GL_SDL_log_dom);
    evas_gl_common_module_close();
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "gl_sdl",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, gl_sdl);

#ifndef EVAS_STATIC_BUILD_GL_SDL
EVAS_EINA_MODULE_DEFINE(engine, gl_sdl);
#endif

static void*
_sdl_output_setup		(int w, int h, int fullscreen, int noframe)
{
   Render_Engine		*re = calloc(1, sizeof(Render_Engine));
   SDL_Surface                  *surface;

   /* if we haven't initialized - init (automatic abort if already done) */
   evas_common_cpu_init();
   evas_common_blend_init();
   evas_common_image_init();
   evas_common_convert_init();
   evas_common_scale_init();
   evas_common_rectangle_init();
   evas_common_polygon_init();
   evas_common_line_init();
   evas_common_font_init();
   evas_common_draw_init();
   evas_common_tilebuf_init();

   if (w <= 0) w = 640;
   if (h <= 0) h = 480;
   
   /* GL Initialization */
#ifdef HAVE_SDL_GL_CONTEXT_VERSION
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
   SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
   SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
   SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
   SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
   SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);

   surface = SDL_SetVideoMode(w, h, 32, EVAS_SDL_GL_FLAG
			   | (fullscreen ? SDL_FULLSCREEN : 0)
			   | (noframe ? SDL_NOFRAME : 0));

   if (!surface)
     {
        CRI("SDL_SetVideoMode [ %i x %i x 32 ] failed. %s", w, h, SDL_GetError());
	SDL_Quit();
        exit(-1);
     }

   INF("Screen Depth: %d, Vendor: '%s', Renderer: '%s', Version: '%s'", SDL_GetVideoSurface()->format->BitsPerPixel, glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));

   re->gl_context = evas_gl_common_context_new();
   if (!re->gl_context)
     {
	free(re);
	return NULL;
     }
   evas_gl_common_context_use(re->gl_context);
   evas_gl_common_context_resize(re->gl_context, w, h, re->gl_context->rot);

   /* End GL Initialization */
   re->w = w;
   re->h = h;
   return re;
}

