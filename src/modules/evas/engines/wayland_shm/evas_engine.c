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

/* engine functions */
static void *
eng_info(Evas *eo_evas EINA_UNUSED)
{
   return NULL;
}

static void 
eng_info_free(Evas *eo_evas EINA_UNUSED, void *einfo)
{

}

static int 
eng_setup(Evas *eo_evas, void *einfo)
{
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
