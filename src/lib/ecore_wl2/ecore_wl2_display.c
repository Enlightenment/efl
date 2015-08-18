#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

static Eina_Bool
_cb_create_data(void *data, Ecore_Fd_Handler *hdl)
{
   Ecore_Wl2_Display *ewd;
   struct wl_event_loop *loop;
   /* int ret = 0; */

   ewd = data;

   if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_ERROR))
     {
        /* TODO: handle error case */
        return ECORE_CALLBACK_CANCEL;
     }

   loop = wl_display_get_event_loop(ewd->wl.display);
   wl_event_loop_dispatch(loop, -1);
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

   /* if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_READ)) */
   ret = wl_display_dispatch(ewd->wl.display);

   if ((ret < 0) && ((errno != EAGAIN) && (errno != EINVAL)))
     {
        /* TODO: handle error case */
        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

EAPI Ecore_Wl2_Display *
ecore_wl2_display_create(void)
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

   ewd->name = wl_display_add_socket_auto(ewd->wl.display);
   if (!ewd->name)
     {
        ERR("Failed to add display socket: %m");
        goto socket_err;
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

   /* allocate space for display structure */
   ewd = calloc(1, sizeof(Ecore_Wl2_Display));
   if (!ewd) return NULL;

   /* try to connect to wayland display with this name */
   ewd->wl.display = wl_display_connect(name);
   if (!ewd->wl.display)
     {
        ERR("Could not connect to display %s: %m", name);
        free(ewd);
        return NULL;
     }

   ewd->fd_hdl =
     ecore_main_fd_handler_add(wl_display_get_fd(ewd->wl.display),
                               ECORE_FD_READ | ECORE_FD_ERROR,
                               _cb_connect_data, ewd, NULL, NULL);

   return ewd;
}
