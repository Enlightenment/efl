#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_wl_private.h"
#include "Ecore_Wayland.h"

/* local function prototypes */
static void _ecore_wl_window_cb_configure(void *data, struct wl_shell_surface *shell_surface __UNUSED__, unsigned int timestamp, unsigned int edges, int w, int h);
static void _ecore_wl_window_cb_popup_done(void *data, struct wl_shell_surface *shell_surface __UNUSED__);
static void _ecore_wl_window_configure_send(Ecore_Wl_Window *win, int w, int h, unsigned int timestamp);

/* local variables */
static Eina_Hash *_windows = NULL;

/* wayland listeners */
static const struct wl_shell_surface_listener _ecore_wl_shell_surface_listener = 
{
   _ecore_wl_window_cb_configure,
   _ecore_wl_window_cb_popup_done
};

/* internal functions */
void 
_ecore_wl_window_init(void)
{
   if (!_windows) _windows = eina_hash_pointer_new(free);
}

void 
_ecore_wl_window_shutdown(void)
{
   eina_hash_free(_windows);
   _windows = NULL;
}

/**
 * @defgroup Ecore_Wl_Window_Group Wayland Library Init and Shutdown Functions
 * 
 * Functions that can be used to create a Wayland window.
 */

/**
 * Creates a new window
 * 
 * @param parent The parent window to use. If @p parent is @c 0, the root window 
 *               of the default display is used.
 * @param x      X Position
 * @param y      Y position
 * @param w      Width
 * @param h      Height
 * 
 * @return The new window
 * 
 * @ingroup Ecore_Wl_Window_Group
 * @since 1.2
 */
EAPI Ecore_Wl_Window *
ecore_wl_window_new(Ecore_Wl_Window *parent, int x, int y, int w, int h, int buffer_type)
{
   Ecore_Wl_Window *win;
   static int _win_id = 1;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(win = malloc(sizeof(Ecore_Wl_Window))))
     {
        ERR("Failed to allocate an Ecore Wayland Window");
        return NULL;
     }

   memset(win, 0, sizeof(Ecore_Wl_Window));

   win->display = _ecore_wl_disp;
   win->parent = parent;
   win->allocation.x = x;
   win->allocation.y = y;
   win->allocation.w = w;
   win->allocation.h = h;
   win->saved_allocation = win->allocation;
   win->transparent = EINA_FALSE;
   win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
   win->buffer_type = buffer_type;
   win->id = _win_id++;

   eina_hash_add(_windows, &win->id, win);
   return win;
}

/**
 * Deletes the given window
 * 
 * @param win The given window
 * 
 * @ingroup Ecore_Wl_Window_Group
 * @since 1.2
 */
EAPI void 
ecore_wl_window_free(Ecore_Wl_Window *win)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   eina_hash_del(_windows, &win->id, NULL);

   wl_list_for_each(input, &_ecore_wl_disp->inputs, link)
     {
        if ((input->pointer_focus) && (input->pointer_focus == win))
          input->pointer_focus = NULL;
        if ((input->keyboard_focus) && (input->keyboard_focus == win))
          input->keyboard_focus = NULL;
     }

   if (win->region.input) wl_region_destroy(win->region.input);
   if (win->region.opaque) wl_region_destroy(win->region.opaque);

   if (win->shell_surface) wl_shell_surface_destroy(win->shell_surface);
   win->shell_surface = NULL;

   if (win->surface) wl_surface_destroy(win->surface);
   win->surface = NULL;

//   free(win);
}

/**
 * Signals for Wayland to initiate a window move.
 * 
 * The position requested (@p x, @p y) is not honored by Wayland because 
 * Wayland does not allow specific window placement to be set.
 * 
 * @param win The window to move.
 * @param x   X Position
 * @param y   Y Position
 * 
 * @ingroup Ecore_Wl_Window_Group
 * @since 1.2
 */
EAPI void 
ecore_wl_window_move(Ecore_Wl_Window *win, int x, int y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->allocation.x = x;
   win->allocation.y = y;
   if (win->shell_surface)
     {
        Ecore_Wl_Input *input;

        if (!(input = win->keyboard_device))
          {
             if (win->parent)
               input = win->parent->keyboard_device;
          }

        wl_shell_surface_move(win->shell_surface, input->input_device,
                              input->timestamp);
     }
}

/**
 * Signals for Wayland to initiate a window resize.
 * 
 * The size requested (@p w, @p h) is not honored by Wayland because 
 * Wayland does not allow specific window sizes to be set.
 * 
 * @param win      The window to resize.
 * @param w        Width
 * @param h        Height
 * @param location The edge of the window from where the resize should start.
 * 
 * @ingroup Ecore_Wl_Window_Group
 * @since 1.2
 */
EAPI void 
ecore_wl_window_resize(Ecore_Wl_Window *win, int w, int h, int location)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->allocation.w = w;
   win->allocation.h = h;
   if (win->type != ECORE_WL_WINDOW_TYPE_FULLSCREEN)
     {
        win->region.input = 
          wl_compositor_create_region(_ecore_wl_disp->wl.compositor);
        wl_region_add(win->region.input, win->allocation.x, win->allocation.y, 
                      win->allocation.w, win->allocation.h);
     }
   if (!win->transparent)
     {
        win->region.opaque = 
          wl_compositor_create_region(_ecore_wl_disp->wl.compositor);
        wl_region_add(win->region.opaque, win->allocation.x, win->allocation.y, 
                      win->allocation.w, win->allocation.h);
     }

   if (win->shell_surface)
     {
        Ecore_Wl_Input *input;

        input = win->keyboard_device;
        wl_shell_surface_resize(win->shell_surface, input->input_device, 
                                input->timestamp, location);
     }
}

EAPI void 
ecore_wl_window_damage(Ecore_Wl_Window *win, int x, int y, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (win->surface) 
     wl_surface_damage(win->surface, x, y, w, h);
}

EAPI void 
ecore_wl_window_buffer_attach(Ecore_Wl_Window *win, struct wl_buffer *buffer, int x, int y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if ((win->surface) && (buffer))
     wl_surface_attach(win->surface, buffer, x, y);
   if (win->surface)
     {
        if (win->region.input)
          {
             wl_surface_set_input_region(win->surface, win->region.input);
             wl_region_destroy(win->region.input);
             win->region.input = NULL;
          }
        if (win->region.opaque)
          {
             wl_surface_set_opaque_region(win->surface, win->region.opaque);
             wl_region_destroy(win->region.opaque);
             win->region.opaque = NULL;
          }
        wl_surface_damage(win->surface, 0, 0, 
                          win->allocation.w, win->allocation.h);
     }
}

/**
 * Shows a window
 * 
 * Synonymous to "mapping" a window in Wayland System terminology.
 * 
 * @param win The window to show.
 * 
 * @ingroup Ecore_Wl_Window_Group
 * @since 1.2
 */
EAPI void 
ecore_wl_window_show(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (win->surface) return;

   win->surface = wl_compositor_create_surface(_ecore_wl_disp->wl.compositor);

   wl_surface_set_user_data(win->surface, win);

   win->shell_surface = 
     wl_shell_get_shell_surface(_ecore_wl_disp->wl.shell, win->surface);
   wl_shell_surface_add_listener(win->shell_surface, 
                                 &_ecore_wl_shell_surface_listener, win);

   switch (win->type)
     {
      case ECORE_WL_WINDOW_TYPE_FULLSCREEN:
        wl_shell_surface_set_fullscreen(win->shell_surface, 
                                        WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
                                        0, NULL);
        break;
      case ECORE_WL_WINDOW_TYPE_MAXIMIZED:
        wl_shell_surface_set_maximized(win->shell_surface, NULL);
        break;
      case ECORE_WL_WINDOW_TYPE_TRANSIENT:
        wl_shell_surface_set_transient(win->shell_surface, 
                                       win->parent->shell_surface, 
                                       win->allocation.x, win->allocation.y, 0);
        break;
      case ECORE_WL_WINDOW_TYPE_MENU:
        wl_shell_surface_set_popup(win->shell_surface, 
                                   win->parent->pointer_device->input_device, 
                                   win->parent->pointer_device->timestamp, 
                                   win->parent->shell_surface, 
                                   win->allocation.x, win->allocation.y, 0);
        break;
      case ECORE_WL_WINDOW_TYPE_TOPLEVEL:
      default:
        wl_shell_surface_set_toplevel(win->shell_surface);
        break;
     }

   if (win->type != ECORE_WL_WINDOW_TYPE_FULLSCREEN)
     {
        win->region.input = 
          wl_compositor_create_region(_ecore_wl_disp->wl.compositor);
        wl_region_add(win->region.input, win->allocation.x, win->allocation.y, 
                      win->allocation.w, win->allocation.h);
     }
   if (!win->transparent)
     {
        win->region.opaque = 
          wl_compositor_create_region(_ecore_wl_disp->wl.compositor);
        wl_region_add(win->region.opaque, win->allocation.x, win->allocation.y, 
                      win->allocation.w, win->allocation.h);
     }
}

/**
 * Hides a window
 * 
 * Synonymous to "unmapping" a window in Wayland System terminology.
 * 
 * @param win The window to hide.
 * 
 * @ingroup Ecore_Wl_Window_Group
 * @since 1.2
 */
EAPI void 
ecore_wl_window_hide(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (win->shell_surface) wl_shell_surface_destroy(win->shell_surface);
   win->shell_surface = NULL;
   if (win->surface) wl_surface_destroy(win->surface);
   win->surface = NULL;
}

/**
 * Raises a window
 * 
 * @param win The window to raise.
 * 
 * @ingroup Ecore_Wl_Window_Group
 * @since 1.2
 */
EAPI void 
ecore_wl_window_raise(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (win->shell_surface) 
     wl_shell_surface_set_toplevel(win->shell_surface);
}

EAPI void 
ecore_wl_window_maximized_set(Ecore_Wl_Window *win, Eina_Bool maximized)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if ((win->type == ECORE_WL_WINDOW_TYPE_MAXIMIZED) == maximized) return;
   if (win->type == ECORE_WL_WINDOW_TYPE_TOPLEVEL)
     {
        win->saved_allocation = win->allocation;
        if (win->shell_surface) 
          wl_shell_surface_set_maximized(win->shell_surface, NULL);
        win->type = ECORE_WL_WINDOW_TYPE_MAXIMIZED;
     }
   else
     {
        Ecore_Wl_Input *input;

        input = win->keyboard_device;

        if (win->shell_surface) 
          wl_shell_surface_set_toplevel(win->shell_surface);
        win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
        win->allocation = win->saved_allocation;
        _ecore_wl_window_configure_send(win, win->allocation.w, 
                                        win->allocation.h, input->timestamp);
     }
}

EAPI void 
ecore_wl_window_fullscreen_set(Ecore_Wl_Window *win, Eina_Bool fullscreen)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if ((win->type == ECORE_WL_WINDOW_TYPE_FULLSCREEN) == fullscreen) return;
   if (fullscreen)
     {
        win->type = ECORE_WL_WINDOW_TYPE_FULLSCREEN;
        win->saved_allocation = win->allocation;
        if (win->shell_surface)
          wl_shell_surface_set_fullscreen(win->shell_surface, 
                                          WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
                                          0, NULL);
     }
   else 
     {
        Ecore_Wl_Input *input;

        input = win->keyboard_device;

        if (win->shell_surface)
          wl_shell_surface_set_toplevel(win->shell_surface);
        win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
        win->allocation = win->saved_allocation;
        _ecore_wl_window_configure_send(win, win->allocation.w, 
                                        win->allocation.h, input->timestamp);
     }
}

EAPI void 
ecore_wl_window_transparent_set(Ecore_Wl_Window *win, Eina_Bool transparent)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->transparent = transparent;
}

EAPI void 
ecore_wl_window_update_size(Ecore_Wl_Window *win, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->allocation.w = w;
   win->allocation.h = h;
}

EAPI struct wl_surface *
ecore_wl_window_surface_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return NULL;
   return win->surface;
}

EAPI Ecore_Wl_Window *
ecore_wl_window_find(unsigned int id)
{
   Ecore_Wl_Window *win;

   if (!id) return NULL;
   win = eina_hash_find(_windows, &id);
   if (win) return win;
   return NULL;
}

EAPI void 
ecore_wl_window_type_set(Ecore_Wl_Window *win, Ecore_Wl_Window_Type type)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->type = type;
}

EAPI void 
ecore_wl_window_pointer_set(Ecore_Wl_Window *win, struct wl_buffer *buffer, int hot_x, int hot_y, unsigned int timestamp)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   input = _ecore_wl_disp->input;
   wl_input_device_attach(input->input_device, timestamp, 
                          buffer, hot_x, hot_y);
}

/* local functions */
static void 
_ecore_wl_window_cb_configure(void *data, struct wl_shell_surface *shell_surface __UNUSED__, unsigned int timestamp, unsigned int edges, int w, int h)
{
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(win = data)) return;
   if ((w <= 0) || (h <= 0)) return;

   win->edges = edges;
   win->allocation.w = w;
   win->allocation.h = h;
   if (win->region.input) wl_region_destroy(win->region.input);
   win->region.input = NULL;
   if (win->region.opaque) wl_region_destroy(win->region.opaque);
   win->region.opaque = NULL;
   _ecore_wl_window_configure_send(win, w, h, timestamp);
}

static void 
_ecore_wl_window_cb_popup_done(void *data, struct wl_shell_surface *shell_surface __UNUSED__)
{
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(win = data)) return;
   ecore_wl_input_ungrab(win->pointer_device, 0);
}

static void 
_ecore_wl_window_configure_send(Ecore_Wl_Window *win, int w, int h, unsigned int timestamp)
{
   Ecore_Wl_Event_Window_Configure *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Window_Configure)))) return;
   ev->win = win->id;
   ev->event_win = win->id;
   ev->x = win->allocation.x;
   ev->y = win->allocation.y;
   ev->w = w;
   ev->h = h;
   ev->timestamp = timestamp;
   ecore_event_add(ECORE_WL_EVENT_WINDOW_CONFIGURE, ev, NULL, NULL);
}
