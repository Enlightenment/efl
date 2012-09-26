#ifndef EVAS_X_EGL_H
#define EVAS_X_EGL_H

#include "evas_engine.h"

void *_egl_x_disp_get(void *d);
void _egl_x_disp_terminate(void *ed);
int _egl_x_disp_init(void *ed);
void *_egl_x_disp_choose_config(void *ed);
void *_egl_x_win_surf_new(void *ed, Window win, void *config);
void _egl_x_win_surf_free(void *ed, void *surf);
void *_egl_x_surf_map(void *ed, void *surf, int *stride);
void _egl_x_surf_unmap(void *ed, void *surf);
void _egl_x_surf_swap(void *ed, void *surf, int vsync);

Outbuf *
evas_software_egl_outbuf_setup_x(int w, int h, int rot, Outbuf_Depth depth,
                                 Display *disp, Drawable draw, Visual *vis,
                                 Colormap cmap, int x_depth,
                                 int grayscale, int max_colors, Pixmap mask,
                                 int shape_dither, int destination_alpha);

#endif
