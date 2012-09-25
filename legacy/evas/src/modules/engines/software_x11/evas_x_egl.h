#ifndef EVAS_X_EGL_H
#define EVAS_X_EGL_H

#include "evas_engine.h"

void *_egl_x_disp_get(void *d);
int _egl_x_disp_init(void *ed);
void *_egl_x_disp_choose_config(void *ed);
void *_egl_x_win_surf_new(void *ed, Window win, void *config);
void _egl_x_win_surf_free(void *ed, void *surf);
void *_egl_x_surf_map(void *ed, void *surf, int *stride);
void _egl_x_surf_unmap(void *ed, void *surf);
void _egl_x_surf_swap(void *ed, void *surf, int vsync);

#endif
