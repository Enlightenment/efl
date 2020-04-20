#ifndef _ECORE_WAYLAND_H_
# define _ECORE_WAYLAND_H_

/*
 * Wayland supoprt is considered experimental as wayland itself is still
 * unstable and liable to change core protocol. If you use this api, it is
 * possible it will break in future, until this notice is removed.
 */

# include <Eina.h>
# include <Ecore.h>
# include <wayland-client.h>
# include <wayland-cursor.h>
# include <xkbcommon/xkbcommon.h>

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

# warning The Ecore_Wayland library has been deprecated. Please use the Ecore_Wl2 library

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Ecore_Wl_Output Ecore_Wl_Output;
typedef struct _Ecore_Wl_Input Ecore_Wl_Input;
typedef struct _Ecore_Wl_Global Ecore_Wl_Global; /** @since 1.7.6 */
typedef struct _Ecore_Wl_Subsurf Ecore_Wl_Subsurf; /** @since 1.8 */

# ifndef _ECORE_WAYLAND_WINDOW_PREDEF
typedef struct _Ecore_Wl_Window Ecore_Wl_Window;
# endif

/**
 * @deprecated Do Not Use
 * @since 1.7
 */
typedef struct _Ecore_Wl_Dnd Ecore_Wl_Dnd;

typedef struct _Ecore_Wl_Dnd_Source Ecore_Wl_Dnd_Source;
typedef struct _Ecore_Wl_Dnd_Target Ecore_Wl_Dnd_Target;

typedef struct _Ecore_Wl_Event_Mouse_In Ecore_Wl_Event_Mouse_In;
typedef struct _Ecore_Wl_Event_Mouse_Out Ecore_Wl_Event_Mouse_Out;
typedef struct _Ecore_Wl_Event_Focus_In Ecore_Wl_Event_Focus_In;
typedef struct _Ecore_Wl_Event_Focus_Out Ecore_Wl_Event_Focus_Out;
typedef struct _Ecore_Wl_Event_Window_Configure Ecore_Wl_Event_Window_Configure;
typedef struct _Ecore_Wl_Event_Dnd_Enter Ecore_Wl_Event_Dnd_Enter;
typedef struct _Ecore_Wl_Event_Dnd_Position Ecore_Wl_Event_Dnd_Position;
typedef struct _Ecore_Wl_Event_Dnd_Leave Ecore_Wl_Event_Dnd_Leave;
typedef struct _Ecore_Wl_Event_Dnd_Drop Ecore_Wl_Event_Dnd_Drop;
typedef struct _Ecore_Wl_Event_Dnd_End Ecore_Wl_Event_Dnd_End;
typedef struct _Ecore_Wl_Event_Data_Source_Cancelled Ecore_Wl_Event_Data_Source_Cancelled; /** @since 1.12 */
typedef struct _Ecore_Wl_Event_Data_Source_Send Ecore_Wl_Event_Data_Source_Send; /** @since 1.7 */
typedef struct _Ecore_Wl_Event_Data_Source_Target  Ecore_Wl_Event_Data_Source_Target; /** @since 1.12 */
typedef struct _Ecore_Wl_Event_Selection_Data_Ready Ecore_Wl_Event_Selection_Data_Ready; /** @since 1.7 */
typedef struct _Ecore_Wl_Event_Interfaces_Bound Ecore_Wl_Event_Interfaces_Bound;

enum _Ecore_Wl_Window_Type
{
   ECORE_WL_WINDOW_TYPE_NONE,
   ECORE_WL_WINDOW_TYPE_TOPLEVEL,
   ECORE_WL_WINDOW_TYPE_FULLSCREEN,
   ECORE_WL_WINDOW_TYPE_MAXIMIZED,
   ECORE_WL_WINDOW_TYPE_TRANSIENT,
   ECORE_WL_WINDOW_TYPE_MENU,
   ECORE_WL_WINDOW_TYPE_DND,
   ECORE_WL_WINDOW_TYPE_CUSTOM
};

enum _Ecore_Wl_Window_Buffer_Type
{
   ECORE_WL_WINDOW_BUFFER_TYPE_EGL_WINDOW,
   ECORE_WL_WINDOW_BUFFER_TYPE_EGL_IMAGE,
   ECORE_WL_WINDOW_BUFFER_TYPE_SHM
};

typedef enum _Ecore_Wl_Window_Type Ecore_Wl_Window_Type;
typedef enum _Ecore_Wl_Window_Buffer_Type Ecore_Wl_Window_Buffer_Type;

/** @since 1.7.6 */
struct _Ecore_Wl_Global
{
   EINA_INLIST;
   unsigned int id;
   char *interface;
   unsigned int version;
};

struct _Ecore_Wl_Event_Mouse_In
{
   int modifiers;
   int x, y;
   struct
     {
        int x, y;
     } root;
   unsigned int window;
   unsigned int event_window;
   unsigned int root_window;
   unsigned int timestamp;
};

struct _Ecore_Wl_Event_Mouse_Out
{
   int modifiers;
   int x, y;
   struct
     {
        int x, y;
     } root;
   unsigned int window;
   unsigned int event_window;
   unsigned int root_window;
   unsigned int timestamp;
};

struct _Ecore_Wl_Event_Focus_In
{
   unsigned int win;
   unsigned int timestamp;
};

struct _Ecore_Wl_Event_Focus_Out
{
   unsigned int win;
   unsigned int timestamp;
};

struct _Ecore_Wl_Event_Window_Configure
{
   unsigned int win;
   unsigned int event_win;
   int x, y, w, h;
   int edges;
};

struct _Ecore_Wl_Event_Dnd_Enter
{
   unsigned int win, source;
   char **types;
   int num_types;
   unsigned int serial;
   struct wl_data_offer *offer;
   struct
     {
        int x, y;
     } position;
};

struct _Ecore_Wl_Event_Dnd_Position
{
   unsigned int win, source;
   struct
     {
        int x, y;
     } position;
};

struct _Ecore_Wl_Event_Dnd_Leave
{
   unsigned int win, source;
};

struct _Ecore_Wl_Event_Dnd_Drop
{
   unsigned int win, source;
   struct
     {
        int x, y;
     } position;
};

struct _Ecore_Wl_Event_Dnd_End
{
   unsigned int win, source;
};

/** @since 1.12 */
struct _Ecore_Wl_Event_Data_Source_Cancelled
{
   unsigned int win, source;
};

/** @since 1.7 */
struct _Ecore_Wl_Event_Data_Source_Send
{
   char *type;
   int fd;
};

/** @since 1.12 */
struct _Ecore_Wl_Event_Data_Source_Target
{
   char *type;
};

/** @since 1.7 */
struct _Ecore_Wl_Event_Selection_Data_Ready
{
   char *data;
   int len;
   Eina_Bool done;
};

struct _Ecore_Wl_Event_Interfaces_Bound
{
   Eina_Bool compositor : 1;
   Eina_Bool shm : 1;
   Eina_Bool shell : 1;
   Eina_Bool output : 1;
   Eina_Bool seat : 1;
   Eina_Bool data_device_manager : 1;
   Eina_Bool subcompositor : 1;
};

/**
 * @file
 * @brief Ecore functions for dealing with the Wayland window system.
 *
 * @defgroup Ecore_Wl_Group Ecore_Wayland - Wayland integration
 * @ingroup Ecore
 * Ecore_Wl provides a wrapper and convenience functions for using the
 * Wayland protocol in implementing a window system. Function groups for
 * this part of the library include the following:
 *
 * @li @ref Ecore_Wl_Init_Group
 * @li @ref Ecore_Wl_Display_Group
 * @li @ref Ecore_Wl_Flush_Group
 * @li @ref Ecore_Wl_Window_Group
 * @li @ref Ecore_Wl_Input_Group
 * @li @ref Ecore_Wl_Dnd_Group
 * @li @ref Ecore_Wl_Subsurf
 */

EAPI extern int ECORE_WL_EVENT_MOUSE_IN;
EAPI extern int ECORE_WL_EVENT_MOUSE_OUT;
EAPI extern int ECORE_WL_EVENT_FOCUS_IN;
EAPI extern int ECORE_WL_EVENT_FOCUS_OUT;
EAPI extern int ECORE_WL_EVENT_WINDOW_CONFIGURE;
EAPI extern int ECORE_WL_EVENT_DND_ENTER;
EAPI extern int ECORE_WL_EVENT_DND_POSITION;
EAPI extern int ECORE_WL_EVENT_DND_LEAVE;
EAPI extern int ECORE_WL_EVENT_DND_DROP;
EAPI extern int ECORE_WL_EVENT_DND_OFFER; /** @since 1.8 */
EAPI extern int ECORE_WL_EVENT_DND_END; /** @since 1.8 */
EAPI extern int ECORE_WL_EVENT_DATA_SOURCE_TARGET; /** @since 1.7 */
EAPI extern int ECORE_WL_EVENT_DATA_SOURCE_SEND; /** @since 1.7 */
EAPI extern int ECORE_WL_EVENT_DATA_SOURCE_CANCELLED; /** @since 1.7 */
EAPI extern int ECORE_WL_EVENT_SELECTION_DATA_READY; /** @since 1.7 */
EAPI extern int ECORE_WL_EVENT_INTERFACES_BOUND;

/**
 * @defgroup Ecore_Wl_Init_Group Wayland Library Init and Shutdown Functions
 * @ingroup Ecore_Wl_Group
 *
 * Functions that start and shutdown the Ecore Wayland Library.
 */

/**
 * @ingroup Ecore_Wl_Init_Group
 * @brief Initializes the Wayland display connection to the given display.
 *
 * @param   name Display target name. if @c NULL, the default display is
 *          assumed.
 * @return  The number of times the library has been initialized without being
 *          shut down. @c 0 is returned if an error occurs.
 *
 */
EAPI int ecore_wl_init(const char *name);

/**
 * @ingroup Ecore_Wl_Init_Group
 * @brief Shuts down the Ecore Wayland Library.
 *
 * In shutting down the library, the Wayland display connection is terminated
 * and any event handlers for it are removed.
 *
 * @return  The number of times the library has been initialized without
 *          being shut down.
 *
 */
EAPI int ecore_wl_shutdown(void);

/**
 * @defgroup Ecore_Wl_Flush_Group Wayland Synchronization Functions
 * @ingroup Ecore_Wl_Group
 *
 * Functions that ensure that all commands which have been issued by the
 * Ecore Wayland library have been sent to the server.
 */

/**
 * @ingroup Ecore_Wl_Flush_Group
 * @brief Sends all Wayland commands to the Wayland Display.
 *
 * @since 1.2
 */
EAPI void ecore_wl_flush(void);

/**
 * @ingroup Ecore_Wl_Flush_Group
 * @brief Flushes the command buffer and waits until all requests have been
 * processed by the server.
 *
 * @since 1.2
 */
EAPI void ecore_wl_sync(void);

/**
 * @defgroup Ecore_Wl_Display_Group Wayland Display Functions
 * @ingroup Ecore_Wl_Group
 *
 * Functions that set and retrieve various information about the Wayland Display.
 */

/**
 * @ingroup Ecore_Wl_Display_Group
 * @brief Retrieves the Wayland Shm Interface used for the current Wayland connection.
 *
 * @return The current wayland shm interface, or NULL on error
 *
 * @since 1.2
 */
EAPI struct wl_shm *ecore_wl_shm_get(void);

/**
 * @ingroup Ecore_Wl_Display_Group
 * @brief Retrieves the Wayland Display Interface used for the current Wayland connection.
 *
 * @return The current wayland display interface, or NULL on error
 *
 * @since 1.2
 */
EAPI struct wl_display *ecore_wl_display_get(void);

/**
 * @brief Retrieves the Compositor interface.
 *
 * This interface is used by clients to request the creation of surfaces and
 * regions.
 *
 * @return The current wayland compositor interface, or NULL on error
 * @since 1.8
 */

/**
 * @ingroup Ecore_Wl_Display_Group
 * @brief Retrieves the size of the current screen.
 *
 * @param w Where to return the width. May be NULL. Returns @c 0 on error.
 * @param h Where to return the height. May be NULL. Returns @c 0 on error.
 *
 * @since 1.2
 */
EAPI void ecore_wl_screen_size_get(int *w, int *h);

/**
 * @ingroup Ecore_Wl_Display_Group
 * @brief Retrieves the position of the current screen.
 *
 * @param x Where to return the horizontal position. May be NULL. Returns @c 0 on error.
 * @param y Where to return the vertical position. May be NULL. Returns @c 0 on error.
 *
 * @since 1.2
 */
EAPI void ecore_wl_pointer_xy_get(int *x, int *y);

/**
 * @brief Returns the screen DPI.
 *
 * This is a simplistic call to get DPI. It does not account for differing
 * DPI in the x and y axes nor does it account for multihead or xinerama and
 * xrandr where different parts of the screen may have different DPI etc.
 *
 * @return the general screen DPI (dots/pixels per inch).
 *
 * @since 1.2
 */
EAPI int ecore_wl_dpi_get(void);

/**
 * @brief Dispatches Wayland events.
 *
 * Dispatches any events queued on the default queue.  Wayland will attempt
 * to read from the display fd and queue things up according to the proxy
 * assignment.
 */
EAPI void ecore_wl_display_iterate(void);

/*
 * @ingroup Ecore_Wl_Display_Group
 * @since 1.8
 */
EAPI Eina_Bool ecore_wl_animator_source_set(Ecore_Animator_Source source);

/**
 * @brief Retrieves the requested cursor from the cursor theme.
 *
 * @param cursor_name The desired cursor name to be looked up in the theme
 * @return the cursor or NULL if the cursor cannot be found
 *
 * @since 1.2
 */
EAPI struct wl_cursor *ecore_wl_cursor_get(const char *cursor_name);

/**
 * @defgroup Ecore_Wl_Input_Group Functions dealing with Wayland Input
 * @ingroup Ecore_Wl_Group
 *
 * Functions to interface with Wayland Input.
 */

/**
 * @ingroup Ecore_Wl_Input_Group
 * @since 1.8
 */
EAPI Ecore_Wl_Input *ecore_wl_input_get(void);
EAPI void ecore_wl_input_grab(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int button);
EAPI void ecore_wl_input_ungrab(Ecore_Wl_Input *input);
EAPI void ecore_wl_input_pointer_set(Ecore_Wl_Input *input, struct wl_surface *surface, int hot_x, int hot_y);
EAPI void ecore_wl_input_cursor_from_name_set(Ecore_Wl_Input *input, const char *cursor_name);
EAPI void ecore_wl_input_cursor_default_restore(Ecore_Wl_Input *input);
EAPI struct wl_seat *ecore_wl_input_seat_get(Ecore_Wl_Input *input);

EAPI Eina_Inlist *ecore_wl_outputs_get(void);

/**
 * @ingroup Ecore_Wl_Input_Group
 * @since 1.12
 */
EAPI void ecore_wl_input_cursor_size_set(Ecore_Wl_Input *input, const int size);
EAPI void ecore_wl_input_cursor_theme_name_set(Ecore_Wl_Input *input, const char *cursor_theme_name);

/**
 * @ingroup Ecore_Wl_Input_Group
 * @since 1.17
 */
EAPI struct xkb_keymap *ecore_wl_input_keymap_get(Ecore_Wl_Input *input);


/**
 * @ingroup Ecore_Wl_Display_Group
 * @brief Retrieves the Wayland Globals Interface list used for the current Wayland connection.
 *
 * This call, if done after the ECORE_WL_EVENT_INTERFACES_BOUND event was
 * received already, won't block the mainloop or trigger a dispatch. It will
 * return the current globals immediately. However, if done before this event,
 * it will probably block the mainloop waiting for the sync "done" event to be
 * received (by using one or more wl_display_dispatch call), and then finally
 * return the wl globals list.
 *
 * There's no need to call dispatch manually, since this call will do it if necessary.
 *
 * @return The current wayland globals interface list, or NULL on error
 *
 * @since 1.7.6
 */
EAPI Eina_Inlist *ecore_wl_globals_get(void);

/**
 * @ingroup Ecore_Wl_Display_Group
 * @brief Retrieves the Wayland Registry used for the current Wayland connection.
 *
 * @return The current wayland registry, or NULL on error
 *
 * @since 1.7.6
 */
EAPI struct wl_registry *ecore_wl_registry_get(void);

/**
 * @defgroup Ecore_Wl_Window_Group Wayland Window functions.
 * @ingroup Ecore_Wl_Group
 *
 * Functions that can be used to create a Wayland window.
 */

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Creates a new window.
 *
 * @param parent The parent window to use. If @p parent is @c 0, the root window
 *               of the default display is used.
 * @param x      X Position
 * @param y      Y position
 * @param w      Width
 * @param h      Height
 * @param buffer_type The type of the buffer to be used to create a new Ecore_Wl_Window.
 *
 * @return The new window, or NULL on failure
 *
 * @since 1.2
 */
EAPI Ecore_Wl_Window *ecore_wl_window_new(Ecore_Wl_Window *parent, int x, int y, int w, int h, int buffer_type);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Deletes the given window.
 *
 * @param win The given window
 *
 * @since 1.2
 */
EAPI void ecore_wl_window_free(Ecore_Wl_Window *win);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Signals for Wayland to initiate a window move.
 *
 * The position requested (@p x, @p y) is not honored by Wayland because
 * Wayland does not allow specific window placement to be set.
 *
 * @param win The window to move.
 * @param x   X Position
 * @param y   Y Position
 *
 * @since 1.2
 */
EAPI void ecore_wl_window_move(Ecore_Wl_Window *win, int x, int y);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Signals for Wayland to initiate a window resize.
 *
 * The size requested (@p w, @p h) is not honored by Wayland because
 * Wayland does not allow specific window sizes to be set.
 *
 * @param win      The window to resize.
 * @param w        Width
 * @param h        Height
 * @param location The edge of the window from where the resize should start.
 *
 * @since 1.2
 */
EAPI void ecore_wl_window_resize(Ecore_Wl_Window *win, int w, int h, int location);
EAPI void ecore_wl_window_damage(Ecore_Wl_Window *win, int x, int y, int w, int h);
EAPI void ecore_wl_window_buffer_attach(Ecore_Wl_Window *win, struct wl_buffer *buffer, int x, int y);

/* @since 1.8 */
EAPI void ecore_wl_window_commit(Ecore_Wl_Window *win);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Shows a window.
 *
 * Synonymous to "mapping" a window in Wayland System terminology.
 *
 * @param win The window to show.
 *
 * @since 1.2
 */
EAPI void ecore_wl_window_show(Ecore_Wl_Window *win);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Hides a window.
 *
 * Synonymous to "unmapping" a window in Wayland System terminology.
 *
 * @param win The window to hide.
 *
 * @since 1.2
 */
EAPI void ecore_wl_window_hide(Ecore_Wl_Window *win);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Raises a window.
 *
 * @param win The window to raise.
 *
 * @since 1.2
 */
EAPI void ecore_wl_window_raise(Ecore_Wl_Window *win);
EAPI void ecore_wl_window_maximized_set(Ecore_Wl_Window *win, Eina_Bool maximized);
EAPI Eina_Bool ecore_wl_window_maximized_get(Ecore_Wl_Window *win);
EAPI void ecore_wl_window_fullscreen_set(Ecore_Wl_Window *win, Eina_Bool fullscreen);
EAPI Eina_Bool ecore_wl_window_fullscreen_get(Ecore_Wl_Window *win);
EAPI void ecore_wl_window_transparent_set(Ecore_Wl_Window *win, Eina_Bool transparent);
EAPI Eina_Bool ecore_wl_window_transparent_get(Ecore_Wl_Window *win);
EAPI void ecore_wl_window_update_size(Ecore_Wl_Window *win, int w, int h);
EAPI void ecore_wl_window_update_location(Ecore_Wl_Window *win, int x, int y);
EAPI struct wl_surface *ecore_wl_window_surface_get(Ecore_Wl_Window *win);
EAPI struct wl_shell_surface *ecore_wl_window_shell_surface_get(Ecore_Wl_Window *win);
EAPI struct xdg_surface *ecore_wl_window_xdg_surface_get(Ecore_Wl_Window *win); /** @since 1.12 */
EAPI Ecore_Wl_Window *ecore_wl_window_find(unsigned int id);
EAPI void ecore_wl_window_type_set(Ecore_Wl_Window *win, Ecore_Wl_Window_Type type);
EAPI void ecore_wl_window_pointer_set(Ecore_Wl_Window *win, struct wl_surface *surface, int hot_x, int hot_y);
EAPI void ecore_wl_window_cursor_from_name_set(Ecore_Wl_Window *win, const char *cursor_name);
EAPI void ecore_wl_window_cursor_default_restore(Ecore_Wl_Window *win);
EAPI void ecore_wl_window_parent_set(Ecore_Wl_Window *win, Ecore_Wl_Window *parent);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Iconifies a window.
 *
 * @param win The window to iconifiy
 *
 * @since 1.12
 */
EAPI void ecore_wl_window_iconified_set(Ecore_Wl_Window *win, Eina_Bool iconified);
EAPI Eina_Bool ecore_wl_window_iconified_get(Ecore_Wl_Window *win);


EAPI int ecore_wl_window_id_get(Ecore_Wl_Window *win);
EAPI void ecore_wl_window_title_set(Ecore_Wl_Window *win, const char *title);
EAPI void ecore_wl_window_class_name_set(Ecore_Wl_Window *win, const char *class_name);
EAPI int ecore_wl_window_surface_id_get(Ecore_Wl_Window *win);

EAPI Ecore_Wl_Input *ecore_wl_window_keyboard_get(Ecore_Wl_Window *win);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Returns a wl_surface with no association to any wl_shell_surface.
 *
 * @param win The window which will use this newly created surface.
 * @return wl_surface or NULL on error
 *
 * @since 1.8
 */
EAPI struct wl_surface *ecore_wl_window_surface_create(Ecore_Wl_Window *win);

EAPI void ecore_wl_window_alpha_set(Ecore_Wl_Window *win, Eina_Bool alpha);
EAPI Eina_Bool ecore_wl_window_alpha_get(Ecore_Wl_Window *win);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Returns the Ecore_Wl_Window from a wl_surface.
 *
 * @param surface The surface for which to find the Ecore_Wl_Window from
 * @return the Ecore_Wl_Window associated with this surface, or NULL if not found
 *
 * @since 1.8
 */
EAPI Ecore_Wl_Window *ecore_wl_window_surface_find(struct wl_surface *surface);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Sets the input region of the Ecore_Wl_Window.
 *
 * To set an empty region, pass width and height as 0.
 *
 * An empty input region means the entire window surface will accept input.
 *
 * @param win The window
 * @param x The left point of the region.
 * @param y The top point of the region.
 * @param w The width of the region.
 * @param h The height of the region.
 *
 * @since 1.8
 */
EAPI void ecore_wl_window_input_region_set(Ecore_Wl_Window *win, int x, int y, int w, int h);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Sets the opaque region of the Ecore_Wl_Window.
 *
 * @param win The window
 * @param x The left point of the region.
 * @param y The top point of the region.
 * @param w The width of the region.
 * @param h The height of the region.
 *
 * @since 1.8
 */
EAPI void ecore_wl_window_opaque_region_set(Ecore_Wl_Window *win, int x, int y, int w, int h);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Sets the rotation of the Ecore_Wl_Window.
 *
 * @param win The window
 * @param rotation The degree of rotation for this window
 *
 * @since 1.8
 */
EAPI void ecore_wl_window_rotation_set(Ecore_Wl_Window *win, int rotation);

/**
 * @ingroup Ecore_Wl_Window_Group
 * @brief Gets the rotation of the Ecore_Wl_Window.
 *
 * @param win The window
 * @return The degree of rotation for this window
 *
 * @since 1.8
 */
EAPI int ecore_wl_window_rotation_get(Ecore_Wl_Window *win);

/**
 * @defgroup Ecore_Wl_Dnd_Group Functions dealing with Wayland Drag-n-Drop
 * @ingroup Ecore_Wl_Group
 *
 * Functions to interface with Wayland Drag-n-Drop
 */

/**
 * @deprecated use ecore_wl_dnd_selection_set
 * @since 1.7
*/
EINA_DEPRECATED EAPI Eina_Bool ecore_wl_dnd_set_selection(Ecore_Wl_Dnd *dnd, const char **types_offered);

/**
 * @deprecated use ecore_wl_dnd_selection_get
 * @since 1.7
*/
EINA_DEPRECATED EAPI Eina_Bool ecore_wl_dnd_get_selection(Ecore_Wl_Dnd *dnd, const char *type);

/**
 * @deprecated Do Not Use
 * @since 1.7
 */
EINA_DEPRECATED EAPI Ecore_Wl_Dnd *ecore_wl_dnd_get(void);

/**
 * @deprecated use ecore_wl_dnd_drag_start
 * @since 1.7
 */
EINA_DEPRECATED EAPI Eina_Bool ecore_wl_dnd_start_drag(Ecore_Wl_Dnd *dnd);

/**
 * @deprecated use ecore_wl_dnd_selection_owner_has
 * @since 1.7
 */
EINA_DEPRECATED EAPI Eina_Bool ecore_wl_dnd_selection_has_owner(Ecore_Wl_Dnd *dnd);

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @return false on error, true otherwise
 *
 * @since 1.8
 */
EAPI Eina_Bool ecore_wl_dnd_selection_set(Ecore_Wl_Input *input, const char **types_offered);

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @return false if type not found or on error, true otherwise
 *
 * @since 1.8
 */
EAPI Eina_Bool ecore_wl_dnd_selection_get(Ecore_Wl_Input *input, const char *type);

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @return true if input has a selection source, false otherwise or on error
 *
 * @since 1.8
 */
EAPI Eina_Bool ecore_wl_dnd_selection_owner_has(Ecore_Wl_Input *input);

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @return false on error, true otherwise
 *
 * @since 1.8
 */
EAPI Eina_Bool ecore_wl_dnd_selection_clear(Ecore_Wl_Input *input);

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI void ecore_wl_dnd_drag_start(Ecore_Wl_Input *input, Ecore_Wl_Window *win, Ecore_Wl_Window *dragwin, int x, int y, int w, int h);

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI void ecore_wl_dnd_drag_end(Ecore_Wl_Input *input);

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @return false if type not found or on error, true otherwise
 *
 * @since 1.8
 */
EAPI Eina_Bool ecore_wl_dnd_drag_get(Ecore_Wl_Input *input, const char *type);

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI void ecore_wl_dnd_drag_types_set(Ecore_Wl_Input *input, const char **types_offered);

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @return the data types being offered for drag-and-drop, or NULL on error
 *
 * @since 1.8
 */
EAPI struct wl_array *ecore_wl_dnd_drag_types_get(Ecore_Wl_Input *input);

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
/* EAPI Ecore_Wl_Dnd_Source *ecore_wl_dnd_drag_source_create(Ecore_Wl_Dnd *dnd); */

/**
 * Enables/disables server mode.
 *
 * With this enabled, the compositor acts as a Wayland server, iterating
 * the display and dispatching events.  With it disabled, the process is
 * handled as a client.
 */
EAPI void ecore_wl_server_mode_set(Eina_Bool on);

/**
 * @defgroup Ecore_Wl_Subsurf Functions to manipulate subsurfaces.
 * @ingroup Ecore_Wl_Group
 *
 * Functions to manipulate wayland subsurfaces, using Ecore_Wl_Subsurf.
 *
 * This API is intended to expose Wayland subsurface functionality, although it
 * should not be necessary for most applications to use it, as soon as we have
 * means to make Evas automatically switch Evas images to use subsurfaces.
 *
 * It can/should be used, for instance, when subsurfaces are needed to be not
 * in sync with the main window surface.
 */

/**
 * @ingroup Ecore_Wl_Subsurf
 * @brief Creates and returns a new subsurface.
 *
 * Create a new surface (and subsurface interface), with the parent surface
 * being the one associated with the given @p win.
 *
 * @p win must be visible, otherwise there will be no surface created
 * for it yet.
 * @param win The window
 *
 * @return the allocated and initialized Ecore_Wl_Subsurf object, or
 * NULL on failure
 *
 * @since 1.8
 */
EAPI Ecore_Wl_Subsurf *ecore_wl_subsurf_create(Ecore_Wl_Window *win);

/**
 * @ingroup Ecore_Wl_Subsurf
 * @brief Destroys the given subsurface, as well as the surface associated with it.
 *
 * @param ess the subsurface
 *
 * @since 1.8
 */
EAPI void ecore_wl_subsurf_del(Ecore_Wl_Subsurf *ess);

/**
 * @ingroup Ecore_Wl_Subsurf
 * @brief Gets the wl_surface for this subsurface.
 *
 * @param ess the subsurface
 * @return the wl_surface associated with this subsurface, or NULL on failure
 *
 * @since 1.8
 */
EAPI struct wl_surface *ecore_wl_subsurf_surface_get(Ecore_Wl_Subsurf *ess);

/**
 * @ingroup Ecore_Wl_Subsurf
 * @brief Sets the position of this subsurface, relative to its parent surface.
 * If ess is defined and the x, y coordinates differ from the currently
 * tracked position, this also schedules a sub-surface position change.
 *
 * @param ess the subsurface
 * @param x coordinate in the parent surface
 * @param y coordinate in the parent surface
 *
 * @since 1.8
 */
EAPI void ecore_wl_subsurf_position_set(Ecore_Wl_Subsurf *ess, int x, int y);

/**
 * @ingroup Ecore_Wl_Subsurf
 * @brief Gets the position of this subsurface, relative to its parent surface.
 * Coordinates will be returned in x and y if non-NULL.
 *
 * @param ess the subsurface
 * @param x coordinate in the parent surface, or NULL to ignore
 * @param y coordinate in the parent surface, or NULL to ignore
 *
 * @since 1.8
 */
EAPI void ecore_wl_subsurf_position_get(Ecore_Wl_Subsurf *ess, int *x, int *y);

/**
 * @ingroup Ecore_Wl_Subsurf
 * @brief Places subsurface on layer above a reference surface.
 *
 * Moves the @p ess subsurface to just above the reference
 * @p surface, changing the z-order.  The reference @p surface must
 * be either a sibling or parent surface, else a protocol error will
 * be generated.
 *
 * @param ess the subsurface
 * @param surface the sibling reference surface
 *
 * @since 1.8
 */
EAPI void ecore_wl_subsurf_place_above(Ecore_Wl_Subsurf *ess, struct wl_surface *surface);

/**
 * @ingroup Ecore_Wl_Subsurf
 * @brief Places subsurface on layer below a reference surface.
 *
 * See ecore_wl_subsurf_place_above.
 *
 * @param ess the subsurface
 * @param surface the sibling reference surface
 *
 * @since 1.8
 */
EAPI void ecore_wl_subsurf_place_below(Ecore_Wl_Subsurf *ess, struct wl_surface *surface);

/**
 * @ingroup Ecore_Wl_Subsurf
 * @brief Enables or disables sub-surface synchronization.
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
 * @param ess the subsurface
 * @param val true to enable synchronization, false to desynchronize
 *
 * @since 1.8
 */
EAPI void ecore_wl_subsurf_sync_set(Ecore_Wl_Subsurf *ess, Eina_Bool val);

/**
 * @ingroup Ecore_Wl_Subsurf
 * @brief Sets an opaque region for the given subsurface.
 *
 * This is an optimization hint to the compositor to allow it avoid
 * redrawing content unnecessarily.  Note that marking transparent
 * content as opaque will cause repaint artifacts.
 *
 * Use a 0x0 region size to unset the opaque region.
 *
 * @param ess the subsurface
 * @param x coordinate in the parent surface
 * @param y coordinate in the parent surface
 * @param w width to set as opaque
 * @param h height to set as opaque
 *
 * @since 1.8
 */
EAPI void ecore_wl_subsurf_opaque_region_set(Ecore_Wl_Subsurf *ess, int x, int y, int w, int h);

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
