#ifndef _EVAS_ENGINE_H
# define _EVAS_ENGINE_H

# define LOGFNS 1

# ifdef LOGFNS
#  include <stdio.h>
#  define LOGFN(fl, ln, fn) printf("-EVAS-DRM: %25s: %5i - %s\n", fl, ln, fn);
# else
#  define LOGFN(fl, ln, fn)
# endif

# include "evas_common_private.h"
# include "evas_macros.h"
# include "evas_private.h"
# include "Evas.h"
# include "Evas_Engine_Drm.h"

# include <xf86drm.h>
# include <xf86drmMode.h>
# include <drm_fourcc.h>

# ifdef GL_GLES
#  include <EGL/egl.h>
#  define GL_GLEXT_PROTOTYPES
# endif

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

typedef enum _Outbuf_Depth Outbuf_Depth;
typedef struct _Wl_Swapper Wl_Swapper;
typedef struct _Outbuf Outbuf;

enum _Outbuf_Depth
{
   OUTBUF_DEPTH_NONE,
   OUTBUF_DEPTH_ARGB_32BPP_8888_8888,
   OUTBUF_DEPTH_RGB_32BPP_8888_8888,
   OUTBUF_DEPTH_LAST
};

enum 
{
   MODE_FULL,
   MODE_COPY,
   MODE_DOUBLE,
   MODE_TRIPLE
};

struct _Outbuf
{
   int w, h;
   unsigned int rotation;
   Outbuf_Depth depth;
   int onebuf;

   struct 
     {
        /* struct gbm_device *gbm; */
        /* unsigned int format; */

        /* swapper */
        void *swapper;

        /* one big buffer for updates. flushed on idle_flush */
        RGBA_Image *onebuf;
        Eina_Array onebuf_regions;

        /* a list of pending regions to write out */
        Eina_List *pending_writes;

        /* list of previous frame pending regions to write out */
        Eina_List *prev_pending_writes;

        Eina_Bool destination_alpha : 1;
     } priv;
};

Outbuf *evas_swapbuf_setup(int w, int h, unsigned int rotation, Outbuf_Depth depth, Eina_Bool alpha);
void evas_swapbuf_free(Outbuf *ob);
void evas_swapbuf_reconfigure(Outbuf *ob, int x, int y, int w, int h, unsigned int rotation, Outbuf_Depth depth, Eina_Bool alpha);
RGBA_Image *evas_swapbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void evas_swapbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
void evas_swapbuf_update_region_free(Outbuf *ob, RGBA_Image *update);
void evas_swapbuf_flush(Outbuf *ob);
void evas_swapbuf_idle_flush(Outbuf *ob EINA_UNUSED);
int evas_swapbuf_state_get(Outbuf *ob);

Wl_Swapper *evas_swapper_setup(int dx, int dy, int w, int h, Outbuf_Depth depth, Eina_Bool alpha, int fd);
Wl_Swapper *evas_swapper_reconfigure(Wl_Swapper *ws, int dx, int dy, int w, int h, Outbuf_Depth depth, Eina_Bool alpha);
void evas_swapper_swap(Wl_Swapper *ws, Eina_Rectangle *rects, unsigned int count);
void evas_swapper_free(Wl_Swapper *ws);
void *evas_swapper_buffer_map(Wl_Swapper *ws, int *w, int *h);
void evas_swapper_buffer_unmap(Wl_Swapper *ws);
int evas_swapper_buffer_state_get(Wl_Swapper *ws);
void evas_swapper_buffer_idle_flush(Wl_Swapper *ws);

#endif
