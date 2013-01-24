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
