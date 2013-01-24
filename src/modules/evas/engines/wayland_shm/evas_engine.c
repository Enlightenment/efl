#include "evas_common.h"
#include "evas_private.h"
#include "Evas_Engine_Wayland_Shm.h"
#include "evas_engine.h"
#include "evas_swapbuf.h"
#include "evas_outbuf.h"

/* local structures */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
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
   void (*outbuf_reconfigure)(Outbuf *ob, int w, int h, unsigned int rotation, Outbuf_Depth depth, Eina_Bool alpha);
   RGBA_Image *(*outbuf_update_region_new)(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
   void (*outbuf_update_region_push)(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
   void (*outbuf_update_region_free)(Outbuf *ob, RGBA_Image *update);
   void (*outbuf_flush)(Outbuf *ob);
   void (*outbuf_idle_flush)(Outbuf *ob);
};

/* local function prototypes */
static void *_output_engine_setup(int w, int h, unsigned int rotation, unsigned int depth, Eina_Bool destination_alpha, struct wl_shm *wl_shm, struct wl_surface *wl_surface, int try_swap);

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

   /* try to allocate a new render engine */
   if (!(re = calloc(1, sizeof(Render_Engine)))) 
     return NULL;

   /* return allocated render engine */
   return re;
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

   /* try to cast the engine info to our engine info */
   if (!(info = (Evas_Engine_Info_Wayland_Shm *)einfo))
     return 0;

   /* try to get evas public data from the canvas */
   if (!(epd = eo_data_get(eo_evas, EVAS_CLASS)))
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
             if (getenv("EVAS_NO_DRI_SWAPBUF")) try_swap = 0;
             else try_swap = 1;
          }

     }

   return 0;
}

static void 
eng_output_free(void *data)
{

}

static void 
eng_output_resize(void *data, int w, int h)
{

}

static void 
eng_output_tile_size_set(void *data, int w, int h)
{

}

static void 
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{

}

static void 
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{

}

static void 
eng_output_redraws_clear(void *data)
{

}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   return NULL;
}

static void 
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h, Evas_Render_Mode render_mode)
{

}

static void 
eng_output_flush(void *data, Evas_Render_Mode render_mode)
{

}

static void 
eng_output_idle_flush(void *data)
{

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
