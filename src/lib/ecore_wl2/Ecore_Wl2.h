#ifndef _ECORE_WL2_H_
# define _ECORE_WL2_H_

# include <Eina.h>
# include <Ecore.h>
# include <wayland-client.h>
# include <wayland-cursor.h>
# include <xkbcommon/xkbcommon.h>

# define WL_HIDE_DEPRECATED
# include <wayland-server.h>

# ifdef EAPI
#  undef EAPI
# endif

# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif

/* # ifdef __cplusplus */
/* extern "C" { */
/* # endif */

# ifndef _ECORE_WL2_WINDOW_PREDEF
typedef struct _Ecore_Wl2_Window Ecore_Wl2_Window;
# endif

typedef struct _Ecore_Wl2_Display Ecore_Wl2_Display;

typedef struct _Ecore_Wl2_Global
{
   Eina_Stringshare *interface;
   unsigned int id, version;
} Ecore_Wl2_Global;

typedef struct _Ecore_Wl2_Event_Global
{
   Eina_Stringshare *interface;
   unsigned int id, version;
} Ecore_Wl2_Event_Global;

typedef enum _Ecore_Wl2_Window_Type
{
   ECORE_WL2_WINDOW_TYPE_NONE,
   ECORE_WL2_WINDOW_TYPE_TOPLEVEL,
   ECORE_WL2_WINDOW_TYPE_FULLSCREEN,
   ECORE_WL2_WINDOW_TYPE_MAXIMIZED,
   ECORE_WL2_WINDOW_TYPE_TRANSIENT,
   ECORE_WL2_WINDOW_TYPE_MENU,
   ECORE_WL2_WINDOW_TYPE_DND,
   ECORE_WL2_WINDOW_TYPE_CUSTOM
} Ecore_Wl2_Window_Type;

EAPI extern int ECORE_WL2_EVENT_GLOBAL_ADDED;
EAPI extern int ECORE_WL2_EVENT_GLOBAL_REMOVED;

/**
 * @file
 * @brief Ecore functions for dealing with the Wayland display protocol
 *
 * @defgroup Ecore_Wl2_Group Ecore_Wl2 - Wayland integration
 * @ingroup Ecore
 *
 * Ecore_Wl2 provides a wrapper and convenience functions for using the
 * Wayland protocol in implementing a window system. Function groups for
 * this part of the library include the following:
 *
 * @li @ref Ecore_Wl2_Init_Group
 * @li @ref Ecore_Wl2_Display_Group
 * @li @ref Ecore_Wl2_Window_Group
 */

/**
 * @defgroup Ecore_Wl2_Init_Group Wayland Library Init and Shutdown Functions
 * @ingroup Ecore_Wl2_Group
 *
 * Functions that start and shutdown the Ecore Wl2 Library.
 */

/**
 * Initialize the Ecore_Wl2 library
 *
 * @return  The number of times the library has been initialized without being
 *          shutdown. 0 is returned if an error occurs.
 *
 * @ingroup Ecore_Wl2_Init_Group
 */
EAPI int ecore_wl2_init(void);

/**
 * Shutdown the Ecore_Wl2 Library
 *
 * @return  The number of times the library has been initialized without
 *          being shutdown.
 *
 * @ingroup Ecore_Wl2_Init_Group
 */
EAPI int ecore_wl2_shutdown(void);

/**
 * @defgroup Ecore_Wl2_Display_Group Wayland Library Display Functions
 * @ingroup Ecore_Wl2_Group
 *
 * Functions that deal with creating, connecting, or interacting with
 * Wayland displays
 */

/**
 * Create a new Wayland display
 *
 * @brief This function is typically used to create a new display for
 * use with compositors, or to create a new display for use in nested
 * compositors.
 *
 * @param name The display target name to create. If @c NULL, a default
 *             display name will be assigned.
 * @return The newly created Ecore_Wl2_Display
 *
 * @ingroup Ecore_Wl2_Display_Group
 */
EAPI Ecore_Wl2_Display *ecore_wl2_display_create(const char *name);

/**
 * Destroy an existing Wayland display
 *
 * @brief This function is typically used by servers to terminate an
 * existing Wayland display.
 *
 * @param display The display to terminate
 *
 * @ingroup Ecore_Wl2_Display_Group
 */
EAPI void ecore_wl2_display_destroy(Ecore_Wl2_Display *display);

/**
 * Connect to an existing Wayland display
 *
 * @brief This function is typically used by clients to connect to an
 * existing Wayland display.
 *
 * @param name The display target name to connect to. If @c NULL, the default
 *             display is assumed.
 *
 * @return The Ecore_Wl2_Display which was connected to
 *
 * @ingroup Ecore_Wl2_Display_Group
 */
EAPI Ecore_Wl2_Display *ecore_wl2_display_connect(const char *name);

/**
 * Disconnect an existing Wayland display
 *
 * @brief This function is typically used by clients to disconnect from an
 * existing Wayland display.
 *
 * @param display The display to disconnect from
 *
 * @ingroup Ecore_Wl2_Display_Group
 */
EAPI void ecore_wl2_display_disconnect(Ecore_Wl2_Display *display);

/**
 * Terminate a Wayland display's main loop
 *
 * @brief This function is typically used by servers to terminate the
 * Wayland display main loop. This is usually only called when a server
 * encounters an error.
 *
 * @param display The Ecore_Wl2_Display to terminate
 *
 * @ingroup Ecore_Wl2_Display_Group
 */
EAPI void ecore_wl2_display_terminate(Ecore_Wl2_Display *display);

/**
 * Retrieve the existing Wayland display
 *
 * @param display The Ecore_Wl2_Display for which to retrieve the existing
 *                Wayland display from
 *
 * @return The wl_display which this Ecore_Wl2_Display is using
 *
 * @ingroup Ecore_Wl2_Display_Group
 */
EAPI struct wl_display *ecore_wl2_display_get(Ecore_Wl2_Display *display);

/**
 * Retrieve the wl_shm from a given Ecore_Wl2_Display
 *
 * @param display The Ecore_Wl2_Display for which to retrieve the existing
 *                Wayland shm interface from
 *
 * @return The wl_shm which this Ecore_Wl2_Display is using
 *
 * @ingroup Ecore_Wl2_Display_Group
 */
EAPI struct wl_shm *ecore_wl2_display_shm_get(Ecore_Wl2_Display *display);

/**
 * Return an Eina_Iterator that can be used to iterate through globals
 *
 * @param display The Ecore_Wl2_Display for which to return a global iterator
 *
 * @ingroup Ecore_Wl2_Display_Group
 */
EAPI Eina_Iterator *ecore_wl2_display_globals_get(Ecore_Wl2_Display *display);

/**
 * @defgroup Ecore_Wl2_Window_Group Wayland Library Window Functions
 * @ingroup Ecore_Wl2_Group
 *
 * Functions that deal with creating, connecting, or interacting with
 * Wayland windows
 */

/**
 * Create a new Ecore_Wl2_Window
 *
 * @param display The Ecore_Wl2_Display on which to create this new window
 * @param parent The Ecore_Wl2_Window which is the parent of this window
 * @param x Initial x position of window
 * @param y Initial y position of window
 * @param w Initial width of window
 * @param h Initial height of window
 *
 * @ingroup Ecore_Wl2_Window_Group
 */
EAPI Ecore_Wl2_Window *ecore_wl2_window_new(Ecore_Wl2_Display *display, Ecore_Wl2_Window *parent, int x, int y, int w, int h);

/**
 * Get the window id associated with an Ecore_Wl2_Window
 *
 * @param window The Ecore_Wl2_Window of which to retrieve the window id
 *
 * @return The id associated with this window
 *
 * @ingroup Ecore_Wl2_Window_Group
 */
EAPI int ecore_wl2_window_id_get(Ecore_Wl2_Window *window);

/**
 * Get the wl_surface which belongs to this window
 *
 * @param window The Ecore_Wl2_Window to get the surface of
 *
 * @return The wl_surface associated with this window.
 *
 * @ingroup Ecore_Wl2_Window_Group
 */
EAPI struct wl_surface *ecore_wl2_window_surface_get(Ecore_Wl2_Window *window);

/**
 * Show a given Ecore_Wl2_Window
 *
 * @param window The Ecore_Wl2_Window to show
 *
 * @ingroup Ecore_Wl2_Window_Group
 */
EAPI void ecore_wl2_window_show(Ecore_Wl2_Window *window);

/**
 * Hide a given Ecore_Wl2_Window
 *
 * @param window The Ecore_Wl2_Window to hide
 *
 * @ingroup Ecore_Wl2_Window_Group
 */
EAPI void ecore_wl2_window_hide(Ecore_Wl2_Window *window);

/**
 * Free a given Ecore_Wl2_Window
 *
 * @param window The Ecore_Wl2_Window to free
 *
 * @ingroup Ecore_Wl2_Window_Group
 */
EAPI void ecore_wl2_window_free(Ecore_Wl2_Window *window);

/**
 * Move a given Ecore_Wl2_Window
 *
 * @brief The position requested (@p x, @p y) is not honored by Wayland because
 * Wayland does not allow specific window placement to be set.
 *
 * @param window The Ecore_Wl2_Window which to move
 * @param x Desired x position of window
 * @param y Desired y position of window
 *
 * @ingroup Ecore_Wl2_Window_Group
 */
EAPI void ecore_wl2_window_move(Ecore_Wl2_Window *window, int x, int y);

/**
 * Resize a given Ecore_Wl2_Window
 *
 * @brief The size requested (@p w, @p h) is not honored by Wayland because
 * Wayland does not allow specific window sizes to be set.
 *
 * @param window The Ecore_Wl2_Window which to resize
 * @param w Desired width of window
 * @param h Desired height of window
 * @param location The edge of the window from where the resize should start
 *
 * @ingroup Ecore_Wl2_Window_Group
 */
EAPI void ecore_wl2_window_resize(Ecore_Wl2_Window *window, int w, int h, int location);

/**
 * Raise a given Ecore_Wl2_Window
 *
 * @param window The Ecore_Wl2_Window which to raise
 *
 * @ingroup Ecore_Wl2_Window_Group
 */
EAPI void ecore_wl2_window_raise(Ecore_Wl2_Window *window);

/* # ifdef __cplusplus */
/* } */
/* # endif */

# undef EAPI
# define EAPI

#endif
