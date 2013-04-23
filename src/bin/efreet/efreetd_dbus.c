#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Eldbus.h>

#include "efreetd.h"
#include "efreetd_cache.h"

#define BUS "org.enlightenment.Efreet"
#define PATH "/org/enlightenment/Efreet"
#define INTERFACE "org.enlightenment.Efreet"

/* internal */
enum
{
   EFREET_SIGNAL_ICON_CACHE_UPDATE = 0,
   EFREET_SIGNAL_DESKTOP_CACHE_UPDATE
};

static Eldbus_Connection *conn;
static Eldbus_Service_Interface *iface;

static Ecore_Timer *_shutdown = NULL;
static int clients = 0;

static Eina_Bool
do_shutdown(void *data EINA_UNUSED)
{
   quit();
   return ECORE_CALLBACK_CANCEL;
}

static void
disconnected(void *context EINA_UNUSED, Eldbus_Connection *connection EINA_UNUSED, void *event_info EINA_UNUSED)
{
   INF("disconnected");
   quit();
}

static void
client_name_owner_changed_cb(void *data EINA_UNUSED, const char *bus, const char *old_id EINA_UNUSED, const char *new_id)
{
   if (new_id[0])
     return;
   eldbus_name_owner_changed_callback_del(conn, bus,
                                         client_name_owner_changed_cb, NULL);
   clients--;
   if (clients <= 0)
     {
        clients = 0;
        if (_shutdown) ecore_timer_del(_shutdown);
        _shutdown = ecore_timer_add(10.0, do_shutdown, NULL);
     }
}

static Eldbus_Message *
do_register(const Eldbus_Service_Interface *ifc EINA_UNUSED, const Eldbus_Message *message)
{
   Eldbus_Message *reply;
   const char *lang;

   if (!eldbus_message_arguments_get(message, "s", &lang))
     {
        ERR("Error getting arguments.");
        return NULL;
     }
   setenv("LANG", lang, 1);

   clients++;
   if (_shutdown) ecore_timer_del(_shutdown);
   _shutdown = NULL;

   eldbus_name_owner_changed_callback_add(conn,
                                         eldbus_message_sender_get(message),
                                         client_name_owner_changed_cb, NULL,
                                         EINA_FALSE);
   reply = eldbus_message_method_return_new(message);
   eldbus_message_arguments_append(reply, "b", cache_desktop_exists());
   return reply;
}

static Eldbus_Message *
add_desktop_dirs(const Eldbus_Service_Interface *ifc EINA_UNUSED, const Eldbus_Message *message)
{
   Eldbus_Message_Iter *array = NULL;
   const char *dir;

   if (!eldbus_message_arguments_get(message, "as", &array))
     {
        ERR("Error getting arguments.");
        return NULL;
     }

   while (eldbus_message_iter_get_and_next(array, 's', &dir))
     {
        cache_desktop_dir_add(dir);
     }

   return NULL;
}

static Eldbus_Message *
add_icon_dirs(const Eldbus_Service_Interface *ifc EINA_UNUSED, const Eldbus_Message *message)
{
   Eldbus_Message_Iter *array = NULL;
   const char *dir;

   if (!eldbus_message_arguments_get(message, "as", &array))
     {
        ERR("Error getting arguments.");
        return NULL;
     }

   while (eldbus_message_iter_get_and_next(array, 's', &dir))
     {
        cache_icon_dir_add(dir);
     }

   return NULL;
}

static Eldbus_Message *
build_desktop_cache(const Eldbus_Service_Interface *ifc EINA_UNUSED, const Eldbus_Message *message EINA_UNUSED)
{
   const char *lang;

   if (!eldbus_message_arguments_get(message, "s", &lang))
     {
        ERR("Error getting arguments.");
        return NULL;
     }
   setenv("LANG", lang, 1);

   cache_desktop_update();
   return NULL;
}

static Eldbus_Message *
add_icon_exts(const Eldbus_Service_Interface *ifc EINA_UNUSED, const Eldbus_Message *message)
{
   Eldbus_Message_Iter *array = NULL;
   const char *ext;

   if (!eldbus_message_arguments_get(message, "as", &array))
     {
        ERR("Error getting arguments.");
        return NULL;
     }

   while (eldbus_message_iter_get_and_next(array, 's', &ext))
     {
        cache_icon_ext_add(ext);
     }

   return NULL;
}

static const Eldbus_Signal signals[] = {
     [EFREET_SIGNAL_ICON_CACHE_UPDATE] = {"IconCacheUpdate", ELDBUS_ARGS({ "b", "update" }), 0},
     [EFREET_SIGNAL_DESKTOP_CACHE_UPDATE] = {"DesktopCacheUpdate", ELDBUS_ARGS({ "b", "update" }), 0},
     { NULL, NULL, 0 }
};

static const Eldbus_Method methods[] = {
       {
          "Register", ELDBUS_ARGS({"s", "lang info"}), ELDBUS_ARGS({"b", "cache exists"}),
          do_register, 0
       },
       {
          "AddDesktopDirs", ELDBUS_ARGS({"as", "dirs"}), NULL,
          add_desktop_dirs, ELDBUS_METHOD_FLAG_NOREPLY
       },
       {
          "BuildDesktopCache", ELDBUS_ARGS({"s", "lang info"}), NULL,
          build_desktop_cache, ELDBUS_METHOD_FLAG_NOREPLY
       },
       {
          "AddIconDirs", ELDBUS_ARGS({"as", "dirs"}), NULL,
          add_icon_dirs, ELDBUS_METHOD_FLAG_NOREPLY
       },
       {
          "AddIconExts", ELDBUS_ARGS({"as", "exts"}), NULL,
          add_icon_exts, ELDBUS_METHOD_FLAG_NOREPLY
       },
       { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Service_Interface_Desc desc = {
   INTERFACE, methods, signals, NULL, NULL, NULL
};

static void
on_name_request(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   unsigned int reply;

   if (eldbus_message_error_get(msg, NULL, NULL))
     {
        ERR("error on on_name_request");
        quit();
        return;
     }

   if (!eldbus_message_arguments_get(msg, "u", &reply))
     {
        ERR("error getting arguments on on_name_request");
        quit();
        return;
     }

   if (reply != ELDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER)
     {
        ERR("error name already in use");
        quit();
        return;
     }
   INF("name requested");
}

/* external */
void
send_signal_icon_cache_update(Eina_Bool update)
{
   eldbus_service_signal_emit(iface, EFREET_SIGNAL_ICON_CACHE_UPDATE, update);
}

void
send_signal_desktop_cache_update(Eina_Bool update)
{
   eldbus_service_signal_emit(iface, EFREET_SIGNAL_DESKTOP_CACHE_UPDATE, update);
}

Eina_Bool
dbus_init(void)
{
   if (!eldbus_init()) return EINA_FALSE;

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   if (!conn) goto conn_error;

   eldbus_connection_event_callback_add(conn,
      ELDBUS_CONNECTION_EVENT_DISCONNECTED, disconnected, NULL);
   iface = eldbus_service_interface_register(conn, PATH, &desc);
   eldbus_name_request(conn, BUS, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                      on_name_request, NULL);

   return EINA_TRUE;
conn_error:
   eldbus_shutdown();
   return EINA_FALSE;
}

Eina_Bool
dbus_shutdown(void)
{
   eldbus_connection_unref(conn);
   eldbus_shutdown();
   return EINA_TRUE;

}
