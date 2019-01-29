#ifndef EVAS_ENGINE_H
# define EVAS_ENGINE_H

# include "evas_common_private.h"
# include "evas_macros.h"
# include "evas_private.h"
# include "Evas_Engine_Drm.h"
# include "../software_generic/Evas_Engine_Software_Generic.h"

# include <Ecore.h>

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
   int w, h, bpp, rotation;
   unsigned int depth, format;

   Evas_Engine_Info_Drm *info;

   Eina_Bool alpha : 1;
};

Outbuf *_outbuf_setup(Evas_Engine_Info_Drm *einfo, int w, int h);
void _outbuf_free(Outbuf *ob);
int _outbuf_get_rotation(Outbuf *ob);
void _outbuf_reconfigure(Outbuf *ob, int w, int h, int rotation, Outbuf_Depth depth);
void *_outbuf_new_region_for_update(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);

#endif
