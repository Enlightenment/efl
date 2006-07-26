/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_dbus_test.h"

#ifdef BUILD_ECORE_DBUS

static int ecore_dbus_event_server_add(void *udata, int ev_type, void *ev);
static int ecore_dbus_event_server_del(void *udata, int ev_type, void *ev);
static int ecore_dbus_event_server_data(void *udata, int ev_type, void *ev);

static const char *event_type_get(Ecore_DBus_Message_Type type);

static int state = 0;

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
					       ecore_dbus_event_server_add, NULL);
	handler[i++] = ecore_event_handler_add(ECORE_DBUS_EVENT_SERVER_DEL,
					       ecore_dbus_event_server_del, NULL);
	handler[i++] = ecore_event_handler_add(ECORE_DBUS_EVENT_SERVER_DATA,
					       ecore_dbus_event_server_data, NULL);

	ecore_main_loop_begin();

	for (i = 0; i < 3; i++)
	  ecore_event_handler_del(handler[i]);

	ecore_dbus_server_del(svr);
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
   ecore_dbus_method_hello(event->server);
   state++;
   return 0;
}

static int
ecore_dbus_event_server_del(void *udata, int ev_type, void *ev)
{
   Ecore_DBus_Event_Server_Del *event;

   event = ev;
   printf("ecore_dbus_event_server_del\n");
   ecore_main_loop_quit();
   return 0;
}

static int
ecore_dbus_event_server_data(void *udata, int ev_type, void *ev)
{
   Ecore_DBus_Event_Server_Data *event;

   event = ev;
   printf("ecore_dbus_event_server_data %s\n", event->member);
   if (event->type == ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN)
     {
	if (state == 1)
	  {
	     printf("Check if hal is running\n");
	     ecore_dbus_method_name_has_owner(event->server, "org.freedesktop.Hal");
	     state++;
	  }
	else if (state == 2)
	  {
	     int *exists;

	     exists = ecore_dbus_message_body_field_get(event->message, 0);
	     if ((!exists) || (!*exists))
	       {
		  printf("No hal\n");
		  ecore_main_loop_quit();
	       }
	     else
	       {
		  printf("Add listener for devices\n");
		  ecore_dbus_method_add_match(event->server,
			"type='signal',"
			"interface='org.freedesktop.Hal.Manager',"
			"sender='org.freedesktop.Hal',"
			"path='/org/freedesktop/Hal/Manager'");

		  state++;
	       }
	  }
	else if (state == 3)
	  {
	     printf("Should be listening for device changes!");
	  }
	else
	  {
	     printf("Hm: %s\n", event->member);
	     ecore_dbus_message_print(event->message);
	  }
     }
   else if (event->type == ECORE_DBUS_MESSAGE_TYPE_SIGNAL)
     {
	if (!strcmp(event->member, "org.freedesktop.Hal.Manager.DeviceAdded"))
	  {
	     printf("Device added: %s\n", (char *)ecore_dbus_message_body_field_get(event->message, 0));
	  }
	else if (!strcmp(event->member, "org.freedesktop.Hal.Manager.DeviceRemoved"))
	  {
	     printf("Device removed: %s\n", (char *)ecore_dbus_message_body_field_get(event->message, 0));
	  }
     }
   else
     {
	printf("Event: %s\n", event_type_get(event->type));
	ecore_dbus_message_print(event->message);
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
