#include "EDBus.h"
#include <Ecore.h>

static void
on_dial(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;
   const char *call_path;

   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "o", &call_path))
     {
        fprintf(stderr, "Error: could not get call path\n");
        return;
     }

   printf("dialed! call path: %s\n", call_path);
}

int
main(int argc, char *argv[])
{
   EDBus_Connection *conn;
   EDBus_Object *obj;
   EDBus_Proxy *manager;
   EDBus_Pending *pending;
   const char *number, *hide_callerid;

   if (argc < 2)
     {
        fprintf(stderr, "Usage:\n\t%s <number> [hide_callerid]\n", argv[0]);
        return EXIT_FAILURE;
     }

   number = argv[1];
   hide_callerid = (argc > 2) ? argv[2] : "";

   ecore_init();
   edbus_init();

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SYSTEM);
   if (!conn)
     {
        fprintf(stderr, "Error: could not get system bus\n");
        return EXIT_FAILURE;
     }

   obj = edbus_object_get(conn, "org.ofono", "/");
   if (!obj)
     {
        fprintf(stderr, "Error: could not get object\n");
        return EXIT_FAILURE;
     }

   manager = edbus_proxy_get(obj, "org.ofono.Manager");
   if (!manager)
     {
        fprintf(stderr, "Error: could not get proxy\n");
        return EXIT_FAILURE;
     }

   pending = edbus_proxy_call(manager, "Dial", on_dial, NULL,
                              -1, "ss", number, hide_callerid);
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

