#include "EDBus.h"
#include <Ecore.h>

#define BUS "com.profusion"
#define PATH "/com/profusion/Test"
#define IFACE "com.profusion.Test"

EDBus_Connection *conn;

static Eina_Bool
_timer1_cb(void *data)
{
   printf("\n## ecore_main_loop_quit()\n");
   ecore_main_loop_quit();
   return EINA_TRUE;
}

static Eina_Bool
_read_cache(void *data)
{
   EDBus_Proxy *proxy = data;
   const char *txt;
   int num;
   Eina_Value *v = edbus_proxy_property_local_get(proxy, "text");
   eina_value_get(v, &txt);
   v = edbus_proxy_property_local_get(proxy, "int32");
   eina_value_get(v, &num);

   printf("Read cache: %s | %d\n", txt, num);

   return EINA_FALSE;
}

static void
on_plus_one(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   int num2 = 0;

   if (edbus_message_error_get(msg, NULL, NULL))
     {
        printf("Message error\n\n");
        return;
     }
   if (!edbus_message_arguments_get(msg, "i", &num2))
     {
        printf("Error getting arguments.");
        return;
     }

   printf("on_plus_one() %d\n", num2);
}

static void
set_property_resp2(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname;
   const char *errmsg;

   printf("set_property_resp2()\n");
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        printf("Message error %s - %s\n\n", errname, errmsg);
        return;
     }
}

static void
get_property_resp2(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   EDBus_Proxy *proxy = data;
   EDBus_Message_Iter *variant = NULL;
   char *type;
   char *resp2;
   const char *errname;
   const char *errmsg;

   printf("get_property_resp2()\n");
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        printf("Message error %s - %s\n\n", errname, errmsg);
        return;
     }
   if (!edbus_message_arguments_get(msg, "v", &variant))
     {
        printf("Error getting arguments.");
        return;
     }

   type = edbus_message_iter_signature_get(variant);
   if (type[1])
     {
        printf("It is a complex type, not handle yet.\n\n");
        return;
     }
   if (type[0] != 's')
     {
        printf("Expected type is string.\n\n");
        return;
     }
   if (!edbus_message_iter_arguments_get(variant, "s", &resp2))
     {
        printf("error in edbus_message_iter_arguments_get()\n\n");
        return;
     }
   printf("resp2=%s\n", resp2);
   free(type);

   edbus_proxy_property_set(proxy, "Resp2", 's', "lalala", set_property_resp2, NULL);
}

static void
on_send_array_int(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   EDBus_Message_Iter *array = NULL;
   int num;

   printf("on_send_array_int()\n");
   if (edbus_message_error_get(msg, NULL, NULL))
     {
        printf("Message error\n\n");
        return;
     }
   if (!edbus_message_arguments_get(msg, "ai", &array))
     {
        printf("Error getting arguments.");
        return;
     }

   while (edbus_message_iter_get_and_next(array, 'i', &num))
     {
        printf("%d\n", num);
     }
}

static void
on_send_array(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   EDBus_Message_Iter *array = NULL;
   char *txt = NULL;
   char *string[10];
   int i = 0;
   int z;

   printf("on_send_array()\n");
   if (edbus_message_error_get(msg, NULL, NULL))
     {
        printf("Message error\n\n");
        return;
     }
   if (!edbus_message_arguments_get(msg, "as", &array))
     {
        printf("Error getting arguments.");
        return;
     }

   while (edbus_message_iter_get_and_next(array, 's', &txt))
     {
        string[i] = txt;
        i++;
     }

   for (z = 0; z < i; z++)
     printf("string = %s\n", string[z]);
}

static void
on_receive_array_with_size(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname;
   const char *errmsg;

   printf("on_receive_array_with_size()\n");
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
     }
}

static void
on_send_variant(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   printf("on_send_variant()\n\n");
}

static void
on_receive_array(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname;
   const char *errmsg;

   printf("on_receive_array()\n");
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
     }
}

static void
_property_changed(void *data, EDBus_Proxy *proxy, void *event_info)
{
   EDBus_Proxy_Event_Property_Changed *event = event_info;
   const char *name;
   const Eina_Value *value;
   printf("property changed\n");

   name = event->name;
   value = event->value;

   if (!strcmp(name, "text"))
     {
        const char *txt;
        eina_value_get(value, &txt);
        printf("[%s] = %s\n", name, txt);
     }
   else if (!strcmp(name, "int32"))
     {
        int num;
        eina_value_get(value, &num);
        printf("[%s] = %d\n", name, num);
     }
}

int
main(void)
{
   EDBus_Object *test2_obj;
   EDBus_Proxy *test2_proxy;
   EDBus_Pending *pending;
   EDBus_Message_Iter *iter, *array_of_string, *variant;
   EDBus_Message_Iter *array_itr, *structure;
   EDBus_Message *msg;
   int size_of_array = 5;
   const char *array[5] = { "aaaa", "bbbb", "cccc", "dddd", "eeee" };
   int i;
   int plus_one = 24;

   ecore_init();
   edbus_init();

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);

   test2_obj = edbus_object_get(conn, BUS, PATH);
   test2_proxy = edbus_proxy_get(test2_obj, IFACE);

   msg = edbus_proxy_method_call_new(test2_proxy, "ReceiveArray");
   iter = edbus_message_iter_get(msg);
   array_of_string = edbus_message_iter_container_new(iter, 'a',"s");
   if (!array_of_string) printf("array_of_string == NULL\n\n");
   for (i = 0; i < 5; i++)
     edbus_message_iter_basic_append(array_of_string, 's', array[i]);
   edbus_message_iter_container_close(iter, array_of_string);
   pending = edbus_proxy_send(test2_proxy, msg, on_receive_array, NULL, -1);
   if (!pending) printf("Error in edbus_proxy_send()\n\n");
   edbus_message_unref(msg);

   msg = edbus_proxy_method_call_new(test2_proxy, "ReceiveArrayOfStringIntWithSize");
   iter = edbus_message_iter_get(msg);
   if (!edbus_message_iter_arguments_set(iter, "ia(si)", size_of_array, &array_of_string))
     printf("error on edbus_massage_iterator_arguments_set()\n\n");
   for (i = 0; i < size_of_array; i++)
     {
         EDBus_Message_Iter *struct_of_si;
         edbus_message_iter_arguments_set(array_of_string, "(si)", &struct_of_si);
         edbus_message_iter_arguments_set(struct_of_si, "si", array[i], i);
         edbus_message_iter_container_close(array_of_string, struct_of_si);
     }
   edbus_message_iter_container_close(iter, array_of_string);
   pending = edbus_proxy_send(test2_proxy, msg, on_receive_array_with_size, NULL, -1);
   edbus_message_unref(msg);

   msg = edbus_proxy_method_call_new(test2_proxy, "SendVariantData");
   iter = edbus_message_iter_get(msg);
   variant = edbus_message_iter_container_new(iter, 'v', "s");
   edbus_message_iter_basic_append(variant, 's', "test");
   edbus_message_iter_container_close(iter, variant);
   pending = edbus_proxy_send(test2_proxy, msg, on_send_variant, NULL, -1);
   edbus_message_unref(msg);

   msg = edbus_proxy_method_call_new(test2_proxy, "DoubleContainner");
   iter = edbus_message_iter_get(msg);
   /**
    * edbus_message_iterator_arguments_set(itr, "a(ii)a(ii)", &array_itr, &array_itr2);
    * this will cause a error, we could not open another container until
    * we close the first one
    */
   edbus_message_iter_arguments_set(iter, "a(ii)", &array_itr);
   for (i = 0; i < 5; i++)
     {
        edbus_message_iter_arguments_set(array_itr, "(ii)", &structure);
        edbus_message_iter_arguments_set(structure, "ii", i, i*i);
        edbus_message_iter_container_close(array_itr, structure);
     }
   edbus_message_iter_container_close(iter, array_itr);
   edbus_message_iter_arguments_set(iter, "a(ii)", &array_itr);
   for (i = 0; i < 7; i++)
     {
        edbus_message_iter_arguments_set(array_itr, "(ii)", &structure);
        edbus_message_iter_arguments_set(structure, "ii", i, i*i*i);
        edbus_message_iter_container_close(array_itr, structure);
     }
   edbus_message_iter_container_close(iter, array_itr);
   edbus_proxy_send(test2_proxy, msg, NULL, NULL, -1);
   edbus_message_unref(msg);

   pending = edbus_proxy_call(test2_proxy, "SendArrayInt", on_send_array_int, NULL,
                                 -1 , "");

   pending = edbus_proxy_call(test2_proxy, "SendArray", on_send_array, NULL,
                              -1 , "");

   pending = edbus_proxy_call(test2_proxy, "PlusOne", on_plus_one, NULL,
                              -1 , "i", plus_one);

   pending = edbus_proxy_property_get(test2_proxy, "Resp2", get_property_resp2, test2_proxy);
   edbus_proxy_event_callback_add(test2_proxy, EDBUS_PROXY_EVENT_PROPERTY_CHANGED, _property_changed, NULL);

   ecore_timer_add(10, _read_cache, test2_proxy);
   ecore_timer_add(50, _timer1_cb, NULL);

   ecore_main_loop_begin();

   edbus_proxy_event_callback_del(test2_proxy, EDBUS_PROXY_EVENT_PROPERTY_CHANGED, _property_changed, NULL);
   edbus_connection_unref(conn);

   edbus_shutdown();
   ecore_shutdown();
   return 0;
}
