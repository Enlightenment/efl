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
# include <xf86drm.h>
# include <xf86drmMode.h>

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

typedef struct _Outbuf_Fb
{
   int age;
   Ecore_Drm2_Fb *fb;

   Eina_Bool valid : 1;
   Eina_Bool drawn : 1;
} Outbuf_Fb;

struct _Outbuf
{
   Ecore_Drm2_Device *dev;
   int w, h, bpp, rotation;
   unsigned int depth, format;

   Evas_Engine_Info_Drm *info;

   struct
     {
        Eina_List *fb_list;
        Outbuf_Fb *draw;
        Ecore_Drm2_Output *output;
        Ecore_Drm2_Plane *plane;
        Eina_List *pending;
        Eina_Rectangle *rects;
        unsigned int rect_count;
        int unused_duration;
     } priv;

   Eina_Bool alpha : 1;
   Eina_Bool vsync : 1;
};

Outbuf *_outbuf_setup(Evas_Engine_Info_Drm *info, int w, int h);
void _outbuf_free(Outbuf *ob);
int _outbuf_rotation_get(Outbuf *ob);
void _outbuf_reconfigure(Outbuf *ob, int w, int h, int rotation, Outbuf_Depth depth);
Render_Output_Swap_Mode _outbuf_state_get(Outbuf *ob);
void *_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void _outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
void _outbuf_flush(Outbuf *ob, Tilebuf_Rect *surface_damage, Tilebuf_Rect *buffer_damage, Evas_Render_Mode render_mode);
void _outbuf_damage_region_set(Outbuf *ob, Tilebuf_Rect *damage);

#endif
