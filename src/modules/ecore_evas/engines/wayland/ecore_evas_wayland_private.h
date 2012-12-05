#ifndef _ECORE_EVAS_WAYLAND_PRIVATE_H_
#define _ECORE_EVAS_WAYLAND_PRIVATE_H_

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

//#define LOGFNS 1
#ifdef LOGFNS
# include <stdio.h>
# define LOGFN(fl, ln, fn) \
   printf("-ECORE_EVAS-WL: %25s: %5i - %s\n", fl, ln, fn);
#else
# define LOGFN(fl, ln, fn)
#endif

#include <Eina.h>

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"
#include <Ecore_Wayland.h>

typedef struct _Ecore_Evas_Engine_Wl_Data Ecore_Evas_Engine_Wl_Data;

struct _Ecore_Evas_Engine_Wl_Data {
   Ecore_Wl_Window *parent, *win;
   Evas_Object *frame;

# if defined(BUILD_ECORE_EVAS_WAYLAND_SHM)
   struct wl_shm_pool *pool;
   size_t pool_size;
   void *pool_data;
   struct wl_buffer *buffer;
# endif
};

Ecore_Evas_Interface_Wayland *_ecore_evas_wl_interface_new(void);

int  _ecore_evas_wl_common_init(void);
int  _ecore_evas_wl_common_shutdown(void);
void _ecore_evas_wl_common_pre_free(Ecore_Evas *ee);
void _ecore_evas_wl_common_free(Ecore_Evas *ee);
void _ecore_evas_wl_common_callback_resize_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
void _ecore_evas_wl_common_callback_move_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
void _ecore_evas_wl_common_callback_delete_request_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
void _ecore_evas_wl_common_callback_focus_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
void _ecore_evas_wl_common_callback_focus_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
void _ecore_evas_wl_common_callback_mouse_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
void _ecore_evas_wl_common_callback_mouse_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
void _ecore_evas_wl_common_move(Ecore_Evas *ee, int x, int y);
void _ecore_evas_wl_common_raise(Ecore_Evas *ee);
void _ecore_evas_wl_common_title_set(Ecore_Evas *ee, const char *title);
void _ecore_evas_wl_common_name_class_set(Ecore_Evas *ee, const char *n, const char *c);
void _ecore_evas_wl_common_size_min_set(Ecore_Evas *ee, int w, int h);
void _ecore_evas_wl_common_size_max_set(Ecore_Evas *ee, int w, int h);
void _ecore_evas_wl_common_size_base_set(Ecore_Evas *ee, int w, int h);
void _ecore_evas_wl_common_size_step_set(Ecore_Evas *ee, int w, int h);
void _ecore_evas_wl_common_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y);
void _ecore_evas_wl_common_layer_set(Ecore_Evas *ee, int layer);
void _ecore_evas_wl_common_iconified_set(Ecore_Evas *ee, int iconify);
void _ecore_evas_wl_common_maximized_set(Ecore_Evas *ee, int max);
void _ecore_evas_wl_common_fullscreen_set(Ecore_Evas *ee, int full);
void _ecore_evas_wl_common_ignore_events_set(Ecore_Evas *ee, int ignore);
int  _ecore_evas_wl_common_pre_render(Ecore_Evas *ee);
int  _ecore_evas_wl_common_render_updates(Ecore_Evas *ee);
void _ecore_evas_wl_common_post_render(Ecore_Evas *ee);
int  _ecore_evas_wl_common_render(Ecore_Evas *ee);
void _ecore_evas_wl_common_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);
void _ecore_evas_wl_common_screen_dpi_get(const Ecore_Evas *ee, int *xdpi, int *ydpi);

Evas_Object * _ecore_evas_wl_common_frame_add(Evas *evas);

#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
void _ecore_evas_wayland_shm_resize(Ecore_Evas *ee, int location);
#endif

#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
void _ecore_evas_wayland_egl_resize(Ecore_Evas *ee, int location);
#endif

#endif /* _ECORE_EVAS_WAYLAND_PRIVATE_H_ */
