#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <EDBus.h>

#include "efreetd.h"
#include "efreetd_cache.h"

#define BUS "org.enlightenment"
#define PATH "/org/enlightenment"
#define INTERFACE "org.enlightenment.Efreet"

/* internal */
enum
{
   EFREET_SIGNAL_ICON_CACHE_UPDATE = 0,
   EFREET_SIGNAL_DESKTOP_CACHE_UPDATE
};

static EDBus_Connection *conn;
static EDBus_Service_Interface *iface;

static EDBus_Message *
ping(const EDBus_Service_Interface *ifc __UNUSED__, const EDBus_Message *message)
{
   printf("ping\n");
   return edbus_message_method_return_new(message);
}
 
static EDBus_Message *
add_desktop_dirs(const EDBus_Service_Interface *ifc __UNUSED__, const EDBus_Message *message)
{
   EDBus_Message_Iter *array = NULL;
   const char *dir;

   printf("Add desktop dirs\n");
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

   printf("Add icon dirs\n");
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
add_icon_exts(const EDBus_Service_Interface *ifc __UNUSED__, const EDBus_Message *message)
{
   EDBus_Message_Iter *array = NULL;
   const char *ext;

   printf("Add icon exts\n");
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
          "Ping", NULL, NULL,
          ping, 0
       },
       {
          "AddDesktopDirs", NULL, EDBUS_ARGS({"as", "dirs"}),
          add_desktop_dirs, EDBUS_METHOD_FLAG_NOREPLY
       },
       {
          "AddIconDirs", NULL, EDBUS_ARGS({"as", "dirs"}),
          add_icon_dirs, EDBUS_METHOD_FLAG_NOREPLY
       },
       {
          "AddIconExts", NULL, EDBUS_ARGS({"as", "exts"}),
          add_icon_exts, EDBUS_METHOD_FLAG_NOREPLY
       },
       { NULL, NULL, NULL, NULL, 0 }
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

   iface = edbus_service_interface_register(conn, PATH, INTERFACE, methods,
                                            signals);
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
