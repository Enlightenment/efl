#include "elput_private.h"

#if defined(HAVE_SYSTEMD) || defined(HAVE_ELOGIND)

static void
_logind_session_active_cb_free(void *data EINA_UNUSED, void *event)
{
   Elput_Event_Session_Active *ev;

   ev = event;
   eina_stringshare_del(ev->session);
   free(ev);
}

static void
_logind_session_active_send(Elput_Manager *em, Eina_Bool active)
{
   Elput_Event_Session_Active *ev;

   if (active)
     _elput_input_enable(em);
   else
     _elput_input_disable(em);

   ev = calloc(1, sizeof(Elput_Event_Session_Active));
   if (!ev) return;

   ev->session = eina_stringshare_add(em->sid);
   ev->active = active;

   ecore_event_add(ELPUT_EVENT_SESSION_ACTIVE, ev,
                   _logind_session_active_cb_free, NULL);
}

static void
_logind_device_pause_complete(Elput_Manager *em, uint32_t major, uint32_t minor)
{
   Eldbus_Message *msg;

   msg = eldbus_proxy_method_call_new(em->dbus.session, "PauseDeviceComplete");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        eldbus_message_unref(msg);
        return;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);

   eldbus_proxy_send(em->dbus.session, msg, NULL, NULL, -1);
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
   if (!em->drm_opens) return;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        return;
     }

   if (eldbus_message_arguments_get(msg, "uus", &maj, &min, &type))
     {
        /* If we opened a device during probing then we're still going
         * to get a "gone" callback when we release it, so we'd better
         * eat that instead of treating it like losing the drm device
         * we currently have open, and crapping up libinput's internals
         * for a nice deferred explosion at shutdown...
         *
         * FIXME: do this better?
         */
        if ((em->drm_opens > 1) && (maj == 226) && !strcmp(type, "gone"))
          {
             em->drm_opens--;
             return;
          }

        if (!strcmp(type, "pause"))
          _logind_device_pause_complete(em, maj, min);

        if (maj == 226) // DRM_MAJOR
          _logind_session_active_send(em, EINA_FALSE);
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
        if (maj == 226) // DRM_MAJOR
          _logind_session_active_send(em, EINA_TRUE);
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
_logind_session_object_path_get(Elput_Manager *em)
{
   Eldbus_Message *msg, *reply;
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;
   const char *errname, *errmsg;
   char *str;
   Eina_Bool ret = EINA_FALSE;

   obj =
     eldbus_object_get(em->dbus.conn, "org.freedesktop.login1",
                       "/org/freedesktop/login1");
   if (!obj) return EINA_FALSE;

   proxy = eldbus_proxy_get(obj, "org.freedesktop.login1.Manager");
   if (!proxy) goto proxy_fail;

   msg = eldbus_proxy_method_call_new(proxy, "GetSession");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto message_fail;
     }

   eldbus_message_arguments_append(msg, "s", em->sid);

   reply = eldbus_proxy_send_and_block(proxy, msg, -1);
   if (eldbus_message_error_get(reply, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        eldbus_message_unref(reply);
        goto message_fail;
     }
   if (!eldbus_message_arguments_get(reply, "o", &str))
     {
        eldbus_message_unref(reply);
        goto message_fail;
     }

   em->dbus.path = strdup(str);
   eldbus_message_unref(reply);
   ret = EINA_TRUE;

message_fail:
   eldbus_proxy_unref(proxy);
proxy_fail:
   eldbus_object_unref(obj);
   return ret;
}

static Eina_Bool
_logind_dbus_setup(Elput_Manager *em)
{
   Eldbus_Proxy *proxy;

   if (!_logind_session_object_path_get(em)) return EINA_FALSE;

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
   em->dbus.manager = proxy;

   eldbus_proxy_signal_handler_add(proxy, "SessionRemoved",
                                   _cb_session_removed, em);

   proxy =
     eldbus_proxy_get(em->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get dbus proxy");
        goto proxy_err;
     }
   em->dbus.session = proxy;

   eldbus_proxy_signal_handler_add(proxy, "PauseDevice",
                                   _cb_device_paused, em);
   eldbus_proxy_signal_handler_add(proxy, "ResumeDevice",
                                   _cb_device_resumed, em);
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
   Eldbus_Message *msg, *reply;
   const char *errname, *errmsg;

   msg = eldbus_proxy_method_call_new(em->dbus.session, "TakeControl");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        return EINA_FALSE;
     }

   eldbus_message_arguments_append(msg, "b", EINA_FALSE);

   reply = eldbus_proxy_send_and_block(em->dbus.session, msg, -1);
   if (eldbus_message_error_get(reply, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        eldbus_message_unref(reply);
        return EINA_FALSE;
     }

   eldbus_message_unref(reply);

   return EINA_TRUE;
}

static void
_logind_control_release(Elput_Manager *em)
{
   Eldbus_Message *msg;

   msg = eldbus_proxy_method_call_new(em->dbus.session, "ReleaseControl");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        return;
     }

   eldbus_proxy_send(em->dbus.session, msg, NULL, NULL, -1);
}

static void
_logind_device_release(Elput_Manager *em, uint32_t major, uint32_t minor)
{
   Eldbus_Message *msg;

   msg = eldbus_proxy_method_call_new(em->dbus.session, "ReleaseDevice");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        return;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);

   eldbus_proxy_send(em->dbus.session, msg, NULL, NULL, -1);
}

static void
_logind_pipe_write_fd(Elput_Manager *em, int fd)
{
   int ret;

   while (1)
     {
        ret = write(em->input.pipe, &fd, sizeof(int));
        if (ret < 0)
          {
             if ((errno == EAGAIN) || (errno == EINTR))
               continue;
             WRN("Failed to write to input pipe");
          }
        break;
     }
   close(em->input.pipe);
   em->input.pipe = -1;
}

static void
_logind_device_take_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eina_Bool p = EINA_FALSE;
   const char *errname, *errmsg;
   int ret, fd = -1;
   int fl, flags;
   Elput_Manager *em = data;

   if (em->input.current_pending == pending)
     em->input.current_pending = NULL;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        goto err;
     }

   if (!eldbus_message_arguments_get(msg, "hb", &fd, &p))
     ERR("Could not get UNIX_FD from dbus message");

   if (fd < 0) goto err;

   fl = fcntl(fd, F_GETFL);
   if (fl < 0) goto err;

   flags = (intptr_t)eldbus_pending_data_get(pending, "flags");

   if (flags & O_NONBLOCK)
     fl |= O_NONBLOCK;

   ret = fcntl(fd, F_SETFL, fl);
   if (ret < 0) goto err;

   _logind_pipe_write_fd(em, fd);
   return;

err:
   if (fd >= 0)
     {
        uintptr_t majo, mino;

        close(fd);
        majo = (uintptr_t)eldbus_pending_data_get(pending, "major");
        mino = (uintptr_t)eldbus_pending_data_get(pending, "minor");
        _logind_device_release(em, majo, mino);
     }
   fd = -1;
   _logind_pipe_write_fd(em, fd);
}

static void
_logind_device_take_async(Elput_Manager *em, int flags, uint32_t major, uint32_t minor)
{
   Eldbus_Message *msg;
   intptr_t fd = -1;

   msg = eldbus_proxy_method_call_new(em->dbus.session, "TakeDevice");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        _logind_pipe_write_fd(em, fd);
        return;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);

   em->input.current_pending =
     eldbus_proxy_send(em->dbus.session, msg, _logind_device_take_cb, em, -1);
   if (!em->input.current_pending) CRIT("FAIL!");
   eldbus_pending_data_set(em->input.current_pending, "major",
                           (uintptr_t*)(uintptr_t)major);
   eldbus_pending_data_set(em->input.current_pending, "minor",
                           (uintptr_t*)(uintptr_t)minor);
   eldbus_pending_data_set(em->input.current_pending, "flags",
                           (intptr_t*)(intptr_t)flags);
}

static int
_logind_device_take(Elput_Manager *em, uint32_t major, uint32_t minor)
{
   Eldbus_Message *msg, *reply;
   Eina_Bool p = EINA_FALSE;
   const char *errname, *errmsg;
   int fd = -1;

   msg = eldbus_proxy_method_call_new(em->dbus.session, "TakeDevice");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        return -1;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);

   reply = eldbus_proxy_send_and_block(em->dbus.session, msg, -1);
   if (eldbus_message_error_get(reply, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        eldbus_message_unref(reply);
        return -1;
     }

   if (!eldbus_message_arguments_get(reply, "hb", &fd, &p))
     ERR("Could not get UNIX_FD from dbus message");

   eldbus_message_unref(reply);
   return fd;
}

static Eina_Bool
_logind_activate(Elput_Manager *em)
{
   Eldbus_Message *msg;

   msg = eldbus_proxy_method_call_new(em->dbus.session, "Activate");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        return EINA_FALSE;
     }

   eldbus_proxy_send(em->dbus.session, msg, NULL, NULL, -1);
   return EINA_TRUE;
}

static Eina_Bool
_logind_connect(Elput_Manager **manager, const char *seat, unsigned int tty)
{
   Elput_Manager *em;
   int ret = 0;
   char *s = NULL;

   em = calloc(1, sizeof(Elput_Manager));
   if (!em) return EINA_FALSE;

   em->interface = &_logind_interface;
   em->seat = eina_stringshare_add(seat);

   ret = sd_pid_get_session(getpid(), &em->sid);

   if (ret < 0)
     {
        if (ret == -ENODATA || ret == -ENXIO)
          ERR("Could not get systemd session, the pid is outside a session, check that you are running inside a logind-session.");
        else
          ERR("Could not get systemd session");
        goto session_err;
     }

   if (!em->sid) goto session_err;

   ret = sd_session_get_seat(em->sid, &s);
   if (ret < 0)
     {
        ERR("Failed to get session seat");
        free(s);
        goto seat_err;
     }
   else if ((seat) && (s) && (strcmp(seat, s)))
     {
        ERR("Seat '%s' differs from session seat '%s'", seat, s);
        free(s);
        goto seat_err;
     }

   if ((seat) && (!strcmp(seat, "seat0")))
     {
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
   eldbus_proxy_unref(em->dbus.manager);
   eldbus_proxy_unref(em->dbus.session);
   eldbus_object_unref(em->dbus.obj);
   free(em->dbus.path);
   _logind_dbus_close(em->dbus.conn);
   eina_stringshare_del(em->seat);
   free(em->sid);
   if (em->cached.context) xkb_context_unref(em->cached.context);
   if (em->cached.keymap) xkb_keymap_unref(em->cached.keymap);
   free(em);
}

static void
_logind_open_async(Elput_Manager *em, const char *path, int flags)
{
   struct stat st;
   intptr_t fd = -1;

   if ((stat(path, &st) < 0) || (!S_ISCHR(st.st_mode)))
     _logind_pipe_write_fd(em, fd);
   else
     _logind_device_take_async(em, flags, major(st.st_rdev), minor(st.st_rdev));
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

   if (major(st.st_rdev) == 226) //DRM_MAJOR
     em->drm_opens++;

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
   close(fd);
   if (ret < 0) return;

   if (!S_ISCHR(st.st_mode)) return;

   _logind_device_release(em, major(st.st_rdev), minor(st.st_rdev));
}

static Eina_Bool
_logind_vt_set(Elput_Manager *em, int vt)
{
   Eldbus_Message *msg;

   msg =
     eldbus_message_method_call_new("org.freedesktop.login1",
                                    "/org/freedesktop/login1/seat/self",
                                    "org.freedesktop.login1.Seat", "SwitchTo");
   if (!msg) return EINA_FALSE;

   if (!eldbus_message_arguments_append(msg, "u", vt))
     {
        eldbus_message_unref(msg);
        return EINA_FALSE;
     }

   eldbus_connection_send(em->dbus.conn, msg, NULL, NULL, -1);

   return EINA_TRUE;
}

Elput_Interface _logind_interface =
{
   _logind_connect,
   _logind_disconnect,
   _logind_open,
   _logind_open_async,
   _logind_close,
   _logind_vt_set,
};

#endif
