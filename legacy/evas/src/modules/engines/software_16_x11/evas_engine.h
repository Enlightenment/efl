#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "evas_common_soft16.h"

typedef struct _X_Output_Buffer       X_Output_Buffer;

struct _X_Output_Buffer
{
   Soft16_Image    *im;
   Display         *display;
   XImage          *xim;
   XShmSegmentInfo *shm_info;
   void            *data;
};

/****/
void             evas_software_x11_x_init                        (void);

int              evas_software_x11_x_can_do_shm                  (Display *d);
X_Output_Buffer *evas_software_x11_x_output_buffer_new           (Display *d, Visual *v, int depth, int w, int h, int try_shm, void *data);
void             evas_software_x11_x_output_buffer_free          (X_Output_Buffer *xob, int sync);
void             evas_software_x11_x_output_buffer_paste         (X_Output_Buffer *xob, Drawable d, GC gc, int x, int y, int w, int h, int sync);
DATA8           *evas_software_x11_x_output_buffer_data          (X_Output_Buffer *xob, int *bytes_per_line_ret);
int              evas_software_x11_x_output_buffer_depth         (X_Output_Buffer *xob);
int              evas_software_x11_x_output_buffer_byte_order    (X_Output_Buffer *xob);
int              evas_software_x11_x_output_buffer_bit_order     (X_Output_Buffer *xob);

#endif
