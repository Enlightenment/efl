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

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_way_shm_log_dom, __VA_ARGS__)

typedef struct _Outbuf Outbuf;
struct _Outbuf 
{
   int w, h;
   int rotation;

   struct 
     {
        void *dest;

        RGBA_Image *buffer;

        Eina_Bool destination_alpha : 1;
     } priv;
};

void evas_outbuf_free(Outbuf *ob);
void evas_outbuf_resize(Outbuf *ob, int w, int h);
Outbuf *evas_outbuf_setup(int w, int h, int rot, Eina_Bool alpha, void *dest);
RGBA_Image *evas_outbuf_new_region_for_update(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void evas_outbuf_push_updated_region(Outbuf *ob, RGBA_Image *update, int x __UNUSED__, int y, int w, int h);
void evas_outbuf_free_region_for_update(Outbuf *ob, RGBA_Image *update);

#endif
