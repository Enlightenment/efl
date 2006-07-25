/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_dbus_test.h"

#ifdef BUILD_ECORE_DBUS

static int _ecore_dbus_event_server_add(void *udata, int ev_type, void *ev);
static int _ecore_dbus_event_server_del(void *udata, int ev_type, void *ev);
static int _ecore_dbus_event_server_data(void *udata, int ev_type, void *ev);

int
main(int argc, char **argv)
{
   Ecore_DBus_Server *svr;

   ecore_dbus_init();
   svr = ecore_dbus_server_connect(ECORE_DBUS_BUS_SYSTEM,
				   "/var/run/dbus/system_dbus_socket", -1, NULL);
   if (!svr)
     {
	printf("Couldn't connect to dbus system server!\n");
     }
   else
     {
	int i = 0;
	Ecore_Event_Handler *handler[3];

	printf("Connected!\n");

	handler[i++] = ecore_event_handler_add(ECORE_DBUS_EVENT_SERVER_ADD,
					       _ecore_dbus_event_server_add, NULL);
	handler[i++] = ecore_event_handler_add(ECORE_DBUS_EVENT_SERVER_DEL,
					       _ecore_dbus_event_server_del, NULL);
	handler[i++] = ecore_event_handler_add(ECORE_DBUS_EVENT_SERVER_DATA,
					       _ecore_dbus_event_server_data, NULL);

	ecore_main_loop_begin();

	for (i = 0; i < 3; i++)
	  ecore_event_handler_del(handler[i]);

	ecore_dbus_server_del(svr);
     }
   ecore_dbus_shutdown();
   return 0;
}

static int
_ecore_dbus_event_server_add(void *udata, int ev_type, void *ev)
{
   Ecore_DBus_Event_Server_Add *event;
   int ret;

   event = ev;
   printf("_ecore_dbus_event_server_add\n");
   ret = ecore_dbus_message_new_method_call(event->server, "org.freedesktop.DBus" /*destination*/,
					    "/org/freedesktop/DBus" /*path*/,
					    "org.freedesktop.DBus" /*interface*/,
					    "Hello" /*method*/, NULL /*fmt*/);
   printf("ret: %d\n", ret);
   return 0;
}

static int
_ecore_dbus_event_server_del(void *udata, int ev_type, void *ev)
{
   Ecore_DBus_Event_Server_Del *event;

   event = ev;
   printf("_ecore_dbus_event_server_del\n");
   return 0;
}

static int
_ecore_dbus_event_server_data(void *udata, int ev_type, void *ev)
{
   Ecore_DBus_Event_Server_Data *event;

   event = ev;
   printf("_ecore_dbus_event_server_data\n");
   return 0;
}
#else
int
main(int argc, const char **argv)
{
   printf("Ecore_DBus module not compiled. This program is empty.\n");
   return -1;
}
#endif
