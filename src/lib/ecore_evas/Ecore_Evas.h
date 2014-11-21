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
 * @internal
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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @internal
 * @defgroup Ecore_Evas_Group Ecore_Evas wrapper/helper set of functions
 * @ingroup Ecore_Group
 *
 * Ecore evas is a set of functions that makes it easy to tie together ecore's
 * main loop and input handling to evas. As such it is a natural base for EFL
 * applications. While this combination makes it easy to create the basic
 * aspects all applications need, for normal applications (ones with buttons,
 * checkboxes and layouts) one should consider using Elementary.
 *
 * Ecore evas is extremely well suited for applications that are not based on
 * widgets. It has a main loop that delivers events, does basic window handling
 * and leaves all of the drawing up to the user. This works very well if used
 * in conjunction with Edje or if doing custom drawing as, for example, is done
 * in games.
 *
 * @{
 */

/* These are dummy and just tells you what API levels ecore_evas supports - not 
 * the actual support is compiled in. You need to query for that separately.
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
#define HAVE_ECORE_EVAS_PSL1GHT 1
#define HAVE_ECORE_EVAS_WAYLAND_SHM 1
#define HAVE_ECORE_EVAS_WAYLAND_EGL 1

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
   ECORE_EVAS_ENGINE_OPENGL_COCOA,
   ECORE_EVAS_ENGINE_SOFTWARE_SDL,
   ECORE_EVAS_ENGINE_DIRECTFB,
   ECORE_EVAS_ENGINE_SOFTWARE_FB,
   ECORE_EVAS_ENGINE_SOFTWARE_8_X11,
   ECORE_EVAS_ENGINE_SOFTWARE_16_X11,
   ECORE_EVAS_ENGINE_SOFTWARE_16_DDRAW,
   ECORE_EVAS_ENGINE_SOFTWARE_16_WINCE,
   ECORE_EVAS_ENGINE_OPENGL_SDL,
   ECORE_EVAS_ENGINE_EWS,
   ECORE_EVAS_ENGINE_PSL1GHT,
   ECORE_EVAS_ENGINE_WAYLAND_SHM,
   ECORE_EVAS_ENGINE_WAYLAND_EGL
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
typedef unsigned int Ecore_X_Pixmap;
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

#ifndef __ECORE_COCOA_H__
typedef struct _Ecore_Cocoa_Window Ecore_Cocoa_Window;
#endif

#ifndef _ECORE_EVAS_PRIVATE_H
/* Basic data types */
typedef struct _Ecore_Evas Ecore_Evas;
typedef void   (*Ecore_Evas_Event_Cb) (Ecore_Evas *ee); /**< Callback used for several ecore evas events @since 1.2 */
#endif

#ifndef _ECORE_WAYLAND_H_
#define _ECORE_WAYLAND_WINDOW_PREDEF
typedef struct _Ecore_Wl_Window Ecore_Wl_Window;
#endif

/* module setup/shutdown calls */

EAPI int         ecore_evas_engine_type_supported_get(Ecore_Evas_Engine_Type engine);

/**
 * @brief   Initializes the Ecore_Evas system.
 *
 * @details This function sets up the Evas wrapper system - initializes Evas and Ecore libraries.
 *
 * @return  The number of time the lib has been initialized, \n
 *          otherwise @c 0 on failure
 *
 * @see ecore_evas_shutdown()
 */
EAPI int         ecore_evas_init(void);

/**
 * @brief   Shuts down the Ecore_Evas system.
 *
 * @details This function closes the Evas wrapper system down - shuts down Evas and Ecore libraries.
 *
 * @return  @c 0 if ecore evas is fully shut down, \n
 *          otherwise > @c 0 if it still being used
 *
 * @see ecore_evas_init()
 */
EAPI int         ecore_evas_shutdown(void);

EAPI void        ecore_evas_app_comp_sync_set(Eina_Bool do_sync);
EAPI Eina_Bool   ecore_evas_app_comp_sync_get(void);

/**
 * @brief   Gets a list of supported engines names.
 *
 * @return  The newly allocated list with engines names \n 
 *          Engines names strings are internal and should be 
 *          considered constants, do not free or modify them, 
 *          to free the list use ecore_evas_engines_free().
 */
EAPI Eina_List  *ecore_evas_engines_get(void);

/**
 * @brief  Frees the list returned by ecore_evas_engines_get().
 *
 * @param[in]  engines  The list with engines names
 */
EAPI void        ecore_evas_engines_free(Eina_List *engines);

/**
 * @brief   Creates a new Ecore_Evas based on engine name and common parameters.
 *
 * @param[in]   engine_name    The engine name as returned by ecore_evas_engines_get(), \n 
 *                         otherwise set @c NULL to use environment variable @a ECORE_EVAS_ENGINE \n 
 *                         This can be undefined and in this case this call tries
 *                         to find the first working engine.
 * @param[in]   x              The horizontal position of window (not supported in all engines)
 * @param[in]   y              The vertical position of window (not supported in all engines)
 * @param[in]   w              The width of window
 * @param[in]   h              The height of window
 * @param[in]   extra_options  The string with extra parameter, dependent on engines or @ NULL \n 
 *                         The string is usually in the form: 'key1=value1;key2=value2'.
 *                         Pay attention that when getting that from shell commands, most
 *                         consider ';' as the command terminator, so you need to escape
 *                         it or use quotes.
 * @return  The Ecore_Evas instance, \n 
 *          otherwise @c NULL on failure
 */
EAPI Ecore_Evas *ecore_evas_new(const char *engine_name, int x, int y, int w, int h, const char *extra_options);

/**
 * @brief   Sets whether an Ecore_Evas has an alpha channel or not.
 *
 * @details This function allows you to make an Ecore_Evas translucent using an
 *          alpha channel. See ecore_evas_shaped_set() for details. The difference
 *          between a shaped window and a window with an alpha channel is that an
 *          alpha channel supports multiple levels of transparency, as opposed to
 *          the @c 1 bit transparency of a shaped window (a pixel is either opaque, or
 *          it is transparent).
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee     The Ecore_Evas to shape
 * @param[in]   alpha  Set @c EINA_TRUE to enable the alpha channel, \n
 *                 otherwise @c EINA_FALSE to disable it
 */
EAPI void        ecore_evas_alpha_set(Ecore_Evas *ee, Eina_Bool alpha);

/**
 * @brief   Checks whether an Ecore_Evas has an alpha channel.
 *
 * @remarks This function returns @c EINA_TRUE if @a ee has an alpha channel, and
 *          @c EINA_FALSE if it does not.
 *
 * @param[in]   ee  The Ecore_Evas to query
 * @return  @c EINA_TRUE if @a ee has an alpha channel, \n
 *          otherwise @c EINA_FALSE if it does not
 *
 * @see ecore_evas_alpha_set()
 */
EAPI Eina_Bool   ecore_evas_alpha_get(const Ecore_Evas *ee);

/**
 * @brief   Sets whether an Ecore_Evas has an transparent window or not.
 *
 * @details This function sets some translucency options.
 *
 * @remarks For more complete support see ecore_evas_alpha_set().
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee           The Ecore_Evas to shape
 * @param[in]   transparent  @c EINA_TRUE to enable the transparent window, \n
 *                       otherwise @c EINA_FALSE to disable it
 *
 * @see ecore_evas_alpha_set()
 */
EAPI void        ecore_evas_transparent_set(Ecore_Evas *ee, Eina_Bool transparent);

/**
 * @brief   Checks whether an Ecore_Evas is transparent.
 *
 * @param[in]   ee  The Ecore_Evas to query
 * @return  @c EINA_TRUE if ee is transparent, \n
 *          otherwise @c EINA_FALSE if it is not
 *
 * @see ecore_evas_transparent_set()
 */
EAPI Eina_Bool   ecore_evas_transparent_get(const Ecore_Evas *ee);

/**
 * @brief   Gets the geometry of an Ecore_Evas.
 *
 * @details This function takes four pointers to (already allocated) int, and places
 *          the geometry of @a ee in them. If any of the parameters is not desired, you
 *          may pass @c NULL on them.
 *
 * @code
 * int x, y, w, h;
 * ecore_evas_geometry_get(ee, &x, &y, &w, &h);
 * @endcode
 *
 * @param[in]   ee  The Ecore_Evas whose geometry y
 * @param[out]   x   A pointer to an int to place the x coordinate in
 * @param[out]   y   A pointer to an int to place the y coordinate in
 * @param[out]   w   A pointer to an int to place the w size in
 * @param[out]   h   A pointer to an int to place the h size in
 *
 * @see ecore_evas_new()
 * @see ecore_evas_resize()
 * @see ecore_evas_move()
 * @see ecore_evas_move_resize()
 */
EAPI void        ecore_evas_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);

/**
 * @brief   Gets the geometry recently requested by an Ecore_Evas.
 *
 * @details This function takes four pointers to (already allocated) ints, and places
 *          the geometry which @a ee recently requested. If any of the
 *          parameters is not desired, you may pass @c NULL on them.
 *          This function can represent the recently requested geometry.
 *          The ecore_evas_geometry_get function returns the value that is updated after engine
 *          finished the request. By comparison, ecore_evas_request_geometry_get returns
 *          the recently requested value.
 * @since   1.1
 *
 * @code
 * int x, y, w, h;
 * ecore_evas_request_geometry_get(ee, &x, &y, &w, &h);
 * @endcode
 *
 * @param[in]  ee  The Ecore_Evas whose geometry y
 * @param[out]  x   A pointer to an int to place the x coordinate in
 * @param[out]  y   A pointer to an int to place the y coordinate in
 * @param[out]  w   A pointer to an int to place the w size in
 * @param[out]  h   A pointer to an int to place the h size in
 */
EAPI void        ecore_evas_request_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);

/**
 * @brief   Sets the focus of an Ecore_Evas' window.
 *
 * @details This function focuses @a ee if @a on is @c EINA_TRUE, 
 *          or unfocuses @a ee if @a on is @c EINA_FALSE.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas
 * @param[in]   on  Set @c EINA_TRUE for focus, \n
 *              otherwise set @c EINA_FALSE to defocus
 */
EAPI void        ecore_evas_focus_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * @brief   Checks whether an Ecore_Evas' window is focused.
 *
 * @param[in]   ee  The Ecore_Evas to check
 * @return  @c EINA_TRUE if @a ee is focused, \n
 *          otherwise @c EINA_FALSE if it is not focused.
 *
 * @see ecore_evas_focus_set()
 */
EAPI Eina_Bool   ecore_evas_focus_get(const Ecore_Evas *ee);

/**
 * @brief   Iconifies or uniconifies an Ecore_Evas' window.
 *
 * @details This function iconifies @a ee if @a on is @c EINA_TRUE, or uniconifies @a ee
 *          if @a on is @c EINA_FALSE.
 *
 * @remarks Iconify and minimize are synonyms.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas
 * @param[in]   on  Set @c EINA_TRUE to iconify, \n
 *              otherwise set @c EINA_FALSE to uniconify
 */
EAPI void        ecore_evas_iconified_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * @brief   Checks whether an Ecore_Evas' window is iconified.
 *
 * @remarks Iconify and minimize are synonyms.
 *
 * @param[in]   ee  The Ecore_Evas to check
 * @return  @c EINA_TRUE if @a ee is iconified, \n
 *          otherwise @c EINA_FALSE if it is not iconified
 *
 * @see ecore_evas_iconified_set()
 */
EAPI Eina_Bool   ecore_evas_iconified_get(const Ecore_Evas *ee);

/**
 * @brief   Sets whether an Ecore_Evas' window is borderless or not.
 *
 * @details This function makes @a ee borderless if @a on is @c EINA_TRUE, or bordered
 *          if @a on is @c EINA_FALSE.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas
 * @param[in]   on  Set @c EINA_TRUE for borderless, \n
 *              otherwise set @c EINA_FALSE for bordered
 */
EAPI void        ecore_evas_borderless_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * @brief   Checks whether an Ecore_Evas' window is borderless.
 *
 * @param[in]   ee  The Ecore_Evas to check
 * @return  @c EINA_TRUE if @a ee is borderless, \n
 *          otherwise @c EINA_FALSE if it is not borderless
 *
 * @see ecore_evas_borderless_set()
 */
EAPI Eina_Bool   ecore_evas_borderless_get(const Ecore_Evas *ee);

/**
 * @brief   Sets whether or not an Ecore_Evas' window is fullscreen.
 *
 * @details This function causes @a ee to be fullscreen if @a on is @c EINA_TRUE, and
 *          not to be fullscreen if @a on is @c EINA_FALSE.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas
 * @param[in]   on  Set @c EINA_TRUE for fullscreen, \n
 *              otherwise set @c EINA_FALSE
 */
EAPI void        ecore_evas_fullscreen_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * @brief   Checks whether an Ecore_Evas' window is fullscreen.
 *
 * @param[in]   ee  The Ecore_Evas to check
 * @return  @c EINA_TRUE if @a ee is fullscreen, \n
 *          otherwise @c EINA_FALSE if it is not fullscreen
 *
 * @see ecore_evas_fullscreen_set()
 */
EAPI Eina_Bool   ecore_evas_fullscreen_get(const Ecore_Evas *ee);

/**
 * @brief   Sets another window that this window is a group member of.
 * @since   1.2
 *
 * @remarks If @a ee_group is @c NULL, @a ee is removed from the group, otherwise it is
 *          added. Note that if you free the @a ee_group canvas before @a ee, then
 *          getting the group canvas with ecore_evas_window_group_get() returns
 *          an invalid handle.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee        The Ecore_Evas
 * @param[in]   ee_group  The other group member
 */
EAPI void        ecore_evas_window_group_set(Ecore_Evas *ee, const Ecore_Evas *ee_group);

/**
 * @brief   Gets the canvas group that is set.
 * @details This returns the handle set by ecore_evas_window_group_set().
 *
 * @since   1.2
 *
 * @param[in]   ee  The Ecore_Evas
 * @return  The Canvas group handle
 *
 * @see ecore_evas_window_group_set()
 */
EAPI const Ecore_Evas *ecore_evas_window_group_get(const Ecore_Evas *ee);

/**
 * @brief   Sets the aspect ratio of a canvas window.
 *
 * @details This function sets the desired aspect ratio of a canvas window.
 * @since   1.2
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee      The Ecore_Evas
 * @param[in]   aspect  The aspect ratio (width divided by height), \n
 *                  otherwise set @c 0 to disable
 */
EAPI void        ecore_evas_aspect_set(Ecore_Evas *ee, double aspect);

/**
 * @brief   Gets the aspect ratio of a canvas window.
 *
 * @details This function returns the value set by ecore_evas_aspect_set().
 * @since   1.2
 *
 * @param[in]   ee  The Ecore_Evas to set
 * @return  The aspect ratio
 *
 * @see ecore_evas_aspect_set()
 */
EAPI double      ecore_evas_aspect_get(const Ecore_Evas *ee);

/**
 * @brief   Sets the urgent hint flag.
 *
 * @details This function sets the "urgent" state hint on a window so that the desktop environment
 *          can highlight it somehow.
 * @since   1.2
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee      The Ecore_Evas
 * @param[in]   urgent  Set @c EINA_TRUE to set the urgent state flag, \n
 *                  otherwise set @c EINA_FALSE to not set it
 */
EAPI void        ecore_evas_urgent_set(Ecore_Evas *ee, Eina_Bool urgent);

/**
 * @brief   Gets the urgent state on the canvas window.
 *
 * @details This returns the value set by ecore_evas_urgent_set().
 * @since   1.2
 *
 * @param[in]   ee  The Ecore_Evas to set
 * @return  @c EINA_TRUE if the urgent state is set, \n
 *          otherwise @c EINA_FALSE if it is not set
 *
 * @see ecore_evas_urgent_set()
 */
EAPI Eina_Bool   ecore_evas_urgent_get(const Ecore_Evas *ee);

/**
 * @brief   Sets the modal state flag on the canvas window.
 *
 * @details This function hints if the window should be modal (for example, if it is also transient
 *          for another window, the other window maybe be denied focus by
 *          the desktop window manager).
 * @since   1.2
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee     The Ecore_Evas
 * @param[in]   modal  Set @c EINA_TRUE to set the window as modal, \n 
 *                 otherwise set @c EINA_FALSE to set the window as not modal
 */
EAPI void        ecore_evas_modal_set(Ecore_Evas *ee, Eina_Bool modal);

/**
 * @brief   Checks whether the modal flag is set.
 *
 * @details This returns the value set by ecore_evas_modal_set().
 * @since   1.2
 *
 * @param[in]   ee  The Ecore_Evas
 * @return  @c EINA_TRUE if the window is modal, \n
 *          otherwise @c EINA_FALSE if the window is not modal
 *
 * @see ecore_evas_modal_set()
 */
EAPI Eina_Bool   ecore_evas_modal_get(const Ecore_Evas *ee);

/**
 * @brief   Sets the "i demand attention" flag on a canvas window.
 * @since   1.2
 *
 * @remarks A window may demand attention (for example, you must enter a password before
 *          continuing), and so you may flag a window with this function.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee      The Ecore_Evas
 * @param[in]   demand  Set @c EINA_TRUE to set the flag, \n
 *                  otherwise set @c EINA_FALSE to not set the flag
 */
EAPI void        ecore_evas_demand_attention_set(Ecore_Evas *ee, Eina_Bool demand);

/**
 * @brief   Checks whether the "i demand attention" flag is set.
 *
 * @details This function returns the value set by ecore_evas_demand_attention_set().
 * @since   1.2
 *
 * @param[in]   ee  The Ecore_Evas
 * @return  @c EINA_TRUE if the "i demand attention" flag is set, \n
 *          otherwise @c EINA_FALSE if the flag is not set
 *
 * @see ecore_evas_demand_attention_set()
 */
EAPI Eina_Bool   ecore_evas_demand_attention_get(const Ecore_Evas *ee);

/**
 * @brief   Sets the "focus skip" flag.
 * @since   1.2
 *
 * @remarks A window may not want to accept focus, be in the taskbar or pager
 *          sometimes (example for a small notification window that hovers around
 *          a taskbar or panel, or hovers around a window until some activity
 *          dismisses it).
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee    The Ecore_Evas
 * @param[in]   skip  Set @c EINA_TRUE to set the "focus skip", \n
 *                otherwise @c EINA_FALSE to not set the flag
 */
EAPI void        ecore_evas_focus_skip_set(Ecore_Evas *ee, Eina_Bool skip);

/**
 * @brief   Checks whether the "focus skip" flag is set.
 *
 * @details This returns the value set by ecore_evas_focus_skip_set().
 * @since   1.2
 *
 * @param[in]   ee  The Ecore_Evas to set
 * @return  @c EINA_TRUE if the "focus skip" flag is set, \n
 *          otherwise @c EINA_FALSE if the flag is not set
 *
 * @see ecore_evas_focus_skip_set()
 */
EAPI Eina_Bool   ecore_evas_focus_skip_get(const Ecore_Evas *ee);

/**
 * @brief   Sets whether this evas should ignore @b all events.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee      The Ecore_Evas
 * @param[in]   ignore  Set @c EINA_TRUE for Ecore_Evas to ignore events, \n
 *                  otherwise @c EINA_FALSE to not ignore the events
 */
EAPI void        ecore_evas_ignore_events_set(Ecore_Evas *ee, Eina_Bool ignore);

/**
 * @brief   Checks whether the ignore state of the Ecore_Evas window is set.
 *
 * @param[in]   ee  The Ecore_Evas
 * @return  @c EINA_TRUE if ignore state is set, \n
 *          otherwise @c EINA_FALSE if the ignore state is not set
 *
 * @see ecore_evas_ignore_events_set()
 */
EAPI Eina_Bool   ecore_evas_ignore_events_get(const Ecore_Evas *ee);

/**
 * @brief   Checks whether an Ecore_Evas window is visible.
 *
 * @details This function queries @a ee and returns @c 1 if it is visible, 
 *          and @c 0 if it is not visible.
 *
 * @param[in]   ee  The Ecore_Evas to query
 * @return  @c 1 if the window visible, \n
 *          otherwise @c 0 if it is not visible
 *
 * @see ecore_evas_show()
 * @see ecore_evas_hide()
 */
EAPI int         ecore_evas_visibility_get(const Ecore_Evas *ee);

/**
 * @brief   Sets the layer of an Ecore_Evas window.
 *
 * @details This function moves @a ee to the layer @a layer.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee     The Ecore_Evas
 * @param[in]   layer  The layer to put @a ee on
 *
 * @see ecore_evas_lower()
 * @see ecore_evas_raise()
 */
EAPI void        ecore_evas_layer_set(Ecore_Evas *ee, int layer);

/**
 * @brief   Gets the layer of an Ecore_Evas window.
 *
 * @param[in]   ee  The Ecore_Evas to set
 * @return  The layer @a ee's window is on
 *
 * @see ecore_evas_layer_set()
 * @see ecore_evas_lower()
 * @see ecore_evas_raise()
 */
EAPI int         ecore_evas_layer_get(const Ecore_Evas *ee);

/**
 * @brief   Maximizes (or unmaximizes) an Ecore_Evas window.
 *
 * @details This function maximizes @a ee if @a on is @c EINA_TRUE, 
 *          or unmaximizes @a ee if @a on is @c EINA_FALSE.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas
 * @param[in]   on  Set @c EINA_TRUE to maximize, \n
 *              otherwise set @c EINA_FALSE to unmaximize
 */
EAPI void        ecore_evas_maximized_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * @brief   Checks whether an Ecore_Evas window is maximized.
 *
 * @param[in]   ee  The Ecore_Evas to set
 * @return  @c EINA_TRUE if @a ee is maximized, \n
 *          otherwise @c EINA_FALSE if is not maximized
 *
 * @see ecore_evas_maximized_set()
 */
EAPI Eina_Bool   ecore_evas_maximized_get(const Ecore_Evas *ee);

/**
 * @brief   Sets the Ecore_Evas window profile list.
 * @since   1.7.0
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee            The Ecore_Evas
 * @param[in]   profiles      The profile name list
 * @param[in]   num_profiles  The number of profile names
 */
EAPI void        ecore_evas_profiles_set(Ecore_Evas *ee, const char **profiles, unsigned int num_profiles);

/**
 * @brief   Gets the Ecore_Evas window profile name.
 * @since   1.7.0
 *
 * @param[in]   ee  The Ecore_Evas
 * @return  The profile name
 */
EAPI const char *ecore_evas_profile_get(const Ecore_Evas *ee);

EAPI Eina_Bool        ecore_evas_wm_rotation_supported_get(const Ecore_Evas *ee);
EAPI void             ecore_evas_wm_rotation_preferred_rotation_set(Ecore_Evas *ee, int rotation);
EAPI int              ecore_evas_wm_rotation_preferred_rotation_get(const Ecore_Evas *ee);
EAPI void             ecore_evas_wm_rotation_available_rotations_set(Ecore_Evas *ee, const int *rotations, unsigned int count);
EAPI Eina_Bool        ecore_evas_wm_rotation_available_rotations_get(const Ecore_Evas *ee, int **rotations, unsigned int *count);

/**
 * @brief   Sets manual rotation done mode for Ecore_Evas window.
 * @since   1.8.0
 *
 * @param[in]   ee   The Ecore_Evas
 * @param[in]   set  Set @c EINA_TRUE if the window manager should not rotate the Ecore_Evas's window until
 *               the rotation done event is received by ecore_evas_wm_rotation_manual_rotation_done, \n
 *               otherwise set @c EINA_FALSE if the manual rotation mode should be disabled
 *
 */
EAPI void             ecore_evas_wm_rotation_manual_rotation_done_set(Ecore_Evas *ee, Eina_Bool set);
/**
 * @brief   Gets manual rotation done mode of Ecore_Evas's window.
 * @since   1.8.0
 *
 * @param[in]   ee  The Ecore_Evas
 * @return  @c EINA_TRUE if the manual rotation done mode is enabled, \n
 *          otherwise @c EINA_FALSE if it is not enabled
 */
EAPI Eina_Bool        ecore_evas_wm_rotation_manual_rotation_done_get(const Ecore_Evas *ee);
/**
 * @brief   Sets the rotation finish manually.
 * @since   1.8.0
 *
 * @param[in]   ee  The Ecore_Evas
 *
 */
EAPI void             ecore_evas_wm_rotation_manual_rotation_done(Ecore_Evas *ee);

/**
 * @brief   Gets the list of supported auxiliary hint strings.
 * @since   1.8.0
 *
 * @remarks Do not change the returned list of its contents. Auxiliary hint
 *          strings are internal and should be considered constants, do not free or
 *          modify them.
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @remarks The window auxiliary hint is the value which is used to decide which actions should
 *          be made available to the user by the window manager. If you want to set specific hint
 *          to your window, then you should check whether it exists in the supported auxiliary
 *          hints that are registered in the root window by the window manager. Once you have added
 *          an auxiliary hint, you can get a new ID which is used to change value and delete hint.
 *          The window manager sends the response message to the application on receiving auxiliary
 *          hint change event. A list of auxiliary hint within the Ecore_Evas has this format:
 *          ID:HINT:VALUE,ID:HINT:VALUE,...
 *
 * @param[in]   ee  The Ecore_Evas
 * @return  The list of supported auxiliary hint strings
 */
EAPI const Eina_List *ecore_evas_aux_hints_supported_get(const Ecore_Evas *ee);

/**
 * @brief   Gets the list of allowed auxiliary hint ID.
 *
 * @since   1.8.0
 *
 * @remarks This function is low level. Instead of using it directly, consider
 *          using the callback mechanism in Elementary such as "aux,hint,allowed".
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas
 * @return  The list of allowed auxiliary hint ID
 */
EAPI Eina_List       *ecore_evas_aux_hints_allowed_get(const Ecore_Evas *ee);

/**
 * @brief   Creates an auxiliary hint of the Ecore_Evas.
 *
 * @since   1.8.0
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee    The Ecore_Evas
 * @param[in]   hint  The auxiliary hint string
 * @param[in]   val   The value string
 * @return  The ID of created auxiliary hint, \n
 *          otherwise @c -1 on failure
 */
EAPI int              ecore_evas_aux_hint_add(Ecore_Evas *ee, const char *hint, const char *val);

/**
 * @brief   Deletes an auxiliary hint of the Ecore_Evas.
 *
 * @since   1.8.0
 *
 * @remarks Support for this depends on the underlying windowing system.
 * @param[in]   ee  The Ecore_Evas
 * @param[in]   id  The ID of the auxiliary hint.
 * @return  @c EINA_TRUE if the hint is deleted successfully, \n
 *          otherwise @c EINA_FALSE in case of errors
 */
EAPI Eina_Bool        ecore_evas_aux_hint_del(Ecore_Evas *ee, const int id);

/**
 * @brief   Changes a value of the auxiliary hint.
 *
 * @since   1.8.0
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee   The Ecore_Evas
 * @param[in]   id   The auxiliary hint ID
 * @param[in]   val  The value string to be set
 * @return  @c EINA_TRUE if the value is changed successfully, \n
 *          otherwise @c EINA_FALSE in case of errors
 */
EAPI Eina_Bool        ecore_evas_aux_hint_val_set(Ecore_Evas *ee, const int id, const char *val);

/**
 * @brief   Sends message to parent ecore.
 * @since   1.8.0
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee          The Ecore_Evas to set
 * @param[in]   msg_domain  The domain of message
 * @param[in]   msg_id      The ID of message
 * @param[in]   data        The data of message
 * @param[in]   size        The size of message data
 *
 * @see ecore_evas_msg_send()
 * @see ecore_evas_callback_msg_parent_handle_set()
 * @see eecore_evas_callback_msg_handle_set()
 */
EAPI void ecore_evas_msg_parent_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size);

/**
 * @brief   Sends message to child ecore.
 * @since   1.8.0
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee          The Ecore_Evas to set
 * @param[in]   msg_domain  The domain of message
 * @param[in]   msg_id      The ID of message
 * @param[in]   data        The data of message
 * @param[in]   size        The size of message data
 *
 * @see ecore_evas_msg_parent_send()
 * @see ecore_evas_callback_msg_parent_handle_set()
 * @see eecore_evas_callback_msg_handle_set()
 */
EAPI void ecore_evas_msg_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size);

/**
 * @brief   Sets a callback for parent Ecore_Evas message.
 * @since   1.8.0
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee                  The Ecore_Evas to set callbacks on
 * @param[in]   func_parent_handle  The handle to be called when message arrives
 *
 * @see ecore_evas_msg_parent_send()
 * @see ecore_evas_msg_send()
 * @see eecore_evas_callback_msg_handle_set()
 */
EAPI void ecore_evas_callback_msg_parent_handle_set(Ecore_Evas *ee, void (*func_parent_handle)(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size));

/**
 * @brief   Sets a callback for child Ecore_Evas message.
 * @since   1.8.0
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee           The Ecore_Evas to set callbacks on
 * @param[in]   func_handle  The handle to be called when message arrives
 *
 * @see ecore_evas_msg_parent_send()
 * @see ecore_evas_msg_send()
 * @see ecore_evas_callback_msg_parent_handle_set()
 */
EAPI void ecore_evas_callback_msg_handle_set(Ecore_Evas *ee, void (*func_handle)(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size));

/**
 * @brief   Moves an Ecore_Evas.
 *
 * @details This function moves @a ee to the screen coordinates (@a x, @a y).
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas to move
 * @param[in]   x   The x coordinate to move to
 * @param[in]   y   The y coordinate to move to
 *
 * @see ecore_evas_new()
 * @see ecore_evas_resize()
 * @see ecore_evas_move_resize()
 */
EAPI void        ecore_evas_move(Ecore_Evas *ee, int x, int y);
/**
 * @brief   Resizes an Ecore_Evas.
 *
 * @details This function resizes @a ee to @a w x @a h.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas to move
 * @param[in]   w   The w coordinate to resize to
 * @param[in]   h   The h coordinate to resize to
 *
 * @see ecore_evas_new()
 * @see ecore_evas_move()
 * @see ecore_evas_move_resize()
 */
EAPI void        ecore_evas_resize(Ecore_Evas *ee, int w, int h);

/**
 * @brief   Moves and resizes an Ecore_Evas.
 *
 * @details This moves @a ee to the screen coordinates (@a x, @a y) and  resizes
 *          it to @a w x @a h.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas to move and resize
 * @param[in]   x   The x coordinate to move to
 * @param[in]   y   The y coordinate to move to
 * @param[in]   w   The w coordinate to resize to
 * @param[in]   h   The h coordinate to resize to
 *
 * @see ecore_evas_new()
 * @see ecore_evas_move()
 * @see ecore_evas_resize()
 */
EAPI void        ecore_evas_move_resize(Ecore_Evas *ee, int x, int y, int w, int h);

/**
 * @brief   Sets the rotation of an Ecore_Evas window.
 *
 * @remarks The allowed values of @a rot depend on the engine being used. Most only
 *          allow multiples of @c 90.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee   The Ecore_Evas
 * @param[in]   rot  The angle (in degrees) of rotation
 *
 * @see ecore_evas_rotation_with_resize_set()
 */
EAPI void        ecore_evas_rotation_set(Ecore_Evas *ee, int rot);

/**
 * @brief   Sets the rotation of an Ecore_Evas window.
 *
 * @remarks Like ecore_evas_rotation_set(), but it also resizes the window's contents so
 *          that they fit inside the current window geometry.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee   The Ecore_Evas
 * @param[in]   rot  The angle (in degrees) of rotation
 *
 * @see ecore_evas_rotation_set()
 */
EAPI void        ecore_evas_rotation_with_resize_set(Ecore_Evas *ee, int rot);

/**
 * @brief   Gets the rotation of an Ecore_Evas window.
 *
 * @param[in]   ee  The Ecore_Evas
 * @return  The angle (in degrees) of rotation
 *
 * @see ecore_evas_rotation_set()
 * @see ecore_evas_rotation_with_resize_set()
 */
EAPI int         ecore_evas_rotation_get(const Ecore_Evas *ee);

/**
 * @brief   Raises an Ecore_Evas window.
 *
 * @details This functions raises the Ecore_Evas to the front.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas to raise
 *
 * @see ecore_evas_lower()
 * @see ecore_evas_layer_set()
 */
EAPI void        ecore_evas_raise(Ecore_Evas *ee);

/**
 * @brief   Lowers an Ecore_Evas window.
 *
 * @details This functions lowers the Ecore_Evas to the back.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas to raise
 *
 * @see ecore_evas_raise()
 * @see ecore_evas_layer_set()
 */
EAPI void        ecore_evas_lower(Ecore_Evas *ee);

/**
 * @brief   Sets the title of an Ecore_Evas window.
 *
 * @details This function sets the title of @a ee to @a t.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas whose title you wish to set
 * @param[in]   t   The title
 */
EAPI void        ecore_evas_title_set(Ecore_Evas *ee, const char *t);

/**
 * @brief   Gets the title of an Ecore_Evas window.
 *
 * @details This function returns the title of @a ee.
 *
 * @param[in]   ee  The Ecore_Evas whose title you wish to get
 * @return  The title of @a ee
 *
 * @see ecore_evas_title_set()
 */
EAPI const char *ecore_evas_title_get(const Ecore_Evas *ee);

/**
 * @brief   Sets the name and class of an Ecore_Evas window.
 *
 * @details This function sets the name of @a ee to @a n, and its class to @a c. The
 *          meaning of @a name and @a class depends on the underlying windowing system.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas
 * @param[in]   n   The name
 * @param[in]   c   The class
 */
EAPI void        ecore_evas_name_class_set(Ecore_Evas *ee, const char *n, const char *c);

/**
 * @brief   Gets the name and class of an Ecore_Evas window.
 *
 * @details This function gets the name of @a ee into @a n, and its class into @a c.
 *
 * @param[in]   ee  The Ecore_Evas to query
 * @param[out]   n   A pointer to a string to place the name in
 * @param[out]   c   A pointer to a string to place the class in
 * @see ecore_evas_name_class_set()
 */
EAPI void        ecore_evas_name_class_get(const Ecore_Evas *ee, const char **n, const char **c);

/**
 * @brief   Gets a pointer to the underlying window.
 *
 * @remarks Support for this depends on the underlying windowing system.
 *
 * @param[in]   ee  The Ecore_Evas
 * @return  A pointer to the underlying window
 */
EAPI Ecore_Window ecore_evas_window_get(const Ecore_Evas *ee);


/* Engine/target specific init calls */
EAPI Ecore_Evas     *ecore_evas_software_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window  ecore_evas_software_x11_window_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool       ecore_evas_software_x11_direct_resize_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_software_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);

/**
 * @brief Create a new Ecore_Evas which does not contain an XWindow. It will
 * only contain an XPixmap to render to
 *
 * @warning The XPixmap ID can change with every frame after it is rendered,
 * so you should ALWAYS call ecore_evas_software_x11_pixmap_get when you
 * need the current pixmap id.
 *
 * @since 1.8
 */
EAPI Ecore_Evas     *ecore_evas_software_x11_pixmap_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);

/**
 * @brief Returns the underlying Ecore_X_Pixmap used in the Ecore_Evas
 *
 * @param[in] disp_name Display name.
 * @param[in] parent X11 parent window.
 * @param[in] x X cooridnate.
 * @param[in] y Y coordinate.
 * @param[in] w Width.
 * @param[in] h Height.
 * @return The underlying Ecore_X_Pixmap
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @warning The XPixmap ID can change with every frame after it is rendered,
 * so you should ALWAYS call ecore_evas_software_x11_pixmap_get when you
 * need the current pixmap id.
 *
 * @since 1.8
 */
EAPI Ecore_Evas * ecore_evas_software_x11_pixmap_new_internal(const char *disp_name, Ecore_X_Window parent,int x, int y, int w, int h);
EAPI Ecore_X_Pixmap ecore_evas_software_x11_pixmap_get(const Ecore_Evas *ee);

#define ECORE_EVAS_GL_X11_OPT_NONE         0
#define ECORE_EVAS_GL_X11_OPT_INDIRECT     1
#define ECORE_EVAS_GL_X11_OPT_VSYNC        2
#define ECORE_EVAS_GL_X11_OPT_SWAP_MODE    3 
#define ECORE_EVAS_GL_X11_OPT_LAST         4

#define ECORE_EVAS_GL_X11_SWAP_MODE_AUTO   0
#define ECORE_EVAS_GL_X11_SWAP_MODE_FULL   1
#define ECORE_EVAS_GL_X11_SWAP_MODE_COPY   2
#define ECORE_EVAS_GL_X11_SWAP_MODE_DOUBLE 3
#define ECORE_EVAS_GL_X11_SWAP_MODE_TRIPLE 4
   
EAPI Ecore_Evas     *ecore_evas_gl_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_Evas     *ecore_evas_gl_x11_options_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h, const int *opt);
EAPI Ecore_X_Window  ecore_evas_gl_x11_window_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_gl_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on);
EAPI Eina_Bool       ecore_evas_gl_x11_direct_resize_get(const Ecore_Evas *ee);
EAPI void            ecore_evas_gl_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);
EAPI void            ecore_evas_gl_x11_pre_post_swap_callback_set(const Ecore_Evas *ee, void *data, void (*pre_cb) (void *data, Evas *e), void (*post_cb) (void *data, Evas *e));

/**
 * @brief Create a new Ecore_Evas which does not contain an XWindow. It will
 * only contain an XPixmap to render to
 *
 * @since 1.8
 *
 * @warning The XPixmap ID can change with every frame after it is rendered,
 * so you should ALWAYS call ecore_evas_software_x11_pixmap_get when you
 * need the current pixmap id.
 *
 * @param[in] disp_name Display name.
 * @param[in] parent X11 parent window.
 * @param[in] x X cooridnate.
 * @param[in] y Y coordinate.
 * @param[in] w Width.
 * @param[in] h Height.
 * @return The underlying Ecore_X_Pixmap
 */
EAPI Ecore_Evas     *ecore_evas_gl_x11_pixmap_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);

/**
 * @brief Returns the underlying Ecore_X_Pixmap used in the Ecore_Evas
 *
 * @param[in] disp_name Display name.
 * @param[in] parent X11 parent window.
 * @param[in] x X cooridnate.
 * @param[in] y Y coordinate.
 * @param[in] w Width.
 * @param[in] h Height.
 * @return The underlying Ecore_X_Pixmap
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @warning The XPixmap ID can change with every frame after it is rendered,
 * so you should ALWAYS call ecore_evas_software_x11_pixmap_get when you
 * need the current pixmap id.
 *
 * @since 1.8
 */
EAPI Ecore_Evas * ecore_evas_gl_x11_pixmap_new_internal(const char *disp_name, Ecore_X_Window parent,int x, int y, int w, int h);
EAPI Ecore_X_Pixmap ecore_evas_gl_x11_pixmap_get(const Ecore_Evas *ee);

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


EAPI Ecore_Evas     *ecore_evas_wayland_shm_new(const char *disp_name, unsigned int parent, int x, int y, int w, int h, Eina_Bool frame);
EAPI Ecore_Evas     *ecore_evas_wayland_egl_new(const char *disp_name, unsigned int parent, int x, int y, int w, int h, Eina_Bool frame);
EAPI void            ecore_evas_wayland_resize(Ecore_Evas *ee, int location);
EAPI void            ecore_evas_wayland_move(Ecore_Evas *ee, int x, int y);

EAPI void            ecore_evas_wayland_pointer_set(Ecore_Evas *ee, int hot_x, int hot_y);
EAPI void            ecore_evas_wayland_type_set(Ecore_Evas *ee, int type);
EAPI Ecore_Wl_Window *ecore_evas_wayland_window_get(const Ecore_Evas *ee);

/**
 * @brief   Creates a new @c Ecore_Evas canvas bound to the Evas @b buffer engine.
 *
 * @details This function creates a new buffer canvas wrapper, with image data array
 *          @b bound to the ARGB format, 8 bits per pixel.
 *
 * @remarks This function allocates the needed pixels array with canonical
 *          @c malloc(). If you wish a custom function to allocate it, consider
 *          using ecore_evas_buffer_allocfunc_new(), instead.
 *
 * @remarks This function actually is a wrapper on
 *          ecore_evas_buffer_allocfunc_new(), using the same @a w and @a h
 *          arguments and canonical @c malloc() and @c free() to the memory
 *          allocation and freeing functions. See the function's documentation
 *          for more details.
 *
 * @param[in]   w  The width of the canvas, in pixels
 * @param[in]   h  The height of the canvas, in pixels
 * @return  A new @c Ecore_Evas instance, \n
 *          otherwise @c NULL on failure
 */
EAPI Ecore_Evas     *ecore_evas_buffer_new(int w, int h);

/**
 * @brief   Creates a new @c Ecore_Evas canvas bound to the Evas
 *          @b buffer engine, giving custom allocation and freeing functions for
 *          the canvas memory region.
 *
 * @details This function creates a new buffer canvas wrapper, with image data array
 *          @b bound to the ARGB format, 8 bits per pixel.
 *
 * @remarks This function is useful when one wants an @c Ecore_Evas buffer
 *          canvas with a custom allocation function, like one getting memory
 *          chunks from a memory pool, for example.
 *
 * @remarks On any resizing of this @c Ecore_Evas buffer canvas, its image data
 *          is @b freed, to be allocated again with the new size.
 *
 * @remarks @a w and @a h sizes have to greater or equal to @c 1. Otherwise,
 *          they are interpreted as @c 1, exactly.
 *
 * @param[in]   w           The width of the canvas, in canvas units
 * @param[in]   h           The height of the canvas, in canvas units
 * @param[in]   alloc_func  The function to be called to allocate the memory
 *                      needed for the new buffer canvas \n 
 *                      @a data is passed the same value as the @a data of this function, 
 *                      while @a size is passed @a w times @a h times @c sizeof(int).
 * @param[in]   free_func   The function to be called to free the memory used by
 *                      the new buffer canvas \n 
 *                      @a data is passed the same value as the @a data of this function, 
 *                      while @a pix is passed the canvas memory pointer.
 * @param[in]   data        The custom data to be passed to the allocation and freeing functions
 * @return  A new @c Ecore_Evas instance, \n
 *          otherwise @c NULL on failure
 *
 * @see ecore_evas_buffer_new()
 */
EAPI Ecore_Evas     *ecore_evas_buffer_allocfunc_new(int w, int h, void *(*alloc_func) (void *data, int size), void (*free_func) (void *data, void *pix), const void *data);

/**
 * @brief   Grabs a pointer to the actual pixels array of a given
 *          @c Ecore_Evas @b buffer canvas/window.
 *
 * @remarks Besides returning a pointer to the actual pixel array of the given
 *          canvas, this call forces a <b>rendering update on @a ee</b>, first.
 *
 * @remarks A common use case for this call is to create an image object, from
 *          @b another canvas, to have as data @a ee's contents, thus taking a 
 *          snapshot of the canvas. For that case, one can also use the
 *          ecore_evas_object_image_new() helper function.
 *
 * @param[in]   ee  An @c Ecore_Evas handle
 * @return  A pointer to the internal pixels array of @a ee
 */
EAPI const void     *ecore_evas_buffer_pixels_get(Ecore_Evas *ee);

/**
 * @brief   Creates a new @c Ecore_Evas canvas bound to the Evas
 *          @b ews (Ecore + Evas Single Process Windowing System) engine.
 *
 * @since   1.1
 *
 * @remarks EWS is a simple single process windowing system. The backing store
 *          is also an @c Ecore_Evas that can be setup with
 *          ecore_evas_ews_setup() and retrieved with
 *          ecore_evas_ews_ecore_evas_get(). It allows window management
 *          using events prefixed with @c ECORE_EVAS_EVENT_EWS_.
 *
 * @remarks The EWS windows (returned by this function or
 *          ecore_evas_new("ews"...)) is all software buffer windows
 *          automatic rendered to the backing store.
 *
 * @param[in]   x  The horizontal position of window, in pixels
 * @param[in]   y  The vertical position of window, in pixels
 * @param[in]   w  The width of the canvas, in pixels
 * @param[in]   h  The height of the canvas, in pixels
 * @return  A new @c Ecore_Evas instance, \n
 *          otherwise @c NULL on failure
 *
 * @see ecore_evas_ews_setup()
 * @see ecore_evas_ews_ecore_evas_get()
 */
EAPI Ecore_Evas     *ecore_evas_ews_new(int x, int y, int w, int h);

/**
 * @brief   Gets the backing store image object that represents the given
 *          window in EWS.
 * @since   1.1
 *
 * @remarks This should not be modified anyhow, but may be helpful to
 *          determine stacking and geometry of it for window managers
 *          that decorate windows.
 *
 * @param[in]   ee  The Ecore_Evas from which to get the backing store
 * @return  The evas object of EWS backing store
 *
 * @see ecore_evas_ews_manager_set()
 * @see ecore_evas_ews_evas_get()
 */
EAPI Evas_Object *ecore_evas_ews_backing_store_get(const Ecore_Evas *ee);

/**
 * @brief   Calls the window to be deleted (freed), but can let user decide to
 *          forbid it by using ecore_evas_callback_delete_request_set()
 * @since   1.1
 *
 * @param[in]   ee  The Ecore_Evas for which window is deleted
 */
EAPI void ecore_evas_ews_delete_request(Ecore_Evas *ee);

/**
 * @brief   Creates an Evas image object with image data <b>bound to an
 *          own, internal @c Ecore_Evas canvas wrapper</b>.
 *
 * @remarks This creates a @b special Evas image object. The image's pixel
 *          array gets bound to the same image data array of an @b internal
 *          @b buffer @c Ecore_Evas canvas. The user of this function is, then,
 *          supposed to grab that @c Ecore_Evas handle, with
 *          ecore_evas_object_ecore_evas_get(), and use its canvas to render
 *          whichever contents he/she wants, @b independently of the contents
 *          of the canvas owned by @a ee_target. Those contents reflect on
 *          the canvas of @a ee, though, being exactly the image data of the
 *          object returned by this function.
 *
 *          This is a helper function for the scenario of one wanting to grab a
 *          buffer canvas' contents (with ecore_evas_buffer_pixels_get()) to be
 *          used on another canvas, for whichever reason. The most common goal
 *          of this setup is to @b save an image file with a whole canvas as
 *          contents, which could not be achieved by using an image file within
 *          the target canvas.
 *
 * @remarks Always resize the returned image and its underlying
 *          @c Ecore_Evas handle accordingly. They must be kept with same sizes
 *          for things to work as expected. Also, you @b must issue
 *          @c evas_object_image_size_set() on the image with that same size. If
 *          the image is to be shown in a canvas bound to an engine different
 *          than the buffer one, then you must also set this image's @b fill
 *          properties accordingly.
 *
 * @remarks The image returned is always bound to the
 *          @c EVAS_COLORSPACE_ARGB8888 colorspace, always.
 *
 * @remarks Use ecore_evas_object_evas_get() to grab the image's internal
 *          own canvas directly.
 *
 * @remarks If you are taking a snapshot of this image's internal canvas, remember to
 *          flush its internal @c Ecore_Evas firstly, with
 *          ecore_evas_manual_render().
 *
 * @param[in]   ee_target  The Ecore_Evas to have the canvas receiving the new image object
 * @return  A handle to the new image object
 */
EAPI Evas_Object    *ecore_evas_object_image_new(Ecore_Evas *ee_target);

/**
 * @brief   Gets the internal @c Ecore_Evas handle of an image
 *          object created using ecore_evas_object_image_new().
 *
 * @param[in]   obj  A handle to an image object created using ecore_evas_object_image_new()
 * @return  The underlying @c Ecore_Evas handle in @a obj
 */
EAPI Ecore_Evas     *ecore_evas_object_ecore_evas_get(Evas_Object *obj);

/**
 * @brief   Gets the canvas bound to the internal @c Ecore_Evas
 *          handle of an image object created using ecore_evas_object_image_new().
 *
 * @param[in]   obj  A handle to an image object created using ecore_evas_object_image_new()
 * @return  A handle to @a obj's underlying @c Ecore_Evas's canvas
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

EAPI Ecore_Evas *ecore_evas_cocoa_new(Ecore_Cocoa_Window *parent,
                      int x,
                      int y,
                      int w,
                      int h);

EAPI Ecore_Evas *ecore_evas_psl1ght_new(const char* name, int w, int h);


/* Generic manipulation calls */
/**
 * @brief   Gets the engine name used by this Ecore_Evas(window).
 *
 * @param[in]   ee  The Ecore_Evas whose engine's name is desired
 * @return  A string that can(usually) be used in ecore_evas_new()
 *
 * @see ecore_evas_free()
 */
EAPI const char *ecore_evas_engine_name_get(const Ecore_Evas *ee);
/**
 * @brief   Gets the Ecore_Evas for this Evas.
 *
 * @remarks Use this only on Evas created with ecore evas.
 *
 * @param[in]   e  The Evas to get the Ecore_Evas from
 * @return  The Ecore_Evas that holds this Evas, \n
 *          otherwise @c NULL if not held by one
 */
EAPI Ecore_Evas *ecore_evas_ecore_evas_get(const Evas *e);
/**
 * @brief   Frees an Ecore_Evas.
 *
 * @details This function frees up any memory used by the Ecore_Evas.
 *
 * @param[in]   ee  The Ecore_Evas to free.
 */
EAPI void        ecore_evas_free(Ecore_Evas *ee);
/**
 * @brief   Gets user data associated with an Ecore_Evas.
 *
 * @details This function retrieves user specific data that has been stored within an
 *          Ecore_Evas structure with ecore_evas_data_set().
 *
 * @return  A pointer to the user data on success, \n
 *          otherwise @c NULL on error
 *
 * @param[in]   ee   The Ecore_Evas to retrieve the user data from
 * @param[in]   key  The key which the user data to be retrieved is associated with
 *
 * @see ecore_evas_data_set()
 */
EAPI void       *ecore_evas_data_get(const Ecore_Evas *ee, const char *key);
/**
 * @brief   Stores user data in an Ecore_Evas structure.
 *
 * @details This function associates the @a data with a @a key which is stored by
 *          the Ecore_Evas @a ee. Be aware that a call to ecore_evas_free() does
 *          not free any memory for the associated user data. This is the responsibility
 *          of the caller.
 *
 * @param[in]   ee    The Ecore_Evas to store the user data in
 * @param[in]   key   A unique string to associate the user data against \n 
 *                This must not be @c NULL.
 * @param[in]   data  A pointer to the user data to store
 *
 * @see ecore_evas_callback_pre_free_set()
 * @see ecore_evas_free()
 * @see ecore_evas_data_get()
 */
EAPI void        ecore_evas_data_set(Ecore_Evas *ee, const char *key, const void *data);
/**
 * @brief   Sets a callback for Ecore_Evas resize events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever @a ee is resized.
 *
 * @remarks If and when this function is called depends on the underlying
 *          windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_resize_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas move events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever @a ee is moved.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_move_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas show events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever @a ee is shown.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_show_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas hide events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever @a ee is hidden.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_hide_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas delete request events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever @a ee gets a delete request.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_delete_request_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas destroy events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever @a ee is destroyed.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_destroy_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas focus in events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever @a ee gets focus.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_focus_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief   Sets a callback for Ecore_Evas focus out events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever @a ee loses focus.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param   ee    The Ecore_Evas to set callbacks on
 * @param   func  The function to call
 */
EAPI void        ecore_evas_callback_focus_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas sticky events.
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever @a ee becomes sticky.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param   ee    The Ecore_Evas to set callbacks on
 * @param   func  The function to call
 */
EAPI void        ecore_evas_callback_sticky_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief   Sets a callback for Ecore_Evas focus out events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever @a ee becomes un-sticky.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call

 */
EAPI void        ecore_evas_callback_unsticky_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas mouse in events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever the mouse enters @a ee.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_mouse_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas mouse out events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever the mouse leaves @a ee.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_mouse_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas pre render events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called just before the evas in @a ee is rendered.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_pre_render_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas mouse post render events.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called just after the evas in @a ee is rendered.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_post_render_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas pre-free event.
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called just before the instance @a ee is freed.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_pre_free_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * @brief   Sets a callback for Ecore_Evas state changes.
 * @since   1.2
 *
 * @remarks A call to this function sets a callback on an Ecore_Evas, causing
 *          @a func to be called whenever @a ee changes state.
 *
 * @remarks When this function is called depends on the underlying windowing system.
 * @param[in]   ee    The Ecore_Evas to set callbacks on
 * @param[in]   func  The function to call
 */
EAPI void        ecore_evas_callback_state_change_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief   Gets an Ecore_Evas's Evas.
 *
 * @details This function returns the Evas contained within @a ee.
 *
 * @param[in]   ee  The Ecore_Evas whose Evas you wish to get
 * @return  The Evas wrapped by @a ee
 */
EAPI Evas       *ecore_evas_get(const Ecore_Evas *ee);

/**
 * @brief   Provides managed move co-ordinates for an Ecore_Evas.
 *
 * @details This sets the managed geometry position of the @a ee to (@a x, @a y).
 *
 * @param[in]   ee  The Ecore_Evas to move
 * @param[in]   x   The x coordinate to set as the managed location
 * @param[in]   y   The y coordinate to set as the managed location
 */
EAPI void        ecore_evas_managed_move(Ecore_Evas *ee, int x, int y);

/**
 * @brief   Sets whether an Ecore_Evas is shaped or not.
 *
 * @remarks This function allows one to make an Ecore_Evas shaped to the contents of the
 *          evas. If @a shaped is @c EINA_TRUE, @a ee is transparent in parts of
 *          the evas that contain no objects. If @a shaped is @c EINA_FALSE, then @a ee
 *          is rectangular, and parts with no data show random framebuffer
 *          artifacting. For non-shaped Ecore_Evases, it is recommended to cover the
 *          entire evas with a background object.
 *
 * @param[in]   ee      The Ecore_Evas to shape
 * @param[in]   shaped  Set @c EINA_TRUE to shape, \n
 *                  otherwise set @c EINA_FALSE to not shape
 */
EAPI void        ecore_evas_shaped_set(Ecore_Evas *ee, Eina_Bool shaped);

/**
 * @brief   Checks whether an Ecore_Evas is shaped.
 *
 * @details This function returns @c EINA_TRUE if @a ee is shaped, and @c EINA_FALSE if not.
 *
 * @param[in]   ee  The Ecore_Evas to query
 * @return  @c EINA_TRUE if shaped, \n
 *          otherwise @c EINA_FALSE if is not shaped
 */
EAPI Eina_Bool   ecore_evas_shaped_get(const Ecore_Evas *ee);
/**
 * @brief   Shows an Ecore_Evas window.
 *
 * @details This function makes @a ee visible.
 *
 * @param[in]   ee  The Ecore_Evas to show
 */
EAPI void        ecore_evas_show(Ecore_Evas *ee);
/**
 * @brief   Hides an Ecore_Evas' window.
 *
 * @details This function makes @a ee hidden (not visible).
 *
 * @param[in]   ee  The Ecore_Evas to hide
 */
EAPI void        ecore_evas_hide(Ecore_Evas *ee);

/**
 * @brief   Activates (sets focus to, via the window manager) an Ecore_Evas' window.
 *
 * @details This functions activates the Ecore_Evas.
 *
 * @param[in]   ee  The Ecore_Evas to activate
 */
EAPI void        ecore_evas_activate(Ecore_Evas *ee);


/**
 * @brief   Sets the minimum size of a given @c Ecore_Evas window.
 *
 * @details This function sets the minimum size of @a ee to be @a w x @a h.
 *          You cannot resize that window to dimensions smaller than
 *          the ones set.
 *
 * @remarks When base sizes are set, using ecore_evas_size_base_set(),
 *          they are used to calculate a window's minimum size, instead of
 *          those set by this function.
 *
 * @param[in]   ee  An @c Ecore_Evas window's handle
 * @param[in]   w   The minimum width
 * @param[in]   h   The minimum height
 *
 * @see ecore_evas_size_min_get()
 */
EAPI void        ecore_evas_size_min_set(Ecore_Evas *ee, int w, int h);

/**
 * @brief   Gets the minimum size set for a given @c Ecore_Evas window.
 *
 * @remarks Use @c NULL pointers on the size components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   ee  An @c Ecore_Evas window's handle
 * @param[out]   w   A pointer to an int to place the minimum width in
 * @param[out]   h   A pointer to an int to place the minimum height in
 *
 * @see ecore_evas_size_min_set() for more details
 */
EAPI void        ecore_evas_size_min_get(const Ecore_Evas *ee, int *w, int *h);

/**
 * @brief   Sets the maximum size of a given @c Ecore_Evas window.
 *
 * @details This function sets the maximum size of @a ee to be @a w x @a h.
 *          You cannot resize that window to dimensions bigger than
 *          the ones set.
 *
 * @param[in]   ee  An @c Ecore_Evas window's handle
 * @param[in]   w   The maximum width
 * @param[in]   h   The maximum height
 *
 * @see ecore_evas_size_max_get()
 */
EAPI void        ecore_evas_size_max_set(Ecore_Evas *ee, int w, int h);

/**
 * @brief   Gets the maximum size set for a given @c Ecore_Evas window.
 *
 * @remarks Use @c NULL pointers on the size components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   ee  An @c Ecore_Evas window's handle
 * @param[out]   w   A pointer to an int to place the maximum width in
 * @param[out]   h   A pointer to an int to place the maximum height in
 *
 * @see ecore_evas_size_max_set() for more details
 */
EAPI void        ecore_evas_size_max_get(const Ecore_Evas *ee, int *w, int *h);

/**
 * @brief   Sets the base size for a given @c Ecore_Evas window.
 *
 * @details This function sets the @b base size of @a ee to be @a w x @a h.
 *          When base sizes are set, they are used to calculate a window's
 *          @b minimum size, instead of those set by ecore_evas_size_min_get().
 *
 * @param[in]   ee  An @c Ecore_Evas window's handle
 * @param[in]   w   The base width
 * @param[in]   h   The base height
 *
 * @see ecore_evas_size_base_get()
 */
EAPI void        ecore_evas_size_base_set(Ecore_Evas *ee, int w, int h);

/**
 * @brief   Gets the base size set for a given @c Ecore_Evas window.
 *
 * @remarks Use @c NULL pointers on the size components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   ee  An @c Ecore_Evas window's handle
 * @param[out]   w   A pointer to an int to place the base width in
 * @param[out]   h   A pointer to an int to place the base height in
 *
 * @see ecore_evas_size_base_set() for more details
 */
EAPI void        ecore_evas_size_base_get(const Ecore_Evas *ee, int *w, int *h);

/**
 * @brief   Sets the "size step" for a given @c Ecore_Evas window.
 *
 * @details This function sets the size steps of @a ee to be @a w x @a h. This
 *          limits the size of this @c Ecore_Evas window to be @b always an
 *          integer multiple of the step size, for each axis.
 *
 * @param[in]   ee  An @c Ecore_Evas window's handle
 * @param[in]   w   The step width
 * @param[in]   h   The step height
 */
EAPI void        ecore_evas_size_step_set(Ecore_Evas *ee, int w, int h);

/**
 * @brief   Gets the "size step" set for a given @c Ecore_Evas window.
 *
 * @remarks Use @c NULL pointers on the size components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   ee  An @c Ecore_Evas window's handle
 * @param[out]   w   A pointer to an int to place the step width in
 * @param[out]   h   A pointer to an int to place the step height in
 *
 * @see ecore_evas_size_base_set() for more details
 */
EAPI void        ecore_evas_size_step_get(const Ecore_Evas *ee, int *w, int *h);

/**
 * @brief   Sets the cursor of an Ecore_Evas.
 *
 * @details This function makes the mouse cursor over @a ee be the image specified by
 *          @a file. The actual point within the image that the mouse is at is specified
 *          by @a hot_x and @a hot_y, which are coordinates with respect to the top left
 *          corner of the cursor image.
 *
 * @remarks This function creates an object from the image and uses ecore_evas_object_cursor_set().
 *
 * @param[in]   ee     The Ecore_Evas
 * @param[in]   file   The path to an image file for the cursor
 * @param[in]   layer  The layer in which the cursor appears
 * @param[in]   hot_x  The x coordinate of the cursor's hot spot
 * @param[in]   hot_y  The y coordinate of the cursor's hot spot
 *
 * @see ecore_evas_object_cursor_set()
 */
EAPI void        ecore_evas_cursor_set(Ecore_Evas *ee, const char *file, int layer, int hot_x, int hot_y);
/**
 * @brief   Gets information about an Ecore_Evas' cursor.
 *
 * @details This function queries information about an Ecore_Evas' cursor.
 *
 * @param[in]   ee     The Ecore_Evas to set
 * @param[out]   obj    A pointer to an Evas_Object to place the cursor Evas_Object
 * @param[out]   layer  A pointer to an int to place the cursor's layer in
 * @param[out]   hot_x  A pointer to an int to place the cursor's hot_x coordinate in
 * @param[out]   hot_y  A pointer to an int to place the cursor's hot_y coordinate in
 *
 * @see ecore_evas_cursor_set()
 * @see ecore_evas_object_cursor_set()
 */
EAPI void        ecore_evas_cursor_get(const Ecore_Evas *ee, Evas_Object **obj, int *layer, int *hot_x, int *hot_y);

/**
 * @brief   Sets the cursor of an Ecore_Evas.
 *
 * @details This function makes the mouse cursor over @a ee be the object specified by
 *          @a obj. The actual point within the object that the mouse is at is specified
 *          by @a hot_x and @a hot_y, which are coordinates with respect to the top left
 *          corner of the cursor object.
 *
 * @param[in]   ee     The Ecore_Evas
 * @param[in]   obj    The Evas_Object which is the cursor
 * @param[in]   layer  The layer in which the cursor appears
 * @param[in]   hot_x  The x coordinate of the cursor's hot spot
 * @param[in]   hot_y  The y coordinate of the cursor's hot spot
 *
 * @see ecore_evas_cursor_set()
 */
EAPI void        ecore_evas_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y);

/**
 * @brief   Tells the WM whether or not to ignore an Ecore_Evas' window.
 *
 * @details This function causes the window manager to ignore @a ee if @a on is
 *          @c EINA_TRUE, or not ignore @a ee if @a on is @c EINA_FALSE.
 *
 * @param[in]   ee  The Ecore_Evas
 * @param[in]   on  Set @c EINA_TRUE to ignore, \n
 *              otherwise set @c EINA_FALSE to not ignore
 */
EAPI void        ecore_evas_override_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * @brief   Checks whether an Ecore_Evas window is overridden or not.
 *
 * @param[in]   ee  The Ecore_Evas to set
 * @return  The type of the damage management
 *
 */
EAPI Ecore_Evas_Avoid_Damage_Type ecore_evas_avoid_damage_get(const Ecore_Evas *ee);

/**
 * @brief  Sets the withdrawn state of an Ecore_Evas' window.
 *
 * @param[in]  ee         The Ecore_Evas whose window's withdrawn state is set
 * @param[in]  withdrawn  Set @c EINA_TRUE to set the withdrawn state, \n
 *                    otherwise set @c EINA_FALSE to not set the state
 *
 */
EAPI void        ecore_evas_withdrawn_set(Ecore_Evas *ee, Eina_Bool withdrawn);

/**
 * @brief   Gets the withdrawn state of an Ecore_Evas' window.
 *
 * @param[in]   ee  The Ecore_Evas whose window's withdrawn state is returned
 * @return  @c EINA_TRUE if the Ecore_Evas window's withdrawn state is state, \n
 *          otherwise @c EINA_FALSE if it is not state
 *
 */
EAPI Eina_Bool   ecore_evas_withdrawn_get(const Ecore_Evas *ee);

/**
 * @brief  Sets the sticky state of an Ecore_Evas window.
 *
 * @param[in]  ee      The Ecore_Evas whose window's sticky state is set
 * @param[in]  sticky  Set @c EINA_TRUE to set the sticky state, \n
 *                 otherwise set @c EINA_FALSE to not set it
 *
 */
EAPI void        ecore_evas_sticky_set(Ecore_Evas *ee, Eina_Bool sticky);

/**
 * @brief   Gets the sticky state of an Ecore_Evas' window.
 *
 * @param[in]   ee  The Ecore_Evas whose window's sticky state is returned
 * @return  @c EINA_TRUE if the Ecore_Evas window's sticky state is set, \n
 *          otherwise @c EINA_FALSE if the state is not set
 *
 */
EAPI Eina_Bool   ecore_evas_sticky_get(const Ecore_Evas *ee);

/**
 * @brief Enable/disable manual render
 *
 * @details If @p manual_render is true, default ecore_evas render routine would be
 *          disabled and rendering will be done only manually. If @p manual_render is
 *          false, rendering will be done by default ecore_evas rendering routine, but
 *          still manual rendering is available. Call ecore_evas_manual_render() to
 *          force immediate render.
 *
 * @param[in] ee An @c Ecore_Evas handle
 * @param[in] manual_render Enable/disable manual render. @c EINA_TRUE to enable
 *        manual render, @c EINA_FALSE to disable manual render. @c EINA_FALSE by
 *        default
 *
 * @see ecore_evas_manual_render_get()
 * @see ecore_evas_manual_render()
 */
EAPI void        ecore_evas_manual_render_set(Ecore_Evas *ee, Eina_Bool manual_render);

/**
 * @brief Get enable/disable status of manual render
 *
 * @param[in] ee An @c Ecore_Evas handle
 * @return @c EINA_TRUE if manual render is enabled, @c EINA_FALSE if manual
 *         render is disabled
 *
 * @see ecore_evas_manual_render_set()
 * @see ecore_evas_manual_render()
 */
EAPI Eina_Bool   ecore_evas_manual_render_get(const Ecore_Evas *ee);

/**
 * @brief   Registers an @c Ecore_Evas to receive events through ecore_input_evas.
 *
 * @details This function calls ecore_event_window_register() with the @a ee as its @a
 *          id argument, @a window argument, and uses its @a Evas too. It is useful when
 *          no @a window information is available on a given @a Ecore_Evas backend.
 * @since   1.1
 *
 * @param[in]   ee  The @c Ecore_Evas handle
 *
 * @see ecore_evas_input_event_unregister()
 */
EAPI void        ecore_evas_input_event_register(Ecore_Evas *ee);

/**
 * @brief  Unregisters an @c Ecore_Evas receiving events through ecore_input_evas.
 * @since  1.1
 *
 * @param[in]  ee  The @c Ecore_Evas handle
 *
 * @see ecore_evas_input_event_register()
 */
EAPI void        ecore_evas_input_event_unregister(Ecore_Evas *ee);

/**
 * @brief   Forces immediate rendering on a given @c Ecore_Evas window.
 *
 * @remarks Use this call to forcefully flush the @a ee's canvas rendering
 *          pipeline, thus bringing its window to an up-to-date state.
 * @param[in]   ee  An @c Ecore_Evas handle
 */
EAPI void        ecore_evas_manual_render(Ecore_Evas *ee);
EAPI void        ecore_evas_comp_sync_set(Ecore_Evas *ee, Eina_Bool do_sync);
EAPI Eina_Bool   ecore_evas_comp_sync_get(const Ecore_Evas *ee);

/**
 * @brief   Gets geometry of screen associated with this Ecore_Evas.
 *
 * @since   1.1
 *
 * @param[in]   ee  The Ecore_Evas whose window's to query container screen geometry
 * @param[out]   x   The horizontal offset value that is returned \n 
 *              This may be @c NULL.
 * @param[out]   y   The vertical offset value that is returned \n 
 *              This may be @c NULL.
 * @param[out]   w   The width value that is returned \n 
 *              This may be @c NULL.
 * @param[out]   h   The height value \n 
 *              This may be @c NULL.
 */
EAPI void        ecore_evas_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);

/**
 * @brief  Gets the DPI of the screen the Ecore_Evas is primarily on.
 *
 * @since  1.7
 *
 * @param[in]  ee    The Ecore_Evas whose window's to query
 * @param[out]  xdpi  The pointer to integer to store horizontal DPI \n 
 *               This may be @c NULL.
 * @param[out]  ydpi  The pointer to integer to store vertical DPI \n 
 *               This may be @c NULL.
 */
EAPI void        ecore_evas_screen_dpi_get(const Ecore_Evas *ee, int *xdpi, int *ydpi);

EAPI void        ecore_evas_draw_frame_set(Ecore_Evas *ee, Eina_Bool draw_frame);
EAPI Eina_Bool   ecore_evas_draw_frame_get(const Ecore_Evas *ee);

/**
 * @brief   Associates the given object to this ecore evas.
 *
 * @remarks Association means that operations on one affects the other. For
 *          example, moving the object moves the window, resizing the object
 *          also affects the ecore evas window, hide and show applies as well.
 *
 * @remarks This is meant to simplify development, since you often need to associate
 *          these events with your "base" objects, background or bottom-most object.
 *
 * @remarks Be aware that some methods might not be what you would like, deleting
 *          either the window or the object deletes the other. If you want to
 *          change that behavior, say to hide window when it is closed, you
 *          must use ecore_evas_callback_delete_request_set() and set your own code,
 *          like ecore_evas_hide(). Just remember that if you override delete_request
 *          and still want to delete the window or object, you must do that yourself.
 *
 * @remarks Since we now define delete_request, deleting windows does not quit
 *          main loop. If you wish to do so, you should listen for EVAS_CALLBACK_FREE
 *          on the object, that way you get notified and you can call
 *          ecore_main_loop_quit().
 *
 * @remarks Flags can be OR'ed of:
 *          @li ECORE_EVAS_OBJECT_ASSOCIATE_BASE (or 0): to listen to basic events
 *              like delete, resize and move, but no stacking or layer are used.
 *          @li ECORE_EVAS_OBJECT_ASSOCIATE_STACK: stacking operations act
 *              on the Ecore_Evas, not the object. So evas_object_raise()
 *              calls ecore_evas_raise(). Relative operations (stack_above, stack_below)
 *              are still not implemented.
 *          @li ECORE_EVAS_OBJECT_ASSOCIATE_LAYER: stacking operations act
 *              on the Ecore_Evas, not the object. So evas_object_layer_set()
 *              calls ecore_evas_layer_set().
 *          @li ECORE_EVAS_OBJECT_ASSOCIATE_DEL: the object delete deletes the
 *              ecore_evas and the delete_requests on the ecore_evas are also deleted.
 *
 * @param[in]   ee     The Ecore_Evas to associate to @a obj
 * @param[in]   obj    The object to associate to @a ee
 * @param[in]   flags  The association flags
 * @return  @c EINA_TRUE if associated successfully, \n
 *          otherwise @c EINA_FALSE on failure
 */
EAPI Eina_Bool    ecore_evas_object_associate(Ecore_Evas *ee, Evas_Object *obj, Ecore_Evas_Object_Associate_Flags flags);

/**
 * @brief   Cancels the association set with ecore_evas_object_associate().
 *
 * @param[in]   ee   The Ecore_Evas to dissociate from @a obj
 * @param[in]   obj  The object to dissociate from @a ee
 * @return  @c EINA_TRUE if the association is cancelled successfully, 
 *          otherwise @c EINA_FALSE on failure
 */
EAPI Eina_Bool    ecore_evas_object_dissociate(Ecore_Evas *ee, Evas_Object *obj);

/**
 * @brief   Gets the object associated with @a ee.
 *
 * @param[in]   ee  The Ecore_Evas to get the object from.
 * @return  The associated object, \n
 *          otherwise @c NULL if there is no associated object
 */
EAPI Evas_Object *ecore_evas_object_associate_get(const Ecore_Evas *ee);

/* Helper function to be used with ECORE_GETOPT_CALLBACK_*() */
EAPI unsigned char ecore_getopt_callback_ecore_evas_list_engines(const Ecore_Getopt *parser, const Ecore_Getopt_Desc *desc, const char *str, void *data, Ecore_Getopt_Value *storage);

/**
 * @brief   Gets a list of all the ecore_evases.
 *
 * @remarks The returned list of ecore evases is only valid until the canvases are
 *          destroyed (and should not be cached for instance). The list can be freed by
 *          just deleting the list.
 *
 * @return  A list of ecore_evases
 */
EAPI Eina_List   *ecore_evas_ecore_evas_list_get(void);

/* Specific calls to an x11 environment ecore_evas */
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
 * @internal
 * @defgroup Ecore_Evas_Ews Ecore_Evas Single Process Windowing System.
 * @ingroup Ecore_Evas_Group
 *
 * These are global scope functions to manage the EWS to be used by
 * ecore_evas_ews_new().
 *
 * @since 1.1
 * @{
 */

/**
 * @brief   Sets the engine to be used by the backing store engine.
 * @since   1.1
 *
 * @param[in]   engine   The engine to be set
 * @param[in]   options  The options of the engine to be set
 * @return  @c EINA_TRUE if the engine is set successfully, \n
 *          otherwise @c EINA_FALSE if ews is already in use
 */
EAPI Eina_Bool ecore_evas_ews_engine_set(const char *engine, const char *options);

/**
 * @brief   Reconfigures the backing store used.
 * @since   1.1
 *
 * @param[in]   x  The X coordinate to be used
 * @param[in]   y  The Y coordinate to be used
 * @param[in]   w  The width of the Ecore_Evas to setup
 * @param[in]   h  The height of the Ecore_Evas to setup
 * @return  @c EINA_TRUE if the backing store is reconfigured successfully, \n
 *          otherwise @c EINA_FALSE on failure
 */
EAPI Eina_Bool ecore_evas_ews_setup(int x, int y, int w, int h);

/**
 * @brief   Gets the internal backing store in use.
 * @since   1.1
 *
 * @remarks This forces it to be created, making future calls to
 *          ecore_evas_ews_engine_set() void.
 *
 * @return  The internal backing store in use
 * @see ecore_evas_ews_evas_get()
 */
EAPI Ecore_Evas *ecore_evas_ews_ecore_evas_get(void);

/**
 * @brief   Gets the internal backing store in use.
 * @since   1.1
 *
 * @remarks This forces it to be created, making future calls to
 *          ecore_evas_ews_engine_set() void.
 *
 * @return  The internal backing store in use
 * @see ecore_evas_ews_ecore_evas_get()
 */
EAPI Evas *ecore_evas_ews_evas_get(void);

/**
 * @brief   Gets the current background.
 *
 * @return  The background object
 * @see ecore_evas_ews_background_set()
 */
EAPI Evas_Object *ecore_evas_ews_background_get(void);

/**
 * @brief   Sets the current background. 
 * @details This must be created at evas ecore_evas_ews_evas_get().
 *
 * @remarks It is kept at lowest layer (EVAS_LAYER_MIN) and below
 *          everything else. You can set any object, default is a black
 *          rectangle.
 *
 * @remarks The previous object is deleted
 * @param[in]   o  The Evas_Object for which to set the background
 */
EAPI void ecore_evas_ews_background_set(Evas_Object *o);

/**
 * @brief   Gets all Ecore_Evas* created by EWS.
 * @since   1.1
 *
 * @remarks Do not change the returned list or its contents.
 *
 * @return  An eina list of Ecore_evases
 */
EAPI const Eina_List *ecore_evas_ews_children_get(void);

/**
 * @brief   Sets the identifier of the manager taking care of internal windows.
 * @since   1.1
 *
 * @remarks The ECORE_EVAS_EWS_EVENT_MANAGER_CHANGE event is issued. Consider
 *          handling it to know if you should stop handling events yourself
 *          (that is, another manager took over).
 *
 * @param[in]   manager  A unique identifier address
 *
 * @see ecore_evas_ews_manager_get()
 */
EAPI void        ecore_evas_ews_manager_set(const void *manager);

/**
 * @brief   Gets the identifier of the manager taking care of internal windows.
 * @since   1.1
 *
 * @return  The value set by ecore_evas_ews_manager_set()
 */
EAPI const void *ecore_evas_ews_manager_get(void);

EAPI extern int ECORE_EVAS_EWS_EVENT_MANAGER_CHANGE; /**< Manager is changed @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_ADD; /**< Window is created @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_DEL; /**< Window is deleted, pointer is already invalid but may be used as reference for further cleanup work. @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_RESIZE; /**< Window is resized @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_MOVE; /**< Window is moved @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_SHOW; /**< Window becomes visible @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_HIDE; /**< Window becomes hidden @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_FOCUS; /**< Window is focused @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_UNFOCUS; /**< Window lost focus @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_RAISE; /**< Window is raised @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_LOWER; /**< Window is lowered @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_ACTIVATE; /**< Window is activated @since 1.1 */

EAPI extern int ECORE_EVAS_EWS_EVENT_ICONIFIED_CHANGE; /**< Window minimized or iconified changed @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_MAXIMIZED_CHANGE; /**< Window maximized changed @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_LAYER_CHANGE; /**< Window layer changed @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_FULLSCREEN_CHANGE; /**< Window fullscreen changed @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE; /**< Some other window property changed (title, name, class, alpha, transparent, shaped etc) @since 1.1 */

/**
 * @}
 */

/**
 * @internal
 * @defgroup Ecore_Evas_Extn External plug/socket infrastructure to remote canvases
 * @ingroup Ecore_Evas_Group
 *
 * These functions allow one process to create a "socket" that is plugged into which another
 * process can create a "plug" remotely to plug into.
 * Socket can provide content for several plugs.
 * This is best for small sized objects (about the size range
 * of a small icon up to a few large icons). Since the plug is actually an
 * image object, you can fetch the pixel data.
 *
 * @since 1.2
 * @{
 */

EAPI extern int ECORE_EVAS_EXTN_CLIENT_ADD; /**< This event is received when a plug has connected to an extn socket @since 1.2 */
EAPI extern int ECORE_EVAS_EXTN_CLIENT_DEL; /**< This event is received when a plug has disconnected from an extn socket @since 1.2 */

/**
 * @brief   Creates a new Ecore_Evas canvas for the new external ecore evas socket.
 *
 * @since   1.2
 *
 * @remarks This function creates a new extn_socket canvas wrapper, with image data array
 *          @b bound to the ARGB format, 8 bits per pixel.
 *
 * @remarks If creation is successful, an Ecore_Evas handle is returned or @c NULL if
 *          creation fails. Also callbacks such as focus, show, and hide are also called
 *          if the plug object is shown, or already visible on connect, or if it is
 *          hidden later, focused or unfocused.
 *
 * @remarks This function has to be flowed by ecore_evas_extn_socket_listen(),
 *          for starting ecore ipc service.
 *
 * @code
 * Eina_Bool res = EINA_FALSE;
 * Ecore_Evas *ee = ecore_evas_extn_socket_new(1, 1);
 *
 * res = ecore_evas_extn_socket_listen("svcname", 1, EINA_FALSE);
 * if (!res) return;
 * ecore_evas_resize(ee, 240, 400);
 * @endcode
 *
 * or
 *
 * @code
 * Eina_Bool res = EINA_FALSE;
 * Ecore_Evas *ee = ecore_evas_extn_socket_new(240, 400);
 *
 * res = ecore_evas_extn_socket_listen("svcname", 1, EINA_FALSE);
 * if (!res) return;
 * @endcode
 *
 * @remarks When a client(plug) connects, you get the ECORE_EVAS_EXTN_CLIENT_ADD event
 *          in the ecore event queue, with event_info being the image object pointer
 *          passed as a void pointer. When a client disconnects you get the
 *          ECORE_EVAS_EXTN_CLIENT_DEL event.
 *
 * @remarks You can set up event handles for these events as follows:
 *
 * @code
 * static void client_add_cb(void *data, int event, void *event_info)
 * {
 *   Evas_Object *obj = event_info;
 *   printf("client added to image object %p\n", obj);
 *   evas_object_show(obj);
 * }
 *
 * static void client_del_cb(void *data, int event, void *event_info)
 * {
 *   Evas_Object *obj = event_info;
 *   printf("client deleted from image object %p\n", obj);
 *   evas_object_hide(obj);
 * }
 *
 * void setup(void)
 * {
 *   ecore_event_handler_add(ECORE_EVAS_EXTN_CLIENT_ADD,
 *                           client_add_cb, NULL);
 *   ecore_event_handler_add(ECORE_EVAS_EXTN_CLIENT_DEL,
 *                           client_del_cb, NULL);
 * }
 * @endcode
 *
 * @remarks Note that events come in later after the event happened. You may want to be
 *          careful as data structures you had associated with the image object
 *          may have been freed after deleting, but the object may still be around
 *          awaiting cleanup and thus still be valid.
 *
 * @param[in]   w  The width of the canvas, in pixels
 * @param[in]   h  The height of the canvas, in pixels
 * @return  A new @c Ecore_Evas instance, \n
 *          otherwise @c NULL on failure
 *
 * @see ecore_evas_extn_socket_listen()
 * @see ecore_evas_extn_plug_new()
 * @see ecore_evas_extn_plug_object_data_lock()
 * @see ecore_evas_extn_plug_object_data_unlock() 
 */
EAPI Ecore_Evas *ecore_evas_extn_socket_new(int w, int h);

/**
 * @brief   Creates a socket to provide the service for external ecore evas socket.
 *
 * @details This creates socket specified by @a svcname, @a svcnum and @a svcsys. If
 *          creation is successful, @c EINA_TRUE is returned or @c EINA_FALSE if
 *          creation fails.
 *
 * @since   1.2
 *
 * @param[in]   ee       The Ecore_Evas
 * @param[in]   svcname  The name of the service to be advertised \n
 *                   Ensure that it is unique (when combined with @a svcnum). 
 *                   Otherwise creation may fail.
 * @param[in]   svcnum   A number (any value, @c 0 being the common default) to
 *                   differentiate multiple instances of services with the same name
 * @param[in]   svcsys   Set @c EINA_TRUE to create a system-wide service all users can connect to, \n
 *                   otherwise set @c EINA_FALSE if the service should be private to the
 *                   user ID that created the service
 * @return  @c EINA_TRUE if created successfully, \n
 *          otherwise @c EINA_FALSE on failure
 *
 * @see ecore_evas_extn_socket_new()
 * @see ecore_evas_extn_plug_new()
 * @see ecore_evas_extn_plug_object_data_lock()
 * @see ecore_evas_extn_plug_object_data_unlock()
 */
EAPI Eina_Bool ecore_evas_extn_socket_listen(Ecore_Evas *ee, const char *svcname, int svcnum, Eina_Bool svcsys);

/**
 * @brief   Grabs a pointer to the actual pixels array of a given
 *          external ecore evas socket.
 *
 * @since   1.8
 *
 * @param[in]   ee  The Ecore_Evas
 * @return  The pixel data
 */
EAPI void *ecore_evas_extn_socket_pixels_get(Ecore_Evas *ee);

/**
 * @brief   Marks a region of the extn_socket canvas that has been updated.
 *
 * @since   1.8
 *
 * @param[in]   ee  The Ecore_Evas
 * @param[in]   x   The X-offset of the region to be updated
 * @param[in]   y   The Y-offset of the region to be updated
 * @param[in]   w   The width of the region to be updated
 * @param[in]   h   The height of the region to be updated
 */
EAPI void ecore_evas_extn_socket_update_add(Ecore_Evas *ee, int x, int y, int w, int h);

/**
 * @brief Lock the pixel data so the plug cannot change it
 *
 * @param[in] ee The Ecore_Evas.
 *
 * @see ecore_evas_extn_socket_new()
 * @see ecore_evas_extn_socket_unlock()
 *
 * @since 1.8
 */
EAPI void ecore_evas_extn_socket_lock(Ecore_Evas *ee);

/**
 * @brief Unlock the pixel data so the plug can change it again
 *
 * @param[in] ee The Ecore_Evas.
 *
 * @see ecore_evas_extn_socket_new()
 * @see ecore_evas_extn_socket_lock()
 *
 * @since 1.8
 */
EAPI void ecore_evas_extn_socket_unlock(Ecore_Evas *ee);

/**
 * @brief Set the blocking about mouse events to Ecore Evas.
 *
 * @param ee The Ecore_Evas.
 * @param events_block The blocking about mouse events.
 *
 * @see ecore_evas_extn_socket_events_block_get()
 *
 * @since 1.11
 */
EINA_DEPRECATED EAPI void ecore_evas_extn_socket_events_block_set(Ecore_Evas *ee, Eina_Bool events_block);

/**
 * @brief Get the blocking about mouse events to Ecore Evas.
 *
 * @param ee The Ecore_Evas.

 * @return The blocking about mouse events.
 *
 * @see ecore_evas_extn_socket_events_block_set()
 *
 * @since 1.11
 */
EINA_DEPRECATED EAPI Eina_Bool ecore_evas_extn_socket_events_block_get(Ecore_Evas *ee);

/**
 * @brief   Locks the pixel data so the socket cannot change it.
 * @since   1.2
 *
 * @remarks You may need to get the image pixel data with evas_object_image_data_get()
 *          from the image object, but need to ensure that it does not change while
 *          you are using the data. This function lets you set an advisory lock on the
 *          image data so the external plug process does not render to it or alter it.
 *
 * @remarks You should only hold the lock for just as long as you need to read out the
 *          image data or otherwise deal with it, and then unlock it with
 *          ecore_evas_extn_plug_object_data_unlock(). Keeping a lock over more than
 *          one iteration of the main ecore loop is problematic, so avoid it. Also
 *          forgetting to unlock may cause the socket process to freeze and thus create
 *          odd behavior.
 *
 * @param[in]   obj  The image object returned by ecore_evas_extn_plug_new() to lock
 *
 * @see ecore_evas_extn_plug_new()
 * @see ecore_evas_extn_plug_object_data_unlock()
 */
EAPI void ecore_evas_extn_plug_object_data_lock(Evas_Object *obj);

/**
 * @brief   Unlocks the pixel data so the socket can change it again.
 *
 * @details This function unlocks after an advisor lock has been taken by
 *          ecore_evas_extn_plug_object_data_lock().
 * @since   1.2
 *
 * @param[in]   obj  The image object returned by ecore_evas_extn_plug_new() to unlock
 *
 * @see ecore_evas_extn_plug_new()
 * @see ecore_evas_extn_plug_object_data_lock()
 */
EAPI void ecore_evas_extn_plug_object_data_unlock(Evas_Object *obj);

/**
 * @brief   Creates a new external ecore evas plug.
 * @details This creates an image object that contains the output of another
 *          processes socket canvas when it connects. All input is sent back to
 *          this process as well, effectively swallowing or placing the socket process
 *          in the canvas of the plug process in place of the image object. The image
 *          object by default is created to be filled (equivalent of
 *          evas_object_image_filled_add() on creation) so image content scales
 *          to fill the image unless otherwise reconfigured. The Ecore_Evas size
 *          of the plug is the master size and determines size in pixels of the
 *          plug canvas. You can change the size with something like:
 *
 * @code
 * Eina_Bool res = EINA_FALSE;
 * Evas_Object *obj = ecore_evas_extn_plug_new(ee);
 *
 * res = ecore_evas_extn_plug_connect("svcname", 1, EINA_FALSE);
 * if (!res) return;
 * ecore_evas_resize(ee, 240, 400);
 * @endcode
 *
 * @since   1.2
 *
 * @param[in]   ee_target  The Ecore_Evas containing the canvas in which the new image object lives
 * @return  An evas image object that contains the image output of a socket
 *
 * @see ecore_evas_extn_socket_new()
 * @see ecore_evas_extn_plug_connect()
 */
EAPI Evas_Object *ecore_evas_extn_plug_new(Ecore_Evas *ee_target);

/**
 * @brief   Connects an external ecore evas plug to service provided by external
 *          ecore evas socket.
 *
 * @since   1.2
 *
 * @param[in]   obj      The Ecore_Evas containing the canvas in which the new image
 *                   object lives
 * @param[in]   svcname  The service name to connect to set up by the socket
 * @param[in]   svcnum   The service number to connect to (set up by socket)
 * @param[in]   svcsys   Set @c EINA_TRUE to if the service is a system one, \n
 *                   otherwise set @c EINA_FALSE if it is not a system one (set up by socket)
 * @return  @c EINA_TRUE if created successfully, 
 *          otherwise @c EINA_FALSE on failure
 *
 * @see ecore_evas_extn_plug_new()
 */
EAPI Eina_Bool ecore_evas_extn_plug_connect(Evas_Object *obj, const char *svcname, int svcnum, Eina_Bool svcsys);

/**
 * @brief Retrieve the Visual used for pixmap creation
 *
 * @since 1.8
 *
 * @param[in] ee The Ecore_Evas containing the pixmap
 *
 * @return The Visual which was used when creating the pixmap
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system. This function should only be called if the Ecore_Evas was
 * created using @c ecore_evas_software_x11_pixmap_new or @c ecore_evas_gl_x11_pixmap_new
 */
EAPI void *ecore_evas_pixmap_visual_get(const Ecore_Evas *ee);

/**
 * @brief Retrieve the Colormap used for pixmap creation
 *
 * @since 1.8
 *
 * @param[in] ee The Ecore_Evas containing the pixmap
 *
 * @return The Colormap which was used when creating the pixmap
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system. This function should only be called if the Ecore_Evas was
 * created using @c ecore_evas_software_x11_pixmap_new or @c ecore_evas_gl_x11_pixmap_new
 */
EAPI unsigned long ecore_evas_pixmap_colormap_get(const Ecore_Evas *ee);

/**
 * @brief Retrieve the depth used for pixmap creation
 *
 * @since 1.8
 *
 * @param[in] ee The Ecore_Evas containing the pixmap
 *
 * @return The depth which was used when creating the pixmap
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system. This function should only be called if the Ecore_Evas was
 * created using @c ecore_evas_software_x11_pixmap_new or @c ecore_evas_gl_x11_pixmap_new
 */
EAPI int ecore_evas_pixmap_depth_get(const Ecore_Evas *ee);

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
