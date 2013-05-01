#ifndef EVAS_ENGINE_H
# define EVAS_ENGINE_H

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

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_drm_log_dom, __VA_ARGS__)

typedef struct _Outbuf Outbuf;

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
   unsigned int rotation, depth;
   Eina_Bool destination_alpha : 1;
};

Outbuf *evas_outbuf_setup(int w, int h, unsigned int rotation, unsigned int depth, Eina_Bool alpha);
void evas_outbuf_free(Outbuf *ob);
void evas_outbuf_reconfigure(Outbuf *ob, int w, int h, unsigned int rotation, unsigned int depth, Eina_Bool alpha);

#endif
