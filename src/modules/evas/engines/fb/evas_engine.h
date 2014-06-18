#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include "evas_fb.h"

#include "../software_generic/Evas_Engine_Software_Generic.h"

extern int _evas_engine_fb_log_dom;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_fb_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_fb_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_fb_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_fb_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_fb_log_dom, __VA_ARGS__)

struct _Outbuf
{
   Outbuf_Depth    depth;
   int             w, h;
   int             rot;

   struct {
      struct {
	 FB_Mode  *fb;
      } fb;
      struct {
	 DATA32    r, g, b;
      } mask;
      RGBA_Image  *back_buf;
   } priv;
};

/****/

void         evas_fb_outbuf_fb_init                   (void);
void         evas_fb_outbuf_fb_free                   (Outbuf *buf);

Outbuf      *evas_fb_outbuf_fb_setup_fb               (int w, int h, int rot, Outbuf_Depth depth, int vt_no, int dev_no, int refresh);

void         evas_fb_outbuf_fb_blit                   (Outbuf *buf, int src_x, int src_y, int w, int h, int dst_x, int dst_y);
void         evas_fb_outbuf_fb_update                 (Outbuf *buf, int x, int y, int w, int h);
RGBA_Image  *evas_fb_outbuf_fb_new_region_for_update  (Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void         evas_fb_outbuf_fb_free_region_for_update (Outbuf *buf, RGBA_Image *update);
void         evas_fb_outbuf_fb_push_updated_region    (Outbuf *buf, RGBA_Image *update, int x, int y, int w, int h);
void         evas_fb_outbuf_fb_reconfigure            (Outbuf *buf, int w, int h, int rot, Outbuf_Depth depth);
int          evas_fb_outbuf_fb_get_width              (Outbuf *buf);
int          evas_fb_outbuf_fb_get_height             (Outbuf *buf);
Outbuf_Depth evas_fb_outbuf_fb_get_depth              (Outbuf *buf);
int          evas_fb_outbuf_fb_get_rot                (Outbuf *buf);
int          evas_fb_outbuf_fb_get_have_backbuf       (Outbuf *buf);
void         evas_fb_outbuf_fb_set_have_backbuf       (Outbuf *buf, int have_backbuf);

#endif
