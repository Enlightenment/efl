#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include "evas_fb.h"

typedef struct _Outbuf                Outbuf;

typedef enum   _Outbuf_Depth          Outbuf_Depth;

enum _Outbuf_Depth
{
   OUTBUF_DEPTH_NONE,
     OUTBUF_DEPTH_INHERIT,
     OUTBUF_DEPTH_RGB_16BPP_565_565_DITHERED,
     OUTBUF_DEPTH_RGB_16BPP_555_555_DITHERED,
     OUTBUF_DEPTH_RGB_16BPP_444_444_DITHERED,
     OUTBUF_DEPTH_RGB_16BPP_565_444_DITHERED,
     OUTBUF_DEPTH_RGB_32BPP_888_8888,
     OUTBUF_DEPTH_LAST
};

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
