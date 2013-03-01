#ifndef _EVAS_SWAPPER_H
# define _EVAS_SWAPPER_H

#include "evas_engine.h"

typedef struct _Wl_Swapper Wl_Swapper;

Wl_Swapper *evas_swapper_setup(int dx, int dy, int w, int h, Outbuf_Depth depth, Eina_Bool alpha, struct wl_shm *shm, struct wl_surface *surface);
Wl_Swapper *evas_swapper_reconfigure(Wl_Swapper *ws, int dx, int dy, int w, int h, Outbuf_Depth depth, Eina_Bool alpha);
void evas_swapper_swap(Wl_Swapper *ws, Eina_Rectangle *rects, unsigned int count);
void evas_swapper_free(Wl_Swapper *ws);
void *evas_swapper_buffer_map(Wl_Swapper *ws, int *w, int *h);
void evas_swapper_buffer_unmap(Wl_Swapper *ws);
int evas_swapper_buffer_state_get(Wl_Swapper *ws);
void evas_swapper_buffer_idle_flush(Wl_Swapper *ws);

#endif
