#ifndef EVAS_XLIB_SWAPBUF_H
#define EVAS_XLIB_SWAPBUF_H


#include "evas_engine.h"


void         evas_software_xlib_swapbuf_init(void);
void         evas_software_xlib_swapbuf_free(Outbuf *buf);
Outbuf      *evas_software_xlib_swapbuf_setup_x(int          w,
                                                int          h,
                                                int          rot,
                                                Outbuf_Depth depth,
                                                Display     *disp,
                                                Drawable     draw,
                                                Visual      *vis,
                                                Colormap     cmap,
                                                int          x_depth,
                                                int          grayscale,
                                                int          max_colors,
                                                Pixmap       mask,
                                                int          shape_dither,
                                                int          destination_alpha);
void  *evas_software_xlib_swapbuf_new_region_for_update(Outbuf *buf,
                                                        int     x,
                                                        int     y,
                                                        int     w,
                                                        int     h,
                                                        int    *cx,
                                                        int    *cy,
                                                        int    *cw,
                                                        int    *ch);
void         evas_software_xlib_swapbuf_free_region_for_update(Outbuf     *buf,
                                                               RGBA_Image *update);
void         evas_software_xlib_swapbuf_flush(Outbuf *buf, Tilebuf_Rect *surface_damage, Tilebuf_Rect *buffer_damage, Evas_Render_Mode render_mode);
void         evas_software_xlib_swapbuf_idle_flush(Outbuf *buf);
void         evas_software_xlib_swapbuf_push_updated_region(Outbuf     *buf,
                                                            RGBA_Image *update,
                                                            int         x,
                                                            int         y,
                                                            int         w,
                                                            int         h);
void         evas_software_xlib_swapbuf_reconfigure(Outbuf      *buf,
                                                    int          w,
                                                    int          h,
                                                    int          rot,
                                                    Outbuf_Depth depth);
int          evas_software_xlib_swapbuf_get_rot(Outbuf *buf);
void         evas_software_xlib_swapbuf_rotation_set(Outbuf *buf,
                                                     int     rot);
Eina_Bool    evas_software_xlib_swapbuf_alpha_get(Outbuf *buf);
Render_Output_Swap_Mode evas_software_xlib_swapbuf_buffer_state_get(Outbuf *buf);
#endif
