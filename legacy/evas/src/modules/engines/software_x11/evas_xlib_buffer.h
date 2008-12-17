#ifndef EVAS_XLIB_BUFFER_H
#define EVAS_XLIB_BUFFER_H


#include "evas_engine.h"


typedef struct _X_Output_Buffer X_Output_Buffer;

struct _X_Output_Buffer
{
   Display         *display;
   XImage          *xim;
   XShmSegmentInfo *shm_info;
   Visual          *visual;
   void            *data;
   int              w;
   int              h;
   int              bpl;
   int              psize;
};

void evas_software_xlib_x_write_mask_line               (Outbuf *buf, X_Output_Buffer *xob, DATA32 *src, int w, int y);

int evas_software_xlib_x_can_do_shm                     (Display *d);

X_Output_Buffer *evas_software_xlib_x_output_buffer_new (Display *d, Visual *v, int depth, int w, int h, int try_shm, void *data);

void evas_software_xlib_x_output_buffer_free            (X_Output_Buffer *xob, int sync);

void evas_software_xlib_x_output_buffer_paste           (X_Output_Buffer *xob, Drawable d, GC gc, int x, int y, int sync);

DATA8 *evas_software_xlib_x_output_buffer_data          (X_Output_Buffer *xob, int *bytes_per_line_ret);

int evas_software_xlib_x_output_buffer_depth            (X_Output_Buffer *xob);

int evas_software_xlib_x_output_buffer_byte_order       (X_Output_Buffer *xob);

int evas_software_xlib_x_output_buffer_bit_order        (X_Output_Buffer *xob);


#endif
