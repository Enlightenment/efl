#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H
#include "evas_common_private.h"

#include "../software_generic/Evas_Engine_Software_Generic.h"

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

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_buffer_log_dom, __VA_ARGS__)


struct _Outbuf
{
   int                           w, h;
   Outbuf_Depth                  depth;

   void                         *dest;
   unsigned int                  dest_row_bytes;
   void                         *switch_data;

   int                           alpha_level;
   DATA32                        color_key;
   Eina_Bool                     use_color_key : 1;
   Eina_Bool                     first_frame : 1;

   struct {
      void * (*new_update_region) (int x, int y, int w, int h, int *row_bytes);
      void   (*free_update_region) (int x, int y, int w, int h, void *data);
      void * (*switch_buffer) (void *data, void *dest_buffer);
   } func;

   struct {
      RGBA_Image                *back_buf;
   } priv;
};

/****/

void         evas_buffer_outbuf_buf_init                   (void);
void         evas_buffer_outbuf_buf_free                   (Outbuf *buf);

void         evas_buffer_outbuf_buf_update_fb              (Outbuf *buf,
                                                            int w, int h,
                                                            Outbuf_Depth depth,
                                                            void *dest,
                                                            int dest_row_bytes,
                                                            int use_color_key,
                                                            DATA32 color_key,
                                                            int alpha_level,
                                                            void * (*new_update_region) (int x, int y, int w, int h, int *row_bytes),
                                                            void   (*free_update_region) (int x, int y, int w, int h, void *data),
                                                            void * (*switch_buffer) (void *data, void *dest_buffer),
                                                            void *switch_data);
Outbuf      *evas_buffer_outbuf_buf_setup_fb               (int w, int h, Outbuf_Depth depth, void *dest, int dest_row_bytes, int use_color_key, DATA32 color_key, int alpha_level,
							    void * (*new_update_region) (int x, int y, int w, int h, int *row_bytes),
							    void   (*free_update_region) (int x, int y, int w, int h, void *data),
                                                            void * (*switch_buffer)(void *switch_data, void *dest),
                                                            void *switch_data);


void         evas_buffer_outbuf_reconfigure                (Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth);
void        *evas_buffer_outbuf_buf_new_region_for_update  (Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void         evas_buffer_outbuf_buf_free_region_for_update (Outbuf *buf, RGBA_Image *update);
void         evas_buffer_outbuf_buf_push_updated_region    (Outbuf *buf, RGBA_Image *update, int x, int y, int w, int h);
void         evas_buffer_outbuf_buf_switch_buffer          (Outbuf *buf, Tilebuf_Rect *surface_damage, Tilebuf_Rect *buffer_damage, Evas_Render_Mode render_mode);
Render_Output_Swap_Mode evas_buffer_outbuf_buf_swap_mode_get(Outbuf *buf);
int          evas_buffer_outbuf_buf_rot_get                (Outbuf *buf);

#endif
