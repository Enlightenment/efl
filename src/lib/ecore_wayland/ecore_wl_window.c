#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl_private.h"

/* local function prototypes */
static void _ecore_wl_window_cb_ping(void *data EINA_UNUSED, struct wl_shell_surface *shell_surface, unsigned int serial);
static void _ecore_wl_window_cb_configure(void *data, struct wl_shell_surface *shell_surface EINA_UNUSED, unsigned int edges, int w, int h);
static void _ecore_wl_window_cb_popup_done(void *data, struct wl_shell_surface *shell_surface EINA_UNUSED);
static void _ecore_wl_window_cb_surface_enter(void *data, struct wl_surface *surface, struct wl_output *output EINA_UNUSED);
static void _ecore_wl_window_cb_surface_leave(void *data, struct wl_surface *surface, struct wl_output *output EINA_UNUSED);
static void _ecore_wl_window_configure_send(Ecore_Wl_Window *win, int w, int h);
static char *_ecore_wl_window_id_str_get(unsigned int win_id);

/* local variables */
static Eina_Hash *_windows = NULL;

/* wayland listeners */
static const struct wl_surface_listener _ecore_wl_surface_listener = 
{
   _ecore_wl_window_cb_surface_enter,
   _ecore_wl_window_cb_surface_leave
};

static const struct wl_shell_surface_listener _ecore_wl_shell_surface_listener = 
{
   _ecore_wl_window_cb_ping,
   _ecore_wl_window_cb_configure,
   _ecore_wl_window_cb_popup_done
};

/* internal functions */
void 
_ecore_wl_window_init(void)
{
   if (!_windows) 
     _windows = eina_hash_string_superfast_new(NULL);
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
 * @param buffer_type The type of the buffer to be used to create a new Ecore_Wl_Window.
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
   /* win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL; */
   win->type = ECORE_WL_WINDOW_TYPE_NONE;
   win->buffer_type = buffer_type;
   win->id = _win_id++;

   eina_hash_add(_windows, _ecore_wl_window_id_str_get(win->id), win);
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

   eina_hash_del(_windows, _ecore_wl_window_id_str_get(win->id), win);

   wl_list_for_each(input, &_ecore_wl_disp->inputs, link)
     {
        if ((input->pointer_focus) && (input->pointer_focus == win))
          input->pointer_focus = NULL;
        if ((input->keyboard_focus) && (input->keyboard_focus == win))
          input->keyboard_focus = NULL;
     }

   if (win->region.input) wl_region_destroy(win->region.input);
   win->region.input = NULL;
   if (win->region.opaque) wl_region_destroy(win->region.opaque);
   win->region.opaque = NULL;
   if (win->shell_surface) wl_shell_surface_destroy(win->shell_surface);
   win->shell_surface = NULL;

   if (win->surface) wl_surface_destroy(win->surface);
   win->surface = NULL;

   /* HMMM, why was this disabled ? */
   free(win);
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
               {
                  if (!(input = win->parent->keyboard_device))
                    input = win->parent->pointer_device;
               }
          }

        if ((!input) || (!input->seat)) return;

        wl_shell_surface_move(win->shell_surface, input->seat,
                              input->display->serial);
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

   if (win->type != ECORE_WL_WINDOW_TYPE_FULLSCREEN)
     {
        win->allocation.w = w;
        win->allocation.h = h;

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

        if (!(input = win->keyboard_device))
          {
             if (win->parent)
               {
                  if (!(input = win->parent->keyboard_device))
                    input = win->parent->pointer_device;
               }
          }

        if ((!input) || (!input->seat)) return;

        wl_shell_surface_resize(win->shell_surface, input->seat, 
                                input->display->serial, location);
     }
}

EAPI void 
ecore_wl_window_damage(Ecore_Wl_Window *win, int x, int y, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (win->surface) 
     {
        wl_surface_damage(win->surface, x, y, w, h);
        wl_surface_commit(win->surface);
     }
}

EAPI void 
ecore_wl_window_buffer_attach(Ecore_Wl_Window *win, struct wl_buffer *buffer, int x, int y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   switch (win->buffer_type)
     {
      case ECORE_WL_WINDOW_BUFFER_TYPE_EGL_WINDOW:
        win->server_allocation = win->allocation;
        break;
      case ECORE_WL_WINDOW_BUFFER_TYPE_EGL_IMAGE:
      case ECORE_WL_WINDOW_BUFFER_TYPE_SHM:
        if (win->surface)
          {
             if (win->edges & 4) //  resizing from the left
               x = win->server_allocation.w - win->allocation.w;
             else
               x = 0;

             if (win->edges & 1) // resizing from the top
               y = win->server_allocation.h - win->allocation.h;
             else
               y = 0;

             win->edges = 0;

             /* if (buffer) */
             wl_surface_attach(win->surface, buffer, x, y);
             wl_surface_damage(win->surface, 0, 0, 
                               win->allocation.w, win->allocation.h);
             wl_surface_commit(win->surface);

             win->server_allocation = win->allocation;
          }
        break;
      default:
        return;
     }

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
   /* wl_surface_add_listener(win->surface, &_ecore_wl_surface_listener, win); */

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
                                       win->parent->surface, 
                                       win->allocation.x, win->allocation.y, 0);
        break;
      case ECORE_WL_WINDOW_TYPE_MENU:
        wl_shell_surface_set_popup(win->shell_surface, 
                                   _ecore_wl_disp->input->seat,
                                   _ecore_wl_disp->serial,
                                   win->parent->surface, 
                                   win->allocation.x, win->allocation.y, 0);
        break;
      case ECORE_WL_WINDOW_TYPE_NONE:
        win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
        /* fallthrough */
      case ECORE_WL_WINDOW_TYPE_TOPLEVEL:
        wl_shell_surface_set_toplevel(win->shell_surface);
        break;
      default:
        break;
     }

   /* if (win->type != ECORE_WL_WINDOW_TYPE_FULLSCREEN) */
   /*   { */
   /*      win->region.input =  */
   /*        wl_compositor_create_region(_ecore_wl_disp->wl.compositor); */
   /*      wl_region_add(win->region.input, win->allocation.x, win->allocation.y,  */
   /*                    win->allocation.w, win->allocation.h); */
   /*   } */

   /* if (!win->transparent) */
   /*   { */
   /*      win->region.opaque =  */
   /*        wl_compositor_create_region(_ecore_wl_disp->wl.compositor); */
   /*      wl_region_add(win->region.opaque, win->allocation.x, win->allocation.y,  */
   /*                    win->allocation.w, win->allocation.h); */
   /*   } */
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
   else if (win->type == ECORE_WL_WINDOW_TYPE_MAXIMIZED)
     {
        if (win->shell_surface) 
          wl_shell_surface_set_toplevel(win->shell_surface);
        win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
        _ecore_wl_window_configure_send(win, win->saved_allocation.w, 
                                        win->saved_allocation.h);
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
        if (win->shell_surface)
          wl_shell_surface_set_toplevel(win->shell_surface);
        win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
        _ecore_wl_window_configure_send(win, win->saved_allocation.w, 
                                        win->saved_allocation.h);
     }
}

EAPI void 
ecore_wl_window_transparent_set(Ecore_Wl_Window *win, Eina_Bool transparent)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->transparent = transparent;
   if (win->region.opaque) wl_region_destroy(win->region.opaque);
   win->region.opaque = NULL;
   if (!win->transparent)
     {
        win->region.opaque = 
          wl_compositor_create_region(_ecore_wl_disp->wl.compositor);
        wl_region_add(win->region.opaque, win->allocation.x, win->allocation.y, 
                      win->allocation.w, win->allocation.h);
     }
}

EAPI void 
ecore_wl_window_update_size(Ecore_Wl_Window *win, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->allocation.w = w;
   win->allocation.h = h;
}

EAPI void 
ecore_wl_window_update_location(Ecore_Wl_Window *win, int x, int y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->allocation.x = x;
   win->allocation.y = y;
}

EAPI struct wl_surface *
ecore_wl_window_surface_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return NULL;
   return win->surface;
}

/* @since 1.2 */
EAPI struct wl_shell_surface *
ecore_wl_window_shell_surface_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return NULL;
   return win->shell_surface;
}

EAPI Ecore_Wl_Window *
ecore_wl_window_find(unsigned int id)
{
   Ecore_Wl_Window *win = NULL;

   win = eina_hash_find(_windows, _ecore_wl_window_id_str_get(id));
   return win;
}

EAPI void 
ecore_wl_window_type_set(Ecore_Wl_Window *win, Ecore_Wl_Window_Type type)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->type = type;
}

EAPI void 
ecore_wl_window_pointer_set(Ecore_Wl_Window *win, struct wl_surface *surface, int hot_x, int hot_y)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   if ((input = win->pointer_device))
     ecore_wl_input_pointer_set(input, surface, hot_x, hot_y);
}

EAPI void
ecore_wl_window_cursor_from_name_set(Ecore_Wl_Window *win, const char *cursor_name)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   if ((input = win->pointer_device))
     ecore_wl_input_cursor_from_name_set(input, cursor_name);
}

EAPI void
ecore_wl_window_cursor_default_restore(Ecore_Wl_Window *win)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   if ((input = win->pointer_device))
     ecore_wl_input_cursor_default_restore(input);
}

/* @since 1.2 */
EAPI void 
ecore_wl_window_parent_set(Ecore_Wl_Window *win, Ecore_Wl_Window *parent)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   win->parent = parent;
}

/* local functions */
static void 
_ecore_wl_window_cb_ping(void *data EINA_UNUSED, struct wl_shell_surface *shell_surface, unsigned int serial)
{
   if (!shell_surface) return;
   wl_shell_surface_pong(shell_surface, serial);
}

static void 
_ecore_wl_window_cb_configure(void *data, struct wl_shell_surface *shell_surface EINA_UNUSED, unsigned int edges, int w, int h)
{
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(win = data)) return;

   if ((w <= 0) || (h <= 0)) return;

   if ((win->allocation.w != w) || (win->allocation.h != h))
     {
        win->edges = edges;
        if (win->region.input) wl_region_destroy(win->region.input);
        win->region.input = NULL;
        if (win->region.opaque) wl_region_destroy(win->region.opaque);
        win->region.opaque = NULL;

        _ecore_wl_window_configure_send(win, w, h);
     }
}

static void 
_ecore_wl_window_cb_popup_done(void *data, struct wl_shell_surface *shell_surface EINA_UNUSED)
{
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!shell_surface) return;
   if (!(win = data)) return;
   ecore_wl_input_ungrab(win->pointer_device);
}

static void 
_ecore_wl_window_cb_surface_enter(void *data, struct wl_surface *surface, struct wl_output *output EINA_UNUSED)
{
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!surface) return;
   if (!(win = data)) return;
}

static void 
_ecore_wl_window_cb_surface_leave(void *data, struct wl_surface *surface, struct wl_output *output EINA_UNUSED)
{
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!surface) return;
   if (!(win = data)) return;
}

static void 
_ecore_wl_window_configure_send(Ecore_Wl_Window *win, int w, int h)
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
   ecore_event_add(ECORE_WL_EVENT_WINDOW_CONFIGURE, ev, NULL, NULL);
}

static char *
_ecore_wl_window_id_str_get(unsigned int win_id)
{
   const char *vals = "qWeRtYuIoP5$&<~";
   static char id[9];
   unsigned int val;

   val = win_id;
   id[0] = vals[(val >> 28) & 0xf];
   id[1] = vals[(val >> 24) & 0xf];
   id[2] = vals[(val >> 20) & 0xf];
   id[3] = vals[(val >> 16) & 0xf];
   id[4] = vals[(val >> 12) & 0xf];
   id[5] = vals[(val >> 8) & 0xf];
   id[6] = vals[(val >> 4) & 0xf];
   id[7] = vals[(val) & 0xf];
   id[8] = 0;

   return id;
}
