#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

#include "linux-dmabuf-unstable-v1-client-protocol.h"
#include "efl-hints-client-protocol.h"

static Eina_Hash *_server_displays = NULL;
static Eina_Hash *_client_displays = NULL;

static Eina_Bool _cb_connect_idle(void *data);
static Eina_Bool _cb_connect_data(void *data, Ecore_Fd_Handler *hdl);
static Eina_Bool _ecore_wl2_display_connect(Ecore_Wl2_Display *ewd, Eina_Bool sync);

void
_display_event_free(void *d, void *event)
{
   ecore_wl2_display_disconnect(d);
   free(event);
}

static void
_ecore_wl2_display_event(Ecore_Wl2_Display *ewd, int event)
{
   Ecore_Wl2_Event_Connect *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Connect));
   EINA_SAFETY_ON_NULL_RETURN(ev);
   ev->display = ewd;
   ewd->refs++;
   ecore_event_add(event, ev, _display_event_free, ewd);
}

static void
_ecore_wl2_display_signal_exit(void)
{
   Ecore_Event_Signal_Exit *ev;

   ev = calloc(1, sizeof(Ecore_Event_Signal_Exit));
   if (!ev) return;

   ev->quit = EINA_TRUE;
   ecore_event_add(ECORE_EVENT_SIGNAL_EXIT, ev, NULL, NULL);
}

static void
_dmabuf_cb_format(void *data EINA_UNUSED, struct zwp_linux_dmabuf_v1 *dmabuf EINA_UNUSED, uint32_t format EINA_UNUSED)
{
   /* It would be awfully nice if this actually happened */
};

static const struct zwp_linux_dmabuf_v1_listener _dmabuf_listener =
{
   _dmabuf_cb_format
};

static void
_zxdg_shell_cb_ping(void *data EINA_UNUSED, struct zxdg_shell_v6 *shell, uint32_t serial)
{
   zxdg_shell_v6_pong(shell, serial);
}

static const struct zxdg_shell_v6_listener _zxdg_shell_listener =
{
   _zxdg_shell_cb_ping,
};

static void
_session_recovery_create_uuid(void *data EINA_UNUSED, struct zwp_e_session_recovery *session_recovery EINA_UNUSED, struct wl_surface *surface, const char *uuid)
{
   Ecore_Wl2_Window *win;

   /* surface may have been destroyed */
   if (!surface) return;
   win = wl_surface_get_user_data(surface);

   eina_stringshare_replace(&win->uuid, uuid);
}

static const struct zwp_e_session_recovery_listener _session_listener =
{
   _session_recovery_create_uuid,
};

static void
_aux_hints_supported_aux_hints(void *data, struct efl_aux_hints *aux_hints EINA_UNUSED, struct wl_surface *surface_resource, struct wl_array *hints, uint32_t num_hints)
{
   Ecore_Wl2_Display *ewd = data;
   struct wl_surface *surface = surface_resource;
   Ecore_Wl2_Window *win = NULL;
   char *p = NULL;
   char **str = NULL;
   const char *hint = NULL;
   unsigned int i = 0;
   Ecore_Wl2_Event_Aux_Hint_Supported *ev;

   if (!surface) return;
   win = _ecore_wl2_display_window_surface_find(ewd, surface_resource);
   if (!win) return;

   p = hints->data;
   str = calloc(num_hints, sizeof(char *));
   if (!str) return;

   while ((const char *)p < ((const char *)hints->data + hints->size))
     {
        str[i] = (char *)eina_stringshare_add(p);
        p += strlen(p) + 1;
        i++;
     }
   for (i = 0; i < num_hints; i++)
     {
        hint = eina_stringshare_add(str[i]);
        win->supported_aux_hints =
               eina_list_append(win->supported_aux_hints, hint);
     }
   if (str)
     {
        for (i = 0; i < num_hints; i++)
          {
             if (str[i])
               {
                  eina_stringshare_del(str[i]);
                  str[i] = NULL;
               }
          }
        free(str);
     }

   if (!(ev = calloc(1, sizeof(Ecore_Wl2_Event_Aux_Hint_Supported)))) return;
   ev->win = win->id;
   ev->display = ewd;
   ewd->refs++;
   ecore_event_add(ECORE_WL2_EVENT_AUX_HINT_SUPPORTED, ev, _display_event_free, ewd);
}

static void
_aux_hints_allowed_aux_hint(void *data, struct efl_aux_hints *aux_hints  EINA_UNUSED, struct wl_surface *surface_resource, int id)
{
   struct wl_surface *surface = surface_resource;
   Ecore_Wl2_Window *win = NULL;
   Ecore_Wl2_Display *ewd = data;
   Ecore_Wl2_Event_Aux_Hint_Allowed *ev;

   if (!surface) return;
   win = _ecore_wl2_display_window_surface_find(ewd, surface_resource);
   if (!win) return;

   if (!(ev = calloc(1, sizeof(Ecore_Wl2_Event_Aux_Hint_Allowed)))) return;
   ev->win = win->id;
   ev->id = id;
   ev->display = ewd;
   ewd->refs++;
   ecore_event_add(ECORE_WL2_EVENT_AUX_HINT_ALLOWED, ev, _display_event_free, ewd);
}

 static void
_cb_aux_message_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Aux_Message *ev;
   char *str;

   ev = event;
   ecore_wl2_display_disconnect(ev->display);
   eina_stringshare_del(ev->key);
   eina_stringshare_del(ev->val);
   EINA_LIST_FREE(ev->options, str)
      eina_stringshare_del(str);
   free(ev);
}

 static void
_aux_hints_aux_message(void *data, struct efl_aux_hints *aux_hints EINA_UNUSED, struct wl_surface *surface_resource, const char *key, const char *val, struct wl_array *options)
{
   Ecore_Wl2_Window *win = NULL;
   Ecore_Wl2_Event_Aux_Message *ev;
   char *p = NULL, *str = NULL;
   Eina_List *opt_list = NULL;
   Ecore_Wl2_Display *ewd = data;

   if (!surface_resource) return;
   win = _ecore_wl2_display_window_surface_find(ewd, surface_resource);
   if (!win) return;

   if (!(ev = calloc(1, sizeof(Ecore_Wl2_Event_Aux_Message)))) return;

   if ((options) && (options->size))
     {
        p = options->data;
        while ((const char *)p < ((const char *)options->data + options->size))
          {
             str = (char *)eina_stringshare_add(p);
             opt_list = eina_list_append(opt_list, str);
             p += strlen(p) + 1;
          }
     }

   ev->win = win->id;
   ev->key = eina_stringshare_add(key);
   ev->val = eina_stringshare_add(val);
   ev->options = opt_list;
   ev->display = ewd;
   ewd->refs++;

   ecore_event_add(ECORE_WL2_EVENT_AUX_MESSAGE, ev, _cb_aux_message_free, NULL);
}

static const struct efl_aux_hints_listener _aux_hints_listener =
{
   _aux_hints_supported_aux_hints,
   _aux_hints_allowed_aux_hint,
   _aux_hints_aux_message,
};

static void
_cb_global_event_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Global *ev;

   ev = event;
   eina_stringshare_del(ev->interface);
   ecore_wl2_display_disconnect(ev->display);
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
   else
     goto event;

   if (!strcmp(interface, "wl_compositor"))
     {
        Ecore_Wl2_Window *window;
        ewd->wl.compositor_version = MIN(version, 4);
        ewd->wl.compositor =
          wl_registry_bind(registry, id, &wl_compositor_interface,
                           ewd->wl.compositor_version);
        EINA_INLIST_FOREACH(ewd->windows, window)
          _ecore_wl2_window_surface_create(window);
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
   else if (!strcmp(interface, "zwp_linux_dmabuf_v1"))
     {
        ewd->wl.dmabuf =
          wl_registry_bind(registry, id, &zwp_linux_dmabuf_v1_interface, 1);
        zwp_linux_dmabuf_v1_add_listener(ewd->wl.dmabuf, &_dmabuf_listener, ewd);
     }
   else if (!strcmp(interface, "wl_data_device_manager"))
     {
        ewd->wl.data_device_manager_version = MIN(version, 3);
        ewd->wl.data_device_manager =
          wl_registry_bind(registry, id, &wl_data_device_manager_interface, ewd->wl.data_device_manager_version);
     }
   else if ((eina_streq(interface, "www")) &&
            (getenv("EFL_WAYLAND_ENABLE_WWW")))
     {
        Ecore_Wl2_Window *window;

        ewd->wl.www = wl_registry_bind(registry, id, &www_interface, 1);
        EINA_INLIST_FOREACH(ewd->windows, window)
          _ecore_wl2_window_www_surface_init(window);
     }
   else if ((!strcmp(interface, "zwp_e_session_recovery")) &&
            (!no_session_recovery))
     {
        ewd->wl.session_recovery =
          wl_registry_bind(registry, id,
                           &zwp_e_session_recovery_interface, 1);
        zwp_e_session_recovery_add_listener(ewd->wl.session_recovery,
                                            &_session_listener, ewd);
     }
   else if (!strcmp(interface, "efl_aux_hints"))
     {
        Ecore_Wl2_Window *window;
        ewd->wl.efl_aux_hints =
          wl_registry_bind(registry, id,
                           &efl_aux_hints_interface, 1);
        efl_aux_hints_add_listener(ewd->wl.efl_aux_hints, &_aux_hints_listener, ewd);
        EINA_INLIST_FOREACH(ewd->windows, window)
          if (window->surface) efl_aux_hints_get_supported_aux_hints(ewd->wl.efl_aux_hints, window->surface);
     }
   else if (!strcmp(interface, "zwp_teamwork"))
     {
        ewd->wl.teamwork =
          wl_registry_bind(registry, id,
                           &zwp_teamwork_interface, EFL_TEAMWORK_VERSION);
     }
   else if (!strcmp(interface, "wl_output"))
     _ecore_wl2_output_add(ewd, id);
   else if (!strcmp(interface, "wl_seat"))
     _ecore_wl2_input_add(ewd, id, version);
   else if (!strcmp(interface, "efl_hints"))
     {
        Ecore_Wl2_Window *window;

        ewd->wl.efl_hints = wl_registry_bind(registry, id, &efl_hints_interface, 1);
        EINA_INLIST_FOREACH(ewd->windows, window)
          if (window->zxdg_toplevel && window->aspect.set)
            efl_hints_set_aspect(window->display->wl.efl_hints, window->zxdg_toplevel,
              window->aspect.w, window->aspect.h, window->aspect.aspect);
     }

event:
   /* allocate space for event structure */
   ev = calloc(1, sizeof(Ecore_Wl2_Event_Global));
   if (!ev) return;

   ev->id = id;
   ev->display = ewd;
   ewd->refs++;
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
   ev->display = ewd;
   ewd->refs++;
   ev->version = global->version;
   ev->interface = eina_stringshare_add(global->interface);

   /* raise an event saying a global has been removed */
   ecore_event_add(ECORE_WL2_EVENT_GLOBAL_REMOVED, ev,
                   _cb_global_event_free, NULL);

   /* delete this global from our hash */
   if (ewd->globals) eina_hash_del_by_key(ewd->globals, &id);
}

static const struct wl_registry_listener _registry_listener =
{
   _cb_global_add,
   _cb_global_remove
};

static Eina_Bool
_cb_create_data(void *data, Ecore_Fd_Handler *hdl EINA_UNUSED)
{
   Ecore_Wl2_Display *ewd = data;
   struct wl_event_loop *loop;

   loop = wl_display_get_event_loop(ewd->wl.display);
   wl_event_loop_dispatch(loop, 0);

   /* wl_display_flush_clients(ewd->wl.display); */

   return ECORE_CALLBACK_RENEW;
}

static void
_cb_create_prepare(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Ecore_Wl2_Display *ewd = data;

   wl_display_flush_clients(ewd->wl.display);
}

static Eina_Bool 
_recovery_timer(Ecore_Wl2_Display *ewd)
{
   if (!_ecore_wl2_display_connect(ewd, 1))
     return EINA_TRUE;

   ewd->recovery_timer = NULL;
   return EINA_FALSE;
}

static void
_ecore_wl2_display_globals_cleanup(Ecore_Wl2_Display *ewd)
{
   if (ewd->wl.session_recovery)
     zwp_e_session_recovery_destroy(ewd->wl.session_recovery);
   if (ewd->wl.www) www_destroy(ewd->wl.www);
   if (ewd->wl.zxdg_shell) zxdg_shell_v6_destroy(ewd->wl.zxdg_shell);
   if (ewd->wl.shm) wl_shm_destroy(ewd->wl.shm);
   if (ewd->wl.data_device_manager)
     wl_data_device_manager_destroy(ewd->wl.data_device_manager);
   if (ewd->wl.compositor) wl_compositor_destroy(ewd->wl.compositor);
   if (ewd->wl.subcompositor) wl_subcompositor_destroy(ewd->wl.subcompositor);
   if (ewd->wl.dmabuf) zwp_linux_dmabuf_v1_destroy(ewd->wl.dmabuf);
   if (ewd->wl.efl_aux_hints) efl_aux_hints_destroy(ewd->wl.efl_aux_hints);
   if (ewd->wl.efl_hints) efl_hints_destroy(ewd->wl.efl_hints);

   if (ewd->wl.registry) wl_registry_destroy(ewd->wl.registry);
}

static void
_recovery_timer_add(Ecore_Wl2_Display *ewd)
{
   Eina_Inlist *tmp, *tmp2;
   Ecore_Wl2_Output *output;
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;

   eina_hash_free_buckets(ewd->globals);
   ecore_idle_enterer_del(ewd->idle_enterer);
   ewd->idle_enterer = NULL;

   ecore_main_fd_handler_del(ewd->fd_hdl);
   ewd->fd_hdl = NULL;

   ewd->shell_done = EINA_FALSE;

   _ecore_wl2_display_globals_cleanup(ewd);

   memset(&ewd->wl, 0, sizeof(ewd->wl));
   EINA_INLIST_FOREACH_SAFE(ewd->inputs, tmp, input)
     _ecore_wl2_input_del(input);

   EINA_INLIST_FOREACH_SAFE(ewd->outputs, tmp, output)
     _ecore_wl2_output_del(output);

   EINA_INLIST_FOREACH_SAFE(ewd->windows, tmp, window)
     {
        Ecore_Wl2_Subsurface *subsurf;

        EINA_INLIST_FOREACH_SAFE(window->subsurfs, tmp2, subsurf)
          _ecore_wl2_subsurf_unmap(subsurf);
        _ecore_wl2_window_semi_free(window);
        window->configure_serial = 0;
        window->zxdg_configure_ack = NULL;
        window->zxdg_set_min_size = NULL;
        window->zxdg_set_max_size = NULL;
     }

   ewd->recovery_timer =
     ecore_timer_add(0.5, (Ecore_Task_Cb)_recovery_timer, ewd);
   _ecore_wl2_display_event(ewd, ECORE_WL2_EVENT_DISCONNECT);
}

static void
_begin_recovery_maybe(Ecore_Wl2_Display *ewd, int code)
{
   if ((_server_displays || (code != EPROTO)) && ewd->wl.session_recovery)// && (errno == EPIPE))
     _recovery_timer_add(ewd);
   else if (!_server_displays)
     {
        ERR("Wayland Socket Error: %s", strerror(errno));
        _ecore_wl2_display_signal_exit();
     }
}

static Eina_Bool
_cb_connect_data(void *data, Ecore_Fd_Handler *hdl)
{
   Ecore_Wl2_Display *ewd = data;
   int ret = 0, code;

   if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_READ))
     {
        ret = wl_display_dispatch(ewd->wl.display);
        code = errno;
        if ((ret < 0) && (code != EAGAIN)) goto err;
     }

   if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_WRITE))
     {
        ret = wl_display_flush(ewd->wl.display);
        code = errno;
        if (ret == 0)
          ecore_main_fd_handler_active_set(hdl, ECORE_FD_READ);

        if ((ret < 0) && (code != EAGAIN)) goto err;
     }

   return ECORE_CALLBACK_RENEW;

err:
   ewd->fd_hdl = NULL;
   _begin_recovery_maybe(ewd, code);

   return ECORE_CALLBACK_CANCEL;
}

static void
_cb_globals_hash_del(void *data)
{
   Ecore_Wl2_Global *global;

   global = data;

   eina_stringshare_del(global->interface);

   free(global);
}

static Eina_Bool
_cb_connect_idle(void *data)
{
   Ecore_Wl2_Display *ewd = data;
   int ret = 0, code;

   ret = wl_display_get_error(ewd->wl.display);
   code = errno;
   if (ret < 0) goto err;

   ret = wl_display_dispatch_pending(ewd->wl.display);
   code = errno;
   if (ret < 0) goto err;

   ret = wl_display_flush(ewd->wl.display);
   code = errno;
   if ((ret < 0) && (code == EAGAIN))
     ecore_main_fd_handler_active_set(ewd->fd_hdl,
                                      (ECORE_FD_READ | ECORE_FD_WRITE));

   return ECORE_CALLBACK_RENEW;

err:
   if ((ret < 0) && (code != EAGAIN))
     {
        ewd->idle_enterer = NULL;
        _begin_recovery_maybe(ewd, code);

        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static Ecore_Wl2_Global *
_ecore_wl2_global_find(Ecore_Wl2_Display *ewd, const char *interface)
{
   Eina_Iterator *itr;
   Ecore_Wl2_Global *global = NULL, *g = NULL;

   itr = eina_hash_iterator_data_new(ewd->globals);
   if (!itr) return NULL;

   EINA_ITERATOR_FOREACH(itr, g)
     {
        if (!strcmp(g->interface, interface))
          {
             global = g;
             break;
          }
     }

   eina_iterator_free(itr);
   return global;
}

static void
_ecore_wl2_shell_bind(Ecore_Wl2_Display *ewd)
{
   Ecore_Wl2_Global *global = NULL;
   const char **itr;
   const char *shells[] =
     {
        "zxdg_shell_v6",
        NULL
     };

   if (ewd->shell_done) return;

   for (itr = shells; *itr != NULL; itr++)
     {
        global = _ecore_wl2_global_find(ewd, *itr);
        if (!global) continue;
        break;
     }

   if (!global) return;

   if (!strcmp(global->interface, "zxdg_shell_v6"))
     {
        ewd->wl.zxdg_shell =
          wl_registry_bind(ewd->wl.registry, global->id,
                           &zxdg_shell_v6_interface, 1);
        zxdg_shell_v6_add_listener(ewd->wl.zxdg_shell,
                                   &_zxdg_shell_listener, NULL);
        ewd->shell_done = EINA_TRUE;
     }
}

static void
_cb_sync_done(void *data, struct wl_callback *cb, uint32_t serial EINA_UNUSED)
{
   Ecore_Wl2_Event_Sync_Done *ev;
   Ecore_Wl2_Display *ewd;

   ewd = data;
   ewd->sync_done = EINA_TRUE;

   _ecore_wl2_shell_bind(ewd);

   wl_callback_destroy(cb);

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Sync_Done));
   if (!ev) return;

   ev->display = ewd;
   ewd->refs++;
   ecore_event_add(ECORE_WL2_EVENT_SYNC_DONE, ev, _display_event_free, ewd);
}

static const struct wl_callback_listener _sync_listener =
{
   _cb_sync_done
};

static Eina_Bool
_ecore_wl2_display_connect(Ecore_Wl2_Display *ewd, Eina_Bool sync)
{
   struct wl_callback *cb;

   /* try to connect to wayland display with this name */
   ewd->wl.display = wl_display_connect(ewd->name);
   if (!ewd->wl.display) return EINA_FALSE;

   ewd->wl.registry = wl_display_get_registry(ewd->wl.display);
   wl_registry_add_listener(ewd->wl.registry, &_registry_listener, ewd);

   cb = wl_display_sync(ewd->wl.display);
   wl_callback_add_listener(cb, &_sync_listener, ewd);

   if (sync)
     {
        /* NB: If we are connecting (as a client), then we will need to setup
         * a callback for display_sync and wait for it to complete. There is no
         * other option here as we need the compositor, shell, etc, to be setup
         * before we can allow a user to make use of the API functions */
        while (!ewd->sync_done)
          {
             int ret;

             ret = wl_display_dispatch(ewd->wl.display);
             if ((ret < 0) && (errno != EAGAIN))
               {
                  ERR("Received Fatal Error on Wayland Display");

                  wl_registry_destroy(ewd->wl.registry);
                  return EINA_FALSE;
               }
          }
     }

   ewd->fd_hdl =
     ecore_main_fd_handler_add(wl_display_get_fd(ewd->wl.display),
                               ECORE_FD_READ | ECORE_FD_WRITE | ECORE_FD_ERROR,
                               _cb_connect_data, ewd, NULL, NULL);

   ewd->idle_enterer = ecore_idle_enterer_add(_cb_connect_idle, ewd);

   _ecore_wl2_display_event(ewd, ECORE_WL2_EVENT_CONNECT);
   return EINA_TRUE;
}

static void
_ecore_wl2_display_cleanup(Ecore_Wl2_Display *ewd)
{
   Ecore_Wl2_Output *output;
   Ecore_Wl2_Input *input;
   Eina_Inlist *tmp;

   if (ewd->xkb_context) xkb_context_unref(ewd->xkb_context);

   /* free each input */
   EINA_INLIST_FOREACH_SAFE(ewd->inputs, tmp, input)
     _ecore_wl2_input_del(input);

   /* free each output */
   EINA_INLIST_FOREACH_SAFE(ewd->outputs, tmp, output)
     _ecore_wl2_output_del(output);

   if (ewd->idle_enterer) ecore_idle_enterer_del(ewd->idle_enterer);

   if (ewd->fd_hdl) ecore_main_fd_handler_del(ewd->fd_hdl);

   eina_hash_free(ewd->globals);

   _ecore_wl2_display_globals_cleanup(ewd);
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

   if (!_server_displays)
     _server_displays = eina_hash_string_superfast_new(NULL);

   if (name)
     {
        /* someone wants to create a server with a specific display */

        /* check hash of cached server displays for this name */
        ewd = eina_hash_find(_server_displays, name);
        if (ewd) goto found;
     }

   /* allocate space for display structure */
   ewd = calloc(1, sizeof(Ecore_Wl2_Display));
   if (!ewd) return NULL;

   ewd->refs++;
   ewd->pid = getpid();

   /* try to create new wayland display */
   ewd->wl.display = wl_display_create();
   if (!ewd->wl.display)
     {
        ERR("Could not create wayland display");
        goto create_err;
     }

   if (!name)
     {
        const char *n;

        n = wl_display_add_socket_auto(ewd->wl.display);
        if (!n)
          {
             ERR("Failed to add display socket");
             goto socket_err;
          }

        ewd->name = strdup(n);
     }
   else
     {
        if (wl_display_add_socket(ewd->wl.display, name))
          {
             ERR("Failed to add display socket");
             goto socket_err;
          }

        ewd->name = strdup(name);
     }

   setenv("WAYLAND_DISPLAY", ewd->name, 1);
   DBG("WAYLAND_DISPLAY: %s", ewd->name);

   loop = wl_display_get_event_loop(ewd->wl.display);

   ewd->fd_hdl =
     ecore_main_fd_handler_add(wl_event_loop_get_fd(loop),
                               ECORE_FD_READ | ECORE_FD_ERROR,
                               _cb_create_data, ewd, NULL, NULL);

   ecore_main_fd_handler_prepare_callback_set(ewd->fd_hdl,
                                              _cb_create_prepare, ewd);

   /* add this new server display to hash */
   eina_hash_add(_server_displays, ewd->name, ewd);

   return ewd;

socket_err:
   wl_display_destroy(ewd->wl.display);

create_err:
   free(ewd);
   return NULL;

found:
   ewd->refs++;
   return ewd;
}

Eina_Bool
_ecore_wl2_display_sync_get(void)
{
   return !_server_displays || !eina_hash_population(_server_displays);
}

EAPI Ecore_Wl2_Display *
ecore_wl2_display_connect(const char *name)
{
   Ecore_Wl2_Display *ewd;
   const char *n;
   Eina_Bool hash_create = !_client_displays;

   if (!_client_displays)
     _client_displays = eina_hash_string_superfast_new(NULL);

   if (!name)
     {
        /* client wants to connect to default display */
        n = getenv("WAYLAND_DISPLAY");
        if (!n) n = "wayland-0";

        /* we have a default wayland display */

        /* check hash of cached client displays for this name */
        ewd = eina_hash_find(_client_displays, n);
        if (ewd) goto found;
     }
   else
     {
        /* client wants to connect to specific display */

        /* check hash of cached client displays for this name */
        ewd = eina_hash_find(_client_displays, name);
        if (ewd) goto found;
     }

   /* allocate space for display structure */
   ewd = calloc(1, sizeof(Ecore_Wl2_Display));
   if (!ewd) return NULL;

   ewd->refs++;

   if (name)
     ewd->name = strdup(name);
   else if (n)
     ewd->name = strdup(n);

   ewd->globals = eina_hash_int32_new(_cb_globals_hash_del);

   /* check server display hash and match on pid. If match, skip sync */
   if (!_ecore_wl2_display_connect(ewd, _ecore_wl2_display_sync_get()))
     goto connect_err;

   ewd->xkb_context = xkb_context_new(0);
   if (!ewd->xkb_context) goto context_err;

   /* add this new client display to hash */
   eina_hash_add(_client_displays, ewd->name, ewd);

   return ewd;

context_err:
   ecore_main_fd_handler_del(ewd->fd_hdl);
   wl_registry_destroy(ewd->wl.registry);
   wl_display_disconnect(ewd->wl.display);

connect_err:
   eina_hash_free(ewd->globals);
   free(ewd->name);
   free(ewd);

   if (hash_create)
     {
        eina_hash_free(_client_displays);
        _client_displays = NULL;
     }
   return NULL;

found:
   ewd->refs++;
   return ewd;
}

EAPI void
ecore_wl2_display_disconnect(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN(display);

   --display->refs;
   if (display->refs == 0)
     {
        int ret;

        do
          {
             ret = wl_display_dispatch_pending(display->wl.display);
          } while (ret > 0);

        _ecore_wl2_display_cleanup(display);

        wl_display_disconnect(display->wl.display);

        /* remove this client display from hash */
        eina_hash_del_by_key(_client_displays, display->name);

        free(display->name);
        free(display);
     }
}

EAPI void
ecore_wl2_display_destroy(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN(display);

   --display->refs;
   if (display->refs == 0)
     {
        /* this ensures that things like wl_registry are destroyed
         * before we destroy the actual wl_display */
        _ecore_wl2_display_cleanup(display);

        wl_display_destroy(display->wl.display);

        /* remove this client display from hash */
        eina_hash_del_by_key(_server_displays, display->name);
        ecore_timer_del(display->recovery_timer);

        free(display->name);
        free(display);
     }
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

EAPI void *
ecore_wl2_display_dmabuf_get(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);
   return display->wl.dmabuf;
}

EAPI Eina_Iterator *
ecore_wl2_display_globals_get(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(display->globals, NULL);

   return eina_hash_iterator_data_new(display->globals);
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

EAPI Ecore_Wl2_Window *
ecore_wl2_display_window_find(Ecore_Wl2_Display *display, unsigned int id)
{
   Ecore_Wl2_Window *window;

   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);

   EINA_INLIST_FOREACH(display->windows, window)
     if (window->id == (int)id) return window;

   return NULL;
}

EAPI struct wl_registry *
ecore_wl2_display_registry_get(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);

   return display->wl.registry;
}

EAPI int
ecore_wl2_display_compositor_version_get(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(display, 0);

   return display->wl.compositor_version;
}

EAPI Eina_Iterator *
ecore_wl2_display_inputs_get(Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(display->pid, NULL);
   return eina_inlist_iterator_new(display->inputs);
}

EAPI Ecore_Wl2_Input *
ecore_wl2_display_input_find(const Ecore_Wl2_Display *display, unsigned int id)
{
   Ecore_Wl2_Input *input;

   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(display->pid, NULL);
   EINA_INLIST_FOREACH(display->inputs, input)
     if (input->id == id) return input;
   return NULL;
}

EAPI Ecore_Wl2_Input *
ecore_wl2_display_input_find_by_name(const Ecore_Wl2_Display *display, const char *name)
{
   Ecore_Wl2_Input *input;

   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(display->pid, NULL);
   EINA_INLIST_FOREACH(display->inputs, input)
     if (eina_streq(input->name, name)) return input;
   return NULL;
}

EAPI Eina_Bool
ecore_wl2_display_sync_is_done(const Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(display, EINA_FALSE);
   return display->sync_done;
}

EAPI const char *
ecore_wl2_display_name_get(const Ecore_Wl2_Display *display)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);
   return display->name;
}
