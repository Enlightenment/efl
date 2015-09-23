#include "ecore_buffer_con.h"

#define DEBUG  1

typedef struct _Ecore_Buffer_Con Ecore_Buffer_Con;

struct _Ecore_Buffer_Con
{
   struct wl_display *display;
   struct wl_registry *registry;
   struct bq_mgr *bq_mgr;
   Ecore_Fd_Handler *fd_hdl;
   Ecore_Idle_Enterer *idle_enterer;
   int fd;
   Eina_Bool init_done;
};

static Eina_Bool   _connection_fatal_error = EINA_FALSE;
Ecore_Buffer_Con  *_connection = NULL;

static void
_ecore_buffer_con_cb_registry_global(void *data, struct wl_registry *wl_registry, uint32_t id, const char *interface, uint32_t version)
{
   Ecore_Buffer_Con *conn = data;

   DBG("Added Wl Global Registry - name %d interface %s version %d",
       id, interface, version);

   if (!strncmp(interface, "bq_mgr", strlen("bq_mgr")))
     {
        conn->bq_mgr =
           wl_registry_bind(wl_registry, id, &bq_mgr_interface, 1);
     }
}

static void
_ecore_buffer_con_cb_registry_global_remove(void *data EINA_UNUSED, struct wl_registry *wl_registry EINA_UNUSED, uint32_t name EINA_UNUSED)
{
   DBG("Removed Wl Global Registry - name %d", name);
}

static void
_ecore_buffer_con_signal_exit_free(void *data EINA_UNUSED, void *event)
{
   free(event);
}

static void
_ecore_buffer_con_signal_exit(void)
{
   Ecore_Event_Signal_Exit *ev;

   if (!(ev = calloc(1, sizeof(Ecore_Event_Signal_Exit))))
     return;

   ev->quit = 1;
   ecore_event_add(ECORE_EVENT_SIGNAL_EXIT, ev,
                   _ecore_buffer_con_signal_exit_free, NULL);
}

static Eina_Bool
_ecore_buffer_con_cb_idle_enterer(void *data)
{
   Ecore_Buffer_Con *conn;
   int ret = 0;

   if (_connection_fatal_error) return ECORE_CALLBACK_CANCEL;

   if (!(conn = data)) return ECORE_CALLBACK_RENEW;

   ret = wl_display_get_error(conn->display);
   if (ret < 0) goto err;

   ret = wl_display_flush(conn->display);
   if ((ret < 0) && (errno == EAGAIN))
     ecore_main_fd_handler_active_set(conn->fd_hdl,
                                      (ECORE_FD_READ | ECORE_FD_WRITE));

   ret = wl_display_dispatch_pending(conn->display);
   if (ret < 0) goto err;

   return ECORE_CALLBACK_RENEW;

err:
   if ((ret < 0) && ((errno != EAGAIN) && (errno != EINVAL)))
     {
        _connection_fatal_error = EINA_TRUE;

        /* raise exit signal */
        _ecore_buffer_con_signal_exit();

        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_buffer_con_cb_fd_handle(void *data, Ecore_Fd_Handler *hdl)
{
   Ecore_Buffer_Con *conn = data;
   int ret = 0;

   if (_connection_fatal_error) return ECORE_CALLBACK_CANCEL;
   if (!conn) return ECORE_CALLBACK_RENEW;

   if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_ERROR))
     {
        ERR("Received error on wayland display fd");
        _connection_fatal_error = EINA_TRUE;
        _ecore_buffer_con_signal_exit();

        return ECORE_CALLBACK_CANCEL;
     }

   if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_READ))
     ret = wl_display_dispatch(conn->display);
   else if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_WRITE))
     {
        ret = wl_display_flush(conn->display);
        if (ret == 0)
          ecore_main_fd_handler_active_set(hdl, ECORE_FD_READ);
     }

   if ((ret < 0) && ((errno != EAGAIN) && (errno != EINVAL)))
     {
        _connection_fatal_error = EINA_TRUE;

        /* raise exit signal */
        _ecore_buffer_con_signal_exit();

        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_buffer_con_init_callback(void *data, struct wl_callback *callback, uint32_t serial EINA_UNUSED)
{
   Ecore_Buffer_Con *conn = data;

   DBG("Queue Server Connected");

   if (!conn)
     return;

   wl_callback_destroy(callback);
   conn->init_done = EINA_TRUE;
}

static const struct wl_callback_listener _ecore_buffer_con_init_sync_listener =
{
   _ecore_buffer_con_init_callback
};

struct wl_registry_listener _ecore_buffer_registry_listener =
{
   _ecore_buffer_con_cb_registry_global,
   _ecore_buffer_con_cb_registry_global_remove
};

Eina_Bool
_ecore_buffer_con_init(void)
{
   struct wl_callback *callback;
   const char *name = "bq_mgr_daemon";

   DBG("Ecore_Buffer_Con Init - name %s", name);

   _connection = calloc(1, sizeof(Ecore_Buffer_Con));
   if (!_connection)
     {
        ERR("Failed to allocation");
        return EINA_FALSE;
     }

   if (!(_connection->display = wl_display_connect(name)))
     {
        ERR("Failed to connect to Queue Server");
        goto err_connect;
     }

   _connection->fd = wl_display_get_fd(_connection->display);
   _connection->fd_hdl =
      ecore_main_fd_handler_add(_connection->fd,
                                ECORE_FD_READ | ECORE_FD_WRITE | ECORE_FD_ERROR,
                                _ecore_buffer_con_cb_fd_handle,
                                _connection, NULL, NULL);

   _connection->idle_enterer =
      ecore_idle_enterer_add(_ecore_buffer_con_cb_idle_enterer, _connection);

   if (!(_connection->registry = wl_display_get_registry(_connection->display)))
     goto err_get_registry;

   wl_registry_add_listener(_connection->registry,
                            &_ecore_buffer_registry_listener, _connection);

   _connection->init_done = EINA_FALSE;
   callback = wl_display_sync(_connection->display);
   wl_callback_add_listener(callback, &_ecore_buffer_con_init_sync_listener,
                            _connection);

   return EINA_TRUE;;
err_get_registry:
   wl_display_disconnect(_connection->display);
err_connect:
   free(_connection);
   return EINA_FALSE;
}

void
_ecore_buffer_con_shutdown(void)
{
   if (!_connection) return;

   DBG("Ecore_Buffer_Con Shutdown");

   if (_connection->fd_hdl)
     ecore_main_fd_handler_del(_connection->fd_hdl);

   if (_connection->idle_enterer)
     ecore_idle_enterer_del(_connection->idle_enterer);

   if (_connection->bq_mgr)
     bq_mgr_destroy(_connection->bq_mgr);

   if (_connection->display)
     wl_display_disconnect(_connection->display);

   free(_connection);
   _connection = NULL;
}

struct bq_provider *
_ecore_buffer_con_provider_create(const char *name)
{
   return bq_mgr_create_provider(_connection->bq_mgr, name);
}

struct bq_consumer *
_ecore_buffer_con_consumer_create(const char *name, int queue_size, int w, int h)
{
   return bq_mgr_create_consumer(_connection->bq_mgr, name, queue_size, w, h);
}

void
_ecore_buffer_con_init_wait(void)
{
   int ret;

   while (!_connection->init_done)
     {
        ret = wl_display_dispatch(_connection->display);
        if ((ret < 0) && ((errno != EAGAIN) && (errno != EINVAL)))
          {
             /* raise exit signal */
             ERR("Wayland socket error: %s", strerror(errno));
             abort();
             break;
          }
     }
}
