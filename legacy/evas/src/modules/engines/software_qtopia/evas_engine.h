#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

typedef struct _Outbuf                Outbuf;
typedef struct _QT_Direct             QT_Direct;
typedef struct _QT_Direct_Rect        QT_Direct_Rect;

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

typedef enum   _Outbuf_Depth          Outbuf_Depth;


struct _Outbuf
{
   Outbuf_Depth    depth;
   int             w, h;
   int             rot;

   struct {
      void *target;

      struct {
	 DATA32    r, g, b;
      } mask;
      RGBA_Image  *back_buf;
   } priv;
};

struct _QT_Direct_Rect
{
   int x, y, w, h;
};

struct _QT_Direct
{
   /* our target widget */
   void *target;
   /* our direct painter object */
   void *direct;
   /* The fb */
   struct
     {
	DATA8             *data;
	int                bpp;
	int                rotation;
	int                width;
	struct {
	   DATA32          r, g, b;
	} mask;
     } fb;
   /* Relative to fb coords */
   struct {
      int                 count;
      QT_Direct_Rect     *r;
   } rects;
   /* Relative to fb coords */
   QT_Direct_Rect         location;
   QT_Direct_Rect         updated;
};

#ifdef __cplusplus
extern "C" {
#endif

/****/
void             x_software_qtopia_init                    (void);

void         evas_qtopia_outbuf_software_qtopia_init                   (void);
void         evas_qtopia_outbuf_software_qtopia_free                   (Outbuf *buf);

Outbuf      *evas_qtopia_outbuf_software_qtopia_setup_q                (int w, int h, int rot, Outbuf_Depth depth, void *target);

void         evas_qtopia_outbuf_software_qtopia_blit                   (Outbuf *buf, int src_x, int src_y, int w, int h, int dst_x, int dst_y);
void         evas_qtopia_outbuf_software_qtopia_update                 (Outbuf *buf, int x, int y, int w, int h);
RGBA_Image  *evas_qtopia_outbuf_software_qtopia_new_region_for_update  (Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void         evas_qtopia_outbuf_software_qtopia_free_region_for_update (Outbuf *buf, RGBA_Image *update);
void         evas_qtopia_outbuf_software_qtopia_push_updated_region    (Outbuf *buf, RGBA_Image *update, int x, int y, int w, int h);
void         evas_qtopia_outbuf_software_qtopia_reconfigure            (Outbuf *buf, int w, int h, int rot, Outbuf_Depth depth);
int          evas_qtopia_outbuf_software_qtopia_get_width              (Outbuf *buf);
int          evas_qtopia_outbuf_software_qtopia_get_height             (Outbuf *buf);
Outbuf_Depth evas_qtopia_outbuf_software_qtopia_get_depth              (Outbuf *buf);
int          evas_qtopia_outbuf_software_qtopia_get_rot                (Outbuf *buf);
int          evas_qtopia_outbuf_software_qtopia_get_have_backbuf       (Outbuf *buf);
void         evas_qtopia_outbuf_software_qtopia_set_have_backbuf       (Outbuf *buf, int have_backbuf);

QT_Direct   *evas_qt_main_direct_start                     (void *target);
void         evas_qt_main_direct_stop                      (QT_Direct *d);

#ifdef __cplusplus
}
#endif

#endif
