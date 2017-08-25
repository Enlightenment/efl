#include "evas_engine.h"

typedef struct _X_Swapper X_Swapper;

X_Swapper *evas_xlib_swapper_new(Display *disp, Drawable draw, Visual *vis,
                                 int depth, int w, int h);
void evas_xlib_swapper_free(X_Swapper *swp);
void *evas_xlib_swapper_buffer_map(X_Swapper *swp, int *bpl, int *w, int *h);
void evas_xlib_swapper_buffer_unmap(X_Swapper *swp);
void evas_xlib_swapper_swap(X_Swapper *swp, Eina_Rectangle *rects, int nrects);
Render_Output_Swap_Mode evas_xlib_swapper_buffer_state_get(X_Swapper *swp);
int evas_xlib_swapper_depth_get(X_Swapper *swp);
int evas_xlib_swapper_byte_order_get(X_Swapper *swp);
int evas_xlib_swapper_bit_order_get(X_Swapper *swp);

