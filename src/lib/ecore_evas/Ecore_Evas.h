#ifndef _ECORE_EVAS_H
#define _ECORE_EVAS_H

#include <Evas.h>
#include <Ecore_Evas_Types.h>

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
 * @li @ref Ecore_Evas_Window_Sizes_Example_c
 * @li @ref Ecore_Evas_Buffer_Example_01_c
 * @li @ref Ecore_Evas_Buffer_Example_02_c
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
 * @ingroup Ecore
 *
 * Ecore evas is a set of functions that makes it easy to tie together ecore's
 * main loop and input handling to evas. As such it's a natural base for EFL
 * applications. While this combination makes it easy to create the basic
 * aspects all applications need, for normal applications(ones with buttons,
 * checkboxes and layouts) one should consider using Elementary.
 *
 * Ecore evas is extremely well suited for applications that are not based on
 * widgets. It has a main loop that delivers events, does basic window handling
 * and leaves all of the drawing up to the user. This works very well if used
 * in conjunction with Edje or if doing custom drawing as, for example, is done
 * in games.
 *
 * This is a list of examples of these functions:
 * @li @ref ecore_evas_basics_example_c
 * @li @ref ecore_evas_object_example_c
 * @li @ref ecore_evas_callbacks_example_c
 * @li @ref Ecore_Evas_Window_Sizes_Example_c
 * @li @ref Ecore_Evas_Buffer_Example_01_c
 * @li @ref Ecore_Evas_Buffer_Example_02_c
 *
 * @{
 */

/* these are dummy and just tell u what API levels ecore_evas supports - not if
 * the actual support is compiled in. You need to query for that separately.
 */
#define HAVE_ECORE_EVAS_X 1
#define HAVE_ECORE_EVAS_FB 1
#define HAVE_ECORE_EVAS_X11_GL 1
//#define HAVE_ECORE_EVAS_X11_16 1
//#define HAVE_ECORE_EVAS_DIRECTFB 1
#define HAVE_ECORE_EVAS_WIN32 1
#define HAVE_ECORE_EVAS_COCOA 1
#define HAVE_ECORE_EVAS_SDL 1
//#define HAVE_ECORE_EVAS_WINCE 1
#define HAVE_ECORE_EVAS_EWS 1
#define HAVE_ECORE_EVAS_PSL1GHT 1
#define HAVE_ECORE_EVAS_WAYLAND_SHM 1
#define HAVE_ECORE_EVAS_WAYLAND_EGL 1
#define HAVE_ECORE_EVAS_DRM 1

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
   ECORE_EVAS_ENGINE_WAYLAND_EGL,
   ECORE_EVAS_ENGINE_DRM
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
 *
 * @param engines list with engines names
 */
EAPI void        ecore_evas_engines_free(Eina_List *engines);
/**
 * @brief Creates a new Ecore_Evas based on engine name and common parameters.
 *
 * @param engine_name engine name as returned by
 *        ecore_evas_engines_get() or @c NULL to use environment variable
 *        ECORE_EVAS_ENGINE, that can be undefined and in this case
 *        this call will try to find the first working engine.
 * @param x horizontal position of window (not supported in all engines)
 * @param y vertical position of window (not supported in all engines)
 * @param w width of window
 * @param h height of window
 * @param extra_options string with extra parameter, dependent on engines
 *        or @ NULL. String is usually in the form: 'key1=value1;key2=value2'.
 *        Pay attention that when getting that from shell commands, most
 *        consider ';' as the command terminator, so you need to escape
 *        it or use quotes.
 *
 * @return Ecore_Evas instance or @c NULL if creation failed.
 */
EAPI Ecore_Evas *ecore_evas_new(const char *engine_name, int x, int y, int w, int h, const char *extra_options);
/**
 * @brief Set whether an Ecore_Evas has an alpha channel or not.
 *
 * @param ee The Ecore_Evas to shape
 * @param alpha @c EINA_TRUE to enable the alpha channel, @c EINA_FALSE to
 * disable it
 *
 * This function allows you to make an Ecore_Evas translucent using an
 * alpha channel. See ecore_evas_shaped_set() for details. The difference
 * between a shaped window and a window with an alpha channel is that an
 * alpha channel supports multiple levels of transparency, as opposed to
 * the 1 bit transparency of a shaped window (a pixel is either opaque, or
 * it's transparent).
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_alpha_set(Ecore_Evas *ee, Eina_Bool alpha);
/**
 * @brief Query whether an Ecore_Evas has an alpha channel.
 * @param ee The Ecore_Evas to query.
 * @return @c EINA_TRUE if ee has an alpha channel, @c EINA_FALSE if it does
 * not.
 *
 * This function returns @c EINA_TRUE if @p ee has an alpha channel, and
 * @c EINA_FALSE if it does not.
 *
 * @see ecore_evas_alpha_set()
 */
EAPI Eina_Bool   ecore_evas_alpha_get(const Ecore_Evas *ee);
/**
 * @brief Set whether an Ecore_Evas has an transparent window or not.
 *
 * @param ee The Ecore_Evas to shape
 * @param transparent @c EINA_TRUE to enable the transparent window,
 * @c EINA_FALSE to disable it
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
 * @return @c EINA_TRUE if ee is transparent, @c EINA_FALSE if it isn't.
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
 * may pass @c NULL on them.
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
 * the geometry which @p ee was latest recently requested . If any of the
 * parameters is not desired you may pass @c NULL on them.
 * This function can represent recently requested geometry.
 * ecore_evas_geometry_get function returns the value is updated after engine
 * finished request. By comparison, ecore_evas_request_geometry_get returns
 * recently requested value.
 *
 * @code
 * int x, y, w, h;
 * ecore_evas_request_geometry_get(ee, &x, &y, &w, &h);
 * @endcode
 *
 * @since 1.1
 */
EAPI void        ecore_evas_request_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);
/**
 * @brief Set the focus of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas
 * @param on @c EINA_TRUE for focus, @c EINA_FALSE to defocus.
 *
 * This function focuses @p ee if @p on is @c EINA_TRUE, or unfocuses @p ee if
 * @p on is @c EINA_FALSE.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_focus_set(Ecore_Evas *ee, Eina_Bool on);
/**
 * @brief Query whether an Ecore_Evas' window is focused or not.
 *
 * @param ee The Ecore_Evas to set
 * @return @c EINA_TRUE if @p ee if focused, @c EINA_FALSE if not.
 *
 * @see ecore_evas_focus_set()
 */
EAPI Eina_Bool   ecore_evas_focus_get(const Ecore_Evas *ee);
/**
 * @brief Iconify or uniconify an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas
 * @param on @c EINA_TRUE to iconify, @c EINA_FALSE to uniconify.
 *
 * This function iconifies @p ee if @p on is @c EINA_TRUE, or uniconifies @p ee
 * if @p on is @c EINA_FALSE.
 *
 * @note Iconify and minimize are synonyms.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_iconified_set(Ecore_Evas *ee, Eina_Bool on);
/**
 * @brief Query whether an Ecore_Evas' window is iconified or not.
 *
 * @param ee The Ecore_Evas to set
 * @return @c EINA_TRUE if @p ee is iconified, @c EINA_FALSE if not.
 *
 * @note Iconify and minimize are synonyms.
 *
 * @see ecore_evas_iconified_set()
 */
EAPI Eina_Bool   ecore_evas_iconified_get(const Ecore_Evas *ee);
/**
 * @brief Set whether an Ecore_Evas' window is borderless or not.
 *
 * @param ee The Ecore_Evas
 * @param on @c EINA_TRUE for borderless, @c EINA_FALSE for bordered.
 *
 * This function makes @p ee borderless if @p on is @c EINA_TRUE, or bordered
 * if @p on is @c EINA_FALSE.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_borderless_set(Ecore_Evas *ee, Eina_Bool on);
/**
 * @brief Query whether an Ecore_Evas' window is borderless or not.
 *
 * @param ee The Ecore_Evas to set
 * @return @c EINA_TRUE if @p ee is borderless, @c EINA_FALSE if not.
 *
 * @see ecore_evas_borderless_set()
 */
EAPI Eina_Bool   ecore_evas_borderless_get(const Ecore_Evas *ee);
/**
 * @brief Set whether or not an Ecore_Evas' window is fullscreen.
 *
 * @param ee The Ecore_Evas
 * @param on @c EINA_TRUE fullscreen, @c EINA_FALSE not.
 *
 * This function causes @p ee to be fullscreen if @p on is @c EINA_TRUE, or
 * not if @p on is @c EINA_FALSE.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_fullscreen_set(Ecore_Evas *ee, Eina_Bool on);
/**
 * @brief Query whether an Ecore_Evas' window is fullscreen or not.
 *
 * @param ee The Ecore_Evas to set
 * @return @c EINA_TRUE if @p ee is fullscreen, @c EINA_FALSE if not.
 *
 * @see ecore_evas_fullscreen_set()
 */
EAPI Eina_Bool   ecore_evas_fullscreen_get(const Ecore_Evas *ee);
/**
 * @brief Set another window that this window is a group member of
 *
 * @param ee The Ecore_Evas
 * @param ee_group The other group member
 *
 * If @p ee_group is @c NULL, @p ee is removed from the group, otherwise it is
 * added. Note that if you free the @p ee_group canvas before @p ee, then
 * getting the group canvas with ecore_evas_window_group_get() will return
 * an invalid handle.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.2
 */
EAPI void        ecore_evas_window_group_set(Ecore_Evas *ee, const Ecore_Evas *ee_group);
/**
 * @brief Get the canvas group set.
 *
 * This returns the handle set by ecore_evas_window_group_set().
 *
 * @param ee The Ecore_Evas to set
 * @return The Canvas group handle
 *
 * @see ecore_evas_window_group_set()
 * @since 1.2
 */
EAPI const Ecore_Evas *ecore_evas_window_group_get(const Ecore_Evas *ee);
/**
 * @brief Set the aspect ratio of a canvas window
 *
 * @param ee The Ecore_Evas
 * @param aspect The aspect ratio (width divided by height), or 0 to disable
 *
 * This sets the desired aspect ratio of a canvas window
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.2
 */
EAPI void        ecore_evas_aspect_set(Ecore_Evas *ee, double aspect);
/**
 * @brief Get the aspect ratio of a canvas window
 *
 * This returns the value set by ecore_evas_aspect_set().
 *
 * @param ee The Ecore_Evas to set
 * @return The aspect ratio
 *
 * @see ecore_evas_aspect_set()
 * @since 1.2
 */
EAPI double      ecore_evas_aspect_get(const Ecore_Evas *ee);
/**
 * @brief Set The urgent hint flag
 *
 * @param ee The Ecore_Evas
 * @param urgent The urgent state flag
 *
 * This sets the "urgent" state hint on a window so the desktop environment
 * can highlight it somehow.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.2
 */
EAPI void        ecore_evas_urgent_set(Ecore_Evas *ee, Eina_Bool urgent);
/**
 * @brief Get the urgent state on the cavas window
 *
 * This returns the value set by ecore_evas_urgent_set()
 *
 * @param ee The Ecore_Evas to set
 * @return The urgent state set
 *
 * @see ecore_evas_urgent_set()
 * @since 1.2
 */
EAPI Eina_Bool   ecore_evas_urgent_get(const Ecore_Evas *ee);
/**
 * @brief Set the modal state flag on the canvas window
 *
 * @param ee The Ecore_Evas
 * @param modal The modal hint flag
 *
 * This hints if the window should be modal (eg if it is also transient
 * for another window, the other window will maybe be denied focus by
 * the desktop window manager).
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.2
 */
EAPI void        ecore_evas_modal_set(Ecore_Evas *ee, Eina_Bool modal);
/**
 * @brief Get The modal flag
 *
 * This returns the value set by ecore_evas_modal_set().
 *
 * @param ee The Ecore_Evas to set
 * @return The modal flag
 *
 * @see ecore_evas_modal_set()
 * @since 1.2
 */
EAPI Eina_Bool   ecore_evas_modal_get(const Ecore_Evas *ee);
/**
 * @brief Set the "i demand attention" flag on a canvas window
 *
 * @param ee The Ecore_Evas
 * @param demand The flag state to set
 *
 * A window may demand attention now (eg you must enter a password before
 * continuing), and so it may flag a window with this.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.2
 */
EAPI void        ecore_evas_demand_attention_set(Ecore_Evas *ee, Eina_Bool demand);
/**
 * @brief Get the "i demand attention" flag
 *
 * This returns the value set by ecore_evas_demand_attention_set().
 *
 * @param ee The Ecore_Evas to set
 * @return The "i demand attention" flag.
 *
 * @see ecore_evas_demand_attention_set()
 * @since 1.2
 */
EAPI Eina_Bool   ecore_evas_demand_attention_get(const Ecore_Evas *ee);
/**
 * @brief Set the "focus skip" flag
 *
 * @param ee The Ecore_Evas
 * @param skip The "focus skip" state to set.
 *
 * A window may not want to accept focus, be in the taskbar, pager etc.
 * sometimes (example for a small notification window that hovers around
 * a taskbar or panel, or hovers around a window until some activity
 * dismisses it).
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.2
 */
EAPI void        ecore_evas_focus_skip_set(Ecore_Evas *ee, Eina_Bool skip);
/**
 * @brief Get the "focus skip" flag
 *
 * This returns the value set by ecore_evas_focus_skip_set().
 *
 * @param ee The Ecore_Evas to set
 * @return The "focus skip" flag.
 *
 * @see ecore_evas_focus_skip_set()
 * @since 1.2
 */
EAPI Eina_Bool   ecore_evas_focus_skip_get(const Ecore_Evas *ee);

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
 * @param on @c EINA_TRUE to maximize, @c EINA_FALSE to unmaximize.
 *
 * This function maximizes @p ee if @p on is @c EINA_TRUE, or unmaximizes @p ee
 * if @p on is @c EINA_FALSE.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_maximized_set(Ecore_Evas *ee, Eina_Bool on);
/**
 * @brief Query whether an Ecore_Evas' window is maximized or not.
 *
 * @param ee The Ecore_Evas to set
 * @return @c EINA_TRUE if @p ee is maximized, @c EINA_FALSE if not.
 *
 * @see ecore_evas_maximized_set()
 */
EAPI Eina_Bool   ecore_evas_maximized_get(const Ecore_Evas *ee);
/**
 * @brief Query if the underlying windowing system supports the window profile.
 *
 * @param ee The Ecore_Evas
 * @return @c EINA_TRUE if the window profile is supported, @c EINA_FALSE otherwise.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 */
EAPI Eina_Bool   ecore_evas_window_profile_supported_get(const Ecore_Evas *ee);
/**
 * @brief Set the window profile
 *
 * @param ee The Ecore_Evas to set
 * @param profile The string value of the window profile
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 */
EAPI void        ecore_evas_window_profile_set(Ecore_Evas *ee, const char *profile);
/**
 * @brief Get the window profile
 *
 * @param ee The Ecore_Evas to get the window profile from.
 * @return The string value of the window profile, or NULL if none exists
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 */
EAPI const char *ecore_evas_window_profile_get(const Ecore_Evas *ee);
/**
 * @brief Set the array of available window profiles
 *
 * @param ee The Ecore_Evas to set
 * @param profiles The string array of available window profiels
 * @param count The number of members in profiles
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 */
EAPI void        ecore_evas_window_available_profiles_set(Ecore_Evas *ee, const char **profiles, const unsigned int count);
/**
 * @brief Get the array of available window profiles
 *
 * @param ee The Ecore_Evas to get available window profiles from.
 * @param profiles Where to return the string array of available window profiles
 * @param count Where to return the number of members in profiles
 * @return EINA_TRUE if available window profiles exist, EINA_FALSE otherwise
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 */
EAPI Eina_Bool   ecore_evas_window_available_profiles_get(Ecore_Evas *ee, char ***profiles, unsigned int *count);
/**
 * @brief Query if the underlying windowing system supports the window manager rotation.
 *
 * @param ee The Ecore_Evas
 * @return @c EINA_TRUE if the window manager rotation is supported, @c EINA_FALSE otherwise.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.9.0
 */
EAPI Eina_Bool   ecore_evas_wm_rotation_supported_get(const Ecore_Evas *ee);
/**
 * @brief Set the preferred rotation hint.
 *
 * @param ee The Ecore_Evas to set
 * @param rotation Value to set the preferred rotation hint
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.9.0
 */
EAPI void        ecore_evas_wm_rotation_preferred_rotation_set(Ecore_Evas *ee, int rotation);
/**
 * @brief Get the preferred rotation hint.
 *
 * @param ee The Ecore_Evas to get the preferred rotation hint from.
 * @return The preferred rotation hint, -1 on failure.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.9.0
 */
EAPI int         ecore_evas_wm_rotation_preferred_rotation_get(const Ecore_Evas *ee);
/**
 * @brief Set the array of available window rotations.
 *
 * @param ee The Ecore_Evas to set
 * @param rotations The integer array of available window rotations
 * @param count The number of members in rotations
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.9.0
 */
EAPI void        ecore_evas_wm_rotation_available_rotations_set(Ecore_Evas *ee, const int *rotations, unsigned int count);
/**
 * @brief Get the array of available window rotations.
 *
 * @param ee The Ecore_Evas to get available window rotations from.
 * @param rotations Where to return the integer array of available window rotations
 * @param count Where to return the number of members in rotations
 * @return EINA_TRUE if available window rotations exist, EINA_FALSE otherwise
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.9.0
 */
EAPI Eina_Bool   ecore_evas_wm_rotation_available_rotations_get(const Ecore_Evas *ee, int **rotations, unsigned int *count);
/**
 * @brief Set manual rotation done mode of Ecore_Evas's window
 *
 * @param ee The Ecore_Evas
 * @param set If true, the window manager will not rotate the Ecore_Evas's window until
 * the rotation done event is received by ecore_evas_wm_rotation_manual_rotation_done.
 * If false, the manual rotation mode is disabled.
 *
 * @since 1.9.0
 */
EAPI void        ecore_evas_wm_rotation_manual_rotation_done_set(Ecore_Evas *ee, Eina_Bool set);
/**
 * @brief Get manual rotation done mode of Ecore_Evas's window
 *
 * @param ee The Ecore_Evas
 * @return If true, the manual rotation done mode is enabled
 *
 * @since 1.9.0
 */
EAPI Eina_Bool   ecore_evas_wm_rotation_manual_rotation_done_get(const Ecore_Evas *ee);
/**
 * @brief Set rotation finish manually
 *
 * @param ee The Ecore_Evas
 *
 * @since 1.9.0
 */
EAPI void        ecore_evas_wm_rotation_manual_rotation_done(Ecore_Evas *ee);
/**
 * @brief Send message to parent ecore
 *
 * @param ee The Ecore_Evas to set
 * @param msg_domain The domain of message
 * @param msg_id The id of message
 * @param data The data of message
 * @param size The size of message data
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 *
 * @see ecore_evas_msg_send()
 * @see ecore_evas_callback_msg_parent_handle_set()
 * @see eecore_evas_callback_msg_handle_set()
 *
 * This is a list of examples of these functions:
 * @li @ref ecore_evas_extn_socket_example
 * @li @ref ecore_evas_extn_plug_example
 */
EAPI void ecore_evas_msg_parent_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size);
/**
 * @brief Send message to child ecore
 *
 * @param ee The Ecore_Evas to set
 * @param msg_domain The domain of message
 * @param msg_id The id of message
 * @param data The data of message
 * @param size The size of message data
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 *
 * @see ecore_evas_msg_parent_send()
 * @see ecore_evas_callback_msg_parent_handle_set()
 * @see eecore_evas_callback_msg_handle_set()
 */
EAPI void ecore_evas_msg_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size);
/**
 * Set a callback for parent Ecore_Evas message.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func_parent_handle The handle to be called when message arive.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 *
 * @see ecore_evas_msg_parent_send()
 * @see ecore_evas_msg_send()
 * @see eecore_evas_callback_msg_handle_set()
 */
EAPI void ecore_evas_callback_msg_parent_handle_set(Ecore_Evas *ee, void (*func_parent_handle)(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size));
/**
 * Set a callback for child Ecore_Evas message.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func_handle  The handle to be called when message arive
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 *
 * @see ecore_evas_msg_parent_send()
 * @see ecore_evas_msg_send()
 * @see ecore_evas_callback_msg_parent_handle_set()
 */
EAPI void ecore_evas_callback_msg_handle_set(Ecore_Evas *ee, void (*func_handle)(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size));

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
 * This function gets the name of @p ee into @p n, and its class into
 * @p c.
 *
 * @param ee The Ecore_Evas to query.
 * @param n A pointer to a string to place the name in.
 * @param c A pointer to a string to place the class in.
 * @see ecore_evas_name_class_set()
 */
EAPI void        ecore_evas_name_class_get(const Ecore_Evas *ee, const char **n, const char **c);
/**
 * @brief Returns a pointer to the underlying window.
 *
 * @param ee The Ecore_Evas whose window is desired.
 * @return A pointer to the underlying window.
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
 * @param ee The Ecore_Evas whose pixmap is desired.
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
 * @warning The XPixmap ID can change with every frame after it is rendered, 
 * so you should ALWAYS call ecore_evas_software_x11_pixmap_get when you 
 * need the current pixmap id.
 * 
 * @since 1.8
 */
EAPI Ecore_Evas     *ecore_evas_gl_x11_pixmap_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);

/**
 * @brief Returns the underlying Ecore_X_Pixmap used in the Ecore_Evas
 * 
 * @param ee The Ecore_Evas whose pixmap is desired.
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
EAPI Ecore_X_Pixmap ecore_evas_gl_x11_pixmap_get(const Ecore_Evas *ee);

EAPI Ecore_Evas     *ecore_evas_xrender_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h) EINA_DEPRECATED;
EAPI Ecore_X_Window  ecore_evas_xrender_x11_window_get(const Ecore_Evas *ee) EINA_DEPRECATED;
EAPI void            ecore_evas_xrender_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on) EINA_DEPRECATED;
EAPI Eina_Bool       ecore_evas_xrender_x11_direct_resize_get(const Ecore_Evas *ee) EINA_DEPRECATED;
EAPI void            ecore_evas_xrender_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win) EINA_DEPRECATED;

EAPI Ecore_Evas     *ecore_evas_software_x11_8_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h) EINA_DEPRECATED;
EAPI Ecore_X_Window  ecore_evas_software_x11_8_window_get(const Ecore_Evas *ee) EINA_DEPRECATED;
EAPI Ecore_X_Window  ecore_evas_software_x11_8_subwindow_get(const Ecore_Evas *ee) EINA_DEPRECATED;
EAPI void            ecore_evas_software_x11_8_direct_resize_set(Ecore_Evas *ee, Eina_Bool on) EINA_DEPRECATED;
EAPI Eina_Bool       ecore_evas_software_x11_8_direct_resize_get(const Ecore_Evas *ee) EINA_DEPRECATED;
EAPI void            ecore_evas_software_x11_8_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win) EINA_DEPRECATED;

EAPI Ecore_Evas     *ecore_evas_software_x11_16_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h) EINA_DEPRECATED;
EAPI Ecore_X_Window  ecore_evas_software_x11_16_window_get(const Ecore_Evas *ee) EINA_DEPRECATED;
EAPI void            ecore_evas_software_x11_16_direct_resize_set(Ecore_Evas *ee, Eina_Bool on) EINA_DEPRECATED;
EAPI Eina_Bool       ecore_evas_software_x11_16_direct_resize_get(const Ecore_Evas *ee) EINA_DEPRECATED;
EAPI void            ecore_evas_software_x11_16_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win) EINA_DEPRECATED;

EAPI Ecore_Evas     *ecore_evas_fb_new(const char *disp_name, int rotation, int w, int h);

EAPI Ecore_Evas     *ecore_evas_directfb_new(const char *disp_name, int windowed, int x, int y, int w, int h) EINA_DEPRECATED;
EAPI Ecore_DirectFB_Window *ecore_evas_directfb_window_get(const Ecore_Evas *ee) EINA_DEPRECATED;


EAPI Ecore_Evas     *ecore_evas_wayland_shm_new(const char *disp_name, unsigned int parent, int x, int y, int w, int h, Eina_Bool frame);
EAPI Ecore_Evas     *ecore_evas_wayland_egl_new(const char *disp_name, unsigned int parent, int x, int y, int w, int h, Eina_Bool frame);
EAPI void            ecore_evas_wayland_resize(Ecore_Evas *ee, int location);
EAPI void            ecore_evas_wayland_move(Ecore_Evas *ee, int x, int y);

EAPI void            ecore_evas_wayland_pointer_set(Ecore_Evas *ee, int hot_x, int hot_y);
EAPI void            ecore_evas_wayland_type_set(Ecore_Evas *ee, int type);
EAPI Ecore_Wl_Window *ecore_evas_wayland_window_get(const Ecore_Evas *ee);

EAPI Ecore_Evas     *ecore_evas_drm_new(const char *device, unsigned int parent, int x, int y, int w, int h);

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
 * @return The evas object of EWS backing store.
 *
 * @note This should not be modified anyhow, but may be helpful to
 *       determine stacking and geometry of it for window managers
 *       that decorate windows.
 *
 * @param ee The Ecore_Evas from which to get the backing store.
 * @see ecore_evas_ews_manager_set()
 * @see ecore_evas_ews_evas_get()
 * @since 1.1
 */
EAPI Evas_Object *ecore_evas_ews_backing_store_get(const Ecore_Evas *ee);

/**
 * Calls the window to be deleted (freed), but can let user decide to
 * forbid it by using ecore_evas_callback_delete_request_set()
 *
 * @param ee The Ecore_Evas for which window will be deleted.
 * @since 1.1
 */
EAPI void ecore_evas_ews_delete_request(Ecore_Evas *ee);

/**
 * @brief Create an Evas image object with image data <b>bound to an
 * own, internal @c Ecore_Evas canvas wrapper</b>
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
                                                   int                 height) EINA_DEPRECATED;

EAPI Ecore_Evas     *ecore_evas_software_wince_fb_new(Ecore_WinCE_Window *parent,
                                                      int                 x,
                                                      int                 y,
                                                      int                 width,
                                                      int                 height) EINA_DEPRECATED;

EAPI Ecore_Evas     *ecore_evas_software_wince_gapi_new(Ecore_WinCE_Window *parent,
                                                        int                 x,
                                                        int                 y,
                                                        int                 width,
                                                        int                 height) EINA_DEPRECATED;

EAPI Ecore_Evas     *ecore_evas_software_wince_ddraw_new(Ecore_WinCE_Window *parent,
                                                         int                 x,
                                                         int                 y,
                                                         int                 width,
                                                         int                 height) EINA_DEPRECATED;

EAPI Ecore_Evas     *ecore_evas_software_wince_gdi_new(Ecore_WinCE_Window *parent,
                                                       int                 x,
                                                       int                 y,
                                                       int                 width,
                                                       int                 height) EINA_DEPRECATED;

EAPI Ecore_WinCE_Window *ecore_evas_software_wince_window_get(const Ecore_Evas *ee) EINA_DEPRECATED;

EAPI Ecore_Evas *ecore_evas_cocoa_new(Ecore_Cocoa_Window *parent,
				      int x,
				      int y,
				      int w,
				      int h);

EAPI Ecore_Evas *ecore_evas_psl1ght_new(const char* name, int w, int h);


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
 * @return The Ecore_Evas that holds this Evas, or @c NULL if not held by one.
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
 * @returns @c NULL on error or no data found, A pointer to the user data on
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
EAPI void        ecore_evas_callback_resize_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_move_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_show_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_hide_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_delete_request_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_destroy_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_focus_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_focus_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_sticky_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_unsticky_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_mouse_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_mouse_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_pre_render_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_post_render_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
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
EAPI void        ecore_evas_callback_pre_free_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
/**
 * Set a callback for Ecore_Evas state changes.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee changes state.
 *
 * @since 1.2
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_state_change_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * Get an Ecore_Evas's Evas
 * @param ee The Ecore_Evas whose Evas you wish to get
 * @return The Evas wrapped by @p ee
 *
 * This function returns the Evas contained within @p ee.
 */
EAPI Evas       *ecore_evas_get(const Ecore_Evas *ee);

/**
 * Provide Managed move co-ordinates for an Ecore_Evas
 * @param ee The Ecore_Evas to move
 * @param x The x coordinate to set as the managed location
 * @param y The y coordinate to set as the managed location
 *
 * This sets the managed geometry position of the @p ee to (@p x, @p y)
 */
EAPI void        ecore_evas_managed_move(Ecore_Evas *ee, int x, int y);

/**
 * Set whether an Ecore_Evas is shaped or not.
 *
 * @param ee The Ecore_Evas to shape.
 * @param shaped @c EINA_TRUE to shape, @c EINA_FALSE to not.
 *
 * This function allows one to make an Ecore_Evas shaped to the contents of the
 * evas. If @p shaped is @c EINA_TRUE, @p ee will be transparent in parts of
 * the evas that contain no objects. If @p shaped is @c EINA_FALSE, then @p ee
 * will be rectangular, and parts with no data will show random framebuffer
 * artifacting. For non-shaped Ecore_Evases, it is recommended to cover the
 * entire evas with a background object.
 */
EAPI void        ecore_evas_shaped_set(Ecore_Evas *ee, Eina_Bool shaped);

/**
 * Query whether an Ecore_Evas is shaped or not.
 *
 * @param ee The Ecore_Evas to query.
 * @return @c EINA_TRUE if shaped, @c EINA_FALSE if not.
 *
 * This function returns @c EINA_TRUE if @p ee is shaped, and @c EINA_FALSE if not.
 */
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

/**
 * Activate (set focus to, via the window manager) an Ecore_Evas' window.
 * @param ee The Ecore_Evas to activate.
 *
 * This functions activates the Ecore_Evas.
 */
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
 * limits the size of this @c Ecore_Evas window to be @b always an
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

/**
 * Tell the WM whether or not to ignore an Ecore_Evas' window
 *
 * @param ee The Ecore_Evas.
 * @param on @c EINA_TRUE to ignore, @c EINA_FALSE to not.
 *
 * This function causes the window manager to ignore @p ee if @p on is
 * @c EINA_TRUE, or not ignore @p ee if @p on is @c EINA_FALSE.
 */
EAPI void        ecore_evas_override_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * Query whether an Ecore_Evas' window is overridden or not
 *
 * @param ee The Ecore_Evas to set.
 * @return @c EINA_TRUE if @p ee is overridden, @c EINA_FALSE if not.
 */
EAPI Eina_Bool   ecore_evas_override_get(const Ecore_Evas *ee);

/**
 * Set whether or not an Ecore_Evas' window should avoid damage
 *
 * @param ee The Ecore_Evas
 * @param on The type of the damage management
 *
 * This option causes updates of the Ecore_Evas to be done on a pixmap, and
 * then copied to the window, or the pixmap used directly on the window,
 * depending on the setting. Possible options are:
 *
 * @li @ref ECORE_EVAS_AVOID_DAMAGE_NONE - every expose event triggers a new
 * damage and consequently render of the affected area. The rendering of things
 * happens directly on the window;
 *
 * @li @ref ECORE_EVAS_AVOID_DAMAGE_EXPOSE - there's a pixmap where everything
 * is rendered into, and then copied to the window. On expose events, there's
 * no need to render things again, just to copy the exposed region to the
 * window;
 *
 * @li @ref ECORE_EVAS_AVOID_DAMAGE_BUILT_IN - there's the same pixmap as the
 * previous one, but it is set as a "background pixmap" of the window.  The
 * rendered things appear directly on the window, with no need to copy
 * anything, but would stay stored on the pixmap, so there's no need to render
 * things again on expose events. This option can be faster than the previous
 * one, but may lead to artifacts during resize of the window.
 */
EAPI void        ecore_evas_avoid_damage_set(Ecore_Evas *ee, Ecore_Evas_Avoid_Damage_Type on);

/**
 * Query whether an Ecore_Evas' window avoids damage or not
 * @param ee The Ecore_Evas to set
 * @return The type of the damage management
 *
 */
EAPI Ecore_Evas_Avoid_Damage_Type ecore_evas_avoid_damage_get(const Ecore_Evas *ee);

/**
 * Set the withdrawn state of an Ecore_Evas' window.
 * @param ee The Ecore_Evas whose window's withdrawn state is set.
 * @param withdrawn The Ecore_Evas window's new withdrawn state.
 *
 */
EAPI void        ecore_evas_withdrawn_set(Ecore_Evas *ee, Eina_Bool withdrawn);

/**
 * Returns the withdrawn state of an Ecore_Evas' window.
 * @param ee The Ecore_Evas whose window's withdrawn state is returned.
 * @return The Ecore_Evas window's withdrawn state.
 *
 */
EAPI Eina_Bool   ecore_evas_withdrawn_get(const Ecore_Evas *ee);

/**
 * Set the sticky state of an Ecore_Evas window.
 *
 * @param ee The Ecore_Evas whose window's sticky state is set.
 * @param sticky The Ecore_Evas window's new sticky state.
 *
 */
EAPI void        ecore_evas_sticky_set(Ecore_Evas *ee, Eina_Bool sticky);

/**
 * Returns the sticky state of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas whose window's sticky state is returned.
 * @return The Ecore_Evas window's sticky state.
 *
 */
EAPI Eina_Bool   ecore_evas_sticky_get(const Ecore_Evas *ee);

/**
 * Enable/disable manual render
 *
 * @param ee An @c Ecore_Evas handle
 * @param manual_render Enable/disable manual render. @c EINA_TRUE to enable
 * manual render, @c EINA_FALSE to disable manual render. @c EINA_FALSE by
 * default
 *
 * If @p manual_render is true, default ecore_evas render routine would be
 * disabled and rendering will be done only manually. If @p manual_render is
 * false, rendering will be done by default ecore_evas rendering routine, but
 * still manual rendering is available. Call ecore_evas_manual_render() to
 * force immediate render.
 *
 * @see ecore_evas_manual_render_get()
 * @see ecore_evas_manual_render()
 */
EAPI void        ecore_evas_manual_render_set(Ecore_Evas *ee, Eina_Bool manual_render);

/**
 * Get enable/disable status of manual render
 *
 * @param ee An @c Ecore_Evas handle
 * @return @c EINA_TRUE if manual render is enabled, @c EINA_FALSE if manual
 * render is disabled
 *
 * @see ecore_evas_manual_render_set()
 * @see ecore_evas_manual_render()
 */
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
 * @param x where to return the horizontal offset value. May be @c NULL.
 * @param y where to return the vertical offset value. May be @c NULL.
 * @param w where to return the width value. May be @c NULL.
 * @param h where to return the height value. May be @c NULL.
 *
 * @since 1.1
 */
EAPI void        ecore_evas_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);

/**
 * @brief Get the dpi of the screen the Ecore_Evas is primarily on.
 *
 * @param ee The Ecore_Evas whose window's to query.
 * @param xdpi Pointer to integer to store horizontal DPI. May be @c NULL.
 * @param ydpi Pointer to integer to store vertical DPI. May be @c NULL.
 *
 * @since 1.7
 */
EAPI void        ecore_evas_screen_dpi_get(const Ecore_Evas *ee, int *xdpi, int *ydpi);

EAPI void        ecore_evas_draw_frame_set(Ecore_Evas *ee, Eina_Bool draw_frame);
EAPI Eina_Bool   ecore_evas_draw_frame_get(const Ecore_Evas *ee);

/**
 * @brief Associate the given object to this ecore evas.
 *
 * @param ee The Ecore_Evas to associate to @a obj
 * @param obj The object to associate to @a ee
 * @param flags The association flags.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
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
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool    ecore_evas_object_dissociate(Ecore_Evas *ee, Evas_Object *obj);
/**
 * @brief Get the object associated with @p ee
 *
 * @param ee The Ecore_Evas to get the object from.
 * @return The associated object, or @c NULL if there is no associated object.
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
/**
 * @brief Get a list of all the sub ecore_evases.
 *
 * @param ee Ecore_Evas to get the list from.
 * @return A list of sub ecore_evases, or @c NULL if there is no sub ecore_evases.
 */
EAPI Eina_List   *ecore_evas_sub_ecore_evas_list_get(const Ecore_Evas *ee);

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
 * @ingroup Ecore_Evas_Group
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
 * @param engine The engine to be set.
 * @param options The options of the engine to be set.
 * @return @c EINA_TRUE on success, @c EINA_FALSE if ews is already in use.
 * @since 1.1
 */
EAPI Eina_Bool ecore_evas_ews_engine_set(const char *engine, const char *options);

/**
 * Reconfigure the backing store used.
 *
 * @param x The X coordinate to be used.
 * @param y The Y coordinate to be used.
 * @param w The width of the Ecore_Evas to setup.
 * @param h The height of the Ecore_Evas to setup.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 * @since 1.1
 */
EAPI Eina_Bool ecore_evas_ews_setup(int x, int y, int w, int h);

/**
 * Return the internal backing store in use.
 *
 * @return The internal backing store in use.
 * @note this will forced it to be created, making future calls to
 * ecore_evas_ews_engine_set() void.
 *
 * @see ecore_evas_ews_evas_get()
 * @since 1.1
 */
EAPI Ecore_Evas *ecore_evas_ews_ecore_evas_get(void);

/**
 * Return the internal backing store in use.
 *
 * @return The internal backing store in use.
 * @note this will forced it to be created, making future calls to
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
 * @param o The Evas_Object for which to set the background.
 */
EAPI void ecore_evas_ews_background_set(Evas_Object *o);

/**
 * Return all Ecore_Evas* created by EWS.
 *
 * @return An eina list of Ecore_evases.
 e @note Do not change the returned list or its contents.
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

EAPI extern int ECORE_EVAS_EWS_EVENT_MANAGER_CHANGE; /**< manager was changed @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_ADD; /**< window was created @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_DEL; /**< window was deleted, pointer is already invalid but may be used as reference for further cleanup work. @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_RESIZE; /**< window was resized @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_MOVE; /**< window was moved @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_SHOW; /**< window become visible @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_HIDE; /**< window become hidden @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_FOCUS; /**< window was focused @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_UNFOCUS; /**< window lost focus @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_RAISE; /**< window was raised @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_LOWER; /**< window was lowered @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_ACTIVATE; /**< window was activated @since 1.1 */

EAPI extern int ECORE_EVAS_EWS_EVENT_ICONIFIED_CHANGE; /**< window minimized/iconified changed @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_MAXIMIZED_CHANGE; /**< window maximized changed @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_LAYER_CHANGE; /**< window layer changed @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_FULLSCREEN_CHANGE; /**< window fullscreen changed @since 1.1 */
EAPI extern int ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE; /**< some other window property changed (title, name, class, alpha, transparent, shaped...) @since 1.1 */

/**
 * @}
 */

/**
 * @defgroup Ecore_Evas_Extn External plug/socket infrastructure to remote canvases
 * @ingroup Ecore_Evas_Group
 *
 * These functions allow 1 process to create a "socket" was pluged into which another
 * process can create a "plug" remotely to plug into.
 * Socket can provides content for several plugs.
 * This is best for small sized objects (about the size range
 * of a small icon up to a few large icons). Sine the plug is actually an
 * image object, you can fetch the pixel data
 *
 * @since 1.2
 * @{
 */

EAPI extern int ECORE_EVAS_EXTN_CLIENT_ADD; /**< this event is received when a plug has connected to an extn socket @since 1.2 */
EAPI extern int ECORE_EVAS_EXTN_CLIENT_DEL; /**< this event is received when a plug has disconnected from an extn socket @since 1.2 */

/**
 * @brief Create a new Ecore_Evas canvas for the new external ecore evas socket
 *
 * @param w The width of the canvas, in pixels
 * @param h The height of the canvas, in pixels
 * @return A new @c Ecore_Evas instance or @c NULL, on failure
 *
 * This creates a new extn_socket canvas wrapper, with image data array
 * @b bound to the ARGB format, 8 bits per pixel.
 *
 * If creation is successful, an Ecore_Evas handle is returned or @c NULL if
 * creation fails. Also focus, show, hide etc. callbacks will also be called
 * if the plug object is shown, or already visible on connect, or if it is
 * hidden later, focused or unfocused.
 *
 * This function has to be flowed by ecore_evas_extn_socket_listen(),
 * for starting ecore ipc service.
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
 * When a client(plug) connects, you will get the ECORE_EVAS_EXTN_CLIENT_ADD event
 * in the ecore event queue, with event_info being the image object pointer
 * passed as a void pointer. When a client disconnects you will get the
 * ECORE_EVAS_EXTN_CLIENT_DEL event.
 *
 * You can set up event handles for these events as follows:
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
 * Note that events come in later after the event happened. You may want to be
 * careful as data structures you had associated with the image object
 * may have been freed after deleting, but the object may still be around
 * awating cleanup and thus still be valid.You can change the size with something like:
 *
 * @see ecore_evas_extn_socket_listen()
 * @see ecore_evas_extn_plug_new()
 * @see ecore_evas_extn_plug_object_data_lock()
 * @see ecore_evas_extn_plug_object_data_unlock()
 *
 * @since 1.2
 */
EAPI Ecore_Evas *ecore_evas_extn_socket_new(int w, int h);

/**
 * @brief Create a socket to provide the service for external ecore evas
 * socket.
 *
 * @param ee The Ecore_Evas.
 * @param svcname The name of the service to be advertised. ensure that it is
 * unique (when combined with @p svcnum) otherwise creation may fail.
 * @param svcnum A number (any value, @c 0 being the common default) to
 * differentiate multiple instances of services with the same name.
 * @param svcsys A boolean that if true, specifies to create a system-wide
 * service all users can connect to, otherwise the service is private to the
 * user ide that created the service.
 * @return @c EINA_TRUE if creation is successful, @c EINA_FALSE if it does
 * not.
 *
 * This creates socket specified by @p svcname, @p svcnum and @p svcsys. If
 * creation is successful, @c EINA_TRUE is returned or @c EINA_FALSE if
 * creation fails.
 *
 * @see ecore_evas_extn_socket_new()
 * @see ecore_evas_extn_plug_new()
 * @see ecore_evas_extn_plug_object_data_lock()
 * @see ecore_evas_extn_plug_object_data_unlock()
 *
 * @since 1.2
 */
EAPI Eina_Bool ecore_evas_extn_socket_listen(Ecore_Evas *ee, const char *svcname, int svcnum, Eina_Bool svcsys);

/**
 * @brief Lock the pixel data so the socket cannot change it
 *
 * @param obj The image object returned by ecore_evas_extn_plug_new() to lock
 *
 * You may need to get the image pixel data with evas_object_image_data_get()
 * from the image object, but need to ensure that it does not change while
 * you are using the data. This function lets you set an advisory lock on the
 * image data so the external plug process will not render to it or alter it.
 *
 * You should only hold the lock for just as long as you need to read out the
 * image data or otherwise deal with it, and then unlock it with
 * ecore_evas_extn_plug_object_data_unlock(). Keeping a lock over more than
 * 1 iteration of the main ecore loop will be problematic, so avoid it. Also
 * forgetting to unlock may cause the socket process to freeze and thus create
 * odd behavior.
 *
 * @see ecore_evas_extn_plug_new()
 * @see ecore_evas_extn_plug_object_data_unlock()
 *
 * @since 1.2
 */
EAPI void ecore_evas_extn_plug_object_data_lock(Evas_Object *obj);

/**
 * @brief Unlock the pixel data so the socket can change it again.
 *
 * @param obj The image object returned by ecore_evas_extn_plug_new() to unlock
 *
 * This unlocks after an advisor lock has been taken by
 * ecore_evas_extn_plug_object_data_lock().
 *
 * @see ecore_evas_extn_plug_new()
 * @see ecore_evas_extn_plug_object_data_lock()
 *
 * @since 1.2
 */
EAPI void ecore_evas_extn_plug_object_data_unlock(Evas_Object *obj);

/**
 * @brief Create a new external ecore evas plug
 *
 * @param ee_target The Ecore_Evas containing the canvas in which the new image object will live.
 * @return An evas image object that will contain the image output of a socket.
 *
 * This creates an image object that will contain the output of another
 * processes socket canvas when it connects. All input will be sent back to
 * this process as well, effectively swallowing or placing the socket process
 * in the canvas of the plug process in place of the image object. The image
 * object by default is created to be filled (equivalent of
 * evas_object_image_filled_add() on creation) so image content will scale
 * to fill the image unless otherwise reconfigured. The Ecore_Evas size
 * of the plug is the master size and determines size in pixels of the
 * plug canvas. You can change the size with something like:
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
 * @see ecore_evas_extn_socket_new()
 * @see ecore_evas_extn_plug_connect()
 * @since 1.2
 */
EAPI Evas_Object *ecore_evas_extn_plug_new(Ecore_Evas *ee_target);

/**
 * @brief Connect an external ecore evas plug to service provided by external
 * ecore evas socket.
 *
 * @param obj The Ecore_Evas containing the canvas in which the new image
 * object will live.
 * @param svcname The service name to connect to set up by the socket.
 * @param svcnum The service number to connect to (set up by socket).
 * @param svcsys Boolean to set if the service is a system one or not (set up
 * by socket).
 * @return @c EINA_TRUE if creation is successful, @c EINA_FALSE if it does
 * not.
 *
 * @see ecore_evas_extn_plug_new()
 *
 * @since 1.2
 */
EAPI Eina_Bool ecore_evas_extn_plug_connect(Evas_Object *obj, const char *svcname, int svcnum, Eina_Bool svcsys);

/**
 * @brief Retrieve the coordinates of the mouse pointer
 * 
 * @param ee The Ecore_Evas containing the pointer
 * @param x Pointer to integer to store horizontal coordinate. May be @c NULL.
 * @param y Pointer to integer to store vertical coordinate. May be @c NULL.
 * 
 * @since 1.8
 */
EAPI void ecore_evas_pointer_xy_get(const Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y);

/**
 * @brief Retrieve the coordinates of the mouse pointer
 * 
 * @param ee The Ecore_Evas containing the pointer
 * @param x The horizontal coordinate to move the pointer to
 * @param y The vertical coordinate to move the pointer to
 * 
 * @return @c EINA_TRUE on success, EINA_FALSE on failure.
 * 
 * @since 1.8
 */
EAPI Eina_Bool ecore_evas_pointer_warp(const Ecore_Evas *ee, Evas_Coord x, Evas_Coord y);

/**
 * @brief Retrieve the Visual used for pixmap creation
 * 
 * @param ee The Ecore_Evas containing the pixmap
 * 
 * @return The Visual which was used when creating the pixmap
 * 
 * @warning If and when this function is called depends on the underlying
 * windowing system. This function should only be called if the Ecore_Evas was
 * created using @c ecore_evas_software_x11_pixmap_new or @c ecore_evas_gl_x11_pixmap_new
 * 
 * @since 1.8
 */
EAPI void *ecore_evas_pixmap_visual_get(const Ecore_Evas *ee);

/**
 * @brief Retrieve the Colormap used for pixmap creation
 * 
 * @param ee The Ecore_Evas containing the pixmap
 * 
 * @return The Colormap which was used when creating the pixmap
 * 
 * @warning If and when this function is called depends on the underlying
 * windowing system. This function should only be called if the Ecore_Evas was
 * created using @c ecore_evas_software_x11_pixmap_new or @c ecore_evas_gl_x11_pixmap_new
 * 
 * @since 1.8
 */
EAPI unsigned long ecore_evas_pixmap_colormap_get(const Ecore_Evas *ee);

/**
 * @brief Retrieve the depth used for pixmap creation
 * 
 * @param ee The Ecore_Evas containing the pixmap
 * 
 * @return The depth which was used when creating the pixmap
 * 
 * @warning If and when this function is called depends on the underlying
 * windowing system. This function should only be called if the Ecore_Evas was
 * created using @c ecore_evas_software_x11_pixmap_new or @c ecore_evas_gl_x11_pixmap_new
 * 
 * @since 1.8
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
