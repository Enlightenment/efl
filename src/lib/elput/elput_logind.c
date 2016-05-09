#include "elput_private.h"

#ifdef HAVE_SYSTEMD

static void
_logind_device_pause_complete(Elput_Manager *em, uint32_t major, uint32_t minor)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;

   proxy =
     eldbus_proxy_get(em->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get proxy for session");
        return;
     }

   msg = eldbus_proxy_method_call_new(proxy, "PauseDeviceComplete");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto end;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);

   eldbus_proxy_send(proxy, msg, NULL, NULL, -1);

end:
   eldbus_message_unref(msg);
   eldbus_proxy_unref(proxy);
}

static void
_cb_session_removed(void *data, const Eldbus_Message *msg)
{
   Elput_Manager *em;
   const char *errname, *errmsg;
   const char *sid;

   em = data;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        return;
     }

   if (eldbus_message_arguments_get(msg, "s", &sid))
     {
        if (!strcmp(sid, em->sid))
          {
             WRN("Logind session removed");
             /* TODO: call manager restore function */
          }
     }
}

static void
_cb_device_paused(void *data, const Eldbus_Message *msg)
{
   Elput_Manager *em;
   const char *errname, *errmsg;
   const char *type;
   uint32_t maj, min;

   em = data;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        return;
     }

   if (eldbus_message_arguments_get(msg, "uus", &maj, &min, &type))
     {
        if (!strcmp(type, "pause"))
          _logind_device_pause_complete(em, maj, min);

        if ((em->sync) && (maj == 226)) // DRM_MAJOR
          {
             /* TODO: _ecore_drm2_launcher_activate_send(em, EINA_FALSE); */
          }
     }
}

static void
_cb_device_resumed(void *data, const Eldbus_Message *msg)
{
   Elput_Manager *em;
   const char *errname, *errmsg;
   uint32_t maj;

   em = data;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        return;
     }

   if (eldbus_message_arguments_get(msg, "u", &maj))
     {
        if ((em->sync) && (maj == 226)) // DRM_MAJOR
          {
             /* TODO: _ecore_drm2_launcher_activate_send(em, EINA_TRUE); */
          }
     }
}

static void
_cb_property_changed(void *data, Eldbus_Proxy *proxy EINA_UNUSED, void *event)
{
   Elput_Manager *em;
   Eldbus_Proxy_Event_Property_Changed *ev;
   Eina_Bool active = EINA_FALSE;

   em = data;
   ev = event;

   DBG("DBus Property Changed: %s", ev->name);

   if (!strcmp(ev->name, "Active"))
     {
        eina_value_get(ev->value, &active);
        if ((!em->sync) || (!active))
          {
             /* TODO: _ecore_drm2_launcher_activate_send(em, active); */
          }
     }
}

static Eina_Bool
_logind_session_vt_get(const char *sid, unsigned int *vt)
{
# ifdef HAVE_SYSTEMD_LOGIN_209
   return (sd_session_get_vt(sid, vt) >= 0);
# else
   int ret = 0;
   char *tty;

   ret = sd_session_get_tty(sid, &tty);
   if (ret < 0) return ret;

   ret = sscanf(tty, "tty%u", vt);
   free(tty);

   if (ret != 1) return EINA_FALSE;
   return EINA_TRUE;
# endif
}

static Eina_Bool
_logind_dbus_open(Eldbus_Connection **conn)
{
   if (!eldbus_init()) return EINA_FALSE;

   *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   if (!*conn) return EINA_FALSE;

   return EINA_TRUE;
}

static void
_logind_dbus_close(Eldbus_Connection *conn)
{
   if (conn) eldbus_connection_unref(conn);
   eldbus_shutdown();
}

static Eina_Bool
_logind_dbus_setup(Elput_Manager *em)
{
   Eldbus_Proxy *proxy;
   int ret = 0;

   ret = asprintf(&em->dbus.path,
                  "/org/freedesktop/login1/session/%s", em->sid);
   if (ret < 0) return EINA_FALSE;

   em->dbus.obj =
     eldbus_object_get(em->dbus.conn, "org.freedesktop.login1",
                       em->dbus.path);
   if (!em->dbus.obj)
     {
        ERR("Could not get dbus object");
        goto obj_err;
     }

   proxy =
     eldbus_proxy_get(em->dbus.obj, "org.freedesktop.login1.Manager");
   if (!proxy)
     {
        ERR("Could not get dbus proxy");
        goto proxy_err;
     }

   eldbus_proxy_signal_handler_add(proxy, "SessionRemoved",
                                   _cb_session_removed, em);
   eldbus_proxy_unref(proxy);

   proxy =
     eldbus_proxy_get(em->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get dbus proxy");
        goto proxy_err;
     }

   eldbus_proxy_signal_handler_add(proxy, "PauseDevice",
                                   _cb_device_paused, em);
   eldbus_proxy_signal_handler_add(proxy, "ResumeDevice",
                                   _cb_device_resumed, em);
   eldbus_proxy_unref(proxy);

   proxy =
     eldbus_proxy_get(em->dbus.obj, "org.freedesktop.DBus.Properties");
   if (!proxy)
     {
        ERR("Could not get dbus proxy");
        goto proxy_err;
     }

   eldbus_proxy_properties_monitor(proxy, EINA_TRUE);
   eldbus_proxy_event_callback_add(proxy, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                   _cb_property_changed, em);
   eldbus_proxy_unref(proxy);

   return EINA_TRUE;

proxy_err:
   eldbus_object_unref(em->dbus.obj);
obj_err:
   free(em->dbus.path);
   return EINA_FALSE;
}

static Eina_Bool
_logind_control_take(Elput_Manager *em)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg, *reply;
   const char *errname, *errmsg;

   proxy =
     eldbus_proxy_get(em->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get proxy for session");
        return EINA_FALSE;
     }

   msg = eldbus_proxy_method_call_new(proxy, "TakeControl");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto msg_err;
     }

   eldbus_message_arguments_append(msg, "b", EINA_FALSE);

   reply = eldbus_proxy_send_and_block(proxy, msg, -1);
   if (eldbus_message_error_get(reply, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        goto msg_err;
     }

   eldbus_message_unref(reply);
   eldbus_proxy_unref(proxy);

   return EINA_TRUE;

msg_err:
   eldbus_proxy_unref(proxy);
   return EINA_FALSE;
}

static void
_logind_control_release(Elput_Manager *em)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;

   proxy =
     eldbus_proxy_get(em->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get proxy for session");
        return;
     }

   msg = eldbus_proxy_method_call_new(proxy, "ReleaseControl");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto end;
     }

   eldbus_proxy_send(proxy, msg, NULL, NULL, -1);

end:
   eldbus_proxy_unref(proxy);
}

static int
_logind_device_take(Elput_Manager *em, uint32_t major, uint32_t minor)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg, *reply;
   Eina_Bool p = EINA_FALSE;
   const char *errname, *errmsg;
   int fd = -1;

   proxy =
     eldbus_proxy_get(em->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get dbus proxy");
        return -1;
     }

   msg = eldbus_proxy_method_call_new(proxy, "TakeDevice");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto err;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);

   reply = eldbus_proxy_send_and_block(proxy, msg, -1);
   if (eldbus_message_error_get(reply, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        goto err;
     }

   if (!eldbus_message_arguments_get(reply, "hb", &fd, &p))
     ERR("Could not get UNIX_FD from dbus message");

   eldbus_message_unref(reply);

err:
   eldbus_proxy_unref(proxy);
   return fd;
}

static void
_logind_device_release(Elput_Manager *em, uint32_t major, uint32_t minor)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;

   proxy =
     eldbus_proxy_get(em->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get proxy for session");
        return;
     }

   msg = eldbus_proxy_method_call_new(proxy, "ReleaseDevice");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto end;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);

   eldbus_proxy_send(proxy, msg, NULL, NULL, -1);

end:
   eldbus_proxy_unref(proxy);
}

static Eina_Bool
_logind_activate(Elput_Manager *em)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;

   proxy =
     eldbus_proxy_get(em->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get proxy for session");
        return EINA_FALSE;
     }

   msg = eldbus_proxy_method_call_new(proxy, "Activate");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto msg_err;
     }

   eldbus_proxy_send(proxy, msg, NULL, NULL, -1);

   eldbus_proxy_unref(proxy);

   return EINA_TRUE;

msg_err:
   eldbus_proxy_unref(proxy);
   return EINA_FALSE;
}

static Eina_Bool
_logind_connect(Elput_Manager **manager, const char *seat, unsigned int tty, Eina_Bool sync)
{
   Elput_Manager *em;
   int ret = 0;
   char *s;

   em = calloc(1, sizeof(Elput_Manager));
   if (!em) return EINA_FALSE;

   em->interface = &_logind_interface;
   em->sync = sync;
   em->seat = eina_stringshare_add(seat);

   ret = sd_pid_get_session(getpid(), &em->sid);
   if (ret < 0)
     {
        ERR("Could not get systemd session");
        goto session_err;
     }

   ret = sd_session_get_seat(em->sid, &s);
   if (ret < 0)
     {
        ERR("Failed to get session seat");
        free(s);
        goto seat_err;
     }
   else if ((seat) && (strcmp(seat, s)))
     {
        ERR("Seat '%s' differs from session seat '%s'", seat, s);
        free(s);
        goto seat_err;
     }

   if (!_logind_session_vt_get(em->sid, &em->vt_num))
     {
        ERR("Could not get session vt");
        goto vt_err;
     }
   else if ((tty > 0) && (em->vt_num != tty))
     {
        ERR("Requested VT %u differs from session VT %u", tty, em->vt_num);
        goto vt_err;
     }

   free(s);

   if (!_logind_dbus_open(&em->dbus.conn))
     {
        ERR("Could not connect to dbus");
        goto vt_err;
     }

   if (!_logind_dbus_setup(em))
     {
        ERR("Could not setup dbus");
        goto dbus_err;
     }

   if (!_logind_control_take(em))
     {
        ERR("Could not take control of session");
        goto ctrl_err;
     }

   if (!_logind_activate(em))
     {
        ERR("Could not activate session");
        goto actv_err;
     }

   *(Elput_Manager **)manager = em;

   return EINA_TRUE;

actv_err:
   _logind_control_release(em);
ctrl_err:
   eldbus_object_unref(em->dbus.obj);
   free(em->dbus.path);
dbus_err:
   _logind_dbus_close(em->dbus.conn);
vt_err:
seat_err:
   free(em->sid);
session_err:
   free(em);
   return EINA_FALSE;
}

static void
_logind_disconnect(Elput_Manager *em)
{
   _logind_control_release(em);
   eldbus_object_unref(em->dbus.obj);
   free(em->dbus.path);
   _logind_dbus_close(em->dbus.conn);
   eina_stringshare_del(em->seat);
   free(em->sid);
   free(em);
}

static int
_logind_open(Elput_Manager *em, const char *path, int flags)
{
   struct stat st;
   int ret, fd = -1;
   int fl;

   ret = stat(path, &st);
   if (ret < 0) return -1;

   if (!S_ISCHR(st.st_mode)) return -1;

   fd = _logind_device_take(em, major(st.st_rdev), minor(st.st_rdev));
   if (fd < 0) return fd;

   fl = fcntl(fd, F_GETFL);
   if (fl < 0) goto err;

   if (flags & O_NONBLOCK)
     fl |= O_NONBLOCK;

   ret = fcntl(fd, F_SETFL, fl);
   if (ret < 0) goto err;

   return fd;

err:
   close(fd);
   _logind_device_release(em, major(st.st_rdev), minor(st.st_rdev));
   return -1;
}

static void
_logind_close(Elput_Manager *em, int fd)
{
   struct stat st;
   int ret;

   ret = fstat(fd, &st);
   if (ret < 0) return;

   if (!S_ISCHR(st.st_mode)) return;

   _logind_device_release(em, major(st.st_rdev), minor(st.st_rdev));
}

Elput_Interface _logind_interface =
{
   _logind_connect,
   _logind_disconnect,
   _logind_open,
   _logind_close,
};

#endif
