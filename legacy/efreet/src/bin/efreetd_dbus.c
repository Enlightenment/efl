#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <EDBus.h>

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

static EDBus_Connection *conn;
static EDBus_Service_Interface *iface;

static Ecore_Timer *shutdown = NULL;
static int clients = 0;

static Eina_Bool
do_shutdown(void *data __UNUSED__)
{
   quit();
   return ECORE_CALLBACK_CANCEL;
}

static EDBus_Message *
do_register(const EDBus_Service_Interface *ifc __UNUSED__, const EDBus_Message *message)
{
   EDBus_Message *reply;

   clients++;
   if (shutdown) ecore_timer_del(shutdown);
   shutdown = NULL;
   reply = edbus_message_method_return_new(message);
   edbus_message_arguments_set(reply, "b", cache_desktop_exists());
   return reply;
}

static EDBus_Message *
do_unregister(const EDBus_Service_Interface *ifc __UNUSED__, const EDBus_Message *message __UNUSED__)
{
   clients--;
   if (clients <= 0)
     {
        clients = 0;
        if (shutdown) ecore_timer_del(shutdown);
        shutdown = ecore_timer_add(10.0, do_shutdown, NULL);
     }
   return NULL;
}

static EDBus_Message *
add_desktop_dirs(const EDBus_Service_Interface *ifc __UNUSED__, const EDBus_Message *message)
{
   EDBus_Message_Iter *array = NULL;
   const char *dir;

   if (!edbus_message_arguments_get(message, "as", &array))
     {
        ERR("Error getting arguments.");
        return NULL;
     }

   while (edbus_message_iter_get_and_next(array, 's', &dir))
     {
        cache_desktop_dir_add(dir);
     }

   return NULL;
}

static EDBus_Message *
add_icon_dirs(const EDBus_Service_Interface *ifc __UNUSED__, const EDBus_Message *message)
{
   EDBus_Message_Iter *array = NULL;
   const char *dir;

   if (!edbus_message_arguments_get(message, "as", &array))
     {
        ERR("Error getting arguments.");
        return NULL;
     }

   while (edbus_message_iter_get_and_next(array, 's', &dir))
     {
        cache_icon_dir_add(dir);
     }

   return NULL;
}

static EDBus_Message *
build_desktop_cache(const EDBus_Service_Interface *ifc __UNUSED__, const EDBus_Message *message __UNUSED__)
{
   cache_desktop_update();
   return NULL;
}

static EDBus_Message *
add_icon_exts(const EDBus_Service_Interface *ifc __UNUSED__, const EDBus_Message *message)
{
   EDBus_Message_Iter *array = NULL;
   const char *ext;

   if (!edbus_message_arguments_get(message, "as", &array))
     {
        ERR("Error getting arguments.");
        return NULL;
     }

   while (edbus_message_iter_get_and_next(array, 's', &ext))
     {
        cache_icon_ext_add(ext);
     }

   return NULL;
}

static const EDBus_Signal signals[] = {
     [EFREET_SIGNAL_ICON_CACHE_UPDATE] = {"IconCacheUpdate", EDBUS_ARGS({ "b", "update" }), 0},
     [EFREET_SIGNAL_DESKTOP_CACHE_UPDATE] = {"DesktopCacheUpdate", EDBUS_ARGS({ "b", "update" }), 0},
     { NULL, NULL, 0 }
};

static const EDBus_Method methods[] = {
       {
          "Register", NULL, EDBUS_ARGS({"b", "cache exists"}),
          do_register, 0
       },
       {
          "Unregister", NULL, NULL,
          do_unregister, 0
       },
       {
          "AddDesktopDirs", EDBUS_ARGS({"as", "dirs"}), NULL,
          add_desktop_dirs, EDBUS_METHOD_FLAG_NOREPLY
       },
       {
          "BuildDesktopCache", NULL, NULL,
          build_desktop_cache, EDBUS_METHOD_FLAG_NOREPLY
       },
       {
          "AddIconDirs", EDBUS_ARGS({"as", "dirs"}), NULL,
          add_icon_dirs, EDBUS_METHOD_FLAG_NOREPLY
       },
       {
          "AddIconExts", EDBUS_ARGS({"as", "exts"}), NULL,
          add_icon_exts, EDBUS_METHOD_FLAG_NOREPLY
       },
       { NULL, NULL, NULL, NULL, 0 }
};

static const EDBus_Service_Interface_Desc desc = {
   INTERFACE, methods, signals, NULL, NULL, NULL
};

static void
on_name_request(void *data __UNUSED__, const EDBus_Message *msg, EDBus_Pending *pending __UNUSED__)
{
   unsigned int flag;

   if (edbus_message_error_get(msg, NULL, NULL))
     {
        ERR("error on on_name_request");
        return;
     }

   if (!edbus_message_arguments_get(msg, "u", &flag))
     {
        ERR("error getting arguments on on_name_request");
        return;
     }

   if (!(flag & EDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER))
     {
        ERR("error name already in use");
        return;
     }
}

/* external */
void
send_signal_icon_cache_update(Eina_Bool update)
{
   edbus_service_signal_emit(iface, EFREET_SIGNAL_ICON_CACHE_UPDATE, update);
}

void
send_signal_desktop_cache_update(Eina_Bool update)
{
   edbus_service_signal_emit(iface, EFREET_SIGNAL_DESKTOP_CACHE_UPDATE, update);
}

Eina_Bool
dbus_init(void)
{
   if (!edbus_init()) return EINA_FALSE;

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);
   if (!conn) goto conn_error;

   iface = edbus_service_interface_register(conn, PATH, &desc);
   edbus_name_request(conn, BUS, EDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                      on_name_request, NULL);

   return EINA_TRUE;
conn_error:
   edbus_shutdown();
   return EINA_FALSE;
}

Eina_Bool
dbus_shutdown(void)
{
   edbus_connection_unref(conn);
   edbus_shutdown();
   return EINA_TRUE;

}
