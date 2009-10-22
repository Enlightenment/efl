#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H
#include "evas_common.h"
/*  this thing is for eina_log */
extern int _evas_engine_buffer_log_dom ;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_buffer_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_buffer_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_buffer_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_buffer_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_buffer_log_dom, __VA_ARGS__)

typedef struct _Outbuf                Outbuf;

typedef enum   _Outbuf_Depth          Outbuf_Depth;

enum _Outbuf_Depth
{
   OUTBUF_DEPTH_NONE,
     OUTBUF_DEPTH_ARGB_32BPP_8888_8888,
     OUTBUF_DEPTH_BGRA_32BPP_8888_8888,
     OUTBUF_DEPTH_RGB_32BPP_888_8888,
     OUTBUF_DEPTH_BGR_32BPP_888_8888,
     OUTBUF_DEPTH_RGB_24BPP_888_888,
     OUTBUF_DEPTH_BGR_24BPP_888_888,
     OUTBUF_DEPTH_LAST
};

struct _Outbuf
{
   int                           w, h;
   Outbuf_Depth                  depth;

   void                         *dest;
   int                           dest_row_bytes;

   int                           alpha_level;
   DATA32                        color_key;
   char                          use_color_key : 1;

   struct {
      void * (*new_update_region) (int x, int y, int w, int h, int *row_bytes);
      void   (*free_update_region) (int x, int y, int w, int h, void *data);
   } func;

   struct {
      RGBA_Image                *back_buf;
   } priv;
};

/****/

void         evas_buffer_outbuf_buf_init                   (void);
void         evas_buffer_outbuf_buf_free                   (Outbuf *buf);

Outbuf      *evas_buffer_outbuf_buf_setup_fb               (int w, int h, Outbuf_Depth depth, void *dest, int dest_row_bytes, int use_color_key, DATA32 color_key, int alpha_level,
							    void * (*new_update_region) (int x, int y, int w, int h, int *row_bytes),
							    void   (*free_update_region) (int x, int y, int w, int h, void *data));


RGBA_Image  *evas_buffer_outbuf_buf_new_region_for_update  (Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void         evas_buffer_outbuf_buf_free_region_for_update (Outbuf *buf, RGBA_Image *update);
void         evas_buffer_outbuf_buf_push_updated_region    (Outbuf *buf, RGBA_Image *update, int x, int y, int w, int h);

#endif

