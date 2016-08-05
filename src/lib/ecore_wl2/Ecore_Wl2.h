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

# ifdef EFL_BETA_API_SUPPORT

typedef struct _Ecore_Wl2_Subsurface Ecore_Wl2_Subsurface;

#  ifndef _ECORE_WL2_WINDOW_PREDEF
typedef struct _Ecore_Wl2_Window Ecore_Wl2_Window;
#  endif

typedef struct _Ecore_Wl2_Display Ecore_Wl2_Display;
typedef struct _Ecore_Wl2_Output Ecore_Wl2_Output;
typedef struct _Ecore_Wl2_Input Ecore_Wl2_Input;
typedef struct _Ecore_Wl2_Seat Ecore_Wl2_Seat;
typedef struct _Ecore_Wl2_Pointer Ecore_Wl2_Pointer;
typedef struct _Ecore_Wl2_Keyboard Ecore_Wl2_Keyboard;
typedef struct _Ecore_Wl2_Touch Ecore_Wl2_Touch;

/* matches protocol values */
typedef enum
{
   ECORE_WL2_DRAG_ACTION_NONE = 0,
   ECORE_WL2_DRAG_ACTION_COPY = 1,
   ECORE_WL2_DRAG_ACTION_MOVE = 2,
   ECORE_WL2_DRAG_ACTION_ASK = 4,
} Ecore_Wl2_Drag_Action;

struct _Ecore_Wl2_Event_Connection
{
   Ecore_Wl2_Display *display;
};
typedef struct _Ecore_Wl2_Event_Connection Ecore_Wl2_Event_Connect;
typedef struct _Ecore_Wl2_Event_Connection Ecore_Wl2_Event_Disconnect;

typedef struct _Ecore_Wl2_Global
{
   Eina_Stringshare *interface;
   unsigned int id, version;
} Ecore_Wl2_Global;

typedef struct _Ecore_Wl2_Event_Global
{
   Ecore_Wl2_Display *display;
   Eina_Stringshare *interface;
   unsigned int id, version;
} Ecore_Wl2_Event_Global;

typedef struct _Ecore_Wl2_Event_Focus_In
{
   unsigned int window;
   unsigned int timestamp;
} Ecore_Wl2_Event_Focus_In;

typedef struct _Ecore_Wl2_Event_Focus_Out
{
   unsigned int window;
   unsigned int timestamp;
} Ecore_Wl2_Event_Focus_Out;

typedef struct _Ecore_Wl2_Event_Dnd_Enter
{
   unsigned int win, source, serial;
   char **types;
   int num_types, x, y;
   struct wl_data_offer *offer;
} Ecore_Wl2_Event_Dnd_Enter;

typedef struct _Ecore_Wl2_Event_Dnd_Leave
{
   unsigned int win, source;
} Ecore_Wl2_Event_Dnd_Leave;

typedef struct _Ecore_Wl2_Event_Dnd_Motion
{
   unsigned int win, source, serial;
   int x, y;
} Ecore_Wl2_Event_Dnd_Motion;

typedef struct _Ecore_Wl2_Event_Dnd_Drop
{
   unsigned int win, source;
   int x, y;
} Ecore_Wl2_Event_Dnd_Drop;

typedef struct _Ecore_Wl2_Event_Dnd_End
{
   unsigned int win, source;
} Ecore_Wl2_Event_Dnd_End;

struct _Ecore_Wl2_Event_Data_Source_Event
{
   unsigned int win, source;
   Ecore_Wl2_Drag_Action action;
};

typedef struct _Ecore_Wl2_Event_Data_Source_Event Ecore_Wl2_Event_Data_Source_End;
typedef struct _Ecore_Wl2_Event_Data_Source_Event Ecore_Wl2_Event_Data_Source_Drop;
typedef struct _Ecore_Wl2_Event_Data_Source_Event Ecore_Wl2_Event_Data_Source_Action;

typedef struct _Ecore_Wl2_Event_Data_Source_Target
{
   char *type;
} Ecore_Wl2_Event_Data_Source_Target;

typedef struct _Ecore_Wl2_Event_Data_Source_Send
{
   char *type;
   int fd;
} Ecore_Wl2_Event_Data_Source_Send;

typedef enum
{
   ECORE_WL2_SELECTION_CNP,
   ECORE_WL2_SELECTION_DND
} Ecore_Wl2_Selection_Type;

typedef struct _Ecore_Wl2_Event_Selection_Data_Ready
{
   char *data;
   int len;
   Ecore_Wl2_Selection_Type sel_type;
} Ecore_Wl2_Event_Selection_Data_Ready;

typedef enum
{
   ECORE_WL2_WINDOW_STATE_NONE = 0,
   ECORE_WL2_WINDOW_STATE_FULLSCREEN = (1 << 0),
   ECORE_WL2_WINDOW_STATE_MAXIMIZED = (1 << 1),
} Ecore_Wl2_Window_States;

typedef struct _Ecore_Wl2_Event_Window_Configure
{
   unsigned int win, event_win, edges;
   unsigned int w, h;
   unsigned int states;
} Ecore_Wl2_Event_Window_Configure;

typedef struct _Ecore_Wl2_Event_Sync_Done
{
   Ecore_Wl2_Display *display;
} Ecore_Wl2_Event_Sync_Done;

typedef enum _Ecore_Wl2_Window_Type
{
   ECORE_WL2_WINDOW_TYPE_NONE,
   ECORE_WL2_WINDOW_TYPE_TOPLEVEL,
   ECORE_WL2_WINDOW_TYPE_MENU,
   ECORE_WL2_WINDOW_TYPE_DND,
   ECORE_WL2_WINDOW_TYPE_CUSTOM,
   ECORE_WL2_WINDOW_TYPE_LAST
} Ecore_Wl2_Window_Type;

typedef void (*Ecore_Wl2_Bind_Cb)(struct wl_client *client, void *data, uint32_t version, uint32_t id);
typedef void (*Ecore_Wl2_Unbind_Cb)(struct wl_resource *resource);

EAPI extern int ECORE_WL2_EVENT_DISCONNECT; /** @since 1.18 */
EAPI extern int ECORE_WL2_EVENT_CONNECT; /** @since 1.18 */
EAPI extern int ECORE_WL2_EVENT_GLOBAL_ADDED; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_GLOBAL_REMOVED; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_FOCUS_IN; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_FOCUS_OUT; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_DND_ENTER; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_DND_LEAVE; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_DND_MOTION; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_DND_DROP; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_DND_END; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_DND_DATA_READY; /** @since 1.18 */
EAPI extern int ECORE_WL2_EVENT_DATA_SOURCE_END; /** @since 1.18 */
EAPI extern int ECORE_WL2_EVENT_DATA_SOURCE_DROP; /** @since 1.18 */
EAPI extern int ECORE_WL2_EVENT_DATA_SOURCE_ACTION; /** @since 1.18 */
EAPI extern int ECORE_WL2_EVENT_DATA_SOURCE_TARGET; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_DATA_SOURCE_SEND; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_CNP_DATA_READY; /** @since 1.18 */
EAPI extern int ECORE_WL2_EVENT_WINDOW_CONFIGURE; /** @since 1.17 */
EAPI extern int ECORE_WL2_EVENT_SYNC_DONE; /** @since 1.17 */

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
 * @li @ref Ecore_Wl2_Output_Group
 * @li @ref Ecore_Wl2_Dnd_Group
 * @li @ref Ecore_Wl2_Input_Group
 * @li @ref Ecore_Wl2_Subsurface_Group
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
 * @since 1.17
 */
EAPI int ecore_wl2_init(void);

/**
 * Shutdown the Ecore_Wl2 Library
 *
 * @return  The number of times the library has been initialized without
 *          being shutdown.
 *
 * @ingroup Ecore_Wl2_Init_Group
 * @since 1.17
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
 * @since 1.17
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
 * @since 1.17
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
 * @since 1.17
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
 * @since 1.17
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
 * @since 1.17
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
 * @since 1.17
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
 * @since 1.17
 */
EAPI struct wl_shm *ecore_wl2_display_shm_get(Ecore_Wl2_Display *display);

/**
 * Retrieve the wl_dmabuf from a given Ecore_Wl2_Display
 *
 *
 * @param display The Ecore_Wl2_Display for which to retrieve the existing
 *                Wayland dmabuf interface from
 *
 *
 * @return The wl_dmabuf which this Ecore_Wl2_Display is using
 *
 * @ingroup Ecore_Wl2_Display_Group
 *
 * @note This is intended for client use only and should be used only
 *       after ecore_wl2_display_connect().  Also, the return type is
 *       void * instead of zpw_linux_dmabuf_v1 * since we don't want
 *       to change our public API every time the version changes in
 *       wayland-protocols.
 *
 * @since 1.18
 */
EAPI void *ecore_wl2_display_dmabuf_get(Ecore_Wl2_Display *display);

/**
 * Return an Eina_Iterator that can be used to iterate through globals
 *
 * @param display The Ecore_Wl2_Display for which to return a global iterator
 *
 * @note The caller of this function should free the returned Eina_Iterator
 * when finished with it.
 *
 * @ingroup Ecore_Wl2_Display_Group
 * @since 1.17
 */
EAPI Eina_Iterator *ecore_wl2_display_globals_get(Ecore_Wl2_Display *display);

/**
 * Retrieves the size of the current screen.
 *
 * @param display The display to get the screen size of
 * @param w where to return the width. May be NULL. Returns 0 on error.
 * @param h where to return the height. May be NULL. Returns 0 on error.
 *
 * @ingroup Ecore_Wl2_Display_Group
 * @since 1.17
 */
EAPI void ecore_wl2_display_screen_size_get(Ecore_Wl2_Display *display, int *w, int *h);

/**
 * Find an Ecore_Wl2_Window based on id
 *
 * @param display The display to search for the window
 * @param id The id of the window to find
 *
 * @return The Ecore_Wl2_Window if found, or NULL if no such window exists
 *
 * @ingroup Ecore_Wl2_Display_Group
 * @since 1.17
 */
EAPI Ecore_Wl2_Window *ecore_wl2_display_window_find(Ecore_Wl2_Display *display, unsigned int id);

/**
 * Retrieves the Wayland Registry used for the current Wayland display.
 *
 * @param display The display to get the registry of
 *
 * @return The current wayland registry, or NULL on error
 *
 * @ingroup Ecore_Wl2_Display_Group
 * @since 1.17
 */
EAPI struct wl_registry *ecore_wl2_display_registry_get(Ecore_Wl2_Display *display);

/**
 * @defgroup Ecore_Wl2_Window_Group Wayland Library Window Functions
 * @ingroup Ecore_Wl2_Group
 *
 * Functions that deal with creating, destroying, or interacting with
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
 * @since 1.17
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
 * @since 1.17
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
 * @since 1.17
 */
EAPI struct wl_surface *ecore_wl2_window_surface_get(Ecore_Wl2_Window *window);

/**
 * Get the id of a given Ecore_Wl2_Window
 *
 * @param window The window to return the id of
 *
 * @return The id of the given window, or -1 on failure
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI int ecore_wl2_window_surface_id_get(Ecore_Wl2_Window *window);

/**
 * Show a given Ecore_Wl2_Window
 *
 * @param window The Ecore_Wl2_Window to show
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_show(Ecore_Wl2_Window *window);

/**
 * Hide a given Ecore_Wl2_Window
 *
 * @param window The Ecore_Wl2_Window to hide
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_hide(Ecore_Wl2_Window *window);

/**
 * Free a given Ecore_Wl2_Window
 *
 * @param window The Ecore_Wl2_Window to free
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
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
 * @since 1.17
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
 * @since 1.17
 */
EAPI void ecore_wl2_window_resize(Ecore_Wl2_Window *window, int w, int h, int location);

/**
 * Raise a given Ecore_Wl2_Window
 *
 * @param window The Ecore_Wl2_Window which to raise
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_raise(Ecore_Wl2_Window *window);

/**
 * Get if a given window is alpha
 *
 * @param window The window to get if is alpha
 *
 * @return EINA_TRUE if window is alpha, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI Eina_Bool ecore_wl2_window_alpha_get(Ecore_Wl2_Window *window);

/**
 * Set a given window's alpha property
 *
 * @param window The window on which to set the alpha property
 * @param alpha EINA_TRUE to set window as alpha, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_alpha_set(Ecore_Wl2_Window *window, Eina_Bool alpha);

/**
 * Set a given window's transparent property
 *
 * @param window The window on which to set the transparent property
 * @param transparent EINA_TRUE to set window as transparent,
 *                    EINA_FALSE otherwise
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_transparent_set(Ecore_Wl2_Window *window, Eina_Bool transparent);

/**
 * Set the opaque region of the Ecore_Wl2_Window
 *
 * @param win The window
 * @param x The left point of the region.
 * @param y The top point of the region.
 * @param w The width of the region.
 * @param h The height of the region.
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_opaque_region_set(Ecore_Wl2_Window *window, int x, int y, int w, int h);

/**
 * Set the input region of the Ecore_Wl2_Window.
 *
 * To set an empty region, pass width and height as 0.
 *
 * An empty input region means the entire window surface will accept input.
 *
 * @param window The window to set the input region of
 * @param x The left point of the region.
 * @param y The top point of the region.
 * @param w The width of the region.
 * @param h The height of the region.
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_input_region_set(Ecore_Wl2_Window *window, int x, int y, int w, int h);

/**
 * Get if a given window is maximized
 *
 * @param window The window to get the maximized state of
 *
 * @return EINA_TRUE if window is maximized, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI Eina_Bool ecore_wl2_window_maximized_get(Ecore_Wl2_Window *window);

/**
 * Set the maximized state of a given window
 *
 * @param window The window to set the maximized state of
 * @param maximized EINA_TRUE to set maximized, EINA_FALSE to unset
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_maximized_set(Ecore_Wl2_Window *window, Eina_Bool maximized);

/**
 * Get if a given window is fullscreen
 *
 * @param window The window to get the fullscreen state of
 *
 * @return EINA_TRUE if window is fullscreen, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI Eina_Bool ecore_wl2_window_fullscreen_get(Ecore_Wl2_Window *window);

/**
 * Set the fullscreen state of a given window
 *
 * @param window The window to set the fullscreen state of
 * @param maximized EINA_TRUE to set fullscreen, EINA_FALSE to unset
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_fullscreen_set(Ecore_Wl2_Window *window, Eina_Bool fullscreen);

/**
 * Get if a given window is rotated
 *
 * @param window The window to get the rotation of
 *
 * @return The amount of rotation for this window, or -1 on failure
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI int ecore_wl2_window_rotation_get(Ecore_Wl2_Window *window);

/**
 * Set the rotation of a given window
 *
 * @param window The window to set the rotation of
 * @param rotation The amount of rotation
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_rotation_set(Ecore_Wl2_Window *window, int rotation);

/**
 * Set the title of a given window
 *
 * @param window The window to set the title of
 * @param title The title of the window
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_title_set(Ecore_Wl2_Window *window, const char *title);

/**
 * Set the class of a given window
 *
 * @param window The window to set the class of
 * @param clas The class of the window
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_class_set(Ecore_Wl2_Window *window, const char *clas);

/**
 * Get the geometry of a given window
 *
 * @brief The window geometry returned here is the window geometry as
 * recognized by xdg_surface_set_window_geometry. As such, it represents the
 * "visible bounds" of a window from the user's perspective.
 *
 * @param window The window to get the geometry of
 * @param x The left point of the window geometry
 * @param y The top point of the window geometry
 * @param w The width of the window geometry
 * @param h The height of the window geometry
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_geometry_get(Ecore_Wl2_Window *window, int *x, int *y, int *w, int *h);

/**
 * Set the geometry of a given window
 *
 * @brief The window geometry referenced here is the window geometry as
 * recognized by xdg_surface_set_window_geometry. As such, it represents the
 * "visible bounds" of a window from the user's perspective.
 *
 * @param window The window to set the geometry of
 * @param x The left point of the window geometry
 * @param y The top point of the window geometry
 * @param w The width of the window geometry
 * @param h The height of the window geometry
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_geometry_set(Ecore_Wl2_Window *window, int x, int y, int w, int h);

/**
 * Get the iconified state of a given window
 *
 * @param window The window to get the iconified state of
 *
 * @return EINA_TRUE if window is iconified, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI Eina_Bool ecore_wl2_window_iconified_get(Ecore_Wl2_Window *window);

/**
 * Iconify a window
 *
 * @param win The window to iconifiy
 * @param iconified The new iconified state to set
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_iconified_set(Ecore_Wl2_Window *window, Eina_Bool iconified);

/**
 * Retrieves the mouse position of the current window.
 *
 * @param window The window on which to retrieve the mouse position
 * @param x where to return the horizontal position. May be NULL. Returns 0 on error.
 * @param y where to return the vertical position. May be NULL. Returns 0 on error.
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_pointer_xy_get(Ecore_Wl2_Window *window, int *x, int *y);

/**
 * Set a given wl_surface to use as the pointer on a window
 *
 * @param window The window to set this surface as the pointer on
 * @param surface The surface to use as the pointer
 * @param hot_x The x coordinate to use as the cursor hot spot
 * @param hot_y The y coordinate to use as the cursor hot spot
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_pointer_set(Ecore_Wl2_Window *window, struct wl_surface *surface, int hot_x, int hot_y);

/**
 * Set a specific cursor on a given window
 *
 * @brief This function will try to find a matching cursor inside the existing
 * cursor theme and set the pointer which is over the given window to be
 * the specified cursor
 *
 * @param window The window to set the cursor on
 * @param cursor The name of the cursor to try and set
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_cursor_from_name_set(Ecore_Wl2_Window *window, const char *cursor);

/**
 * Set the type of a given window
 *
 * @param window The window to set the type of
 * @type The Ecore_Wl2_Window_Type to set on the window
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_type_set(Ecore_Wl2_Window *window, Ecore_Wl2_Window_Type type);

/* TODO: doxy */
/** @since 1.17 */
EAPI Ecore_Wl2_Input *ecore_wl2_window_input_get(Ecore_Wl2_Window *window);

/**
 * @defgroup Ecore_Wl2_Input_Group Wayland Library Input Functions
 * @ingroup Ecore_Wl2_Group
 *
 * Functions that deal with creating, destroying, or interacting with
 * Wayland Input
 */

/* TODO: doxy */
/** @since 1.17 */
EAPI void ecore_wl2_input_grab(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, unsigned int button);

/* TODO: doxy */
/** @since 1.17 */
EAPI void ecore_wl2_input_ungrab(Ecore_Wl2_Input *input);

/* TODO: doxy */
/** @since 1.17 */
EAPI struct wl_seat *ecore_wl2_input_seat_get(Ecore_Wl2_Input *input);

/**
 * @defgroup Ecore_Wl2_Dnd_Group Wayland Library Drag-n-Drop Functions
 * @ingroup Ecore_Wl2_Group
 *
 * Functions that deal with creating, destroying, or interacting with
 * Wayland Drag-n-Drop
 */

/* TODO: doxy */
/** @since 1.17 */
EAPI void ecore_wl2_dnd_drag_types_set(Ecore_Wl2_Input *input, const char **types);

/* TODO: doxy */
/** @since 1.17 */
EAPI void ecore_wl2_dnd_drag_start(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, Ecore_Wl2_Window *drag_window);

/* TODO: doxy */
/** @since 1.17 */
EAPI Eina_Bool ecore_wl2_dnd_drag_get(Ecore_Wl2_Input *input, const char *type);

/* TODO: doxy */
/** @since 1.17 */
EAPI void ecore_wl2_dnd_drag_end(Ecore_Wl2_Input *input);

/* TODO: doxy */
/** @since 1.17 */
EAPI Eina_Bool ecore_wl2_dnd_selection_owner_has(Ecore_Wl2_Input *input);

/* TODO: doxy */
/** @since 1.17 */
EAPI Eina_Bool ecore_wl2_dnd_selection_set(Ecore_Wl2_Input *input, const char **types);

/* TODO: doxy */
/** @since 1.17 */
EAPI Eina_Bool ecore_wl2_dnd_selection_get(Ecore_Wl2_Input *input, const char *type);

/* TODO: doxy */
/** @since 1.17 */
EAPI Eina_Bool ecore_wl2_dnd_selection_clear(Ecore_Wl2_Input *input);

/**
 * @defgroup Ecore_Wl2_Subsurface_Group Functions to manipulate subsurfaces.
 * @ingroup Ecore_Wl2_Group
 *
 * Functions to manipulate wayland subsurfaces, using Ecore_Wl2_Subsurface.
 *
 * This API is intended to expose Wayland subsurface functionality, although it
 * should not be necessary for most applications to use it, as soon as we have
 * means to make Evas automatically switch Evas images to use subsurfaces.
 *
 * It can/should be used, for instance, when subsurfaces are needed to be not
 * in sync with the main window surface.
 */

/**
 * Create and return a new subsurface.
 *
 * Create a new surface (and subsurface interface), with the parent surface
 * being the one associated with the given @param win.
 *
 * The @param win must be visible, otherwise there will be no surface created
 * for it yet.
 *
 * @return the allocated and initialized Ecore_Wl2_Subsurface object, or
 * NULL on failure
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI Ecore_Wl2_Subsurface *ecore_wl2_subsurface_new(Ecore_Wl2_Window *window);

/**
 * Destroy the given subsurface, as well as the surface associated with it.
 *
 * @param subsurface the subsurface
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_del(Ecore_Wl2_Subsurface *subsurface);

/**
 * Get the wl_surface for this subsurface
 *
 * @param subsurface the subsurface
 *
 * @return the wl_surface associated with this subsurface, or NULL on failure
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI struct wl_surface *ecore_wl2_subsurface_surface_get(Ecore_Wl2_Subsurface *subsurface);

/**
 * Set the position of this subsurface, relative to its parent surface.
 * If subsurface is defined and the x, y coordinates differ from the currently
 * tracked position, this also schedules a sub-surface position change.
 *
 * @param subsurface the subsurface
 * @param x coordinate in the parent surface
 * @param y coordinate in the parent surface
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_position_set(Ecore_Wl2_Subsurface *subsurface, int x, int y);

/**
 * Get the position of this subsurface, relative to its parent surface.
 * Coordinates will be returned in x and y if non-NULL.
 *
 * @param subsurface the subsurface
 * @param x coordinate in the parent surface, or NULL to ignore
 * @param y coordinate in the parent surface, or NULL to ignore
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_position_get(Ecore_Wl2_Subsurface *subsurface, int *x, int *y);

/**
 * Place subsurface on layer above a reference surface
 *
 * Moves the @param subsurface to just above the reference @param
 * surface, changing the z-order.  The reference @param surface must
 * be either a sibling or parent surface, else a protocol error will
 * be generated.
 *
 * @param subsurface the subsurface
 * @param surface the sibling reference surface
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_place_above(Ecore_Wl2_Subsurface *subsurface, struct wl_surface *surface);

/**
 * Place subsurface on layer below a reference surface
 *
 * See ecore_wl2_subsurface_place_above.
 *
 * @param subsurface the subsurface
 * @param surface the sibling reference surface
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_place_below(Ecore_Wl2_Subsurface *subsurface, struct wl_surface *surface);

/**
 * Enables or disables sub-surface synchronization
 *
 * When synchronization is enabled, surface commits on the subsurface
 * will be cached and only applied when the parent surface's state is
 * applied.  This ensures atomic updates of the parent and all of its
 * synchronized sub-surfaces.
 *
 * When synchronization is disabled, commits will apply to the pending
 * state directly without caching, just like a normal wl_surface.  If
 * there are already cached events when this is set, those events are
 * applied simultaneously with the desync event.
 *
 * Attempting to enable synchronization when the subsurface already
 * thinks it's sync'd, or desync when it believes its desync'd, will
 * be trivially ignored and will not generate a Wayland event.
 *
 * See Wayland's set_desync documentation for further details and
 * exceptional cases.
 *
 * @param subsurface the subsurface
 * @param sync true to enable synchronization, false to desynchronize
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_sync_set(Ecore_Wl2_Subsurface *subsurface, Eina_Bool sync);

/**
 * Set an opaque region for the given subsurface.
 *
 * This is an optimization hint to the compositor to allow it avoid
 * redrawing content unnecessarily.  Note that marking transparent
 * content as opaque will cause repaint artifacts.
 *
 * Use a 0x0 region size to unset the opaque region.
 *
 * @param subsurface the subsurface
 * @param x coordinate in the parent surface
 * @param y coordinate in the parent surface
 * @param w width to set as opaque
 * @param h height to set as opaque
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_opaque_region_set(Ecore_Wl2_Subsurface *subsurface, int x, int y, int w, int h);

/**
 * @defgroup Ecore_Wl2_Output_Group Wayland Library Output Functions
 * @ingroup Ecore_Wl2_Group
 *
 * Functions that deal with creating, destroying, or interacting with
 * Wayland Outputs
 */

/**
 * Return the DPI of a given output
 *
 * This is a simplistic call to get DPI. It does not account for differing
 * DPI in the x and y axes nor does it account for multihead or xinerama and
 * xrandr where different parts of the screen may have different DPI etc.
 *
 * @param output The output to get the DPI of
 *
 * @return the general screen DPI (dots/pixels per inch).
 *
 * @ingroup Ecore_Wl2_Output_Group
 * @since 1.17
 */
EAPI int ecore_wl2_output_dpi_get(Ecore_Wl2_Output *output);

/**
 * Return the version of the display's compositor object
 *
 * @param disp the display to get the compositor object version from
 *
 * @return the version of the display's compositor object
 *
 * @ingroup Ecore_Wl2_Display_Group
 * @since 1.17
 */
EAPI int ecore_wl2_display_compositor_version_get(Ecore_Wl2_Display *disp);

/** @since 1.17 */
EAPI Ecore_Wl2_Seat *ecore_wl2_seat_create(Ecore_Wl2_Display *display, const char *name, const struct wl_seat_interface *implementation, int version, Ecore_Wl2_Bind_Cb bind_cb, Ecore_Wl2_Unbind_Cb unbind_cb);
/** @since 1.17 */
EAPI void ecore_wl2_seat_destroy(Ecore_Wl2_Seat *seat);
/** @since 1.17 */
EAPI void ecore_wl2_seat_capabilities_send(Ecore_Wl2_Seat *seat, enum wl_seat_capability caps);
/** @since 1.17 */
EAPI void ecore_wl2_seat_pointer_release(Ecore_Wl2_Seat *seat);

/** @since 1.17 */
EAPI Ecore_Wl2_Pointer *ecore_wl2_pointer_get(Ecore_Wl2_Seat *seat);
/** @since 1.17 */
EAPI Eina_Bool ecore_wl2_pointer_resource_create(Ecore_Wl2_Pointer *ptr, struct wl_client *client, const struct wl_pointer_interface *implementation, int version, uint32_t id);

/** @since 1.17 */
EAPI Ecore_Wl2_Keyboard *ecore_wl2_keyboard_get(Ecore_Wl2_Seat *seat);
/** @since 1.17 */
EAPI Eina_Bool ecore_wl2_keyboard_resource_create(Ecore_Wl2_Keyboard *kbd, struct wl_client *client, const struct wl_keyboard_interface *implementation, int version, uint32_t id);
/** @since 1.17 */
EAPI void ecore_wl2_keyboard_repeat_info_set(Ecore_Wl2_Keyboard *kbd, double rate, double delay);

/** @since 1.17 */
EAPI Ecore_Wl2_Display *ecore_wl2_window_display_get(const Ecore_Wl2_Window *window);

/* # ifdef __cplusplus */
/* } */
/* # endif */

# endif

# undef EAPI
# define EAPI

#endif
