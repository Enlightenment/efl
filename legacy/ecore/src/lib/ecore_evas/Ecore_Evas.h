#ifndef _ECORE_EVAS_H
#define _ECORE_EVAS_H

#include <Evas.h>
#include <Ecore_Getopt.h>
#include <Ecore_Input.h>

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
 *
 * The following is a list of example that partially exemplify Ecore_Evas's API:
 * @li @ref ecore_evas_callbacks_example_c
 * @li @ref ecore_evas_object_example_c
 * @li @ref ecore_evas_basics_example_c
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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Ecore_Evas_Group Ecore_Evas wrapper/helper set of functions
 *
 * This is a list of examples of these functions:
 * - @ref Ecore_Evas_Window_Sizes_Example_c
 * @{
 */

/* these are dummy and just tell u what API levels ecore_evas supports - not if
 * the actual support is compiled in. you need to query for that separately.
 */
#define HAVE_ECORE_EVAS_X 1
#define HAVE_ECORE_EVAS_FB 1
#define HAVE_ECORE_EVAS_X11_GL 1
#define HAVE_ECORE_EVAS_X11_16 1
#define HAVE_ECORE_EVAS_DIRECTFB 1
#define HAVE_ECORE_EVAS_WIN32 1
#define HAVE_ECORE_EVAS_COCOA 1
#define HAVE_ECORE_EVAS_SDL 1
#define HAVE_ECORE_EVAS_WINCE 1

typedef enum _Ecore_Evas_Engine_Type
{
   ECORE_EVAS_ENGINE_SOFTWARE_BUFFER,
   ECORE_EVAS_ENGINE_SOFTWARE_XLIB,
   ECORE_EVAS_ENGINE_XRENDER_X11,
   ECORE_EVAS_ENGINE_OPENGL_X11,
   ECORE_EVAS_ENGINE_SOFTWARE_XCB,
   ECORE_EVAS_ENGINE_XRENDER_XCB,
   ECORE_EVAS_ENGINE_SOFTWARE_GDI,
   ECORE_EVAS_ENGINE_SOFTWARE_DDRAW,
   ECORE_EVAS_ENGINE_DIRECT3D,
   ECORE_EVAS_ENGINE_OPENGL_GLEW,
   ECORE_EVAS_ENGINE_COCOA,
   ECORE_EVAS_ENGINE_SOFTWARE_SDL,
   ECORE_EVAS_ENGINE_DIRECTFB,
   ECORE_EVAS_ENGINE_SOFTWARE_FB,
   ECORE_EVAS_ENGINE_SOFTWARE_8_X11,
   ECORE_EVAS_ENGINE_SOFTWARE_16_X11,
   ECORE_EVAS_ENGINE_SOFTWARE_16_DDRAW,
   ECORE_EVAS_ENGINE_SOFTWARE_16_WINCE,
   ECORE_EVAS_ENGINE_OPENGL_SDL
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
typedef struct _Ecore_Win32_Window Ecore_Win32_Window;
#endif

#ifndef __ECORE_WINCE_H__
typedef struct _Ecore_WinCE_Window Ecore_WinCE_Window;
#endif

#ifndef _ECORE_EVAS_PRIVATE_H
/* basic data types */
typedef struct _Ecore_Evas Ecore_Evas;
#endif

/* module setup/shutdown calls */

EAPI int         ecore_evas_engine_type_supported_get(Ecore_Evas_Engine_Type engine);

/**
 * @brief Init the Ecore_Evas system.
 *
 * @return How many times the lib has been initialized, 0 indicates failure.
 *
 * Set up the Evas wrapper system. Init Evas and Ecore libraries.
 *
 * @see ecore_evas_shutdown()
 */
EAPI int         ecore_evas_init(void);
/**
 * @brief Shut down the Ecore_Evas system.
 *
 * @return 0 if ecore evas is fully shut down, or > 0 if it still being used.
 *
 * This closes the Evas wrapper system down. Shut down Evas and Ecore libraries.
 *
 * @see ecore_evas_init()
 */
EAPI int         ecore_evas_shutdown(void);

EAPI void        ecore_evas_app_comp_sync_set(Eina_Bool do_sync);
EAPI Eina_Bool   ecore_evas_app_comp_sync_get(void);

/**
 * @brief Returns a list of supported engines names.
 *
 * @return Newly allocated list with engines names. Engines names
 * strings are internal and should be considered constants, do not
 * free or modify them, to free the list use ecore_evas_engines_free().
 */
EAPI Eina_List  *ecore_evas_engines_get(void);
/**
 * @brief Free list returned by ecore_evas_engines_get()
 */
EAPI void        ecore_evas_engines_free(Eina_List *engines);
/**
 * @brief Creates a new Ecore_Evas based on engine name and common parameters.
 *
 * @param engine_name engine name as returned by
 *        ecore_evas_engines_get() or NULL to use environment variable
 *        ECORE_EVAS_ENGINE, that can be undefined and in this case
 *        this call will try to find the first working engine.
 * @param x horizontal position of window (not supported in all engines)
 * @param y vertical position of window (not supported in all engines)
 * @param w width of window
 * @param h height of window
 * @param extra_options string with extra parameter, dependent on engines
 *        or NULL. String is usually in the form: 'key1=value1;key2=value2'.
 *        Pay attention that when getting that from shell commands, most
 *        consider ';' as the command terminator, so you need to escape
 *        it or use quotes.
 *
 * @return Ecore_Evas instance or NULL if creation failed.
 */
EAPI Ecore_Evas *ecore_evas_new(const char *engine_name, int x, int y, int w, int h, const char *extra_options);


/* engine/target specific init calls */
EAPI Ecore_Evas     *ecore_evas_software_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window  ecore_evas_software_x11_window_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool       ecore_evas_software_x11_direct_resize_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);

#define ECORE_EVAS_GL_X11_OPT_NONE     0
#define ECORE_EVAS_GL_X11_OPT_INDIRECT 1
#define ECORE_EVAS_GL_X11_OPT_VSYNC    2
#define ECORE_EVAS_GL_X11_OPT_LAST     3

EAPI Ecore_Evas     *ecore_evas_gl_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_Evas     *ecore_evas_gl_x11_options_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h, const int *opt);
EAPI Ecore_X_Window  ecore_evas_gl_x11_window_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_gl_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool       ecore_evas_gl_x11_direct_resize_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_gl_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);
EAPI void            ecore_evas_gl_x11_pre_post_swap_callback_set(const Ecore_Evas *ee, void *data, void (*pre_cb) (void *data, Evas *e), void (*post_cb) (void *data, Evas *e));

EAPI Ecore_Evas     *ecore_evas_xrender_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window  ecore_evas_xrender_x11_window_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_xrender_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool       ecore_evas_xrender_x11_direct_resize_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_xrender_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);

EAPI Ecore_Evas     *ecore_evas_software_x11_8_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window  ecore_evas_software_x11_8_window_get(const Ecore_Evas *ee);
EAPI Ecore_X_Window  ecore_evas_software_x11_8_subwindow_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_8_direct_resize_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool       ecore_evas_software_x11_8_direct_resize_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_8_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);

EAPI Ecore_Evas     *ecore_evas_software_x11_16_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window  ecore_evas_software_x11_16_window_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_16_direct_resize_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool       ecore_evas_software_x11_16_direct_resize_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_16_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);

EAPI Ecore_Evas     *ecore_evas_fb_new(const char *disp_name, int rotation, int w, int h);

EAPI Ecore_Evas     *ecore_evas_directfb_new(const char *disp_name, int windowed, int x, int y, int w, int h);
EAPI Ecore_DirectFB_Window *ecore_evas_directfb_window_get(const Ecore_Evas *ee);

EAPI Ecore_Evas     *ecore_evas_buffer_new(int w, int h);
EAPI Ecore_Evas     *ecore_evas_buffer_allocfunc_new(int w, int h, void *(*alloc_func) (void *data, int size), void (*free_func) (void *data, void *pix), const void *data);
EAPI const void     *ecore_evas_buffer_pixels_get(Ecore_Evas *ee);

EAPI Evas_Object    *ecore_evas_object_image_new(Ecore_Evas *ee_target);
EAPI Ecore_Evas     *ecore_evas_object_ecore_evas_get(Evas_Object *obj);
EAPI Evas           *ecore_evas_object_evas_get(Evas_Object *obj);

EAPI Ecore_Evas     *ecore_evas_software_gdi_new(Ecore_Win32_Window *parent,
                                                 int                 x,
                                                 int                 y,
                                                 int                 width,
                                                 int                 height);

EAPI Ecore_Evas     *ecore_evas_software_ddraw_new(Ecore_Win32_Window *parent,
                                                   int                 x,
                                                   int                 y,
                                                   int                 width,
                                                   int                 height);

EAPI Ecore_Evas     *ecore_evas_software_16_ddraw_new(Ecore_Win32_Window *parent,
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
EAPI Ecore_Evas     *ecore_evas_gl_sdl_new(const char* name, int w, int h, int fullscreen, int noframe);

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

EAPI Ecore_Evas *ecore_evas_cocoa_new(const char* name, int w, int h);

/* generic manipulation calls */
/**
 * @brief Get the engine name used by this Ecore_Evas(window).
 *
 * @param ee Ecore_Evas whose engine's name is desired.
 * @return A string that can(usually) be used in ecore_evas_new()
 *
 * @see ecore_evas_free()
 */
EAPI const char *ecore_evas_engine_name_get(const Ecore_Evas *ee);
/**
 * @brief Return the Ecore_Evas for this Evas
 *
 * @param e The Evas to get the Ecore_Evas from
 * @return The Ecore_Evas that holds this Evas, or NULL if not held by one.
 *
 * @warning Only use on Evas' created with ecore evas!
 */
EAPI Ecore_Evas *ecore_evas_ecore_evas_get(const Evas *e);
/**
 * @brief Free an Ecore_Evas
 *
 * @param ee The Ecore_Evas to free
 *
 * This frees up any memory used by the Ecore_Evas.
 */
EAPI void        ecore_evas_free(Ecore_Evas *ee);
/**
 * @brief Retrieve user data associated with an Ecore_Evas.
 *
 * @param ee The Ecore_Evas to retrieve the user data from.
 * @param key The key which the user data to be retrieved is associated with.
 *
 * This function retrieves user specific data that has been stored within an
 * Ecore_Evas structure with ecore_evas_data_set().
 *
 * @returns NULL on error or no data found, A pointer to the user data on
 *     success.
 *
 * @see ecore_evas_data_set()
 */
EAPI void       *ecore_evas_data_get(const Ecore_Evas *ee, const char *key);
/**
 * @brief Store user data in an Ecore_Evas structure.
 *
 * @param ee The Ecore_Evas to store the user data in.
 * @param key A unique string to associate the user data against. Cannot
 * be NULL.
 * @param data A pointer to the user data to store.
 *
 * This function associates the @p data with a @p key which is stored by
 * the Ecore_Evas @p ee. Be aware that a call to ecore_evas_free() will
 * not free any memory for the associated user data, this is the responsibility
 * of the caller.
 *
 * @see ecore_evas_callback_pre_free_set()
 * @see ecore_evas_free()
 * @see ecore_evas_data_get()
 */
EAPI void        ecore_evas_data_set(Ecore_Evas *ee, const char *key, const void *data);
/**
 * Set a callback for Ecore_Evas resize events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is resized.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_resize_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas move events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is moved.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_move_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas show events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is shown.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_show_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas hide events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is hidden.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_hide_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas delete request events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee gets a delete request.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_delete_request_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas destroy events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is destroyed.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_destroy_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas focus in events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee gets focus.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_focus_in_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas focus out events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee loses focus.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_focus_out_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas sticky events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee becomes sticky.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_sticky_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas un-sticky events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee becomes un-sticky.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_unsticky_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas mouse in events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever the mouse enters @p ee.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_mouse_in_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas mouse out events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever the mouse leaves @p ee.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_mouse_out_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas pre render events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called just before the evas in @p ee is rendered.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_pre_render_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas mouse post render events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called just after the evas in @p ee is rendered.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_post_render_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
/**
 * Set a callback for Ecore_Evas pre-free event.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called just before the instance @p ee is freed.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_pre_free_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
EAPI Evas       *ecore_evas_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_move(Ecore_Evas *ee, int x, int y);
EAPI void        ecore_evas_managed_move(Ecore_Evas *ee, int x, int y);
EAPI void        ecore_evas_resize(Ecore_Evas *ee, int w, int h);
EAPI void        ecore_evas_move_resize(Ecore_Evas *ee, int x, int y, int w, int h);
EAPI void        ecore_evas_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);
EAPI void        ecore_evas_rotation_set(Ecore_Evas *ee, int rot);
EAPI void        ecore_evas_rotation_with_resize_set(Ecore_Evas *ee, int rot);
EAPI int         ecore_evas_rotation_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_shaped_set(Ecore_Evas *ee, Eina_Bool shaped);
EAPI Eina_Bool   ecore_evas_shaped_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_alpha_set(Ecore_Evas *ee, Eina_Bool alpha);
EAPI Eina_Bool   ecore_evas_alpha_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_transparent_set(Ecore_Evas *ee, Eina_Bool transparent);
EAPI Eina_Bool   ecore_evas_transparent_get(const Ecore_Evas *ee);
/**
 * @brief Show an Ecore_Evas' window
 *
 * @param ee The Ecore_Evas to show.
 *
 * This function makes @p ee visible.
 */
EAPI void        ecore_evas_show(Ecore_Evas *ee);
/**
 * @brief Hide an Ecore_Evas' window
 *
 * @param ee The Ecore_Evas to hide.
 *
 * This function makes @p ee hidden(not visible).
 */
EAPI void        ecore_evas_hide(Ecore_Evas *ee);
EAPI int         ecore_evas_visibility_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_raise(Ecore_Evas *ee);
EAPI void        ecore_evas_lower(Ecore_Evas *ee);
EAPI void        ecore_evas_activate(Ecore_Evas *ee);
EAPI void        ecore_evas_title_set(Ecore_Evas *ee, const char *t);
EAPI const char *ecore_evas_title_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_name_class_set(Ecore_Evas *ee, const char *n, const char *c);
EAPI void        ecore_evas_name_class_get(const Ecore_Evas *ee, const char **n, const char **c);

/**
 * Set the minimum size of a given @c Ecore_Evas window
 *
 * @param ee An @c Ecore_Evas window's handle
 * @param w The minimum width
 * @param h The minimum height
 *
 * This function sets the minimum size of @p ee to be @p w x @p h.
 * One won't be able to resize that window to dimensions smaller than
 * the ones set.
 *
 * @note When base sizes are set, via ecore_evas_size_base_set(),
 * they'll be used to calculate a window's minimum size, instead of
 * those set by this function.
 *
 * @see ecore_evas_size_min_get()
 */
EAPI void        ecore_evas_size_min_set(Ecore_Evas *ee, int w, int h);

/**
 * Get the minimum size set for a given @c Ecore_Evas window
 *
 * @param ee An @c Ecore_Evas window's handle
 * @param w A pointer to an int to place the minimum width in.
 * @param h A pointer to an int to place the minimum height in.
 *
 * @note Use @c NULL pointers on the size components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see ecore_evas_size_min_set() for more details
 */
EAPI void        ecore_evas_size_min_get(const Ecore_Evas *ee, int *w, int *h);

/**
 * Set the maximum size of a given @c Ecore_Evas window
 *
 * @param ee An @c Ecore_Evas window's handle
 * @param w The maximum width
 * @param h The maximum height
 *
 * This function sets the maximum size of @p ee to be @p w x @p h.
 * One won't be able to resize that window to dimensions bigger than
 * the ones set.
 *
 * @see ecore_evas_size_max_get()
 */
EAPI void        ecore_evas_size_max_set(Ecore_Evas *ee, int w, int h);

/**
 * Get the maximum size set for a given @c Ecore_Evas window
 *
 * @param ee An @c Ecore_Evas window's handle
 * @param w A pointer to an int to place the maximum width in.
 * @param h A pointer to an int to place the maximum height in.
 *
 * @note Use @c NULL pointers on the size components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see ecore_evas_size_max_set() for more details
 */
EAPI void        ecore_evas_size_max_get(const Ecore_Evas *ee, int *w, int *h);

/**
 * Set the base size for a given @c Ecore_Evas window
 *
 * @param ee An @c Ecore_Evas window's handle
 * @param w The base width
 * @param h The base height
 *
 * This function sets the @b base size of @p ee to be @p w x @p h.
 * When base sizes are set, they'll be used to calculate a window's
 * @b minimum size, instead of those set by ecore_evas_size_min_get().
 *
 * @see ecore_evas_size_base_get()
 */
EAPI void        ecore_evas_size_base_set(Ecore_Evas *ee, int w, int h);

/**
 * Get the base size set for a given @c Ecore_Evas window
 *
 * @param ee An @c Ecore_Evas window's handle
 * @param w A pointer to an int to place the base width in.
 * @param h A pointer to an int to place the base height in.
 *
 * @note Use @c NULL pointers on the size components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see ecore_evas_size_base_set() for more details
 */
EAPI void        ecore_evas_size_base_get(const Ecore_Evas *ee, int *w, int *h);

/**
 * Set the "size step" for a given @c Ecore_Evas window
 *
 * @param ee An @c Ecore_Evas window's handle
 * @param w The step width
 * @param h The step height
 *
 * This function sets the size steps of @p ee to be @p w x @p h. This
 * limits the size of this @cEcore_Evas window to be @b always an
 * integer multiple of the step size, for each axis.
 */
EAPI void        ecore_evas_size_step_set(Ecore_Evas *ee, int w, int h);

/**
 * Get the "size step" set for a given @c Ecore_Evas window
 *
 * @param ee An @c Ecore_Evas window's handle
 * @param w A pointer to an int to place the step width in.
 * @param h A pointer to an int to place the step height in.
 *
 * @note Use @c NULL pointers on the size components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see ecore_evas_size_base_set() for more details
 */
EAPI void        ecore_evas_size_step_get(const Ecore_Evas *ee, int *w, int *h);

/**
 * @brief Set the cursor of an Ecore_Evas.
 *
 * @param ee The Ecore_Evas
 * @param file  The path to an image file for the cursor.
 * @param layer The layer in which the cursor will appear.
 * @param hot_x The x coordinate of the cursor's hot spot.
 * @param hot_y The y coordinate of the cursor's hot spot.
 *
 * This function makes the mouse cursor over @p ee be the image specified by
 * @p file. The actual point within the image that the mouse is at is specified
 * by @p hot_x and @p hot_y, which are coordinates with respect to the top left
 * corner of the cursor image.
 *
 * @note This function creates an object from the image and uses
 * ecore_evas_object_cursor_set().
 *
 * @see ecore_evas_object_cursor_set()
 */
EAPI void        ecore_evas_cursor_set(Ecore_Evas *ee, const char *file, int layer, int hot_x, int hot_y);
/**
 * @brief Get information about an Ecore_Evas' cursor
 *
 * @param ee The Ecore_Evas to set
 * @param obj A pointer to an Evas_Object to place the cursor Evas_Object.
 * @param layer A pointer to an int to place the cursor's layer in.
 * @param hot_x A pointer to an int to place the cursor's hot_x coordinate in.
 * @param hot_y A pointer to an int to place the cursor's hot_y coordinate in.
 *
 * This function queries information about an Ecore_Evas' cursor.
 *
 * @see ecore_evas_cursor_set()
 * @see ecore_evas_object_cursor_set()
 */
EAPI void        ecore_evas_cursor_get(const Ecore_Evas *ee, Evas_Object **obj, int *layer, int *hot_x, int *hot_y);
/**
 * @brief Set the cursor of an Ecore_Evas
 *
 * @param ee The Ecore_Evas
 *
 * @param obj The Evas_Object which will be the cursor.
 * @param layer The layer in which the cursor will appear.
 * @param hot_x The x coordinate of the cursor's hot spot.
 * @param hot_y The y coordinate of the cursor's hot spot.
 *
 * This function makes the mouse cursor over @p ee be the object specified by
 * @p obj. The actual point within the object that the mouse is at is specified
 * by @p hot_x and @p hot_y, which are coordinates with respect to the top left
 * corner of the cursor object.
 *
 * @see ecore_evas_cursor_set()
 */
EAPI void        ecore_evas_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y);
EAPI void        ecore_evas_layer_set(Ecore_Evas *ee, int layer);
EAPI int         ecore_evas_layer_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_focus_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool   ecore_evas_focus_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_iconified_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool   ecore_evas_iconified_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_borderless_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool   ecore_evas_borderless_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_override_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool   ecore_evas_override_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_maximized_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool   ecore_evas_maximized_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_fullscreen_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool   ecore_evas_fullscreen_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_avoid_damage_set(Ecore_Evas *ee, Ecore_Evas_Avoid_Damage_Type on);
EAPI Ecore_Evas_Avoid_Damage_Type ecore_evas_avoid_damage_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_withdrawn_set(Ecore_Evas *ee, Eina_Bool withdrawn);
EAPI Eina_Bool   ecore_evas_withdrawn_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_sticky_set(Ecore_Evas *ee, Eina_Bool sticky);
EAPI Eina_Bool   ecore_evas_sticky_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_ignore_events_set(Ecore_Evas *ee, Eina_Bool ignore);
EAPI Eina_Bool   ecore_evas_ignore_events_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_manual_render_set(Ecore_Evas *ee, Eina_Bool manual_render);
EAPI Eina_Bool   ecore_evas_manual_render_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_manual_render(Ecore_Evas *ee);
EAPI void        ecore_evas_comp_sync_set(Ecore_Evas *ee, Eina_Bool do_sync);
EAPI Eina_Bool   ecore_evas_comp_sync_get(const Ecore_Evas *ee);
       
EAPI Ecore_Window ecore_evas_window_get(const Ecore_Evas *ee);

EAPI void        ecore_evas_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);

/**
 * @brief Associate the given object to this ecore evas.
 *
 * @param ee The Ecore_Evas to associate to @a obj
 * @param obj The object to associate to @a ee
 * @param flags The association flags.
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * Association means that operations on one will affect the other, for
 * example moving the object will move the window, resize the object will
 * also affect the ecore evas window, hide and show applies as well.
 *
 * This is meant to simplify development, since you often need to associate
 * these events with your "base" objects, background or bottom-most object.
 *
 * Be aware that some methods might not be what you would like, deleting
 * either the window or the object will delete the other. If you want to
 * change that behavior, let's say to hide window when it's closed, you
 * must use ecore_evas_callback_delete_request_set() and set your own code,
 * like ecore_evas_hide(). Just remember that if you override delete_request
 * and still want to delete the window/object, you must do that yourself.
 *
 * Since we now define delete_request, deleting windows will not quit
 * main loop, if you wish to do so, you should listen for EVAS_CALLBACK_FREE
 * on the object, that way you get notified and you can call
 * ecore_main_loop_quit().
 *
 * Flags can be OR'ed of:
 * @li ECORE_EVAS_OBJECT_ASSOCIATE_BASE (or 0): to listen to basic events
 *     like delete, resize and move, but no stacking or layer are used.
 * @li ECORE_EVAS_OBJECT_ASSOCIATE_STACK: stacking operations will act
 *     on the Ecore_Evas, not the object. So evas_object_raise() will
 *     call ecore_evas_raise(). Relative operations (stack_above, stack_below)
 *     are still not implemented.
 * @li ECORE_EVAS_OBJECT_ASSOCIATE_LAYER: stacking operations will act
 *     on the Ecore_Evas, not the object. So evas_object_layer_set() will
 *     call ecore_evas_layer_set().
 * @li ECORE_EVAS_OBJECT_ASSOCIATE_DEL: the object delete will delete the
 *     ecore_evas as well as delete_requests on the ecore_evas will delete
 *     etc.
 */
EAPI Eina_Bool    ecore_evas_object_associate(Ecore_Evas *ee, Evas_Object *obj, Ecore_Evas_Object_Associate_Flags flags);
/**
 * @brief Cancel the association set with ecore_evas_object_associate().
 *
 * @param ee The Ecore_Evas to dissociate from @a obj
 * @param obj The object to dissociate from @a ee
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool    ecore_evas_object_dissociate(Ecore_Evas *ee, Evas_Object *obj);
/**
 * @brief Get the object associated with @p ee
 *
 * @param ee The Ecore_Evas to get the object from.
 * @return The associated object, or NULL if there is no associated object.
 */
EAPI Evas_Object *ecore_evas_object_associate_get(const Ecore_Evas *ee);

/* helper function to be used with ECORE_GETOPT_CALLBACK_*() */
EAPI unsigned char ecore_getopt_callback_ecore_evas_list_engines(const Ecore_Getopt *parser, const Ecore_Getopt_Desc *desc, const char *str, void *data, Ecore_Getopt_Value *storage);

/**
 * @brief Get a list of all the ecore_evases.
 *
 * @return A list of ecore_evases.
 *
 * The returned list of ecore evases is only valid until the canvases are
 * destroyed (and should not be cached for instance). The list can be freed by
 * just deleting the list.
 */
EAPI Eina_List   *ecore_evas_ecore_evas_list_get(void);

/* specific calls to an x11 environment ecore_evas */
EAPI void           ecore_evas_x11_leader_set(Ecore_Evas *ee, Ecore_X_Window win);
EAPI Ecore_X_Window ecore_evas_x11_leader_get(Ecore_Evas *ee);
EAPI void           ecore_evas_x11_leader_default_set(Ecore_Evas *ee);
EAPI void           ecore_evas_x11_shape_input_rectangle_set(Ecore_Evas *ee, int x, int y, int w, int h);
EAPI void           ecore_evas_x11_shape_input_rectangle_add(Ecore_Evas *ee, int x, int y, int w, int h);
EAPI void           ecore_evas_x11_shape_input_rectangle_subtract(Ecore_Evas *ee, int x, int y, int w, int h);
EAPI void           ecore_evas_x11_shape_input_empty(Ecore_Evas *ee);
EAPI void           ecore_evas_x11_shape_input_reset(Ecore_Evas *ee);
EAPI void           ecore_evas_x11_shape_input_apply(Ecore_Evas *ee);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
