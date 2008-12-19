/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _ECORE_EVAS_H
#define _ECORE_EVAS_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

/**
 * @file Ecore_Evas.h
 * @brief Evas wrapper functions
 */

/* FIXME:
 * to do soon:
 * - iconfication api needs to work
 * - maximization api needs to work
 * - document all calls
 *
 * later:
 * - buffer back-end that renders to an evas_image_object ???
 * - qt back-end ???
 * - dfb back-end ??? (dfb's threads make this REALLY HARD)
 */

#include <Evas.h>

#ifdef __cplusplus
extern "C" {
#endif

/* these are dummy and just tell u what API levels ecore_evas supports - not if
 * the actual support is compiled in. you need to query for that separately.
 */
#define HAVE_ECORE_EVAS_X 1
#define HAVE_ECORE_EVAS_FB 1
#define HAVE_ECORE_EVAS_X11_GL 1
#define HAVE_ECORE_EVAS_X11_16 1
#define HAVE_ECORE_EVAS_DIRECTFB 1
#define HAVE_ECORE_EVAS_WIN32 1
#define HAVE_ECORE_EVAS_SDL 1
#define HAVE_ECORE_EVAS_WINCE 1

typedef enum _Ecore_Evas_Engine_Type
{
   ECORE_EVAS_ENGINE_SOFTWARE_BUFFER,
   ECORE_EVAS_ENGINE_SOFTWARE_X11,
   ECORE_EVAS_ENGINE_XRENDER_X11,
   ECORE_EVAS_ENGINE_OPENGL_X11,
   ECORE_EVAS_ENGINE_SOFTWARE_XCB,
   ECORE_EVAS_ENGINE_XRENDER_XCB,
   ECORE_EVAS_ENGINE_SOFTWARE_DDRAW,
   ECORE_EVAS_ENGINE_DIRECT3D,
   ECORE_EVAS_ENGINE_OPENGL_GLEW,
   ECORE_EVAS_ENGINE_SDL,
   ECORE_EVAS_ENGINE_DIRECTFB,
   ECORE_EVAS_ENGINE_SOFTWARE_FB,
   ECORE_EVAS_ENGINE_SOFTWARE_16_X11,
   ECORE_EVAS_ENGINE_SOFTWARE_16_DDRAW,
   ECORE_EVAS_ENGINE_SOFTWARE_16_WINCE
} Ecore_Evas_Engine_Type;

typedef enum _Ecore_Evas_Avoid_Damage_Type
{
   ECORE_EVAS_AVOID_DAMAGE_NONE = 0,
   ECORE_EVAS_AVOID_DAMAGE_EXPOSE = 1,
   ECORE_EVAS_AVOID_DAMAGE_BUILT_IN = 2
} Ecore_Evas_Avoid_Damage_Type;

typedef enum _Ecore_Evas_Object_Associate_Flags
{
  ECORE_EVAS_OBJECT_ASSOCIATE_BASE  = 0,
  ECORE_EVAS_OBJECT_ASSOCIATE_STACK = 1 << 0,
  ECORE_EVAS_OBJECT_ASSOCIATE_LAYER = 1 << 1,
  ECORE_EVAS_OBJECT_ASSOCIATE_DEL = 1 << 2
} Ecore_Evas_Object_Associate_Flags;

#ifndef _ECORE_X_H
#define _ECORE_X_WINDOW_PREDEF
typedef unsigned int Ecore_X_Window;
#endif

#ifndef _ECORE_DIRECTFB_H
#define _ECORE_DIRECTFB_WINDOW_PREDEF
typedef struct _Ecore_DirectFB_Window Ecore_DirectFB_Window;
#endif

#ifndef __ECORE_WIN32_H__
typedef void Ecore_Win32_Window;
#endif

#ifndef __ECORE_WINCE_H__
typedef void Ecore_WinCE_Window;
#endif

#ifndef _ECORE_EVAS_PRIVATE_H
/* basic data types */
typedef struct _Ecore_Evas Ecore_Evas;
#endif

/* module setup/shutdown calls */

EAPI int         ecore_evas_engine_type_supported_get(Ecore_Evas_Engine_Type engine);

EAPI int         ecore_evas_init(void);
EAPI int         ecore_evas_shutdown(void);

EAPI Eina_List  *ecore_evas_engines_get(void);
EAPI void        ecore_evas_engines_free(Eina_List *engines);
EAPI Ecore_Evas *ecore_evas_new(const char *engine_name, int x, int y, int w, int h, const char *extra_options);


/* engine/target specific init calls */
EAPI Ecore_Evas     *ecore_evas_software_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window  ecore_evas_software_x11_window_get(const Ecore_Evas *ee);
EAPI Ecore_X_Window  ecore_evas_software_x11_subwindow_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_direct_resize_set(Ecore_Evas *ee, int on);
EAPI int             ecore_evas_software_x11_direct_resize_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);

EAPI Ecore_Evas     *ecore_evas_gl_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window  ecore_evas_gl_x11_window_get(const Ecore_Evas *ee);
EAPI Ecore_X_Window  ecore_evas_gl_x11_subwindow_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_gl_x11_direct_resize_set(Ecore_Evas *ee, int on);
EAPI int             ecore_evas_gl_x11_direct_resize_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_gl_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);

EAPI Ecore_Evas     *ecore_evas_xrender_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window  ecore_evas_xrender_x11_window_get(const Ecore_Evas *ee);
EAPI Ecore_X_Window  ecore_evas_xrender_x11_subwindow_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_xrender_x11_direct_resize_set(Ecore_Evas *ee, int on);
EAPI int             ecore_evas_xrender_x11_direct_resize_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_xrender_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);

EAPI Ecore_Evas     *ecore_evas_software_x11_16_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window  ecore_evas_software_x11_16_window_get(const Ecore_Evas *ee);
EAPI Ecore_X_Window  ecore_evas_software_x11_16_subwindow_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_16_direct_resize_set(Ecore_Evas *ee, int on);
EAPI int             ecore_evas_software_x11_16_direct_resize_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_16_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);

EAPI Ecore_Evas     *ecore_evas_fb_new(const char *disp_name, int rotation, int w, int h);

EAPI Ecore_Evas     *ecore_evas_directfb_new(const char *disp_name, int windowed, int x, int y, int w, int h);
EAPI Ecore_DirectFB_Window *ecore_evas_directfb_window_get(const Ecore_Evas *ee);

EAPI Ecore_Evas     *ecore_evas_buffer_new(int w, int h);
EAPI const void     *ecore_evas_buffer_pixels_get(Ecore_Evas *ee);

EAPI Evas_Object    *ecore_evas_object_image_new(Ecore_Evas *ee_target);

EAPI Ecore_Evas     *ecore_evas_software_ddraw_new(Ecore_Win32_Window *parent,
                                                   int                 x,
                                                   int                 y,
                                                   int                 width,
                                                   int                 height);

EAPI Ecore_Evas     *ecore_evas_software_ddraw_16_new(Ecore_Win32_Window *parent,
                                                      int                 x,
                                                      int                 y,
                                                      int                 width,
                                                      int                 height);

EAPI Ecore_Evas     *ecore_evas_direct3d_new(Ecore_Win32_Window *parent,
                                             int                 x,
                                             int                 y,
                                             int                 width,
                                             int                 height);

EAPI Ecore_Evas *ecore_evas_gl_glew_new(Ecore_Win32_Window *parent,
                                        int                 x,
                                        int                 y,
                                        int                 width,
                                        int                 height);

EAPI Ecore_Win32_Window *ecore_evas_win32_window_get(const Ecore_Evas *ee);

EAPI Ecore_Evas     *ecore_evas_sdl_new(const char* name, int w, int h, int fullscreen, int hwsurface, int noframe, int alpha);
EAPI Ecore_Evas     *ecore_evas_sdl16_new(const char* name, int w, int h, int fullscreen, int hwsurface, int noframe, int alpha);

EAPI Ecore_Evas     *ecore_evas_software_wince_new(Ecore_WinCE_Window *parent,
                                                   int                 x,
                                                   int                 y,
                                                   int                 width,
                                                   int                 height);

EAPI Ecore_Evas     *ecore_evas_software_wince_fb_new(Ecore_WinCE_Window *parent,
                                                      int                 x,
                                                      int                 y,
                                                      int                 width,
                                                      int                 height);

EAPI Ecore_Evas     *ecore_evas_software_wince_gapi_new(Ecore_WinCE_Window *parent,
                                                        int                 x,
                                                        int                 y,
                                                        int                 width,
                                                        int                 height);

EAPI Ecore_Evas     *ecore_evas_software_wince_ddraw_new(Ecore_WinCE_Window *parent,
                                                         int                 x,
                                                         int                 y,
                                                         int                 width,
                                                         int                 height);

EAPI Ecore_Evas     *ecore_evas_software_wince_gdi_new(Ecore_WinCE_Window *parent,
                                                       int                 x,
                                                       int                 y,
                                                       int                 width,
                                                       int                 height);

EAPI Ecore_WinCE_Window *ecore_evas_software_wince_window_get(const Ecore_Evas *ee);

/* generic manipulation calls */
EAPI const char *ecore_evas_engine_name_get(const Ecore_Evas *ee);
EAPI Ecore_Evas *ecore_evas_ecore_evas_get(const Evas *e);
EAPI void        ecore_evas_free(Ecore_Evas *ee);
EAPI void       *ecore_evas_data_get(const Ecore_Evas *ee, const char *key);
EAPI void        ecore_evas_data_set(Ecore_Evas *ee, const char *key, const void *data);
EAPI void        ecore_evas_callback_resize_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_move_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_show_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_hide_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_delete_request_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_destroy_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_focus_in_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_focus_out_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_sticky_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_unsticky_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_mouse_in_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_mouse_out_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_pre_render_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_post_render_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI void        ecore_evas_callback_pre_free_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI Evas       *ecore_evas_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_move(Ecore_Evas *ee, int x, int y);
EAPI void        ecore_evas_managed_move(Ecore_Evas *ee, int x, int y);
EAPI void        ecore_evas_resize(Ecore_Evas *ee, int w, int h);
EAPI void        ecore_evas_move_resize(Ecore_Evas *ee, int x, int y, int w, int h);
EAPI void        ecore_evas_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);
EAPI void        ecore_evas_rotation_set(Ecore_Evas *ee, int rot);
EAPI int         ecore_evas_rotation_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_shaped_set(Ecore_Evas *ee, int shaped);
EAPI int         ecore_evas_shaped_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_alpha_set(Ecore_Evas *ee, int alpha);
EAPI int         ecore_evas_alpha_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_show(Ecore_Evas *ee);
EAPI void        ecore_evas_hide(Ecore_Evas *ee);
EAPI int         ecore_evas_visibility_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_raise(Ecore_Evas *ee);
EAPI void        ecore_evas_lower(Ecore_Evas *ee);
EAPI void        ecore_evas_activate(Ecore_Evas *ee);
EAPI void        ecore_evas_title_set(Ecore_Evas *ee, const char *t);
EAPI const char *ecore_evas_title_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_name_class_set(Ecore_Evas *ee, const char *n, const char *c);
EAPI void        ecore_evas_name_class_get(const Ecore_Evas *ee, const char **n, const char **c);
EAPI void        ecore_evas_size_min_set(Ecore_Evas *ee, int w, int h);
EAPI void        ecore_evas_size_min_get(const Ecore_Evas *ee, int *w, int *h);
EAPI void        ecore_evas_size_max_set(Ecore_Evas *ee, int w, int h);
EAPI void        ecore_evas_size_max_get(const Ecore_Evas *ee, int *w, int *h);
EAPI void        ecore_evas_size_base_set(Ecore_Evas *ee, int w, int h);
EAPI void        ecore_evas_size_base_get(const Ecore_Evas *ee, int *w, int *h);
EAPI void        ecore_evas_size_step_set(Ecore_Evas *ee, int w, int h);
EAPI void        ecore_evas_size_step_get(const Ecore_Evas *ee, int *w, int *h);
EAPI void        ecore_evas_cursor_set(Ecore_Evas *ee, const char *file, int layer, int hot_x, int hot_y);
EAPI void        ecore_evas_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y);
EAPI void        ecore_evas_cursor_get(const Ecore_Evas *ee, Evas_Object **obj, int *layer, int *hot_x, int *hot_y);
EAPI void        ecore_evas_layer_set(Ecore_Evas *ee, int layer);
EAPI int         ecore_evas_layer_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_focus_set(Ecore_Evas *ee, int on);
EAPI int         ecore_evas_focus_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_iconified_set(Ecore_Evas *ee, int on);
EAPI int         ecore_evas_iconified_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_borderless_set(Ecore_Evas *ee, int on);
EAPI int         ecore_evas_borderless_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_override_set(Ecore_Evas *ee, int on);
EAPI int         ecore_evas_override_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_maximized_set(Ecore_Evas *ee, int on);
EAPI int         ecore_evas_maximized_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_fullscreen_set(Ecore_Evas *ee, int on);
EAPI int         ecore_evas_fullscreen_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_avoid_damage_set(Ecore_Evas *ee, Ecore_Evas_Avoid_Damage_Type on);
EAPI Ecore_Evas_Avoid_Damage_Type ecore_evas_avoid_damage_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_withdrawn_set(Ecore_Evas *ee, int withdrawn);
EAPI int         ecore_evas_withdrawn_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_sticky_set(Ecore_Evas *ee, int sticky);
EAPI int         ecore_evas_sticky_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_ignore_events_set(Ecore_Evas *ee, int ignore);
EAPI int         ecore_evas_ignore_events_get(const Ecore_Evas *ee);
EAPI void       *ecore_evas_window_get(const Ecore_Evas *ee);


EAPI int          ecore_evas_object_associate(Ecore_Evas *ee, Evas_Object *obj, Ecore_Evas_Object_Associate_Flags flags);
EAPI int          ecore_evas_object_dissociate(Ecore_Evas *ee, Evas_Object *obj);
EAPI Evas_Object *ecore_evas_object_associate_get(const Ecore_Evas *ee);

#ifdef __cplusplus
}
#endif

#endif
