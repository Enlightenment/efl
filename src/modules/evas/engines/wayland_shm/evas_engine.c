#include "evas_common_private.h"
#include "evas_private.h"
#include "Evas_Engine_Wayland_Shm.h"
#include "evas_engine.h"
#include "evas_swapbuf.h"

/* local structures */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Evas_Engine_Info_Wayland_Shm *info;
   Outbuf *ob;
   Tilebuf *tb;

   Tilebuf_Rect *rects;
   Tilebuf_Rect *prev_rects[3];
   Eina_Inlist *cur_rect;

   short mode;

   Eina_Bool end : 1;
   Eina_Bool lost_back : 1;

   /* function pointers for output buffer functions that we can 
    * override based on if we are swapping or not */
   void (*outbuf_free)(Outbuf *ob);
   void (*outbuf_reconfigure)(Outbuf *ob, int x, int y, int w, int h, unsigned int rotation, Outbuf_Depth depth, Eina_Bool alpha);
   RGBA_Image *(*outbuf_update_region_new)(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
   void (*outbuf_update_region_push)(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
   void (*outbuf_update_region_free)(Outbuf *ob, RGBA_Image *update);
   void (*outbuf_flush)(Outbuf *ob);
   void (*outbuf_idle_flush)(Outbuf *ob);
};

/* local function prototypes */
static void *_output_engine_setup(int w, int h, unsigned int rotation, unsigned int depth, Eina_Bool destination_alpha, struct wl_shm *wl_shm, struct wl_surface *wl_surface, int try_swap);
static Tilebuf_Rect *_merge_rects(Tilebuf *tb, Tilebuf_Rect *r1, Tilebuf_Rect *r2, Tilebuf_Rect *r3);

/* engine function prototypes */
static void *eng_info(Evas *eo_evas EINA_UNUSED);
static void eng_info_free(Evas *eo_evas EINA_UNUSED, void *einfo);
static int eng_setup(Evas *eo_evas, void *einfo);
static void eng_output_free(void *data);
static void eng_output_resize(void *data, int w, int h);
static void eng_output_tile_size_set(void *data, int w, int h);
static void eng_output_redraws_rect_add(void *data, int x, int y, int w, int h);
static void eng_output_redraws_rect_del(void *data, int x, int y, int w, int h);
static void eng_output_redraws_clear(void *data);
static void *eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
static void eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h, Evas_Render_Mode render_mode);
static void eng_output_flush(void *data, Evas_Render_Mode render_mode);
static void eng_output_idle_flush(void *data);

/* local variables */
static Evas_Func func, pfunc;

/* external variables */
int _evas_engine_way_shm_log_dom = -1;

/* local functions */
static void *
_output_engine_setup(int w, int h, unsigned int rotation, unsigned int depth, Eina_Bool destination_alpha, struct wl_shm *wl_shm, struct wl_surface *wl_surface, int try_swap)
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* try to allocate a new render engine */
   if (!(re = calloc(1, sizeof(Render_Engine)))) 
     return NULL;

   /* try to create a new tilebuf first */
   if (!(re->tb = evas_common_tilebuf_new(w, h)))
     {
        free(re);
        return NULL;
     }

   /* set tile size for the tile buffer */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   if (try_swap)
     {
        if ((re->ob = evas_swapbuf_setup(w, h, rotation, depth,
                                         destination_alpha, wl_shm,
                                         wl_surface)))
          {
             re->outbuf_free = evas_swapbuf_free;
             re->outbuf_reconfigure = evas_swapbuf_reconfigure;
             re->outbuf_update_region_new = evas_swapbuf_update_region_new;
             re->outbuf_update_region_push = evas_swapbuf_update_region_push;
             re->outbuf_update_region_free = evas_swapbuf_update_region_free;
             re->outbuf_flush = evas_swapbuf_flush;
             re->outbuf_idle_flush = evas_swapbuf_idle_flush;
          }
     }

   /* if creating an output buffer failed, then return NULL */
   if (!re->ob)
     {
        if (re->tb) evas_common_tilebuf_free(re->tb);
        free(re);
        return NULL;
     }

   /* return allocated render engine */
   return re;
}

static Tilebuf_Rect *
_merge_rects(Tilebuf *tb, Tilebuf_Rect *r1, Tilebuf_Rect *r2, Tilebuf_Rect *r3)
{
   Tilebuf_Rect *r, *rects;
   
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (r1)
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(r1), r)
          evas_common_tilebuf_add_redraw(tb, r->x, r->y, r->w, r->h);
     }
   if (r2)
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(r2), r)
          evas_common_tilebuf_add_redraw(tb, r->x, r->y, r->w, r->h);
     }
   if (r3)
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(r3), r)
          evas_common_tilebuf_add_redraw(tb, r->x, r->y, r->w, r->h);
     }
   rects = evas_common_tilebuf_get_render_rects(tb);

/*   
   // bounding box -> make a bounding box single region update of all regions.
   // yes we could try and be smart and figure out size of regions, how far
   // apart etc. etc. to try and figure out an optimal "set". this is a tradeoff
   // between multiple update regions to render and total pixels to render.
   if (rects)
     {
        px1 = rects->x; py1 = rects->y;
        px2 = rects->x + rects->w; py2 = rects->y + rects->h;
        EINA_INLIST_FOREACH(EINA_INLIST_GET(rects), r)
          {
             if (r->x < x1) px1 = r->x;
             if (r->y < y1) py1 = r->y;
             if ((r->x + r->w) > x2) px2 = r->x + r->w;
             if ((r->y + r->h) > y2) py2 = r->y + r->h;
          }
        evas_common_tilebuf_free_render_rects(rects);
        rects = calloc(1, sizeof(Tilebuf_Rect));
        if (rects)
          {
             rects->x = px1;
             rects->y = py1;
             rects->w = px2 - px1;
             rects->h = py2 - py1;
          }
     }
 */
   evas_common_tilebuf_clear(tb);
   return rects;
}

/* engine functions */
static void *
eng_info(Evas *eo_evas EINA_UNUSED)
{
   Evas_Engine_Info_Wayland_Shm *info;

   /* try to allocate space for engine info */
   if (!(info = calloc(1, sizeof(Evas_Engine_Info_Wayland_Shm))))
     return NULL;

   /* fill in default engine info fields */
   info->magic.magic = rand();
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;

   /* return allocated engine info */
   return info;
}

static void 
eng_info_free(Evas *eo_evas EINA_UNUSED, void *einfo)
{
   Evas_Engine_Info_Wayland_Shm *info;

   /* try to free previously allocated engine info */
   if ((info = (Evas_Engine_Info_Wayland_Shm *)einfo))
     free(info);
}

static int 
eng_setup(Evas *eo_evas, void *einfo)
{
   Evas_Engine_Info_Wayland_Shm *info;
   Evas_Public_Data *epd;
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* try to cast the engine info to our engine info */
   if (!(info = (Evas_Engine_Info_Wayland_Shm *)einfo))
     return 0;

   /* try to get evas public data from the canvas */
   if (!(epd = eo_data_scope_get(eo_evas, EVAS_CANVAS_CLASS)))
     return 0;

   /* test for valid engine output */
   if (!(re = epd->engine.data.output))
     {
        static int try_swap = -1;

        /* NB: If we have no valid output then assume we have not been 
         * initialized yet and call any needed common init routines */
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

        if (try_swap == -1)
          {
             /* check for env var to see if we should try swapping */
             if (getenv("EVAS_NO_WAYLAND_SWAPBUF")) try_swap = 0;
             else try_swap = 1;
          }

        if (!(re = 
              _output_engine_setup(epd->output.w, epd->output.h,
                                   info->info.rotation, info->info.depth,
                                   info->info.destination_alpha,
                                   info->info.wl_shm, info->info.wl_surface,
                                   try_swap)))
          return 0;
        re->info = info;
     }
   else
     {
        int ponebuf = 0;

        if ((re) && (re->ob)) ponebuf = re->ob->onebuf;

        /* free any existing tile buffer */
        if (re->tb) evas_common_tilebuf_free(re->tb);

        /* we have an existing output buffer, free it */
        if (re->ob) re->outbuf_free(re->ob);

        /* create new tile buffer */
        if ((re->tb = evas_common_tilebuf_new(epd->output.w, epd->output.h)))
          evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

        if ((re->ob = evas_swapbuf_setup(epd->output.w, epd->output.h,
                                         info->info.rotation,
                                         info->info.depth,
                                         info->info.destination_alpha,
                                         info->info.wl_shm,
                                         info->info.wl_surface)))
          {
             re->outbuf_free = evas_swapbuf_free;
             re->outbuf_reconfigure = evas_swapbuf_reconfigure;
             re->outbuf_update_region_new = evas_swapbuf_update_region_new;
             re->outbuf_update_region_push = evas_swapbuf_update_region_push;
             re->outbuf_update_region_free = evas_swapbuf_update_region_free;
             re->outbuf_flush = evas_swapbuf_flush;
             re->outbuf_idle_flush = evas_swapbuf_idle_flush;
          }

        re->info = info;
        if ((re) && (re->ob)) re->ob->onebuf = ponebuf;
     }

   /* reassign render engine to output */
   epd->engine.data.output = re;
   if (!epd->engine.data.output) return 0;

   if (!epd->engine.data.context)
     {
        epd->engine.data.context = 
          epd->engine.func->context_new(epd->engine.data.output);
     }

   /* return success */
   return 1;
}

static void 
eng_output_free(void *data)
{
   Render_Engine *re;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((re = data))
     {
        re->outbuf_free(re->ob);
        evas_common_tilebuf_free(re->tb);
        if (re->rects) 
          evas_common_tilebuf_free_render_rects(re->rects);
        if (re->prev_rects[0]) 
          evas_common_tilebuf_free_render_rects(re->prev_rects[0]);
        if (re->prev_rects[1]) 
          evas_common_tilebuf_free_render_rects(re->prev_rects[1]);
        if (re->prev_rects[2]) 
          evas_common_tilebuf_free_render_rects(re->prev_rects[2]);
        free(re);
     }

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void 
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;
   Evas_Engine_Info_Wayland_Shm *info;
   int dx = 0, dy = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;

   if (!(info = re->info)) return;

   if (info->info.edges & 4)
     {
        if ((info->info.rotation == 90) || (info->info.rotation == 270))
          dx = re->ob->h - h;
        else
          dx = re->ob->w - w;
     }

   if (info->info.edges & 1)
     {
        if ((info->info.rotation == 90) || (info->info.rotation == 270))
          dy = re->ob->w - w;
        else
          dy = re->ob->h - h;
     }

   re->outbuf_reconfigure(re->ob, dx, dy, w, h,
                          info->info.rotation, info->info.depth,
                          info->info.destination_alpha);

   evas_common_tilebuf_free(re->tb);
   if ((re->tb = evas_common_tilebuf_new(w, h)))
     evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
}

static void 
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void 
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void 
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void 
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_clear(re->tb);
}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;
   RGBA_Image *surface;
   Tilebuf_Rect *rect;
   Eina_Bool first_rect = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#define CLEAR_PREV_RECTS(x) \
   do { \
      if (re->prev_rects[x]) \
        evas_common_tilebuf_free_render_rects(re->prev_rects[x]); \
      re->prev_rects[x] = NULL; \
   } while (0)
   
   re = (Render_Engine *)data;
   if (re->end)
     {
	re->end = 0;
	return NULL;
     }
   
   if (!re->rects)
     {
        re->mode = evas_swapbuf_state_get(re->ob);
	re->rects = evas_common_tilebuf_get_render_rects(re->tb);
        if (re->rects)
          {
             if ((re->lost_back) || (re->mode == MODE_FULL))
               {
                  /* if we lost our backbuffer since the last frame redraw all */
                  re->lost_back = 0;
                  evas_common_tilebuf_add_redraw(re->tb, 0, 0, re->ob->w, re->ob->h);
                  evas_common_tilebuf_free_render_rects(re->rects);
                  re->rects = evas_common_tilebuf_get_render_rects(re->tb);
               }
             /* ensure we get rid of previous rect lists we dont need if mode
              * changed/is appropriate */
             evas_common_tilebuf_clear(re->tb);
             CLEAR_PREV_RECTS(2);
             re->prev_rects[2] = re->prev_rects[1];
             re->prev_rects[1] = re->prev_rects[0];
             re->prev_rects[0] = re->rects;
             re->rects = NULL;
             switch (re->mode)
               {
                case MODE_FULL:
                case MODE_COPY: // no prev rects needed
                  re->rects = 
                    _merge_rects(re->tb, re->prev_rects[0], NULL, NULL);
                  break;
                case MODE_DOUBLE: // double mode - only 1 level of prev rect
                  re->rects = 
                    _merge_rects(re->tb, re->prev_rects[0], re->prev_rects[1], NULL);
                  break;
                case MODE_TRIPLE: // keep all
                  re->rects = 
                    _merge_rects(re->tb, re->prev_rects[0], re->prev_rects[1], re->prev_rects[2]);
                  break;
                default:
                  break;
               }
             first_rect = EINA_TRUE;
          }
        evas_common_tilebuf_clear(re->tb);
        re->cur_rect = EINA_INLIST_GET(re->rects);
     }
   if (!re->cur_rect) return NULL;
   rect = (Tilebuf_Rect *)re->cur_rect;
   if (re->rects)
     {
        switch (re->mode)
          {
           case MODE_COPY:
           case MODE_DOUBLE:
           case MODE_TRIPLE:
             rect = (Tilebuf_Rect *)re->cur_rect;
             *x = rect->x;
             *y = rect->y;
             *w = rect->w;
             *h = rect->h;
             *cx = rect->x;
             *cy = rect->y;
             *cw = rect->w;
             *ch = rect->h;
             re->cur_rect = re->cur_rect->next;
             break;
           case MODE_FULL:
             re->cur_rect = NULL;
             if (x) *x = 0;
             if (y) *y = 0;
             if (w) *w = re->ob->w;
             if (h) *h = re->ob->h;
             if (cx) *cx = 0;
             if (cy) *cy = 0;
             if (cw) *cw = re->ob->w;
             if (ch) *ch = re->ob->h;
             break;
           default:
             break;
          }
        if (first_rect)
          {
             // do anything needed for the first frame
          }
        surface = 
          re->outbuf_update_region_new(re->ob,
                                       *x, *y, *w, *h,
                                       cx, cy, cw, ch);
        if (!re->cur_rect) re->end = EINA_TRUE;
        return surface;
     }

   return NULL;
}

static void 
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h, Evas_Render_Mode render_mode)
{
   Render_Engine *re;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   if (!(re = (Render_Engine *)data)) return;
#if defined(BUILD_PIPE_RENDER)
   evas_common_pipe_map_begin(surface);
#endif
   re->outbuf_update_region_push(re->ob, surface, x, y, w, h);
   re->outbuf_update_region_free(re->ob, surface);
   evas_common_cpu_end_opt();
}

static void 
eng_output_flush(void *data, Evas_Render_Mode render_mode)
{
   Render_Engine *re;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   if (!(re = (Render_Engine *)data)) return;
   re->outbuf_flush(re->ob);
   if (re->rects)
     {
        evas_common_tilebuf_free_render_rects(re->rects);
        re->rects = NULL;
     }
}

static void 
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;
   re->outbuf_idle_flush(re->ob);
}

/* module functions */
static int 
module_open(Evas_Module *em)
{
   /* check for valid evas module */
   if (!em) return 0;

   /* try to create our logging domain */
   _evas_engine_way_shm_log_dom = 
     eina_log_domain_register("evas-wayland_shm", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_way_shm_log_dom < 0)
     {
        /* creating the logging domain failed. notify user */
        EINA_LOG_ERR("Could not create a module log domain.");

        /* return failure */
        return 0;
     }

   /* try to inherit base functions from the software generic engine */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic"))
     return 0;

   /* copy base functions from the software_generic engine */
   func = pfunc;

   /* override any engine specific functions that we provide */
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

   /* advertise out our own api */
   em->functions = (void *)(&func);

   /* return success */
   return 1;
}

static void 
module_close(Evas_Module *em EINA_UNUSED)
{
   /* if we have the log domain, unregister it */
   if (_evas_engine_way_shm_log_dom > -1)
     eina_log_domain_unregister(_evas_engine_way_shm_log_dom);
}

static Evas_Module_Api evas_modapi = 
{
   EVAS_MODULE_API_VERSION, "wayland_shm", "none", {module_open, module_close}
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, wayland_shm);

#ifndef EVAS_STATIC_BUILD_WAYLAND_SHM
EVAS_EINA_MODULE_DEFINE(engine, wayland_shm);
#endif
