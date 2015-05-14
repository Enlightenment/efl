#include "ecore_buffer_connection.h"

struct _Ecore_Buffer_Connection
{
   struct wl_display *display;
   struct wl_registry *registry;
   struct bq_mgr *bq_mgr;

   int fd;
   Ecore_Fd_Handler *fd_hdl;
   Ecore_Idle_Enterer *idle_enterer;
   Eina_Bool init_done;
};

int _ecore_buffer_queue_log_dom = -1;
Ecore_Buffer_Connection *_ecore_buffer_connection = NULL;

static int _ecore_buffer_queue_init_count = 0;
static Eina_Bool _connection_fatal_error = EINA_FALSE;

static void       _ecore_buffer_connection_init_callback(void *data, struct wl_callback *callback, uint32_t serial EINA_UNUSED);
static void       _ecore_buffer_connection_signal_exit_free(void *data EINA_UNUSED, void *event);
static void       _ecore_buffer_connection_signal_exit(void);
static Eina_Bool  _ecore_buffer_connection_cb_fd_handle(void *data, Ecore_Fd_Handler *hdl);
static Eina_Bool  _ecore_buffer_connection_cb_idle_enterer(void *data);
static void       _ecore_buffer_connection_cb_registry_global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version);
static void       _ecore_buffer_connection_cb_registry_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name);

static const struct wl_callback_listener _ecore_buffer_connection_init_sync_listener =
{
   _ecore_buffer_connection_init_callback
};

struct wl_registry_listener _ecore_buffer_registry_listener =
{
   _ecore_buffer_connection_cb_registry_global,
   _ecore_buffer_connection_cb_registry_global_remove
};

EAPI int
ecore_buffer_queue_init(void)
{
   struct wl_callback *callback;

   if (++_ecore_buffer_queue_init_count != 1)
     return _ecore_buffer_queue_init_count;

   _ecore_buffer_queue_log_dom =
      eina_log_domain_register("ecore_buffer_queue", EINA_COLOR_GREEN);

   if (_ecore_buffer_queue_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: ecore_buffer_queue");
        return 0;
     }

   DBG("Ecore_Buffer_Connection Init");

   _ecore_buffer_connection = calloc(sizeof(Ecore_Buffer_Connection), 1);
   if (!_ecore_buffer_connection)
     return --_ecore_buffer_queue_init_count;

   if (!(_ecore_buffer_connection->display = wl_display_connect(NULL)))
     {
        ERR("Failed to connect to Queue Server");
        goto err_connect;
     }

   _ecore_buffer_connection->fd =
      wl_display_get_fd(_ecore_buffer_connection->display);

   _ecore_buffer_connection->fd_hdl =
      ecore_main_fd_handler_add(_ecore_buffer_connection->fd,
                                ECORE_FD_READ | ECORE_FD_WRITE | ECORE_FD_ERROR,
                                _ecore_buffer_connection_cb_fd_handle,
                                _ecore_buffer_connection, NULL, NULL);

   _ecore_buffer_connection->idle_enterer =
      ecore_idle_enterer_add(_ecore_buffer_connection_cb_idle_enterer,
                             _ecore_buffer_connection);

   _ecore_buffer_connection->registry =
      wl_display_get_registry(_ecore_buffer_connection->display);
   if (!_ecore_buffer_connection->registry)
     goto err_get_registry;

   wl_registry_add_listener(_ecore_buffer_connection->registry,
                            &_ecore_buffer_registry_listener,
                            _ecore_buffer_connection);

   _ecore_buffer_connection->init_done = EINA_FALSE;
   callback = wl_display_sync(_ecore_buffer_connection->display);
   wl_callback_add_listener(callback,
                            &_ecore_buffer_connection_init_sync_listener,
                            _ecore_buffer_connection);

   return _ecore_buffer_queue_init_count;

err_get_registry:
   eina_log_domain_unregister(_ecore_buffer_queue_log_dom);
   wl_display_disconnect(_ecore_buffer_connection->display);
err_connect:
   free(_ecore_buffer_connection);

   return --_ecore_buffer_queue_init_count;
}

EAPI void
ecore_buffer_queue_shutdown(void)
{
   if (_ecore_buffer_queue_init_count < 1)
     {
        WARN("Ecore_buffer Disconnect called without Connect");
        return;
     }

   DBG("Ecore_Buffer_Connection Shutdown");

   if (!_ecore_buffer_connection)
     return;

   if (_ecore_buffer_connection->fd_hdl)
     ecore_main_fd_handler_del(_ecore_buffer_connection->fd_hdl);

   if (_ecore_buffer_connection->idle_enterer)
     ecore_idle_enterer_del(_ecore_buffer_connection->idle_enterer);

   if (_ecore_buffer_connection->bq_mgr)
     bq_mgr_destroy(_ecore_buffer_connection->bq_mgr);

   if (_ecore_buffer_connection->display)
     wl_display_disconnect(_ecore_buffer_connection->display);

   if (_ecore_buffer_queue_log_dom > 0)
     {
        eina_log_domain_unregister(_ecore_buffer_queue_log_dom);
        _ecore_buffer_queue_log_dom = -1;
     }

   free(_ecore_buffer_connection);
   _ecore_buffer_connection = NULL;
}

struct bq_provider *
_ecore_buffer_connection_provider_create(const char *name)
{
   if (!name)
     return NULL;

   return bq_mgr_create_provider(_ecore_buffer_connection->bq_mgr, name);
}

struct bq_consumer *
_ecore_buffer_connection_consumer_create(const char *name, int queue_size, int w, int h)
{
   if (!name)
     return NULL;

   if (queue_size < 1)
     return NULL;

   if ((w < 1) || (h < 1))
     return NULL;

   return bq_mgr_create_consumer(_ecore_buffer_connection->bq_mgr,
                                                  name, queue_size, w, h);
}

void
_ecore_buffer_connection_init_wait(void)
{
   if (!_ecore_buffer_connection)
     return;

   if (!_ecore_buffer_connection->display)
     return;

   while (!_ecore_buffer_connection->init_done)
     wl_display_dispatch(_ecore_buffer_connection->display);
}

static void
_ecore_buffer_connection_init_callback(void *data, struct wl_callback *callback, uint32_t serial EINA_UNUSED)
{
   Ecore_Buffer_Connection *conn = data;

   if (!conn)
     return;

   DBG("Queue Server Connected");

   wl_callback_destroy(callback);
   conn->init_done = EINA_TRUE;
}

static void
_ecore_buffer_connection_signal_exit_free(void *data EINA_UNUSED, void *event)
{
   free(event);
}

static void
_ecore_buffer_connection_signal_exit(void)
{
   Ecore_Event_Signal_Exit *ev;

   ev = calloc(sizeof(Ecore_Event_Signal_Exit), 1);
   if (!ev)
     return;

   ev->quit = 1;
   ecore_event_add(ECORE_EVENT_SIGNAL_EXIT, ev,
                   _ecore_buffer_connection_signal_exit_free, NULL);
}

static Eina_Bool
_ecore_buffer_connection_cb_fd_handle(void *data, Ecore_Fd_Handler *hdl)
{
   Ecore_Buffer_Connection *conn = data;
   int ret = 0;

   if (_connection_fatal_error)
     return ECORE_CALLBACK_CANCEL;

   if (!conn)
     return ECORE_CALLBACK_RENEW;

   if (ecore_main_fd_handler_active_get(hdl, ECORE_FD_ERROR))
     {
        ERR("Received error on wayland display fd");
        _connection_fatal_error = EINA_TRUE;
        _ecore_buffer_connection_signal_exit();

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
        _ecore_buffer_connection_signal_exit();

        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_buffer_connection_cb_idle_enterer(void *data)
{
   Ecore_Buffer_Connection *conn = data;
   int ret = 0;

   if (_connection_fatal_error)
     return ECORE_CALLBACK_CANCEL;

   if (!conn)
     return ECORE_CALLBACK_RENEW;

   ret = wl_display_get_error(conn->display);
   if (ret < 0)
     goto err;

   ret = wl_display_flush(conn->display);
   if ((ret < 0) && (errno == EAGAIN))
     ecore_main_fd_handler_active_set(conn->fd_hdl,
                                      (ECORE_FD_READ | ECORE_FD_WRITE));

   ret = wl_display_dispatch_pending(conn->display);
   if (ret < 0)
     goto err;

   return ECORE_CALLBACK_RENEW;

err:
   if ((ret < 0) && ((errno != EAGAIN) && (errno != EINVAL)))
     {
        _connection_fatal_error = EINA_TRUE;

        /* raise exit signal */
        _ecore_buffer_connection_signal_exit();

        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_buffer_connection_cb_registry_global(void *data, struct wl_registry *wl_registry, uint32_t id, const char *interface, uint32_t version)
{
   Ecore_Buffer_Connection *conn = data;

   if (!conn)
     return;

   DBG("Added Wl Global Registry - name %d interface %s version %d",
       id, interface, version);

   if (!strncmp(interface, "bq_mgr", strlen("bq_mgr")))
     conn->bq_mgr = wl_registry_bind(wl_registry, id, &bq_mgr_interface, 1);
}

static void
_ecore_buffer_connection_cb_registry_global_remove(void *data EINA_UNUSED, struct wl_registry *wl_registry EINA_UNUSED, uint32_t name EINA_UNUSED)
{
   DBG("Removed Wl Global Registry - name %d", name);
}

void
ecore_buffer_queue_sync(void)
{
   if (!_ecore_buffer_connection)
     return;

   if (!_ecore_buffer_connection->display)
     return;

   wl_display_roundtrip(_ecore_buffer_connection->display);
}
