#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include <sys/ipc.h>
#include <sys/shm.h>

#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <xcb/xcb_image.h>

#include "evas_common.h"
#include "evas_common_soft8.h"

extern int _evas_engine_soft8_x11_log_dom;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_soft8_x11_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_soft8_x11_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_soft8_x11_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_soft8_x11_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_soft8_x11_log_dom, __VA_ARGS__)

typedef struct _X_Output_Buffer       X_Output_Buffer;

struct _X_Output_Buffer
{
   Soft8_Image			*im;
   xcb_connection_t		*connection;
   xcb_screen_t			*screen;
   xcb_image_t			*xim;
   xcb_drawable_t		drawable;
   xcb_gcontext_t		gc;
   xcb_shm_segment_info_t	*shm_info;
   unsigned char		*pal;
   void				*data;
};

/****/
void             evas_software_x11_x_init                        (void);

int              evas_software_x11_x_can_do_shm                  (xcb_connection_t *c, xcb_screen_t *screen);
X_Output_Buffer *evas_software_x11_x_output_buffer_new           (xcb_connection_t *, xcb_screen_t *screen, int depth, unsigned char *pal, int w, int h, int try_shm, void *data);
void             evas_software_x11_x_output_buffer_free          (X_Output_Buffer *xob, int sync);
void             evas_software_x11_x_output_buffer_paste         (X_Output_Buffer *xob, xcb_drawable_t d, xcb_gcontext_t gc, int x, int y, int w, int h, int sync);
DATA8           *evas_software_x11_x_output_buffer_data          (X_Output_Buffer *xob, int *bytes_per_line_ret);
int              evas_software_x11_x_output_buffer_depth         (X_Output_Buffer *xob);
int              evas_software_x11_x_output_buffer_byte_order    (X_Output_Buffer *xob);
int              evas_software_x11_x_output_buffer_bit_order     (X_Output_Buffer *xob);

#endif
