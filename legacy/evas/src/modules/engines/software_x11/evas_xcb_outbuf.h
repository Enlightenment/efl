#ifndef EVAS_XCB_OUTBUF_H
# define EVAS_XCB_OUTBUF_H

# include "evas_engine.h"

void evas_software_xcb_outbuf_init(void);
void evas_software_xcb_outbuf_free(Outbuf *buf);
Outbuf *evas_software_xcb_outbuf_setup(int w, int h, int rot, Outbuf_Depth depth, xcb_connection_t *conn, xcb_screen_t *screen, xcb_drawable_t draw, xcb_visualtype_t *vis, xcb_colormap_t cmap, int xdepth, Eina_Bool grayscale, int max_colors, xcb_drawable_t mask, Eina_Bool shape_dither, Eina_Bool alpha);
RGBA_Image *evas_software_xcb_outbuf_new_region_for_update(Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void evas_software_xcb_outbuf_free_region_for_update(Outbuf *buf, RGBA_Image *update);
void evas_software_xcb_outbuf_flush(Outbuf *buf);
void evas_software_xcb_outbuf_idle_flush(Outbuf *buf);
void evas_software_xcb_outbuf_push_updated_region(Outbuf *buf, RGBA_Image *update, int x, int y, int w, int h);
void evas_software_xcb_outbuf_reconfigure(Outbuf *buf, int w, int h, int rot, Outbuf_Depth depth);
int evas_software_xcb_outbuf_width_get(Outbuf *buf);
int evas_software_xcb_outbuf_height_get(Outbuf *buf);
Outbuf_Depth evas_software_xcb_outbuf_depth_get(Outbuf *buf);
void evas_software_xcb_outbuf_drawable_set(Outbuf *buf, xcb_drawable_t drawable);
void evas_software_xcb_outbuf_mask_set(Outbuf *buf, xcb_drawable_t mask);
int evas_software_xcb_outbuf_rotation_get(Outbuf *buf);
void evas_software_xcb_outbuf_rotation_set(Outbuf *buf, int rotation);
Eina_Bool evas_software_xcb_outbuf_alpha_get(Outbuf *buf);
void evas_software_xcb_outbuf_debug_set(Outbuf *buf, Eina_Bool debug);
void evas_software_xcb_outbuf_debug_show(Outbuf *buf, xcb_drawable_t drawable, int x, int y, int w, int h);

# ifdef EVAS_FRAME_QUEUING
void evas_software_xcb_outbuf_priv_set(Outbuf *buf, void *cur, void *prev);
# endif

#endif
