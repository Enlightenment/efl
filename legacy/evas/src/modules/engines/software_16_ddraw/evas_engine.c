#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Software_16_DDraw.h"
#include "evas_common_soft16.h"

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   HWND                 window;
   LPDIRECTDRAW         object;
   LPDIRECTDRAWSURFACE  surface_primary;
   LPDIRECTDRAWSURFACE  surface_back;
   LPDIRECTDRAWSURFACE  surface_source;
   int                  width;
   int                  height;
   int                  rotation;
   Tilebuf             *tb;
   Tilebuf_Rect        *rects;
   Tilebuf_Rect        *cur_rect;
   DDraw_Output_Buffer *ddob;
   Soft16_Image        *tmp_out; /* used by indirect render, like rotation */
   HRGN                 clip_rects;
   unsigned char        end : 1;
};

/* prototypes we will use here */

static void *eng_info(Evas *e);
static void eng_info_free(Evas *e, void *info);
static void eng_setup(Evas *e, void *info);
static void eng_output_free(void *data);
static void eng_output_resize(void *data, int w, int h);
static void eng_output_tile_size_set(void *data, int w, int h);
static void eng_output_redraws_rect_add(void *data, int x, int y, int w, int h);
static void eng_output_redraws_rect_del(void *data, int x, int y, int w, int h);
static void eng_output_redraws_clear(void *data);
static void *eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
static void eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h);
static void eng_output_flush(void *data);
static void eng_output_idle_flush(void *data);

/* engine api this module provides */
static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_Software_16_DDraw *info;

   info = calloc(1, sizeof(Evas_Engine_Info_Software_16_DDraw));
   if (!info) return NULL;
   info->magic.magic = rand();
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_Software_16_DDraw *in;

   in = (Evas_Engine_Info_Software_16_DDraw *)info;
   free(in);
}

static void
_tmp_out_free(Soft16_Image *tmp_out)
{
   free(tmp_out->pixels);
   free(tmp_out);
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
	if ((re->tmp_out->w < w) || (re->tmp_out->h < h))
	  {
	     _tmp_out_free(re->tmp_out);
	     re->tmp_out = NULL;
	  }
     }

   if (!re->tmp_out)
     {
	Soft16_Image *im;

	im = calloc(1, sizeof(Soft16_Image));
	im->w = w;
	im->h = h;
	im->stride = w + ((w % 4) ? (4 - (w % 4)) : 0);
	im->have_alpha = 0;
	im->references = 1;
	im->free_pixels = 1;
	im->pixels = malloc(h * im->stride * sizeof(DATA16));

	re->tmp_out = im;
     }
}


static void
eng_setup(Evas *e, void *in)
{
   Render_Engine                      *re;
   Evas_Engine_Info_Software_16_DDraw *info;

   info = (Evas_Engine_Info_Software_16_DDraw *)in;
   if (!e->engine.data.output)
     {
	/* the only check - simplistic, i know, but enough for this
	 * "special purpose" engine. Remember it is meant to be used
	 * for limited power devices that have a 16bit display mode
	 * and no real other acceleration, and high resolution so we
	 * can pre-dither into 16bpp. */
        if (info->info.depth != 16)
         return;
	/* do common routine init - we wil at least use it for core
	 * image loading and font loading/glyph rendering & placement */
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

	/* render engine specific data */
	re = calloc(1, sizeof(Render_Engine));
        if (!re)
          return;
	e->engine.data.output = re;
	re->window = info->info.window;
	re->object = info->info.object;
	re->surface_primary = info->info.surface_primary;
	re->surface_back = info->info.surface_back;
	re->surface_source = info->info.surface_source;
	re->width = e->output.w;
	re->height = e->output.h;
	re->rotation = info->info.rotation;
	re->tb = evas_common_tilebuf_new(e->output.w, e->output.h);
	if (re->tb)
	  evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
     }
   else
     {
	/* we changed the info after first init - do a re-eval where
	 * appropriate */
       if (info->info.depth != 16)
         return;
	re = e->engine.data.output;
	if (re->tb) evas_common_tilebuf_free(re->tb);
	re->window = info->info.window;
	re->object = info->info.object;
	re->surface_primary = info->info.surface_primary;
	re->surface_back = info->info.surface_back;
	re->surface_source = info->info.surface_source;
	re->width = e->output.w;
	re->height = e->output.h;
	re->rotation = info->info.rotation;
	re->tb = evas_common_tilebuf_new(e->output.w, e->output.h);
	if (re->tb)
	  evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
	if (re->tmp_out)
	  {
	     _tmp_out_free(re->tmp_out);
	     re->tmp_out = NULL;
	  }
     }
   if (!e->engine.data.output) return;
   /* add a draw context if we dont have one */
   if (!e->engine.data.context)
     e->engine.data.context =
     e->engine.func->context_new(e->engine.data.output);
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re->ddob) evas_software_ddraw_output_buffer_free(re->ddob, 0);
   if (re->clip_rects) DeleteObject(re->clip_rects);
   if (re->tb) evas_common_tilebuf_free(re->tb);
   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   if (re->tmp_out) _tmp_out_free(re->tmp_out);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
   evas_common_soft16_image_shutdown();
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   if ((re->width == w) && (re->height == h)) return;

   if (re->ddob)
     evas_software_ddraw_surface_resize(re->ddob);

   evas_common_tilebuf_free(re->tb);
   re->width = w;
   re->height = h;
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb)
     evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   if (re->ddob)
     {
        evas_software_ddraw_output_buffer_free(re->ddob, 0);
	re->ddob = NULL;
     }
   if (re->clip_rects)
     {
	DeleteObject(re->clip_rects);
	re->clip_rects = NULL;
     }
   if (re->tmp_out)
     {
	_tmp_out_free(re->tmp_out);
	re->tmp_out = NULL;
     }
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

static inline void
_output_buffer_alloc(Render_Engine *re)
{
   int width;
   int height;

   if (re->ddob) return;

   if ((re->rotation == 0) || (re->rotation == 180))
     {
	width = re->width;
	height = re->height;
     }
   else
     {
	width = re->height;
	height = re->width;
     }

   re->ddob = evas_software_ddraw_output_buffer_new(re->window,
                                                    re->object,
                                                    re->surface_primary,
                                                    re->surface_back,
                                                    re->surface_source,
                                                    width,
                                                    height);
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
	if (!re->rects) return NULL;

	re->cur_rect = re->rects;
	_output_buffer_alloc(re);
	if (re->rotation != 0) _tmp_out_alloc(re); /* grows if required */
     }
   if (!re->cur_rect)
     {
	if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	return NULL;
     }
   rect = re->cur_rect;
   ux = rect->x; uy = rect->y; uw = rect->w; uh = rect->h;
   re->cur_rect = (Tilebuf_Rect *)((EINA_INLIST_GET(re->cur_rect))->next);
   if (!re->cur_rect)
     {
	evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	re->end = 1;
     }

   *x = ux; *y = uy; *w = uw; *h = uh;
   if (re->rotation == 0)
     {
	*cx = ux; *cy = uy; *cw = uw; *ch = uh;
	return &re->ddob->im;
     }
   else
     {
	*cx = 0; *cy = 0; *cw = uw; *ch = uh;
	return re->tmp_out;
     }
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

   d = &re->ddob->im;
   s = re->tmp_out;

   if ((w < 1) || (h < 1) || (out_x >= d->w) || (out_y >= d->h))
     return;

   if (re->rotation == 90)
     _blit_rot_90(d, s, out_x, out_y, w, h);
   else if (re->rotation == 180)
     _blit_rot_180(d, s, out_x, out_y, w, h);
   else if (re->rotation == 270)
     _blit_rot_270(d, s, out_x, out_y, w, h);
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;
   HRGN           region;
   int            xx;
   int            yy;
   int            width;
   int            height;

   re = (Render_Engine *)data;

   if (!re->clip_rects)
      re->clip_rects = CreateRectRgn(0, 0, 0, 0);

   if (re->rotation == 0)
     {
	xx = x;
	yy = y;
	width = w;
	height = h;
     }
   else if (re->rotation == 90)
     {
	xx = y;
	yy = re->width - w - x;
	width = h;
	height = w;
     }
   else if (re->rotation == 180)
     {
	xx = re->width - w - x;
	yy = re->height - h - y;
	width = w;
	height = h;
     }
   else if (re->rotation == 270)
     {
	xx = re->height - h - y;
	yy = x;
	width = h;
	height = w;
     }

   region = CreateRectRgn(xx, yy, xx + width, yy + height);

   if (re->rotation != 0)
     _tmp_out_process(re, xx, yy, w, h);
   CombineRgn(re->clip_rects, re->clip_rects, region, RGN_OR);
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;
   void          *ddraw_data;
   int            ddraw_width;
   int            ddraw_height;
   int            ddraw_pitch;
   int            ddraw_depth;

   re = (Render_Engine *)data;
   if (re->clip_rects)
     {
        /* FIXME : i have to manage that */
/* 	XSetRegion(re->disp, re->gc, re->clip_rects); */
	DeleteObject(re->clip_rects);
	re->clip_rects = NULL;
     }
   else return;

   evas_software_ddraw_output_buffer_paste(re->ddob);

   /* FIXME : i have to manage that */
/*    XSetClipMask(re->disp, re->gc, None); */
}

static void
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re->ddob)
     {
	evas_software_ddraw_output_buffer_free(re->ddob, 0);
	re->ddob = NULL;
     }
   if (re->clip_rects)
     {
	DeleteObject(re->clip_rects);
	re->clip_rects = NULL;
     }
   if (re->tmp_out)
     {
	_tmp_out_free(re->tmp_out);
	re->tmp_out = NULL;
     }
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
     "software_16_ddraw",
     "none"
};
