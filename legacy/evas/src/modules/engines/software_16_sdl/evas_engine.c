#include <assert.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <SDL/SDL.h>

#include "evas_engine.h"

/* function tables - filled in later (func and parent func) */
static Evas_Func        func = {};
static Evas_Func        pfunc = {};

static Engine_Image_Entry       *_sdl16_image_alloc       (void);
static void                      _sdl16_image_delete      (Engine_Image_Entry *eim);

static int                       _sdl16_image_constructor (Engine_Image_Entry *ie, void* data);
static void                      _sdl16_image_destructor  (Engine_Image_Entry *eim);

static void                      _sdl16_image_dirty_region(Engine_Image_Entry *eim, int x, int y, int w, int h);

static int                       _sdl16_image_dirty       (Engine_Image_Entry *dst, const Engine_Image_Entry *src);

static int                       _sdl16_image_size_set    (Engine_Image_Entry *dst, const Engine_Image_Entry *src);

static int                       _sdl16_image_update_data (Engine_Image_Entry* dst, void* engine_data);

static void                      _sdl16_image_load        (Engine_Image_Entry *eim, const Image_Entry* im);
static int                       _sdl16_image_mem_size_get(Engine_Image_Entry *eim);

#ifdef DEBUG_SDL
static void                      _sdl16_image_debug       (const char* context, Engine_Image_Entry* im);
#endif

static const Evas_Cache_Engine_Image_Func       _sdl16_cache_engine_image_cb = {
  NULL /* key */,
  _sdl16_image_alloc /* alloc */,
  _sdl16_image_delete /* dealloc */,
  _sdl16_image_constructor /* constructor */,
  _sdl16_image_destructor /* destructor */,
  _sdl16_image_dirty_region /* dirty_region */,
  _sdl16_image_dirty /* dirty */,
  _sdl16_image_size_set /* size_set */,
  _sdl16_image_update_data /* update_data */,
  _sdl16_image_load /* load */,
  _sdl16_image_mem_size_get /* mem_size_get */,
#ifdef DEBUG_SDL  /* debug */
  _sdl16_image_debug
#else
  NULL
#endif
};

#define _SDL_UPDATE_PIXELS(EIM)                                 \
  ((Soft16_Image *) EIM->cache_entry.src)->pixels = EIM->surface->pixels;

#define RMASK565 0xf800
#define GMASK565 0x07e0
#define BMASK565 0x001f
#define AMASK565 0x0000

/* engine api this module provides */
static void *
evas_engine_sdl16_info(Evas *e __UNUSED__)
{
   Evas_Engine_Info_SDL_16      *info;

   info = calloc(1, sizeof(Evas_Engine_Info_SDL_16));
   if (!info) return NULL;
   info->magic.magic = rand();
   return info;
}

static void
evas_engine_sdl16_info_free(Evas *e __UNUSED__, void *info)
{
   Evas_Engine_Info_SDL_16 *in;

   in = (Evas_Engine_Info_SDL_16 *)info;
   free(in);
}

static void
_tmp_out_alloc(Render_Engine *re)
{
   Tilebuf_Rect *r;
   int w = 0, h = 0;

   EINA_INLIST_FOREACH(re->rects, r)
     {
	if (r->w > w) w = r->w;
	if (r->h > h) h = r->h;
     }

   if (re->tmp_out)
     {
	if ((re->tmp_out->cache_entry.w < w) || (re->tmp_out->cache_entry.h < h))
	  {
             evas_cache_image_drop(&re->tmp_out->cache_entry);
	     re->tmp_out = NULL;
	  }
     }

   if (!re->tmp_out)
     {
	Soft16_Image *im;

        im = (Soft16_Image *) evas_cache_image_empty(evas_common_soft16_image_cache_get());
        im->cache_entry.flags.alpha = 0;
        evas_cache_image_surface_alloc(&im->cache_entry, w, h);

	re->tmp_out = im;
     }
}

static void*
_sdl16_output_setup(int w, int h, int rotation, int fullscreen, int noframe, int hwsurface)
{
   Render_Engine	*re = calloc(1, sizeof(Render_Engine));
   SDL_Surface          *surface;

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
   evas_common_soft16_image_init();

   if (w <= 0) w = 640;
   if (h <= 0) h = 480;

   re->cache = evas_cache_engine_image_init(&_sdl16_cache_engine_image_cb, evas_common_soft16_image_cache_get());
   if (!re->cache)
     {
        fprintf(stderr, "Evas_Cache_Engine_Image allocation failed!\n");
        exit(-1);
     }

   re->tb = evas_common_tilebuf_new(w, h);
   /* in preliminary tests 16x16 gave highest framerates */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   re->w = w;
   re->h = h;
   re->rot = rotation;
   re->flags.hwsurface = hwsurface;
   re->flags.fullscreen = fullscreen;
   re->flags.noframe = noframe;
   re->flags.end = 0;

   re->update_rects_count = 0;
   re->update_rects_limit = 0;
   re->update_rects = NULL;

   surface = SDL_SetVideoMode(w, h, 16,
                              (hwsurface ? SDL_HWSURFACE : SDL_SWSURFACE)
                              | (fullscreen ? SDL_FULLSCREEN : 0)
                              | (noframe ? SDL_NOFRAME : 0));
   if (!surface)
     {
        fprintf(stderr, "SDL_SetVideoMode [ %i x %i x 16 ] failed\n", w, h);
        exit(-1);
     }

   SDL_SetAlpha(surface, SDL_RLEACCEL, 0);
   SDL_FillRect(surface, NULL, 0);

   re->soft16_engine_image = (SDL_Engine_Image_Entry *) evas_cache_engine_image_engine(re->cache, surface);
   if (!re->soft16_engine_image)
     {
        fprintf(stderr, "Soft16_Image allocation from SDL failed\n");
        exit(-1);
     }

   return re;
}


static void
evas_engine_sdl16_setup(Evas *e, void *in)
{
   Evas_Engine_Info_SDL_16      *info = (Evas_Engine_Info_SDL_16 *) in;

   if (evas_output_method_get(e) != evas_render_method_lookup("software_16_sdl"))
     return ;

   SDL_Init(SDL_INIT_NOPARACHUTE);

   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
     {
        fprintf(stderr, "SDL_Init failed with %s\n", SDL_GetError());
        exit(-1);
     }

   e->engine.data.output = _sdl16_output_setup(e->output.w, e->output.h,
                                               info->info.rotation,
                                               info->info.fullscreen,
                                               info->info.noframe,
                                               info->info.hwsurface);
   if (!e->engine.data.output)
     return;

   e->engine.func = &func;
   e->engine.data.context = e->engine.func->context_new(e->engine.data.output);
}

static void
evas_engine_sdl16_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re->tb) evas_common_tilebuf_free(re->tb);
   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   if (re->tmp_out) evas_cache_image_drop(&re->tmp_out->cache_entry);
   if (re->soft16_engine_image) evas_cache_engine_image_drop(&re->soft16_engine_image->cache_entry);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
   evas_common_soft16_image_shutdown();
}

static void
evas_engine_sdl16_output_resize(void *data, int w, int h)
{
   Render_Engine        *re = data;
   SDL_Surface          *surface;

   if ((re->w == w) && (re->h == h)) return;

   evas_cache_engine_image_drop(&re->soft16_engine_image->cache_entry);

   evas_common_tilebuf_free(re->tb);
   re->w = w;
   re->h = h;
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb)
     evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   surface = SDL_SetVideoMode(w, h, 16,
                              (re->flags.hwsurface ? SDL_HWSURFACE : SDL_SWSURFACE)
                              | (re->flags.fullscreen ? SDL_FULLSCREEN : 0)
                              | (re->flags.noframe ? SDL_NOFRAME : 0));
   if (!surface)
     {
        fprintf(stderr, "Unable to change the resolution to : %ix%i\n", w, h);
        exit(-1);
     }
   re->soft16_engine_image = (SDL_Engine_Image_Entry *) evas_cache_engine_image_engine(re->cache, surface);
   if (!re->soft16_engine_image)
     {
	fprintf(stderr, "RGBA_Image allocation from SDL failed\n");
	exit(-1);
     }

   SDL_FillRect(surface, NULL, 0);

   if (re->tmp_out)
     {
        evas_cache_image_drop(&re->tmp_out->cache_entry);
	re->tmp_out = NULL;
     }
}

static void
evas_engine_sdl16_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void
evas_engine_sdl16_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void
evas_engine_sdl16_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void
evas_engine_sdl16_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_clear(re->tb);
}

static void *
evas_engine_sdl16_output_redraws_next_update_get(void *data,
                                                 int *x, int *y, int *w, int *h,
                                                 int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine        *re = data;
   Tilebuf_Rect         *tb_rect;
   SDL_Rect              rect;

   if (re->flags.end)
     {
	re->flags.end = 0;
	return NULL;
     }
   if (!re->rects)
     {
	re->rects = evas_common_tilebuf_get_render_rects(re->tb);
	re->cur_rect = re->rects;
	if (re->rot != 0) _tmp_out_alloc(re); /* grows if required */
     }
   if (!re->cur_rect)
     {
	if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	return NULL;
     }

   tb_rect = re->cur_rect;
   *cx = *x = tb_rect->x;
   *cy = *y = tb_rect->y;
   *cw = *w = tb_rect->w;
   *ch = *h = tb_rect->h;
   re->cur_rect = (Tilebuf_Rect *)((EINA_INLIST_GET(re->cur_rect))->next);
   if (!re->cur_rect)
     {
        evas_common_tilebuf_free_render_rects(re->rects);
        re->rects = NULL;
        re->flags.end = 1;
     }

   if (re->rot != 0)
     {
        *cx = 0;
        *cy = 0;
     }

   rect.x = *x;
   rect.y = *y;
   rect.w = *w;
   rect.h = *h;

   SDL_FillRect(re->soft16_engine_image->surface, &rect, 0);

   /* Return the "fake" surface so it is passed to the drawing routines. */
   return re->soft16_engine_image;
}

static void
_blit_rot_90(Soft16_Image *dst, const Soft16_Image *src,
	     int out_x, int out_y, int w, int h)
{
   DATA16 *dp, *sp;
   int x, y;

   sp = src->pixels;
   dp = dst->pixels + (out_x +
		       (w + out_y - 1) * dst->stride);

   for (y = 0; y < h; y++)
     {
	DATA16 *dp_itr, *sp_itr;

	sp_itr = sp;
	dp_itr = dp;

	for (x = 0; x < w; x++)
	  {
	     *dp_itr = *sp_itr;

	     sp_itr++;
	     dp_itr -= dst->stride;
	  }
	sp += src->stride;
	dp++;
     }
}

static void
_blit_rot_180(Soft16_Image *dst, const Soft16_Image *src,
	      int out_x, int out_y, int w, int h)
{
   DATA16 *dp, *sp;
   int x, y;

   sp = src->pixels;
   dp = dst->pixels + ((w + out_x - 1) +
		       (h + out_y - 1) * dst->stride);

   for (y = 0; y < h; y++)
     {
	DATA16 *dp_itr, *sp_itr;

	sp_itr = sp;
	dp_itr = dp;

	for (x = 0; x < w; x++)
	  {
	     *dp_itr = *sp_itr;

	     sp_itr++;
	     dp_itr--;
	  }
	sp += src->stride;
	dp -= dst->stride;
     }
}

static void
_blit_rot_270(Soft16_Image *dst, const Soft16_Image *src,
	      int out_x, int out_y, int w, int h)
{
   DATA16 *dp, *sp;
   int x, y;

   sp = src->pixels;
   dp = dst->pixels + ((h + out_x - 1) +
		       out_y * dst->stride);

   for (y = 0; y < h; y++)
     {
	DATA16 *dp_itr, *sp_itr;

	sp_itr = sp;
	dp_itr = dp;

	for (x = 0; x < w; x++)
	  {
	     *dp_itr = *sp_itr;

	     sp_itr++;
	     dp_itr += dst->stride;
	  }
	sp += src->stride;
	dp--;
     }
}

static void
_tmp_out_process(Render_Engine *re, int out_x, int out_y, int w, int h)
{
   Soft16_Image *d, *s;
   DATA16 *dp, *sp;
   int y, x, d_dir;

   d = (Soft16_Image *) re->soft16_engine_image->cache_entry.src;
   s = re->tmp_out;

   if ((w < 1) || (h < 1)
       || (out_x >= d->cache_entry.w) || (out_y >= d->cache_entry.h))
     return;

   if (re->rot == 90)
     _blit_rot_90(d, s, out_x, out_y, w, h);
   else if (re->rot == 180)
     _blit_rot_180(d, s, out_x, out_y, w, h);
   else if (re->rot == 270)
     _blit_rot_270(d, s, out_x, out_y, w, h);
}

static void
evas_engine_sdl16_output_redraws_next_update_push(void *data, void *surface __UNUSED__,
                                                  int x, int y, int w, int h)
{
   Render_Engine        *re = data;
   SDL_Rect              rect;

   if (re->update_rects_count + 1 > re->update_rects_limit)
     {
        re->update_rects_limit += 8;
        re->update_rects = realloc(re->update_rects, sizeof (SDL_Rect) * re->update_rects_limit);
     }

   rect.x = x;
   rect.y = y;
   rect.w = w;
   rect.h = h;

   switch (re->rot)
     {
      case 0:
         break;
      case 90:
         rect.x = y;
         rect.y = re->w - w - x;
         rect.w = h;
         rect.h = w;
         break;
      case 180:
         rect.x = re->w - w - x;
         rect.y = re->h - h - y;
         break;
      case 270:
         rect.x = re->h - h - y;
         rect.y = x;
         rect.w = h;
         rect.h = w;
         break;
      default:
         abort();
     }

   re->update_rects[re->update_rects_count] = rect;

   if (re->rot != 0)
     _tmp_out_process(re, rect.x, rect.y, w, h);

   ++re->update_rects_count;
}

static void
evas_engine_sdl16_output_flush(void *data)
{
   Render_Engine        *re = data;

   if (re->update_rects_count > 0)
     SDL_UpdateRects(re->soft16_engine_image->surface, re->update_rects_count, re->update_rects);

   re->update_rects_count = 0;
}

static void
evas_engine_sdl16_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re->tmp_out)
     {
	evas_cache_image_drop(&re->tmp_out->cache_entry);
	re->tmp_out = NULL;
     }
}

static void*
evas_engine_sdl16_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine*	re = (Render_Engine*) data;;

   *error = 0;
   return evas_cache_engine_image_request(re->cache, file, key, lo, NULL, error);
}

static int
evas_engine_sdl16_image_alpha_get(void *data __UNUSED__, void *image)
{
   SDL_Engine_Image_Entry       *eim = image;
   Soft16_Image                 *im;

   if (!eim) return 1;
   im = (Soft16_Image *) eim->cache_entry.src;
   if (im->cache_entry.flags.alpha) return 1;
   return 0;
}

static void
evas_engine_sdl16_image_size_get(void *data __UNUSED__, void *image, int *w, int *h)
{
   SDL_Engine_Image_Entry       *eim;

   eim = image;
   if (w) *w = eim->cache_entry.w;
   if (h) *h = eim->cache_entry.h;
}

static int
evas_engine_sdl16_image_colorspace_get(void *data __UNUSED__, void *image __UNUSED__)
{
   return EVAS_COLORSPACE_RGB565_A5P;
}

static void
evas_engine_sdl16_image_colorspace_set(void *data __UNUSED__, void *image __UNUSED__, int cspace __UNUSED__)
{
   /* FIXME: Not implemented. */
}

static void*
evas_engine_sdl16_image_new_from_copied_data(void *data,
                                             int w, int h,
                                             DATA32* image_data,
                                             int alpha, int cspace)
{
   Render_Engine	*re = data;

   if (cspace != EVAS_COLORSPACE_RGB565_A5P)
     {
        fprintf(stderr, "Unsupported colorspace %d in %s() (%s:%d)\n",
                cspace, __FUNCTION__, __FILE__, __LINE__);
        return NULL;
     }

   fprintf(stderr, "s image_data: %p\n", image_data);

   return evas_cache_engine_image_copied_data(re->cache,
                                              w, h,
                                              image_data,
                                              alpha, cspace, NULL);
}

static void*
evas_engine_sdl16_image_new_from_data(void *data, int w, int h, DATA32* image_data, int alpha, int cspace)
{
   Render_Engine	*re = data;

   if (cspace != EVAS_COLORSPACE_RGB565_A5P)
     {
        fprintf(stderr, "Unsupported colorspace %d in %s() (%s:%d)\n",
                cspace, __FUNCTION__, __FILE__, __LINE__);
        return NULL;
     }

   return evas_cache_engine_image_data(re->cache,
                                       w, h,
                                       image_data,
                                       alpha, cspace, NULL);
}

static void
evas_engine_sdl16_image_free(void *data __UNUSED__, void *image)
{
   SDL_Engine_Image_Entry       *eim = image;

   evas_cache_engine_image_drop(&eim->cache_entry);
}

static void*
evas_engine_sdl16_image_size_set(void *data __UNUSED__, void *image, int w, int h)
{
   SDL_Engine_Image_Entry       *eim = image;

   return evas_cache_engine_image_size_set(&eim->cache_entry, w, h);
}

static void*
evas_engine_sdl16_image_dirty_region(void *data __UNUSED__,
                                     void *image,
                                     int x, int y, int w, int h)
{
   SDL_Engine_Image_Entry       *eim = image;

   return evas_cache_engine_image_dirty(&eim->cache_entry, x, y, w, h);
}

static void*
evas_engine_sdl16_image_data_get(void *data __UNUSED__, void *image,
                                 int to_write, DATA32** image_data)
{
   SDL_Engine_Image_Entry       *eim = image;
   Soft16_Image                 *im;

   if (!eim)
     {
        *image_data = NULL;
        return NULL;
     }
   im = (Soft16_Image *) eim->cache_entry.src;
   evas_cache_image_load_data(&im->cache_entry);

   if (to_write)
     eim = (SDL_Engine_Image_Entry *) evas_cache_engine_image_alone(&eim->cache_entry,
                                                                    NULL);

   /* FIXME: Handle colorspace convertion correctly. */
   if (image_data) *image_data = (DATA32 *) im->pixels;

   return eim;
}

static void*
evas_engine_sdl16_image_data_put(void *data, void *image, DATA32* image_data)
{
   SDL_Engine_Image_Entry       *eim = image;
   SDL_Engine_Image_Entry       *eim_new;
   Render_Engine                *re = data;
   Soft16_Image                 *im;

   if (!eim) return NULL;
   im = (Soft16_Image *) eim->cache_entry.src;

   /* FIXME: Handle colorspace convertion correctly. */
   if ((DATA16 *) image_data == im->pixels) return eim;

   eim_new = (SDL_Engine_Image_Entry *) evas_cache_engine_image_data(re->cache,
                                                                     eim->cache_entry.w, eim->cache_entry.h,
                                                                     image_data,
                                                                     func.image_alpha_get(data, eim),
                                                                     func.image_colorspace_get(data, eim),
                                                                     NULL);
   evas_cache_engine_image_drop(&eim->cache_entry);

   return eim_new;
}

static void
evas_engine_sdl16_image_data_preload_request(void *data __UNUSED__, void *image, const void *target)
{
   SDL_Engine_Image_Entry       *eim = image;
   Soft16_Image                 *im;

   if (!eim) return ;
   im = (Soft16_Image *) eim->cache_entry.src;
   if (!im) return ;
   evas_cache_image_preload_data(&im->cache_entry, target);
}

static void
evas_engine_sdl16_image_data_preload_cancel(void *data __UNUSED__, void *image, const void *target)
{
   SDL_Engine_Image_Entry       *eim = image;
   Soft16_Image                 *im;

   if (!eim) return ;
   im = (Soft16_Image *) eim->cache_entry.src;
   if (!im) return ;
   evas_cache_image_preload_cancel(&im->cache_entry, target);
}

static void*
evas_engine_sdl16_image_alpha_set(void *data __UNUSED__, void *image, int has_alpha)
{
   SDL_Engine_Image_Entry       *eim = image;
   Soft16_Image                 *im;

   if (!eim) return NULL;

   im = (Soft16_Image *) eim->cache_entry.src;

   if (im->cache_entry.flags.alpha == has_alpha) return eim;

   eim = (SDL_Engine_Image_Entry *) evas_cache_engine_image_alone(&eim->cache_entry,
                                                                  NULL);

   im = (Soft16_Image *) eim->cache_entry.src;

   im->cache_entry.flags.alpha = has_alpha;
   eim = (SDL_Engine_Image_Entry *) evas_cache_engine_image_dirty(&eim->cache_entry, 0, 0, eim->cache_entry.w, eim->cache_entry.h);

   return eim;
}

static void*
evas_engine_sdl16_image_border_set(void *data __UNUSED__, void *image, int l __UNUSED__, int r __UNUSED__, int t __UNUSED__, int b __UNUSED__)
{
   return image;
}

static void
evas_engine_sdl16_image_border_get(void *data __UNUSED__, void *image __UNUSED__, int *l __UNUSED__, int *r __UNUSED__, int *t __UNUSED__, int *b __UNUSED__)
{
   /* FIXME: need to know what evas expect from this call */
}

static void
evas_engine_sdl16_image_draw(void *data __UNUSED__, void *context, void *surface, void *image,
                             int src_region_x, int src_region_y, int src_region_w, int src_region_h,
                             int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h,
                             int smooth)
{
   SDL_Engine_Image_Entry       *eim = image;
   SDL_Engine_Image_Entry       *dst = surface;
   int                           mustlock_im = 0;
   int                           mustlock_dst = 0;

   evas_cache_engine_image_load_data(&eim->cache_entry);

   /* Fallback to software method */
   if (SDL_MUSTLOCK(dst->surface))
     {
        mustlock_dst = 1;
	SDL_LockSurface(dst->surface);
	_SDL_UPDATE_PIXELS(dst);
     }

   if (eim->surface && SDL_MUSTLOCK(eim->surface))
     {
        mustlock_im = 1;
	SDL_LockSurface(eim->surface);
	_SDL_UPDATE_PIXELS(eim);
     }

   soft16_image_draw((Soft16_Image *) eim->cache_entry.src,
                     (Soft16_Image *) dst->cache_entry.src,
                     context,
                     src_region_x, src_region_y, src_region_w, src_region_h,
                     dst_region_x, dst_region_y, dst_region_w, dst_region_h,
                     smooth);

   if (mustlock_im)
     SDL_UnlockSurface(eim->surface);

   if (mustlock_dst)
     SDL_UnlockSurface(dst->surface);
}

static void
evas_engine_sdl16_image_cache_flush(void *data)
{
   Render_Engine        *re = (Render_Engine*) data;
   int                   size;

   size = evas_cache_engine_image_get(re->cache);
   evas_cache_engine_image_set(re->cache, 0);
   evas_cache_engine_image_set(re->cache, size);
}

static void
evas_engine_sdl16_image_cache_set(void *data, int bytes)
{
   Render_Engine        *re = (Render_Engine*) data;

   evas_cache_engine_image_set(re->cache, bytes);
}

static int
evas_engine_sdl16_image_cache_get(void *data)
{
   Render_Engine        *re = (Render_Engine*) data;

   return evas_cache_engine_image_get(re->cache);
}

static char*
evas_engine_sdl16_image_comment_get(void *data __UNUSED__, void *image __UNUSED__, char *key __UNUSED__)
{
   return NULL;
}

static char*
evas_engine_sdl16_image_format_get(void *data __UNUSED__, void *image __UNUSED__)
{
   /* FIXME: need to know what evas expect from this call */
   return NULL;
}

static void
evas_engine_sdl16_font_draw(void *data __UNUSED__, void *context, void *surface, void *font, int x, int y, int w __UNUSED__, int h __UNUSED__, int ow __UNUSED__, int oh __UNUSED__, const char *text)
{
   static RGBA_Image            *im = NULL;
   SDL_Engine_Image_Entry       *eim = surface;
   Soft16_Image                 *dst = (Soft16_Image *) eim->cache_entry.src;
   int                           mustlock_im = 0;

   if (!im)
     im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
   evas_cache_image_surface_alloc(&im->cache_entry, dst->cache_entry.w, dst->cache_entry.h);

   if (eim->surface && SDL_MUSTLOCK(eim->surface))
     {
        mustlock_im = 1;
	SDL_LockSurface(eim->surface);
	_SDL_UPDATE_PIXELS(eim);
     }
   evas_common_draw_context_font_ext_set(context,
                                         dst,
                                         soft16_font_glyph_new,
                                         soft16_font_glyph_free,
                                         soft16_font_glyph_draw);
   evas_common_font_draw(im, context, font, x, y, text);
   evas_common_draw_context_font_ext_set(context,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL);

   if (mustlock_im)
     SDL_UnlockSurface(eim->surface);
}

static void
evas_engine_sdl16_line_draw(void *data __UNUSED__, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   SDL_Engine_Image_Entry       *eim = surface;
   int                           mustlock_im = 0;

   if (eim->surface && SDL_MUSTLOCK(eim->surface))
     {
        mustlock_im = 1;
	SDL_LockSurface(eim->surface);
	_SDL_UPDATE_PIXELS(eim);
     }

   soft16_line_draw((Soft16_Image *) eim->cache_entry.src,
                    context,
                    x1, y1, x2, y2);

   if (mustlock_im)
     SDL_UnlockSurface(eim->surface);
}

static void
evas_engine_sdl16_rectangle_draw(void *data __UNUSED__, void *context, void *surface, int x, int y, int w, int h)
{
   SDL_Engine_Image_Entry       *eim = surface;
#if ENGINE_SDL_PRIMITIVE
   RGBA_Draw_Context            *dc = context;
#endif
   Soft16_Image                 *im;
   int                           mustlock_im = 0;

#if ENGINE_SDL_PRIMITIVE
   if (A_VAL(&dc->col.col) != 0x00)
     {
        if (A_VAL(&dc->col.col) != 0xFF)
          {
#endif
             if (eim->surface && SDL_MUSTLOCK(eim->surface))
               {
                  mustlock_im = 1;
                  SDL_LockSurface(eim->surface);
                  _SDL_UPDATE_PIXELS(eim);
               }

             im = (Soft16_Image *) eim->cache_entry.src;

             soft16_rectangle_draw(im, context, x, y, w, h);

             if (mustlock_im)
               SDL_UnlockSurface(eim->surface);
#if ENGINE_SDL_PRIMITIVE
          }
        else
          {
             SDL_Rect        dstrect;

             if (dc->clip.use)
               {
                  SDL_Rect   cliprect;

                  cliprect.x = dc->clip.x;
                  cliprect.y = dc->clip.y;
                  cliprect.w = dc->clip.w;
                  cliprect.h = dc->clip.h;

                  SDL_SetClipRect(eim->surface, &cliprect);
               }

             dstrect.x = x;
             dstrect.y = y;
             dstrect.w = w;
             dstrect.h = h;

             SDL_FillRect(eim->surface, &dstrect, SDL_MapRGBA(eim->surface->format, R_VAL(&dc->col.col), G_VAL(&dc->col.col), B_VAL(&dc->col.col), 0xFF));

             if (dc->clip.use)
               SDL_SetClipRect(eim->surface, NULL);
          }
     }
#endif
}

static void
evas_engine_sdl16_polygon_draw(void *data __UNUSED__, void *context, void *surface, void *polygon)
{
   SDL_Engine_Image_Entry       *eim = surface;
   int                           mustlock_im = 0;

   if (eim->surface && SDL_MUSTLOCK(eim->surface))
     {
        mustlock_im = 1;
	SDL_LockSurface(eim->surface);
	_SDL_UPDATE_PIXELS(eim);
     }

   soft16_polygon_draw((Soft16_Image *) eim->cache_entry.src, context, polygon);

   if (mustlock_im)
     SDL_UnlockSurface(eim->surface);
}

static void
evas_engine_sdl16_image_stride_get(void *data __UNUSED__, void *image, int *stride)
{
   SDL_Engine_Image_Entry       *eim = image;

   if (stride) *stride = 0;
   if (!image) return;
   if (stride) *stride = ((Soft16_Image*) eim->cache_entry.src)->stride;
}

/* module advertising code */
EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_16")) return 0;
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, evas_engine_sdl16_)
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
   ORD(image_alpha_get);
   ORD(image_size_get);
   ORD(image_colorspace_get);
   ORD(image_colorspace_set);
   ORD(image_new_from_copied_data);
   ORD(image_new_from_data);
   ORD(image_free);
   ORD(image_size_set);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_data_preload_request);
   ORD(image_data_preload_cancel);
   ORD(image_alpha_set);
   ORD(image_border_set);
   ORD(image_border_get);
   ORD(image_draw);
   ORD(image_cache_flush);
   ORD(image_cache_set);
   ORD(image_cache_get);
   ORD(image_comment_get);
   ORD(image_format_get);
   ORD(image_stride_get);
   ORD(font_draw);
   ORD(line_draw);
   ORD(rectangle_draw);
   ORD(polygon_draw);
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
     "software_16_sdl",
     "none"
};

static Engine_Image_Entry*
_sdl16_image_alloc(void)
{
   SDL_Engine_Image_Entry       *new;

   new = calloc(1, sizeof (SDL_Engine_Image_Entry));

   return (Engine_Image_Entry *) new;
}

static void
_sdl16_image_delete(Engine_Image_Entry *eim)
{
   free(eim);
}

static int
_sdl16_image_constructor(Engine_Image_Entry *ie, void* data __UNUSED__)
{
   SDL_Surface                  *sdl = NULL;
   SDL_Engine_Image_Entry       *eim = (SDL_Engine_Image_Entry *) ie;
   Soft16_Image                 *im;

   im = (Soft16_Image *) ie->src;

   if (im->pixels)
     {
        /* FIXME: Take care of CSPACE */
        sdl = SDL_CreateRGBSurfaceFrom(im->pixels,
                                       ie->w, ie->h,
                                       16, ie->w * 2,
                                       RMASK565, GMASK565, BMASK565, AMASK565);
        eim->surface = sdl;
        eim->flags.engine_surface = 0;
     }

   return 0;
}

static void
_sdl16_image_destructor(Engine_Image_Entry *eim)
{
   SDL_Engine_Image_Entry       *seie = (SDL_Engine_Image_Entry *) eim;

   if (seie->surface && !seie->flags.engine_surface)
     SDL_FreeSurface(seie->surface);
   seie->surface = NULL;
}

static void
_sdl16_image_dirty_region(Engine_Image_Entry *eim, int x, int y, int w, int h)
{
   SDL_Engine_Image_Entry       *dst;
   RGBA_Image *im;

   dst = (SDL_Engine_Image_Entry *) eim;

   SDL_UpdateRect(dst->surface, x, y, w, h);

   im = (RGBA_Image *)eim->src;
   im->flags |= RGBA_IMAGE_IS_DIRTY;
}

static int
_sdl16_image_dirty(Engine_Image_Entry *dst, const Engine_Image_Entry *src __UNUSED__)
{
   SDL_Engine_Image_Entry       *eim = (SDL_Engine_Image_Entry *) dst;
   SDL_Surface                  *sdl = NULL;
   Soft16_Image                 *im;

   im = (Soft16_Image *) dst->src;

   /* FIXME: Take care of CSPACE */
   sdl = SDL_CreateRGBSurfaceFrom(im->pixels,
                                  dst->w, dst->h,
                                  16, dst->w * 2,
                                  RMASK565, GMASK565, BMASK565, AMASK565);
   eim->surface = sdl;
   eim->flags.engine_surface = 0;

   return 0;
}

static int
_sdl16_image_size_set(Engine_Image_Entry *dst, const Engine_Image_Entry *src __UNUSED__)
{
   SDL_Engine_Image_Entry       *eim = (SDL_Engine_Image_Entry *) dst;
   SDL_Surface                  *sdl;
   Soft16_Image                 *im;

   im = (Soft16_Image *) dst->src;

   /* FIXME: handle im == NULL */
   sdl = SDL_CreateRGBSurfaceFrom(im->pixels,
                                  dst->w, dst->h,
                                  16, dst->w * 2,
                                  RMASK565, GMASK565, BMASK565, AMASK565);

   eim->surface = sdl;

   return 0;
}

static int
_sdl16_image_update_data(Engine_Image_Entry* dst, void* engine_data)
{
   SDL_Engine_Image_Entry       *eim = (SDL_Engine_Image_Entry *) dst;
   SDL_Surface                  *sdl = NULL;
   Soft16_Image                 *im;

   im = (Soft16_Image *) dst->src;

   if (engine_data)
     {
        sdl = engine_data;

        if (im)
          {
             im->pixels = sdl->pixels;
             im->stride = sdl->pitch / 2;
             im->flags.free_pixels = 0;
/*              im->alpha = calloc(1, sizeof (DATA8) * _calc_stride(sdl->w) * sdl->h); */
/*              im->flags.free_alpha = 0; */
/*              im->flags.have_alpha = 1; */
             im->alpha = NULL;
             im->flags.free_alpha = 0;
             im->cache_entry.flags.alpha = 0;

             dst->src->w = sdl->w;
             dst->src->h = sdl->h;
          }
        dst->w = sdl->w;
        dst->h = sdl->h;
     }
   else
     {
        SDL_FreeSurface(eim->surface);
        /* FIXME: Take care of CSPACE */
        sdl = SDL_CreateRGBSurfaceFrom(im->pixels,
                                       dst->w, dst->h,
                                       16, dst->w * 2,
                                       RMASK565, GMASK565, BMASK565, AMASK565);
     }

   eim->surface = sdl;

   return 0;
}

static void
_sdl16_image_load(Engine_Image_Entry *eim, const Image_Entry* ie_im)
{
   SDL_Engine_Image_Entry       *load = (SDL_Engine_Image_Entry *) eim;
   SDL_Surface                  *sdl;

   if (!load->surface)
     {
        Soft16_Image            *im;

        im = (Soft16_Image *) ie_im;

        sdl = SDL_CreateRGBSurfaceFrom(im->pixels,
                                       eim->w, eim->h,
                                       16, eim->w * 2,
                                       RMASK565, GMASK565, BMASK565, AMASK565);
        load->surface = sdl;
     }
}

static int
_sdl16_image_mem_size_get(Engine_Image_Entry *eim)
{
   SDL_Engine_Image_Entry       *seie = (SDL_Engine_Image_Entry *) eim;
   int                           size = 0;

   /* FIXME: Count surface size. */
   if (seie->surface)
     size = sizeof (SDL_Surface) + sizeof (SDL_PixelFormat);

   return size;
}

#ifdef DEBUG_SDL
static void
_sdl16_image_debug(const char* context, Engine_Image_Entry* im)
{
}
#endif
