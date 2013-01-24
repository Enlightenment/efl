#ifndef _EVAS_ENGINE_H
# define _EVAS_ENGINE_H

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

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_way_shm_log_dom, __VA_ARGS__)

# include <wayland-client.h>

typedef enum _Outbuf_Depth Outbuf_Depth;

enum _Outbuf_Depth
{
   OUTBUF_DEPTH_NONE,
     OUTBUF_DEPTH_ARGB_32BPP_8888_8888,
     OUTBUF_DEPTH_RGB_32BPP_8888_8888,
     OUTBUF_DEPTH_LAST
};
