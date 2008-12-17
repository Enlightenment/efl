#ifndef EVAS_XCB_BUFFER_H
#define EVAS_XCB_BUFFER_H


#include "evas_engine.h"


typedef struct _Xcb_Output_Buffer     Xcb_Output_Buffer;

struct _Xcb_Output_Buffer
{
   xcb_connection_t       *connection;
   xcb_image_t            *image;
   xcb_shm_segment_info_t *shm_info;
   void                   *data;
   int                     w;
   int                     h;
   int                     bpl;
   int                     psize;
};

void               evas_software_xcb_x_write_mask_line         (Outbuf            *buf,
                                                                Xcb_Output_Buffer *xcbob,
								DATA32            *src,
								int                w,
								int                y);
int                evas_software_xcb_x_can_do_shm              (xcb_connection_t *c,
                                                                xcb_screen_t     *screen);
Xcb_Output_Buffer *evas_software_xcb_x_output_buffer_new       (xcb_connection_t *c,
								int            depth,
								int            w,
								int            h,
								int            try_shm,
								void          *data);
void               evas_software_xcb_x_output_buffer_free      (Xcb_Output_Buffer *xcbob,
								int                sync);
void               evas_software_xcb_x_output_buffer_paste     (Xcb_Output_Buffer *xcbob,
								xcb_drawable_t        d,
								xcb_gcontext_t        gc,
								int                x,
								int                y,
								int                sync);
DATA8             *evas_software_xcb_x_output_buffer_data      (Xcb_Output_Buffer *xcbob,
								int               *bytes_per_line_ret);
int                evas_software_xcb_x_output_buffer_depth     (Xcb_Output_Buffer *xcbob);
int                evas_software_xcb_x_output_buffer_byte_order(Xcb_Output_Buffer *xcbob);
int                evas_software_xcb_x_output_buffer_bit_order (Xcb_Output_Buffer *xcbob);


#endif
