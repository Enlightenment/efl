#include "evas_engine.h"

/* local structures */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Evas_Engine_Info_Drm *info;

   Tilebuf *tb;
   Tilebuf_Rect *rects;
   Tilebuf_Rect *prev_rects[3];

   Outbuf *ob;

   short mode;

   Eina_Inlist *cur_rect;

   Eina_Bool end : 1;
   Eina_Bool lost_back : 1;
};

/* local function prototypes */
static void *_output_setup(Evas_Engine_Info_Drm *info, int w, int h, int swap);

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* external variables */
int _evas_engine_drm_log_dom;

/* local functions */
static void *
_output_setup(Evas_Engine_Info_Drm *info, int w, int h, int swap)
{
   Render_Engine *re;

   /* try to allocate space for our render engine structure */
   if (!(re = calloc(1, sizeof(Render_Engine))))
     return NULL;

   /* try to create a new tilebuffer */
   if (!(re->tb = evas_common_tilebuf_new(w, h)))
     {
        free(re);
        return NULL;
     }

   /* set tilesize */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   /* if we have no drm device, get one */
   if (info->info.fd < 0)
     {
        /* try to init drm (this includes openening the card & tty) */
        if (!evas_drm_init(info, 0))
          {
             if (re->tb) evas_common_tilebuf_free(re->tb);
             free(re);
             return NULL;
          }
     }

   if (swap)
     {
        /* free any existing outbuf */
        if (re->ob) evas_outbuf_free(re->ob);

        /* try to create new outbuf */
        if (!(re->ob = evas_outbuf_setup(info, w, h)))
          {
             if (re->tb) evas_common_tilebuf_free(re->tb);

             /* shutdown drm card & tty */
             evas_drm_shutdown(info);

             free(re);
             return NULL;
          }
     }

   /* return the allocated render_engine structure */
   return re;
}

/* engine api functions */
static void *
eng_info(Evas *evas EINA_UNUSED)
{
   Evas_Engine_Info_Drm *info;

   /* try to allocate space for our engine info structure */
   if (!(info = calloc(1, sizeof(Evas_Engine_Info_Drm))))
     return NULL;

   /* set some engine default properties */
   info->magic.magic = rand();
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;

   return info;
}

static void 
eng_info_free(Evas *evas EINA_UNUSED, void *einfo)
{
   Evas_Engine_Info_Drm *info;

   /* free the engine info */
   if ((info = (Evas_Engine_Info_Drm *)einfo))
     free(info);
}

static int 
eng_setup(Evas *evas, void *einfo)
{
   Evas_Engine_Info_Drm *info;
   Evas_Public_Data *epd;
   Render_Engine *re;

   /* try to cast to our engine info structure */
   if (!(info = (Evas_Engine_Info_Drm *)einfo)) return 0;

   /* try to get the evas public data */
   if (!(epd = eo_data_scope_get(evas, EVAS_CLASS))) return 0;

   /* set canvas reference
    * 
    * NB: We do this here so that on a vt switch, we can disable 
    * rendering (or re-enable) for this canvas */
   info->info.evas = evas;

   /* check for valid engine output */
   if (!(re = epd->engine.data.output))
     {
        static int swap = -1;

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

        /* check if swapping is disabled */
        if (swap == -1)
          {
             if (getenv("EVAS_DRM_NO_SWAP")) swap = 0;
             else swap = 1;
          }

        /* try to create a new render_engine */
        if (!(re = _output_setup(info, epd->output.w, epd->output.h, swap)))
          return 0;
     }
   else
     {
        /* if we have an existing outbuf, free it */
        if (re->ob) evas_outbuf_free(re->ob);

        /* try to create a new outbuf */
        if (!(re->ob = evas_outbuf_setup(info, epd->output.w, epd->output.h)))
          return 0;
     }

   /* update the info structure pointer */
   re->info = info;

   /* reassign engine output */
   epd->engine.data.output = re;
   if (!epd->engine.data.output) return 0;

   /* check for valid engine context */
   if (!epd->engine.data.context)
     {
        /* create a context if needed */
        epd->engine.data.context = 
          epd->engine.func->context_new(epd->engine.data.output);
     }

   return 1;
}

static void 
eng_output_free(void *data)
{
   Render_Engine *re;

   if ((re = data))
     {
        if (re->ob) evas_outbuf_free(re->ob);
        if (re->tb) evas_common_tilebuf_free(re->tb);
        if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
        if (re->prev_rects[0])
          evas_common_tilebuf_free_render_rects(re->prev_rects[0]);
        if (re->prev_rects[1])
          evas_common_tilebuf_free_render_rects(re->prev_rects[1]);
        if (re->prev_rects[2])
          evas_common_tilebuf_free_render_rects(re->prev_rects[2]);

        evas_drm_shutdown(re->info);

        free(re);
     }

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void 
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   /* try to get the render_engine */
   if (!(re = (Render_Engine *)data)) return;

   evas_outbuf_reconfigure(re->info, re->ob, w, h);

   if (re->tb) evas_common_tilebuf_free(re->tb);
   if ((re->tb = evas_common_tilebuf_new(w, h)))
     evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
}

static void 
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   /* try to get the render_engine */
   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void 
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   /* try to get the render_engine */
   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void 
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   /* try to get the render_engine */
   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void 
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   /* try to get the render_engine */
   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_clear(re->tb);
}

static Tilebuf_Rect *
_merge_rects(Tilebuf *tb, Tilebuf_Rect *r1, Tilebuf_Rect *r2, Tilebuf_Rect *r3)
{
   Tilebuf_Rect *r, *rects;
   
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

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;
   RGBA_Image *img;
   Tilebuf_Rect *rect;

   /* try to get the render_engine */
   if (!(re = (Render_Engine *)data)) return NULL;

#define CLEAR_PREV_RECTS(x) \
   do { \
      if (re->prev_rects[x]) \
        evas_common_tilebuf_free_render_rects(re->prev_rects[x]); \
      re->prev_rects[x] = NULL; \
   } while (0)

   if (re->end)
     {
        re->end = EINA_FALSE;
        return NULL;
     }

   if (!re->rects)
     {
        re->mode = evas_outbuf_buffer_state_get(re->ob);
        if ((re->rects = evas_common_tilebuf_get_render_rects(re->tb)))
          {
             if ((re->lost_back) || (re->mode == MODE_FULL))
               {
                  re->lost_back = EINA_FALSE;
                  evas_common_tilebuf_add_redraw(re->tb, 
                                                 0, 0, re->ob->w, re->ob->h);
                  evas_common_tilebuf_free_render_rects(re->rects);
                  re->rects = evas_common_tilebuf_get_render_rects(re->tb);
               }

             evas_common_tilebuf_clear(re->tb);
             CLEAR_PREV_RECTS(2);
             re->prev_rects[2] = re->prev_rects[1];
             re->prev_rects[1] = re->prev_rects[0];
             re->prev_rects[0] = re->rects;
             re->rects = NULL;

             switch (re->mode)
               {
                case MODE_FULL:
                case MODE_COPY:
                  re->rects = 
                    _merge_rects(re->tb, re->prev_rects[0], NULL, NULL);
                  break;
                case MODE_DOUBLE:
                  re->rects = 
                    _merge_rects(re->tb, re->prev_rects[0], re->prev_rects[1], NULL);
                  break;
                case MODE_TRIPLE:
                  re->rects = 
                    _merge_rects(re->tb, re->prev_rects[0], re->prev_rects[1], re->prev_rects[2]);
                  break;
                default:
                  break;
               }
          }

        /* NB: Not sure this is entirely needed here as it's already done 
         * inside _merge_rects */
        /* evas_common_tilebuf_clear(re->tb); */
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
        img = 
          evas_outbuf_update_region_new(re->ob, *x, *y, *w, *h, 
                                        cx, cy, cw, ch);
        if (!re->cur_rect) 
          {
             evas_common_tilebuf_free_render_rects(re->rects);
             re->rects = NULL;
             re->end = EINA_TRUE;
          }

        return img;
     }

   return NULL;
}

static void 
eng_output_redraws_next_update_push(void *data, void *img, int x, int y, int w, int h, Evas_Render_Mode render_mode)
{
   Render_Engine *re;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   if (!(re = (Render_Engine *)data)) return;
#if defined(BUILD_PIPE_RENDER)
   evas_common_pipe_map_begin(img);
#endif
   evas_outbuf_update_region_push(re->ob, img, x, y, w, h);

   /* NB: No reason to free region here. That is done on flush anyway */
   /* re->outbuf_update_region_free(re->ob, img); */

   evas_common_cpu_end_opt();
}

static void 
eng_output_flush(void *data, Evas_Render_Mode render_mode)
{
   Render_Engine *re;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   if (!(re = (Render_Engine *)data)) return;
   evas_outbuf_flush(re->ob);
   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   re->rects = NULL;
}

/* module api functions */
static int
module_open(Evas_Module *em)
{
   /* check for valid evas module */
   if (!em) return 0;

   /* try to inherit functions from software_generic engine */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;

   /* try to create eina logging domain */
   _evas_engine_drm_log_dom = 
     eina_log_domain_register("evas-drm", EVAS_DEFAULT_LOG_COLOR);

   /* if we could not create a logging domain, error out */
   if (_evas_engine_drm_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   /* store parent functions */
   func = pfunc;

   /* override the methods we provide */
   EVAS_API_OVERRIDE(info, &func, eng_);
   EVAS_API_OVERRIDE(info_free, &func, eng_);
   EVAS_API_OVERRIDE(setup, &func, eng_);
   EVAS_API_OVERRIDE(output_free, &func, eng_);
   EVAS_API_OVERRIDE(output_resize, &func, eng_);
   EVAS_API_OVERRIDE(output_tile_size_set, &func, eng_);
   EVAS_API_OVERRIDE(output_redraws_rect_add, &func, eng_);
   EVAS_API_OVERRIDE(output_redraws_rect_del, &func, eng_);
   EVAS_API_OVERRIDE(output_redraws_clear, &func, eng_);
   EVAS_API_OVERRIDE(output_redraws_next_update_get, &func, eng_);
   EVAS_API_OVERRIDE(output_redraws_next_update_push, &func, eng_);
   EVAS_API_OVERRIDE(output_flush, &func, eng_);
   /* EVAS_API_OVERRIDE(output_idle_flush, &func, eng_); */

   /* advertise our engine functions */
   em->functions = (void *)(&func);

   return 1;
}

static void 
module_close(Evas_Module *em EINA_UNUSED)
{
   /* unregister the eina log domain for this engine */
   eina_log_domain_unregister(_evas_engine_drm_log_dom);
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION, "drm", "none", { module_open, module_close }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, drm);

#ifndef EVAS_STATIC_BUILD_DRM
EVAS_EINA_MODULE_DEFINE(engine, drm);
#endif
