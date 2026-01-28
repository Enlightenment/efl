#ifndef EVAS_ENGINE_H
# define EVAS_ENGINE_H

# include "evas_common_private.h"
# include "evas_macros.h"
# include "evas_private.h"
# include "Evas.h"
# include "Evas_Engine_Drm.h"
# include <Ecore.h>
# include <Ecore_Drm2.h>
# include <drm_fourcc.h>

# include "../software_generic/Evas_Engine_Software_Generic.h"

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

   RGBA_Image *onebuf;
   Eina_Array onebuf_regions;

   Eina_List *update_regions;

   Eina_Bool alpha : 1;
};

Outbuf *_outbuf_setup(Evas_Engine_Info_Drm *info, int w, int h);
void _outbuf_free(Outbuf *ob);
Render_Output_Swap_Mode _outbuf_swap_mode_get(Outbuf *ob);
int _outbuf_rotation_get(Outbuf *ob);
void _outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth);
void _outbuf_damage_region_set(Outbuf *ob, Tilebuf_Rect *damage);
void *_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void _outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
void _outbuf_idle_flush(Outbuf *ob);
void _outbuf_flush(Outbuf *ob, Tilebuf_Rect *surface_damage, Tilebuf_Rect *buffer_damage, Evas_Render_Mode render_mode);

#endif
