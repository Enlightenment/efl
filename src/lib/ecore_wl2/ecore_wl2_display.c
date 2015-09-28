#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

static void
_xdg_shell_cb_ping(void *data EINA_UNUSED, struct xdg_shell *shell, uint32_t serial)
{
   xdg_shell_pong(shell, serial);
}

static const struct xdg_shell_listener _xdg_shell_listener =
{
   _xdg_shell_cb_ping
};

static void
_cb_global_event_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Global *ev;

   ev = event;
   eina_stringshare_del(ev->interface);
   free(ev);
}

static void
_cb_global_add(void *data, struct wl_registry *registry, unsigned int id, const char *interface, unsigned int version)
{
   Ecore_Wl2_Display *ewd;
   Ecore_Wl2_Event_Global *ev;

   ewd = data;

   /* test to see if we have already added this global to our hash */
   if (!eina_hash_find(ewd->globals, &id))
     {
        Ecore_Wl2_Global *global;

        /* allocate space for new global */
        global = calloc(1, sizeof(Ecore_Wl2_Global));
        if (!global) return;

        global->id = id;
        global->interface = eina_stringshare_add(interface);
        global->version = version;

        /* add this global to our hash */
        if (!eina_hash_add(ewd->globals, &global->id, global))
          {
             eina_stringshare_del(global->interface);
             free(global);
          }
     }

   if (!strcmp(interface, "wl_compositor"))
     {
        ewd->wl.compositor =
          wl_registry_bind(registry, id, &wl_compositor_interface, 3);
     }
   else if (!strcmp(interface, "wl_subcompositor"))
     {
        ewd->wl.subcompositor =
          wl_registry_bind(registry, id, &wl_subcompositor_interface, 1);
     }
   else if (!strcmp(interface, "wl_shm"))
     {
        ewd->wl.shm =
          wl_registry_bind(registry, id, &wl_shm_interface, 1);
     }
   else if (!strcmp(interface, "wl_data_device_manager"))
     {
        ewd->wl.data_device_manager =
          wl_registry_bind(registry, id, &wl_data_device_manager_interface, 1);
     }
   else if (!strcmp(interface, "wl_shell"))
     {
        ewd->wl.wl_shell =
          wl_registry_bind(registry, id, &wl_shell_interface, 1);
     }
   else if ((!strcmp(interface, "xdg_shell")) &&
            (!getenv("EFL_WAYLAND_DONT_USE_XDG_SHELL")))
     {
        ewd->wl.xdg_shell =
          wl_registry_bind(registry, id, &xdg_shell_interface, 1);
        xdg_shell_use_unstable_version(ewd->wl.xdg_shell, XDG_VERSION);
        xdg_shell_add_listener(ewd->wl.xdg_shell, &_xdg_shell_listener, NULL);
     }
   else if (!strcmp(interface, "wl_output"))
     _ecore_wl2_output_add(ewd, id);
   else if (!strcmp(interface, "wl_seat"))
     _ecore_wl2_input_add(ewd, id);

   /* allocate space for event structure */
   ev = calloc(1, sizeof(Ecore_Wl2_Event_Global));
   if (!ev) return;

   ev->id = id;
   ev->version = version;
   ev->interface = eina_stringshare_add(interface);

   /* raise an event saying a new global has been added */
   ecore_event_add(ECORE_WL2_EVENT_GLOBAL_ADDED, ev,
                   _cb_global_event_free, NULL);
}

static void
_cb_global_remove(void *data, struct wl_registry *registry EINA_UNUSED, unsigned int id)
{
   Ecore_Wl2_Display *ewd;
   Ecore_Wl2_Global *global;
   Ecore_Wl2_Event_Global *ev;

   ewd = data;

   /* try to find this global in our hash */
   global = eina_hash_find(ewd->globals, &id);
   if (!global) return;

   /* allocate space for event structure */
   ev = calloc(1, sizeof(Ecore_Wl2_Event_Global));
   if (!ev) return;

   ev->id = id;
   ev->version = global->version;
   ev->interface = eina_stringshare_add(global->interface);

   /* raise an event saying a global has been removed */
   ecore_event_add(ECORE_WL2_EVENT_GLOBAL_REMOVED, ev,
                   _cb_global_event_free, NULL);

   /* delete this global from our hash */
   eina_hash_del_by_key(ewd->globals, &id);
}

static const struct wl_registry_listener _registry_listener =
{
   _cb_global_add,
   _cb_global_remove
};

static Eina_Bool
_cb_create_data(void *data, Ecore_Fd_Handler *hdl)
{
   Ecore_Wl2_Display *ewd;
   struct wl_event_loop *loop;

   ewd = data;

   if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_ERROR))
     {
        /* TODO: handle error case */
        return ECORE_CALLBACK_CANCEL;
     }

   loop = wl_display_get_event_loop(ewd->wl.display);
   wl_event_loop_dispatch(loop, 0);
   wl_display_flush_clients(ewd->wl.display);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_cb_connect_data(void *data, Ecore_Fd_Handler *hdl)
{
   Ecore_Wl2_Display *ewd;
   int ret = 0;

   ewd = data;

   if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_ERROR))
     {
        /* TODO: handle error case */
        return ECORE_CALLBACK_CANCEL;
     }

   if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_READ))
     ret = wl_display_dispatch(ewd->wl.display);
   else if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_WRITE))
     {
        ret = wl_display_flush(ewd->wl.display);
        if (ret == 0)
          ecore_main_fd_handler_active_set(hdl, ECORE_FD_READ);
     }

   if ((ret < 0) && ((errno != EAGAIN) && (errno != EINVAL)))
     {
        /* TODO: handle error case */
        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static void
_cb_globals_hash_del(void *data)
{
   Ecore_Wl2_Global *global;

   global = data;

   eina_stringshare_del(global->interface);

   free(global);
}

static void
_cb_sync_done(void *data, struct wl_callback *cb, uint32_t serial EINA_UNUSED)
{
   Ecore_Wl2_Display *ewd;

   ewd = data;
   ewd->sync_done = EINA_TRUE;

   wl_callback_destroy(cb);
}

static const struct wl_callback_listener _sync_listener =
{
   _cb_sync_done
};

static void
_animator_tick_cb_begin(void *data)
{
   Ecore_Wl2_Display *display;
   Ecore_Wl2_Window *window;

   display = data;
   if (!display) return;

   EINA_INLIST_FOREACH(display->windows, window)
     _ecore_wl2_window_animator_add(window);
}

static void
_animator_tick_cb_end(void *data)
{
   Ecore_Wl2_Display *display;

   display = data;
   if (!display) return;

   _ecore_wl2_window_animator_end();
}

static void
_ecore_wl2_display_cleanup(Ecore_Wl2_Display *ewd)
{
   Ecore_Wl2_Output *output;
   Eina_Inlist *tmp;

   /* free each output */
   EINA_INLIST_FOREACH_SAFE(ewd->outputs, tmp, output)
     _ecore_wl2_output_del(output);

   if (ewd->xkb_context) xkb_context_unref(ewd->xkb_context);

   wl_registry_destroy(wl_display_get_registry(ewd->wl.display));

   if (ewd->fd_hdl) ecore_main_fd_handler_del(ewd->fd_hdl);

   eina_hash_free(ewd->globals);
}

Ecore_Wl2_Window *
_ecore_wl2_display_window_surface_find(Ecore_Wl2_Display *display, struct wl_surface *wl_surface)
{
   Ecore_Wl2_Window *window;

   if ((!display) || (!wl_surface)) return NULL;

   EINA_INLIST_FOREACH(display->windows, window)
     {
        if ((window->surface) &&
            (window->surface == wl_surface))
          return window;
     }

   return NULL;
}

EAPI Ecore_Wl2_Display *
ecore_wl2_display_create(const char *name)
{
   Ecore_Wl2_Display *ewd;
   struct wl_event_loop *loop;

   /* allocate space for display structure */
   ewd = calloc(1, sizeof(Ecore_Wl2_Display));
   if (!ewd) return NULL;

   /* try to create new wayland display */
   ewd->wl.display = wl_display_create();
   if (!ewd->wl.display)
     {
        ERR("Could not create wayland display: %m");
        goto create_err;
     }

   if (!name)
     {
        ewd->name = wl_display_add_socket_auto(ewd->wl.display);
        if (!ewd->name)
          {
             ERR("Failed to add display socket: %m");
             goto socket_err;
          }
     }
   else
     {
        ewd->name = strdup(name);
        if (wl_display_add_socket(ewd->wl.display, name))
          {
             ERR("Failed to add display socket: %m");
             goto socket_err;
          }
     }

   loop = wl_display_get_event_loop(ewd->wl.display);

   ewd->fd_hdl =
     ecore_main_fd_handler_add(wl_event_loop_get_fd(loop),
                               ECORE_FD_READ | ECORE_FD_ERROR,
                               _cb_create_data, ewd, NULL, NULL);

   return ewd;

socket_err:
   wl_display_destroy(ewd->wl.display);

create_err:
   free(ewd);
   return NULL;
}

EAPI Ecore_Wl2_Display *
ecore_wl2_display_connect(const char *name)
{
   Ecore_Wl2_Display *ewd;
   struct wl_callback *cb;

   /* allocate space for display structure */
   ewd = calloc(1, sizeof(Ecore_Wl2_Display));
   if (!ewd) return NULL;

   ewd->globals = eina_hash_int32_new(_cb_globals_hash_del);

   /* try to connect to wayland display with this name */
   ewd->wl.display = wl_display_connect(name);
   if (!ewd->wl.display)
     {
        ERR("Could not connect to display %s: %m", name);
        goto connect_err;
     }

   ewd->fd_hdl =
     ecore_main_fd_handler_add(wl_display_get_fd(ewd->wl.display),
                               ECORE_FD_READ | ECORE_FD_WRITE | ECORE_FD_ERROR,
                               _cb_connect_data, ewd, NULL, NULL);

   wl_registry_add_listener(wl_display_get_registry(ewd->wl.display),
                            &_registry_listener, ewd);

   ewd->xkb_context = xkb_context_new(0);
   if (!ewd->xkb_context) goto context_err;

   /* NB: If we are connecting (as a client), then we will need to setup
    * a callback for display_sync and wait for it to complete. There is no
    * other option here as we need the compositor, shell, etc, to be setup
    * before we can allow a user to make use of the API functions we provide */
   cb = wl_display_sync(ewd->wl.display);
   wl_callback_add_listener(cb, &_sync_listener, ewd);

   while (!ewd->sync_done)
     wl_display_dispatch(ewd->wl.display);

   return ewd;

context_err:
   ecore_main_fd_handler_del(ewd->fd_hdl);
   wl_registry_destroy(wl_display_get_registry(ewd->wl.display));
   wl_display_disconnect(ewd->wl.display);

connect_err:
   eina_hash_free(ewd->globals);
   free(ewd);
   return NULL;
}

EAPI void
ecore_wl2_display_disconnect(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN(display);
   _ecore_wl2_display_cleanup(display);
   wl_display_disconnect(display->wl.display);
}

EAPI void
ecore_wl2_display_destroy(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN(display);
   _ecore_wl2_display_cleanup(display);
   wl_display_destroy(display->wl.display);
}

EAPI void
ecore_wl2_display_terminate(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN(display);
   wl_display_terminate(display->wl.display);
}

EAPI struct wl_display *
ecore_wl2_display_get(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);
   return display->wl.display;
}

EAPI struct wl_shm *
ecore_wl2_display_shm_get(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);
   return display->wl.shm;
}

EAPI Eina_Iterator *
ecore_wl2_display_globals_get(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);
   return eina_hash_iterator_data_new(display->globals);
}

EAPI Eina_Bool
ecore_wl2_display_animator_source_set(Ecore_Wl2_Display *display, Ecore_Animator_Source source)
{
   switch (source)
     {
      case ECORE_ANIMATOR_SOURCE_CUSTOM:
        ecore_animator_custom_source_tick_begin_callback_set
          (_animator_tick_cb_begin, display);
        ecore_animator_custom_source_tick_end_callback_set
          (_animator_tick_cb_end, display);
        break;
      case ECORE_ANIMATOR_SOURCE_TIMER:
        ecore_animator_custom_source_tick_begin_callback_set(NULL, NULL);
        ecore_animator_custom_source_tick_end_callback_set(NULL, NULL);
        break;
     }

   return EINA_TRUE;
}

EAPI void
ecore_wl2_display_screen_size_get(Ecore_Wl2_Display *display, int *w, int *h)
{
   Ecore_Wl2_Output *output;
   int ow = 0, oh = 0;

   EINA_SAFETY_ON_NULL_RETURN(display);

   if (w) *w = 0;
   if (h) *h = 0;

   EINA_INLIST_FOREACH(display->outputs, output)
     {
        switch (output->transform)
          {
           case WL_OUTPUT_TRANSFORM_90:
           case WL_OUTPUT_TRANSFORM_270:
           case WL_OUTPUT_TRANSFORM_FLIPPED_90:
           case WL_OUTPUT_TRANSFORM_FLIPPED_270:
             ow += output->geometry.h;
             oh += output->geometry.w;
             break;
           default:
             ow += output->geometry.w;
             oh += output->geometry.h;
             break;
          }
     }

   if (w) *w = ow;
   if (h) *h = oh;
}
