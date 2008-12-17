#ifndef EVAS_XCB_OUTBUF_H
#define EVAS_XCB_OUTBUF_H


#include "evas_engine.h"


void         evas_software_xcb_outbuf_init (void);

void         evas_software_xcb_outbuf_free (Outbuf *buf);

Outbuf      *evas_software_xcb_outbuf_setup_x (int               w,
                                               int               h,
                                               int               rot,
                                               Outbuf_Depth      depth,
                                               xcb_connection_t *conn,
                                               xcb_screen_t     *screen,
                                               xcb_drawable_t    draw,
                                               xcb_visualtype_t *vis,
                                               xcb_colormap_t    cmap,
                                               int               x_depth,
                                               int               grayscale,
                                               int               max_colors,
                                               xcb_drawable_t    mask,
                                               int               shape_dither,
                                               int               destination_alpha);


RGBA_Image  *evas_software_xcb_outbuf_new_region_for_update (Outbuf *buf,
                                                             int     x,
                                                             int     y,
                                                             int     w,
                                                             int     h,
                                                             int    *cx,
                                                             int    *cy,
                                                             int    *cw,
                                                             int    *ch);

void         evas_software_xcb_outbuf_free_region_for_update (Outbuf     *buf,
                                                              RGBA_Image *update);

void         evas_software_xcb_outbuf_flush (Outbuf *buf);

void         evas_software_xcb_outbuf_idle_flush (Outbuf *buf);

void         evas_software_xcb_outbuf_push_updated_region (Outbuf     *buf,
                                                           RGBA_Image *update,
                                                           int         x,
                                                           int         y,
                                                           int         w,
                                                           int         h);

void         evas_software_xcb_outbuf_reconfigure (Outbuf      *buf,
                                                   int          w,
                                                   int          h,
                                                   int          rot,
                                                   Outbuf_Depth depth);

int          evas_software_xcb_outbuf_get_width (Outbuf *buf);

int          evas_software_xcb_outbuf_get_height (Outbuf *buf);

Outbuf_Depth evas_software_xcb_outbuf_get_depth (Outbuf *buf);

int          evas_software_xcb_outbuf_get_rot (Outbuf *buf);

void         evas_software_xcb_outbuf_drawable_set (Outbuf  *buf,
                                                    xcb_drawable_t draw);

void         evas_software_xcb_outbuf_mask_set (Outbuf *buf,
                                                xcb_drawable_t mask);

void         evas_software_xcb_outbuf_rotation_set (Outbuf *buf,
                                                    int     rot);

void         evas_software_xcb_outbuf_debug_set (Outbuf *buf,
                                                 int     debug);

void         evas_software_xcb_outbuf_debug_show (Outbuf  *buf,
                                                  xcb_drawable_t draw,
                                                  int      x,
                                                  int      y,
                                                  int      w,
                                                  int      h);

#endif
