/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"

static int init_count = 0;
static Ecore_Event_Handler *handler = NULL;

static int _ecore_dbus_event_method_call_cb(void *udata, int evtype, void *ev);

static void _ecore_dbus_object_free(Ecore_DBus_Object *obj);
static void _ecore_dbus_object_method_free(Ecore_DBus_Object_Method *method);

int
ecore_dbus_objects_init()
{
   if (++init_count != 1) return init_count;

   handler = ecore_event_handler_add(ECORE_DBUS_EVENT_METHOD_CALL,
	       _ecore_dbus_event_method_call_cb, NULL);

   return init_count;
}

void
ecore_dbus_objects_shutdown()
{
   if (!init_count) return;
   if (--init_count) return;

   ecore_event_handler_del(handler);
}

static int
_ecore_dbus_event_method_call_cb(void *udata, int evtype, void *ev)
{
   Ecore_DBus_Event_Server_Data *event;
   Ecore_DBus_Object *obj;
   Ecore_DBus_Object_Method *method;

   event = ev;
   if (!event->header.path  || !event->header.member) return 1;

   event = ev;
   obj = ecore_hash_get(event->server->objects, event->header.path);

   if (!obj)
     {
	Ecore_List *keys;
	char *key;
	printf("[ecore_dbus] object %s not found\n", event->header.path);

	keys = ecore_hash_keys(event->server->objects);
	printf("Keys\n----\n");
	while ((key = ecore_list_next(keys)))
	     printf("  %s => %d\n", key, (int)ecore_hash_get(event->server->objects, key));

	ecore_list_destroy(keys);

	/* XXX send an 'unknown object' error */
	return 0;
     }

   printf("[ecore_dbus] got object %s\n", event->header.path);
   ecore_list_goto_first(obj->methods);
   printf("[ecore_dbus] looking for method %s.%s\n", event->header.interface, event->header.member);
   while((method = ecore_list_next(obj->methods)))
     {
	printf("[ecore_dbus] test against %s.%s\n", method->interface, method->name);
	if (event->header.interface && event->header.interface[0])
	  {
	     if (!strcmp(event->header.interface, method->interface) &&
		 !strcmp(event->header.member, method->name)) break;
	  }
	else
	  {
	     if (!strcmp(event->header.member, method->name)) break;
	  }

	method = NULL;
     }
   if (method)
     {
	printf("[ecore_dbus] method found %s\n", event->header.member);
	method->func(method->data, event);
	return 0;
     }

   printf("[ecore_dbus] unknown method %s\n", event->header.member);
   /* XXX auto-send an 'unknown method' error */
   return 0; 
}

/* objects */

EAPI Ecore_DBus_Object *
ecore_dbus_object_add(Ecore_DBus_Server *server, const char *path)
{
   Ecore_DBus_Object *obj;
   char *key;

   obj = malloc(sizeof(Ecore_DBus_Object));
   if (!obj) return NULL;

   obj->server = server;
   obj->path = strdup(path);
   obj->methods = ecore_list_new();
   ecore_list_set_free_cb(obj->methods, ECORE_FREE_CB(_ecore_dbus_object_method_free));
   printf("[ecore_dbus] adding obj %d to key %s\n", (int)obj, path);
   key = strdup(path);
   ecore_hash_set(server->objects, key, obj);

   return obj;
}

EAPI void
ecore_dbus_object_free(Ecore_DBus_Object *obj)
{
   if (!obj) return;

   if (obj->server) ecore_hash_remove(obj->server->objects, obj->path);
   _ecore_dbus_object_free(obj);
}

static void
_ecore_dbus_object_free(Ecore_DBus_Object *obj)
{
   if (!obj) return;

   if (obj->path) free(obj->path);
   if (obj->methods) ecore_list_destroy(obj->methods);
   free(obj);
}

/* object methods */

EAPI Ecore_DBus_Object_Method *
ecore_dbus_object_method_add(Ecore_DBus_Object *obj, const char *interface, const char *method_name, Ecore_DBus_Method_Call_Cb func, void *data)
{
   Ecore_DBus_Object_Method *m;
   char buf[PATH_MAX];

   if (!obj || !interface || !method_name || !func) return NULL;

   m = malloc(sizeof(Ecore_DBus_Object_Method));
   m->object = obj;
   m->interface = strdup(interface);
   m->name =  strdup(method_name);

   m->func = func;
   m->data = data;
   
   ecore_list_append(obj->methods, m);

   /* add match to server for this method */
   snprintf(buf, sizeof(buf), "type=method_call,path=%s,interface=%s,method=%s", obj->path, interface, method_name);
   ecore_dbus_method_add_match(obj->server, buf, NULL, NULL, NULL);

   return m;
}

EAPI void
ecore_dbus_object_method_free(Ecore_DBus_Object_Method *method)
{
   if (!method) return;

   if (method->object && ecore_list_goto(method->object->methods, method))
	ecore_list_remove(method->object->methods);

   _ecore_dbus_object_method_free(method);
}

static void
_ecore_dbus_object_method_free(Ecore_DBus_Object_Method *method)
{
   if (!method) return;
   if (method->interface) free(method->interface);
   if (method->name) free(method->name);
   free(method);
}

