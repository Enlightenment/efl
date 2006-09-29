/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_dbus_test.h"

#ifdef BUILD_ECORE_DBUS

static int ecore_dbus_event_server_add(void *udata, int ev_type, void *ev);
static int ecore_dbus_event_server_del(void *udata, int ev_type, void *ev);
static int ecore_dbus_event_method_call(void *udata, int ev_type, void *ev);

static void ecore_dbus_method_error_cb(void *data, const char *error);

static void _test_object_init(Ecore_DBus_Server *svr);


static Ecore_DBus_Server *svr = NULL;

int
main(int argc, char **argv)
{
   ecore_dbus_init();

   svr = ecore_dbus_server_session_connect(NULL);
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
	handler[i++] = ecore_event_handler_add(ECORE_DBUS_EVENT_METHOD_CALL,
					       ecore_dbus_event_method_call, NULL);

	ecore_main_loop_begin();

	for (i = 0; i < 2; i++)
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

   ecore_dbus_method_request_name(event->server, "org.enlightenment.Test", 0, NULL, ecore_dbus_method_error_cb, NULL);
   _test_object_init(event->server);
   return 0;
}


static int
ecore_dbus_event_method_call(void *udata, int ev_type, void *ev)
{
   Ecore_DBus_Event_Method_Call *event;

   event = ev;
   printf("ecore_dbus_event_method_call\n");

   return 1;
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
ecore_dbus_method_error_cb(void *data, const char *error)
{
   printf("Error: %s\n", error);
   ecore_main_loop_quit();
}


static void
_test_object_test(Ecore_DBus_Event_Method_Call *event)
{
   Ecore_DBus_Message *msg;
   unsigned int *i;
   char *s1, *s2;
   Ecore_List *l;
   int first;

   printf("received call to test!\n");
   msg = event->message;

   i = (unsigned int *)ecore_dbus_message_body_field_get(msg, 0);
   s1 = (char *)ecore_dbus_message_body_field_get(msg, 1);
   l = (Ecore_List *)ecore_dbus_message_body_field_get(msg, 2);
   s2 = (char *)ecore_dbus_message_body_field_get(msg, 3);

   if (!i || !s1 || !l || !s2) 
   {
      /* XXX reply with error */
      return;
   }

   printf("	params: i = %d, s1 = \"%s\", l = (", i ? *i : 0, s1);
   ecore_list_goto_first(l);
   first = 1;
   while(i = ecore_list_next(l))
     {
	if (!first) printf(", ");
	else first = 0;

	printf("%d", i ? *i : 0);
     }
   printf("), s2: %s\n", s2);
}

static void
_test_object_init(Ecore_DBus_Server *svr)
{
   Ecore_DBus_Object *obj;
   printf("init object!\n");
   if (!svr) return;
   obj = ecore_dbus_object_add(svr, "/org/enlightenment/test");
   ecore_dbus_object_method_add(obj, "org.enlightenment.Test", "Test", _test_object_test, NULL);
}

#else
int
main(int argc, const char **argv)
{
   printf("Ecore_DBus module not compiled. This program is empty.\n");
   return -1;
}
#endif
