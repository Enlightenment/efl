#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"
#include "efl-hints-client-protocol.h"

static void _ecore_wl2_window_hide_send(Ecore_Wl2_Window *window);

void
_ecore_wl2_window_semi_free(Ecore_Wl2_Window *window)
{
   if (window->xdg_popup) xdg_popup_destroy(window->xdg_popup);
   window->xdg_popup = NULL;

   if (window->xdg_toplevel) xdg_toplevel_destroy(window->xdg_toplevel);
   window->xdg_toplevel = NULL;

   if (window->xdg_surface) xdg_surface_destroy(window->xdg_surface);
   window->xdg_surface = NULL;

   if (window->zxdg_popup) zxdg_popup_v6_destroy(window->zxdg_popup);
   window->zxdg_popup = NULL;

   if (window->zxdg_toplevel) zxdg_toplevel_v6_destroy(window->zxdg_toplevel);
   window->zxdg_toplevel = NULL;

   if (window->zxdg_surface) zxdg_surface_v6_destroy(window->zxdg_surface);
   window->zxdg_surface = NULL;

   if (window->www_surface)
     www_surface_destroy(window->www_surface);
   window->www_surface = NULL;

   if (window->surface) wl_surface_destroy(window->surface);
   window->surface = NULL;
   window->surface_id = -1;

   if (window->callback) wl_callback_destroy(window->callback);
   window->callback = NULL;

   window->outputs = eina_list_free(window->outputs);

   window->commit_pending = EINA_FALSE;
}

static void
_ecore_wl2_window_activate_send(Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Event_Window_Activate *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Activate));
   if (!ev) return;

   ev->win = window->id;
   if (window->parent)
     ev->parent_win = window->parent->id;
   ev->event_win = window->id;
   ecore_event_add(ECORE_WL2_EVENT_WINDOW_ACTIVATE, ev, NULL, NULL);
}

static void
_ecore_wl2_window_deactivate_send(Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Event_Window_Deactivate *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Deactivate));
   if (!ev) return;

   ev->win = window->id;
   if (window->parent)
     ev->parent_win = window->parent->id;
   ev->event_win = window->id;
   ecore_event_add(ECORE_WL2_EVENT_WINDOW_DEACTIVATE, ev, NULL, NULL);
}

static void
_ecore_wl2_window_configure_send(Ecore_Wl2_Window *win)
{
   Ecore_Wl2_Event_Window_Configure *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Configure));
   if (!ev) return;

   ev->win = win->id;
   ev->event_win = win->id;

   if ((win->set_config.geometry.w == win->def_config.geometry.w) &&
       (win->set_config.geometry.h == win->def_config.geometry.h))
     ev->w = ev->h = 0;
   else if ((!win->def_config.geometry.w) && (!win->def_config.geometry.h) &&
            (!win->def_config.fullscreen) &&
            (!win->def_config.maximized) &&
            ((win->def_config.fullscreen != win->req_config.fullscreen) ||
             (win->def_config.maximized != win->req_config.maximized)))
     ev->w = win->saved.w, ev->h = win->saved.h;
   else
     ev->w = win->def_config.geometry.w, ev->h = win->def_config.geometry.h;

   ev->edges = !!win->def_config.resizing;
   if (win->def_config.fullscreen)
     ev->states |= ECORE_WL2_WINDOW_STATE_FULLSCREEN;
   if (win->def_config.maximized)
     ev->states |= ECORE_WL2_WINDOW_STATE_MAXIMIZED;

   win->req_config = win->def_config;
   ecore_event_add(ECORE_WL2_EVENT_WINDOW_CONFIGURE, ev, NULL, NULL);

   if (win->def_config.focused)
     _ecore_wl2_window_activate_send(win);
   else
     _ecore_wl2_window_deactivate_send(win);
}

static void
_configure_complete(Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Event_Window_Configure_Complete *ev;

   window->pending.configure = EINA_FALSE;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Configure_Complete));
   if (!ev) return;

   ev->win = window->id;
   ecore_event_add(ECORE_WL2_EVENT_WINDOW_CONFIGURE_COMPLETE, ev, NULL, NULL);

}

#include "window_v6.x"

static void
_www_surface_end_drag(void *data, struct www_surface *www_surface EINA_UNUSED)
{
   Ecore_Wl2_Window *window = data;
   Ecore_Wl2_Event_Window_WWW_Drag *ev;

   ev = malloc(sizeof(Ecore_Wl2_Event_Window_WWW_Drag));
   EINA_SAFETY_ON_NULL_RETURN(ev);
   ev->window = window->id;
   ev->dragging = 0;

   ecore_event_add(_ecore_wl2_event_window_www_drag, ev, NULL, NULL);
}

static void
_www_surface_start_drag(void *data, struct www_surface *www_surface EINA_UNUSED)
{
   Ecore_Wl2_Window *window = data;
   Ecore_Wl2_Event_Window_WWW_Drag *ev;

   ev = malloc(sizeof(Ecore_Wl2_Event_Window_WWW_Drag));
   EINA_SAFETY_ON_NULL_RETURN(ev);
   ev->window = window->id;
   ev->dragging = 1;

   ecore_event_add(_ecore_wl2_event_window_www_drag, ev, NULL, NULL);
}

static void
_www_surface_status(void *data, struct www_surface *www_surface EINA_UNUSED, int32_t x_rel, int32_t y_rel, uint32_t timestamp)
{
   Ecore_Wl2_Window *window = data;
   Ecore_Wl2_Event_Window_WWW *ev;

   ev = malloc(sizeof(Ecore_Wl2_Event_Window_WWW));
   EINA_SAFETY_ON_NULL_RETURN(ev);
   ev->window = window->id;
   ev->x_rel = x_rel;
   ev->y_rel = y_rel;
   ev->timestamp = timestamp;

   ecore_event_add(_ecore_wl2_event_window_www, ev, NULL, NULL);
}

static struct www_surface_listener _www_surface_listener =
{
   .status = _www_surface_status,
   .start_drag = _www_surface_start_drag,
   .end_drag = _www_surface_end_drag,
};

void
_ecore_wl2_window_www_surface_init(Ecore_Wl2_Window *window)
{
   if (!window->surface) return;
   if (!window->display->wl.www) return;
   if (window->www_surface) return;
   window->www_surface = www_create(window->display->wl.www, window->surface);
   www_surface_set_user_data(window->www_surface, window);
   www_surface_add_listener(window->www_surface, &_www_surface_listener,
                            window);
}


static void
_xdg_surface_cb_configure(void *data, struct xdg_surface *xdg_surface EINA_UNUSED, uint32_t serial)
{
   Ecore_Wl2_Window *window;

   window = data;
   window->def_config.serial = serial;

   if (window->pending.configure)
     {
        window->saved.w = window->set_config.geometry.w;
        window->saved.h = window->set_config.geometry.h;
     }
   if (window->pending.configure && window->updating)
     ERR("Window shouldn't be rendering before initial configure");

   if (!window->updating)
     _ecore_wl2_window_configure_send(window);

   if (window->pending.configure)
     _configure_complete(window);
}

static const struct xdg_surface_listener _xdg_surface_listener =
{
   _xdg_surface_cb_configure,
};

static void
_xdg_toplevel_cb_configure(void *data, struct xdg_toplevel *xdg_toplevel EINA_UNUSED, int32_t width, int32_t height, struct wl_array *states)
{
   Ecore_Wl2_Window *win = data;
   uint32_t *s;

   win->def_config.maximized = EINA_FALSE;
   win->def_config.fullscreen = EINA_FALSE;
   win->def_config.focused = EINA_FALSE;
   win->def_config.resizing = EINA_FALSE;
   win->def_config.geometry.w = width;
   win->def_config.geometry.h = height;

   wl_array_for_each(s, states)
     {
        switch (*s)
          {
           case ZXDG_TOPLEVEL_V6_STATE_MAXIMIZED:
             win->def_config.maximized = EINA_TRUE;
             break;
           case ZXDG_TOPLEVEL_V6_STATE_FULLSCREEN:
             win->def_config.fullscreen = EINA_TRUE;
             break;
           case ZXDG_TOPLEVEL_V6_STATE_RESIZING:
             win->def_config.resizing = EINA_TRUE;
             break;
           case ZXDG_TOPLEVEL_V6_STATE_ACTIVATED:
             win->def_config.focused = EINA_TRUE;
           default:
             break;
          }
     }
}

static void
_xdg_toplevel_cb_close(void *data, struct xdg_toplevel *xdg_toplevel EINA_UNUSED)
{
   Ecore_Wl2_Window *win;

   win = data;
   if (!win) return;

   ecore_wl2_window_free(win);
}

static const struct xdg_toplevel_listener _xdg_toplevel_listener =
{
   _xdg_toplevel_cb_configure,
   _xdg_toplevel_cb_close,
};

static void
_xdg_popup_cb_configure(void *data, struct xdg_popup *xdg_popup EINA_UNUSED, int32_t x EINA_UNUSED, int32_t y EINA_UNUSED, int32_t width, int32_t height)
{
   Ecore_Wl2_Window *win = data;
   win->def_config.geometry.w = width;
   win->def_config.geometry.h = height;
}

static void
_xdg_popup_cb_done(void *data, struct xdg_popup *xdg_popup EINA_UNUSED)
{
   Ecore_Wl2_Window *win;

   win = data;
   if (!win) return;

   if (win->grab) _ecore_wl2_input_ungrab(win->grab);

   _ecore_wl2_window_hide_send(win);
}

static const struct xdg_popup_listener _xdg_popup_listener =
{
   _xdg_popup_cb_configure,
   _xdg_popup_cb_done,
};

static void
_ecore_wl2_window_xdg_popup_create(Ecore_Wl2_Window *win)
{
   struct xdg_positioner *pos;

   EINA_SAFETY_ON_NULL_RETURN(win->parent);
   pos = xdg_wm_base_create_positioner(win->display->wl.xdg_wm_base);
   if (!pos) return;

   xdg_positioner_set_anchor_rect(pos, win->set_config.geometry.x,
                                      win->set_config.geometry.y,
                                      1, 1);
   xdg_positioner_set_size(pos, win->set_config.geometry.w,
                               win->set_config.geometry.h);
   xdg_positioner_set_anchor(pos, XDG_POSITIONER_ANCHOR_TOP_LEFT);
   xdg_positioner_set_gravity(pos, ZXDG_POSITIONER_V6_ANCHOR_BOTTOM |
                                  ZXDG_POSITIONER_V6_ANCHOR_RIGHT);

   win->xdg_popup = xdg_surface_get_popup(win->xdg_surface,
                               win->parent->xdg_surface, pos);

   xdg_positioner_destroy(pos);
   if (win->grab)
     xdg_popup_grab(win->xdg_popup, win->grab->wl.seat,
                        wl_display_get_serial(win->display->wl.display));
   xdg_popup_set_user_data(win->xdg_popup, win);
   xdg_popup_add_listener(win->xdg_popup, &_xdg_popup_listener, win);

   win->pending.configure = EINA_TRUE;

   ecore_wl2_window_commit(win, EINA_TRUE);
}

static void
_window_shell_surface_create(Ecore_Wl2_Window *window)
{
   if (window->xdg_surface) return;
   window->xdg_surface =
     xdg_wm_base_get_xdg_surface(window->display->wl.xdg_wm_base,
                                   window->surface);
   xdg_surface_set_user_data(window->xdg_surface, window);
   xdg_surface_add_listener(window->xdg_surface,
                                &_xdg_surface_listener, window);

   window->xdg_configure_ack = xdg_surface_ack_configure;
   window->pending.configure = EINA_TRUE;
   if (window->display->wl.efl_hints)
     {
        if (window->aspect.set)
          efl_hints_set_aspect(window->display->wl.efl_hints, window->xdg_surface,
            window->aspect.w, window->aspect.h, window->aspect.aspect);
        if (window->weight.set)
          efl_hints_set_weight(window->display->wl.efl_hints, window->xdg_surface,
            window->weight.w, window->weight.h);
     }

   if (window->type == ECORE_WL2_WINDOW_TYPE_MENU)
     _ecore_wl2_window_xdg_popup_create(window);
   else
     {
        struct xdg_toplevel *ptop = NULL;

        window->xdg_toplevel =
          xdg_surface_get_toplevel(window->xdg_surface);
        xdg_toplevel_set_user_data(window->xdg_toplevel, window);
        xdg_toplevel_add_listener(window->xdg_toplevel,
                                      &_xdg_toplevel_listener, window);

        if (window->deferred_minimize)
          xdg_toplevel_set_minimized(window->xdg_toplevel);
        window->deferred_minimize = EINA_FALSE;

        if (window->title)
          xdg_toplevel_set_title(window->xdg_toplevel, window->title);
        if (window->class)
          xdg_toplevel_set_app_id(window->xdg_toplevel, window->class);

        window->xdg_set_min_size = xdg_toplevel_set_min_size;
        window->xdg_set_max_size = xdg_toplevel_set_max_size;

        if (window->parent)
          ptop = window->parent->xdg_toplevel;

        if (ptop)
          xdg_toplevel_set_parent(window->xdg_toplevel, ptop);

        if (window->set_config.maximized)
          xdg_toplevel_set_maximized(window->xdg_toplevel);

        if (window->set_config.fullscreen)
          xdg_toplevel_set_fullscreen(window->xdg_toplevel, NULL);
     }

   ecore_wl2_window_commit(window, EINA_TRUE);
}

void
_ecore_wl2_window_shell_surface_init(Ecore_Wl2_Window *window)
{
   if (!window->surface) return;
   if (window->display->wl.xdg_wm_base) _window_shell_surface_create(window);
   if (window->display->wl.zxdg_shell) _window_v6_shell_surface_create(window);

   if (window->display->wl.session_recovery)
     {
        if (window->uuid)
          {
             zwp_e_session_recovery_set_uuid(window->display->wl.session_recovery,
                                             window->surface, window->uuid);
             if (window->xdg_surface)
               xdg_surface_set_window_geometry(window->xdg_surface,
                                                   window->set_config.geometry.x,
                                                   window->set_config.geometry.y,
                                                   window->set_config.geometry.w,
                                                   window->set_config.geometry.h);
             if (window->zxdg_surface)
               zxdg_surface_v6_set_window_geometry(window->zxdg_surface,
                                                   window->set_config.geometry.x,
                                                   window->set_config.geometry.y,
                                                   window->set_config.geometry.w,
                                                   window->set_config.geometry.h);

             ecore_wl2_window_opaque_region_set(window,
                                                window->opaque.x,
                                                window->opaque.y,
                                                window->opaque.w,
                                                window->opaque.h);
          }
        else
          zwp_e_session_recovery_get_uuid(window->display->wl.session_recovery, window->surface);
     }
}

static void
_surface_enter(void *data, struct wl_surface *surf EINA_UNUSED, struct wl_output *op)
{
   Ecore_Wl2_Window *win;
   Ecore_Wl2_Output *output;

   win = data;

   output = _ecore_wl2_output_find(win->display, op);
   EINA_SAFETY_ON_NULL_RETURN(output);

   win->outputs = eina_list_append(win->outputs, output);
}

static void
_surface_leave(void *data, struct wl_surface *surf EINA_UNUSED, struct wl_output *op)
{
   Ecore_Wl2_Window *win;
   Ecore_Wl2_Output *output;

   win = data;
   output = _ecore_wl2_output_find(win->display, op);
   EINA_SAFETY_ON_NULL_RETURN(output);

   win->outputs = eina_list_remove(win->outputs, output);
   if (!win->outputs)
     {
        Ecore_Wl2_Event_Window_Offscreen *ev;
        ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Offscreen));
        if (ev)
          {
             ev->win = win->id;
             ecore_event_add(ECORE_WL2_EVENT_WINDOW_OFFSCREEN, ev, NULL, NULL);
          }
     }
}

static const struct wl_surface_listener _surface_listener =
{
   _surface_enter,
   _surface_leave,
};

void
_ecore_wl2_window_surface_create(Ecore_Wl2_Window *window)
{
   if (!window->display->wl.compositor) return;

   if (!window->surface)
     {
        window->surface =
          wl_compositor_create_surface(window->display->wl.compositor);
        if (!window->surface)
          {
             ERR("Failed to create surface for window");
             return;
          }
        wl_surface_set_user_data(window->surface, window);

        window->surface_id =
          wl_proxy_get_id((struct wl_proxy *)window->surface);

        wl_surface_add_listener(window->surface, &_surface_listener, window);
        if (window->display->wl.efl_aux_hints)
          {
             efl_aux_hints_get_supported_aux_hints(window->display->wl.efl_aux_hints, window->surface);
             if (_ecore_wl2_display_sync_get())
               wl_display_roundtrip(window->display->wl.display);
          }
     }
}

static void
_ecore_wl2_window_show_send(Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Event_Window_Show *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Show));
   if (!ev) return;

   ev->win = window->id;
   if (window->parent)
     ev->parent_win = window->parent->id;
   ev->event_win = window->id;
   ecore_event_add(ECORE_WL2_EVENT_WINDOW_SHOW, ev, NULL, NULL);
}

static void
_ecore_wl2_window_hide_send(Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Event_Window_Hide *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Hide));
   if (!ev) return;

   ev->win = window->id;
   if (window->parent)
     ev->parent_win = window->parent->id;
   ev->event_win = window->id;
   ecore_event_add(ECORE_WL2_EVENT_WINDOW_HIDE, ev, NULL, NULL);
}

EAPI Ecore_Wl2_Window *
ecore_wl2_window_new(Ecore_Wl2_Display *display, Ecore_Wl2_Window *parent, int x, int y, int w, int h)
{
   Ecore_Wl2_Window *win;
   static int _win_id = 1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);
   if (display->pid) CRI("CANNOT CREATE WINDOW WITH SERVER DISPLAY");

   /* try to allocate space for window structure */
   win = calloc(1, sizeof(Ecore_Wl2_Window));
   if (!win) return NULL;
   display->refs++;

   win->display = display;
   win->parent = parent;
   win->id = _win_id++;

   win->set_config.geometry.x = x;
   win->set_config.geometry.y = y;
   win->set_config.geometry.w = w;
   win->set_config.geometry.h = h;

   win->opaque.x = x;
   win->opaque.y = y;
   win->opaque.w = w;
   win->opaque.h = h;

   win->pending.configure = EINA_TRUE;
   display->windows =
     eina_inlist_append(display->windows, EINA_INLIST_GET(win));

   _ecore_wl2_window_surface_create(win);

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

   _ecore_wl2_window_surface_create(window);

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
   EINA_SAFETY_ON_NULL_RETURN(window);

   _ecore_wl2_window_surface_create(window);

   if (window->input_set)
     ecore_wl2_window_input_region_set(window, window->input_rect.x,
                                       window->input_rect.y,
                                       window->input_rect.w,
                                       window->input_rect.h);
   if (window->opaque_set)
     ecore_wl2_window_opaque_region_set(window, window->opaque.x,
                                        window->opaque.y, window->opaque.w,
                                        window->opaque.h);

   if ((window->type != ECORE_WL2_WINDOW_TYPE_DND) &&
       (window->type != ECORE_WL2_WINDOW_TYPE_NONE))
     {
        _ecore_wl2_window_shell_surface_init(window);
        _ecore_wl2_window_www_surface_init(window);
        _ecore_wl2_window_show_send(window);
     }
   else
     _configure_complete(window);
}

EAPI void
ecore_wl2_window_hide(Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Subsurface *subsurf;
   Eina_Inlist *tmp;

   EINA_SAFETY_ON_NULL_RETURN(window);

   _ecore_wl2_window_hide_send(window);

   EINA_INLIST_FOREACH_SAFE(window->subsurfs, tmp, subsurf)
     _ecore_wl2_subsurf_unmap(subsurf);

   if (window->commit_pending)
     {
        /* We've probably been hidden while an animator
         * is ticking.  Cancel the callback.
         */
        window->commit_pending = EINA_FALSE;
        if (window->callback)
          {
             wl_callback_destroy(window->callback);
             window->callback = NULL;
          }
     }

   if (window->surface)
     {
        wl_surface_attach(window->surface, NULL, 0, 0);
        ecore_wl2_window_commit(window, EINA_TRUE);
        window->commit_pending = EINA_FALSE;
     }

   /* The commit added a callback, disconnect it */
   if (window->callback)
     {
        wl_callback_destroy(window->callback);
        window->callback = NULL;
     }

   window->set_config.serial = 0;
   window->req_config.serial = 0;
   window->def_config.serial = 0;
   window->zxdg_configure_ack = NULL;
   window->xdg_configure_ack = NULL;
   window->xdg_set_min_size = NULL;
   window->xdg_set_max_size = NULL;
   window->zxdg_set_min_size = NULL;
   window->zxdg_set_max_size = NULL;
}

static void
_ecore_wl2_window_aux_hint_free(Ecore_Wl2_Window *win)
{
   const char *supported;

   EINA_LIST_FREE(win->supported_aux_hints, supported)
     if (supported) eina_stringshare_del(supported);
}

EAPI void
ecore_wl2_window_free(Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Display *display;
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Subsurface *subsurf;
   Eina_Inlist *tmp;

   EINA_SAFETY_ON_NULL_RETURN(window);

   display = window->display;

   EINA_INLIST_FOREACH(display->inputs, input)
      _ecore_wl2_input_window_remove(input, window);

   EINA_INLIST_FOREACH_SAFE(window->subsurfs, tmp, subsurf)
     _ecore_wl2_subsurf_free(subsurf);

   _ecore_wl2_window_aux_hint_free(window);

   if (window->callback) wl_callback_destroy(window->callback);
   window->callback = NULL;

   if (window->uuid && window->surface && window->display->wl.session_recovery)
     zwp_e_session_recovery_destroy_uuid(window->display->wl.session_recovery,
                                         window->surface, window->uuid);

   _ecore_wl2_window_semi_free(window);

   eina_stringshare_replace(&window->uuid, NULL);

   if (window->title) eina_stringshare_del(window->title);
   if (window->class) eina_stringshare_del(window->class);
   if (window->role) eina_stringshare_del(window->role);

   display->windows =
     eina_inlist_remove(display->windows, EINA_INLIST_GET(window));

   ecore_wl2_display_disconnect(window->display);
   free(window);
}

EAPI void
ecore_wl2_window_move(Ecore_Wl2_Window *window, Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   EINA_SAFETY_ON_NULL_RETURN(window->display->inputs);

   if (!input)
     {
        ERR("NULL input parameter is deprecated");
        input = EINA_INLIST_CONTAINER_GET(window->display->inputs, Ecore_Wl2_Input);
     }
   if (window->xdg_toplevel)
     xdg_toplevel_move(window->xdg_toplevel, input->wl.seat,
                           window->display->serial);
   if (window->zxdg_toplevel)
     zxdg_toplevel_v6_move(window->zxdg_toplevel, input->wl.seat,
                           window->display->serial);
   ecore_wl2_display_flush(window->display);

   _ecore_wl2_input_ungrab(input);
}

EAPI void
ecore_wl2_window_resize(Ecore_Wl2_Window *window, Ecore_Wl2_Input *input, int location)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   EINA_SAFETY_ON_NULL_RETURN(window->display->inputs);

   if (!input)
     {
        ERR("NULL input parameter is deprecated");
        input = EINA_INLIST_CONTAINER_GET(window->display->inputs, Ecore_Wl2_Input);
     }

   if (window->xdg_toplevel)
     xdg_toplevel_resize(window->xdg_toplevel, input->wl.seat,
                             window->display->serial, location);
   if (window->zxdg_toplevel)
     zxdg_toplevel_v6_resize(window->zxdg_toplevel, input->wl.seat,
                             window->display->serial, location);
   ecore_wl2_display_flush(window->display);

   _ecore_wl2_input_ungrab(input);
}

EAPI void
ecore_wl2_window_raise(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (window->zxdg_toplevel)
     {
        struct wl_array states;
        uint32_t *s;

        wl_array_init(&states);
        s = wl_array_add(&states, sizeof(*s));
        *s = ZXDG_TOPLEVEL_V6_STATE_ACTIVATED;
        _zxdg_toplevel_cb_configure(window, window->zxdg_toplevel,
                                    window->set_config.geometry.w,
                                    window->set_config.geometry.h, &states);
        wl_array_release(&states);
     }
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
   Ecore_Wl2_Surface *surf;

   EINA_SAFETY_ON_NULL_RETURN(window);

   if (window->alpha == alpha) return;

   window->alpha = alpha;
   surf = window->wl2_surface;
   if (surf)
     ecore_wl2_surface_reconfigure(surf, surf->w, surf->h, 0, alpha);
}

EAPI void
ecore_wl2_window_opaque_region_set(Ecore_Wl2_Window *window, int x, int y, int w, int h)
{
   int nx = 0, ny = 0, nw = 0, nh = 0;

   EINA_SAFETY_ON_NULL_RETURN(window);

   switch (window->rotation)
     {
      case 0:
        nx = x;
        ny = y;
        nw = w;
        nh = h;
        break;
      case 90:
        nx = y;
        ny = x;
        nw = h;
        nh = w;
        break;
      case 180:
        nx = x;
        ny = x + y;
        nw = w;
        nh = h;
        break;
      case 270:
        nx = x + y;
        ny = x;
        nw = h;
        nh = w;
        break;
      default:
        break;
     }

   if ((window->opaque.x == nx) && (window->opaque.y == ny) &&
       (window->opaque.w == nw) && (window->opaque.h == nh))
     return;

   window->opaque.x = nx;
   window->opaque.y = ny;
   window->opaque.w = nw;
   window->opaque.h = nh;
   window->opaque_set = x || y || w || h;
   window->pending.opaque = EINA_TRUE;
}

EAPI void
ecore_wl2_window_input_region_set(Ecore_Wl2_Window *window, int x, int y, int w, int h)
{
   int nx = 0, ny = 0, nw = 0, nh = 0;

   EINA_SAFETY_ON_NULL_RETURN(window);

   switch (window->rotation)
     {
      case 0:
        nx = x;
        ny = y;
        nw = w;
        nh = h;
        break;
      case 90:
        nx = y;
        ny = x;
        nw = h;
        nh = w;
        break;
      case 180:
        nx = x;
        ny = x + y;
        nw = w;
        nh = h;
        break;
      case 270:
        nx = x + y;
        ny = x;
        nw = h;
        nh = w;
        break;
      default:
        break;
     }

   if ((window->input_rect.x == nx) && (window->input_rect.y == ny) &&
       (window->input_rect.w == nw) && (window->input_rect.h == nh))
     return;

   window->input_rect.x = nx;
   window->input_rect.y = ny;
   window->input_rect.w = nw;
   window->input_rect.h = nh;
   window->input_set = x || y || w || h;
   window->pending.input = EINA_TRUE;
}

EAPI Eina_Bool
ecore_wl2_window_maximized_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   return window->set_config.maximized;
}

EAPI void
ecore_wl2_window_maximized_set(Ecore_Wl2_Window *window, Eina_Bool maximized)
{
   Eina_Bool prev;

   EINA_SAFETY_ON_NULL_RETURN(window);

   prev = window->set_config.maximized;
   maximized = !!maximized;
   if (prev == maximized) return;

   window->set_config.maximized = maximized;
   if (window->updating)
     {
        window->pending.maximized = EINA_TRUE;
        return;
     }

   if (maximized)
     {
        if (!window->set_config.fullscreen)
          window->saved = window->set_config.geometry;

        if (window->xdg_toplevel)
          xdg_toplevel_set_maximized(window->xdg_toplevel);
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_set_maximized(window->zxdg_toplevel);
     }
   else
     {
        if (window->xdg_toplevel)
          xdg_toplevel_unset_maximized(window->xdg_toplevel);
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_unset_maximized(window->zxdg_toplevel);
     }
   ecore_wl2_display_flush(window->display);
}

EAPI Eina_Bool
ecore_wl2_window_fullscreen_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   return window->set_config.fullscreen;
}

EAPI void
ecore_wl2_window_fullscreen_set(Ecore_Wl2_Window *window, Eina_Bool fullscreen)
{
   Eina_Bool prev;

   EINA_SAFETY_ON_NULL_RETURN(window);

   prev = window->set_config.fullscreen;
   fullscreen = !!fullscreen;
   if (prev == fullscreen) return;

   window->set_config.fullscreen = fullscreen;
   if (window->updating)
     {
        window->pending.fullscreen = EINA_TRUE;
        return;
     }

   if (fullscreen)
     {
        if (!window->set_config.maximized)
          window->saved = window->set_config.geometry;

        if (window->xdg_toplevel)
          xdg_toplevel_set_fullscreen(window->xdg_toplevel, NULL);
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_set_fullscreen(window->zxdg_toplevel, NULL);
     }
   else
     {
        if (window->xdg_toplevel)
          xdg_toplevel_unset_fullscreen(window->xdg_toplevel);
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_unset_fullscreen(window->zxdg_toplevel);
     }
   ecore_wl2_display_flush(window->display);
}

EAPI int
ecore_wl2_window_rotation_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, -1);

   return window->rotation;
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
   if (!window->xdg_toplevel && !window->xdg_toplevel) return;

   if (window->xdg_toplevel)
     xdg_toplevel_set_title(window->xdg_toplevel, window->title);
   if (window->zxdg_toplevel)
     zxdg_toplevel_v6_set_title(window->zxdg_toplevel, window->title);
   ecore_wl2_display_flush(window->display);
}

EAPI void
ecore_wl2_window_class_set(Ecore_Wl2_Window *window, const char *clas)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   eina_stringshare_replace(&window->class, clas);
   if (!window->class) return;
   if (!window->xdg_toplevel && !window->xdg_toplevel) return;

   if (window->xdg_toplevel)
     xdg_toplevel_set_app_id(window->xdg_toplevel, window->class);
   if (window->zxdg_toplevel)
     zxdg_toplevel_v6_set_app_id(window->zxdg_toplevel, window->class);
   ecore_wl2_display_flush(window->display);
}

EAPI void
ecore_wl2_window_geometry_get(Ecore_Wl2_Window *window, int *x, int *y, int *w, int *h)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (x) *x = window->set_config.geometry.x;
   if (y) *y = window->set_config.geometry.y;
   if (w) *w = window->set_config.geometry.w;
   if (h) *h = window->set_config.geometry.h;
}

EAPI void
ecore_wl2_window_geometry_set(Ecore_Wl2_Window *window, int x, int y, int w, int h)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if ((window->set_config.geometry.x == x) &&
       (window->set_config.geometry.y == y) &&
       (window->set_config.geometry.w == w) &&
       (window->set_config.geometry.h == h))
     return;

   window->set_config.geometry.x = x;
   window->set_config.geometry.y = y;
   window->set_config.geometry.w = w;
   window->set_config.geometry.h = h;
   window->pending.geom = EINA_TRUE;
}

EAPI void
ecore_wl2_window_iconified_set(Ecore_Wl2_Window *window, Eina_Bool iconified)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   iconified = !!iconified;

   if (!window->xdg_toplevel && !window->zxdg_toplevel)
     {
        window->deferred_minimize = iconified;
        return;
     }

   if (iconified)
     {
        if (window->xdg_toplevel)
          xdg_toplevel_set_minimized(window->xdg_toplevel);
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_set_minimized(window->zxdg_toplevel);
        ecore_wl2_display_flush(window->display);
     }
   else
     {
        if (window->xdg_toplevel)
          {
             struct wl_array states;
             uint32_t *s;

             wl_array_init(&states);
             s = wl_array_add(&states, sizeof(*s));
             *s = XDG_TOPLEVEL_STATE_ACTIVATED;
             _xdg_toplevel_cb_configure(window, window->xdg_toplevel,
                                         window->set_config.geometry.w,
                                         window->set_config.geometry.h, &states);
             wl_array_release(&states);
          }
        if (window->zxdg_toplevel)
          {
             struct wl_array states;
             uint32_t *s;

             wl_array_init(&states);
             s = wl_array_add(&states, sizeof(*s));
             *s = ZXDG_TOPLEVEL_V6_STATE_ACTIVATED;
             _zxdg_toplevel_cb_configure(window, window->zxdg_toplevel,
                                         window->set_config.geometry.w,
                                         window->set_config.geometry.h, &states);
             wl_array_release(&states);
          }
     }
}

EAPI void
ecore_wl2_window_type_set(Ecore_Wl2_Window *window, Ecore_Wl2_Window_Type type)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   window->type = type;
}

EAPI void
ecore_wl2_window_popup_input_set(Ecore_Wl2_Window *window, Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   EINA_SAFETY_ON_NULL_RETURN(input);
   EINA_SAFETY_ON_TRUE_RETURN(window->type != ECORE_WL2_WINDOW_TYPE_MENU);
   window->grab = input;
}

EAPI Ecore_Wl2_Display *
ecore_wl2_window_display_get(const Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(window->display, NULL);

   if (window->display->recovering) return NULL;

   return window->display;
}

EAPI Eina_Bool
ecore_wl2_window_shell_surface_exists(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);
   return !!window->zxdg_surface || !!window->xdg_surface;
}

EAPI Eina_Bool
ecore_wl2_window_activated_get(const Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);
   return window->req_config.focused;
}

EAPI Ecore_Wl2_Output *
ecore_wl2_window_output_find(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, NULL);

   return eina_list_data_get(window->outputs);
}

EAPI void
ecore_wl2_window_buffer_transform_set(Ecore_Wl2_Window *window, int transform)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   wl_surface_set_buffer_transform(window->surface, transform);
}

EAPI void
ecore_wl2_window_wm_rotation_supported_set(Ecore_Wl2_Window *window, Eina_Bool enabled)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   window->wm_rot.supported = enabled;
}

EAPI Eina_Bool
ecore_wl2_window_wm_rotation_supported_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);
   return window->wm_rot.supported;
}

EAPI void
ecore_wl2_window_rotation_app_set(Ecore_Wl2_Window *window, Eina_Bool set)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   window->wm_rot.app_set = set;
}

EAPI Eina_Bool
ecore_wl2_window_rotation_app_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);
   return window->wm_rot.app_set;
}

EAPI void
ecore_wl2_window_preferred_rotation_set(Ecore_Wl2_Window *window, int rot)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   window->wm_rot.preferred_rot = rot;
}

EAPI int
ecore_wl2_window_preferred_rotation_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, 0);
   return window->wm_rot.preferred_rot;
}

EAPI void
ecore_wl2_window_available_rotations_set(Ecore_Wl2_Window *window, const int *rots, unsigned int count)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   window->wm_rot.count = count;
   window->wm_rot.available_rots = (int *)rots;
}

EAPI Eina_Bool
ecore_wl2_window_available_rotations_get(Ecore_Wl2_Window *window, int **rots, unsigned int *count)
{
   unsigned int i = 0;
   int *val = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   *rots = NULL;
   *count = window->wm_rot.count;

   if (window->wm_rot.count >= 1)
     {
        val = calloc(window->wm_rot.count, sizeof(int));
        if (!val) return EINA_FALSE;

        for (; i < window->wm_rot.count; i++)
          val[i] = ((int *)window->wm_rot.available_rots)[i];

        *rots = val;
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI void
ecore_wl2_window_rotation_change_prepare_send(Ecore_Wl2_Window *window, int rot, int w, int h, Eina_Bool resize)
{
   Ecore_Wl2_Event_Window_Rotation_Change_Prepare *ev;

   EINA_SAFETY_ON_NULL_RETURN(window);

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Rotation_Change_Prepare));
   if (!ev) return;

   ev->win = window->id;
   ev->rotation = rot;
   ev->w = w;
   ev->h = h;
   ev->resize = resize;

   ecore_event_add(ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE, ev, NULL, NULL);
}

EAPI void
ecore_wl2_window_rotation_change_prepare_done_send(Ecore_Wl2_Window *window, int rot)
{
   Ecore_Wl2_Event_Window_Rotation_Change_Prepare_Done *ev;

   EINA_SAFETY_ON_NULL_RETURN(window);

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Rotation_Change_Prepare_Done));
   if (!ev) return;

   ev->win = window->id;
   ev->rotation = rot;
   ev->w = 0;
   ev->h = 0;
   ev->resize = 0;

   ecore_event_add(ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE_DONE,
                   ev, NULL, NULL);
}

EAPI void
ecore_wl2_window_rotation_change_request_send(Ecore_Wl2_Window *window, int rot)
{
   Ecore_Wl2_Event_Window_Rotation_Change_Request *ev;

   EINA_SAFETY_ON_NULL_RETURN(window);

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Rotation_Change_Request));
   if (!ev) return;

   ev->win = window->id;
   ev->rotation = rot;
   ev->w = 0;
   ev->h = 0;
   ev->resize = 0;

   ecore_event_add(ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_REQUEST,
                   ev, NULL, NULL);
}

EAPI void
ecore_wl2_window_rotation_change_done_send(Ecore_Wl2_Window *window, int rot, int w, int h)
{
   Ecore_Wl2_Event_Window_Rotation_Change_Done *ev;

   EINA_SAFETY_ON_NULL_RETURN(window);

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Rotation_Change_Done));
   if (!ev) return;

   ev->win = window->id;
   ev->rotation = rot;
   ev->w = w;
   ev->h = h;
   ev->resize = 0;

   ecore_event_add(ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_DONE,
                   ev, NULL, NULL);
}

EAPI Eina_List *
ecore_wl2_window_aux_hints_supported_get(Ecore_Wl2_Window *win)
{
   Eina_List *res = NULL;
   Eina_List *ll;
   char *supported_hint = NULL;
   const char *hint = NULL;

   if (!win) return NULL;
   if (!win->surface) return NULL;

   EINA_LIST_FOREACH(win->supported_aux_hints, ll, supported_hint)
     {
        hint = eina_stringshare_add(supported_hint);
        res = eina_list_append(res, hint);
     }
   return res;
}

EAPI void
ecore_wl2_window_aux_hint_add(Ecore_Wl2_Window *win, int id, const char *hint, const char *val)
{
   if (!win) return;
   if ((!win->surface) || (!win->display->wl.efl_aux_hints)) return;

   efl_aux_hints_add_aux_hint(win->display->wl.efl_aux_hints, win->surface, id, hint, val);
   ecore_wl2_display_flush(win->display);
}

EAPI void
ecore_wl2_window_aux_hint_change(Ecore_Wl2_Window *win, int id, const char *val)
{
   if (!win) return;
   if ((!win->surface) && (!win->display->wl.efl_aux_hints)) return;

   efl_aux_hints_change_aux_hint(win->display->wl.efl_aux_hints, win->surface, id, val);
   ecore_wl2_display_flush(win->display);
}

EAPI void
ecore_wl2_window_aux_hint_del(Ecore_Wl2_Window *win, int id)
{
   if (!win) return;
   if ((!win->surface) || (!win->display->wl.efl_aux_hints)) return;

   efl_aux_hints_del_aux_hint(win->display->wl.efl_aux_hints, win->surface, id);
   ecore_wl2_display_flush(win->display);
}

EAPI void
ecore_wl2_window_focus_skip_set(Ecore_Wl2_Window *window, Eina_Bool focus_skip)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   window->focus_skip = focus_skip;
}

EAPI Eina_Bool
ecore_wl2_window_focus_skip_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);
   return window->focus_skip;
}

EAPI void
ecore_wl2_window_role_set(Ecore_Wl2_Window *window, const char *role)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   eina_stringshare_replace(&window->role, role);
}

EAPI void
ecore_wl2_window_floating_mode_set(Ecore_Wl2_Window *window, Eina_Bool floating)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   window->floating = floating;
}

EAPI Eina_Bool
ecore_wl2_window_floating_mode_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);
   return window->floating;
}

EAPI void
ecore_wl2_window_aspect_set(Ecore_Wl2_Window *window, int w, int h, unsigned int aspect)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   EINA_SAFETY_ON_TRUE_RETURN(w < 1);
   EINA_SAFETY_ON_TRUE_RETURN(h < 1);

   if ((window->aspect.aspect == aspect) && (window->aspect.w == w) &&
       (window->aspect.h == h))
     return;

   window->aspect.w = w;
   window->aspect.h = h;
   window->aspect.aspect = aspect;
   window->aspect.set = 1;
   if (!window->display->wl.efl_hints) return;
   if (window->xdg_surface)
     efl_hints_set_aspect(window->display->wl.efl_hints,
                          window->xdg_surface, w, h, aspect);
   ecore_wl2_display_flush(window->display);
}

EAPI void
ecore_wl2_window_weight_set(Ecore_Wl2_Window *window, double w, double h)
{
   int ww, hh;
   EINA_SAFETY_ON_NULL_RETURN(window);

   ww = lround(w * 100);
   hh = lround(h * 100);

   if ((window->weight.w == ww) && (window->weight.h == hh))
     return;

   window->weight.w = ww;
   window->weight.h = hh;
   window->weight.set = 1;
   if (!window->display->wl.efl_hints) return;
   if (window->xdg_surface)
     efl_hints_set_weight(window->display->wl.efl_hints,
                          window->xdg_surface, ww, hh);
   ecore_wl2_display_flush(window->display);
}

static void
_frame_cb(void *data, struct wl_callback *callback, uint32_t timestamp)
{
   Ecore_Wl2_Frame_Cb_Handle *cb;
   Ecore_Wl2_Window *window;
   Eina_List *l, *ll;

   window = data;
   window->commit_pending = EINA_FALSE;
   wl_callback_destroy(callback);
   window->callback = NULL;
   EINA_LIST_FOREACH_SAFE(window->frame_callbacks, l, ll, cb)
     cb->cb(window, timestamp, cb->data);
}

static struct wl_callback_listener _frame_listener =
{
   _frame_cb
};

static void
_maximized_set(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_FALSE_RETURN(window->zxdg_toplevel || window->xdg_toplevel);

   if (window->set_config.maximized)
     {
        window->saved = window->set_config.geometry;
        if (window->xdg_toplevel)
          xdg_toplevel_set_maximized(window->xdg_toplevel);
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_set_maximized(window->zxdg_toplevel);
     }
   else
     {
        if (window->xdg_toplevel)
          xdg_toplevel_unset_maximized(window->xdg_toplevel);
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_unset_maximized(window->zxdg_toplevel);
     }
}

static void
_fullscreen_set(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_FALSE_RETURN(window->zxdg_toplevel || window->xdg_toplevel);

   if (window->set_config.fullscreen)
     {
        window->saved = window->set_config.geometry;
        if (window->xdg_toplevel)
          xdg_toplevel_set_fullscreen(window->xdg_toplevel, NULL);
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_set_fullscreen(window->zxdg_toplevel, NULL);
     }
   else
     {
        if (window->xdg_toplevel)
          xdg_toplevel_unset_fullscreen(window->xdg_toplevel);
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_unset_fullscreen(window->zxdg_toplevel);
     }
}

static struct wl_region *
_region_create(struct wl_compositor *comp, int x, int y, int w, int h)
{
   struct wl_region *out;

   out = wl_compositor_create_region(comp);
   if (!out)
     {
        ERR("Failed to create region");
        return NULL;
     }

   wl_region_add(out, x, y, w, h);

   return out;
}

static void
_regions_set(Ecore_Wl2_Window *window)
{
   struct wl_region *region = NULL;

   if (window->pending.opaque)
     {
        if (window->opaque_set)
          {
             region = _region_create(window->display->wl.compositor,
                                     window->opaque.x, window->opaque.y,
                                     window->opaque.w, window->opaque.h);
             if (!region) return;
          }
        wl_surface_set_opaque_region(window->surface, region);
     }

   if (!window->pending.input) goto out;
   if (window->type == ECORE_WL2_WINDOW_TYPE_DND) goto out;

   if (!window->input_set)
     {
        wl_surface_set_input_region(window->surface, NULL);
        goto out;
     }

   if (region && (window->opaque.x == window->input_rect.x) &&
       (window->opaque.y == window->input_rect.y) &&
       (window->opaque.w == window->input_rect.w) &&
       (window->opaque.h == window->input_rect.h))
     {
        wl_surface_set_input_region(window->surface, region);
        goto out;
     }
   if (region) wl_region_destroy(region);

   region = _region_create(window->display->wl.compositor,
                           window->input_rect.x, window->input_rect.y,
                           window->input_rect.w, window->input_rect.h);
   if (!region) return;
   wl_surface_set_input_region(window->surface, region);

out:
   if (region) wl_region_destroy(region);
}

EAPI void
ecore_wl2_window_commit(Ecore_Wl2_Window *window, Eina_Bool flush)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   EINA_SAFETY_ON_NULL_RETURN(window->surface);

   if (window->commit_pending)
     {
        if (window->callback)
          wl_callback_destroy(window->callback);
        window->callback = NULL;
        ERR("Commit before previous commit processed");
     }
   if (!window->pending.configure)
     {
        if (window->has_buffer)
          window->commit_pending = EINA_TRUE;
        window->callback = wl_surface_frame(window->surface);
        wl_callback_add_listener(window->callback, &_frame_listener, window);
        /* Dispatch any state we've been saving along the way */
        if (window->pending.geom)
          {
             if (window->xdg_toplevel)
               xdg_surface_set_window_geometry(window->xdg_surface,
                                                   window->set_config.geometry.x,
                                                   window->set_config.geometry.y,
                                                   window->set_config.geometry.w,
                                                   window->set_config.geometry.h);
             if (window->zxdg_surface)
               zxdg_surface_v6_set_window_geometry(window->zxdg_surface,
                                                   window->set_config.geometry.x,
                                                   window->set_config.geometry.y,
                                                   window->set_config.geometry.w,
                                                   window->set_config.geometry.h);
          }
        if (window->pending.opaque || window->pending.input)
          _regions_set(window);

        if (window->pending.maximized)
          _maximized_set(window);

        if (window->pending.fullscreen)
          _fullscreen_set(window);

        window->pending.geom = EINA_FALSE;
        window->pending.opaque = EINA_FALSE;
        window->pending.input = EINA_FALSE;
        window->pending.maximized = EINA_FALSE;
        window->pending.fullscreen = EINA_FALSE;
     }

   if (window->req_config.serial != window->set_config.serial)
     {
        if (window->xdg_configure_ack)
           window->xdg_configure_ack(window->xdg_surface,
                                      window->req_config.serial);
        if (window->zxdg_configure_ack)
           window->zxdg_configure_ack(window->zxdg_surface,
                                      window->req_config.serial);
        window->set_config.serial = window->req_config.serial;
     }
   if (flush)
     {
        wl_surface_commit(window->surface);
        ecore_wl2_display_flush(window->display);
     }

   if (!window->updating) return;

   window->updating = EINA_FALSE;
   if (window->def_config.serial != window->set_config.serial)
      _ecore_wl2_window_configure_send(window);
}

EAPI void
ecore_wl2_window_false_commit(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   EINA_SAFETY_ON_NULL_RETURN(window->surface);
   EINA_SAFETY_ON_TRUE_RETURN(window->pending.configure);
   EINA_SAFETY_ON_TRUE_RETURN(window->commit_pending);

   window->callback = wl_surface_frame(window->surface);
   wl_callback_add_listener(window->callback, &_frame_listener, window);
   wl_surface_commit(window->surface);
   ecore_wl2_display_flush(window->display);
   if (window->has_buffer)
     window->commit_pending = EINA_TRUE;
}

EAPI Eina_Bool
ecore_wl2_window_pending_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   return window->commit_pending;
}

EAPI Ecore_Wl2_Frame_Cb_Handle *
ecore_wl2_window_frame_callback_add(Ecore_Wl2_Window *window, Ecore_Wl2_Frame_Cb cb, void *data)
{
   Ecore_Wl2_Frame_Cb_Handle *callback;

   EINA_SAFETY_ON_NULL_RETURN_VAL(window, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, NULL);

   callback = malloc(sizeof(*callback));
   EINA_SAFETY_ON_NULL_RETURN_VAL(callback, NULL);
   callback->cb = cb;
   callback->data = data;
   callback->win = window;
   window->frame_callbacks =
     eina_list_append(window->frame_callbacks, callback);
   return callback;
}

EAPI void
ecore_wl2_window_frame_callback_del(Ecore_Wl2_Frame_Cb_Handle *handle)
{
   EINA_SAFETY_ON_NULL_RETURN(handle);

   handle->win->frame_callbacks =
     eina_list_remove(handle->win->frame_callbacks, handle);
   free(handle);
}

EAPI void
ecore_wl2_window_buffer_attach(Ecore_Wl2_Window *win, void *buffer, int x, int y, Eina_Bool implicit)
{
   EINA_SAFETY_ON_NULL_RETURN(win);
   EINA_SAFETY_ON_NULL_RETURN(win->surface);

   /* FIXME: Haven't given any thought to x and y since we always use 0... */
   if (!implicit) wl_surface_attach(win->surface, buffer, x, y);
   win->buffer = buffer;
   if (!implicit && !buffer)
     win->has_buffer = EINA_FALSE;
   else
     win->has_buffer = EINA_TRUE;
}

EAPI Eina_Bool
ecore_wl2_window_resizing_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   return window->req_config.resizing;
}

EAPI void
ecore_wl2_window_update_begin(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   EINA_SAFETY_ON_TRUE_RETURN(window->updating);

   window->updating = EINA_TRUE;
}

EAPI void
ecore_wl2_window_damage(Ecore_Wl2_Window *window, Eina_Rectangle *rects, unsigned int count)
{
   void (*damage)(struct wl_surface *, int32_t, int32_t, int32_t, int32_t);
   unsigned int k;
   int compositor_version;

   EINA_SAFETY_ON_NULL_RETURN(window);

   compositor_version = window->display->wl.compositor_version;

   if (compositor_version >= WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION)
     damage = wl_surface_damage_buffer;
   else
     damage = wl_surface_damage;

   if ((rects) && (count > 0))
     for (k = 0; k < count; k++)
       damage(window->surface, rects[k].x, rects[k].y, rects[k].w, rects[k].h);
   else
     damage(window->surface, 0, 0, INT_MAX, INT_MAX);
}

EAPI void
ecore_wl2_window_surface_flush(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (!window->wl2_surface) return;
   ecore_wl2_surface_flush(window->wl2_surface);
}
