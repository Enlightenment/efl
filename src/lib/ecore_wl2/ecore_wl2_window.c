#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"
#include "efl-hints-client-protocol.h"

void
_ecore_wl2_window_semi_free(Ecore_Wl2_Window *window)
{
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
}


static void
_ecore_wl2_window_configure_send(Ecore_Wl2_Window *window, int w, int h, unsigned int edges, Eina_Bool fs, Eina_Bool max)
{
   Ecore_Wl2_Event_Window_Configure *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Configure));
   if (!ev) return;

   ev->win = window->id;
   ev->event_win = window->id;
   ev->w = w;
   ev->h = h;
   ev->edges = edges;
   if (fs)
     ev->states |= ECORE_WL2_WINDOW_STATE_FULLSCREEN;
   if (max)
     ev->states |= ECORE_WL2_WINDOW_STATE_MAXIMIZED;

   ecore_event_add(ECORE_WL2_EVENT_WINDOW_CONFIGURE, ev, NULL, NULL);
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
_zxdg_surface_cb_configure(void *data, struct zxdg_surface_v6 *zxdg_surface EINA_UNUSED, uint32_t serial)
{
   Ecore_Wl2_Window *window;
   Ecore_Wl2_Event_Window_Configure_Complete *ev;

   window = data;
   window->configure_serial = serial;
   if (!window->pending.configure) return;
   window->pending.configure = EINA_FALSE;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Window_Configure_Complete));
   if (!ev) return;

   ev->win = window->id;
   ecore_event_add(ECORE_WL2_EVENT_WINDOW_CONFIGURE_COMPLETE, ev, NULL, NULL);
}

static const struct zxdg_surface_v6_listener _zxdg_surface_listener =
{
   _zxdg_surface_cb_configure,
};

static void
_zxdg_toplevel_cb_configure(void *data, struct zxdg_toplevel_v6 *zxdg_toplevel EINA_UNUSED, int32_t width, int32_t height, struct wl_array *states)
{
   Ecore_Wl2_Window *win = data;
   uint32_t *s;
   Eina_Bool fs, max;

   if ((!win->maximized) && (!win->fullscreen))
     win->saved = win->geometry;

   fs = win->fullscreen;
   max = win->maximized;

   win->minimized = EINA_FALSE;
   win->maximized = EINA_FALSE;
   win->fullscreen = EINA_FALSE;
   win->focused = EINA_FALSE;
   win->resizing = EINA_FALSE;

   wl_array_for_each(s, states)
     {
        switch (*s)
          {
           case ZXDG_TOPLEVEL_V6_STATE_MAXIMIZED:
             win->maximized = EINA_TRUE;
             break;
           case ZXDG_TOPLEVEL_V6_STATE_FULLSCREEN:
             win->fullscreen = EINA_TRUE;
             break;
           case ZXDG_TOPLEVEL_V6_STATE_RESIZING:
             win->resizing = EINA_TRUE;
             break;
           case ZXDG_TOPLEVEL_V6_STATE_ACTIVATED:
             win->focused = EINA_TRUE;
             win->minimized = EINA_FALSE;
           default:
             break;
          }
     }

   if ((win->geometry.w == width) && (win->geometry.h == height))
     width = height = 0;
   else if ((!width) && (!height) && (!win->fullscreen) && (!win->maximized) &&
            ((win->fullscreen != fs) || (win->maximized != max)))
     width = win->saved.w, height = win->saved.h;

   _ecore_wl2_window_configure_send(win, width, height, !!win->resizing,
                                    win->fullscreen, win->maximized);

   if (win->focused)
     _ecore_wl2_window_activate_send(win);
   else
     _ecore_wl2_window_deactivate_send(win);
}

static void
_zxdg_toplevel_cb_close(void *data, struct zxdg_toplevel_v6 *zxdg_toplevel EINA_UNUSED)
{
   Ecore_Wl2_Window *win;

   win = data;
   if (!win) return;

   ecore_wl2_window_free(win);
}

static const struct zxdg_toplevel_v6_listener _zxdg_toplevel_listener =
{
   _zxdg_toplevel_cb_configure,
   _zxdg_toplevel_cb_close,
};

static void
_zxdg_popup_cb_configure(void *data EINA_UNUSED, struct zxdg_popup_v6 *zxdg_popup EINA_UNUSED, int32_t x EINA_UNUSED, int32_t y EINA_UNUSED, int32_t w EINA_UNUSED, int32_t h EINA_UNUSED)
{

}

static void
_zxdg_popup_cb_done(void *data, struct zxdg_popup_v6 *zxdg_popup EINA_UNUSED)
{
   Ecore_Wl2_Window *win;

   win = data;
   if (!win) return;

   if (win->grab) _ecore_wl2_input_ungrab(win->grab);
}

static const struct zxdg_popup_v6_listener _zxdg_popup_listener =
{
   _zxdg_popup_cb_configure,
   _zxdg_popup_cb_done,
};

static void
_ecore_wl2_window_zxdg_popup_create(Ecore_Wl2_Window *win)
{
   struct zxdg_positioner_v6 *pos;

   EINA_SAFETY_ON_NULL_RETURN(win->parent);
   pos = zxdg_shell_v6_create_positioner(win->display->wl.zxdg_shell);
   if (!pos) return;

   zxdg_positioner_v6_set_anchor_rect(pos, win->geometry.x, win->geometry.y,
                                      1, 1);
   zxdg_positioner_v6_set_size(pos, win->geometry.w, win->geometry.h);
   zxdg_positioner_v6_set_anchor(pos, ZXDG_POSITIONER_V6_ANCHOR_TOP |
                                 ZXDG_POSITIONER_V6_ANCHOR_LEFT);
   zxdg_positioner_v6_set_gravity(pos, ZXDG_POSITIONER_V6_ANCHOR_BOTTOM |
                                  ZXDG_POSITIONER_V6_ANCHOR_RIGHT);

   win->zxdg_popup = zxdg_surface_v6_get_popup(win->zxdg_surface,
                               win->parent->zxdg_surface, pos);

   zxdg_positioner_v6_destroy(pos);
   if (win->grab)
     zxdg_popup_v6_grab(win->zxdg_popup, win->grab->wl.seat,
                        wl_display_get_serial(win->display->wl.display));
   zxdg_popup_v6_set_user_data(win->zxdg_popup, win);
   zxdg_popup_v6_add_listener(win->zxdg_popup, &_zxdg_popup_listener, win);

   win->pending.configure = EINA_TRUE;

   wl_surface_commit(win->surface);
}

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

void
_ecore_wl2_window_shell_surface_init(Ecore_Wl2_Window *window)
{
   if (!window->surface) return;
   if ((window->display->wl.zxdg_shell) && (!window->zxdg_surface))
     {
        window->zxdg_surface =
          zxdg_shell_v6_get_xdg_surface(window->display->wl.zxdg_shell,
                                        window->surface);
        zxdg_surface_v6_set_user_data(window->zxdg_surface, window);
        zxdg_surface_v6_add_listener(window->zxdg_surface,
                                     &_zxdg_surface_listener, window);

        window->zxdg_configure_ack = zxdg_surface_v6_ack_configure;
        window->pending.configure = EINA_TRUE;

        if (window->type == ECORE_WL2_WINDOW_TYPE_MENU)
          _ecore_wl2_window_zxdg_popup_create(window);
        else
          {
             window->zxdg_toplevel =
               zxdg_surface_v6_get_toplevel(window->zxdg_surface);
             zxdg_toplevel_v6_set_user_data(window->zxdg_toplevel, window);
             zxdg_toplevel_v6_add_listener(window->zxdg_toplevel,
                                           &_zxdg_toplevel_listener, window);

             if (window->title)
               zxdg_toplevel_v6_set_title(window->zxdg_toplevel, window->title);
             if (window->class)
               zxdg_toplevel_v6_set_app_id(window->zxdg_toplevel, window->class);

             window->zxdg_set_min_size = zxdg_toplevel_v6_set_min_size;
             window->zxdg_set_max_size = zxdg_toplevel_v6_set_max_size;

             {
                struct zxdg_toplevel_v6 *ptop = NULL;

                if (window->parent)
                  ptop = window->parent->zxdg_toplevel;

                if (ptop)
                  zxdg_toplevel_v6_set_parent(window->zxdg_toplevel, ptop);
             }

             if (window->maximized)
               zxdg_toplevel_v6_set_maximized(window->zxdg_toplevel);

             if (window->fullscreen)
               zxdg_toplevel_v6_set_fullscreen(window->zxdg_toplevel, NULL);
             if (window->aspect.set && window->display->wl.efl_hints)
               efl_hints_set_aspect(window->display->wl.efl_hints, window->zxdg_toplevel,
                 window->aspect.w, window->aspect.h, window->aspect.aspect);
          }

        wl_surface_commit(window->surface);
     }

   if (window->display->wl.session_recovery)
     {
        if (window->uuid)
          {
             zwp_e_session_recovery_set_uuid(window->display->wl.session_recovery,
                                             window->surface, window->uuid);
             if (window->zxdg_surface)
               zxdg_surface_v6_set_window_geometry(window->zxdg_surface,
                                                   window->geometry.x,
                                                   window->geometry.y,
                                                   window->geometry.w,
                                                   window->geometry.h);

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

   win->geometry.x = x;
   win->geometry.y = y;
   win->geometry.w = w;
   win->geometry.h = h;

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
     window->pending.configure = EINA_FALSE;
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

   if (window->surface)
     {
        wl_surface_attach(window->surface, NULL, 0, 0);
        wl_surface_commit(window->surface);
     }

   window->configure_serial = 0;
   window->zxdg_configure_ack = NULL;
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
     input = EINA_INLIST_CONTAINER_GET(window->display->inputs, Ecore_Wl2_Input);

   window->moving = EINA_TRUE;

   if (window->zxdg_toplevel)
     zxdg_toplevel_v6_move(window->zxdg_toplevel, input->wl.seat,
                           window->display->serial);
}

EAPI void
ecore_wl2_window_resize(Ecore_Wl2_Window *window, Ecore_Wl2_Input *input, int location)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   EINA_SAFETY_ON_NULL_RETURN(window->display->inputs);
   if (!input)
     input = EINA_INLIST_CONTAINER_GET(window->display->inputs, Ecore_Wl2_Input);

   if (window->zxdg_toplevel)
     zxdg_toplevel_v6_resize(window->zxdg_toplevel, input->wl.seat,
                             window->display->serial, location);
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
                                    window->geometry.w,
                                    window->geometry.h, &states);
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
   int nx = 0, ny = 0, nw = 0, nh = 0;

   EINA_SAFETY_ON_NULL_RETURN(window);

   if ((x == 0) && (y == 0) && (w == 0) && (h == 0))
     {
        if (window->surface)
          wl_surface_set_opaque_region(window->surface, NULL);
        return;
     }

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
   window->opaque_set = EINA_TRUE;

   if (!window->surface) return;

   region = wl_compositor_create_region(window->display->wl.compositor);
   if (!region)
     {
        ERR("Failed to create opaque region");
        return;
     }

   wl_region_add(region, window->opaque.x, window->opaque.y,
                 window->opaque.w, window->opaque.h);
   wl_surface_set_opaque_region(window->surface, region);
   wl_region_destroy(region);
}

EAPI void
ecore_wl2_window_input_region_set(Ecore_Wl2_Window *window, int x, int y, int w, int h)
{
   struct wl_region *region;
   int nx = 0, ny = 0, nw = 0, nh = 0;

   EINA_SAFETY_ON_NULL_RETURN(window);

   if ((x == 0) && (y == 0) && (w == 0) && (h == 0))
     {
        if (window->surface)
          wl_surface_set_input_region(window->surface, NULL);
        return;
     }

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
   window->input_set = EINA_TRUE;

   if (!window->surface) return;
   if (window->type == ECORE_WL2_WINDOW_TYPE_DND) return;

   region = wl_compositor_create_region(window->display->wl.compositor);
   if (!region)
     {
        ERR("Failed to create input region");
        return;
     }

   wl_region_add(region, window->input_rect.x, window->input_rect.y,
                 window->input_rect.w, window->input_rect.h);
   wl_surface_set_input_region(window->surface, region);
   wl_region_destroy(region);
}

EAPI Eina_Bool
ecore_wl2_window_maximized_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   return window->maximized;
}

EAPI void
ecore_wl2_window_maximized_set(Ecore_Wl2_Window *window, Eina_Bool maximized)
{
   Eina_Bool prev;

   EINA_SAFETY_ON_NULL_RETURN(window);

   prev = window->maximized;
   maximized = !!maximized;
   if (prev == maximized) return;

   if (!ecore_wl2_window_shell_surface_exists(window))
     window->maximized = maximized;

   if (maximized)
     {
        window->saved = window->geometry;

        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_set_maximized(window->zxdg_toplevel);
     }
   else
     {
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_unset_maximized(window->zxdg_toplevel);
     }
}

EAPI Eina_Bool
ecore_wl2_window_fullscreen_get(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   return window->fullscreen;
}

EAPI void
ecore_wl2_window_fullscreen_set(Ecore_Wl2_Window *window, Eina_Bool fullscreen)
{
   Eina_Bool prev;

   EINA_SAFETY_ON_NULL_RETURN(window);

   prev = window->fullscreen;
   fullscreen = !!fullscreen;
   if (prev == fullscreen) return;

   if (!ecore_wl2_window_shell_surface_exists(window))
     window->fullscreen = fullscreen;

   if (fullscreen)
     {
        window->saved = window->geometry;

        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_set_fullscreen(window->zxdg_toplevel, NULL);
     }
   else
     {
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_unset_fullscreen(window->zxdg_toplevel);
     }
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

   if (window->zxdg_toplevel)
     zxdg_toplevel_v6_set_title(window->zxdg_toplevel, window->title);
}

EAPI void
ecore_wl2_window_class_set(Ecore_Wl2_Window *window, const char *clas)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   eina_stringshare_replace(&window->class, clas);
   if (!window->class) return;

   if (window->zxdg_toplevel)
     zxdg_toplevel_v6_set_app_id(window->zxdg_toplevel, window->class);
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

   if (window->zxdg_toplevel)
     zxdg_surface_v6_set_window_geometry(window->zxdg_surface, x, y, w, h);
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
   Eina_Bool prev;

   EINA_SAFETY_ON_NULL_RETURN(window);

   prev = window->minimized;
   iconified = !!iconified;
   if (prev == iconified) return;

   window->minimized = iconified;

   if (iconified)
     {
        if (window->zxdg_toplevel)
          zxdg_toplevel_v6_set_minimized(window->zxdg_toplevel);
     }
   else
     {
        if (window->zxdg_toplevel)
          {
             struct wl_array states;
             uint32_t *s;

             wl_array_init(&states);
             s = wl_array_add(&states, sizeof(*s));
             *s = ZXDG_TOPLEVEL_V6_STATE_ACTIVATED;
             _zxdg_toplevel_cb_configure(window, window->zxdg_toplevel,
                                         window->geometry.w,
                                         window->geometry.h, &states);
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
   return window->display;
}

EAPI Eina_Bool
ecore_wl2_window_shell_surface_exists(Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);

   return !!window->zxdg_surface;
}

EAPI Eina_Bool
ecore_wl2_window_activated_get(const Ecore_Wl2_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, EINA_FALSE);
   return window->focused;
}

EAPI Ecore_Wl2_Output *
ecore_wl2_window_output_find(Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Output *out;
   Eina_Inlist *tmp;
   int x = 0, y = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(window, NULL);

   x = window->geometry.x;
   y = window->geometry.y;

   EINA_INLIST_FOREACH_SAFE(window->display->outputs, tmp, out)
     {
        int ox, oy, ow, oh;

        ox = out->geometry.x;
        oy = out->geometry.y;

        switch (out->transform)
          {
           case WL_OUTPUT_TRANSFORM_90:
           case WL_OUTPUT_TRANSFORM_270:
           case WL_OUTPUT_TRANSFORM_FLIPPED_90:
           case WL_OUTPUT_TRANSFORM_FLIPPED_270:
             ow = out->geometry.h;
             oh = out->geometry.w;
             break;
           default:
             ow = out->geometry.w;
             oh = out->geometry.h;
             break;
          }

        if (((x >= ox) && (x < ow)) && ((y >= oy) && (y < oh)))
          return out;
     }

   return NULL;
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
   if ((win->surface) && (win->display->wl.efl_aux_hints))
     efl_aux_hints_add_aux_hint(win->display->wl.efl_aux_hints, win->surface, id, hint, val);
}

EAPI void
ecore_wl2_window_aux_hint_change(Ecore_Wl2_Window *win, int id, const char *val)
{
   if (!win) return;
   if ((win->surface) && (win->display->wl.efl_aux_hints))
     efl_aux_hints_change_aux_hint(win->display->wl.efl_aux_hints, win->surface, id, val);
}

EAPI void
ecore_wl2_window_aux_hint_del(Ecore_Wl2_Window *win, int id)
{
   if (!win) return;
   if ((win->surface) && (win->display->wl.efl_aux_hints))
     efl_aux_hints_del_aux_hint(win->display->wl.efl_aux_hints, win->surface, id);
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
   if ((window->aspect.aspect == aspect) &&
     (window->aspect.w == w) &&
     (window->aspect.h == h)) return;
   window->aspect.w = w;
   window->aspect.h = h;
   window->aspect.aspect = aspect;
   window->aspect.set = 1;
   if (window->display->wl.efl_hints && window->zxdg_toplevel)
     efl_hints_set_aspect(window->display->wl.efl_hints, window->zxdg_toplevel, w, h, aspect);
}
