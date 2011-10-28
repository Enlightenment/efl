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
 * - @ref Ecore_Evas_Buffer_Example_01_c
 * - @ref Ecore_Evas_Buffer_Example_02_c
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
#define HAVE_ECORE_EVAS_EWS 1

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
   ECORE_EVAS_ENGINE_OPENGL_SDL,
   ECORE_EVAS_ENGINE_EWS
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
/**
 * @brief Set whether an Ecore_Evas has an alpha channel or not.
 *
 * @param ee The Ecore_Evas to shape
 * @param alpha EINA_TRUE to enable the alpha channel, EINA_FALSE to disable it
 *
 * This function allows you to make an Ecore_Evas translucent using an
 * alpha channel. See ecore_evas_shaped_set() for details. The difference
 * between a shaped window and a window with an alpha channel is that an
 * alpha channel supports multiple levels of transpararency, as opposed to
 * the 1 bit transparency of a shaped window (a pixel is either opaque, or
 * it's transparent).
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_alpha_set(Ecore_Evas *ee, Eina_Bool alpha);
/**
 * @brief Query whether an Ecore_Evas has an alpha channel.
 * @param ee The Ecore_Evas to query.
 * @return EINA_TRUE if ee has an alpha channel, EINA_FALSE if it does not.
 *
 * This function returns EINA_TRUE if @p ee has an alpha channel, and EINA_FALSE
 * if it does not.
 *
 * @see ecore_evas_alpha_set()
 */
EAPI Eina_Bool   ecore_evas_alpha_get(const Ecore_Evas *ee);
/**
 * @brief Set whether an Ecore_Evas has an transparent window or not.
 *
 * @param ee The Ecore_Evas to shape
 * @param transparent EINA_TRUE to enable the transparent window, EINA_FALSE to
 * disable it
 *
 * This function sets some translucency options, for more complete support see
 * ecore_evas_alpha_set().
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @see ecore_evas_alpha_set()
 */
EAPI void        ecore_evas_transparent_set(Ecore_Evas *ee, Eina_Bool transparent);
/**
 * @brief  Query whether an Ecore_Evas is transparent.
 *
 * @param ee The Ecore_Evas to query.
 * @return EINA_TRUE if ee is transparent, EINA_FALSE if it isn't.
 *
 * @see ecore_evas_transparent_set()
 */
EAPI Eina_Bool   ecore_evas_transparent_get(const Ecore_Evas *ee);
/**
 * @brief  Get the geometry of an Ecore_Evas.
 *
 * @param ee The Ecore_Evas whose geometry y
 * @param x A pointer to an int to place the x coordinate in
 * @param y A pointer to an int to place the y coordinate in
 * @param w A pointer to an int to place the w size in
 * @param h A pointer to an int to place the h size in
 *
 * This function takes four pointers to (already allocated) ints, and places
 * the geometry of @p ee in them. If any of the parameters is not desired you
 * may pass NULL on them.
 *
 * @code
 * int x, y, w, h;
 * ecore_evas_geometry_get(ee, &x, &y, &w, &h);
 * @endcode
 *
 * @see ecore_evas_new()
 * @see ecore_evas_resize()
 * @see ecore_evas_move()
 * @see ecore_evas_move_resize()
 */
EAPI void        ecore_evas_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);
/**
 * @brief  Get the geometry which an Ecore_Evas was latest recently requested.
 *
 * @param ee The Ecore_Evas whose geometry y
 * @param x A pointer to an int to place the x coordinate in
 * @param y A pointer to an int to place the y coordinate in
 * @param w A pointer to an int to place the w size in
 * @param h A pointer to an int to place the h size in
 *
 * This function takes four pointers to (already allocated) ints, and places
 * the geometry which @p ee was latest recently requested . If any of the parameters is not desired you
 * may pass NULL on them. 
 * This function can represent recently requested geomety. 
 * ecore_evas_geometry_get function returns the value is updated after engine finished request.
 * By comparison, ecore_evas_request_geometry_get returns recently requested value.
 *
 * @code
 * int x, y, w, h;
 * ecore_evas_request_geometry_get(ee, &x, &y, &w, &h);
 * @endcode
 *
 */
EAPI void        ecore_evas_request_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);
/**
 * @brief Set the focus of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas
 * @param on EINA_TRUE for focus, EINA_FALSE to defocus.
 *
 * This function focuses @p ee if @p on is EINA_TRUE, or unfocuses @p ee if @p
 * on is EINA_FALSE.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_focus_set(Ecore_Evas *ee, Eina_Bool on);
/**
 * @brief Query whether an Ecore_Evas' window is focused or not.
 *
 * @param ee The Ecore_Evas to set
 * @return EINA_TRUE if @p ee if focused, EINA_FALSE if not.
 *
 * @see ecore_evas_focus_set()
 */
EAPI Eina_Bool   ecore_evas_focus_get(const Ecore_Evas *ee);
/**
 * @brief Iconify or uniconify an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas
 * @param on EINA_TRUE to iconify, EINA_FALSE to uniconify.
 *
 * This function iconifies @p ee if @p on is EINA_TRUE, or uniconifies @p ee if
 * @p on is EINA_FALSE.
 *
 * @note Iconify and minize are synonims.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_iconified_set(Ecore_Evas *ee, Eina_Bool on);
/**
 * @brief Query whether an Ecore_Evas' window is iconified or not.
 *
 * @param ee The Ecore_Evas to set
 * @return EINA_TRUE if @p ee is iconified, EINA_FALSE if not.
 *
 * @note Iconify and minize are synonims.
 *
 * @see ecore_evas_iconified_set()
 */
EAPI Eina_Bool   ecore_evas_iconified_get(const Ecore_Evas *ee);
/**
 * @brief Set whether an Ecore_Evas' window is borderless or not.
 *
 * @param ee The Ecore_Evas
 * @param on EINA_TRUE for borderless, EINA_FALSE for bordered.
 *
 * This function makes @p ee borderless if @p on is EINA_TRUE, or bordered if @p
 * on is EINA_FALSE.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_borderless_set(Ecore_Evas *ee, Eina_Bool on);
/**
 * @brief Query whether an Ecore_Evas' window is borderless or not.
 *
 * @param ee The Ecore_Evas to set
 * @return EINA_TRUE if @p ee is borderless, EINA_FALSE if not.
 *
 * @see ecore_evas_borderless_set()
 */
EAPI Eina_Bool   ecore_evas_borderless_get(const Ecore_Evas *ee);
/**
 * @brief Set whether or not an Ecore_Evas' window is fullscreen.
 *
 * @param ee The Ecore_Evas
 * @param on EINA_TRUE fullscreen, EINA_FALSE not.
 *
 * This function causes @p ee to be fullscreen if @p on is EINA_TRUE,
 * or not if @p on is EINA_FALSE.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_fullscreen_set(Ecore_Evas *ee, Eina_Bool on);
/**
 * @brief Query whether an Ecore_Evas' window is fullscreen or not.
 *
 * @param ee The Ecore_Evas to set
 * @return EINA_TRUE if @p ee is fullscreen, EINA_FALSE if not.
 *
 * @see ecore_evas_fullscreen_set()
 */
EAPI Eina_Bool   ecore_evas_fullscreen_get(const Ecore_Evas *ee);
/**
 * @brief Set if this evas should ignore @b all events.
 *
 * @param ee The Ecore_Evas whose window's to ignore events.
 * @param ignore The Ecore_Evas new ignore state.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_ignore_events_set(Ecore_Evas *ee, Eina_Bool ignore);
/**
 * @brief Returns the ignore state of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas whose window's ignore events state is returned.
 * @return The Ecore_Evas window's ignore state.
 *
 * @see ecore_evas_ignore_events_set()
 */
EAPI Eina_Bool   ecore_evas_ignore_events_get(const Ecore_Evas *ee);
/**
 * @brief Query whether an Ecore_Evas' window is visible or not.
 *
 * @param ee The Ecore_Evas to query.
 * @return 1 if visible, 0 if not.
 *
 * This function queries @p ee and returns 1 if it is visible, and 0 if not.
 *
 * @see ecore_evas_show()
 * @see ecore_evas_hide()
 */
EAPI int         ecore_evas_visibility_get(const Ecore_Evas *ee);
/**
 * @brief Set the layer of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas
 * @param layer The layer to put @p ee on.
 *
 * This function moves @p ee to the layer @p layer.
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @see ecore_evas_lower()
 * @see ecore_evas_raise()
 */
EAPI void        ecore_evas_layer_set(Ecore_Evas *ee, int layer);
/**
 * @brief Get the layer of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas to set
 * @return the layer @p ee's window is on.
 *
 * @see ecore_evas_layer_set()
 * @see ecore_evas_lower()
 * @see ecore_evas_raise()
 */
EAPI int         ecore_evas_layer_get(const Ecore_Evas *ee);
/**
 * @brief Maximize (or unmaximize) an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas
 * @param on EINA_TRUE to maximize, EINA_FALSE to unmaximize.
 *
 * This function maximizes @p ee if @p on is EINA_TRUE, or unmaximizes @p ee
 * if @p on is EINA_FALSE.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_maximized_set(Ecore_Evas *ee, Eina_Bool on);
/**
 * @brief Query whether an Ecore_Evas' window is maximized or not.
 *
 * @param ee The Ecore_Evas to set
 * @return EINA_TRUE if @p ee is maximized, EINA_FALSE if not.
 *
 * @see ecore_evas_maximized_set()
 */
EAPI Eina_Bool   ecore_evas_maximized_get(const Ecore_Evas *ee);
/**
 * @brief Move an Ecore_Evas.
 *
 * @param ee The Ecore_Evas to move
 * @param x The x coordinate to move to
 * @param y The y coordinate to move to
 *
 * This moves @p ee to the screen coordinates (@p x, @p y)
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @see ecore_evas_new()
 * @see ecore_evas_resize()
 * @see ecore_evas_move_resize()
 */
EAPI void        ecore_evas_move(Ecore_Evas *ee, int x, int y);
/**
 * @brief Resize an Ecore_Evas.
 *
 * @param ee The Ecore_Evas to move
 * @param w The w coordinate to resize to
 * @param h The h coordinate to resize to
 *
 * This resizes @p ee to @p w x @p h.
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @see ecore_evas_new()
 * @see ecore_evas_move()
 * @see ecore_evas_move_resize()
 */
EAPI void        ecore_evas_resize(Ecore_Evas *ee, int w, int h);
/**
 * @brief Move and resize an Ecore_Evas
 *
 * @param ee The Ecore_Evas to move and resize
 * @param x The x coordinate to move to
 * @param y The y coordinate to move to
 * @param w The w coordinate to resize to
 * @param h The h coordinate to resize to
 *
 * This moves @p ee to the screen coordinates (@p x, @p y) and  resizes
 * it to @p w x @p h.
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @see ecore_evas_new()
 * @see ecore_evas_move()
 * @see ecore_evas_resize()
 */
EAPI void        ecore_evas_move_resize(Ecore_Evas *ee, int x, int y, int w, int h);
/**
 * @brief Set the rotation of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas
 * @param rot the angle (in degrees) of rotation.
 *
 * The allowed values of @p rot depend on the engine being used. Most only
 * allow multiples of 90.
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @see ecore_evas_rotation_with_resize_set()
 */
EAPI void        ecore_evas_rotation_set(Ecore_Evas *ee, int rot);
/**
 * @brief Set the rotation of an Ecore_Evas' window
 *
 * @param ee The Ecore_Evas
 * @param rot the angle (in degrees) of rotation.
 *
 * Like ecore_evas_rotation_set(), but it also resizes the window's contents so
 * that they fit inside the current window geometry.
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @see ecore_evas_rotation_set()
 */
EAPI void        ecore_evas_rotation_with_resize_set(Ecore_Evas *ee, int rot);
/**
 * @brief Get the rotation of an Ecore_Evas' window
 *
 * @param ee The Ecore_Evas
 * @return the angle (in degrees) of rotation.
 *
 * @see ecore_evas_rotation_set()
 * @see ecore_evas_rotation_with_resize_set()
 */
EAPI int         ecore_evas_rotation_get(const Ecore_Evas *ee);
/**
 * @brief Raise an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas to raise.
 *
 * This functions raises the Ecore_Evas to the front.
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @see ecore_evas_lower()
 * @see ecore_evas_layer_set()
 */
EAPI void        ecore_evas_raise(Ecore_Evas *ee);
/**
 * @brief Lower an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas to raise.
 *
 * This functions lowers the Ecore_Evas to the back.
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @see ecore_evas_raise()
 * @see ecore_evas_layer_set()
 */
EAPI void        ecore_evas_lower(Ecore_Evas *ee);
/**
 * @brief Set the title of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas whose title you wish to set.
 * @param t The title
 *
 * This function sets the title of @p ee to @p t.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_title_set(Ecore_Evas *ee, const char *t);
/**
 * @brief Get the title of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas whose title you wish to get.
 * @return The title of @p ee.
 *
 * This function returns the title of @p ee.
 *
 * @see ecore_evas_title_set()
 */
EAPI const char *ecore_evas_title_get(const Ecore_Evas *ee);
/**
 * @brief Set the name and class of an Ecore_Evas' window.
 *
 * @param ee the Ecore_Evas
 * @param n the name
 * @param c the class
 *
 * This function sets the name of @p ee to @p n, and its class to @p c. The
 * meaning of @p name and @p class depends on the underlying windowing system.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_name_class_set(Ecore_Evas *ee, const char *n, const char *c);
/**
 * @brief Get the name and class of an Ecore_Evas' window
 *
 * @p ee The Ecore_Evas to query
 * @p n A pointer to a string to place the name in.
 * @p c A pointer to a string to place the class in.
 *
 * This function gets the name of @p ee into @p n, and its class into
 * @p c.
 *
 * @see ecore_evas_name_class_set()
 */
EAPI void        ecore_evas_name_class_get(const Ecore_Evas *ee, const char **n, const char **c);
/**
 * @brief Returns a pointer to the underlying window.
 *
 * @param ee The Ecore_Evas whose window is desired.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI Ecore_Window ecore_evas_window_get(const Ecore_Evas *ee);


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

/**
 * @brief Create a new @c Ecore_Evas canvas bound to the Evas
 * @b buffer engine
 *
 * @param w The width of the canvas, in pixels
 * @param h The height of the canvas, in pixels
 * @return A new @c Ecore_Evas instance or @c NULL, on failure
 *
 * This creates a new buffer canvas wrapper, with image data array
 * @b bound to the ARGB format, 8 bits per pixel.
 *
 * This function will allocate the needed pixels array with canonical
 * @c malloc(). If you wish a custom function to allocate it, consider
 * using ecore_evas_buffer_allocfunc_new(), instead.
 *
 * @note This function actually is a wrapper on
 * ecore_evas_buffer_allocfunc_new(), using the same @a w and @a h
 * arguments and canonical @c malloc() and @c free() to the memory
 * allocation and freeing functions. See that function's documentation
 * for more details.
 */
EAPI Ecore_Evas     *ecore_evas_buffer_new(int w, int h);

/**
 * @brief Create a new @c Ecore_Evas canvas bound to the Evas
 * @b buffer engine, giving custom allocation and freeing functions for
 * the canvas memory region
 *
 * @param w The width of the canvas, in canvas units
 * @param h The height of the canvas, in canvas units
 * @param alloc_func Function to be called to allocate the memory
 * needed for the new buffer canvas. @a data will be passed the same
 * value as the @p data of this function, while @a size will be passed
 * @p w times @p h times @c sizeof(int).
 * @param free_func Function to be called to free the memory used by
 * the new buffer canvas. @a data will be passed the same value as the
 * @p data of this function, while @a pix will be passed the canvas
 * memory pointer.
 * @param data Custom data to be passed to the allocation and freeing
 * functions
 * @return A new @c Ecore_Evas instance or @c NULL, on failure
 *
 * This creates a new buffer canvas wrapper, with image data array
 * @b bound to the ARGB format, 8 bits per pixel.
 *
 * This function is useful when one wants an @c Ecore_Evas buffer
 * canvas with a custom allocation function, like one getting memory
 * chunks from a memory pool, for example.
 *
 * On any resizing of this @c Ecore_Evas buffer canvas, its image data
 * will be @b freed, to be allocated again with the new size.
 *
 * @note @p w and @p h sizes have to greater or equal to 1. Otherwise,
 * they'll be interpreted as 1, exactly.
 *
 * @see ecore_evas_buffer_new()
 */
EAPI Ecore_Evas     *ecore_evas_buffer_allocfunc_new(int w, int h, void *(*alloc_func) (void *data, int size), void (*free_func) (void *data, void *pix), const void *data);

/**
 * @brief Grab a pointer to the actual pixels array of a given
 * @c Ecore_Evas @b buffer canvas/window.
 *
 * @param ee An @c Ecore_Evas handle
 * @return A pointer to the internal pixels array of @p ee
 *
 * Besides returning a pointer to the actual pixel array of the given
 * canvas, this call will force a <b>rendering update on @p ee</b>,
 * first.
 *
 * A common use case for this call is to create an image object, from
 * @b another canvas, to have as data @p ee's contents, thus
 * snapshoting the canvas. For that case, one can also use the
 * ecore_evas_object_image_new() helper function.
 */
EAPI const void     *ecore_evas_buffer_pixels_get(Ecore_Evas *ee);

/**
 * @brief Create a new @c Ecore_Evas canvas bound to the Evas
 * @b ews (Ecore + Evas Single Process Windowing System) engine
 *
 * EWS is a simple single process windowing system. The backing store
 * is also an @c Ecore_Evas that can be setup with
 * ecore_evas_ews_setup() and retrieved with
 * ecore_evas_ews_ecore_evas_get(). It will allow window management
 * using events prefixed with @c ECORE_EVAS_EVENT_EWS_.
 *
 * The EWS windows (returned by this function or
 * ecore_evas_new("ews"...)) will all be software buffer windows
 * automatic rendered to the backing store.
 *
 * @param x horizontal position of window, in pixels
 * @param y vertical position of window, in pixels
 * @param w The width of the canvas, in pixels
 * @param h The height of the canvas, in pixels
 * @return A new @c Ecore_Evas instance or @c NULL, on failure
 *
 * @see ecore_evas_ews_setup()
 * @see ecore_evas_ews_ecore_evas_get()
 *
 * @since 1.1
 */
EAPI Ecore_Evas     *ecore_evas_ews_new(int x, int y, int w, int h);


/**
 * Returns the backing store image object that represents the given
 * window in EWS.
 *
 * @note This should not be modified anyhow, but may be helpful to
 *       determine stacking and geometry of it for window managers
 *       that decorate windows.
 *
 * @see ecore_evas_ews_manager_set()
 * @see ecore_evas_ews_evas_get()
 * @since 1.1
 */
EAPI Evas_Object *ecore_evas_ews_backing_store_get(const Ecore_Evas *ee);

/**
 * Calls the window to be deleted (freed), but can let user decide to
 * forbid it by using ecore_evas_callback_delete_request_set()
 *
 * @since 1.1
 */
EAPI void ecore_evas_ews_delete_request(Ecore_Evas *ee);

/**
 * @brief Create an Evas image object with image data <b>bound to an
 * own, internal @c Ecore_Evas canvas wrapper<b>
 *
 * @param ee_target @c Ecore_Evas to have the canvas receiving the new
 * image object
 * @return A handle to the new image object
 *
 * This will create a @b special Evas image object. The image's pixel
 * array will get bound to the same image data array of an @b internal
 * @b buffer @c Ecore_Evas canvas. The user of this function is, then,
 * supposed to grab that @c Ecore_Evas handle, with
 * ecore_evas_object_ecore_evas_get(), and use its canvas to render
 * whichever contents he/she wants, @b independently of the contents
 * of the canvas owned by @p ee_target. Those contents will reflect on
 * the canvas of @p ee, though, being exactly the image data of the
 * object returned by this function.
 *
 * This is a helper function for the scenario of one wanting to grab a
 * buffer canvas' contents (with ecore_evas_buffer_pixels_get()) to be
 * used on another canvas, for whichever reason. The most common goal
 * of this setup is to @b save an image file with a whole canvas as
 * contents, which could not be achieved by using an image file within
 * the target canvas.
 *
 * @warning Always resize the returned image and its underlying
 * @c Ecore_Evas handle accordingly. They must be kept with same sizes
 * for things to work as expected. Also, you @b must issue
 * @c evas_object_image_size_set() on the image with that same size. If
 * the image is to be shown in a canvas bound to an engine different
 * than the buffer one, then you must also set this image's @b fill
 * properties accordingly.
 *
 * @note The image returned will always be bound to the
 * @c EVAS_COLORSPACE_ARGB8888 colorspace, always.
 *
 * @note Use ecore_evas_object_evas_get() to grab the image's internal
 * own canvas directly.
 *
 * @note If snapshoting this image's internal canvas, remember to
 * flush its internal @c Ecore_Evas firstly, with
 * ecore_evas_manual_render().
 */
EAPI Evas_Object    *ecore_evas_object_image_new(Ecore_Evas *ee_target);

/**
 * @brief Retrieve the internal @c Ecore_Evas handle of an image
 * object created via ecore_evas_object_image_new()
 *
 * @param obj A handle to an image object created via
 * ecore_evas_object_image_new()
 * @return The underlying @c Ecore_Evas handle in @p obj
 */
EAPI Ecore_Evas     *ecore_evas_object_ecore_evas_get(Evas_Object *obj);

/**
 * @brief Retrieve the canvas bound to the internal @c Ecore_Evas
 * handle of an image object created via ecore_evas_object_image_new()
 *
 * @param obj A handle to an image object created via
 * ecore_evas_object_image_new()
 * @return A handle to @p obj's underlying @c Ecore_Evas's canvas
 */
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
EAPI void        ecore_evas_managed_move(Ecore_Evas *ee, int x, int y);
EAPI void        ecore_evas_shaped_set(Ecore_Evas *ee, Eina_Bool shaped);
EAPI Eina_Bool   ecore_evas_shaped_get(const Ecore_Evas *ee);
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
EAPI void        ecore_evas_activate(Ecore_Evas *ee);


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
EAPI void        ecore_evas_override_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool   ecore_evas_override_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_avoid_damage_set(Ecore_Evas *ee, Ecore_Evas_Avoid_Damage_Type on);
EAPI Ecore_Evas_Avoid_Damage_Type ecore_evas_avoid_damage_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_withdrawn_set(Ecore_Evas *ee, Eina_Bool withdrawn);
EAPI Eina_Bool   ecore_evas_withdrawn_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_sticky_set(Ecore_Evas *ee, Eina_Bool sticky);
EAPI Eina_Bool   ecore_evas_sticky_get(const Ecore_Evas *ee);
EAPI void        ecore_evas_manual_render_set(Ecore_Evas *ee, Eina_Bool manual_render);
EAPI Eina_Bool   ecore_evas_manual_render_get(const Ecore_Evas *ee);

/**
 * @brief Registers an @c Ecore_Evas to receive events through ecore_input_evas.
 *
 * @param ee The @c Ecore_Evas handle.
 *
 * This function calls ecore_event_window_register() with the @p ee as its @c
 * id argument, @c window argument, and uses its @c Evas too. It is useful when
 * no @c window information is available on a given @c Ecore_Evas backend.
 *
 * @see ecore_evas_input_event_unregister()
 * @since 1.1
 */
EAPI void        ecore_evas_input_event_register(Ecore_Evas *ee);
/**
 * @brief Unregisters an @c Ecore_Evas receiving events through ecore_input_evas.
 *
 * @param ee The @c Ecore_Evas handle.
 *
 * @see ecore_evas_input_event_register()
 * @since 1.1
 */
EAPI void        ecore_evas_input_event_unregister(Ecore_Evas *ee);

/**
 * @brief Force immediate rendering on a given @c Ecore_Evas window
 *
 * @param ee An @c Ecore_Evas handle
 *
 * Use this call to forcefully flush the @p ee's canvas rendering
 * pipeline, thus bring its window to an up to date state.
 */
EAPI void        ecore_evas_manual_render(Ecore_Evas *ee);
EAPI void        ecore_evas_comp_sync_set(Ecore_Evas *ee, Eina_Bool do_sync);
EAPI Eina_Bool   ecore_evas_comp_sync_get(const Ecore_Evas *ee);
/**
 * @brief Get geometry of screen associated with this Ecore_Evas.
 *
 * @param ee The Ecore_Evas whose window's to query container screen geometry.
 * @param x where to return the horizontal offset value. May be NULL.
 * @param y where to return the vertical offset value. May be NULL.
 * @param w where to return the width value. May be NULL.
 * @param h where to return the height value. May be NULL.
 *
 * @since 1.1
 */
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
 * @defgroup Ecore_Evas_Ews Ecore_Evas Single Process Windowing System.
 *
 * These are global scope functions to manage the EWS to be used by
 * ecore_evas_ews_new().
 *
 * @since 1.1
 * @{
 */

/**
 * Sets the engine to be used by the backing store engine.
 *
 * @return EINA_TRUE on success, EINA_FALSE if ews is already in use.
 * @since 1.1
 */
EAPI Eina_Bool ecore_evas_ews_engine_set(const char *engine, const char *options);

/**
 * Reconfigure the backing store used.
 * @since 1.1
 */
EAPI Eina_Bool ecore_evas_ews_setup(int x, int y, int w, int h);

/**
 * Return the internal backing store in use.
 *
 * @note this will foced it to be created, making future calls to
 * ecore_evas_ews_engine_set() void.
 *
 * @see ecore_evas_ews_evas_get()
 * @since 1.1
 */
EAPI Ecore_Evas *ecore_evas_ews_ecore_evas_get(void);

/**
 * Return the internal backing store in use.
 *
 * @note this will foced it to be created, making future calls to
 * ecore_evas_ews_engine_set() void.
 *
 * @see ecore_evas_ews_ecore_evas_get()
 * @since 1.1
 */
EAPI Evas *ecore_evas_ews_evas_get(void);

/**
 * Get the current background.
 */
EAPI Evas_Object *ecore_evas_ews_background_get(void);

/**
 * Set the current background, must be created at evas ecore_evas_ews_evas_get()
 *
 * It will be kept at lowest layer (EVAS_LAYER_MIN) and below
 * everything else. You can set any object, default is a black
 * rectangle.
 *
 * @note previous object will be deleted!
 */
EAPI void ecore_evas_ews_background_set(Evas_Object *o);

/**
 * Return all Ecore_Evas* created by EWS.
 *
 * @note do not change the returned list or its contents.
 * @since 1.1
 */
EAPI const Eina_List *ecore_evas_ews_children_get(void);

/**
 * Set the identifier of the manager taking care of internal windows.
 *
 * The ECORE_EVAS_EWS_EVENT_MANAGER_CHANGE event is issued. Consider
 * handling it to know if you should stop handling events yourself
 * (ie: another manager took over)
 *
 * @param manager any unique identifier address.
 *
 * @see ecore_evas_ews_manager_get()
 * @since 1.1
 */
EAPI void        ecore_evas_ews_manager_set(const void *manager);

/**
 * Get the identifier of the manager taking care of internal windows.
 *
 * @return the value set by ecore_evas_ews_manager_set()
 * @since 1.1
 */
EAPI const void *ecore_evas_ews_manager_get(void);

EAPI extern int ECORE_EVAS_EWS_EVENT_MANAGER_CHANGE; /**< manager was changed */
EAPI extern int ECORE_EVAS_EWS_EVENT_ADD; /**< window was created */
EAPI extern int ECORE_EVAS_EWS_EVENT_DEL; /**< window was deleted, pointer is already invalid but may be used as reference for further cleanup work. */
EAPI extern int ECORE_EVAS_EWS_EVENT_RESIZE; /**< window was resized */
EAPI extern int ECORE_EVAS_EWS_EVENT_MOVE; /**< window was moved */
EAPI extern int ECORE_EVAS_EWS_EVENT_SHOW; /**< window become visible */
EAPI extern int ECORE_EVAS_EWS_EVENT_HIDE; /**< window become hidden */
EAPI extern int ECORE_EVAS_EWS_EVENT_FOCUS; /**< window was focused */
EAPI extern int ECORE_EVAS_EWS_EVENT_UNFOCUS; /**< window lost focus */
EAPI extern int ECORE_EVAS_EWS_EVENT_RAISE; /**< window was raised */
EAPI extern int ECORE_EVAS_EWS_EVENT_LOWER; /**< window was lowered */
EAPI extern int ECORE_EVAS_EWS_EVENT_ACTIVATE; /**< window was activated */

EAPI extern int ECORE_EVAS_EWS_EVENT_ICONIFIED_CHANGE; /**< window minimized/iconified changed */
EAPI extern int ECORE_EVAS_EWS_EVENT_MAXIMIZED_CHANGE; /**< window maximized changed */
EAPI extern int ECORE_EVAS_EWS_EVENT_LAYER_CHANGE; /**< window layer changed */
EAPI extern int ECORE_EVAS_EWS_EVENT_FULLSCREEN_CHANGE; /**< window fullscreen changed */
EAPI extern int ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE; /**< some other window property changed (title, name, class, alpha, transparent, shaped...) */

/**
 * @}
 */


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
