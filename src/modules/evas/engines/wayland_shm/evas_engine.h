#ifndef _EVAS_ENGINE_H
# define _EVAS_ENGINE_H

//# define LOGFNS 1

# ifdef LOGFNS
#  include <stdio.h>
#  define LOGFN(fl, ln, fn) printf("-EVAS-WL: %25s: %5i - %s\n", fl, ln, fn);
# else
#  define LOGFN(fl, ln, fn)
# endif

extern int _evas_engine_way_shm_log_dom;

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_way_shm_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_way_shm_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_evas_engine_way_shm_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_way_shm_log_dom, __VA_ARGS__)

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_way_shm_log_dom, __VA_ARGS__)

# include <wayland-client.h>

#include "Evas_Engine_Wayland_Shm.h"

#include "../software_generic/Evas_Engine_Software_Generic.h"

struct _Outbuf
{
   Evas_Engine_Info_Wayland_Shm *info;

   int w, h;
   int rotation;
   int onebuf;
   Outbuf_Depth depth;

   struct 
     {
        struct 
          {
             /* wayland shared memory object */
             struct wl_shm *shm;
             struct wl_surface *surface;
          } wl;

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

#endif
