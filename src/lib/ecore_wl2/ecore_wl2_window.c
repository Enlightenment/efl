#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

static void _anim_cb_animate(void *data, struct wl_callback *callback, uint32_t serial EINA_UNUSED);

static Eina_Bool _animator_busy = EINA_FALSE;

static void
_ecore_wl2_window_configure_send(Ecore_Wl2_Window *window, int w, int h, unsigned int edges)
{
   Ecore_Wl2_Event_Window_Configure *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Configure));
   if (!ev) return;

   ev->win = window->id;
   ev->event_win = window->id;
   ev->x = window->geometry.x;
   ev->y = window->geometry.y;
   ev->w = w;
   ev->h = h;
   ev->edges = edges;

   ecore_event_add(ECORE_WL2_EVENT_WINDOW_CONFIGURE, ev, NULL, NULL);
}

static void
_wl_shell_surface_cb_ping(void *data EINA_UNUSED, struct wl_shell_surface *shell_surface, unsigned int serial)
{
   wl_shell_surface_pong(shell_surface, serial);
}

static void
_wl_shell_surface_cb_configure(void *data, struct wl_shell_surface *shell_surface EINA_UNUSED, unsigned int edges, int w, int h)
{
   Ecore_Wl2_Window *win;

   win = data;
   if (!win) return;

   if ((w <= 0) || (h <= 0)) return;
   if ((win->geometry.w != w) || (win->geometry.h != h))
     _ecore_wl2_window_configure_send(win, w, h, edges);
}

static void
_wl_shell_surface_cb_popup_done(void *data EINA_UNUSED, struct wl_shell_surface *shell_surface EINA_UNUSED)
{
   Ecore_Wl2_Window *win;

   win = data;
   if (!win) return;

   _ecore_wl2_input_ungrab(win->input);
}

static const struct wl_shell_surface_listener _wl_shell_surface_listener =
{
   _wl_shell_surface_cb_ping,
   _wl_shell_surface_cb_configure,
   _wl_shell_surface_cb_popup_done
};

static void
_xdg_popup_cb_done(void *data, struct xdg_popup *xdg_popup EINA_UNUSED)
{
   Ecore_Wl2_Window *win;

   win = data;
   if (!win) return;

   _ecore_wl2_input_ungrab(win->input);
}

static const struct xdg_popup_listener _xdg_popup_listener =
{
   _xdg_popup_cb_done,
};

static void
_xdg_surface_cb_configure(void *data, struct xdg_surface *xdg_surface EINA_UNUSED, int32_t w, int32_t h, struct wl_array *states, uint32_t serial)
{
   Ecore_Wl2_Window *win;
   uint32_t *s;

   win = data;
   if (!win) return;

   win->minimized = EINA_FALSE;
   win->maximized = EINA_FALSE;
   win->fullscreen = EINA_FALSE;
   win->focused = EINA_FALSE;
   win->resizing = EINA_FALSE;

   wl_array_for_each(s, states)
     {
        switch (*s)
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
           default:
             break;
          }
     }

   if ((w > 0) && (h > 0))
     _ecore_wl2_window_configure_send(win, w, h, 0);

   xdg_surface_ack_configure(win->xdg_surface, serial);
}

static void
_xdg_surface_cb_delete(void *data, struct xdg_surface *xdg_surface EINA_UNUSED)
{
   Ecore_Wl2_Window *win;

   win = data;
   if (!win) return;

   ecore_wl2_window_free(win);
}

static const struct xdg_surface_listener _xdg_surface_listener =
{
   _xdg_surface_cb_configure,
   _xdg_surface_cb_delete,
};

static void
_ecore_wl2_window_type_set(Ecore_Wl2_Window *win)
{
   switch (win->type)
     {
      case ECORE_WL2_WINDOW_TYPE_FULLSCREEN:
        if (win->xdg_surface)
          xdg_surface_set_fullscreen(win->xdg_surface, NULL);
        else if (win->wl_shell_surface)
          wl_shell_surface_set_fullscreen(win->wl_shell_surface,
                                          WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
                                          0, NULL);
        break;
      case ECORE_WL2_WINDOW_TYPE_MAXIMIZED:
        if (win->xdg_surface)
          xdg_surface_set_maximized(win->xdg_surface);
        else if (win->wl_shell_surface)
          wl_shell_surface_set_maximized(win->wl_shell_surface, NULL);
        break;
      case ECORE_WL2_WINDOW_TYPE_TRANSIENT:
        if (win->xdg_surface)
          xdg_surface_set_parent(win->xdg_surface, win->parent->xdg_surface);
        else if (win->wl_shell_surface)
          wl_shell_surface_set_transient(win->wl_shell_surface,
                                         win->parent->surface,
                                         win->geometry.x, win->geometry.y, 0);
        break;
      case ECORE_WL2_WINDOW_TYPE_MENU:
          {
             Ecore_Wl2_Input *input;

             input = win->input;
             if ((!input) && (win->parent))
               {
                  input = win->parent->input;
               }

             if ((!input) || (!input->wl.seat)) return;

             if (win->xdg_surface)
               {
                  win->xdg_popup =
                    xdg_shell_get_xdg_popup(win->display->wl.xdg_shell,
                                            win->surface, win->parent->surface,
                                            input->wl.seat,
                                            win->display->serial,
                                            win->geometry.x, win->geometry.y);
                  if (!win->xdg_popup)
                    {
                       ERR("Could not create xdg popup: %m");
                       return;
                    }

                  xdg_popup_set_user_data(win->xdg_popup, win);
                  xdg_popup_add_listener(win->xdg_popup,
                                         &_xdg_popup_listener, win);
               }
             else if (win->wl_shell_surface)
               {
                  wl_shell_surface_set_popup(win->wl_shell_surface,
                                             input->wl.seat,
                                             win->display->serial,
                                             win->parent->surface,
                                             win->geometry.x,
                                             win->geometry.y, 0);
               }
          }
        break;
      case ECORE_WL2_WINDOW_TYPE_TOPLEVEL:
        if (win->xdg_surface)
          xdg_surface_set_parent(win->xdg_surface, NULL);
        else if (win->wl_shell_surface)
          wl_shell_surface_set_toplevel(win->wl_shell_surface);
        break;
      default:
        break;
     }
}

static const struct wl_callback_listener _anim_listener =
{
   _anim_cb_animate
};

static void
_anim_cb_animate(void *data, struct wl_callback *callback, uint32_t serial EINA_UNUSED)
{
   Ecore_Wl2_Window *window;

   window = data;
   if (!window) return;

   ecore_animator_custom_tick();

   wl_callback_destroy(callback);
   window->anim_cb = NULL;

   if (_animator_busy)
     {
        window->anim_cb = wl_surface_frame(window->surface);
        wl_callback_add_listener(window->anim_cb, &_anim_listener, window);
        wl_surface_commit(window->surface);
     }
}

void
_ecore_wl2_window_animator_add(Ecore_Wl2_Window *window)
{
   _animator_busy = EINA_TRUE;

   if ((!window->surface) || (window->anim_cb)) return;

   window->anim_cb = wl_surface_frame(window->surface);
   wl_callback_add_listener(window->anim_cb, &_anim_listener, window);
   wl_surface_commit(window->surface);
}

void
_ecore_wl2_window_animator_end(void)
{
   _animator_busy = EINA_FALSE;
}

EAPI Ecore_Wl2_Window *
ecore_wl2_window_new(Ecore_Wl2_Display *display, Ecore_Wl2_Window *parent, int x, int y, int w, int h)
{
   Ecore_Wl2_Window *win;
   static int _win_id = 1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);

   /* try to allocate space for window structure */
   win = calloc(1, sizeof(Ecore_Wl2_Window));
   if (!win) return NULL;

   win->display = display;
   win->parent = parent;
   win->id = _win_id++;

   win->geometry.x = x;
   win->geometry.y = y;
   win->geometry.w = w;
   win->geometry.h = h;

   win->opaque.x = x;
   win->opaque.y = y;
   win->opaque.w = w;
   win->opaque.h = h;

   win->type = ECORE_WL2_WINDOW_TYPE_TOPLEVEL;

   display->windows =
     eina_inlist_append(display->windows, EINA_INLIST_GET(win));

   return win;
}

EAPI int
ecore_wl2_window_id_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, -1);
   return window->id;
}

EAPI struct wl_surface *
ecore_wl2_window_surface_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, NULL);

   if (!window->surface)
     {
        window->surface =
          wl_compositor_create_surface(window->display->wl.compositor);

        window->surface_id =
          wl_proxy_get_id((struct wl_proxy *)window->surface);
     }

   return window->surface;
}

EAPI int
ecore_wl2_window_surface_id_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, -1);
   return window->surface_id;
}

EAPI void
ecore_wl2_window_show(Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Display *disp;

   EINA_SAFETY_ON_NULL_RETURN(window);

   disp = window->display;

   if (!window->surface)
     {
        window->surface =
          wl_compositor_create_surface(window->display->wl.compositor);
     }

   if ((disp->wl.xdg_shell) && (!window->xdg_surface))
     {
        window->xdg_surface =
          xdg_shell_get_xdg_surface(disp->wl.xdg_shell, window->surface);
        if (!window->xdg_surface) goto surf_err;

        if (window->title)
          xdg_surface_set_title(window->xdg_surface, window->title);
        if (window->class)
          xdg_surface_set_app_id(window->xdg_surface, window->class);

        xdg_surface_set_user_data(window->xdg_surface, window);
        xdg_surface_add_listener(window->xdg_surface,
                                 &_xdg_surface_listener, window);
     }
   else if ((disp->wl.wl_shell) && (!window->wl_shell_surface))
     {
        window->wl_shell_surface =
          wl_shell_get_shell_surface(disp->wl.wl_shell, window->surface);
        if (!window->wl_shell_surface) goto surf_err;

        if (window->title)
          wl_shell_surface_set_title(window->wl_shell_surface, window->title);
        if (window->class)
          wl_shell_surface_set_class(window->wl_shell_surface, window->class);

        wl_shell_surface_add_listener(window->wl_shell_surface,
                                      &_wl_shell_surface_listener, window);
     }

   _ecore_wl2_window_type_set(window);

   return;

surf_err:
   ERR("Failed to create surface for window: %m");
}

EAPI void
ecore_wl2_window_hide(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (window->xdg_surface) xdg_surface_destroy(window->xdg_surface);
   window->xdg_surface = NULL;

   if (window->xdg_popup) xdg_popup_destroy(window->xdg_popup);
   window->xdg_popup = NULL;

   if (window->wl_shell_surface)
     wl_shell_surface_destroy(window->wl_shell_surface);
   window->wl_shell_surface = NULL;

   if (window->surface) wl_surface_destroy(window->surface);
   window->surface = NULL;
}

EAPI void
ecore_wl2_window_free(Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Display *display;
   Ecore_Wl2_Input *input;

   EINA_SAFETY_ON_NULL_RETURN(window);

   display = window->display;

   EINA_INLIST_FOREACH(display->inputs, input)
     {
        if ((input->focus.pointer) &&
            (input->focus.pointer == window))
          input->focus.pointer = NULL;
        if ((input->focus.keyboard) &&
            (input->focus.keyboard == window))
          {
             input->focus.keyboard = NULL;
             ecore_timer_del(input->repeat.timer);
             input->repeat.timer = NULL;
          }
     }

   if (window->anim_cb) wl_callback_destroy(window->anim_cb);

   /* TODO: destroy subsurfaces */

   ecore_wl2_window_hide(window);

   if (window->title) eina_stringshare_del(window->title);
   if (window->class) eina_stringshare_del(window->class);

   display->windows =
     eina_inlist_remove(display->windows, EINA_INLIST_GET(window));

   free(window);
}

EAPI void
ecore_wl2_window_move(Ecore_Wl2_Window *window, int x, int y)
{
   Ecore_Wl2_Input *input;

   EINA_SAFETY_ON_NULL_RETURN(window);

   /* test for no-op move */
   /* if ((window->geometry.x == x) && (window->geometry.y == y)) */
   /*   return; */

   window->geometry.x = x;
   window->geometry.y = y;

   input = window->input;
   if ((!input) && (window->parent))
     {
        input = window->parent->input;
     }

   if ((!input) || (!input->wl.seat)) return;

   _ecore_wl2_input_ungrab(input);

   if (window->xdg_surface)
     xdg_surface_move(window->xdg_surface, input->wl.seat,
                      window->display->serial);
   else if (window->wl_shell_surface)
     wl_shell_surface_move(window->wl_shell_surface, input->wl.seat,
                           window->display->serial);
}

EAPI void
ecore_wl2_window_resize(Ecore_Wl2_Window *window, int w, int h, int location)
{
   Ecore_Wl2_Input *input;

   EINA_SAFETY_ON_NULL_RETURN(window);

   /* test for no-op resize */
   /* if ((window->geometry.w == w) && (window->geometry.h == h)) */
   /*   return; */

   window->geometry.w = w;
   window->geometry.h = h;

   input = window->input;
   if ((!input) && (window->parent))
     {
        input = window->parent->input;
     }

   if ((!input) || (!input->wl.seat)) return;

   _ecore_wl2_input_ungrab(input);

   if (window->xdg_surface)
     xdg_surface_resize(window->xdg_surface, input->wl.seat,
                        input->display->serial, location);
   else if (window->wl_shell_surface)
     wl_shell_surface_resize(window->wl_shell_surface, input->wl.seat,
                             input->display->serial, location);
}

EAPI void
ecore_wl2_window_raise(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (window->xdg_surface)
     {
        struct wl_array states;
        uint32_t *s;

        wl_array_init(&states);
        s = wl_array_add(&states, sizeof(*s));
        *s = XDG_SURFACE_STATE_ACTIVATED;
        _xdg_surface_cb_configure(window, window->xdg_surface,
                                  window->geometry.w, window->geometry.h,
                                  &states, 0);
        wl_array_release(&states);
     }
   else if (window->wl_shell_surface)
     wl_shell_surface_set_toplevel(window->wl_shell_surface);
}

EAPI Ecore_Wl2_Window *
ecore_wl2_window_find(Ecore_Wl2_Display *display, int id)
{
   Ecore_Wl2_Window *window;

   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);

   EINA_INLIST_FOREACH(display->windows, window)
     if (window->id == id) return window;

   return NULL;
}

EAPI Eina_Bool
ecore_wl2_window_alpha_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   return window->alpha;
}

EAPI void
ecore_wl2_window_alpha_set(Ecore_Wl2_Window *window, Eina_Bool alpha)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (window->alpha == alpha) return;

   window->alpha = alpha;

   if (!window->alpha)
     ecore_wl2_window_opaque_region_set(window, window->opaque.x,
                                        window->opaque.y, window->opaque.w,
                                        window->opaque.h);
   else
     ecore_wl2_window_opaque_region_set(window, 0, 0, 0, 0);
}

EAPI void
ecore_wl2_window_transparent_set(Ecore_Wl2_Window *window, Eina_Bool transparent)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (window->transparent == transparent) return;

   window->transparent = transparent;

   if (!window->transparent)
     ecore_wl2_window_opaque_region_set(window, window->opaque.x,
                                        window->opaque.y, window->opaque.w,
                                        window->opaque.h);
   else
     ecore_wl2_window_opaque_region_set(window, 0, 0, 0, 0);
}

EAPI void
ecore_wl2_window_opaque_region_set(Ecore_Wl2_Window *window, int x, int y, int w, int h)
{
   struct wl_region *region;

   EINA_SAFETY_ON_NULL_RETURN(window);

   window->opaque.x = x;
   window->opaque.y = y;
   window->opaque.w = w;
   window->opaque.h = h;

   /* TODO: transparent or alpha check ? */

   region = wl_compositor_create_region(window->display->wl.compositor);
   if (!region)
     {
        ERR("Failed to create opaque region: %m");
        return;
     }

   switch (window->rotation)
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

   wl_surface_set_opaque_region(window->surface, region);
   wl_region_destroy(region);
}

EAPI Eina_Bool
ecore_wl2_window_maximized_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   if (window->type == ECORE_WL2_WINDOW_TYPE_MAXIMIZED)
     return EINA_TRUE;

   return EINA_FALSE;
}

EAPI void
ecore_wl2_window_maximized_set(Ecore_Wl2_Window *window, Eina_Bool maximized)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if ((window->type == ECORE_WL2_WINDOW_TYPE_MAXIMIZED) == maximized)
     return;

   if (window->type == ECORE_WL2_WINDOW_TYPE_TOPLEVEL)
     {
        if (window->xdg_surface)
          xdg_surface_set_maximized(window->xdg_surface);
        else if (window->wl_shell_surface)
          wl_shell_surface_set_maximized(window->wl_shell_surface, NULL);

        window->type = ECORE_WL2_WINDOW_TYPE_MAXIMIZED;
     }
   else if (window->type == ECORE_WL2_WINDOW_TYPE_MAXIMIZED)
     {
        if (window->xdg_surface)
          xdg_surface_unset_maximized(window->xdg_surface);
        else if (window->wl_shell_surface)
          wl_shell_surface_set_toplevel(window->wl_shell_surface);

        window->type = ECORE_WL2_WINDOW_TYPE_TOPLEVEL;
     }

   /* TODO: send configure ? */
}

EAPI Eina_Bool
ecore_wl2_window_fullscreen_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   if (window->type == ECORE_WL2_WINDOW_TYPE_FULLSCREEN)
     return EINA_TRUE;

   return EINA_FALSE;
}

EAPI void
ecore_wl2_window_fullscreen_set(Ecore_Wl2_Window *window, Eina_Bool fullscreen)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if ((window->type == ECORE_WL2_WINDOW_TYPE_FULLSCREEN) == fullscreen)
     return;

   if (fullscreen)
     {
        if (window->xdg_surface)
          xdg_surface_set_fullscreen(window->xdg_surface, NULL);
        else if (window->wl_shell_surface)
          wl_shell_surface_set_fullscreen(window->wl_shell_surface,
                                          WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
                                          0, NULL);

        window->type = ECORE_WL2_WINDOW_TYPE_FULLSCREEN;
     }
   else
     {
        if (window->xdg_surface)
          xdg_surface_unset_fullscreen(window->xdg_surface);
        else if (window->wl_shell_surface)
          wl_shell_surface_set_toplevel(window->wl_shell_surface);

        window->type = ECORE_WL2_WINDOW_TYPE_TOPLEVEL;
     }

   /* TODO: send configure ? */
}

EAPI void
ecore_wl2_window_rotation_set(Ecore_Wl2_Window *window, int rotation)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   window->rotation = rotation;
}

EAPI void
ecore_wl2_window_title_set(Ecore_Wl2_Window *window, const char *title)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   eina_stringshare_replace(&window->title, title);
   if (!window->title) return;

   if (window->xdg_surface)
     xdg_surface_set_title(window->xdg_surface, window->title);
   else if (window->wl_shell_surface)
     wl_shell_surface_set_title(window->wl_shell_surface, window->title);
}

EAPI void
ecore_wl2_window_class_name_set(Ecore_Wl2_Window *window, const char *class_name)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   eina_stringshare_replace(&window->class, class_name);
   if (!window->class) return;

   if (window->xdg_surface)
     xdg_surface_set_app_id(window->xdg_surface, window->class);
   else if (window->wl_shell_surface)
     wl_shell_surface_set_class(window->wl_shell_surface, window->class);
}

EAPI void
ecore_wl2_window_geometry_get(Ecore_Wl2_Window *window, int *x, int *y, int *w, int *h)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (x) *x = window->geometry.x;
   if (y) *y = window->geometry.y;
   if (w) *w = window->geometry.w;
   if (h) *h = window->geometry.h;
}

EAPI void
ecore_wl2_window_geometry_set(Ecore_Wl2_Window *window, int x, int y, int w, int h)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if ((window->geometry.x == x) && (window->geometry.y == y) &&
       (window->geometry.w == w) && (window->geometry.h == h))
     return;

   window->geometry.x = x;
   window->geometry.y = y;
   window->geometry.w = w;
   window->geometry.h = h;

   if (window->xdg_surface)
     xdg_surface_set_window_geometry(window->xdg_surface, x, y, w, h);
}

EAPI Eina_Bool
ecore_wl2_window_iconified_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   return window->minimized;
}

EAPI void
ecore_wl2_window_iconified_set(Ecore_Wl2_Window *window, Eina_Bool iconified)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (window->minimized == iconified) return;

   if (iconified)
     {
        if (window->xdg_surface)
          xdg_surface_set_minimized(window->xdg_surface);
     }
   else
     {
        if (window->xdg_surface)
          {
             struct wl_array states;
             uint32_t *s;

             wl_array_init(&states);
             s = wl_array_add(&states, sizeof(*s));
             *s = XDG_SURFACE_STATE_ACTIVATED;
             _xdg_surface_cb_configure(window, window->xdg_surface,
                                       window->geometry.w, window->geometry.h,
                                       &states, 0);
             wl_array_release(&states);
          }

        window->type = ECORE_WL2_WINDOW_TYPE_TOPLEVEL;
     }

   window->minimized = iconified;
}

EAPI void
ecore_wl2_window_pointer_xy_get(Ecore_Wl2_Window *window, int *x, int *y)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (x) *x = 0;
   if (y) *y = 0;

   if (!window->input) return;

   if (x) *x = window->input->pointer.sx;
   if (y) *y = window->input->pointer.sy;
}

EAPI void
ecore_wl2_window_pointer_set(Ecore_Wl2_Window *window, struct wl_surface *surface, int hot_x, int hot_y)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   /* EINA_SAFETY_ON_NULL_RETURN(window->input); */

   if (!window->input) return;

   _ecore_wl2_input_cursor_update_stop(window->input);

   if (window->input->wl.pointer)
     wl_pointer_set_cursor(window->input->wl.pointer,
                           window->input->pointer.enter_serial,
                           surface, hot_x, hot_y);
}

EAPI void
ecore_wl2_window_cursor_from_name_set(Ecore_Wl2_Window *window, const char *cursor)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   /* EINA_SAFETY_ON_NULL_RETURN(window->input); */

   eina_stringshare_replace(&window->cursor, cursor);

   if (!window->input) return;

   _ecore_wl2_input_cursor_update_stop(window->input);
   _ecore_wl2_input_cursor_set(window->input, cursor);
}

EAPI void
ecore_wl2_window_type_set(Ecore_Wl2_Window *window, Ecore_Wl2_Window_Type type)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   window->type = type;
}

EAPI Ecore_Wl2_Input *
ecore_wl2_window_input_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(window->input, NULL);

   return window->input;
}
