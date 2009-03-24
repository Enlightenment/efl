#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Software_16_WinCE.h"
#include "evas_common_soft16.h"


typedef enum
{
  EVAS_ENGINE_WINCE_FB,
  EVAS_ENGINE_WINCE_GAPI,
  EVAS_ENGINE_WINCE_DDRAW,
  EVAS_ENGINE_WINCE_GDI
} Evas_Engine_WinCE_Backend;


/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Evas_Engine_WinCE_Backend backend; /* 1: raw, 2: gapi, 3: ddraw, 4: GDI */
   void               *backend_priv;
   void              (*backend_shutdown)(void *priv);
   FB_Output_Buffer *(*backend_output_buffer_new)(void *priv,
                                                  int width,
                                                  int height);
   void              (*backend_output_buffer_free)(FB_Output_Buffer *fbob);
   void              (*backend_output_buffer_paste)(FB_Output_Buffer *fbob);
   void              (*backend_surface_resize)(FB_Output_Buffer *fbob);

   int               width;
   int               height;
   int               rotation;
   Tilebuf          *tb;
   Tilebuf_Rect     *rects;
   Tilebuf_Rect     *cur_rect;
   FB_Output_Buffer *fbob;
   Soft16_Image     *tmp_out; /* used by indirect render, like rotation */
   HRGN              clip_rects;
   unsigned char     end : 1;
};

/* prototypes we will use here */

static void *eng_info(Evas *e);
static void eng_info_free(Evas *e, void *info);
static int eng_setup(Evas *e, void *info);
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

static int
_suspend(int backend)
{
   switch (backend)
     {
      case 2: /* gapi */
         return evas_software_wince_gapi_suspend();
      default: /* other engines do not need it */
         return 0;
     }
}

static int
_resume(int backend)
{
   switch (backend)
     {
      case 2: /* gapi */
         return evas_software_wince_gapi_resume();
      default: /* other engines do not need it */
         return 0;
     }
}

/* engine api this module provides */
static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_Software_16_WinCE *info;

   info = calloc(1, sizeof(Evas_Engine_Info_Software_16_WinCE));
   if (!info) return NULL;
   info->magic.magic = rand();
   info->func.suspend = _suspend;
   info->func.resume = _resume;
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_Software_16_WinCE *in;

   in = (Evas_Engine_Info_Software_16_WinCE *)info;
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


static int
eng_setup(Evas *e, void *in)
{
   Render_Engine                      *re;
   Evas_Engine_Info_Software_16_WinCE *info;

   info = (Evas_Engine_Info_Software_16_WinCE *)in;
   if (!e->engine.data.output)
     {
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
          return 0;
	e->engine.data.output = re;

        switch(info->info.backend)
          {
           case 1: /* FB */
              re->backend = EVAS_ENGINE_WINCE_FB;
              re->backend_priv = evas_software_wince_fb_init(info->info.window, info->info.width, info->info.height);
              if (!re->backend_priv)
                {
                   free(re);
                   return 0;
                }
              re->backend_shutdown = evas_software_wince_fb_shutdown;
              re->backend_output_buffer_new = evas_software_wince_fb_output_buffer_new;
              re->backend_output_buffer_free = evas_software_wince_fb_output_buffer_free;
              re->backend_output_buffer_paste = evas_software_wince_fb_output_buffer_paste;
              re->backend_surface_resize = evas_software_wince_fb_surface_resize;
              break;
           case 2: /* GAPI */
              re->backend = EVAS_ENGINE_WINCE_GAPI;
              re->backend_priv = evas_software_wince_gapi_init(info->info.window, info->info.width, info->info.height);
              if (!re->backend_priv)
                {
                   free(re);
                   return 0;
                }
              re->backend_shutdown = evas_software_wince_gapi_shutdown;
              re->backend_output_buffer_new = evas_software_wince_gapi_output_buffer_new;
              re->backend_output_buffer_free = evas_software_wince_gapi_output_buffer_free;
              re->backend_output_buffer_paste = evas_software_wince_gapi_output_buffer_paste;
              re->backend_surface_resize = evas_software_wince_gapi_surface_resize;
              break;
           case 3: /* DirectDraw */
              re->backend = EVAS_ENGINE_WINCE_DDRAW;
              re->backend_priv = evas_software_wince_ddraw_init(info->info.window, info->info.width, info->info.height);
              if (!re->backend_priv)
                {
                   free(re);
                   return 0;
                }
              re->backend_shutdown = evas_software_wince_ddraw_shutdown;
              re->backend_output_buffer_new = evas_software_wince_ddraw_output_buffer_new;
              re->backend_output_buffer_free = evas_software_wince_ddraw_output_buffer_free;
              re->backend_output_buffer_paste = evas_software_wince_ddraw_output_buffer_paste;
              re->backend_surface_resize = evas_software_wince_ddraw_surface_resize;
              break;
           case 4: /* GDI */
              re->backend = EVAS_ENGINE_WINCE_GDI;
              re->backend_priv = evas_software_wince_gdi_init(info->info.window, info->info.width, info->info.height, info->info.fullscreen);
              if (!re->backend_priv)
                {
                   free(re);
                   return 0;
                }
              re->backend_shutdown = evas_software_wince_gdi_shutdown;
              re->backend_output_buffer_new = evas_software_wince_gdi_output_buffer_new;
              re->backend_output_buffer_free = evas_software_wince_gdi_output_buffer_free;
              re->backend_output_buffer_paste = evas_software_wince_gdi_output_buffer_paste;
              re->backend_surface_resize = evas_software_wince_gdi_surface_resize;
              break;
           default:
              free(re);
              return 0;
          }

	re->width = e->output.w;
	re->height = e->output.h;
	re->rotation = info->info.rotation;
	re->tb = evas_common_tilebuf_new(e->output.w, e->output.h);
	if (re->tb)
	  evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
     }
   else
     {
	re = e->engine.data.output;
	if (re->tb) evas_common_tilebuf_free(re->tb);

        switch(info->info.backend)
          {
           case 1: /* FB */
              re->backend = EVAS_ENGINE_WINCE_FB;
              re->backend_priv = evas_software_wince_fb_init(info->info.window, info->info.width, info->info.height);
              if (!re->backend_priv)
                {
                   free(re);
                   return 0;
                }
              re->backend_shutdown = evas_software_wince_fb_shutdown;
              re->backend_output_buffer_new = evas_software_wince_fb_output_buffer_new;
              re->backend_output_buffer_free = evas_software_wince_fb_output_buffer_free;
              re->backend_output_buffer_paste = evas_software_wince_fb_output_buffer_paste;
              re->backend_surface_resize = evas_software_wince_fb_surface_resize;
              break;
           case 2: /* GAPI */
              re->backend = EVAS_ENGINE_WINCE_GAPI;
              re->backend_priv = evas_software_wince_gapi_init(info->info.window, info->info.width, info->info.height);
              if (!re->backend_priv)
                {
                   free(re);
                   return 0;
                }
              re->backend_shutdown = evas_software_wince_gapi_shutdown;
              re->backend_output_buffer_new = evas_software_wince_gapi_output_buffer_new;
              re->backend_output_buffer_free = evas_software_wince_gapi_output_buffer_free;
              re->backend_output_buffer_paste = evas_software_wince_gapi_output_buffer_paste;
              re->backend_surface_resize = evas_software_wince_gapi_surface_resize;
              break;
           case 3: /* DirectDraw */
              re->backend = EVAS_ENGINE_WINCE_DDRAW;
              re->backend_priv = evas_software_wince_ddraw_init(info->info.window, info->info.width, info->info.height);
              if (!re->backend_priv)
                {
                   free(re);
                   return 0;
                }
              re->backend_shutdown = evas_software_wince_ddraw_shutdown;
              re->backend_output_buffer_new = evas_software_wince_ddraw_output_buffer_new;
              re->backend_output_buffer_free = evas_software_wince_ddraw_output_buffer_free;
              re->backend_output_buffer_paste = evas_software_wince_ddraw_output_buffer_paste;
              re->backend_surface_resize = evas_software_wince_ddraw_surface_resize;
              break;
           case 4: /* GDI */
              re->backend = EVAS_ENGINE_WINCE_GDI;
              re->backend_priv = evas_software_wince_gdi_init(info->info.window, info->info.width, info->info.height, info->info.fullscreen);
              if (!re->backend_priv)
                {
                   free(re);
                   return 0;
                }
              re->backend_shutdown = evas_software_wince_gdi_shutdown;
              re->backend_output_buffer_new = evas_software_wince_gdi_output_buffer_new;
              re->backend_output_buffer_free = evas_software_wince_gdi_output_buffer_free;
              re->backend_output_buffer_paste = evas_software_wince_gdi_output_buffer_paste;
              re->backend_surface_resize = evas_software_wince_gdi_surface_resize;
              break;
           default:
              free(re);
              return 0;
          }

	re->width = e->output.w;
	re->height = e->output.h;
	re->rotation = info->info.rotation;
	re->tb = evas_common_tilebuf_new(e->output.w, e->output.h);
	if (re->tb)
	  evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
	if (re->tmp_out)
	  {
             evas_cache_image_drop(&re->tmp_out->cache_entry);
	     re->tmp_out = NULL;
	  }
     }
   if (!e->engine.data.output) return 0;
   /* add a draw context if we dont have one */
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
   if (re->fbob) re->backend_output_buffer_free(re->backend_priv);
   re->backend_shutdown(re->backend_priv);
   if (re->clip_rects) DeleteObject(re->clip_rects);
   if (re->tb) evas_common_tilebuf_free(re->tb);
   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   if (re->tmp_out) evas_cache_image_drop(&re->tmp_out->cache_entry);
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

   /* FIXME: is it needed ?? */
   if (re->fbob)
     re->backend_surface_resize(re->fbob);

   evas_common_tilebuf_free(re->tb);
   re->width = w;
   re->height = h;
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb)
     evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   if (re->fbob)
     {
        re->backend_output_buffer_free(re->fbob);
	re->fbob = NULL;
     }
   if (re->clip_rects)
     {
	DeleteObject(re->clip_rects);
	re->clip_rects = NULL;
     }
   if (re->tmp_out)
     {
	evas_cache_image_drop(&re->tmp_out->cache_entry);
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

   if (re->fbob) return;

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

   re->fbob = re->backend_output_buffer_new(re->backend_priv,
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
	return re->fbob->im;
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

   d = re->fbob->im;
   s = re->tmp_out;

   if ((w < 1) || (h < 1) || (out_x >= d->cache_entry.w) || (out_y >= d->cache_entry.h))
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

   re = (Render_Engine *)data;
   if (re->clip_rects)
     {
        /* FIXME : i have to manage that */
/* 	XSetRegion(re->disp, re->gc, re->clip_rects); */
	DeleteObject(re->clip_rects);
	re->clip_rects = NULL;
     }
   else return;

   re->backend_output_buffer_paste(re->fbob);

   /* FIXME : i have to manage that */
/*    XSetClipMask(re->disp, re->gc, None); */
}

static void
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re->fbob)
     {
	re->backend_output_buffer_free(re->fbob);
	re->fbob = NULL;
     }
   if (re->clip_rects)
     {
	DeleteObject(re->clip_rects);
	re->clip_rects = NULL;
     }
   if (re->tmp_out)
     {
	evas_cache_image_drop(&re->tmp_out->cache_entry);
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
   "software_16_wince_fb",
   "none"
};
