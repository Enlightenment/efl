#ifndef _ECORE_EVAS_H
#define _ECORE_EVAS_H

/* FIXME:
 * to do soon:
 * - iconfication api needs to work
 * - maximization api nees to work
 * - attach keyed data to an ecore_evas canvas
 * - document all calls
 * 
 * later:
 * - qt back-end ???
 * - dfb back-end ??? (dfb's threads make this REALLY HARD)
 */

#include <Evas.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ECORE_X_H
#define _ECORE_X_WINDOW_PREDEF
typedef unsigned int Ecore_X_Window;
#endif
   
#ifndef _ECORE_EVAS_PRIVATE_H
/* basic data types */
typedef void Ecore_Evas;   
#endif

/* module setup/shutdown calls */
int         ecore_evas_init(void);
int         ecore_evas_shutdown(void);

/* engine/target specific init calls */
Ecore_Evas     *ecore_evas_software_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
Ecore_Evas     *ecore_evas_gl_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
Ecore_X_Window  ecore_evas_software_x11_window_get(Ecore_Evas *ee);
Ecore_Evas     *ecore_evas_fb_new(char *disp_name, int rotation, int w, int h);

/* generic manipulation calls */
void        ecore_evas_free(Ecore_Evas *ee);
void        ecore_evas_callback_resize_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
void        ecore_evas_callback_move_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
void        ecore_evas_callback_show_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
void        ecore_evas_callback_hide_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
void        ecore_evas_callback_delete_request_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
void        ecore_evas_callback_destroy_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
void        ecore_evas_callback_focus_in_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
void        ecore_evas_callback_focus_out_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
void        ecore_evas_callback_mouse_in_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
void        ecore_evas_callback_mouse_out_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
void        ecore_evas_callback_pre_render_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
void        ecore_evas_callback_post_render_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
Evas       *ecore_evas_get(Ecore_Evas *ee);
void        ecore_evas_move(Ecore_Evas *ee, int x, int y);
void        ecore_evas_resize(Ecore_Evas *ee, int w, int h);
void        ecore_evas_move_resize(Ecore_Evas *ee, int x, int y, int w, int h);
void        ecore_evas_geometry_get(Ecore_Evas *ee, int *x, int *y, int *w, int *h);
void        ecore_evas_rotation_set(Ecore_Evas *ee, int rot);
int         ecore_evas_rotation_get(Ecore_Evas *ee);
void        ecore_evas_shaped_set(Ecore_Evas *ee, int shaped);
int         ecore_evas_shaped_get(Ecore_Evas *ee);
void        ecore_evas_show(Ecore_Evas *ee);
void        ecore_evas_hide(Ecore_Evas *ee);   
int         ecore_evas_visibility_get(Ecore_Evas *ee);
void        ecore_evas_raise(Ecore_Evas *ee);
void        ecore_evas_lower(Ecore_Evas *ee);       
void        ecore_evas_title_set(Ecore_Evas *ee, const char *t);
const char *ecore_evas_title_get(Ecore_Evas *ee);
void        ecore_evas_name_class_set(Ecore_Evas *ee, const char *n, const char *c);
void        ecore_evas_name_class_get(Ecore_Evas *ee, const char **n, const char **c);
void        ecore_evas_size_min_set(Ecore_Evas *ee, int w, int h);
void        ecore_evas_size_min_get(Ecore_Evas *ee, int *w, int *h);
void        ecore_evas_size_max_set(Ecore_Evas *ee, int w, int h);
void        ecore_evas_size_max_get(Ecore_Evas *ee, int *w, int *h);
void        ecore_evas_size_base_set(Ecore_Evas *ee, int w, int h);
void        ecore_evas_size_base_get(Ecore_Evas *ee, int *w, int *h);
void        ecore_evas_size_step_set(Ecore_Evas *ee, int w, int h);
void        ecore_evas_size_step_get(Ecore_Evas *ee, int *w, int *h);
void        ecore_evas_cursor_set(Ecore_Evas *ee, const char *file, int layer, int hot_x, int hot_y);
void        ecore_evas_cursor_get(Ecore_Evas *ee, char **file, int *layer, int *hot_x, int *hot_y);
void        ecore_evas_layer_set(Ecore_Evas *ee, int layer);
int         ecore_evas_layer_get(Ecore_Evas *ee);
void        ecore_evas_focus_set(Ecore_Evas *ee, int on);
int         ecore_evas_focus_get(Ecore_Evas *ee);
void        ecore_evas_iconified_set(Ecore_Evas *ee, int on);
int         ecore_evas_iconified_get(Ecore_Evas *ee);
void        ecore_evas_borderless_set(Ecore_Evas *ee, int on);
int         ecore_evas_borderless_get(Ecore_Evas *ee);
void        ecore_evas_override_set(Ecore_Evas *ee, int on);
int         ecore_evas_override_get(Ecore_Evas *ee);
void        ecore_evas_maximized_set(Ecore_Evas *ee, int on);
int         ecore_evas_maximized_get(Ecore_Evas *ee);
void        ecore_evas_fullscreen_set(Ecore_Evas *ee, int on);
int         ecore_evas_fullscreen_get(Ecore_Evas *ee);
void        ecore_evas_avoid_damage_set(Ecore_Evas *ee, int on);
int         ecore_evas_avoid_damage_get(Ecore_Evas *ee);
void        ecore_evas_withdrawn_set(Ecore_Evas *ee, int withdrawn);
int         ecore_evas_withdrawn_get(Ecore_Evas *ee);
void        ecore_evas_sticky_set(Ecore_Evas *ee, int sticky);
int         ecore_evas_sticky_get(Ecore_Evas *ee);

#ifdef __cplusplus
}
#endif

#endif
