#ifndef EVAS_XCB_BUFFER_H
# define EVAS_XCB_BUFFER_H

# include "evas_engine.h"

typedef struct _Xcb_Output_Buffer Xcb_Output_Buffer;
struct _Xcb_Output_Buffer 
{
   xcb_connection_t *connection;
   xcb_visualtype_t *visual;
   xcb_image_t *xim;
   xcb_shm_segment_info_t *shm_info;
   unsigned char *data;
   int w, h, bpl, psize;
};

void evas_software_xcb_write_mask_line(Outbuf *buf, Xcb_Output_Buffer *xcbob, DATA32 *src, int w, int y);
void evas_software_xcb_write_mask_line_rev(Outbuf *buf, Xcb_Output_Buffer *xcbob, DATA32 *src, int w, int y);
void evas_software_xcb_write_mask_line_vert(Outbuf *buf, Xcb_Output_Buffer *xcbob, DATA32 *src, int h, int y, int w);
void evas_software_xcb_write_mask_line_vert_rev(Outbuf *buf, Xcb_Output_Buffer *xcbob, DATA32 *src, int h, int y, int w);
Eina_Bool evas_software_xcb_can_do_shm(xcb_connection_t *conn, xcb_screen_t *screen);
Xcb_Output_Buffer *evas_software_xcb_output_buffer_new(xcb_connection_t *conn, xcb_visualtype_t *vis, int depth, int w, int h, Eina_Bool try_shm, unsigned char *data);
void evas_software_xcb_output_buffer_free(Xcb_Output_Buffer *xcbob, Eina_Bool sync);
void evas_software_xcb_output_buffer_paste(Xcb_Output_Buffer *xcbob, xcb_drawable_t drawable, xcb_gcontext_t gc, int x, int y, Eina_Bool sync);
DATA8 *evas_software_xcb_output_buffer_data(Xcb_Output_Buffer *xcbob, int *bpl_ret);
int evas_software_xcb_output_buffer_depth(Xcb_Output_Buffer *xcbob);
int evas_software_xcb_output_buffer_byte_order(Xcb_Output_Buffer *xcbob);
int evas_software_xcb_output_buffer_bit_order(Xcb_Output_Buffer *xcbob);

#endif
