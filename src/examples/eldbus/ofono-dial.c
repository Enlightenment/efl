//Compile with:
// gcc -o ofono-dial ofono-dial.c `pkg-config --cflags --libs eldbus ecore`

#include "Eldbus.h"
#include <Ecore.h>

static void
on_dial(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   const char *call_path;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "o", &call_path))
     {
        fprintf(stderr, "Error: could not get call path\n");
        return;
     }

   printf("dialed! call path: %s\n", call_path);
}

int
main(int argc, char *argv[])
{
   Eldbus_Connection *conn;
   Eldbus_Object *obj;
   Eldbus_Proxy *manager;
   Eldbus_Pending *pending;
   const char *number, *hide_callerid;

   if (argc < 2)
     {
        fprintf(stderr, "Usage:\n\t%s <number> [hide_callerid]\n", argv[0]);
        return EXIT_FAILURE;
     }

   number = argv[1];
   hide_callerid = (argc > 2) ? argv[2] : "";

   ecore_init();
   eldbus_init();

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   if (!conn)
     {
        fprintf(stderr, "Error: could not get system bus\n");
        return EXIT_FAILURE;
     }

   obj = eldbus_object_get(conn, "org.ofono", "/");
   if (!obj)
     {
        fprintf(stderr, "Error: could not get object\n");
        return EXIT_FAILURE;
     }

   manager = eldbus_proxy_get(obj, "org.ofono.Manager");
   if (!manager)
     {
        fprintf(stderr, "Error: could not get proxy\n");
        return EXIT_FAILURE;
     }

   pending = eldbus_proxy_call(manager, "Dial", on_dial, NULL,
                              -1, "ss", number, hide_callerid);
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

