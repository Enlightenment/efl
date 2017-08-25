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
 * @brief Evas wrapper functions.
 *
 * The following is a list of examples that partially exemplify Ecore_Evas's API:
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
 * Ecore evas is a set of functions that make it easy to tie together ecore's
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

/* these are dummy and just tell you what API levels ecore_evas supports - not if
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
#define HAVE_ECORE_EVAS_DRM_GL 1

typedef enum _Ecore_Evas_Engine_Type
{
   ECORE_EVAS_ENGINE_SOFTWARE_BUFFER,
   ECORE_EVAS_ENGINE_SOFTWARE_XLIB,
   ECORE_EVAS_ENGINE_XRENDER_X11,
   ECORE_EVAS_ENGINE_OPENGL_X11,
   ECORE_EVAS_ENGINE_SOFTWARE_XCB, /* @deprecated */
   ECORE_EVAS_ENGINE_XRENDER_XCB, /* @deprecated */
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
   ECORE_EVAS_ENGINE_DRM,
   ECORE_EVAS_ENGINE_OPENGL_DRM
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
 * @brief Inits the Ecore_Evas system.
 *
 * @return How many times the lib has been initialized, @c 0 indicates failure.
 *
 * Set up the Evas wrapper system. Init Evas and Ecore libraries.
 *
 * @see ecore_evas_shutdown()
 */
EAPI int         ecore_evas_init(void);

/**
 * @brief Shuts down the Ecore_Evas system.
 *
 * @return @c 0 if ecore evas is fully shut down, or > @c 0 if it still being used.
 *
 * This closes the Evas wrapper system down. Shut down Evas and Ecore libraries.
 *
 * @see ecore_evas_init()
 */
EAPI int         ecore_evas_shutdown(void);

/**
 * @brief Sets application compositor synchronization on/off
 *
 * Turns on client+server synchronized rendering in X11.  App comp sync
 * is disabled by default, but can be turned on optionally.
 *
 * @note This is an experimental functionality and is likely to be removed.
 *
 * @param do_sync True to enable comp syncing, False to disable
 */
EAPI void        ecore_evas_app_comp_sync_set(Eina_Bool do_sync);

/**
 * @brief Get the compositing synchronization state
 *
 * @return True if app comp sync is enabled.
 *
 * @note This is an experimental functionality and is likely to be removed.
 *
 */
EAPI Eina_Bool   ecore_evas_app_comp_sync_get(void);

/**
 * @brief Returns a list of supported engine names.
 *
 * @return Newly allocated list with engine names. Engine names
 * strings are internal and should be considered constants, do not
 * free or modify them, to free the list use ecore_evas_engines_free().
 */
EAPI Eina_List  *ecore_evas_engines_get(void);

/**
 * @brief Free list returned by ecore_evas_engines_get()
 *
 * @param engines List with engine names
 */
EAPI void        ecore_evas_engines_free(Eina_List *engines);

/**
 * @brief Creates a new Ecore_Evas based on engine name and common parameters.
 *
 * @param engine_name Engine name as returned by
 *        ecore_evas_engines_get() or @c NULL to use environment variable
 *        ECORE_EVAS_ENGINE, which can be undefined and in this case
 *        this call will try to find the first working engine.
 * @param x Horizontal position of window (not supported in all engines)
 * @param y Vertical position of window (not supported in all engines)
 * @param w Width of window
 * @param h Height of window
 * @param extra_options String with extra parameter, dependent on engines
 *        or @ NULL. String is usually in the form: 'key1=value1;key2=value2'.
 *        Pay attention that when getting this from shell commands, most
 *        consider ';' as the command terminator, so you need to escape
 *        it or use quotes.
 * @return Ecore_Evas instance or @c NULL if creation failed.
 */
EAPI Ecore_Evas *ecore_evas_new(const char *engine_name, int x, int y, int w, int h, const char *extra_options);

/**
 * @brief Sets whether an Ecore_Evas has an alpha channel or not.
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
 * @brief Queries whether an Ecore_Evas has an alpha channel.
 *
 * @param ee The Ecore_Evas to query.
 *
 * @return @c EINA_TRUE if ee has an alpha channel, @c EINA_FALSE if it does
 * not.
 *
 * @see ecore_evas_alpha_set()
 */
EAPI Eina_Bool   ecore_evas_alpha_get(const Ecore_Evas *ee);

/**
 * @brief Sets whether an Ecore_Evas has an transparent window or not.
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
 * @brief Queries whether an Ecore_Evas is transparent.
 *
 * @param ee The Ecore_Evas to query.
 * @return @c EINA_TRUE if ee is transparent, @c EINA_FALSE if it isn't.
 *
 * @see ecore_evas_transparent_set()
 */
EAPI Eina_Bool   ecore_evas_transparent_get(const Ecore_Evas *ee);

/**
 * @brief Gets the geometry of an Ecore_Evas.
 *
 * @param ee The Ecore_Evas whose geometry y
 * @param x A pointer to an int to place the x coordinate in
 * @param y A pointer to an int to place the y coordinate in
 * @param w A pointer to an int to place the w size in
 * @param h A pointer to an int to place the h size in
 *
 * This function takes four pointers to (already allocated) ints, and places
 * the geometry of @p ee in them. If any of the parameters are not desired you
 * may pass @c NULL for them.
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
 * @brief Gets the geometry which an Ecore_Evas was latest recently requested.
 *
 * @param ee The Ecore_Evas whose geometry y
 * @param x A pointer to an int to place the x coordinate in
 * @param y A pointer to an int to place the y coordinate in
 * @param w A pointer to an int to place the w size in
 * @param h A pointer to an int to place the h size in
 *
 * This function takes four pointers to (already allocated) ints, and places
 * the geometry which @p ee was latest recently requested . If any of the
 * parameters are not desired you may pass @c NULL for them.
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
 * @brief Sets the Ecore_Evas window focus for the default seat.
 *
 * @param ee The Ecore_Evas
 * @param on @c EINA_TRUE for focus, @c EINA_FALSE to defocus.
 *
 * This function focuses @p ee if @p on is @c EINA_TRUE, or unfocuses @p ee if
 * @p on is @c EINA_FALSE.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @see ecore_evas_focus_device_set()
 */
EAPI void        ecore_evas_focus_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * @brief Queries whether the default seat has the Ecore_Evas focus.
 *
 * @param ee The Ecore_Evas to set
 * @return @c EINA_TRUE if @p ee if focused, @c EINA_FALSE if not.
 *
 * @see ecore_evas_focus_set()
 * @see ecore_evas_focus_device_get()
 */
EAPI Eina_Bool   ecore_evas_focus_get(const Ecore_Evas *ee);

/**
 * @brief Sets the Ecore_Evas windows focus for a given seat.
 *
 * @param ee The Ecore_Evas
 * @param seat An Efl_Input_Device that represents the seat or @c NULL for the default seat.
 * @param on @c EINA_TRUE for focus, @c EINA_FALSE to defocus.
 *
 * This function focuses @p ee if @p on is @c EINA_TRUE, or unfocuses @p ee if
 * @p on is @c EINA_FALSE.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @see ecore_evas_focus_device_get()
 * @since 1.19
 */
EAPI void        ecore_evas_focus_device_set(Ecore_Evas *ee, Eo *seat,
                                             Eina_Bool on);
/**
 * @brief Queries whether an Ecore_Evas' window is focused or not.
 *
 * @param ee The Ecore_Evas to set
 * @param seat An Efl_Input_Device that represents the seat or @c NULL for the default seat.
 * @return @c EINA_TRUE if @p ee if focused, @c EINA_FALSE if not.
 *
 * @see ecore_evas_focus_device_set()
 * @since 1.19
 */
EAPI Eina_Bool   ecore_evas_focus_device_get(const Ecore_Evas *ee, Eo *seat);

/**
 * @brief Iconifies or uniconifies an Ecore_Evas' window.
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
 * @brief Queries whether an Ecore_Evas' window is iconified or not.
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
 * @brief Sets whether an Ecore_Evas' window is borderless or not.
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
 * @brief Queries whether an Ecore_Evas' window is borderless or not.
 *
 * @param ee The Ecore_Evas to set
 * @return @c EINA_TRUE if @p ee is borderless, @c EINA_FALSE if not.
 *
 * @see ecore_evas_borderless_set()
 */
EAPI Eina_Bool   ecore_evas_borderless_get(const Ecore_Evas *ee);

/**
 * @brief Sets whether or not an Ecore_Evas' window is fullscreen.
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
 * @brief Queries whether an Ecore_Evas' window is fullscreen or not.
 *
 * @param ee The Ecore_Evas to set
 * @return @c EINA_TRUE if @p ee is fullscreen, @c EINA_FALSE if not.
 *
 * @see ecore_evas_fullscreen_set()
 */
EAPI Eina_Bool   ecore_evas_fullscreen_get(const Ecore_Evas *ee);

/**
 * @brief Sets another window that this window is a group member of.
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
 * @brief Gets the canvas group set.
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
 * @brief Sets the aspect ratio of a canvas window.
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
 * @brief Gets the aspect ratio of a canvas window.
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
 * @brief Sets The urgent hint flag.
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
 * @brief Gets the urgent state on the canvas window.
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
 * @brief Sets the modal state flag on the canvas window.
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
 * @brief Gets The modal flag.
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
 * @brief Sets the "i demand attention" flag on a canvas window.
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
 * @brief Gets the "i demand attention" flag.
 *
 * This returns the value set by ecore_evas_demand_attention_set().
 *
 * @param ee The Ecore_Evas to set
 * @return The "i demand attention" flag
 *
 * @see ecore_evas_demand_attention_set()
 * @since 1.2
 */
EAPI Eina_Bool   ecore_evas_demand_attention_get(const Ecore_Evas *ee);

/**
 * @brief Sets the "focus skip" flag.
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
 * @brief Gets the "focus skip" flag.
 *
 * This returns the value set by ecore_evas_focus_skip_set().
 *
 * @param ee The Ecore_Evas to set
 * @return The "focus skip" flag
 *
 * @see ecore_evas_focus_skip_set()
 * @since 1.2
 */
EAPI Eina_Bool   ecore_evas_focus_skip_get(const Ecore_Evas *ee);

/**
 * @brief Sets if this evas should ignore @b all events.
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
 * @brief Queries whether an Ecore_Evas' window is visible or not.
 *
 * @param ee The Ecore_Evas to query.
 * @return @c 1 if @p ee is visible, @c 0 if not.
 *
 * @see ecore_evas_show()
 * @see ecore_evas_hide()
 */
EAPI int         ecore_evas_visibility_get(const Ecore_Evas *ee);

/**
 * @brief Sets the layer of an Ecore_Evas' window.
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
 * @brief Gets the layer of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas to set
 * @return The layer @p ee's window is on
 *
 * @see ecore_evas_layer_set()
 * @see ecore_evas_lower()
 * @see ecore_evas_raise()
 */
EAPI int         ecore_evas_layer_get(const Ecore_Evas *ee);

/**
 * @brief Maximizes (or unmaximizes) an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas
 * @param on @c EINA_TRUE to maximize, @c EINA_FALSE to unmaximize
 *
 * This function maximizes @p ee if @p on is @c EINA_TRUE, or unmaximizes @p ee
 * if @p on is @c EINA_FALSE.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_maximized_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * @brief Queries whether an Ecore_Evas' window is maximized or not.
 *
 * @param ee The Ecore_Evas to set
 * @return @c EINA_TRUE if @p ee is maximized, @c EINA_FALSE if not.
 *
 * @see ecore_evas_maximized_set()
 */
EAPI Eina_Bool   ecore_evas_maximized_get(const Ecore_Evas *ee);

/**
 * @brief Queries if the underlying windowing system supports the window profile.
 *
 * @param ee The Ecore_Evas
 * @return @c EINA_TRUE if the window profile is supported, @c EINA_FALSE otherwise.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 */
EAPI Eina_Bool   ecore_evas_window_profile_supported_get(const Ecore_Evas *ee);

/**
 * @brief Sets the window profile.
 *
 * @param ee The Ecore_Evas to set
 * @param profile The string value of the window profile
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 */
EAPI void        ecore_evas_window_profile_set(Ecore_Evas *ee, const char *profile);

/**
 * @brief Gets the window profile.
 *
 * @param ee The Ecore_Evas to get the window profile from.
 * @return The string value of the window profile, or NULL if none exists
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 */
EAPI const char *ecore_evas_window_profile_get(const Ecore_Evas *ee);

/**
 * @brief Sets the array of available window profiles.
 *
 * @param ee The Ecore_Evas to set
 * @param profiles The string array of available window profiles
 * @param count The number of members in profiles
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 */
EAPI void        ecore_evas_window_available_profiles_set(Ecore_Evas *ee, const char **profiles, const unsigned int count);

/**
 * @brief Gets the array of available window profiles.
 *
 * @param ee The Ecore_Evas to get available window profiles from.
 * @param profiles Where to return the string array of available window profiles
 * @param count Where to return the number of members in profiles
 * @return @c EINA_TRUE if available window profiles exist, @c EINA_FALSE otherwise
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 */
EAPI Eina_Bool   ecore_evas_window_available_profiles_get(Ecore_Evas *ee, char ***profiles, unsigned int *count);

/**
 * @brief Queries if the underlying windowing system supports the window manager rotation.
 *
 * @param ee The Ecore_Evas
 * @return @c EINA_TRUE if the window manager rotation is supported, @c EINA_FALSE otherwise.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.9.0
 */
EAPI Eina_Bool   ecore_evas_wm_rotation_supported_get(const Ecore_Evas *ee);

/**
 * @brief Sets the preferred rotation hint.
 *
 * @param ee The Ecore_Evas to set
 * @param rotation Value to set the preferred rotation hint
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.9.0
 */
EAPI void        ecore_evas_wm_rotation_preferred_rotation_set(Ecore_Evas *ee, int rotation);

/**
 * @brief Gets the preferred rotation hint.
 *
 * @param ee The Ecore_Evas to get the preferred rotation hint from.
 * @return The preferred rotation hint, @c -1 on failure.
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.9.0
 */
EAPI int         ecore_evas_wm_rotation_preferred_rotation_get(const Ecore_Evas *ee);

/**
 * @brief Sets the array of available window rotations.
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
 * @brief Gets the array of available window rotations.
 *
 * @param ee The Ecore_Evas to get available window rotations from.
 * @param rotations Where to return the integer array of available window rotations
 * @param count Where to return the number of members in rotations
 * @return @c EINA_TRUE if available window rotations exist, @c EINA_FALSE otherwise
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.9.0
 */
EAPI Eina_Bool   ecore_evas_wm_rotation_available_rotations_get(const Ecore_Evas *ee, int **rotations, unsigned int *count);

/**
 * @brief Sets manual rotation done mode of Ecore_Evas's window.
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
 * @brief Gets manual rotation done mode of Ecore_Evas's window.
 *
 * @param ee The Ecore_Evas
 * @return If true, the manual rotation done mode is enabled
 *
 * @since 1.9.0
 */
EAPI Eina_Bool   ecore_evas_wm_rotation_manual_rotation_done_get(const Ecore_Evas *ee);

/**
 * @brief Sets rotation finish manually.
 *
 * @param ee The Ecore_Evas
 *
 * @since 1.9.0
 */
EAPI void        ecore_evas_wm_rotation_manual_rotation_done(Ecore_Evas *ee);

/**
 * @brief Gets the list of supported auxiliary hint strings.
 *
 * @param ee The Ecore_Evas
 * @return List of supported auxiliary hint strings.
 *
 * @note Do not change the returned list of its contents. Auxiliary hint
 * strings are internal and should be considered constants, do not free or
 * modify them.
 * @warning Support for this depends on the underlying windowing system.
 *
 * The window auxiliary hint is the value which is used to decide which actions should
 * be made available to the user by the window manager. If you want to set a specific hint
 * on your window, then you should check whether it exists in the supported auxiliary
 * hints that are registered in the root window by the window manager. Once you've added
 * an auxiliary hint, you can get a new ID which is used to change value and delete hint.
 * The window manager sends the response message to the application on receiving auxiliary
 * hint change event. A list of auxiliary hints within the Ecore_Evas has this format:
 * ID:HINT:VALUE,ID:HINT:VALUE,...
 *
 * @since 1.10.0
 */
EAPI const Eina_List *ecore_evas_aux_hints_supported_get(const Ecore_Evas *ee);

/**
 * @brief Gets the list of allowed auxiliary hint IDs.
 *
 * @param ee The Ecore_Evas
 * @return List of allowed auxiliary hint IDs.
 *
 * @note This function is low level. Instead of using it directly, consider
 * using the callback mechanism in Elementary such as "aux,hint,allowed".
 * @warning Support for this depends on the underlying windowing system.
 *
 * @since 1.10.0
 */
EAPI Eina_List       *ecore_evas_aux_hints_allowed_get(const Ecore_Evas *ee);

/**
 * @brief Creates an auxiliary hint of the Ecore_Evas.
 *
 * @param ee The Ecore_Evas
 * @param hint The auxiliary hint string.
 * @param val The value string.
 * @return The ID of created auxiliary hint, or @c -1 on failure.
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @since 1.10.0
 */
EAPI int              ecore_evas_aux_hint_add(Ecore_Evas *ee, const char *hint, const char *val);

/**
 * @brief Deletes an auxiliary hint of the Ecore_Evas.
 *
 * @param ee The Ecore_Evas
 * @param id The ID of the auxiliary hint.
 * @return @c EINA_TRUE if no error occurred, @c EINA_FALSE otherwise.
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @since 1.10.0
 */
EAPI Eina_Bool        ecore_evas_aux_hint_del(Ecore_Evas *ee, const int id);

/**
 * @brief Changes a value of the auxiliary hint.
 *
 * @param ee The Ecore_Evas
 * @param id The auxiliary hint ID.
 * @param val The value string to be set.
 * @return @c EINA_TRUE if no error occurred, @c EINA_FALSE otherwise.
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @since 1.10.0
 */
EAPI Eina_Bool        ecore_evas_aux_hint_val_set(Ecore_Evas *ee, const int id, const char *val);

/**
 * @brief Gets a value of the auxiliary hint.
 *
 * @param ee The Ecore_Evas
 * @param id The auxiliary hint ID.
 * @return The string value of the auxiliary hint ID, or NULL if none exists
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @since 1.15
 */
EAPI const char      *ecore_evas_aux_hint_val_get(const Ecore_Evas *ee, int id);

/**
 * @brief Gets a ID of the auxiliary hint string.
 *
 * @param ee The Ecore_Evas
 * @param hint The auxiliary hint string.
 * @return The ID of the auxiliary hint string, or @c -1 if none exists
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @since 1.15
 */
EAPI int              ecore_evas_aux_hint_id_get(const Ecore_Evas *ee, const char *hint);

/**
 * @brief Sends message to parent ecore.
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
 * @brief Sends message to child ecore.
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
 * @brief Sets a callback for parent Ecore_Evas message.
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
 * @brief Sets a callback for child Ecore_Evas message.
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
 * @brief Moves an Ecore_Evas.
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
 * @brief Resizes an Ecore_Evas.
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
 * @brief Moves and resizes an Ecore_Evas.
 *
 * @param ee The Ecore_Evas to move and resize
 * @param x The x coordinate to move to
 * @param y The y coordinate to move to
 * @param w The w coordinate to resize to
 * @param h The h coordinate to resize to
 *
 * This moves @p ee to the screen coordinates (@p x, @p y) and resizes
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
 * @brief Sets the rotation of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas
 * @param rot The angle (in degrees) of rotation
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
 * @brief Sets the rotation of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas
 * @param rot The angle (in degrees) of rotation.
 *
 * Like ecore_evas_rotation_set(), but it also resizes the window's content so
 * that it fits inside the current window geometry.
 *
 * @warning Support for this depends on the underlying windowing system.
 *
 * @see ecore_evas_rotation_set()
 */
EAPI void        ecore_evas_rotation_with_resize_set(Ecore_Evas *ee, int rot);

/**
 * @brief Gets the rotation of an Ecore_Evas' window
 *
 * @param ee The Ecore_Evas
 * @return The angle (in degrees) of rotation
 *
 * @see ecore_evas_rotation_set()
 * @see ecore_evas_rotation_with_resize_set()
 */
EAPI int         ecore_evas_rotation_get(const Ecore_Evas *ee);

/**
 * @brief Raises an Ecore_Evas' window.
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
 * @brief Lowers an Ecore_Evas' window.
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
 * @brief Sets the title of an Ecore_Evas' window.
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
 * @brief Gets the title of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas whose title you wish to get.
 * @return The title of @p ee.
 *
 * @see ecore_evas_title_set()
 */
EAPI const char *ecore_evas_title_get(const Ecore_Evas *ee);

/**
 * @brief Sets the name and class of an Ecore_Evas' window.
 *
 * @param ee the Ecore_Evas
 * @param n The name
 * @param c The class
 *
 * This function sets the name of @p ee to @p n, and its class to @p c. The
 * meaning of @p name and @p class depends on the underlying windowing system.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI void        ecore_evas_name_class_set(Ecore_Evas *ee, const char *n, const char *c);

/**
 * @brief Gets the name and class of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas to query.
 * @param n A pointer to a string to place the name in.
 * @param c A pointer to a string to place the class in.
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
 * @return A pointer to the underlying window.
 *
 * @warning Support for this depends on the underlying windowing system.
 */
EAPI Ecore_Window ecore_evas_window_get(const Ecore_Evas *ee);

/* engine/target specific init calls */

/**
 * @brief Creates Ecore_Evas using software x11.
 *
 * @param disp_name The name of the Ecore_Evas to be created.
 * @param parent The parent of the Ecore_Evas to be created.
 * @param x The X coordinate to be used.
 * @param y The Y coordinate to be used.
 * @param w The width of the Ecore_Evas to be created.
 * @param h The height of the Ecore_Evas to be created.
 * @return A handle to the created Ecore_Evas.
 *
 * @note If ecore is not compiled with support for x11 then nothing is done and NULL is returned.
 */
EAPI Ecore_Evas     *ecore_evas_software_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);

/**
 * @brief Gets the window from Ecore_Evas using software x11.
 *
 * @param ee The Ecore_Evas from which to get the window.
 * @return The window of type Ecore_X_Window.
 *
 * @note If ecore is not compiled with support for x11 or if @p ee was not
 * created with ecore_evas_software_x11_new() then nothing is done and
 * 0 is returned.
 */
EAPI Ecore_X_Window  ecore_evas_software_x11_window_get(const Ecore_Evas *ee);

/**
 * @brief Sets the direct_resize of Ecore_Evas using software x11.
 *
 * @param ee The Ecore_Evas in which to set direct resize.
 * @param on Enables the resize of Ecore_Evas if equals @c EINA_TRUE, disables if equals @c EINA_FALSE.
 *
 * @note If ecore is not compiled with support for x11 then nothing is done.
 */
EAPI void            ecore_evas_software_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * @brief Gets if the Ecore_Evas is being directly resized using software x11.
 *
 * @param ee The Ecore_Evas from which to get direct resize.
 * @return @c EINA_TRUE if the resize was managed directly, otherwise return @c EINA_FALSE.
 *
 * @note If ecore is not compiled with support for x11 then nothing is done and EINA_FALSE is returned.
 */
EAPI Eina_Bool       ecore_evas_software_x11_direct_resize_get(const Ecore_Evas *ee);

/**
 * @brief Adds an extra window on Ecore_Evas using software x11.
 *
 * @param ee The Ecore_Evas for which to add the window.
 * @param win The window to be added to the Ecore_Evas.
 *
 * @note If ecore is not compiled with support for x11 then nothing is done.
 */
EAPI void            ecore_evas_software_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);

/**
 * @brief Creates a new Ecore_Evas which does not contain an XWindow. It will
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
 * @brief Returns the underlying Ecore_X_Pixmap used in the Ecore_Evas.
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
#define ECORE_EVAS_GL_X11_OPT_GL_DEPTH     4
#define ECORE_EVAS_GL_X11_OPT_GL_STENCIL   5
#define ECORE_EVAS_GL_X11_OPT_GL_MSAA      6
#define ECORE_EVAS_GL_X11_OPT_LAST         7

#define ECORE_EVAS_GL_X11_SWAP_MODE_AUTO   0
#define ECORE_EVAS_GL_X11_SWAP_MODE_FULL   1
#define ECORE_EVAS_GL_X11_SWAP_MODE_COPY   2
#define ECORE_EVAS_GL_X11_SWAP_MODE_DOUBLE 3
#define ECORE_EVAS_GL_X11_SWAP_MODE_TRIPLE 4

/**
 * @brief Creates Ecore_Evas using opengl x11.
 *
 * @param disp_name The name of the display of the Ecore_Evas to be created.
 * @param parent The parent of the Ecore_Evas to be created.
 * @param x The X coordinate to be used.
 * @param y The Y coordinate to be used.
 * @param w The width of the Ecore_Evas to be created.
 * @param h The height of the Ecore_Evas to be created.
 * @return The new Ecore_Evas.
 *
 * @note If ecore is not compiled with support for x11 then nothing is done and NULL is returned.
 */
EAPI Ecore_Evas     *ecore_evas_gl_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);

/**
 * @brief Create Ecore_Evas using OpenGL X11, with options
 *
 * @param disp_name The name of the display of the Ecore_Evas to be created.
 * @param parent The parent of the Ecore_Evas to be created.
 * @param x The X coordinate to be used.
 * @param y The Y coordinate to be used.
 * @param w The width of the Ecore_Evas to be created.
 * @param h The height of the Ecore_Evas to be created.
 * @param opt The options array (see above)
 * @return The new Ecore_Evas.
 *
 * Allows creation of an Ecore_Evas, similar to ecore_evas_gl_x11_new,
 * except it permits passing in @p opt, a NULL-terminated C array of
 * key/value pairs for various settings, as follows:
 *
 *   ECORE_EVAS_GL_X11_OPT_INDIRECT:  Use indirect rendering [0,1]
 *   ECORE_EVAS_GL_X11_OPT_VSYNC: Use vsync [0,1]
 *   ECORE_EVAS_GL_X11_OPT_SWAP_MODE: Swap mode to assume (see Evas_Engine_Info_Gl_Swap_Mode)
 *   ECORE_EVAS_GL_X11_OPT_GL_DEPTH: depth_bits
 *   ECORE_EVAS_GL_X11_OPT_GL_STENCIL: stencil_bits
 *   ECORE_EVAS_GL_X11_OPT_GL_MSAA: msaa_bits
 *
 * @note If ecore is not compiled with support for x11 then nothing is done and NULL is returned.
 */
EAPI Ecore_Evas     *ecore_evas_gl_x11_options_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h, const int *opt);

/**
 * @brief Gets the window from Ecore_Evas using opengl x11.
 *
 * @param ee The Ecore_Evas from which to get the window.
 * @return The window of type Ecore_X_Window of Ecore_Evas.
 *
 * @note If ecore is not compiled with support for x11 or if @p ee was not
 * created with ecore_evas_gl_x11_new() then nothing is done and
 * 0 is returned.
 */
EAPI Ecore_X_Window  ecore_evas_gl_x11_window_get(const Ecore_Evas *ee);

/**
 * @brief Sets direct_resize for Ecore_Evas using opengl x11.
 *
 * @param ee The Ecore_Evas in which to set direct resize.
 * @param on Enables the resize of Ecore_Evas if equals @c EINA_TRUE, disables if equals @c EINA_FALSE.
 *
 * @note If ecore is not compiled with support for x11 then nothing is done.
 */
EAPI void            ecore_evas_gl_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * @brief Gets if the Ecore_Evas is being directly resized using opengl x11.
 *
 * @param ee The Ecore_Evas from which to get direct resize.
 *
 * @note If ecore is not compiled with support for x11 then nothing is done and EINA_FALSE is returned.
 *
 * @return @c EINA_TRUE if the resize was managed directly, otherwise return @c EINA_FALSE.
 */
EAPI Eina_Bool       ecore_evas_gl_x11_direct_resize_get(const Ecore_Evas *ee);

/**
 * @brief Adds extra window on Ecore_Evas using opengl x11.
 *
 * @param ee The Ecore_Evas for which to add the window.
 * @param win The window to be added to the Ecore_Evas.
 *
 * @note If ecore is not compiled with support for x11 then nothing is done.
 */
EAPI void            ecore_evas_gl_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win);

/**
 * @brief Sets the functions to be used before and after the swap callback.
 *
 * @param ee The Ecore_Evas for which to set the swap callback.
 * @param data The data for which to set the swap callback.
 * @param pre_cb The function to be called before the callback.
 * @param post_cb The function to be called after the callback.
 *
 * @note If ecore is not compiled with support for x11 then nothing is done and the function is returned.
 */
EAPI void            ecore_evas_gl_x11_pre_post_swap_callback_set(const Ecore_Evas *ee, void *data, void (*pre_cb) (void *data, Evas *e), void (*post_cb) (void *data, Evas *e));

/**
 * @brief Creates a new Ecore_Evas which does not contain an XWindow. It will
 * only contain an XPixmap to render to.
 *
 * @warning The XPixmap ID can change with every frame after it is rendered,
 * so you should ALWAYS call ecore_evas_software_x11_pixmap_get when you
 * need the current pixmap id.
 *
 * @since 1.8
 */
EAPI Ecore_Evas     *ecore_evas_gl_x11_pixmap_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h);

/**
 * @brief Returns the underlying Ecore_X_Pixmap used in the Ecore_Evas.
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

/**
 * @brief Creates Ecore_Evas using fb backend.
 *
 * @param disp_name The name of the display to be used.
 * @param rotation The rotation to be used.
 * @param w The width of the Ecore_Evas to be created.
 * @param h The height of the Ecore_Evas to be created.
 * @return The new Ecore_Evas.
 */
EAPI Ecore_Evas     *ecore_evas_fb_new(const char *disp_name, int rotation, int w, int h);

EAPI Ecore_Evas     *ecore_evas_directfb_new(const char *disp_name, int windowed, int x, int y, int w, int h) EINA_DEPRECATED;
EAPI Ecore_DirectFB_Window *ecore_evas_directfb_window_get(const Ecore_Evas *ee) EINA_DEPRECATED;

/**
 * @brief Create an Ecore_Evas window using the wayland-shm engine.
 *
 * @param disp_name Name of the Wayland display to connect to.
 * @param parent ID of the parent window this Ecore_Evas window belongs
 *        to, or 0 if this is a top-level window.
 * @param x Horizontal position of the Ecore_Evas window.
 * @param y Vertical position of the Ecore_Evas window.
 * @param w Width of the Ecore_Evas window to be created.
 * @param h Height of the Ecore_Evas window to be created.
 * @param frame Deprecated.  (Just pass EINA_FALSE.)
 * @return Ecore_Evas instance or @c NULL if creation failed.
 *
 * Wayland's Shared Memory (SHM) rendering model stores client-drawn
 * data in system memory buffers for communication with the display
 * server.  The server then copies the buffer from system memory to the
 * video output as appropriate.  This routine constructs an Ecore_Evas
 * window object that uses SHM buffers for communication using the
 * Wayland protocol.
 *
 * @see ecore_evas_new()
 * @see ecore_evas_wayland_egl_new()
 */
EAPI Ecore_Evas     *ecore_evas_wayland_shm_new(const char *disp_name, unsigned int parent, int x, int y, int w, int h, Eina_Bool frame);

/**
 * @brief Create an Ecore_Evas window using the wayland-egl engine.
 *
 * @param disp_name Name of the Wayland display to connect to.
 * @param parent ID of the parent window this Ecore_Evas window belongs
 *        to, or 0 if this is a top-level window.
 * @param x Horizontal position of the Ecore_Evas window.
 * @param y Vertical position of the Ecore_Evas window.
 * @param w Width of the Ecore_Evas window to be created.
 * @param h Height of the Ecore_Evas window to be created.
 * @param frame Deprecated.  (Just pass EINA_FALSE.)
 * @return Ecore_Evas instance or @c NULL if creation failed.
 *
 * Under Wayland, clients can also set up rendering directly to graphics
 * hardware memory via various rendering APIs such as OpenGL, OpenGL ES,
 * etc.) thus skipping the need for server-side copying of graphics data
 * to the display device, thus providing higher performance than SHM.
 * This routine constructs an Ecore_Evas window object using the
 * Embedded-system Graphics Library (EGL) interface for handling the
 * context management and rendering synchronization.
 *
 * @see ecore_evas_new()
 * @see ecore_evas_wayland_shm_new()
 */
EAPI Ecore_Evas     *ecore_evas_wayland_egl_new(const char *disp_name, unsigned int parent, int x, int y, int w, int h, Eina_Bool frame);

/**
 * @brief Begin resizing the Ecore_Evas window.
 *
 * @param ee The Ecore_Evas.
 * @param location The edge of the window from which the resize should start.
 *
 * The location edge is an enum as defined by the XDG Shell protocol.  See
 * the zxdg_toplevel_v6_resize_edge enum definition for edge numbering.
 */
EAPI void            ecore_evas_wayland_resize(Ecore_Evas *ee, int location);

/**
 * @brief Begin moving the Ecore_Evas window.
 *
 * @param ee The Ecore_Evas.
 * @param x (unused)
 * @param y (unused)
 */
EAPI void            ecore_evas_wayland_move(Ecore_Evas *ee, int x, int y);

/**
 * @brief Set the pointer for the Ecore_Evas window.
 *
 * @param ee The Ecore_Evas.
 * @param hot_x X coordinate.
 * @param hot_y Y coordinate.
 *
 * @note: This routine is unimplemented.
 */
EAPI void            ecore_evas_wayland_pointer_set(Ecore_Evas *ee, int hot_x, int hot_y);

/**
 * @brief Set the type of the Ecore_Evas window.
 *
 * Enables setting the window as top level, menu, dnd, etc.
 *
 * @param ee The Ecore_Evas.
 * @param type The Ecore_Wl2_Window_Type to set on the window.
 *
 * @see ecore_wl2_window_type_set()
 * @see Ecore_Wl2_Window_Type
 */
EAPI void            ecore_evas_wayland_type_set(Ecore_Evas *ee, int type);

/**
 * @brief Gets the window from Ecore_Evas using the wayland backend.
 *
 * @param ee The Ecore_Evas from which to get the window.
 * @return The window of type Ecore_X_Window.
 *
 * @note If ecore is not compiled with support for wayland or if @p ee
 * was not created with ecore_evas_wayland_shm_new() or
 * ecore_evas_wayland_egl_new() then nothing is done and NULL is returned.
 */
EAPI Ecore_Wl_Window *ecore_evas_wayland_window_get(const Ecore_Evas *ee);

/**
 * @brief Gets the window from Ecore_Evas using the wayland2 backend.
 *
 * @param ee The Ecore_Evas from which to get the window.
 * @return The window of type Ecore_X_Window.
 *
 * @note If ecore is not compiled with support for wayland or if @p ee
 * was not created with ecore_evas_wayland_shm_new() or
 * ecore_evas_wayland_egl_new() then nothing is done and NULL is returned.
 *
 * @since 1.17
 */
EAPI Ecore_Wl2_Window *ecore_evas_wayland2_window_get(const Ecore_Evas *ee);

/**
 * @brief Gets the window from Ecore_Evas using the opengl_cocoa backend.
 *
 * @param ee The Ecore_Evas from which to get the window.
 * @return The window of type Ecore_X_Window.
 *
 * @note If ecore is not compiled with support for opengl_cocoa or if @p
 * ee was not created with ecore_evas_cocoa_new() then nothing is done
 * and NULL is returned.
 *
 * @since 1.17
 */
EAPI Ecore_Cocoa_Window *ecore_evas_cocoa_window_get(const Ecore_Evas *ee);

/**
 * @brief Create an Ecore_Evas window using the drm engine.
 *
 * @param disp_name Name of the Wayland display to connect to.
 * @param parent (Unused)
 * @param x Horizontal position of the Ecore_Evas window.
 * @param y Vertical position of the Ecore_Evas window.
 * @param w Width of the Ecore_Evas window to be created.
 * @param h Height of the Ecore_Evas window to be created.
 * @return Ecore_Evas instance or @c NULL if creation failed.
 *
 * Creates an Ecore_Evas window using the Direct Rendering Manager (DRM)
 * backend.  DRM is a kernel-level module providing direct graphic
 * hardware access, bypassing the GL stack entirely.
 *
 * @see ecore_evas_new()
 * @see ecore_evas_gl_drm_new()
 */
EAPI Ecore_Evas     *ecore_evas_drm_new(const char *device, unsigned int parent, int x, int y, int w, int h);

/**
 * @brief Create an Ecore_Evas window using the drm engine with GL support.
 *
 * @param disp_name Name of the Wayland display to connect to.
 * @param parent (Unused)
 * @param x Horizontal position of the Ecore_Evas window.
 * @param y Vertical position of the Ecore_Evas window.
 * @param w Width of the Ecore_Evas window to be created.
 * @param h Height of the Ecore_Evas window to be created.
 * @return Ecore_Evas instance or @c NULL if creation failed.
 *
 * This creates a drm backend backed Ecore_Evas window, that also includes
 * gl support using libglapi.
 *
 * @see ecore_evas_new()
 * @see ecore_evas_drm_new()
 */
EAPI Ecore_Evas     *ecore_evas_gl_drm_new(const char *device, unsigned int parent, int x, int y, int w, int h); /** @since 1.12 */

/**
 * @brief Creates a new @c Ecore_Evas canvas bound to the Evas
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
 * @brief Creates a new @c Ecore_Evas canvas bound to the Evas
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
 * @brief Grabs a pointer to the actual pixels array of a given
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
 * @brief Returns a pointer to the Ecore_Evas parent of the given Ecore_Evas.
 *
 * @param ee An @c Ecore_Evas handle
 * @return A pointer to the Ecore_Evas parent.
 *
 * A use case for the function is to determine if the buffer is used inside
 * X11 or Wayland. Since the buffer engine doesn't give any indication on
 * it, we need to retrieve information from the Ecore_Evas parent.
 *
 * @since 1.13
 */
EAPI Ecore_Evas *ecore_evas_buffer_ecore_evas_parent_get(Ecore_Evas *ee);

/**
 * @brief Creates a new @c Ecore_Evas canvas bound to the Evas
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
 * @param x Horizontal position of window, in pixels
 * @param y Vertical position of window, in pixels
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
 * @brief Returns the backing store image object that represents the given
 * window in EWS.
 *
 * @param ee The Ecore_Evas from which to get the backing store.
 * @return The evas object of EWS backing store.
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
 * @brief Calls the window to be deleted (freed), but can let user decide to
 * forbid it by using ecore_evas_callback_delete_request_set().
 *
 * @param ee The Ecore_Evas for which window will be deleted.
 *
 * @since 1.1
 */
EAPI void ecore_evas_ews_delete_request(Ecore_Evas *ee);

/**
 * @brief Creates an Evas image object with image data <b>bound to an
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
 * whatever contents he/she wants, @b independently of the contents
 * of the canvas owned by @p ee_target. Those contents will reflect on
 * the canvas of @p ee, though, being exactly the image data of the
 * object returned by this function.
 *
 * This is a helper function for the scenario of one wanting to grab a
 * buffer canvas' contents (with ecore_evas_buffer_pixels_get()) to be
 * used on another canvas, for whatever reason. The most common goal
 * of this setup is to @b save an image file with a whole canvas as
 * contents, that cannot be achieved by using an image file within
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
 * object created via ecore_evas_object_image_new().
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

/**
 * @brief Creates a new @c Ecore_Evas canvas bound to the Evas
 * @b gdi (software) engine.
 *
 * @param parent ID of the parent window this Ecore_Evas window belongs
 *        to, or 0 if this is a top-level window.
 * @param x Horizontal position of window, in pixels.
 * @param y Vertical position of window, in pixels.
 * @param width The width of the canvas, in pixels.
 * @param height The height of the canvas, in pixels.
 * @return A new @c Ecore_Evas instance, or @c NULL on failure.
 *
 * The Graphics Device Interface (GDI) is a display API for Microsoft
 * Win32 similar in concept to Xlib or Quartz, providing an
 * abstraction layer for performing advanced drawing functionalities
 * using software rendering.
 */
EAPI Ecore_Evas     *ecore_evas_software_gdi_new(Ecore_Win32_Window *parent,
                                                 int                 x,
                                                 int                 y,
                                                 int                 width,
                                                 int                 height);

/**
 * @brief Creates a new @c Ecore_Evas canvas bound to the Evas
 * @b DirectDraw (software) engine.
 *
 * @param parent ID of the parent window this Ecore_Evas window belongs
 *        to, or 0 if this is a top-level window.
 * @param x Horizontal position of window, in pixels
 * @param y Vertical position of window, in pixels
 * @param width The width of the canvas, in pixels
 * @param height The height of the canvas, in pixels
 * @return A new @c Ecore_Evas instance or @c NULL on failure
 *
 * DirectDraw is an old Microsoft Win32 API for graphics rendering.
 * This API uses the unaccelerated software-based rendering.
 */
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

/**
 * @brief Creates a new @c Ecore_Evas canvas bound to the Evas
 * @b SDL engine.
 *
 * @param name        Device target name, defaults to "EFL SDL" if NULL.
 * @param w           Width of the canvas, in pixels.
 * @param h           Height of the canvas, in pixels.
 * @param fullscreen  Set the fullscreen property for the window.
 * @param hwsurface   Set the hardware surface property for the window.
 * @param noframe     Set the noframe flag on the einfo.
 * @param alpha       Set alpha for the Ecore_Evas window.
 * @return A new @c Ecore_Evas instance, or @c NULL on failure.
 */
EAPI Ecore_Evas     *ecore_evas_sdl_new(const char* name, int w, int h, int fullscreen, int hwsurface, int noframe, int alpha);

EAPI Ecore_Evas     *ecore_evas_sdl16_new(const char* name, int w, int h, int fullscreen, int hwsurface, int noframe, int alpha);

/**
 * @brief Creates a new @c Ecore_Evas canvas bound to the Evas
 * @b SDL + gl engine.
 *
 * @param name        Device target name, defaults to "EFL SDL" if NULL.
 * @param w           Width of the canvas, in pixels.
 * @param h           Height of the canvas, in pixels.
 * @param fullscreen  Set the fullscreen property for the window.
 * @param noframe     Set the noframe flag on the einfo.
 * @return A new @c Ecore_Evas instance or @c NULL on failure
 */
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

/**
 * @brief Creates a new @c Ecore_Evas canvas bound to the Evas
 * @b Cocoa engine.
 *
 * @param parent ID of the parent window this Ecore_Evas window belongs
 *        to, or 0 if this is a top-level window.
 * @param x Horizontal position of window, in pixels
 * @param y Vertical position of window, in pixels
 * @param w Width of the canvas, in pixels.
 * @param h Height of the canvas, in pixels.
 *
 * The cocoa backend is used for MacOS based systems.
 */
EAPI Ecore_Evas *ecore_evas_cocoa_new(Ecore_Cocoa_Window *parent,
                                      int x,
                                      int y,
                                      int w,
                                      int h);

/**
 * @brief Creates a new @c Ecore_Evas canvas bound to the Evas
 * @b psl1ght engine.
 *
 * @param parent ID of the parent window this Ecore_Evas window belongs
 *        to, or 0 if this is a top-level window.
 * @param x Horizontal position of window, in pixels
 * @param y Vertical position of window, in pixels
 * @param w Width of the canvas, in pixels.
 * @param h Height of the canvas, in pixels.
 *
 * The psl1ght backend is used for the PS3 GameOS.
 */
EAPI Ecore_Evas *ecore_evas_psl1ght_new(const char* name, int w, int h) EINA_DEPRECATED;


/* generic manipulation calls */
/**
 * @brief Gets the engine name used by this Ecore_Evas(window).
 *
 * @param ee Ecore_Evas whose engine's name is desired
 * @return A string that can(usually) be used in ecore_evas_new()
 *
 * @see ecore_evas_free()
 */
EAPI const char *ecore_evas_engine_name_get(const Ecore_Evas *ee);

/**
 * @brief Returns the Ecore_Evas for this Evas.
 *
 * @param e The Evas to get the Ecore_Evas from
 * @return The Ecore_Evas that holds this Evas, or @c NULL if not held by one.
 *
 * @warning Only use on Evas' created with ecore evas!
 */
EAPI Ecore_Evas *ecore_evas_ecore_evas_get(const Evas *e);

/**
 * @brief Frees an Ecore_Evas.
 *
 * @param ee The Ecore_Evas to free
 *
 * This frees up any memory used by the Ecore_Evas.
 */
EAPI void        ecore_evas_free(Ecore_Evas *ee);

/**
 * @brief Retrieves user data associated with an Ecore_Evas.
 *
 * @param ee The Ecore_Evas to retrieve the user data from.
 * @param key The key which the user data to be retrieved is associated with.
 * @return @c NULL on error or no data found, A pointer to the user data on
 *     success.
 *
 * This function retrieves user specific data that has been stored within an
 * Ecore_Evas structure with ecore_evas_data_set().
 *
 * @see ecore_evas_data_set()
 */
EAPI void       *ecore_evas_data_get(const Ecore_Evas *ee, const char *key);

/**
 * @brief Stores user data in an Ecore_Evas structure.
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
 * @brief Sets a callback for Ecore_Evas resize events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is resized.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_resize_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas move events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is moved.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_move_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas show events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is shown.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_show_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas hide events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is hidden.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_hide_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas delete request events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee gets a delete request.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_delete_request_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas destroy events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is destroyed.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_destroy_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas focus in events.
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call

 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee gets focus.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 * @note This function only reports focus in events for the default seat!
 *
 * @see ecore_evas_callback_focus_device_in_set()
 */
EAPI void        ecore_evas_callback_focus_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas focus out events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee loses focus.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 * @note This function only reports focus in events for the default seat!
 *
 * @see ecore_evas_callback_focus_device_out_set()
 */
EAPI void        ecore_evas_callback_focus_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas focus in events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee gets focus.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 *
 * @see ecore_evas_callback_focus_device_out_set()
 * @since 1.19
 */
EAPI void        ecore_evas_callback_focus_device_in_set(Ecore_Evas *ee, Ecore_Evas_Focus_Device_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas focus out events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee loses focus.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 *
 * @see ecore_evas_callback_focus_device_in_set()
 * @since 1.19
 */
EAPI void        ecore_evas_callback_focus_device_out_set(Ecore_Evas *ee, Ecore_Evas_Focus_Device_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas sticky events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee becomes sticky.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_sticky_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas un-sticky events.
 *
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
 * @brief Sets a callback for Ecore_Evas mouse in events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever the mouse enters @p ee.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 *
 * @since 1.19
 */
EAPI void        ecore_evas_callback_device_mouse_in_set(Ecore_Evas *ee, Ecore_Evas_Mouse_IO_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas mouse out events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever the mouse leaves @p ee.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 *
 * @since 1.19
 */
EAPI void        ecore_evas_callback_device_mouse_out_set(Ecore_Evas *ee, Ecore_Evas_Mouse_IO_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas mouse in events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever the mouse enters @p ee.
 *
 * @note the @p func will only report events for the default mouse.
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 *
 * @see ecore_evas_callback_device_mouse_in_set
 */
EAPI void        ecore_evas_callback_mouse_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas mouse out events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever the mouse leaves @p ee.
 *
 * @note the @p func will only report events for the default mouse.
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 *
 * @see ecore_evas_callback_device_mouse_out_set
 */
EAPI void        ecore_evas_callback_mouse_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas pre-render events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called just before the evas in @p ee is rendered.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_pre_render_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas mouse post-render events.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called just after the evas in @p ee is rendered.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 */
EAPI void        ecore_evas_callback_post_render_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Sets a callback for Ecore_Evas pre-free event.
 *
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
 * @brief Sets a callback for Ecore_Evas state changes.
 *
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 *
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee changes state.
 *
 * @warning If and when this function is called depends on the underlying
 * windowing system.
 *
 * @since 1.2
 */
EAPI void        ecore_evas_callback_state_change_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);

/**
 * @brief Gets an Ecore_Evas's Evas.
 *
 * @param ee The Ecore_Evas whose Evas you wish to get
 * @return The Evas contained within @p ee.
 */
EAPI Evas       *ecore_evas_get(const Ecore_Evas *ee);

/**
 * @brief Provides Managed move co-ordinates for an Ecore_Evas.
 *
 * @param ee The Ecore_Evas to move
 * @param x The x coordinate to set as the managed location
 * @param y The y coordinate to set as the managed location
 *
 * This sets the managed geometry position of the @p ee to (@p x, @p y)
 */
EAPI void        ecore_evas_managed_move(Ecore_Evas *ee, int x, int y);

/**
 * @brief Sets whether an Ecore_Evas is shaped or not.
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
 * @brief Queries whether an Ecore_Evas is shaped or not.
 *
 * @param ee The Ecore_Evas to query.
 * @return @c EINA_TRUE if shaped, @c EINA_FALSE if not.
 */
EAPI Eina_Bool   ecore_evas_shaped_get(const Ecore_Evas *ee);

/**
 * @brief Shows an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas to show.
 *
 * This function makes @p ee visible.
 */
EAPI void        ecore_evas_show(Ecore_Evas *ee);

/**
 * @brief Hides an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas to hide.
 *
 * This function makes @p ee hidden(not visible).
 */
EAPI void        ecore_evas_hide(Ecore_Evas *ee);

/**
 * @brief Activates (set focus to, via the window manager) an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas to activate.
 *
 * This functions activates the Ecore_Evas.
 */
EAPI void        ecore_evas_activate(Ecore_Evas *ee);


/**
 * @brief Sets the minimum size of a given @c Ecore_Evas window.
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
 * @brief Gets the minimum size set for a given @c Ecore_Evas window.
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
 * @brief Sets the maximum size of a given @c Ecore_Evas window.
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
 * @brief Gets the maximum size set for a given @c Ecore_Evas window.
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
 * @brief Sets the base size for a given @c Ecore_Evas window.
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
 * @brief Gets the base size set for a given @c Ecore_Evas window.
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
 * @brief Sets the "size step" for a given @c Ecore_Evas window.
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
 * @brief Gets the "size step" set for a given @c Ecore_Evas window.
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
 * @brief Sets the cursor for the default pointer device.
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
 * corner of the cursor image. Cursor object will be deleted with Ecore_Evas.
 *
 * @note This function creates an object from the image and uses
 * ecore_evas_object_cursor_set().
 *
 * @warning Previously set cursor will be deleted.
 *
 * @see ecore_evas_object_cursor_set()
 * @see ecore_evas_cursor_unset()
 */
EAPI void        ecore_evas_cursor_set(Ecore_Evas *ee, const char *file, int layer, int hot_x, int hot_y);

/**
 * @brief Gets information about an Ecore_Evas' default pointer device.
 *
 * @param ee The Ecore_Evas to get
 * @param obj A pointer to an Evas_Object to place the cursor Evas_Object.
 * @param layer A pointer to an int to place the cursor's layer in.
 * @param hot_x A pointer to an int to place the cursor's hot_x coordinate in.
 * @param hot_y A pointer to an int to place the cursor's hot_y coordinate in.
 *
 * This function queries information about an Ecore_Evas' cursor.
 *
 * @see ecore_evas_cursor_set()
 * @see ecore_evas_object_cursor_set()
 * @see ecore_evas_cursor_unset()
 */
EAPI void        ecore_evas_cursor_get(const Ecore_Evas *ee, Evas_Object **obj, int *layer, int *hot_x, int *hot_y);

/**
 * @brief Sets the cursor for the default pointer device.
 *
 * @param ee The Ecore_Evas
 * @param obj The Evas_Object which will be the cursor.
 * @param layer The layer in which the cursor will appear.
 * @param hot_x The x coordinate of the cursor's hot spot.
 * @param hot_y The y coordinate of the cursor's hot spot.
 *
 * This function makes the mouse cursor over @p ee be the object specified by
 * @p obj. The actual point within the object that the mouse is at is specified
 * by @p hot_x and @p hot_y, which are coordinates with respect to the top left
 * corner of the cursor object. Cursor object will be deleted with the Ecore_Evas.
 *
 * @see ecore_evas_cursor_set()
 */
EAPI void        ecore_evas_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y);

/**
 * @brief Unsets the cursor of the default pointer device.
 *
 * @param ee The Ecore_Evas to unset the cursor.
 *
 * This function unsets the cursor from the Ecore_Evas, and returns the cursor
 * object. If the cursor was set from ecore_evas_cursor_set(), this function
 * returns the image. In this case, the image should be deleted when it is
 * no longer needed.
 *
 * @see ecore_evas_cursor_set()
 * @see ecore_evas_object_cursor_set()
 * @since 1.11
 */
EAPI Evas_Object*        ecore_evas_cursor_unset(Ecore_Evas *ee);

/**
 * @brief Unsets the cursor of the specified pointer device.
 *
 * @param ee The Ecore_Evas to unset the cursor.
 * @param pointer A pointer device to set the cursor. Use @c NULL for the default.
 *
 * This function unsets the cursor from the Ecore_Evas, and returns the cursor
 * object. If the cursor was set from ecore_evas_cursor_set(), this function
 * returns the image. In this case, the image should be deleted when it is
 * no longer needed.
 *
 * @see ecore_evas_cursor_device_set()
 * @see ecore_evas_object_cursor_device_set()
 * @since 1.20
 */
EAPI Evas_Object *ecore_evas_cursor_device_unset(Ecore_Evas *ee, Efl_Input_Device *pointer);

/**
 * @brief Sets the cursor of an Ecore_Evas specified pointer device.
 *
 * @param ee The Ecore_Evas
 * @param pointer A pointer device to set the cursor. Use @c NULL for the default.
 * @param obj The Evas_Object which will be the cursor.
 * @param layer The layer in which the cursor will appear.
 * @param hot_x The x coordinate of the cursor's hot spot.
 * @param hot_y The y coordinate of the cursor's hot spot.
 *
 * This function makes the mouse cursor over @p ee be the object specified by
 * @p obj. The actual point within the object that the mouse is at is specified
 * by @p hot_x and @p hot_y, which are coordinates with respect to the top left
 * corner of the cursor object. Cursor object will be deleted with the Ecore_Evas.
 *
 * @since 1.19
 */
EAPI void ecore_evas_object_cursor_device_set(Ecore_Evas *ee, Efl_Input_Device *pointer,
                                              Evas_Object *obj, int layer,
                                              int hot_x, int hot_y);
/**
 * @brief Gets information about an Ecore_Evas' specified pointer device.
 *
 * @param ee The Ecore_Evas
 * @param pointer A pointer device to set the cursor. Use @c NULL for the default.
 * @param obj A pointer to an Evas_Object to place the cursor Evas_Object.
 * @param layer A pointer to an int to place the cursor's layer in.
 * @param hot_x A pointer to an int to place the cursor's hot_x coordinate in.
 * @param hot_y A pointer to an int to place the cursor's hot_y coordinate in.
 *
 * This function queries information about an Ecore_Evas' cursor.
 *
 * @see ecore_evas_cursor_device_set()
 * @since 1.19
 */
EAPI Eina_Bool ecore_evas_cursor_device_get(const Ecore_Evas *ee, Efl_Input_Device *pointer,
                                            Evas_Object **obj, int *layer,
                                            int *hot_x, int *hot_y);

/**
 * @brief Tells the WM whether or not to ignore an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas.
 * @param on @c EINA_TRUE to ignore, @c EINA_FALSE to not.
 *
 * This function causes the window manager to ignore @p ee if @p on is
 * @c EINA_TRUE, or not ignore @p ee if @p on is @c EINA_FALSE.
 */
EAPI void        ecore_evas_override_set(Ecore_Evas *ee, Eina_Bool on);

/**
 * @brief Queries whether an Ecore_Evas' window is overridden or not.
 *
 * @param ee The Ecore_Evas to set.
 * @return @c EINA_TRUE if @p ee is overridden, @c EINA_FALSE if not.
 */
EAPI Eina_Bool   ecore_evas_override_get(const Ecore_Evas *ee);

/**
 * @brief Sets whether or not an Ecore_Evas' window should avoid damage.
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
 * no need to render things again, just copy the exposed region to the
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
 * @brief Queries whether an Ecore_Evas' window avoids damage or not.
 *
 * @param ee The Ecore_Evas to set
 * @return The type of the damage management
 */
EAPI Ecore_Evas_Avoid_Damage_Type ecore_evas_avoid_damage_get(const Ecore_Evas *ee);

/**
 * @brief Sets the withdrawn state of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas whose window's withdrawn state is set.
 * @param withdrawn The Ecore_Evas window's new withdrawn state.
 */
EAPI void        ecore_evas_withdrawn_set(Ecore_Evas *ee, Eina_Bool withdrawn);

/**
 * @brief Returns the withdrawn state of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas whose window's withdrawn state is returned.
 * @return The Ecore_Evas window's withdrawn state.
 */
EAPI Eina_Bool   ecore_evas_withdrawn_get(const Ecore_Evas *ee);

/**
 * @brief Sets the sticky state of an Ecore_Evas window.
 *
 * @param ee The Ecore_Evas whose window's sticky state is set.
 * @param sticky The Ecore_Evas window's new sticky state.
 */
EAPI void        ecore_evas_sticky_set(Ecore_Evas *ee, Eina_Bool sticky);

/**
 * @brief Returns the sticky state of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas whose window's sticky state is returned.
 * @return The Ecore_Evas window's sticky state.
 */
EAPI Eina_Bool   ecore_evas_sticky_get(const Ecore_Evas *ee);

/**
 * @brief Enables/disables manual render.
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
 * @brief Gets enable/disable status of manual render.
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
 * @brief Forces immediate rendering on a given @c Ecore_Evas window.
 *
 * @param ee An @c Ecore_Evas handle
 *
 * Use this call to forcefully flush the @p ee's canvas rendering
 * pipeline, thus bring its window to an up to date state.
 */
EAPI void        ecore_evas_manual_render(Ecore_Evas *ee);

/**
 * @brief Sets comp syncing to enabled/disabled
 *
 * @param ee An @c Ecore_Evas handle
 * @param do_sync True to enable comp syncing, False to disable
 *
 * Turns on client+server synchronized rendering in X11.  Comp sync is
 * disabled by default, but can be turned on optionally.  Can also be
 * set via the ECORE_EVAS_COMP_NOSYNC / ECORE_EVAS_COMP_SYNC
 * environmental variables.
 *
 * @note This is an experimental functionality and is likely to be
 * removed in future versions of EFL.
 */
EAPI void        ecore_evas_comp_sync_set(Ecore_Evas *ee, Eina_Bool do_sync);

/**
 * @brief Gets the comp sync state
 *
 * @param ee An @c Ecore_Evas handle
 * @return True if composition synchronization is enabled, False otherwise
 *
 * @note This is an experimental functionality and is likely to be
 * removed in future versions of EFL.
 */
EAPI Eina_Bool   ecore_evas_comp_sync_get(const Ecore_Evas *ee);

/**
 * @brief Gets geometry of screen associated with this Ecore_Evas.
 *
 * @param ee The Ecore_Evas whose window's to query container screen geometry.
 * @param x Where to return the horizontal offset value. May be @c NULL.
 * @param y Where to return the vertical offset value. May be @c NULL.
 * @param w Where to return the width value. May be @c NULL.
 * @param h Where to return the height value. May be @c NULL.
 *
 * @since 1.1
 */
EAPI void        ecore_evas_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);

/**
 * @brief Gets the dpi of the screen the Ecore_Evas is primarily on.
 *
 * @param ee The Ecore_Evas whose window's to query.
 * @param xdpi Pointer to integer to store horizontal DPI. May be @c NULL.
 * @param ydpi Pointer to integer to store vertical DPI. May be @c NULL.
 *
 * @since 1.7
 */
EAPI void        ecore_evas_screen_dpi_get(const Ecore_Evas *ee, int *xdpi, int *ydpi);

EAPI void        ecore_evas_draw_frame_set(Ecore_Evas *ee, Eina_Bool draw_frame) EINA_DEPRECATED;
EAPI Eina_Bool   ecore_evas_draw_frame_get(const Ecore_Evas *ee) EINA_DEPRECATED;

/**
 * @brief Sets shadow geometry for client-side decorations.
 *
 * Note that the framespace contains both a shadow or glow around the window,
 * and the window borders (title bar, etc...).
 *
 * @since 1.19
 */
EAPI void        ecore_evas_shadow_geometry_set(Ecore_Evas *ee, int x, int y, int w, int h);

/**
 * @brief Gets shadow geometry for client-side decorations.
 *
 * Note that the framespace contains both a shadow or glow around the window,
 * and the window borders (title bar, etc...).
 *
 * @since 1.19
 */
EAPI void        ecore_evas_shadow_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h);

/**
 * @brief Associates the given object to this ecore evas.
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
 * @brief Cancels the association set with ecore_evas_object_associate().
 *
 * @param ee The Ecore_Evas to dissociate from @a obj
 * @param obj The object to dissociate from @a ee
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool    ecore_evas_object_dissociate(Ecore_Evas *ee, Evas_Object *obj);

/**
 * @brief Gets the object associated with @p ee.
 *
 * @param ee The Ecore_Evas to get the object from.
 * @return The associated object, or @c NULL if there is no associated object.
 */
EAPI Evas_Object *ecore_evas_object_associate_get(const Ecore_Evas *ee);

/* helper function to be used with ECORE_GETOPT_CALLBACK_*() */
EAPI unsigned char ecore_getopt_callback_ecore_evas_list_engines(const Ecore_Getopt *parser, const Ecore_Getopt_Desc *desc, const char *str, void *data, Ecore_Getopt_Value *storage);

/**
 * @brief Gets a list of all the ecore_evases.
 *
 * @return A list of ecore_evases.
 *
 * The returned list of ecore evases is only valid until the canvases are
 * destroyed (and should not be cached for instance). The list can be freed by
 * just deleting the list.
 */
EAPI Eina_List   *ecore_evas_ecore_evas_list_get(void);

/**
 * @brief Gets a list of all the sub ecore_evases.
 *
 * @param ee Ecore_Evas to get the list from.
 * @return A list of sub ecore_evases, or @c NULL if there is no sub ecore_evases.
 */
EAPI Eina_List   *ecore_evas_sub_ecore_evas_list_get(const Ecore_Evas *ee);

/**
 * @brief Set the window's client leader.
 *
 * @param ee Ecore_Evas for the window.
 * @param win The window.
 *
 * Sets the client leader X atom property for the given window.  All
 * non-transient top-level windows created by an app other than the main
 * window must have this property set to the app's main window.
 */
EAPI void           ecore_evas_x11_leader_set(Ecore_Evas *ee, Ecore_X_Window win);

/**
 * @brief Get the client leader.
 *
 * @param ee Ecore_Evas for the window.
 * @return the current client leader.
 *
 * @see ecore_evas_x11_leader_set()
 */
EAPI Ecore_X_Window ecore_evas_x11_leader_get(Ecore_Evas *ee);

/**
 * @brief Reset the client leader to default.
 *
 * @param ee Ecore_Evas for the window.
 *
 * @see ecore_evas_x11_leader_set()
 */
EAPI void           ecore_evas_x11_leader_default_set(Ecore_Evas *ee);

/**
 * @brief Set the rectangular region that can "contain" the pointer, replacing
 * any previously set region.
 *
 * @param ee The Ecore_Evas
 * @param x The X coordinate for the origin of the input containment rectangle.
 * @param y The Y coordinate for the origin of the input containment rectangle.
 * @param w The width of the input containment rectangle.
 * @param h The height of the input containment rectangle.
 *
 * Ecore_Evas may apply (orthogonal) rotations if needed, via an internal
 * call to _ecore_evas_x11_convert_rectangle_with_angle().  Note that
 * ecore_evas_x11_shape_input_apply() must be called to commit the changes
 * to the window itself.
 */
EAPI void           ecore_evas_x11_shape_input_rectangle_set(Ecore_Evas *ee, int x, int y, int w, int h);

/**
 * @brief Extends the pointer containment region to the union of the new
 * and existing input rectangle.
 *
 * @param ee The Ecore_Evas
 * @param x The X coordinate of the rectangular area to add.
 * @param y The Y coordinate of the rectangular area to add.
 * @param w The width of the rectangluar area to add.
 * @param h The height of the rectangluar area to add.
 *
 * Ecore_Evas may apply (orthogonal) rotations if needed, via an internal
 * call to _ecore_evas_x11_convert_rectangle_with_angle().  Note that
 * ecore_evas_x11_shape_input_apply() must be called to commit the changes
 * to the window itself.
 */
EAPI void           ecore_evas_x11_shape_input_rectangle_add(Ecore_Evas *ee, int x, int y, int w, int h);

/**
 * @brief Modifies the pointer containment region to subtract a rectangular region from it.
 *
 * @param ee The Ecore_Evas
 * @param x The X coordinate of the subtraction rectangle.
 * @param y The Y coordinate of the subtraction rectangle.
 * @param w The width of the subtraction rectangle.
 * @param h The height of the subtraction rectangle.
 *
 * Ecore_Evas may apply (orthogonal) rotations if needed, via an internal
 * call to _ecore_evas_x11_convert_rectangle_with_angle().  Note that
 * ecore_evas_x11_shape_input_apply() must be called to commit the changes
 * to the window itself.
 */
EAPI void           ecore_evas_x11_shape_input_rectangle_subtract(Ecore_Evas *ee, int x, int y, int w, int h);

/**
 * @brief Disables input for the window.
 *
 * @param ee The Ecore_Evas
 *
 * Sets the pointer containment region to a null rectangle, effectively
 * disabling input for the window.  Note that
 * ecore_evas_x11_shape_input_apply() must be called to commit the
 * changes to the window itself.
 */
EAPI void           ecore_evas_x11_shape_input_empty(Ecore_Evas *ee);

/**
 * @brief Unsets the pointer containment for the window.
 *
 * @param ee The Ecore_Evas
 *
 * Sets the pointer containment region to a maximally wide and high
 * rectangle, effectively permitting all allowed input events to reach
 * the window.  Note that ecore_evas_x11_shape_input_apply() must be
 * called to commit the changes to the window itself.
 */
EAPI void           ecore_evas_x11_shape_input_reset(Ecore_Evas *ee);

/**
 * @brief Applies the pointer containment region to the client window.
 *
 * @param ee The Ecore_Evas
 */
EAPI void           ecore_evas_x11_shape_input_apply(Ecore_Evas *ee);

#ifdef EFL_BETA_API_SUPPORT

/**
 * @brief A callback used to accept a new client.
 *
 * @param data The callback data
 * @param ee The Ecore_Evas
 * @param client_host The address of the new client.
 * @return @c EINA_TRUE to accept the client, @c EINA_FALSE otherwise.
 *
 * @see ecore_evas_vnc_start()
 * @since 1.19
 */
typedef Eina_Bool (*Ecore_Evas_Vnc_Client_Accept_Cb)(void *data, Ecore_Evas *ee, const char *client_host);

/**
 * @brief A callback used to inform that a client has disconnected.
 *
 * @param data The callback data
 * @param ee The Ecore_Evas
 * @param client_host The address of the client
 *
 * @see ecore_evas_vnc_start()
 * @since 1.19
 */
typedef void (*Ecore_Evas_Vnc_Client_Disconnected_Cb)(void *data, Ecore_Evas *ee, const char *client_host);

/**
 * @brief Starts a VNC server.
 *
 * @param ee The Ecore_Evas to start the VNC server
 * @param addr The address that will be used to bind the VNC server. Use @c NULL to bind to any interface.
 * @param port The port number to start the VNC server. Use @c -1 to set the default VNC port (5900)
 * @param accept_cb A callback used to accept a new client. If @c NULL all clients will be accepted.
 * @param disc_cb A callback used to inform that a client has disconnected. It may be @c NULL.
 * @param data Data to pass to @a accept_cb and @disc_cb
 * @return an Evas_Object that take everything under it to represent the view of the client.
 *
 * @see ecore_evas_vnc_stop()
 * @see Ecore_Evas_Vnc_Client_Accept_Cb()
 * @since 1.19
 */
EAPI Evas_Object *ecore_evas_vnc_start(Ecore_Evas *ee, const char *addr, int port,
                                       Ecore_Evas_Vnc_Client_Accept_Cb accept_cb,
                                       Ecore_Evas_Vnc_Client_Disconnected_Cb disc_cb,
                                       void *data);

#endif

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
 * @brief Sets the engine to be used by the backing store engine.
 *
 * @param engine The engine to be set.
 * @param options The options of the engine to be set.
 * @return @c EINA_TRUE on success, @c EINA_FALSE if ews is already in use.
 *
 * @since 1.1
 */
EAPI Eina_Bool ecore_evas_ews_engine_set(const char *engine, const char *options);

/**
 * @brief Reconfigures the backing store used.
 *
 * @param x The X coordinate to be used.
 * @param y The Y coordinate to be used.
 * @param w The width of the Ecore_Evas to setup.
 * @param h The height of the Ecore_Evas to setup.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.1
 */
EAPI Eina_Bool ecore_evas_ews_setup(int x, int y, int w, int h);

/**
 * @brief Returns the internal backing store in use.
 *
 * @return The internal backing store in use.
 *
 * @note this will force it to be created, making future calls to
 * ecore_evas_ews_engine_set() void.
 *
 * @see ecore_evas_ews_evas_get()
 * @since 1.1
 */
EAPI Ecore_Evas *ecore_evas_ews_ecore_evas_get(void);

/**
 * @brief Returns the internal backing store in use.
 *
 * @return The internal backing store in use.
 *
 * @note this will force it to be created, making future calls to
 * ecore_evas_ews_engine_set() void.
 *
 * @see ecore_evas_ews_ecore_evas_get()
 * @since 1.1
 */
EAPI Evas *ecore_evas_ews_evas_get(void);

/**
 * @brief Gets the current background.
 */
EAPI Evas_Object *ecore_evas_ews_background_get(void);

/**
 * @brief Sets the current background, must be created by evas ecore_evas_ews_evas_get().
 *
 * @param o The Evas_Object for which to set the background.
 *
 * It will be kept at lowest layer (EVAS_LAYER_MIN) and below
 * everything else. You can set any object, default is a black
 * rectangle.
 *
 * @note previous object will be deleted!
 */
EAPI void ecore_evas_ews_background_set(Evas_Object *o);

/**
 * @brief Returns all Ecore_Evas* created by EWS.
 *
 * @return An eina list of Ecore_evases.
 *
 * @note Do not change the returned list or its contents.
 *
 * @since 1.1
 */
EAPI const Eina_List *ecore_evas_ews_children_get(void);

/**
 * @brief Sets the identifier of the manager taking care of internal windows.
 *
 * @param manager any unique identifier address.
 *
 * The ECORE_EVAS_EWS_EVENT_MANAGER_CHANGE event is issued. Consider
 * handling it to know if you should stop handling events yourself
 * (ie: another manager took over)
 *
 * @see ecore_evas_ews_manager_get()
 * @since 1.1
 */
EAPI void        ecore_evas_ews_manager_set(const void *manager);

/**
 * @brief Gets the identifier of the manager taking care of internal windows.
 *
 * @return the value set by ecore_evas_ews_manager_set()
 *
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
 * @defgroup Ecore_Evas_Extn External plug/socket infrastructure for remote canvases
 * @ingroup Ecore_Evas_Group
 *
 * These functions allow 1 process to create a "socket" was pluged into which another
 * process can create a "plug" remotely to plug into.
 * Socket can provide content for several plugs.
 * This is best for small sized objects (about the size range
 * of a small icon up to a few large icons). Since the plug is actually an
 * image object, you can fetch the pixel data
 *
 * @since 1.2
 * @{
 */

EAPI extern int ECORE_EVAS_EXTN_CLIENT_ADD; /**< this event is received when a plug has connected to an extn socket @since 1.2 */
EAPI extern int ECORE_EVAS_EXTN_CLIENT_DEL; /**< this event is received when a plug has disconnected from an extn socket @since 1.2 */

/**
 * @brief Creates a new Ecore_Evas canvas for the new external ecore evas socket
 *
 * @param w The width of the canvas, in pixels
 * @param h The height of the canvas, in pixels
 * @return A new @c Ecore_Evas instance, or @c NULL on failure
 *
 * This creates a new extn_socket canvas wrapper, with image data array
 * @b bound to the ARGB format, 8 bits per pixel.
 *
 * If creation is successful, an Ecore_Evas handle is returned or @c NULL if
 * creation fails. Also focus, show, hide etc. callbacks will also be called
 * if the plug object is shown, or already visible on connect, or if it is
 * hidden later, focused or unfocused.
 *
 * This function has to be followed by ecore_evas_extn_socket_listen(),
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
 * static Eina_Bool client_add_cb(void *data, int event, void *event_info)
 * {
 *    Ecore_Evas *ee = event_info;
 *    printf("client is connected to external socket %p\n", ee);
 *    return ECORE_CALLBACK_PASS_ON;
 * }
 *
 * static Eina_Bool client_del_cb(void *data, int event, void *event_info)
 * {
 *    Ecore_Evas *ee = event_info;
 *    printf("client is disconnected from external socket %p\n", ee);
 *    return ECORE_CALLBACK_PASS_ON;
 * }
 *
 * void setup(void)
 * {
 *    ecore_event_handler_add(ECORE_EVAS_EXTN_CLIENT_ADD,
 *                           client_add_cb, NULL);
 *    ecore_event_handler_add(ECORE_EVAS_EXTN_CLIENT_DEL,
 *                           client_del_cb, NULL);
 * }
 * @endcode
 *
 * Note that events come in later after the event happened. You may want to be
 * careful as data structures you had associated with the image object
 * may have been freed after deletion, but the object may still be around
 * awaiting cleanup and thus still be valid.You can change the size with something like:
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
 * @brief Creates a socket to provide the service for external ecore evas
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
 * @brief Sets the blocking about mouse events to Ecore Evas.
 *
 * @param ee The Ecore_Evas.
 * @param events_block The blocking about mouse events.
 *
 * @see ecore_evas_extn_socket_events_block_get()
 *
 * @since 1.15
 */
EAPI void ecore_evas_extn_socket_events_block_set(Ecore_Evas *ee, Eina_Bool events_block);

/**
 * @brief Gets the blocking about mouse events to Ecore Evas.
 *
 * @param ee The Ecore_Evas.
 * @return The blocking about mouse events.
 *
 * @see ecore_evas_extn_socket_events_block_set()
 *
 * @since 1.15
 */
EAPI Eina_Bool ecore_evas_extn_socket_events_block_get(Ecore_Evas *ee);

/**
 * @brief Locks the pixel data so the socket cannot change it.
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
 * odd behaviors.
 *
 * @see ecore_evas_extn_plug_new()
 * @see ecore_evas_extn_plug_object_data_unlock()
 *
 * @since 1.2
 */
EAPI void ecore_evas_extn_plug_object_data_lock(Evas_Object *obj);

/**
 * @brief Unlocks the pixel data so the socket can change it again.
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
 * @brief Creates a new external ecore evas plug.
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
 * @brief Connects an external ecore evas plug to service provided by external
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
 * @brief Retrieves the coordinates of the default mouse pointer.
 *
 * @param ee The Ecore_Evas containing the pointer
 * @param x Pointer to integer to store horizontal coordinate. May be @c NULL.
 * @param y Pointer to integer to store vertical coordinate. May be @c NULL.
 *
 * @see ecore_evas_pointer_device_xy_get
 * @since 1.8
 */
EAPI void ecore_evas_pointer_xy_get(const Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y);

/**
 * @brief Sets the coordinates of the mouse pointer.
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
 * @brief Retrieves the coordinates of the mouse pointer.
 *
 * @param ee The Ecore_Evas containing the pointer
 * @param pointer The pointer device, use @c NULL for the default pointer.
 * @param x Pointer to integer to store horizontal coordinate. May be @c NULL.
 * @param y Pointer to integer to store vertical coordinate. May be @c NULL.
 * @since 1.19
 */
EAPI void ecore_evas_pointer_device_xy_get(const Ecore_Evas *ee, const Efl_Input_Device *pointer, Evas_Coord *x, Evas_Coord *y);

/**
 * @brief Retrieves the Visual used for pixmap creation.
 *
 * @param ee The Ecore_Evas containing the pixmap
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
 * @brief Retrieves the Colormap used for pixmap creation.
 *
 * @param ee The Ecore_Evas containing the pixmap
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
 * @brief Retrieves the depth used for pixmap creation.
 *
 * @param ee The Ecore_Evas containing the pixmap
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

#undef EAPI
#define EAPI

#endif
