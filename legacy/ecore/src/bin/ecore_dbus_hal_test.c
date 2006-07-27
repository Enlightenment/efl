/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_dbus_test.h"

#ifdef BUILD_ECORE_DBUS

static int ecore_dbus_event_server_add(void *udata, int ev_type, void *ev);
static int ecore_dbus_event_server_del(void *udata, int ev_type, void *ev);

static void ecore_dbus_method_name_has_owner_cb(void *data, Ecore_DBus_Message_Type type,
						Ecore_DBus_Method_Return *reply);
static void ecore_dbus_method_add_match_cb(void *data, Ecore_DBus_Message_Type type,
					   Ecore_DBus_Method_Return *reply);

static int ecore_dbus_event_server_signal(void *udata, int ev_type, void *ev);

static const char *event_type_get(Ecore_DBus_Message_Type type);

static Ecore_DBus_Server *svr = NULL;

int
main(int argc, char **argv)
{

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
					       ecore_dbus_event_server_add, NULL);
	handler[i++] = ecore_event_handler_add(ECORE_DBUS_EVENT_SERVER_DEL,
					       ecore_dbus_event_server_del, NULL);
	handler[i++] = ecore_event_handler_add(ECORE_DBUS_EVENT_SERVER_SIGNAL,
					       ecore_dbus_event_server_signal, NULL);

	ecore_main_loop_begin();

	for (i = 0; i < 3; i++)
	  ecore_event_handler_del(handler[i]);

	if (svr) ecore_dbus_server_del(svr);
     }
   ecore_dbus_shutdown();
   return 0;
}

static int
ecore_dbus_event_server_add(void *udata, int ev_type, void *ev)
{
   Ecore_DBus_Event_Server_Add *event;

   event = ev;
   printf("ecore_dbus_event_server_add\n");
   ecore_dbus_method_name_has_owner(event->server, "org.freedesktop.Hal",
				    ecore_dbus_method_name_has_owner_cb, NULL);
   return 0;
}

static int
ecore_dbus_event_server_del(void *udata, int ev_type, void *ev)
{
   Ecore_DBus_Event_Server_Del *event;

   event = ev;
   printf("ecore_dbus_event_server_del\n");
   svr = NULL;
   ecore_main_loop_quit();
   return 0;
}

static void
ecore_dbus_method_name_has_owner_cb(void *data, Ecore_DBus_Message_Type type,
				    Ecore_DBus_Method_Return *reply)
{
   unsigned int *exists;
   if (type != ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN)
     {
	ecore_main_loop_quit();
	return;
     }
   printf("ecore_dbus_event_server_method_return %s %s.%s\n", event_type_get(reply->type),
							      reply->header.interface,
							      reply->header.member);

   exists = reply->args[0].value;
   if ((!exists) || (!*exists))
     {
	printf("No hal\n");
	ecore_main_loop_quit();
     }
   else
     {
	printf("Add listener for devices\n");
	ecore_dbus_method_add_match(reply->server,
				    "type='signal',"
				    "interface='org.freedesktop.Hal.Manager',"
				    "sender='org.freedesktop.Hal',"
				    "path='/org/freedesktop/Hal/Manager'",
				    ecore_dbus_method_add_match_cb, NULL);

     }
}

static void
ecore_dbus_method_add_match_cb(void *data, Ecore_DBus_Message_Type type,
			       Ecore_DBus_Method_Return *reply)
{
   if (type != ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN)
     {
	ecore_main_loop_quit();
	return;
     }
   printf("ecore_dbus_event_server_method_return %s %s.%s\n", event_type_get(reply->type),
							      reply->header.interface,
							      reply->header.member);
   printf("Should be listening for device changes!\n");
}

static int
ecore_dbus_event_server_signal(void *udata, int ev_type, void *ev)
{
   Ecore_DBus_Event_Server_Signal *event;

   event = ev;
   printf("ecore_dbus_event_server_signal %s %s.%s\n", event_type_get(event->type),
						     event->header.interface,
						     event->header.member);
   if (!strcmp(event->header.member, "DeviceAdded"))
     {
	printf("Device added: %s\n", (char *)event->args[0].value);
     }
   else if (!strcmp(event->header.member, "DeviceRemoved"))
     {
	printf("Device removed: %s\n", (char *)event->args[0].value);
     }
   return 0;
}

static const char *
event_type_get(Ecore_DBus_Message_Type type)
{
   switch (type)
     {
      case ECORE_DBUS_MESSAGE_TYPE_INVALID:
	 return "ECORE_DBUS_MESSAGE_TYPE_INVALID";
      case ECORE_DBUS_MESSAGE_TYPE_METHOD_CALL:
	 return "ECORE_DBUS_MESSAGE_TYPE_CALL";
      case ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN:
	 return "ECORE_DBUS_MESSAGE_TYPE_RETURN";
      case ECORE_DBUS_MESSAGE_TYPE_ERROR:
	 return "ECORE_DBUS_MESSAGE_TYPE_ERROR";
      case ECORE_DBUS_MESSAGE_TYPE_SIGNAL:
	 return "ECORE_DBUS_MESSAGE_TYPE_SIGNAL";
     }
   return "UNKNOWN";
}
#else
int
main(int argc, const char **argv)
{
   printf("Ecore_DBus module not compiled. This program is empty.\n");
   return -1;
}
#endif
