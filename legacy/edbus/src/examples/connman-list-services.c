#include "EDBus.h"
#include <Ecore.h>

static void
on_services_get(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   EDBus_Message_Iter *array, *entry;
   const char *errname, *errmsg;

   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "a(oa{sv})", &array))
     {
        fprintf(stderr, "Error: could not get array\n");
        return;
     }

   while (edbus_message_iter_get_and_next(array, 'r', &entry))
     {
        EDBus_Message_Iter *properties, *dict_entry;
        const char *path;

        if (!edbus_message_iter_arguments_get(entry, "oa{sv}", &path, &properties))
          {
             fprintf(stderr, "Error: could not get entry contents\n");
             return;
          }

        printf("service: %s\n", path);

        while (edbus_message_iter_get_and_next(properties, 'e', &dict_entry))
          {
             EDBus_Message_Iter *variant;
             const char *key;

             if (!edbus_message_iter_arguments_get(dict_entry, "sv", &key,
                                                      &variant))
               {
                  fprintf(stderr,
                          "Error: could not get property contents\n");
                  return;
               }

             printf("\t%s: type %s\n", key,
                    edbus_message_iter_signature_get(variant));

             /* TODO: get the value from variant */
          }
     }
}

int
main(void)
{
   EDBus_Connection *conn;
   EDBus_Object *obj;
   EDBus_Proxy *manager;
   EDBus_Pending *pending;

   ecore_init();
   edbus_init();

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SYSTEM);
   if (!conn)
     {
        fprintf(stderr, "Error: could not get system bus\n");
        return EXIT_FAILURE;
     }

   obj = edbus_object_get(conn, "net.connman", "/");
   if (!obj)
     {
        fprintf(stderr, "Error: could not get object\n");
        return EXIT_FAILURE;
     }

   manager = edbus_proxy_get(obj, "net.connman.Manager");
   if (!manager)
     {
        fprintf(stderr, "Error: could not get proxy\n");
        return EXIT_FAILURE;
     }

   pending = edbus_proxy_call(manager, "GetServices", on_services_get, NULL,
                              -1, "");

   if (!pending)
     {
        fprintf(stderr, "Error: could not call\n");
        return EXIT_FAILURE;
     }

   ecore_main_loop_begin();

   edbus_proxy_unref(manager);
   edbus_object_unref(obj);
   edbus_connection_unref(conn);

   edbus_shutdown();
   ecore_shutdown();
   return 0;
}

