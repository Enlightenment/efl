#ifndef EVAS_ENGINE_H
# define EVAS_ENGINE_H

# include "evas_common_private.h"
# include "evas_macros.h"
# include "evas_private.h"
# include "Evas.h"
# include "Evas_Engine_Drm.h"

#include "../software_generic/Evas_Engine_Software_Generic.h"

extern int _evas_engine_drm_log_dom;

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_drm_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_drm_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_evas_engine_drm_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_drm_log_dom, __VA_ARGS__)

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_drm_log_dom, __VA_ARGS__)

struct _Outbuf
{
   Evas_Engine_Info_Drm *info;

   int w, h;
   int rotation;
   unsigned int depth;

   struct
     {
        Ecore_Drm_Fb *buffer[4];

        Eina_List *pending_writes;

        unsigned int crtc_id, conn_id, buffer_id;

        int curr, last, num;
     } priv;

   Eina_Bool destination_alpha : 1;
   Eina_Bool vsync : 1;
};

Outbuf *evas_outbuf_setup(Evas_Engine_Info_Drm *info, int w, int h);
void evas_outbuf_free(Outbuf *ob);
void evas_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth);
Render_Engine_Swap_Mode evas_outbuf_buffer_state_get(Outbuf *ob);
int evas_outbuf_rot_get(Outbuf *ob);
void *evas_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void evas_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
void evas_outbuf_update_region_free(Outbuf *ob, RGBA_Image *update);
void evas_outbuf_flush(Outbuf *ob, Tilebuf_Rect *rects, Evas_Render_Mode render_mode);

#endif
