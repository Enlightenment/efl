/**
 * @file Ecore_Cocoa.h
 *
 * @brief Ecore_Cocoa Wrapper Library to interact with the Cocoa
 * envirnement (Mac OS X) from the E environment
 *
 */
#ifndef __ECORE_COCOA_H__
#define __ECORE_COCOA_H__

/*
 * DO NOT USE THIS HEADER. IT IS WORK IN PROGRESS. IT IS NOT FINAL AND
 * THE API MAY CHANGE.
 */

#ifndef ECORE_COCOA_WIP_GNSIDNQI
# warning "You are using a work in progress API. This API is not stable"
# warning "and is subject to change. You use this at your own risk."
#endif

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
 */
typedef struct _Ecore_Cocoa_Window Ecore_Cocoa_Window;
#endif /* ! _ECORE_COCOA_WINDOW_PREDEF */

/**
 * @typedef Ecore_Cocoa_Screen
 * Opaque handler to manipulate a Cocoa Screen through Ecore
 */
typedef struct _Ecore_Cocoa_Screen Ecore_Cocoa_Screen;

/**
 * @typedef Ecore_Cocoa_Object
 * Opaque handler to refer to an objective-c object (aka id)
 */
typedef void Ecore_Cocoa_Object;

/**
 * @typedef Ecore_Cocoa_Event_Window_Resize_Request
 * Type of event thrown when a Cocoa window is resized
 */
typedef struct _Ecore_Cocoa_Event_Window_Resize_Request Ecore_Cocoa_Event_Window_Resize_Request;

/**
 * @typedef Ecore_Cocoa_Event_Window_Focused
 * Type of event thrown when a Cocoa window receives focus
 */
typedef struct _Ecore_Cocoa_Event_Window_Focused Ecore_Cocoa_Event_Window_Focused;

/**
 * @typedef Ecore_Cocoa_Event_Window_Unfocused
 * Type of event thrown when a Cocoa window loses the focus
 */
typedef struct _Ecore_Cocoa_Event_Window_Unfocused Ecore_Cocoa_Event_Window_Unfocused;

/**
 * @typedef Ecore_Cocoa_Event_Window_Destroy
 * Type of event thrown when a Cocoa window gets destoyed
 */
typedef struct _Ecore_Cocoa_Event_Window_Destroy Ecore_Cocoa_Event_Window_Destroy;

/**
 * @typedef Ecore_Cocoa_Cursor
 * Values of the Cocoa cursors handled by Ecore_Cocoa
 * See https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Classes/NSCursor_Class/index.html for images of each cursors.
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

   __ECORE_COCOA_CURSOR_LAST, /**< Sentinel. DO NOT USE */

   ECORE_COCOA_CURSOR_DEFAULT = ECORE_COCOA_CURSOR_ARROW /**< Default Cocoa cursor */

} Ecore_Cocoa_Cursor;


/** Event triggered when a Cocoa window receives focus */
EAPI extern int ECORE_COCOA_EVENT_WINDOW_FOCUSED;

/** Event triggered when a Cocoa window loses focus */
EAPI extern int ECORE_COCOA_EVENT_WINDOW_UNFOCUSED;

/** Event triggered when a Cocoa window is resized */
EAPI extern int ECORE_COCOA_EVENT_WINDOW_RESIZE_REQUEST;

/** Event triggered when a Cocoa window get destroyed */
EAPI extern int ECORE_COCOA_EVENT_WINDOW_DESTROY;

/**
 * @struct _Ecore_Cocoa_Event_Window_Resize_Request
 * Data available when a window is resized
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
 */
struct _Ecore_Cocoa_Event_Window_Focused
{
   Ecore_Cocoa_Object *cocoa_window; /**< Handler of the Cocoa window */
};

/**
 * @struct _Ecore_Cocoa_Event_Window_Unfocused
 * Data available when a Cocoa window losrs focus
 */
struct _Ecore_Cocoa_Event_Window_Unfocused
{
   Ecore_Cocoa_Object *cocoa_window; /**< Handler of the Cocoa window */
};

/**
 * @struct _Ecore_Cocoa_Event_Window_Destroy
 * Data available when a Cocoa window is destroyed
 */
struct _Ecore_Cocoa_Event_Window_Destroy
{
   Ecore_Cocoa_Object *cocoa_window; /**< Handler of the Cocoa window */
};

/**
 * @typedef Ecore_Cocoa_Cnp_Type
 * Type used to interact with the Cocoa pasteboard.
 * It hold types that can apply to a context.
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
 */
EAPI int ecore_cocoa_init(void);

/**
 * Shuts the Ecore_Cocoa library down
 * @return How many times Ecore_Cocoa has been initted
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
 */
EAPI Ecore_Cocoa_Window *ecore_cocoa_window_new(int x,
                                                int y,
                                                int w,
                                                int h);

/**
 * Releases a Cocoa window
 * @param window The window to be released
 */
EAPI void ecore_cocoa_window_free(Ecore_Cocoa_Window *window);

/**
 * Moves a Cocoa window at a given point
 * @param window The window to be moved
 * @param x The new origin of the window (X)
 * @param y The new origin of the window (Y)
 */
EAPI void ecore_cocoa_window_move(Ecore_Cocoa_Window *window,
                                  int                 x,
                                  int                 y);

/**
 * Resizes a Cocoa window to a given size
 * @param window The window to be moved
 * @param w The new width of the window
 * @param h The new height of the window
 */
EAPI void ecore_cocoa_window_resize(Ecore_Cocoa_Window *window,
                                    int                 w,
                                    int                 h);

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
 */
EAPI void ecore_cocoa_window_move_resize(Ecore_Cocoa_Window *window,
                                         int                 x,
                                         int                 y,
                                         int                 w,
                                         int                 h);

/**
 * Gets the geometry of a Cocoa window
 * @param window The window which geometry is queried
 * @param x Pointer used to retrieve its origin in X
 * @param y Pointer used to retrieve its origin in Y
 * @param w Pointer used to retrieve its width
 * @param h Pointer used to retrieve its height
 */
EAPI void ecore_cocoa_window_geometry_get(const Ecore_Cocoa_Window *window,
                                          int                      *x,
                                          int                      *y,
                                          int                      *w,
                                          int                      *h);

/**
 * Gets the size of a Cocoa window
 * @param window The window which size is queried
 * @param w Pointer used to retrieve its width
 * @param h Pointer used to retrieve its height
 */
EAPI void ecore_cocoa_window_size_get(const Ecore_Cocoa_Window *window,
                                      int                      *w,
                                      int                      *h);

/**
 * Sets the minimum size of a Cocoa window
 * @param window The window which minimum size is to be altered
 * @param w The new minimum width of the window
 * @param h The new minimum height of the window
 */
EAPI void ecore_cocoa_window_size_min_set(Ecore_Cocoa_Window *window,
                                          int                 w,
                                          int                 h);

/**
 * Gets the minimum size size of a Cocoa window
 * @param window The window which minimum size is queried
 * @param w Pointer used to retrieve its minimum width
 * @param h Pointer used to retrieve its minimum height
 */
EAPI void ecore_cocoa_window_size_min_get(const Ecore_Cocoa_Window *window,
                                          int                      *w,
                                          int                      *h);

/**
 * Sets the maximum size of a Cocoa window
 * @param window The window which maximum size is to be altered
 * @param w The new maximum width of the window
 * @param h The new maximum height of the window
 */
EAPI void ecore_cocoa_window_size_max_set(Ecore_Cocoa_Window *window,
                                          int                 w,
                                          int                 h);

/**
 * Gets the maximum size size of a Cocoa window
 * @param window The window which maximum size is queried
 * @param w Pointer used to retrieve its maximum width
 * @param h Pointer used to retrieve its maximum height
 */
EAPI void ecore_cocoa_window_size_max_get(const Ecore_Cocoa_Window *window,
                                          int                      *w,
                                          int                      *h);

EAPI void ecore_cocoa_window_size_step_set(Ecore_Cocoa_Window *window,
                                           int                 w,
                                           int                 h);

EAPI void ecore_cocoa_window_size_step_get(const Ecore_Cocoa_Window *window,
                                           int                      *w,
                                           int                      *h);

/**
 * Display a Cocoa window
 * @param window The Cocoa window to be displayed
 */
EAPI void ecore_cocoa_window_show(Ecore_Cocoa_Window *window);

/**
 * Hide a Cocoa window
 * @param window The Cocoa window to be hid
 */
EAPI void ecore_cocoa_window_hide(Ecore_Cocoa_Window *window);

/**
 * Brings a Cocoa window to front
 * @param window The Cocoa window to be raised
 */
EAPI void ecore_cocoa_window_raise(Ecore_Cocoa_Window *window);

/**
 * Brings a Cocoa window back
 * @param window The Cocoa window to be lowered
 */
EAPI void ecore_cocoa_window_lower(Ecore_Cocoa_Window *window);

/**
 * Makes a Cocoa window the current key window by raising it
 * @param window The Cocoa window to be activated
 */
EAPI void ecore_cocoa_window_activate(Ecore_Cocoa_Window *window);

/**
 * Changes the title of a Cocoa window
 * @param window The Cocoa window which title is to be changed
 * @param title The new title of the Cocoa window
 */
EAPI void ecore_cocoa_window_title_set(Ecore_Cocoa_Window *window,
                                       const char         *title);

/**
 * Miniaturize or deminiaturize a Cocoa window
 * @param window The Cocoa window which iconify status is to be changed
 * @param on If #EINA_TRUE, will miniaturize the window. Will deminiaturize it if #EINA_FALSE
 */
EAPI void ecore_cocoa_window_iconified_set(Ecore_Cocoa_Window *window,
                                           Eina_Bool           on);

/**
 * Manage the borders of a Cocoa window
 * @param window The Cocoa window which borders are to be changed
 * @param on If #EINA_TRUE, will remove borders. Will restore them if #EINA_FALSE
 */
EAPI void ecore_cocoa_window_borderless_set(Ecore_Cocoa_Window *window,
                                            Eina_Bool           on);

/**
 * Set the content view of a Cocoa window
 * @param window The Cocoa window which internal view is to be set
 * @param view The NSView to be set as @c window content view
 */
EAPI void ecore_cocoa_window_view_set(Ecore_Cocoa_Window *window,
                                      Ecore_Cocoa_Object *view);

/**
 * Get the height of the title bar of Cocoa windows
 * @return The height of the title bar of Cocoa windows
 */
EAPI int ecore_cocoa_titlebar_height_get(void);

/**
 * Retrieves the actual NSWindow behind the Ecore_Cocoa wrapper
 * @param window The Ecore_Cocoa wrapper which window is to be retrieved
 * @return The Cocoa NSWindow manipulated by @c window
 */
EAPI Ecore_Cocoa_Object *ecore_cocoa_window_get(const Ecore_Cocoa_Window *window);

/**
 * Set the clipboard of Cocoa (NSPasteboard)
 * @param data The contents to be set in the clipboard
 * @param size The size in bytes of @c data
 * @param type
 * @return EINA_TRUE on success, EINA_FALSE on failure
 */
EAPI Eina_Bool ecore_cocoa_selection_clipboard_set(const void *data,
                                                   int size,
                                                   Ecore_Cocoa_Cnp_Type type);

/**
 * Get the contents of the Cocoa clipboard
 * @param size Pointer used to retrieve the size of the received contents
 * @param type The type of object to retrieve from the clipboard
 * @param retrieved_types The types of objects retrieved from the clipboard
 * @return The data retrieved from the clipboard. NULL on failure
 *
 * If @c type was ECORE_COCOA_CNP_TYPE_STRING or ECORE_COCOA_CNP_TYPE_MARKUP,
 * @c retrieved_types will contain ECORE_COCOA_CNP_TYPE_STRING and the data
 * will be a C string (char*) that must be freed after use.
 */
EAPI void *ecore_cocoa_selection_clipboard_get(int *size,
                                               Ecore_Cocoa_Cnp_Type type,
                                               Ecore_Cocoa_Cnp_Type *retrieved_types);

/**
 * Deletes the contents of the Cocoa clipboard
 */
EAPI void ecore_cocoa_selection_clipboard_clear(void);

/**
 * Set the Cocoa cursor for a given Cocoa window
 * @param win The Cocoa window on which the cursor is to be changed.
 * @param c The cursor to be set
 */
EAPI void ecore_cocoa_window_cursor_set(Ecore_Cocoa_Window *win, Ecore_Cocoa_Cursor c);

/**
 * Hide or show the Cocoa cursor for a given Cocoa window
 * @param win The Cocoa window on which the cursor is to be hid
 * @param show Shows the cursor if EINA_TRUE. Hides it if EINA_FALSE
 *
 */
EAPI void ecore_cocoa_window_cursor_show(Ecore_Cocoa_Window *win, Eina_Bool show);

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
