#include <assert.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <SDL/SDL.h>

#include "evas_engine.h"

/* #define DEBUG_SDL */

extern Evas_List*	evas_modules;

static Evas_Func	func = {};
static Evas_Func	pfunc = {};

static void*		_sdl_output_setup		(int w, int h, int fullscreen, int noframe, int alpha, int hwsurface);
static void		_sdl_stretch_blit		(const RGBA_Engine_Image* from, RGBA_Engine_Image* to, int w, int h);

static int              _sdl_image_constructor          (RGBA_Engine_Image*, void* data);
static void             _sdl_image_destructor           (RGBA_Engine_Image *eim);

static void             _sdl_image_dirty_region         (RGBA_Engine_Image *eim, int x, int y, int w, int h);

static int              _sdl_image_dirty                (RGBA_Engine_Image *dst, const RGBA_Engine_Image *src);

static int              _sdl_image_size_set             (RGBA_Engine_Image *dst, const RGBA_Engine_Image *src);

static int              _sdl_image_update_data          (RGBA_Engine_Image* dst, void* engine_data);

static void             _sdl_image_load                 (RGBA_Engine_Image *eim, const RGBA_Image* im);
static int              _sdl_image_mem_size_get         (RGBA_Engine_Image *eim);

#ifdef DEBUG_SDL
static void             _sdl_image_debug(const char* context, RGBA_Engine_Image* im);
#endif

static const Evas_Cache_Engine_Image_Func       _sdl_cache_engine_image_cb = {
   .key = NULL,
   .constructor = _sdl_image_constructor,
   .destructor = _sdl_image_destructor,
   .dirty_region = _sdl_image_dirty_region,
   .dirty = _sdl_image_dirty,
   .size_set = _sdl_image_size_set,
   .update_data = _sdl_image_update_data,
   .load = _sdl_image_load,
   .mem_size_get = _sdl_image_mem_size_get,
#ifdef DEBUG_SDL
   .debug = _sdl_image_debug
#else
   .debug = NULL
#endif
};

#define _SDL_UPDATE_PIXELS(EIM) ;
/*         EIM->src->image->data = ((SDL_Surface*) EIM->engine_data)->pixels; */

#define RMASK 0x00ff0000
#define GMASK 0x0000ff00
#define BMASK 0x000000ff
#define AMASK 0xff000000

/* SDL engine info function */
static void*
evas_engine_sdl_info		(Evas* e)
{
   Evas_Engine_Info_SDL*	info = calloc(1, sizeof (Evas_Engine_Info_SDL));

   if (!info)
      return NULL;

   info->magic.magic = rand();

   return info;
}

static void
evas_engine_sdl_info_free	(Evas* e, void* info)
{
   Evas_Engine_Info_SDL*	in = (Evas_Engine_Info_SDL*) info;

   free(in);
   in = NULL;
}

/* SDL engine output manipulation function */
static void
evas_engine_sdl_setup		(Evas* e, void* in)
{
   Evas_Engine_Info_SDL*	info = (Evas_Engine_Info_SDL*) in;

   /* if we arent set to sdl, why the hell do we get called?! */
   if (evas_output_method_get(e) != evas_render_method_lookup("software_sdl"))
      return ;

   SDL_Init(SDL_INIT_NOPARACHUTE);

   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
     {
	fprintf(stderr, "SDL_Init failed with %s\n", SDL_GetError());
        exit(-1);
     }

   /* lets just set up */
   e->engine.data.output = _sdl_output_setup(e->output.w, e->output.h,
					     info->info.fullscreen,
					     info->info.noframe,
                                             info->info.alpha,
                                             info->info.hwsurface);

   if (!e->engine.data.output)
      return;

   e->engine.func = &func;
   e->engine.data.context = e->engine.func->context_new(e->engine.data.output);
   info->info.surface = ((Render_Engine*) e->engine.data.output)->surface;
}

static void
evas_engine_sdl_output_free	(void *data)
{
   Render_Engine*		re = (Render_Engine*) data;

   if (re->cache)
     evas_cache_engine_image_shutdown(re->cache);

   evas_common_tilebuf_free(re->tb);
   if (re->rects)
      evas_common_tilebuf_free_render_rects(re->rects);

   if (re->update_rects)
     free(re->update_rects);
   memset(re, sizeof (Render_Engine), 0);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();

   SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

static void
evas_engine_sdl_output_resize	(void *data, int w, int h)
{
   /* FIXME */
   Render_Engine*		re = (Render_Engine*) data;
   RGBA_Engine_Image*		eim = NULL;

   if (w == re->tb->outbuf_w && h == re->tb->outbuf_h)
     return;

   eim = re->rgba_engine_image;

   /* Rebuil tilebuf */
   evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb)
      evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   /* Build the new screen */
   re->surface = SDL_SetVideoMode(w, h, 32,
				  (re->hwsurface ? SDL_HWSURFACE : SDL_SWSURFACE)
				  | (re->fullscreen ? SDL_FULLSCREEN : 0)
				  | (re->noframe ? SDL_NOFRAME : 0)
                                  | (re->alpha ? SDL_SRCALPHA : 0));

   if (!re->surface)
     {
	fprintf(stderr, "Unable to change the resolution to : %ix%i\n", w, h);
	exit(-1);
     }
   re->rgba_engine_image = evas_cache_engine_image_engine(re->cache, re->surface);
   if (!re->rgba_engine_image)
     {
	fprintf(stderr, "RGBA_Image allocation from SDL failed\n");
	exit(-1);
     }

   /* Destroy the copy */
   evas_cache_engine_image_drop(eim);
}

static void
evas_engine_sdl_output_tile_size_set	(void *data, int w, int h)
{
   Render_Engine*			re = (Render_Engine*) data;

   evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void
evas_engine_sdl_output_redraws_rect_add	(void *data, int x, int y, int w, int h)
{
   Render_Engine*			re = (Render_Engine*) data;

   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void
evas_engine_sdl_output_redraws_rect_del	(void *data, int x, int y, int w, int h)
{
   Render_Engine*			re = (Render_Engine*) data;

   evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void
evas_engine_sdl_output_redraws_clear	(void *data)
{
   Render_Engine*			re = (Render_Engine*) data;

   evas_common_tilebuf_clear(re->tb);
}

static void*
evas_engine_sdl_output_redraws_next_update_get	(void *data,
						 int *x, int *y, int *w, int *h,
						 int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine*				re = (Render_Engine*) data;
   Tilebuf_Rect*				tb_rect = NULL;

   if (re->end)
     {
	re->end = 0;
	return NULL;
     }
   if (!re->rects)
     {
	re->rects = evas_common_tilebuf_get_render_rects(re->tb);
	re->cur_rect = (Evas_Object_List *) re->rects;
     }
   if (!re->cur_rect)
      return NULL;

   tb_rect = (Tilebuf_Rect*) re->cur_rect;
   *cx = *x = tb_rect->x;
   *cy = *y = tb_rect->y;
   *cw = *w = tb_rect->w;
   *ch = *h = tb_rect->h;
   re->cur_rect = re->cur_rect->next;
   if (!re->cur_rect)
     {
	evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	re->end = 1;
     }

   /* Return the "fake" surface so it is passed to the drawing routines. */
   return re->rgba_engine_image;
}

static void
evas_engine_sdl_output_redraws_next_update_push	(void *data, void *surface,
						 int x, int y, int w, int h)
{
   Render_Engine				*re = (Render_Engine *) data;

   if (re->update_rects_count + 1 > re->update_rects_limit)
     {
	re->update_rects_limit += 8;
	re->update_rects = realloc(re->update_rects, sizeof (SDL_Rect) * re->update_rects_limit);
     }

   re->update_rects[re->update_rects_count].x = x;
   re->update_rects[re->update_rects_count].y = y;
   re->update_rects[re->update_rects_count].w = w;
   re->update_rects[re->update_rects_count].h = h;

   ++re->update_rects_count;

   evas_common_cpu_end_opt();
}

static void
_sdl_image_dirty_region(RGBA_Engine_Image *eim, int x, int y, int w, int h)
{
   SDL_UpdateRect((SDL_Surface*) eim->engine_data, x, y, w, h);
}

static void
evas_engine_sdl_output_flush			(void *data)
{
   Render_Engine				*re = (Render_Engine *) data;

   if (re->update_rects_count > 0)
     SDL_UpdateRects(re->surface, re->update_rects_count, re->update_rects);

   re->update_rects_count = 0;
}


static void
evas_engine_sdl_output_idle_flush               (void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

/*
 * Image objects
 */

static void*
evas_engine_sdl_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine*	re = (Render_Engine*) data;;

   *error = 0;
   return evas_cache_engine_image_request(re->cache, file, key, lo, NULL, error);
}

static int
evas_engine_sdl_image_alpha_get(void *data, void *image)
{
   RGBA_Engine_Image *eim = image;

   if (!eim) return 1;
   switch (eim->src->cs.space)
     {
     case EVAS_COLORSPACE_ARGB8888:
        if (eim->src->flags & RGBA_IMAGE_HAS_ALPHA) return 1;
     default:
        break;
     }
   return 0;
}

static void
evas_engine_sdl_image_size_get(void *data, void *image, int *w, int *h)
{
   RGBA_Engine_Image*   eim;

   eim = image;
   if (w) *w = eim->src->image->w;
   if (h) *h = eim->src->image->h;
}

static int
evas_engine_sdl_image_colorspace_get(void *data, void *image)
{
   RGBA_Engine_Image *eim = image;

   if (!eim) return EVAS_COLORSPACE_ARGB8888;
   return eim->src->cs.space;
}

static void
evas_engine_sdl_image_colorspace_set(void *data, void *image, int cspace)
{
   RGBA_Engine_Image *eim = (RGBA_Engine_Image*) image;

   if (!eim) return;
   if (eim->src->cs.space == cspace) return;

   evas_cache_engine_colorspace(eim, cspace, NULL);
}

static void*
evas_engine_sdl_image_new_from_copied_data(void *data,
					   int w, int h,
					   DATA32* image_data,
                                           int alpha, int cspace)
{
   Render_Engine	*re = (Render_Engine*) data;

   return evas_cache_engine_image_copied_data(re->cache, w, h, image_data, alpha, cspace, NULL);
}

static void*
evas_engine_sdl_image_new_from_data(void *data, int w, int h, DATA32* image_data, int alpha, int cspace)
{
   Render_Engine	*re = (Render_Engine*) data;

   return evas_cache_engine_image_data(re->cache, w, h, image_data, alpha, cspace, NULL);
}

static void
evas_engine_sdl_image_free(void *data, void *image)
{
   RGBA_Engine_Image*		eim = (RGBA_Engine_Image*) image;

   (void) data;

   evas_cache_engine_image_drop(eim);
}

static void*
evas_engine_sdl_image_size_set(void *data, void *image, int w, int h)
{
   RGBA_Engine_Image*		eim = (RGBA_Engine_Image*) image;

   (void) data;

   return evas_cache_engine_image_size_set(eim, w, h);
}

void*
evas_engine_sdl_image_dirty_region(void *data,
				   void *image,
				   int x, int y, int w, int h)
{
   RGBA_Engine_Image*		eim = (RGBA_Engine_Image*) image;

   (void) data;

   return evas_cache_engine_image_dirty(eim, x, y, w, h);
}

void*
evas_engine_sdl_image_data_get(void *data, void *image,
			       int to_write, DATA32** image_data)
{
   RGBA_Engine_Image*		eim = (RGBA_Engine_Image*) image;

   (void) data;

   if (!eim)
     {
        *image_data = NULL;
        return NULL;
     }

   switch (eim->src->cs.space)
     {
     case EVAS_COLORSPACE_ARGB8888:
        if (to_write)
          eim = evas_cache_engine_image_dirty(eim, 0, 0, eim->src->image->w, eim->src->image->h);

        evas_cache_engine_image_load_data(eim);
        *image_data = eim->src->image->data;
        break;
     case EVAS_COLORSPACE_YCBCR422P709_PL:
     case EVAS_COLORSPACE_YCBCR422P601_PL:
        *image_data = eim->src->cs.data;
        break;
     default:
        abort();
        break;
     }
   return eim;
}

void*
evas_engine_sdl_image_data_put(void *data, void *image, DATA32* image_data)
{
   Render_Engine*	re = (Render_Engine*) data;
   RGBA_Engine_Image*	eim = (RGBA_Engine_Image*) image;

   if (!eim) return NULL;

   switch (eim->src->cs.space)
     {
     case EVAS_COLORSPACE_ARGB8888:
        if (image_data != eim->src->image->data)
          {
             evas_cache_engine_image_drop(eim);
             eim = evas_cache_engine_image_data(re->cache,
                                                eim->src->image->w, eim->src->image->h,
                                                image_data,
                                                func.image_alpha_get(data, eim),
                                                func.image_colorspace_get(data, eim),
                                                NULL);
          }
        break;
     case EVAS_COLORSPACE_YCBCR422P601_PL:
     case EVAS_COLORSPACE_YCBCR422P709_PL:
        if (image_data != eim->src->cs.data)
          {
             if (eim->src->cs.data)
               if (!eim->src->cs.no_free)
                 free(eim->src->cs.data);
             eim->src->cs.data = image_data;
             evas_common_image_colorspace_dirty(eim->src);
          }
        break;
     default:
        abort();
        break;
     }
   return eim;
}

void*
evas_engine_sdl_image_alpha_set(void *data, void *image, int has_alpha)
{
   RGBA_Engine_Image*		eim = (RGBA_Engine_Image*) image;

   (void) data;

   if (!eim) return NULL;
   if (eim->src->cs.space != EVAS_COLORSPACE_ARGB8888)
     {
        eim->src->flags &= ~RGBA_IMAGE_HAS_ALPHA;
        return eim;
     }

   eim = evas_cache_engine_image_dirty(eim, 0, 0, eim->src->image->w, eim->src->image->h);

   /* FIXME: update SDL_Surface flags */
   if (has_alpha)
     eim->src->flags |= RGBA_IMAGE_HAS_ALPHA;
   else
     eim->src->flags &= ~RGBA_IMAGE_HAS_ALPHA;
   return eim;
}

void*
evas_engine_sdl_image_border_set(void *data, void *image, int l, int r, int t, int b)
{
   return image;
}

void
evas_engine_sdl_image_border_get(void *data, void *image, int *l, int *r, int *t, int *b)
{
   /* FIXME: need to know what evas expect from this call */
}

void
evas_engine_sdl_image_draw(void *data, void *context, void *surface, void *image,
			   int src_region_x, int src_region_y, int src_region_w, int src_region_h,
			   int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h,
			   int smooth)
{
   Render_Engine*       re = data;
   RGBA_Engine_Image*	eim = (RGBA_Engine_Image*) image;
   RGBA_Engine_Image*   dst = (RGBA_Engine_Image*) surface;
   RGBA_Draw_Context*	dc = (RGBA_Draw_Context*) context;
   int                  mustlock_im = 0;
   int                  mustlock_dst = 0;

   (void) data;

   if (eim->src->cs.space == EVAS_COLORSPACE_ARGB8888)
     evas_cache_engine_image_load_data(eim);

   /* Fallback to software method */
   if (SDL_MUSTLOCK(((SDL_Surface*) dst->engine_data)))
     {
        mustlock_dst = 1;
	SDL_LockSurface(dst->engine_data);
	_SDL_UPDATE_PIXELS(dst);
     }

   if (eim->engine_data && SDL_MUSTLOCK(((SDL_Surface*) eim->engine_data)))
     {
        mustlock_im = 1;
	SDL_LockSurface(eim->engine_data);
	_SDL_UPDATE_PIXELS(eim);
     }

   evas_common_image_colorspace_normalize(eim->src);

   if (smooth)
     evas_common_scale_rgba_in_to_out_clip_smooth(eim->src, dst->src, dc,
                                                  src_region_x, src_region_y, src_region_w, src_region_h,
                                                  dst_region_x, dst_region_y, dst_region_w, dst_region_h);
   else
     evas_common_scale_rgba_in_to_out_clip_sample(eim->src, dst->src, dc,
                                                  src_region_x, src_region_y, src_region_w, src_region_h,
                                                  dst_region_x, dst_region_y, dst_region_w, dst_region_h);
   evas_common_cpu_end_opt ();

   if (mustlock_im)
     SDL_UnlockSurface(eim->engine_data);

   if (mustlock_dst)
     SDL_UnlockSurface(dst->engine_data);
}

void
evas_engine_sdl_image_cache_flush(void *data)
{
   Render_Engine*       re = (Render_Engine*) data;
   int                  size;

   size = evas_cache_engine_image_get(re->cache);
   evas_cache_engine_image_set(re->cache, 0);
   evas_cache_engine_image_set(re->cache, size);
}

void
evas_engine_sdl_image_cache_set(void *data, int bytes)
{
   Render_Engine*       re = (Render_Engine*) data;

   evas_cache_engine_image_set(re->cache, bytes);
}

int
evas_engine_sdl_image_cache_get(void *data)
{
   Render_Engine*       re = (Render_Engine*) data;

   return evas_cache_engine_image_get(re->cache);
}

char*
evas_engine_sdl_image_comment_get(void *data, void *image, char *key)
{
   RGBA_Engine_Image         *eim = (RGBA_Engine_Image*) image;

   return eim->src->info.comment;
}

char*
evas_engine_sdl_image_format_get(void *data, void *image)
{
   /* FIXME: need to know what evas expect from this call */
   return NULL;
}

static void
evas_engine_sdl_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, const char *text)
{
   RGBA_Engine_Image*	eim = (RGBA_Engine_Image*) surface;
   int                  mustlock_im = 0;

   if (eim->engine_data && SDL_MUSTLOCK(((SDL_Surface*) eim->engine_data)))
     {
        mustlock_im = 1;
	SDL_LockSurface(eim->engine_data);
	_SDL_UPDATE_PIXELS(eim);
     }

   evas_common_font_draw(eim->src, context, font, x, y, text);
   evas_common_cpu_end_opt();

   if (mustlock_im)
     SDL_UnlockSurface(eim->engine_data);
}

static void
evas_engine_sdl_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   RGBA_Engine_Image*	eim = (RGBA_Engine_Image*) surface;
   int                  mustlock_im = 0;

   if (eim->engine_data && SDL_MUSTLOCK(((SDL_Surface*) eim->engine_data)))
     {
        mustlock_im = 1;
	SDL_LockSurface(eim->engine_data);
	_SDL_UPDATE_PIXELS(eim);
     }

   evas_common_line_draw(eim->src, context, x1, y1, x2, y2);
   evas_common_cpu_end_opt();

   if (mustlock_im)
     SDL_UnlockSurface(eim->engine_data);
}

static void
evas_engine_sdl_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   RGBA_Engine_Image*	eim = (RGBA_Engine_Image*) surface;
   int                  mustlock_im = 0;

   if (eim->engine_data && SDL_MUSTLOCK(((SDL_Surface*) eim->engine_data)))
     {
        mustlock_im = 1;
	SDL_LockSurface(eim->engine_data);
	_SDL_UPDATE_PIXELS(eim);
     }

   evas_common_rectangle_draw(eim->src, context, x, y, w, h);
   evas_common_cpu_end_opt();

   if (mustlock_im)
     SDL_UnlockSurface(eim->engine_data);
}

static void
evas_engine_sdl_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   RGBA_Engine_Image*	eim = (RGBA_Engine_Image*) surface;
   int                  mustlock_im = 0;

   if (eim->engine_data && SDL_MUSTLOCK(((SDL_Surface*) eim->engine_data)))
     {
        mustlock_im = 1;
	SDL_LockSurface(eim->engine_data);
	_SDL_UPDATE_PIXELS(eim);
     }

   evas_common_polygon_draw(eim->src, context, polygon);
   evas_common_cpu_end_opt();

   if (mustlock_im)
     SDL_UnlockSurface(eim->engine_data);
}

static void
evas_engine_sdl_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h)
{
   RGBA_Engine_Image*	eim = (RGBA_Engine_Image*) surface;
   int                  mustlock_im = 0;

   if (eim->engine_data && SDL_MUSTLOCK(((SDL_Surface*) eim->engine_data)))
     {
        mustlock_im = 1;
	SDL_LockSurface(eim->engine_data);
	_SDL_UPDATE_PIXELS(eim);
     }

   evas_common_gradient_draw(eim->src, context, x, y, w, h, gradient);
   evas_common_cpu_end_opt();

   if (mustlock_im)
     SDL_UnlockSurface(eim->engine_data);
}

EAPI int module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, evas_engine_sdl_)
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
   ORD(image_load);
   ORD(image_new_from_data);
   ORD(image_new_from_copied_data);
   ORD(image_colorspace_set);
   ORD(image_colorspace_get);
   ORD(image_free);
   ORD(image_size_set);
   ORD(image_size_get);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_border_set);
   ORD(image_border_get);
   ORD(image_draw);
   ORD(image_comment_get);
   ORD(image_format_get);
   ORD(image_cache_flush);
   ORD(image_cache_set);
   ORD(image_cache_get);
   ORD(font_draw);
   ORD(line_draw);
   ORD(rectangle_draw);
   ORD(polygon_draw);
   ORD(gradient_draw);
   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

EAPI void module_close(void)
{

}

EAPI Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   EVAS_MODULE_TYPE_ENGINE,
   "software_sdl",
   "none"
};

/* Private routines. */

static void*
_sdl_output_setup		(int w, int h, int fullscreen, int noframe, int alpha, int hwsurface)
{
   Render_Engine		*re = calloc(1, sizeof(Render_Engine));

   /* if we haven't initialized - init (automatic abort if already done) */
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

   re->cache = evas_cache_engine_image_init(&_sdl_cache_engine_image_cb, evas_common_image_cache_get());
   if (!re->cache)
     {
        fprintf(stderr, "Evas_Cache_Engine_Image allocation failed!\n");
        exit(-1);
     }

   re->tb = evas_common_tilebuf_new(w, h);
   /* in preliminary tests 16x16 gave highest framerates */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   re->surface = SDL_SetVideoMode(w, h, 32,
				  (hwsurface ? SDL_HWSURFACE : SDL_SWSURFACE)
				  | (fullscreen ? SDL_FULLSCREEN : 0)
				  | (noframe ? SDL_NOFRAME : 0)
                                  | (alpha ? SDL_SRCALPHA : 0));

   if (!re->surface)
     return NULL;

   SDL_SetAlpha(re->surface, SDL_SRCALPHA | SDL_RLEACCEL, 0);

   /* We create a "fake" RGBA_Image which points to the SDL surface. Each access
    * to that surface is wrapped in Lock / Unlock calls whenever the data is
    * manipulated directly. */
   re->rgba_engine_image = evas_cache_engine_image_engine(re->cache, re->surface);
   if (!re->rgba_engine_image)
     {
	fprintf(stderr, "RGBA_Image allocation from SDL failed\n");
        exit(-1);
     }

   re->alpha = alpha;
   re->hwsurface = hwsurface;
   re->fullscreen = fullscreen;
   re->noframe = noframe;
   return re;
}

static void
_sdl_stretch_blit		(const RGBA_Engine_Image* from,
				 RGBA_Engine_Image* to,
				 int w, int h)
{
   int                          mustlock_from = 0;
   int                          mustlock_to = 0;

   if (from->engine_data)
     if (SDL_MUSTLOCK(((SDL_Surface*) from->engine_data)))
       mustlock_from = 1;

   if (to->engine_data)
     if (SDL_MUSTLOCK(((SDL_Surface*) to->engine_data)))
       mustlock_to = 1;

   if (mustlock_from)
     {
        SDL_LockSurface(from->engine_data);
        _SDL_UPDATE_PIXELS(from);
     }

   if (mustlock_to)
     {
        SDL_LockSurface(to->engine_data);
        _SDL_UPDATE_PIXELS(to);
     }

   evas_common_blit_rectangle(from->src, to->src, 0, 0, w, h, 0, 0);

   if (mustlock_to)
     SDL_UnlockSurface(to->engine_data);

   if (mustlock_from)
     SDL_UnlockSurface(from->engine_data);

   evas_common_cpu_end_opt();
}

static int
_sdl_image_constructor(RGBA_Engine_Image* eim, void* data)
{
   SDL_Surface     *sdl = NULL;

   if (eim->src->image->data)
     {
        /* FIXME: Take care of CSPACE */
        sdl = SDL_CreateRGBSurfaceFrom(eim->src->image->data,
                                       eim->src->image->w, eim->src->image->h,
                                       32, eim->src->image->w * 4,
                                       RMASK, GMASK, BMASK, AMASK);
        eim->engine_data = sdl;
     }

   return 0;
}

static void
_sdl_image_destructor(RGBA_Engine_Image *eim)
{
   if (eim->engine_data)
     SDL_FreeSurface(eim->engine_data);
   eim->engine_data = NULL;
}

static int
_sdl_image_dirty(RGBA_Engine_Image *dst, const RGBA_Engine_Image *src)
{
   SDL_Surface     *sdl = NULL;

   /* FIXME: Take care of CSPACE */
   sdl = SDL_CreateRGBSurfaceFrom(dst->src->image->data,
                                  dst->src->image->w, dst->src->image->h,
                                  32, dst->src->image->w * 4,
                                  0xff0000, 0xff00, 0xff, 0xff000000);
   dst->engine_data = sdl;

   return 0;
}

static int
_sdl_image_update_data(RGBA_Engine_Image *dst, void* engine_data)
{
   SDL_Surface     *sdl = NULL;

   if (engine_data)
     {
        sdl = engine_data;

        dst->src->image->data = sdl->pixels;
        dst->src->image->no_free = 1;
        dst->src->image->w = sdl->w;
        dst->src->image->h = sdl->h;
        dst->src->flags |= RGBA_IMAGE_HAS_ALPHA;
     }
   else
     /* FIXME: Take care of CSPACE */
     sdl = SDL_CreateRGBSurfaceFrom(dst->src->image->data,
                                    dst->src->image->w, dst->src->image->h,
                                    32, dst->src->image->w * 4,
                                    RMASK, GMASK, BMASK, AMASK);

   dst->engine_data = sdl;

   return 0;
}

static int
_sdl_image_size_set(RGBA_Engine_Image *dst, const RGBA_Engine_Image *src)
{
   SDL_Surface*    sdl;

   sdl = SDL_CreateRGBSurfaceFrom(dst->src->image->data,
                                  dst->src->image->w, dst->src->image->h,
                                  32, dst->src->image->w * 4,
                                  RMASK, GMASK, BMASK, AMASK);

   dst->engine_data = sdl;

/*    _sdl_stretch_blit(src, dst, dst->src->image->w, dst->src->image->h); */

   return 0;
}

static void
_sdl_image_load(RGBA_Engine_Image *eim, const RGBA_Image *im)
{
   SDL_Surface*    sdl;

   if (!eim->engine_data)
     {
        sdl = SDL_CreateRGBSurfaceFrom(eim->src->image->data,
                                       eim->src->image->w, eim->src->image->h,
                                       32, eim->src->image->w * 4,
                                       RMASK, GMASK, BMASK, AMASK);
        eim->engine_data = sdl;
     }
}

static int
_sdl_image_mem_size_get(RGBA_Engine_Image *eim)
{
   int  size = 0;

   if (eim->engine_data)
     size = sizeof (SDL_Surface) + sizeof (SDL_PixelFormat) + (eim->src ? evas_common_image_ram_usage(eim->src) : 0);

   return size;
}

#ifdef DEBUG_SDL
static void
_sdl_image_debug(const char* context, RGBA_Engine_Image* eim)
{
   printf ("*** %s image (%p) ***\n", context, eim);
   if (eim)
     {
	printf ("* W: %i\n* H: %i\n", eim->src->image->w, eim->src->image->h);
	printf ("* Pixels: %p\n* SDL Surface: %p\n", eim->src->image->data, eim->engine_data);
        printf ("* Surface->pixels: %p\n", ((SDL_Surface*) eim->engine_data)->pixels);
	printf ("* Flags: %i\n", eim->src->flags);
	printf ("* Filename: %s\n* Key: %s\n", eim->src->info.file, eim->src->info.key);
        printf ("* Reference: %i\n", eim->references);
     }
   printf ("*** ***\n");
}
#endif
