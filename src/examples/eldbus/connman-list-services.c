//Compile with:
// gcc -o connman-list-services connman-list-services.c `pkg-config --cflags --libs eldbus ecore`

#include "Eldbus.h"
#include <Ecore.h>

static void
on_services_get(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Message_Iter *array, *entry;
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "a(oa{sv})", &array))
     {
        fprintf(stderr, "Error: could not get array\n");
        return;
     }

   while (eldbus_message_iter_get_and_next(array, 'r', &entry))
     {
        Eldbus_Message_Iter *properties, *dict_entry;
        const char *path;

        if (!eldbus_message_iter_arguments_get(entry, "oa{sv}", &path, &properties))
          {
             fprintf(stderr, "Error: could not get entry contents\n");
             return;
          }

        printf("service: %s\n", path);

        while (eldbus_message_iter_get_and_next(properties, 'e', &dict_entry))
          {
             Eldbus_Message_Iter *variant;
             const char *key;

             if (!eldbus_message_iter_arguments_get(dict_entry, "sv", &key,
                                                      &variant))
               {
                  fprintf(stderr,
                          "Error: could not get property contents\n");
                  return;
               }

             printf("\t%s: type %s\n", key,
                    eldbus_message_iter_signature_get(variant));

             /* TODO: get the value from variant */
          }
     }
}

int
main(void)
{
   Eldbus_Connection *conn;
   Eldbus_Object *obj;
   Eldbus_Proxy *manager;
   Eldbus_Pending *pending;

   ecore_init();
   eldbus_init();

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   if (!conn)
     {
        fprintf(stderr, "Error: could not get system bus\n");
        return EXIT_FAILURE;
     }

   obj = eldbus_object_get(conn, "net.connman", "/");
   if (!obj)
     {
        fprintf(stderr, "Error: could not get object\n");
        return EXIT_FAILURE;
     }

   manager = eldbus_proxy_get(obj, "net.connman.Manager");
   if (!manager)
     {
        fprintf(stderr, "Error: could not get proxy\n");
        return EXIT_FAILURE;
     }

   pending = eldbus_proxy_call(manager, "GetServices", on_services_get, NULL,
                              -1, "");

   if (!pending)
     {
        fprintf(stderr, "Error: could not call\n");
        return EXIT_FAILURE;
     }

   ecore_main_loop_begin();

   eldbus_proxy_unref(manager);
   eldbus_object_unref(obj);
   eldbus_connection_unref(conn);

   eldbus_shutdown();
   ecore_shutdown();
   return 0;
}

