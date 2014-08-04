#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"
#include <sys/eventfd.h>

static DBusConnection *conn;
static DBusPendingCall *dpending;
static Ecore_Fd_Handler *_dbus_hdlr;
static Ecore_Fd_Handler *_watch_hdlr;
static char *dpath;
static const char *sid;

static void 
_dbus_session_removed(DBusMessage *msg)
{
   const char *n, *o;
   dbus_bool_t ret;

   ret = dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &n, 
                               DBUS_TYPE_OBJECT_PATH, &o, DBUS_TYPE_INVALID);
   if (!ret) return;

   if (!strcmp(n, sid))
     {
        ERR("DBus Session Closed");
//        ecore_main_loop_quit();
     }
}

static void 
_dbus_cb_notify(DBusPendingCall *pending, void *data EINA_UNUSED)
{
   DBusMessage *msg;
   DBusMessageIter iter, s;
   dbus_bool_t ret;
   int type = 0;

   dbus_pending_call_unref(dpending);
   dpending = NULL;

   msg = dbus_pending_call_steal_reply(pending);
   if (!msg) return;

   type = dbus_message_get_type(msg);
   if (type != DBUS_MESSAGE_TYPE_METHOD_RETURN) goto err;

   if ((!dbus_message_iter_init(msg, &iter)) || 
       (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT))
     goto err;

   dbus_message_iter_recurse(&iter, &s);

   if (dbus_message_iter_get_arg_type(&s) != DBUS_TYPE_BOOLEAN)
     goto err;

   dbus_message_iter_get_basic(&s, &ret);
   if (!ret) 
     {
        /* TODO: emit active signal to compositor ? */
     }

err:
   dbus_message_unref(msg);
}

static void 
_dbus_active_get(void)
{
   DBusPendingCall *pend;
   DBusMessage *msg;
   dbus_bool_t ret;
   const char *iface, *n;

   msg = 
     dbus_message_new_method_call("org.freedesktop.login1", dpath, 
                                  "org.freedesktop.DBus.Properties", "Get");
   if (!msg) return;

   iface = "org.freedesktop.login1.Session";
   n = "Active";

   ret = dbus_message_append_args(msg, DBUS_TYPE_STRING, &iface, 
                                  DBUS_TYPE_STRING, &n, DBUS_TYPE_INVALID);
   if (!ret) goto err;

   ret = dbus_connection_send_with_reply(conn, msg, &pend, -1);
   if (!ret) goto err;

   ret = dbus_pending_call_set_notify(pend, _dbus_cb_notify, NULL, NULL);
   if (!ret)
     {
        dbus_pending_call_cancel(pend);
        dbus_pending_call_unref(pend);
        goto err;
     }

   if (dpending)
     {
        dbus_pending_call_cancel(dpending);
        dbus_pending_call_unref(dpending);
     }

   dpending = pend;

   return;

err:
   dbus_message_unref(msg);
}

static void 
_dbus_property_changed(DBusMessage *msg)
{
   DBusMessageIter iter, s, ent;
   const char *iface, *n;
   dbus_bool_t ret;

   if ((!dbus_message_iter_init(msg, &iter)) || 
       (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING))
     return;

   dbus_message_iter_get_basic(&iter, &iface);

   if ((!dbus_message_iter_next(&iter)) || 
       (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY))
     return;

   dbus_message_iter_recurse(&iter, &s);
   while (dbus_message_iter_get_arg_type(&s) == DBUS_TYPE_DICT_ENTRY)
     {
        dbus_message_iter_recurse(&s, &ent);
        if (dbus_message_iter_get_arg_type(&ent) != DBUS_TYPE_STRING)
          return;

        dbus_message_iter_get_basic(&ent, &n);
        if (!dbus_message_iter_next(&ent)) return;

        if (!strcmp(n, "Active"))
          {
             if (dbus_message_iter_get_arg_type(&ent) == DBUS_TYPE_BOOLEAN)
               {
                  dbus_message_iter_get_basic(&ent, &ret);
                  if (!ret)
                    {
                       /* TODO: emit active signal to compositor ? */
                    }
                  return;
               }
          }

        dbus_message_iter_next(&s);
     }

   if ((!dbus_message_iter_next(&iter)) || 
       (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY))
     return;

   dbus_message_iter_recurse(&iter, &s);
   while (dbus_message_iter_get_arg_type(&s) == DBUS_TYPE_STRING)
     {
        dbus_message_iter_get_basic(&s, &n);
        if (!strcmp(n, "Active"))
          {
             _dbus_active_get();
             return;
          }

        dbus_message_iter_next(&s);
     }
}

static void 
_dbus_device_pause_done(uint32_t major, uint32_t minor)
{
   DBusMessage *msg;
   dbus_bool_t ret;

   msg = dbus_message_new_method_call("org.freedesktop.login1", dpath, 
                                      "org.freedesktop.login1.Session", 
                                      "PauseDeviceComplete");
   if (msg)
     {
        ret = dbus_message_append_args(msg, DBUS_TYPE_UINT32, &major, 
                                       DBUS_TYPE_UINT32, &minor, 
                                       DBUS_TYPE_INVALID);
        if (ret)
          dbus_connection_send(conn, msg, NULL);

        dbus_message_unref(msg);
     }
}

static void 
_dbus_device_paused(DBusMessage *msg)
{
   dbus_bool_t ret;
   const char *type;
   uint32_t maj, min;

   ret = dbus_message_get_args(msg, NULL, DBUS_TYPE_UINT32, &maj, 
                               DBUS_TYPE_UINT32, &min, 
                               DBUS_TYPE_STRING, &type, DBUS_TYPE_INVALID);
   if (!ret) return;

   if (!strcmp(type, "pause"))
     _dbus_device_pause_done(maj, min);

   if (maj == DRM_MAJOR)
     {
        /* TODO: emit active signal to compositor ? */
     }
}

static void 
_dbus_device_resumed(DBusMessage *msg)
{
   dbus_bool_t ret;
   uint32_t maj;

   ret = dbus_message_get_args(msg, NULL, 
                               DBUS_TYPE_UINT32, &maj, DBUS_TYPE_INVALID);
   if (!ret) return;

   if (maj == DRM_MAJOR)
     {
        /* TODO: emit active signal to compositor ? */
     }
}

static void 
_dbus_device_release(uint32_t major, uint32_t minor)
{
   DBusMessage *msg;

   msg = dbus_message_new_method_call("org.freedesktop.login1", dpath, 
                                      "org.freedesktop.login1.Session", 
                                      "ReleaseDevice");
   if (msg)
     {
        dbus_bool_t ret;

        ret = dbus_message_append_args(msg, DBUS_TYPE_UINT32, &major, 
                                       DBUS_TYPE_UINT32, &minor, 
                                       DBUS_TYPE_INVALID);
        if (ret) dbus_connection_send(conn, msg, NULL);
        dbus_message_unref(msg);
     }
}

static int 
_dbus_device_take(uint32_t major, uint32_t minor)
{
   DBusMessage *msg, *rep;
   DBusError err;
   dbus_bool_t p, ret;
   int fd = -1;

   msg = dbus_message_new_method_call("org.freedesktop.login1", dpath, 
                                      "org.freedesktop.login1.Session", 
                                      "TakeDevice");
   if (!msg) return -1;

   ret = dbus_message_append_args(msg, DBUS_TYPE_UINT32, &major, 
                                  DBUS_TYPE_UINT32, &minor, DBUS_TYPE_INVALID);
   if (!ret) goto err;

   dbus_error_init(&err);

   rep = 
     dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
   if (!rep)
     {
        if (dbus_error_has_name(&err, DBUS_ERROR_UNKNOWN_METHOD))
          ERR("Old Systemd Version detected");
        else if (dbus_error_is_set(&err))
          ERR("DBusError: %s %s", err.name, err.message);
        dbus_error_free(&err);
        goto err;
     }

   ret = dbus_message_get_args(rep, &err, DBUS_TYPE_UNIX_FD, &fd, 
                               DBUS_TYPE_BOOLEAN, &p, DBUS_TYPE_INVALID);
   if (!ret) 
     {
        if (dbus_error_is_set(&err))
          ERR("DBusError: %s %s", err.name, err.message);

        dbus_error_free(&err);
        goto err_rep;
     }

   return fd;

err_rep:
   dbus_message_unref(rep);
err:
   dbus_message_unref(msg);
   return -1;
}

static int 
_dbus_device_open(const char *path)
{
   struct stat st;
   int ret, fl, fd = -1;
   /* char name[256] = "unknown"; */

   if ((ret = stat(path, &st)) < 0) return -1;
   if (!S_ISCHR(st.st_mode)) return -1;

   fd = _dbus_device_take(major(st.st_rdev), minor(st.st_rdev));
   if (fd < 0)
     {
        ERR("Failed to take device: %s", path);
        return -1;
     }

   if ((fl = fcntl(fd, F_GETFL)) < 0)
     {
        ERR("Failed to get file flags: %m");
        goto flag_err;
     }

   fl = (O_RDWR | O_NONBLOCK);

   if ((ret = fcntl(fd, F_SETFL, fl)) < 0)
     {
        ERR("Failed to set file flags: %m");
        goto flag_err;
     }

   if ((fl = fcntl(fd, F_GETFD)) < 0)
     {
        ERR("Failed to get file fd: %m");
        goto flag_err;
     }

   fl &= ~FD_CLOEXEC;

   if ((ret = fcntl(fd, F_SETFD, fl)) < 0)
     {
        ERR("Failed to set file fds: %m");
        goto flag_err;
     }

   /* if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) */
   /*   { */
   /*      ERR("Could not get device name: %m"); */
   /*      goto flag_err; */
   /*   } */
   /* else */
   /*   { */
   /*      name[sizeof(name) - 1] = '\0'; */
   /*      DBG("%s Opened", name); */
   /*   } */

   return fd;

flag_err:
   _dbus_device_release(major(st.st_rdev), minor(st.st_rdev));
   return -1;
}

static void 
_dbus_device_close(const char *path)
{
   struct stat st;
   int ret;

   if ((ret = stat(path, &st)) < 0) return;
   if (!S_ISCHR(st.st_mode)) return;

   _dbus_device_release(major(st.st_rdev), minor(st.st_rdev));
}

static DBusHandlerResult 
_dbus_cb_filter(DBusConnection *conn EINA_UNUSED, DBusMessage *msg, void *data EINA_UNUSED)
{
   if (dbus_message_is_signal(msg, DBUS_INTERFACE_LOCAL, "Disconnected"))
     {
        ERR("DBus Disconnected");
     }
   else if (dbus_message_is_signal(msg, "org.freedesktop.login1.Manager", 
                                   "SessionRemoved"))
     _dbus_session_removed(msg);
   else if (dbus_message_is_signal(msg, "org.freedesktop.DBus.Properties", 
                                   "PropertiesChanged"))
     _dbus_property_changed(msg);
   else if (dbus_message_is_signal(msg, "org.freedesktop.login1.Session", 
                                   "PauseDevice"))
     _dbus_device_paused(msg);
   else if (dbus_message_is_signal(msg, "org.freedesktop.login1.Sesion", 
                                   "ResumeDevice"))
     _dbus_device_resumed(msg);

   return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static Eina_Bool 
_dbus_cb_dispatch(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   DBusConnection *conn;
   int ret = 0;

   if (!(conn = data)) return ECORE_CALLBACK_CANCEL;

   do
     {
        ret = dbus_connection_dispatch(conn);
        switch (ret)
          {
           case DBUS_DISPATCH_COMPLETE:
             ret = 0;
             break;
           case DBUS_DISPATCH_DATA_REMAINS:
             ret = -EAGAIN;
             break;
           case DBUS_DISPATCH_NEED_MEMORY:
             ret = -ENOMEM;
             break;
           default:
             ret = -EIO;
             break;
          }
     } while (ret == -EAGAIN);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_dbus_cb_watch(void *data, Ecore_Fd_Handler *hdlr)
{
   DBusWatch *watch;
   uint32_t flags = 0;

   if (!(watch = data)) return ECORE_CALLBACK_RENEW;

   if (dbus_watch_get_enabled(watch))
     {
        if (ecore_main_fd_handler_active_get(hdlr, ECORE_FD_READ))
          flags |= DBUS_WATCH_READABLE;
        if (ecore_main_fd_handler_active_get(hdlr, ECORE_FD_WRITE))
          flags |= DBUS_WATCH_WRITABLE;
        if (ecore_main_fd_handler_active_get(hdlr, ECORE_FD_ERROR))
          flags |= DBUS_WATCH_ERROR;

        dbus_watch_handle(watch, flags);
     }

   return ECORE_CALLBACK_RENEW;
}

static dbus_bool_t 
_dbus_watch_add(DBusWatch *watch, void *data EINA_UNUSED)
{
   uint32_t msk = 0, flags = 0;
   int fd = -1;

   msk |= ECORE_FD_ERROR;
   if (dbus_watch_get_enabled(watch))
     {
        flags = dbus_watch_get_flags(watch);
        if (flags & DBUS_WATCH_READABLE)
          msk |= ECORE_FD_READ;
        if (flags & DBUS_WATCH_WRITABLE)
          msk |= ECORE_FD_WRITE;
     }

   fd = dbus_watch_get_unix_fd(watch);

   _watch_hdlr =
     ecore_main_fd_handler_add(fd, msk, _dbus_cb_watch, watch, NULL, NULL);

   dbus_watch_set_data(watch, _watch_hdlr, NULL);

   return TRUE;
}

static void 
_dbus_watch_del(DBusWatch *watch, void *data EINA_UNUSED)
{
   Ecore_Fd_Handler *hdlr;

   if (!(hdlr = dbus_watch_get_data(watch))) return;
   ecore_main_fd_handler_del(hdlr);
   _watch_hdlr = NULL;
}

static void 
_dbus_watch_toggle(DBusWatch *watch, void *data EINA_UNUSED)
{
   uint32_t flags = 0, mask = 0;
   Ecore_Fd_Handler *hdlr;

   if (!(hdlr = dbus_watch_get_data(watch))) return;

   if (dbus_watch_get_enabled(watch))
     {
        flags = dbus_watch_get_flags(watch);
        if (flags & DBUS_WATCH_READABLE)
          mask |= ECORE_FD_READ;
        if (flags & DBUS_WATCH_WRITABLE)
          mask |= ECORE_FD_WRITE;
     }

   ecore_main_fd_handler_active_set(hdlr, mask);
}

static Eina_Bool 
_dbus_cb_timeout(void *data)
{
   DBusTimeout *timeout;

   if (!(timeout = data)) return ECORE_CALLBACK_RENEW;

   if (dbus_timeout_get_enabled(timeout))
     dbus_timeout_handle(timeout);

   return ECORE_CALLBACK_RENEW;
}

static dbus_bool_t 
_dbus_timeout_add(DBusTimeout *timeout, void *data EINA_UNUSED)
{
   if (dbus_timeout_get_enabled(timeout))
     {
        Ecore_Timer *tmr = NULL;
        int tme;

        tme = dbus_timeout_get_interval(timeout);
        if (!(tmr = ecore_timer_loop_add(tme, _dbus_cb_timeout, timeout)))
          return EINA_FALSE;

        dbus_timeout_set_data(timeout, tmr, NULL);
     }

   return EINA_TRUE;
}

static void 
_dbus_timeout_del(DBusTimeout *timeout, void *data EINA_UNUSED)
{
   Ecore_Timer *tmr = NULL;

   if (!(tmr = dbus_timeout_get_data(timeout))) return;
   ecore_timer_del(tmr);
}

static void 
_dbus_timeout_toggle(DBusTimeout *timeout, void *data EINA_UNUSED)
{
   Ecore_Timer *tmr = NULL;

   if (!(tmr = dbus_timeout_get_data(timeout))) return;

   if (dbus_timeout_get_enabled(timeout))
     ecore_timer_thaw(tmr);
   else
     ecore_timer_freeze(tmr);
}

static Eina_Bool 
_dbus_match_add(DBusConnection *conn, const char *format, ...)
{
   DBusError err;
   va_list lst;
   char *tmp;
   int ret;

   va_start(lst, format);
   ret = vasprintf(&tmp, format, lst);
   va_end(lst);

   if (ret < 0) return EINA_FALSE;

   dbus_error_init(&err);
   dbus_bus_add_match(conn, tmp, &err);
   free(tmp);

   if (dbus_error_is_set(&err))
     {
        dbus_error_free(&err);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool 
_dbus_signal_add(DBusConnection *conn, const char *sender, const char *iface, const char *mem, const char *path)
{
   return _dbus_match_add(conn, "type='signal',sender='%s',"
                          "interface='%s',member='%s',path='%s'", 
                          sender, iface, mem, path);
}

static Eina_Bool 
_dbus_setup(void)
{
   int ret = 0;
   dbus_bool_t res;

   ret = asprintf(&dpath, "/org/freedesktop/login1/session/%s", sid);
   if (ret < 0) return EINA_FALSE;

   res = dbus_connection_add_filter(conn, _dbus_cb_filter, NULL, NULL);
   if (!res)
     {
        ERR("Could not setup dbus filter: %m\n");
        goto err;
     }

   res = _dbus_signal_add(conn, "org.freedesktop.login1", 
                          "org.freedesktop.login1.Manager", 
                          "SessionRemoved", "/org/freedesktop/login1");
   if (!res) goto err;

   res = _dbus_signal_add(conn, "org.freedesktop.login1", 
                          "org.freedesktop.login1.Session", 
                          "PauseDevice", dpath);
   if (!res) goto err;

   res = _dbus_signal_add(conn, "org.freedesktop.login1", 
                          "org.freedesktop.login1.Session", 
                          "ResumeDevice", dpath);
   if (!res) goto err;

   res = _dbus_signal_add(conn, "org.freedesktop.login1", 
                          "org.freedesktop.DBus.Properties", 
                          "PropertiesChanged", dpath);
   if (!res) goto err;

   return EINA_TRUE;

err:
   free(dpath);
   return EINA_FALSE;
}

static Eina_Bool 
_dbus_control_take(void)
{
   DBusError err;
   DBusMessage *msg, *rep;
   dbus_bool_t f = EINA_FALSE;

   dbus_error_init(&err);

   msg = 
     dbus_message_new_method_call("org.freedesktop.login1", dpath, 
                                  "org.freedesktop.login1.Session", 
                                  "TakeControl");
   if (!msg) goto err;

   if (!dbus_message_append_args(msg, DBUS_TYPE_BOOLEAN, &f, DBUS_TYPE_INVALID))
     goto msg_err;

   rep = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
   if (!rep)
     {
        if (dbus_error_has_name(&err, DBUS_ERROR_UNKNOWN_METHOD))
          ERR("Old Systemd Version detected\n");
        goto msg_err;
     }

   dbus_message_unref(rep);
   dbus_message_unref(msg);
   dbus_error_free(&err);

   return EINA_TRUE;

msg_err:
   dbus_message_unref(msg);
err:
   if (dbus_error_is_set(&err))
     ERR("DBusError: %s %s", err.name, err.message);
   dbus_error_free(&err);
   return EINA_FALSE;
}

static void 
_dbus_control_release(void)
{
   DBusMessage *msg;

   msg = 
     dbus_message_new_method_call("org.freedesktop.login1", dpath, 
                                  "org.freedesktop.login1.Session", 
                                  "ReleaseControl");
   if (msg)
     {
        dbus_connection_send(conn, msg, NULL);
        dbus_message_unref(msg);
     }
}

static Eina_Bool 
_dbus_bind(DBusConnection *conn)
{
   int fd = -1;

   if ((fd = eventfd(0, EFD_CLOEXEC)) < 0)
     {
        ERR("Could not create eventfd: %m");
        return EINA_FALSE;
     }

   _dbus_hdlr = 
     ecore_main_fd_handler_add(fd, (ECORE_FD_READ | ECORE_FD_WRITE), 
                               _dbus_cb_dispatch, conn, NULL, NULL);
   if (!_dbus_hdlr)
     {
        ERR("Failed to create ecore fd handler");
        goto hdlr_err;
     }

   if (!dbus_connection_set_watch_functions(conn, _dbus_watch_add, 
                                            _dbus_watch_del, 
                                            _dbus_watch_toggle, NULL, NULL))
     {
        ERR("Failed to set dbus watch functions: %m");
        goto watch_err;
     }

   if (!dbus_connection_set_timeout_functions(conn, _dbus_timeout_add, 
                                              _dbus_timeout_del, 
                                              _dbus_timeout_toggle, 
                                              NULL, NULL))
     {
        ERR("Failed to set dbus timeout functions: %m");
        goto timeout_err;
     }

   dbus_connection_ref(conn);

   return EINA_TRUE;

timeout_err:
   dbus_connection_set_watch_functions(conn, NULL, NULL, NULL, NULL, NULL);
watch_err:
   ecore_main_fd_handler_del(_dbus_hdlr);
   _dbus_hdlr = NULL;
hdlr_err:
   close(fd);
   return EINA_FALSE;
}

static DBusConnection *
_dbus_open(void)
{
   DBusConnection *conn;

   dbus_connection_set_change_sigpipe(EINA_FALSE);

   conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, NULL);
   if (!conn)
     {
        ERR("Failed to get dbus connection: %m");
        goto conn_err;
     }

   dbus_connection_set_exit_on_disconnect(conn, EINA_FALSE);

   if (!_dbus_bind(conn))
     {
        ERR("Failed to bind dbus: %m");
        goto bind_err;
     }

   return conn;

bind_err:
   dbus_connection_close(conn);
   dbus_connection_unref(conn);
conn_err:
   return NULL;
}

static void 
_dbus_close(void)
{
   dbus_connection_set_timeout_functions(conn, NULL, NULL, NULL, NULL, NULL);
   dbus_connection_set_watch_functions(conn, NULL, NULL, NULL, NULL, NULL);

   if (_dbus_hdlr) ecore_main_fd_handler_del(_dbus_hdlr);
   _dbus_hdlr = NULL;

   dbus_connection_close(conn);
   dbus_connection_unref(conn);
}

Eina_Bool 
_ecore_drm_dbus_init(const char *session)
{
   if (conn) return EINA_TRUE;

   /* try to init dbus */
   if (!(conn = _dbus_open())) return EINA_FALSE;

   sid = eina_stringshare_add(session);

   /* try to setup signal handlers */
   if (!_dbus_setup()) goto setup_err;

   /* try to take control of the session */
   if (!_dbus_control_take()) goto setup_err;

   return EINA_TRUE;

setup_err:
   _dbus_close();
   eina_stringshare_del(sid);
   return EINA_FALSE;
}

void 
_ecore_drm_dbus_shutdown(void)
{
   _dbus_control_release();
   _dbus_close();
   eina_stringshare_del(sid);
   free(dpath);
}

int 
_ecore_drm_dbus_device_open(const char *device)
{
   return _dbus_device_open(device);
}

void
_ecore_drm_dbus_device_close(const char *device)
{
   _dbus_device_close(device);
}
