#ifndef _EVAS_SWAPBUF_H
# define _EVAS_SWAPBUF_H

# include "evas_engine.h"

Outbuf *evas_swapbuf_setup(Evas_Engine_Info_Wayland_Shm *info, int w, int h, unsigned int rotation, Outbuf_Depth depth, Eina_Bool alpha, struct wl_shm *wl_shm, struct wl_surface *wl_surface);
void evas_swapbuf_free(Outbuf *ob);
void evas_swapbuf_reconfigure(Outbuf *ob, int x, int y, int w, int h, int rotation, Outbuf_Depth depth, Eina_Bool alpha);
RGBA_Image *evas_swapbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void evas_swapbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
void evas_swapbuf_update_region_free(Outbuf *ob, RGBA_Image *update);
void evas_swapbuf_flush(Outbuf *ob);
void evas_swapbuf_idle_flush(Outbuf *ob EINA_UNUSED);
Render_Engine_Swap_Mode evas_swapbuf_state_get(Outbuf *ob);
int evas_swapbuf_rotation_get(Outbuf *ob);

#endif
