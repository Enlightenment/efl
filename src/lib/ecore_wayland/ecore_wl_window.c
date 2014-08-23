#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl_private.h"
#include "xdg-shell-client-protocol.h"

/* local function prototypes */
static void _ecore_wl_window_cb_ping(void *data EINA_UNUSED, struct wl_shell_surface *shell_surface, unsigned int serial);
static void _ecore_wl_window_cb_configure(void *data, struct wl_shell_surface *shell_surface EINA_UNUSED, unsigned int edges, int w, int h);
static void _ecore_wl_window_cb_popup_done(void *data, struct wl_shell_surface *shell_surface EINA_UNUSED);
static void _ecore_wl_window_cb_surface_enter(void *data, struct wl_surface *surface, struct wl_output *output EINA_UNUSED);
static void _ecore_wl_window_cb_surface_leave(void *data, struct wl_surface *surface, struct wl_output *output EINA_UNUSED);
static void _ecore_wl_window_configure_send(Ecore_Wl_Window *win, int w, int h, int edges);
static char *_ecore_wl_window_id_str_get(unsigned int win_id);
static void _ecore_xdg_handle_surface_configure(void *data, struct xdg_surface *xdg_surface, int32_t width, int32_t height,struct wl_array *states, uint32_t serial);
static void _ecore_xdg_handle_popup_done(void *data, struct xdg_popup *xdg_popup, unsigned int serial);

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

static const struct xdg_surface_listener _ecore_xdg_surface_listener = 
{
   _ecore_xdg_handle_surface_configure,
};

static const struct xdg_popup_listener _ecore_xdg_popup_listener = 
{
   _ecore_xdg_handle_popup_done,
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

Eina_Hash *
_ecore_wl_window_hash_get(void)
{
   return _windows;
}

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
   win->saved.w = w;
   win->saved.h = h;
   win->transparent = EINA_FALSE;
   win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
   win->buffer_type = buffer_type;
   win->id = _win_id++;
   win->rotation = 0;

   win->opaque.x = x;
   win->opaque.y = y;
   win->opaque.w = w;
   win->opaque.h = h;

   win->title = NULL;
   win->class_name = NULL;

   eina_hash_add(_windows, _ecore_wl_window_id_str_get(win->id), win);
   return win;
}

EAPI void 
ecore_wl_window_free(Ecore_Wl_Window *win)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   eina_hash_del(_windows, _ecore_wl_window_id_str_get(win->id), win);

   EINA_INLIST_FOREACH(_ecore_wl_disp->inputs, input)
     {
        if ((input->pointer_focus) && (input->pointer_focus == win))
          input->pointer_focus = NULL;
        if ((input->keyboard_focus) && (input->keyboard_focus == win))
          input->keyboard_focus = NULL;
     }

   if (win->anim_callback) wl_callback_destroy(win->anim_callback);
   win->anim_callback = NULL;

   if (win->subsurfs) _ecore_wl_subsurfs_del_all(win);

#ifdef USE_IVI_SHELL
   if (win->ivi_surface) ivi_surface_destroy(win->ivi_surface);
   win->ivi_surface = NULL;
#endif
   if (win->xdg_surface) xdg_surface_destroy(win->xdg_surface);
   win->xdg_surface = NULL;
   if (win->xdg_popup) xdg_popup_destroy(win->xdg_popup);
   win->xdg_popup = NULL;

   if (win->shell_surface) wl_shell_surface_destroy(win->shell_surface);
   win->shell_surface = NULL;
   if (win->surface) wl_surface_destroy(win->surface);
   win->surface = NULL;

   if (win->title) eina_stringshare_del(win->title);
   if (win->class_name) eina_stringshare_del(win->class_name);

   /* HMMM, why was this disabled ? */
   free(win);
}

EAPI void 
ecore_wl_window_move(Ecore_Wl_Window *win, int x, int y)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   input = win->keyboard_device;
   ecore_wl_window_update_location(win, x, y);

   if ((!input) && (win->parent))
     {
        input = win->parent->keyboard_device;
        if (!(input = win->parent->keyboard_device))
          input = win->parent->pointer_device;
     }

   if ((!input) || (!input->seat)) return;

   _ecore_wl_input_grab_release(input, win);

   if (win->xdg_surface)
     xdg_surface_move(win->xdg_surface, input->seat, input->display->serial);
   else if (win->shell_surface)
     wl_shell_surface_move(win->shell_surface, input->seat,
                           input->display->serial);
}

EAPI void 
ecore_wl_window_resize(Ecore_Wl_Window *win, int w, int h, int location)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   input = win->keyboard_device;

   ecore_wl_window_update_size(win, w, h);

   if ((!input) && (win->parent))
     {
        if (!(input = win->parent->keyboard_device))
          input = win->parent->pointer_device;
     }

   if ((!input) || (!input->seat)) return;

   _ecore_wl_input_grab_release(input, win);

   if (win->xdg_surface)
     xdg_surface_resize(win->xdg_surface, input->seat,
                        input->display->serial, location);
   else if (win->shell_surface)
     wl_shell_surface_resize(win->shell_surface, input->seat,
                             input->display->serial, location);
}

EAPI void 
ecore_wl_window_damage(Ecore_Wl_Window *win, int x, int y, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (win->surface) wl_surface_damage(win->surface, x, y, w, h);
}

EAPI void 
ecore_wl_window_commit(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if ((win->surface) && (win->has_buffer)) 
     wl_surface_commit(win->surface);
}

EAPI void 
ecore_wl_window_buffer_attach(Ecore_Wl_Window *win, struct wl_buffer *buffer, int x, int y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   switch (win->buffer_type)
     {
      case ECORE_WL_WINDOW_BUFFER_TYPE_EGL_WINDOW:
        break;
      case ECORE_WL_WINDOW_BUFFER_TYPE_EGL_IMAGE:
      case ECORE_WL_WINDOW_BUFFER_TYPE_SHM:
        if (win->surface)
          {
             win->has_buffer = (buffer != NULL);

             /* if (buffer) */
             wl_surface_attach(win->surface, buffer, x, y);
             wl_surface_damage(win->surface, 0, 0, 
                               win->allocation.w, win->allocation.h);
             wl_surface_commit(win->surface);
          }
        break;
      default:
        return;
     }
}

EAPI struct wl_surface*
ecore_wl_window_surface_create(Ecore_Wl_Window *win)
{
   if (!win) return NULL;
   if (win->surface) return win->surface;
   win->surface = wl_compositor_create_surface(_ecore_wl_compositor_get());
   win->surface_id = wl_proxy_get_id((struct wl_proxy *)win->surface);
   return win->surface;
}

EAPI void 
ecore_wl_window_show(Ecore_Wl_Window *win)
{
#ifdef USE_IVI_SHELL
   char *env;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   ecore_wl_window_surface_create(win);

   if ((win->type != ECORE_WL_WINDOW_TYPE_DND) &&
       (win->type != ECORE_WL_WINDOW_TYPE_NONE))
     {
#ifdef USE_IVI_SHELL
        if ((!win->ivi_surface) && (_ecore_wl_disp->wl.ivi_application))
          {
             if (win->parent && win->parent->ivi_surface)
               win->ivi_surface_id = win->parent->ivi_surface_id + 1;
             else if ((env = getenv("ECORE_IVI_SURFACE_ID")))
               win->ivi_surface_id = atoi(env);
             else
               win->ivi_surface_id = IVI_SURFACE_ID + getpid();

             win->ivi_surface =
               ivi_application_surface_create(_ecore_wl_disp->wl.ivi_application,
                                              win->ivi_surface_id, win->surface);
          }

        if (!win->ivi_surface) 
          {
#endif
             if ((!win->shell_surface) && (_ecore_wl_disp->wl.shell))
               if ((!win->xdg_surface) && (_ecore_wl_disp->wl.xdg_shell))
                 {
                    win->xdg_surface =
                      xdg_shell_get_xdg_surface(_ecore_wl_disp->wl.xdg_shell,
                                                win->surface);
                    if (!win->xdg_surface) return;
                    xdg_surface_set_user_data(win->xdg_surface, win);
                    xdg_surface_add_listener(win->xdg_surface,
                                             &_ecore_xdg_surface_listener, win);
                 }
             else if ((!win->shell_surface) && (_ecore_wl_disp->wl.shell))
               {
                  win->shell_surface = 
                    wl_shell_get_shell_surface(_ecore_wl_disp->wl.shell,
                                               win->surface);
                  if (!win->shell_surface) return;

                  if (win->title)
                    wl_shell_surface_set_title(win->shell_surface, win->title);

                  if (win->class_name)
                    wl_shell_surface_set_class(win->shell_surface, win->class_name);
               }

             if (win->shell_surface)
               wl_shell_surface_add_listener(win->shell_surface, 
                                             &_ecore_wl_shell_surface_listener, win);
#ifdef USE_IVI_SHELL
          }
#endif
             win->xdg_surface =
               xdg_shell_get_xdg_surface(_ecore_wl_disp->wl.xdg_shell,
                                         win->surface);
             if (!win->xdg_surface) return;
             xdg_surface_set_user_data(win->xdg_surface, win);
             xdg_surface_add_listener(win->xdg_surface,
                                      &_ecore_xdg_surface_listener, win);
          }
        else if ((!win->shell_surface) && (_ecore_wl_disp->wl.shell))
          {
             win->shell_surface = 
               wl_shell_get_shell_surface(_ecore_wl_disp->wl.shell,
                                          win->surface);
             if (!win->shell_surface) return;

             wl_shell_surface_add_listener(win->shell_surface,
                                           &_ecore_wl_shell_surface_listener,
                                           win);

             if (win->title)
               wl_shell_surface_set_title(win->shell_surface, win->title);

             if (win->class_name)
               wl_shell_surface_set_class(win->shell_surface, win->class_name);
           }
     }

   /* trap for valid shell surface */
   if ((!win->xdg_surface) && (!win->shell_surface)) return;

   switch (win->type)
     {
      case ECORE_WL_WINDOW_TYPE_FULLSCREEN:
        if (win->xdg_surface)
          xdg_surface_set_fullscreen(win->xdg_surface, NULL);
        else if (win->shell_surface)
          wl_shell_surface_set_fullscreen(win->shell_surface,
                                          WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
                                          0, NULL);
        break;
      case ECORE_WL_WINDOW_TYPE_MAXIMIZED:
        if (win->xdg_surface)
          xdg_surface_set_maximized(win->xdg_surface);
        else if (win->shell_surface)
          wl_shell_surface_set_maximized(win->shell_surface, NULL);
        break;
      case ECORE_WL_WINDOW_TYPE_TRANSIENT:
        if (win->xdg_surface)
          xdg_surface_set_parent(win->xdg_surface, win->parent->surface);
        else if (win->shell_surface)
          wl_shell_surface_set_transient(win->shell_surface,
                                         win->parent->surface,
                                         win->allocation.x,
                                         win->allocation.y, 0);
        break;
      case ECORE_WL_WINDOW_TYPE_MENU:
        if (win->xdg_surface)
          {
             win->xdg_popup = 
               xdg_shell_get_xdg_popup(_ecore_wl_disp->wl.xdg_shell,
                                       win->surface, 
                                       win->parent->surface,
                                       _ecore_wl_disp->input->seat,
                                       _ecore_wl_disp->serial,
                                       win->allocation.x,
                                       win->allocation.y, 0);
             xdg_popup_set_user_data(win->xdg_popup, win);
             xdg_popup_add_listener(win->xdg_popup, 
                                    &_ecore_xdg_popup_listener, win);
          }
        else if (win->shell_surface)
          wl_shell_surface_set_popup(win->shell_surface,
                                     _ecore_wl_disp->input->seat,
                                     _ecore_wl_disp->serial,
                                     win->parent->surface,
                                     win->allocation.x, win->allocation.y, 0);
        break;
      case ECORE_WL_WINDOW_TYPE_TOPLEVEL:
        if (win->xdg_surface)
          xdg_surface_set_parent(win->xdg_surface, NULL);
        else if (win->shell_surface)
          wl_shell_surface_set_toplevel(win->shell_surface);
        break;
      default:
        break;
     }
}

EAPI void 
ecore_wl_window_hide(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   if (win->xdg_surface) xdg_surface_destroy(win->xdg_surface);
   win->xdg_surface = NULL;

   if (win->xdg_popup) xdg_popup_destroy(win->xdg_popup);
   win->xdg_popup = NULL;

   if (win->shell_surface) wl_shell_surface_destroy(win->shell_surface);
   win->shell_surface = NULL;

   if (win->surface) wl_surface_destroy(win->surface);
   win->surface = NULL;
}

EAPI void 
ecore_wl_window_raise(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   /* FIXME: This should raise the xdg surface also */
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
        win->saved.w = win->allocation.w;
        win->saved.h = win->allocation.h;

        if (win->xdg_surface)
          {
             xdg_surface_set_maximized(win->xdg_surface);
             win->type = ECORE_WL_WINDOW_TYPE_MAXIMIZED;
          }
        else if (win->shell_surface)
          {
             wl_shell_surface_set_maximized(win->shell_surface, NULL);
             win->type = ECORE_WL_WINDOW_TYPE_MAXIMIZED;
          }
     }
   else if (win->type == ECORE_WL_WINDOW_TYPE_MAXIMIZED)
     {
        if (win->xdg_surface)
          {
             xdg_surface_unset_maximized(win->xdg_surface);
             win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
             _ecore_wl_window_configure_send(win, win->saved.w, win->saved.h, 0);
          }
        else if (win->shell_surface)
          {
             wl_shell_surface_set_toplevel(win->shell_surface);
             win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
             _ecore_wl_window_configure_send(win, win->saved.w, win->saved.h, 0);
          }
     }
}

EAPI Eina_Bool
ecore_wl_window_maximized_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return EINA_FALSE;

   if (win->type == ECORE_WL_WINDOW_TYPE_MAXIMIZED)
     return EINA_TRUE;

   return EINA_FALSE;
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
        win->saved.w = win->allocation.w;
        win->saved.h = win->allocation.h;

	if (win->xdg_surface)
          xdg_surface_set_fullscreen(win->xdg_surface, NULL);

        if (win->shell_surface)
          wl_shell_surface_set_fullscreen(win->shell_surface, 
                                          WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
                                          0, NULL);
     }
   else 
     {
	if (win->xdg_surface)
          xdg_surface_unset_fullscreen(win->xdg_surface);
        else if (win->shell_surface)
          wl_shell_surface_set_toplevel(win->shell_surface);

        win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
        _ecore_wl_window_configure_send(win, win->saved.w, win->saved.h, 0);
     }
}

EAPI Eina_Bool
ecore_wl_window_fullscreen_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return EINA_FALSE;

   if (win->type == ECORE_WL_WINDOW_TYPE_FULLSCREEN)
     return EINA_TRUE;

   return EINA_FALSE;
}

EAPI void 
ecore_wl_window_transparent_set(Ecore_Wl_Window *win, Eina_Bool transparent)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->transparent = transparent;
   if (!win->transparent)
     ecore_wl_window_opaque_region_set(win, win->opaque.x, win->opaque.y, 
                                       win->opaque.w, win->opaque.h);
   else
     ecore_wl_window_opaque_region_set(win, win->opaque.x, win->opaque.y, 0, 0);
}

EAPI Eina_Bool
ecore_wl_window_alpha_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return EINA_FALSE;

   return win->alpha;
}

EAPI void
ecore_wl_window_alpha_set(Ecore_Wl_Window *win, Eina_Bool alpha)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->alpha = alpha;
   if (!win->alpha)
     ecore_wl_window_opaque_region_set(win, win->opaque.x, win->opaque.y, 
                                       win->opaque.w, win->opaque.h);
   else
     ecore_wl_window_opaque_region_set(win, win->opaque.x, win->opaque.y, 0, 0);
}

EAPI Eina_Bool
ecore_wl_window_transparent_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return EINA_FALSE;

   return win->transparent;
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

/* @since 1.11 */
EAPI struct xdg_surface *
ecore_wl_window_xdg_surface_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return NULL;
   return win->xdg_surface;
}

EAPI Ecore_Wl_Window *
ecore_wl_window_find(unsigned int id)
{
   Ecore_Wl_Window *win = NULL;

   if (!_windows) return NULL;
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

   win->pointer.surface = surface;
   win->pointer.hot_x = hot_x;
   win->pointer.hot_y = hot_y;
   win->pointer.set = EINA_TRUE;

   if ((input = win->pointer_device))
     ecore_wl_input_pointer_set(input, surface, hot_x, hot_y);
}

EAPI void
ecore_wl_window_cursor_from_name_set(Ecore_Wl_Window *win, const char *cursor_name)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   win->pointer.set = EINA_FALSE;

   if ((input = win->pointer_device))
     ecore_wl_input_cursor_from_name_set(input, cursor_name);
}

EAPI void
ecore_wl_window_cursor_default_restore(Ecore_Wl_Window *win)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   win->pointer.set = EINA_FALSE;

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

/* @since 1.12 */
EAPI void 
ecore_wl_window_iconified_set(Ecore_Wl_Window *win, Eina_Bool iconified)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   if (iconified)
     {
        if (win->xdg_surface)
          {
             xdg_surface_set_minimized(win->xdg_surface);
             win->minimized = iconified;
          }
        else if (win->shell_surface)
          {
             /* TODO: handle case of iconifying a wl_shell surface */
          }
     }
   else
     {
        if (win->xdg_surface)
          {
             /* TODO: Handle case of UnIconifying an xdg_surface
              * 
              * NB: This will be needed for Enlightenment IBox scenario */
          }
        else if (win->shell_surface)
          {
             wl_shell_surface_set_toplevel(win->shell_surface);
             win->type = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
             _ecore_wl_window_configure_send(win, win->saved.w, win->saved.h, 0);
          }
     }
}

EAPI Eina_Bool 
ecore_wl_window_iconified_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return EINA_FALSE;
   return win->minimized;
}

EAPI Ecore_Wl_Window *
ecore_wl_window_surface_find(struct wl_surface *surface)
{
   Eina_Iterator *itr;
   Ecore_Wl_Window *win = NULL;
   void *data;

   itr = eina_hash_iterator_data_new(_windows);
   while (eina_iterator_next(itr, &data))
     {
        if (((Ecore_Wl_Window *)data)->surface == surface)
          {
             win = data;
             break;
          }
     }

   eina_iterator_free(itr);

   return win;
}

/* @since 1.8 */
EAPI void 
ecore_wl_window_input_region_set(Ecore_Wl_Window *win, int x, int y, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   win->input.x = x;
   win->input.y = y;
   if ((w > 0) && (h > 0))
     {
        if ((win->input.w == w) && (win->input.h == h))
          return;

        win->input.w = w;
        win->input.h = h;
     }

   if ((win->type != ECORE_WL_WINDOW_TYPE_FULLSCREEN) || 
       (win->type != ECORE_WL_WINDOW_TYPE_DND))
     {
        if ((w > 0) && (h > 0))
          {
             struct wl_region *region = NULL;

             region = 
               wl_compositor_create_region(_ecore_wl_compositor_get());
             wl_region_add(region, x, y, w, h);
             wl_surface_set_input_region(win->surface, region);
             wl_region_destroy(region);
          }
        else
          wl_surface_set_input_region(win->surface, NULL);
     }

   /* ecore_wl_window_commit(win); */
}

/* @since 1.8 */
EAPI void 
ecore_wl_window_opaque_region_set(Ecore_Wl_Window *win, int x, int y, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   win->opaque.x = x;
   win->opaque.y = y;
   if ((w > 0) && (h > 0))
     {
        if ((win->opaque.w == w) && (win->opaque.h == h))
          return;

        win->opaque.w = w;
        win->opaque.h = h;
     }

   if (((w > 0) && (h > 0)) && ((!win->transparent) && (!win->alpha)))
     {
        struct wl_region *region = NULL;

        region = 
          wl_compositor_create_region(_ecore_wl_compositor_get());

        switch (win->rotation)
          {
           case 0:
             wl_region_add(region, x, y, w, h);
             break;
           case 180:
             wl_region_add(region, x, x + y, w, h);
             break;
           case 90:
             wl_region_add(region, y, x, h, w);
             break;
           case 270:
             wl_region_add(region, x + y, x, h, w);
             break;
          }

        wl_surface_set_opaque_region(win->surface, region);
        wl_region_destroy(region);
     }
   else
     wl_surface_set_opaque_region(win->surface, NULL);

   /* ecore_wl_window_commit(win); */
}

/* @since 1.8 */
EAPI void 
ecore_wl_window_rotation_set(Ecore_Wl_Window *win, int rotation)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   win->rotation = rotation;
}

/* @since 1.8 */
EAPI int 
ecore_wl_window_rotation_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return 0;
   return win->rotation;
}

/* @since 1.8 */
EAPI int
ecore_wl_window_id_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return 0;
   return win->id;
}

/* @since 1.8 */
EAPI int
ecore_wl_window_surface_id_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return 0;
   return win->surface_id;
}

/* @since 1.8 */
EAPI void
ecore_wl_window_title_set(Ecore_Wl_Window *win, const char *title)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   eina_stringshare_replace(&win->title, title);

   if ((win->xdg_surface) && (win->title))
     xdg_surface_set_title(win->xdg_surface, win->title);
   else if ((win->shell_surface) && (win->title))
     wl_shell_surface_set_title(win->shell_surface, win->title);
}

/* @since 1.8 */
EAPI void
ecore_wl_window_class_name_set(Ecore_Wl_Window *win, const char *class_name)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   eina_stringshare_replace(&win->class_name, class_name);

   if ((win->xdg_surface) && (win->class_name))
     xdg_surface_set_app_id(win->xdg_surface, win->class_name);
   else if ((win->shell_surface) && (win->class_name))
     wl_shell_surface_set_class(win->shell_surface, win->class_name);
}

/* @since 1.8 */
/* Maybe we need an ecore_wl_window_pointer_get() too */
EAPI Ecore_Wl_Input *
ecore_wl_window_keyboard_get(Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return 0;
   return win->keyboard_device;
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
     _ecore_wl_window_configure_send(win, w, h, edges);
}

static void
_ecore_xdg_handle_surface_configure(void *data, struct xdg_surface *xdg_surface EINA_UNUSED, int32_t width, int32_t height, struct wl_array *states, uint32_t serial)
{
   Ecore_Wl_Window *win = data;
   uint32_t *p;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   win->maximized = EINA_FALSE;
   win->fullscreen = EINA_FALSE;
   win->resizing = EINA_FALSE;
   win->focused = EINA_FALSE;

   wl_array_for_each(p, states)
     {
        uint32_t state = *p;
        switch (state)
          {
           case XDG_SURFACE_STATE_MAXIMIZED:
             win->maximized = EINA_TRUE;
             break;
           case XDG_SURFACE_STATE_FULLSCREEN:
             win->fullscreen = EINA_TRUE;
             break;
           case XDG_SURFACE_STATE_RESIZING:
             win->resizing = EINA_TRUE;
             break;
           case XDG_SURFACE_STATE_ACTIVATED:
             win->focused = EINA_TRUE;
             win->minimized = EINA_FALSE;
             break;
           default:
             break;
          }
     }
   if ((width > 0) && (height > 0))
     _ecore_wl_window_configure_send(win, width, height, 0);
   else
     {
        if ((win->saved.w != 1) || (win->saved.h != 1))
          _ecore_wl_window_configure_send(win, win->saved.w, win->saved.h, 0);
     }

   xdg_surface_ack_configure(win->xdg_surface, serial);
}

static void 
_ecore_wl_window_cb_popup_done(void *data, struct wl_shell_surface *shell_surface)
{
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!shell_surface) return;
   if (!(win = data)) return;
   ecore_wl_input_ungrab(win->pointer_device);
}

static void
_ecore_xdg_handle_popup_done(void *data, struct xdg_popup *xdg_popup, unsigned int serial EINA_UNUSED)
{
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!xdg_popup) return;
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
_ecore_wl_window_configure_send(Ecore_Wl_Window *win, int w, int h, int edges)
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
   ev->edges = edges;
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
