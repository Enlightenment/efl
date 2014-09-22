#include "ecore_drm_private.h"

static int _dbus_init_count = 0;

static const char *dsession;
static Eldbus_Connection *dconn;
static Eldbus_Object *dobj;

static void 
_ecore_drm_dbus_device_pause_done(uint32_t major, uint32_t minor)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;

   /* try to get the Session proxy */
   if (!(proxy = eldbus_proxy_get(dobj, "org.freedesktop.login1.Session")))
     {
        ERR("Could not get eldbus session proxy");
        return;
     }

   if (!(msg = eldbus_proxy_method_call_new(proxy, "PauseDeviceComplete")))
     {
        ERR("Could not create method call for proxy");
        return;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);
   eldbus_proxy_send(proxy, msg, NULL, NULL, -1);
}

static void 
_cb_session_removed(void *ctxt EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *errname, *errmsg;
   const char *sid;

   DBG("Session Removed");

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        return;
     }

   if (eldbus_message_arguments_get(msg, "s", &sid))
     {
        if (!strcmp(sid, dsession))
          ERR("\tCurrent Session Removed!!");
     }
}

static void 
_cb_device_paused(void *ctxt EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *errname, *errmsg;
   const char *type;
   uint32_t maj, min;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        return;
     }

   if (eldbus_message_arguments_get(msg, "uus", &maj, &min, &type))
     {
        if (!strcmp(type, "pause"))
          {
             /* TODO: device pause done */
             _ecore_drm_dbus_device_pause_done(maj, min);
          }

        /* if (maj == DRM_MAJOR) */
        /*   { */
        /*      // emit paused to compositor */
        /*   } */
     }
}

static void 
_cb_device_resumed(void *ctxt EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *errname, *errmsg;
   uint32_t maj;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        return;
     }

   if (eldbus_message_arguments_get(msg, "u", &maj))
     {
        /* if (maj == DRM_MAJOR) */
        /*   { */
        /*      // emit active to compositor */
        /*   } */
     }
}

static void 
_cb_properties_changed(void *data EINA_UNUSED, Eldbus_Proxy *proxy EINA_UNUSED, void *event)
{
   Eldbus_Proxy_Event_Property_Changed *ev;
   /* const Eina_Value *val; */

   ev = event;
   /* val = ev->value; */

   DBG("Properties Changed: %s", ev->name);

   if (!strcmp(ev->name, "Active"))
     {
        /* TODO: Send 'Active' to login1.Session */
     }
}

static Eina_Bool 
_ecore_drm_dbus_session_take(const char *session)
{
   Eldbus_Proxy *proxy;

   if ((session) && (strcmp(session, dsession)))
     {
        ERR("Invalid session: %s", session);
        return EINA_FALSE;
     }

   /* try to get the Session proxy */
   if (!(proxy = eldbus_proxy_get(dobj, "org.freedesktop.login1.Session")))
     {
        ERR("Could not get eldbus session proxy");
        return EINA_FALSE;
     }

   /* send call to take control */
   if (eldbus_proxy_call(proxy, "TakeControl", NULL, NULL, -1, "b", EINA_FALSE))
     {
        ERR("Could not send message to proxy");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool 
_ecore_drm_dbus_session_release(const char *session)
{
   Eldbus_Proxy *proxy;

   if ((session) && (strcmp(session, dsession)))
     {
        ERR("Invalid session: %s", session);
        return EINA_FALSE;
     }

   /* try to get the Session proxy */
   if (!(proxy = eldbus_proxy_get(dobj, "org.freedesktop.login1.Session")))
     {
        ERR("Could not get eldbus session proxy");
        return EINA_FALSE;
     }

   /* send call to release control */
   if (!eldbus_proxy_call(proxy, "ReleaseControl", NULL, NULL, -1, ""))
     ERR("Could not send ReleaseControl message to proxy");

   return EINA_TRUE;
}

static void 
_ecore_drm_dbus_device_release(uint32_t major, uint32_t minor)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;

   /* try to get the Session proxy */
   if (!(proxy = eldbus_proxy_get(dobj, "org.freedesktop.login1.Session")))
     {
        ERR("Could not get eldbus session proxy");
        return;
     }

   if (!(msg = eldbus_proxy_method_call_new(proxy, "ReleaseDevice")))
     {
        ERR("Could not create method call for proxy");
        return;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);

   eldbus_proxy_send(proxy, msg, NULL, NULL, -1);
}

static int 
_ecore_drm_dbus_device_take(uint32_t major, uint32_t minor, Eldbus_Message_Cb callback, const void *data)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;

   /* try to get the Session proxy */
   if (!(proxy = eldbus_proxy_get(dobj, "org.freedesktop.login1.Session")))
     {
        ERR("Could not get eldbus session proxy");
        return -1;
     }

   if (!(msg = eldbus_proxy_method_call_new(proxy, "TakeDevice")))
     {
        ERR("Could not create method call for proxy");
        return -1;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);
   eldbus_proxy_send(proxy, msg, callback, data, -1);

   return 1;
}

int 
_ecore_drm_dbus_init(const char *session)
{
   Eldbus_Proxy *proxy;
   int ret = 0;
   char *dpath;

   if (++_dbus_init_count != 1) return _dbus_init_count;

   if (!session) return --_dbus_init_count;

   /* try to init eldbus */
   if (!eldbus_init())
     {
        ERR("Could not init eldbus library");
        return --_dbus_init_count;
     }

   dsession = eina_stringshare_add(session);

   /* try to get the dbus connection */
   if (!(dconn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM)))
     {
        ERR("Failed to get eldbus system connection");
        goto conn_err;
     }

   /* assemble dbus path */
   ret = asprintf(&dpath, "/org/freedesktop/login1/session/%s", session);
   if (ret < 0)
     {
        ERR("Could not assemble dbus path");
        goto path_err;
     }

   /* try to get the eldbus object */
   if (!(dobj = eldbus_object_get(dconn, "org.freedesktop.login1", dpath)))
     {
        ERR("Could not get eldbus object: %s", dpath);
        goto obj_err;
     }

   /* try to get the Manager proxy */
   if (!(proxy = eldbus_proxy_get(dobj, "org.freedesktop.login1.Manager")))
     {
        ERR("Could not get eldbus proxy");
        goto proxy_err;
     }

   eldbus_proxy_signal_handler_add(proxy, "SessionRemoved", 
                                   _cb_session_removed, NULL);

   /* try to get the Session proxy */
   if (!(proxy = eldbus_proxy_get(dobj, "org.freedesktop.login1.Session")))
     {
        ERR("Could not get eldbus proxy");
        goto proxy_err;
     }

   eldbus_proxy_signal_handler_add(proxy, "PauseDevice", 
                                   _cb_device_paused, NULL);
   eldbus_proxy_signal_handler_add(proxy, "ResumeDevice", 
                                   _cb_device_resumed, NULL);

   /* try to get the Properties proxy */
   if (!(proxy = eldbus_proxy_get(dobj, "org.freedesktop.DBus.Properties")))
     {
        ERR("Could not get eldbus proxy");
        goto proxy_err;
     }

   eldbus_proxy_properties_monitor(proxy, EINA_TRUE);
   eldbus_proxy_event_callback_add(proxy, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED, 
                                   _cb_properties_changed, NULL);

   if (!_ecore_drm_dbus_session_take(dsession))
     {
        ERR("Failed to take control of session");
        goto session_err;
     }

   return _dbus_init_count;

session_err:
   eldbus_proxy_event_callback_del(proxy, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED, 
                                   _cb_properties_changed, NULL);
proxy_err:
   eldbus_object_unref(dobj);
obj_err:
   free(dpath);
path_err:
   eldbus_connection_unref(dconn);
conn_err:
   eina_stringshare_del(dsession);
   eldbus_shutdown();
   return --_dbus_init_count;
}

int 
_ecore_drm_dbus_shutdown(void)
{
   if (--_dbus_init_count != 0) return _dbus_init_count;

   /* release control of the session */
   _ecore_drm_dbus_session_release(dsession);

   /* release dbus object */
   if (dobj) eldbus_object_unref(dobj);

   /* release the dbus connection */
   if (dconn) eldbus_connection_unref(dconn);

   eina_stringshare_del(dsession);

   /* shutdown eldbus library */
   eldbus_shutdown();

   return _dbus_init_count;
}

void 
_ecore_drm_dbus_device_open(const char *device, Eldbus_Message_Cb callback, const void *data)
{
   struct stat st;

   if (stat(device, &st) < 0) return;
   if (!S_ISCHR(st.st_mode)) return;

   _ecore_drm_dbus_device_take(major(st.st_rdev), minor(st.st_rdev), callback, data);
}

void 
_ecore_drm_dbus_device_close(const char *device)
{
   struct stat st;

   if (stat(device, &st) < 0) return;
   if (!S_ISCHR(st.st_mode)) return;

   _ecore_drm_dbus_device_release(major(st.st_rdev), minor(st.st_rdev));
}
