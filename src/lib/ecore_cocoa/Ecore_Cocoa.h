/**
 * @file Ecore_Cocoa.h
 *
 * @brief Ecore_Cocoa Wrapper Library to interact with the Cocoa
 * environment (Mac OS X) from the E environment
 *
 */
#ifndef __ECORE_COCOA_H__
#define __ECORE_COCOA_H__

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#else
# define EAPI
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ECORE_COCOA_WINDOW_PREDEF
/**
 * @typedef Ecore_Cocoa_Window
 * Opaque handler to manipulate a Cocoa Window through Ecore
 * @since 1.18
 */
typedef struct _Ecore_Cocoa_Window Ecore_Cocoa_Window;
#endif /* ! _ECORE_COCOA_WINDOW_PREDEF */

/**
 * @typedef Ecore_Cocoa_Screen
 * Opaque handler to manipulate a Cocoa Screen through Ecore
 * @since 1.18
 */
typedef struct _Ecore_Cocoa_Screen Ecore_Cocoa_Screen;

/**
 * @typedef Ecore_Cocoa_Object
 * Opaque handler to refer to an objective-c object (a.k.a. id)
 * @since 1.18
 */
typedef void Ecore_Cocoa_Object;

/**
 * @typedef Ecore_Cocoa_Event_Window_Resize_Request
 * Type of event thrown when a Cocoa window is resized
 * @since 1.18
 */
typedef struct _Ecore_Cocoa_Event_Window_Resize_Request Ecore_Cocoa_Event_Window_Resize_Request;

/**
 * @typedef Ecore_Cocoa_Event_Window_Focused
 * Type of event thrown when a Cocoa window receives focus
 * @since 1.18
 */
typedef struct _Ecore_Cocoa_Event_Window_Focused Ecore_Cocoa_Event_Window_Focused;

/**
 * @typedef Ecore_Cocoa_Event_Window_Unfocused
 * Type of event thrown when a Cocoa window loses the focus
 * @since 1.18
 */
typedef struct _Ecore_Cocoa_Event_Window_Unfocused Ecore_Cocoa_Event_Window_Unfocused;

/**
 * @typedef Ecore_Cocoa_Event_Window_Destroy
 * Type of event thrown when a Cocoa window gets destoyed
 * @since 1.18
 */
typedef struct _Ecore_Cocoa_Event_Window_Destroy Ecore_Cocoa_Event_Window_Destroy;

/**
 * @typedef Ecore_Cocoa_Terminate_Cb
 * Callback called when a delete request is sent to the application.
 * Such function must return EINA_TRUE for the application to terminate,
 * and EINA_FALSE to ignore application termination.
 * It is typically called by a CMD+Q signal.
 * @param sender The NSApplication instance to be terminated
 * @since 1.19
 */
typedef Eina_Bool (*Ecore_Cocoa_Terminate_Cb)(Ecore_Cocoa_Object *sender);

/**
 * @typedef Ecore_Cocoa_Cursor
 * Values of the Cocoa cursors handled by Ecore_Cocoa
 * See https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Classes/NSCursor_Class/index.html for images of each cursors.
 * @since 1.18
 */
typedef enum
{
   ECORE_COCOA_CURSOR_ARROW = 0, /**< Arrow cursor */
   ECORE_COCOA_CURSOR_CONTEXTUAL_MENU, /**< Contextual menu cursor */
   ECORE_COCOA_CURSOR_CLOSED_HAND, /**< Closed hand cursor */
   ECORE_COCOA_CURSOR_CROSSHAIR, /**< Crosshair cursor */
   ECORE_COCOA_CURSOR_DISAPPEARING_ITEM, /**< Disappearing item cursor */
   ECORE_COCOA_CURSOR_DRAG_COPY, /** Drag copy cursor */
   ECORE_COCOA_CURSOR_DRAG_LINK, /**< Drag link cursor */
   ECORE_COCOA_CURSOR_IBEAM, /**< IBeam cursor */
   ECORE_COCOA_CURSOR_OPEN_HAND, /**< Open hand cursor */
   ECORE_COCOA_CURSOR_OPERATION_NOT_ALLOWED, /**< Operation not allowed cursor */
   ECORE_COCOA_CURSOR_POINTING_HAND, /**< Pointing hand cursor */
   ECORE_COCOA_CURSOR_RESIZE_DOWN, /**< Resize down cursor */
   ECORE_COCOA_CURSOR_RESIZE_LEFT, /**< Resize left cursor */
   ECORE_COCOA_CURSOR_RESIZE_LEFT_RIGHT, /**< Resize left right cursor */
   ECORE_COCOA_CURSOR_RESIZE_RIGHT, /**< Resize right cursor */
   ECORE_COCOA_CURSOR_RESIZE_UP, /**< Resize up cursor */
   ECORE_COCOA_CURSOR_RESIZE_UP_DOWN, /**< Resize up down cursor */
   ECORE_COCOA_CURSOR_IBEAM_VERTICAL, /**< IBeam vertical cursor */

   __ECORE_COCOA_CURSOR_LAST /**< Sentinel. DO NOT USE */
} Ecore_Cocoa_Cursor;


/**
 * @def ECORE_COCOA_CURSOR_DEFAULT
 * Defines the default Cocoa cursor, to be used when nothing
 * is specified or as a fallback.
 * @since 1.18
 */
#define ECORE_COCOA_CURSOR_DEFAULT ECORE_COCOA_CURSOR_ARROW


/**
 * Event triggered when a Cocoa window receives focus
 * @since 1.18
 */
EAPI extern int ECORE_COCOA_EVENT_WINDOW_FOCUSED;

/**
 * Event triggered when a Cocoa window loses focus
 * @since 1.18
 */
EAPI extern int ECORE_COCOA_EVENT_WINDOW_UNFOCUSED;

/**
 * Event triggered when a Cocoa window is resized
 * @since 1.18
 */
EAPI extern int ECORE_COCOA_EVENT_WINDOW_RESIZE_REQUEST;

/** Event triggered when a Cocoa window get destroyed
 * @since 1.18
 */
EAPI extern int ECORE_COCOA_EVENT_WINDOW_DESTROY;

/**
 * @struct _Ecore_Cocoa_Event_Window_Resize_Request
 * Data available when a window is resized
 * @since 1.18
 */
struct _Ecore_Cocoa_Event_Window_Resize_Request
{
   int             w; /**< Current width of the window */
   int             h; /**< Current height of the window */
   Ecore_Cocoa_Object *cocoa_window; /**< Handler of the Cocoa window */
};

/**
 * @struct _Ecore_Cocoa_Event_Window_Focused
 * Data available when a Cocoa window receives focus
 * @since 1.18
 */
struct _Ecore_Cocoa_Event_Window_Focused
{
   Ecore_Cocoa_Object *cocoa_window; /**< Handler of the Cocoa window */
};

/**
 * @struct _Ecore_Cocoa_Event_Window_Unfocused
 * Data available when a Cocoa window losrs focus
 * @since 1.18
 */
struct _Ecore_Cocoa_Event_Window_Unfocused
{
   Ecore_Cocoa_Object *cocoa_window; /**< Handler of the Cocoa window */
};

/**
 * @struct _Ecore_Cocoa_Event_Window_Destroy
 * Data available when a Cocoa window is destroyed
 * @since 1.18
 */
struct _Ecore_Cocoa_Event_Window_Destroy
{
   Ecore_Cocoa_Object *cocoa_window; /**< Handler of the Cocoa window */
};

/**
 * @typedef Ecore_Cocoa_Cnp_Type
 * Type used to interact with the Cocoa pasteboard.
 * It holds types that can apply to a context.
 * @since 1.18
 */
typedef enum
{
   ECORE_COCOA_CNP_TYPE_UNKNOWN = 0, /**< Undefined type */
   ECORE_COCOA_CNP_TYPE_STRING  = (1 << 0), /**< String type (pure text) */
   ECORE_COCOA_CNP_TYPE_MARKUP  = (1 << 1), /**< Elementary markup */
   ECORE_COCOA_CNP_TYPE_IMAGE   = (1 << 2), /**< Image (all formats) */
   ECORE_COCOA_CNP_TYPE_HTML    = (1 << 3) /**< HTML */
} Ecore_Cocoa_Cnp_Type;


/*============================================================================*
 *                                    Core                                    *
 *============================================================================*/

/**
 * Inits the Ecore_Cocoa library
 * @return How many times Ecore_Cocoa has been initted
 * @since 1.18
 */
EAPI int ecore_cocoa_init(void);

/**
 * Shuts the Ecore_Cocoa library down
 * @return How many times Ecore_Cocoa has been shut down
 * @since 1.18
 */
EAPI int ecore_cocoa_shutdown(void);


/*============================================================================*
 *                                   Screen                                   *
 *============================================================================*/

/**
 * Retrieves the size of a Cocoa screen
 * @param screen The screen which size must be retrieved
 * @param [out] w The width of the screen
 * @param [out] h The height of the screen
 * @since 1.18
 */
EAPI void ecore_cocoa_screen_size_get(Ecore_Cocoa_Screen *screen, int *w, int *h);


/*============================================================================*
 *                                   Window                                   *
 *============================================================================*/

/**
 * Creates a Cocoa window
 * @param x The origin (X) where the window must be created
 * @param y The origin (Y) where the window must be created
 * @param w The width of the window
 * @param h The height of the window
 * @return A handler on the window. NULL on failure
 * @since 1.18
 */
EAPI Ecore_Cocoa_Window *ecore_cocoa_window_new(int x,
                                                int y,
                                                int w,
                                                int h)
   EINA_MALLOC
   EINA_WARN_UNUSED_RESULT;

/**
 * Releases a Cocoa window
 * @param window The window to be released
 * @since 1.18
 */
EAPI void ecore_cocoa_window_free(Ecore_Cocoa_Window *window)
   EINA_ARG_NONNULL(1);

/**
 * Moves a Cocoa window at a given point
 * @param window The window to be moved
 * @param x The new origin of the window (X)
 * @param y The new origin of the window (Y)
 * @since 1.18
 */
EAPI void ecore_cocoa_window_move(Ecore_Cocoa_Window *window,
                                  int                 x,
                                  int                 y)
   EINA_ARG_NONNULL(1);

/**
 * Resizes a Cocoa window to a given size
 * @param window The window to be moved
 * @param w The new width of the window
 * @param h The new height of the window
 * @since 1.18
 */
EAPI void ecore_cocoa_window_resize(Ecore_Cocoa_Window *window,
                                    int                 w,
                                    int                 h)
   EINA_ARG_NONNULL(1);

/**
 * Moves and resizes a Cocoa window to a given point and size
 * @param window The window to be moved
 * @param x The new origin of the window (X)
 * @param y The new origin of the window (Y)
 * @param w The new width of the window
 * @param h The new height of the window
 *
 * @see ecore_cocoa_window_resize()
 * @see ecore_cocoa_window_move()
 * @since 1.18
 */
EAPI void ecore_cocoa_window_move_resize(Ecore_Cocoa_Window *window,
                                         int                 x,
                                         int                 y,
                                         int                 w,
                                         int                 h)
   EINA_ARG_NONNULL(1);

/**
 * Gets the geometry of a Cocoa window
 * @param window The window which geometry is queried
 * @param x Pointer used to retrieve its origin in X
 * @param y Pointer used to retrieve its origin in Y
 * @param w Pointer used to retrieve its width
 * @param h Pointer used to retrieve its height
 * @since 1.18
 */
EAPI void ecore_cocoa_window_geometry_get(const Ecore_Cocoa_Window *window,
                                          int                      *x,
                                          int                      *y,
                                          int                      *w,
                                          int                      *h)
   EINA_ARG_NONNULL(1);

/**
 * Gets the size of a Cocoa window
 * @param window The window which size is queried
 * @param w Pointer used to retrieve its width
 * @param h Pointer used to retrieve its height
 * @since 1.18
 */
EAPI void ecore_cocoa_window_size_get(const Ecore_Cocoa_Window *window,
                                      int                      *w,
                                      int                      *h)
   EINA_ARG_NONNULL(1);

/**
 * Sets the minimum size of a Cocoa window
 * @param window The window which minimum size is to be altered
 * @param w The new minimum width of the window
 * @param h The new minimum height of the window
 * @since 1.18
 */
EAPI void ecore_cocoa_window_size_min_set(Ecore_Cocoa_Window *window,
                                          int                 w,
                                          int                 h)
   EINA_ARG_NONNULL(1);

/**
 * Gets the minimum size size of a Cocoa window
 * @param window The window which minimum size is queried
 * @param w Pointer used to retrieve its minimum width
 * @param h Pointer used to retrieve its minimum height
 * @since 1.18
 */
EAPI void ecore_cocoa_window_size_min_get(const Ecore_Cocoa_Window *window,
                                          int                      *w,
                                          int                      *h)
   EINA_ARG_NONNULL(1);

/**
 * Sets the maximum size of a Cocoa window
 * @param window The window which maximum size is to be altered
 * @param w The new maximum width of the window
 * @param h The new maximum height of the window
 * @since 1.18
 */
EAPI void ecore_cocoa_window_size_max_set(Ecore_Cocoa_Window *window,
                                          int                 w,
                                          int                 h)
   EINA_ARG_NONNULL(1);

/**
 * Gets the maximum size size of a Cocoa window
 * @param window The window which maximum size is queried
 * @param w Pointer used to retrieve its maximum width
 * @param h Pointer used to retrieve its maximum height
 * @since 1.18
 */
EAPI void ecore_cocoa_window_size_max_get(const Ecore_Cocoa_Window *window,
                                          int                      *w,
                                          int                      *h)
   EINA_ARG_NONNULL(1);

/**
 * Sets a Cocoa window's resize increment
 * @param window The Cocoa window which resize increment is to be set
 * @param w The width size increment
 * @param h The height size increment
 * @since 1.18
 */
EAPI void ecore_cocoa_window_size_step_set(Ecore_Cocoa_Window *window,
                                           int                 w,
                                           int                 h)
   EINA_ARG_NONNULL(1);

/**
 * Gets a Cocoa window's resize increment
 * @param window The Cocoa window which resize increment queried
 * @param w The width size increment
 * @param h The height size increment
 * @since 1.18
 */
EAPI void ecore_cocoa_window_size_step_get(const Ecore_Cocoa_Window *window,
                                           int                      *w,
                                           int                      *h)
   EINA_ARG_NONNULL(1);

/**
 * Displays a Cocoa window
 * @param window The Cocoa window to be displayed
 * @since 1.18
 */
EAPI void ecore_cocoa_window_show(Ecore_Cocoa_Window *window)
   EINA_ARG_NONNULL(1);

/**
 * Hides a Cocoa window
 * @param window The Cocoa window to be hid
 * @since 1.18
 */
EAPI void ecore_cocoa_window_hide(Ecore_Cocoa_Window *window)
   EINA_ARG_NONNULL(1);

/**
 * Brings a Cocoa window to front
 * @param window The Cocoa window to be raised
 * @since 1.18
 */
EAPI void ecore_cocoa_window_raise(Ecore_Cocoa_Window *window)
   EINA_ARG_NONNULL(1);

/**
 * Brings a Cocoa window back
 * @param window The Cocoa window to be lowered
 * @since 1.18
 */
EAPI void ecore_cocoa_window_lower(Ecore_Cocoa_Window *window)
   EINA_ARG_NONNULL(1);

/**
 * Makes a Cocoa window the current key window by raising it
 * @param window The Cocoa window to be activated
 * @since 1.18
 */
EAPI void ecore_cocoa_window_activate(Ecore_Cocoa_Window *window)
   EINA_ARG_NONNULL(1);

/**
 * Changes the title of a Cocoa window
 * @param window The Cocoa window which title is to be changed
 * @param title The new title of the Cocoa window
 * @since 1.18
 */
EAPI void ecore_cocoa_window_title_set(Ecore_Cocoa_Window *window,
                                       const char         *title)
   EINA_ARG_NONNULL(1);

/**
 * Miniaturizes or deminiaturizes a Cocoa window
 * @param window The Cocoa window which iconify status is to be changed
 * @param on If #EINA_TRUE, will miniaturize the window. Will deminiaturize it if #EINA_FALSE
 * @since 1.18
 */
EAPI void ecore_cocoa_window_iconified_set(Ecore_Cocoa_Window *window,
                                           Eina_Bool           on)
   EINA_ARG_NONNULL(1);

/**
 * Manages the borders of a Cocoa window
 * @param window The Cocoa window which borders are to be changed
 * @param on If #EINA_TRUE, will remove borders. Will restore them if #EINA_FALSE
 * @since 1.18
 */
EAPI void ecore_cocoa_window_borderless_set(Ecore_Cocoa_Window *window,
                                            Eina_Bool           on)
   EINA_ARG_NONNULL(1);

/**
 * Sets the content view of a Cocoa window
 *
 * Refer to Apple's documentation of the property 'contentView' of
 * the NSWindow class for more details about the content view.
 *
 * @param window The Cocoa window which internal view is to be set
 * @param view The NSView to be set as @c window content view
 * @since 1.18
 */
EAPI void ecore_cocoa_window_view_set(Ecore_Cocoa_Window *window,
                                      Ecore_Cocoa_Object *view)
   EINA_ARG_NONNULL(1)
   EINA_ARG_NONNULL(2);

/**
 * Gets the height of the title bar of Cocoa windows
 * @return The height of the title bar of Cocoa windows
 * @since 1.18
 */
EAPI int ecore_cocoa_titlebar_height_get(void);

/**
 * Retrieves the actual NSWindow behind the Ecore_Cocoa wrapper
 * @param window The Ecore_Cocoa wrapper which window is to be retrieved
 * @return The Cocoa NSWindow manipulated by @c window
 * @since 1.18
 */
EAPI Ecore_Cocoa_Object *ecore_cocoa_window_get(const Ecore_Cocoa_Window *window)
   EINA_ARG_NONNULL(1)
   EINA_WARN_UNUSED_RESULT;


/**
 * Sets the Cocoa cursor for a given Cocoa window
 * @param win The Cocoa window on which the cursor is to be changed.
 * @param c The cursor to be set
 * @since 1.18
 */
EAPI void ecore_cocoa_window_cursor_set(Ecore_Cocoa_Window *win,
                                        Ecore_Cocoa_Cursor  c)
   EINA_ARG_NONNULL(1);

/**
 * Hides or shows the Cocoa cursor for a given Cocoa window
 * @param win The Cocoa window on which the cursor is to be hid
 * @param show Shows the cursor if EINA_TRUE. Hides it if EINA_FALSE
 * @since 1.18
 */
EAPI void ecore_cocoa_window_cursor_show(Ecore_Cocoa_Window *win, Eina_Bool show);
   EINA_ARG_NONNULL(1);

/**
 * Overrides the default behaviour in response to an application delete
 * request. When an application receives a delete request (i.e. CMD+Q)
 * the termination callback will be called and its result will determine
 * whether the application should terminate or not
 * @param cb The custom termination callback to set
 * @see Ecore_Cocoa_Terminate_Cb
 * @since 1.19
 */
EAPI void ecore_cocoa_terminate_cb_set(Ecore_Cocoa_Terminate_Cb cb)
   EINA_ARG_NONNULL(1);


/*
 * The clipboard API is still BETA
 */

#ifdef EFL_BETA_API_SUPPORT

/*
 * Sets the clipboard of Cocoa (NSPasteboard)
 * @param data The contents to be set in the clipboard
 * @param size The size in bytes of @c data
 * @param type
 * @return EINA_TRUE on success, EINA_FALSE on failure
 */
EAPI Eina_Bool ecore_cocoa_clipboard_set(const void *data,
                                                   int size,
                                                   Ecore_Cocoa_Cnp_Type type);

/*
 * Gets the contents of the Cocoa clipboard
 * @param size Pointer used to retrieve the size of the received contents
 * @param type The type of object to retrieve from the clipboard
 * @param retrieved_types The types of objects retrieved from the clipboard
 * @return The data retrieved from the clipboard. NULL on failure
 *
 * If @c type was ECORE_COCOA_CNP_TYPE_STRING or ECORE_COCOA_CNP_TYPE_MARKUP,
 * @c retrieved_types will contain ECORE_COCOA_CNP_TYPE_STRING and the data
 * will be a C string (char*) that must be freed after use.
 */
EAPI void *ecore_cocoa_clipboard_get(int *size,
                                     Ecore_Cocoa_Cnp_Type type,
                                     Ecore_Cocoa_Cnp_Type *retrieved_types)
   EINA_WARN_UNUSED_RESULT;

/*
 * Deletes the contents of the Cocoa clipboard
 */
EAPI void ecore_cocoa_clipboard_clear(void);

#endif /* EFL_BETA_API_SUPPORT */

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
