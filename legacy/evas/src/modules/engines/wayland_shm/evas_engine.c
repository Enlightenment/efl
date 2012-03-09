#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Wayland_Shm.h"

/* local structures */
typedef struct _Render_Engine Render_Engine;
struct _Render_Engine 
{
   Tilebuf *tb;
   Tilebuf_Rect *rects;
   Outbuf *ob;
   Eina_Inlist *cur_rect;

   Eina_Bool end : 1;

   void (*outbuf_free)(Outbuf *ob);
   void (*outbuf_resize)(Outbuf *ob, int w, int h);
   RGBA_Image *(*outbuf_new_region_for_update)(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
   void (*outbuf_push_updated_region)(Outbuf *ob, RGBA_Image *surface, int x, int y, int w, int h);
   void (*outbuf_free_region_for_update)(Outbuf *ob, RGBA_Image *update);
};

/* local variables */
static Evas_Func func, pfunc;

/* external variables */
int _evas_engine_way_shm_log_dom = -1;

/* local function prototypes */
static void *_output_setup(int w, int h, int rotation, Eina_Bool alpha, void *dest);

/* engine function prototypes */
static void *eng_info(Evas *evas __UNUSED__);
static void eng_info_free(Evas *evas __UNUSED__, void *info);
static int eng_setup(Evas *evas, void *info);
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
static Eina_Bool eng_canvas_alpha_get(void *data, void *context __UNUSED__);

/* local functions */
static void *
_output_setup(int w, int h, int rotation, Eina_Bool alpha, void *dest) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = calloc(1, sizeof(Render_Engine)))) return NULL;

   if (!(re->ob = evas_outbuf_setup(w, h, rotation, alpha, dest))) 
     {
        free(re);
        return NULL;
     }

   if (!(re->tb = evas_common_tilebuf_new(w, h))) 
     {
        evas_outbuf_free(re->ob);
        free(re);
        return NULL;
     }

   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   return re;
}

/* engine functions */
static void *
eng_info(Evas *evas __UNUSED__) 
{
   Evas_Engine_Info_Wayland_Shm *info;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(info = calloc(1, sizeof(Evas_Engine_Info_Wayland_Shm))))
     return NULL;

   info->magic.magic = rand();
   info->info.debug = EINA_FALSE;
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;

   return info;
}

static void 
eng_info_free(Evas *evas __UNUSED__, void *info) 
{
   Evas_Engine_Info_Wayland_Shm *in;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(in = (Evas_Engine_Info_Wayland_Shm *)info)) return;
   free(in);
}

static int 
eng_setup(Evas *evas, void *info) 
{
   Evas_Engine_Info_Wayland_Shm *in;
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(in = (Evas_Engine_Info_Wayland_Shm *)info)) return 0;

   if (!evas->engine.data.output) 
     {
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

        re = _output_setup(evas->output.w, evas->output.h, 
                           in->info.rotation, in->info.destination_alpha, 
                           in->info.dest);
        if (!re) return 0;

        re->outbuf_free = evas_outbuf_free;
        re->outbuf_resize = evas_outbuf_resize;
        re->outbuf_new_region_for_update = evas_outbuf_new_region_for_update;
        re->outbuf_push_updated_region = evas_outbuf_push_updated_region;
        re->outbuf_free_region_for_update = evas_outbuf_free_region_for_update;
     }
   else 
     {
        if (!(re = evas->engine.data.output)) return 0;
        if (re->ob) re->outbuf_free(re->ob);
        re->ob = evas_outbuf_setup(evas->output.w, evas->output.h, 
                                   in->info.rotation, 
                                   in->info.destination_alpha, in->info.dest);
        if (re->tb) evas_common_tilebuf_free(re->tb);
        if ((re->tb = evas_common_tilebuf_new(evas->output.w, evas->output.h)))
          evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
     }

   evas->engine.data.output = re;

   if (!evas->engine.data.context) 
     {
        evas->engine.data.context = 
          evas->engine.func->context_new(evas->engine.data.output);
     }

   return 1;
}

static void 
eng_output_free(void *data) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((re = (Render_Engine *)data)) 
     {
        if (re->ob) re->outbuf_free(re->ob);
        if (re->tb) evas_common_tilebuf_free(re->tb);
        if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
        free(re);
     }
   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void 
eng_output_resize(void *data, int w, int h) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;

   if (re->ob) re->outbuf_resize(re->ob, w, h);
   if (re->tb) evas_common_tilebuf_free(re->tb);
   if ((re->tb = evas_common_tilebuf_new(w, h)))
     evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
}

static void 
eng_output_tile_size_set(void *data, int w, int h) 
{
   Render_Engine *re = NULL;

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void 
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h) 
{
   Render_Engine *re = NULL;

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void 
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h) 
{
   Render_Engine *re = NULL;

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void 
eng_output_redraws_clear(void *data) 
{
   Render_Engine *re = NULL;

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_clear(re->tb);
}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch) 
{
   Render_Engine *re = NULL;
   RGBA_Image *surface;
   Tilebuf_Rect *rect;
   int ux = 0, uy = 0, uw = 0, uh = 0;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (re->end) 
     {
        re->end = EINA_FALSE;
        return NULL;
     }
   if (!re->rects) 
     {
        re->rects = evas_common_tilebuf_get_render_rects(re->tb);
        re->cur_rect = EINA_INLIST_GET(re->rects);
     }
   if (!re->cur_rect) return NULL;
   rect = (Tilebuf_Rect *)re->cur_rect;
   ux = rect->x;
   uy = rect->y;
   uw = rect->w;
   uh = rect->h;
   re->cur_rect = re->cur_rect->next;
   if (!re->cur_rect) 
     {
        evas_common_tilebuf_free_render_rects(re->rects);
        re->rects = NULL;
        re->end = EINA_TRUE;
     }
   if ((ux + uw) > re->ob->w) uw = re->ob->w - ux;
   if ((uy + uh) > re->ob->h) uh = re->ob->h - uy;
   if ((uw <= 0) || (uh <= 0)) return NULL;
   surface = 
     re->outbuf_new_region_for_update(re->ob, ux, uy, uw, uh, cx, cy, cw, ch);
   if (x) *x = ux;
   if (y) *y = uy;
   if (w) *w = uw;
   if (h) *h = uh;
   return surface;
}

static void 
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h) 
{
   Render_Engine *re = NULL;

   if (!(re = (Render_Engine *)data)) return;
#ifdef BUILD_PIPE_RENDER
   evas_common_pipe_map_begin(surface);
#endif
   if (re->ob) 
     {
        re->outbuf_push_updated_region(re->ob, surface, x, y, w, h);
        re->outbuf_free_region_for_update(re->ob, surface);
     }
   evas_common_cpu_end_opt();
}

static void 
eng_output_flush(void *data) 
{
   Render_Engine *re = NULL;

   if (!(re = (Render_Engine *)data)) return;
}

static void 
eng_output_idle_flush(void *data) 
{
   Render_Engine *re = NULL;

   if (!(re = (Render_Engine *)data)) return;
}

static Eina_Bool 
eng_canvas_alpha_get(void *data, void *context __UNUSED__) 
{
   Render_Engine *re = NULL;

   if (!(re = (Render_Engine *)data)) return EINA_FALSE;
   return EINA_TRUE;
}

/* module functions */
static int 
module_open(Evas_Module *em) 
{
   if (!em) return 0;

   if (!_evas_module_engine_inherit(&pfunc, "software_generic"))
     return 0;

   _evas_engine_way_shm_log_dom = 
     eina_log_domain_register("evas-wayland_shm", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_way_shm_log_dom < 0) 
     {
        EINA_LOG_ERR("Could not create a module log domain.");
        return 0;
     }

   func = pfunc;

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
   ORD(output_flush);
   ORD(output_idle_flush);

   em->functions = (void *)(&func);
   return 1;
}

static void 
module_close(Evas_Module *em __UNUSED__) 
{
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
