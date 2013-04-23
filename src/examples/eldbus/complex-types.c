//Compile with:
// gcc -o complex-types complex-types.c `pkg-config --cflags --libs eldbus ecore`

#include "Eldbus.h"
#include <Ecore.h>

#define BUS "com.profusion"
#define PATH "/com/profusion/Test"
#define IFACE "com.profusion.Test"

Eldbus_Connection *conn;

static Eina_Bool
_timer1_cb(void *data EINA_UNUSED)
{
   printf("\nFishing...\n");
   ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

static void
on_plus_one(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   int num2 = 0;

   if (eldbus_message_error_get(msg, NULL, NULL))
     {
        printf("Message error\n\n");
        return;
     }
   if (!eldbus_message_arguments_get(msg, "i", &num2))
     {
        printf("Error getting arguments.");
        return;
     }

   printf("on_plus_one() %d\n", num2);
}

static void
set_property_resp2(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname;
   const char *errmsg;

   printf("set_property_resp2()\n");
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        printf("Message error %s - %s\n\n", errname, errmsg);
        return;
     }
}

static void
get_property_resp2(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Proxy *proxy = data;
   Eldbus_Message_Iter *variant = NULL;
   char *type;
   char *resp2;
   const char *errname;
   const char *errmsg;

   printf("get_property_resp2()\n");
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        printf("Message error %s - %s\n\n", errname, errmsg);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        printf("Error getting arguments.");
        return;
     }

   type = eldbus_message_iter_signature_get(variant);
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
   if (!eldbus_message_iter_arguments_get(variant, "s", &resp2))
     {
        printf("error in eldbus_message_iter_arguments_get()\n\n");
        return;
     }
   printf("resp2=%s\n", resp2);
   free(type);

   eldbus_proxy_property_set(proxy, "Resp2", "s", &"lalala", set_property_resp2, NULL);
   eldbus_proxy_property_set(proxy, "int32", "i",  (void*)(intptr_t)99, set_property_resp2, NULL);
}

static void
on_send_array_int(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Message_Iter *array = NULL;
   int num;

   printf("on_send_array_int()\n");
   if (eldbus_message_error_get(msg, NULL, NULL))
     {
        printf("Message error\n\n");
        return;
     }
   if (!eldbus_message_arguments_get(msg, "ai", &array))
     {
        printf("Error getting arguments.");
        return;
     }

   while (eldbus_message_iter_get_and_next(array, 'i', &num))
     {
        printf("%d\n", num);
     }
}

static void
on_send_array(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Message_Iter *array = NULL;
   char *txt = NULL;
   char *string[10];
   int i = 0;
   int z;

   printf("on_send_array()\n");
   if (eldbus_message_error_get(msg, NULL, NULL))
     {
        printf("Message error\n\n");
        return;
     }
   if (!eldbus_message_arguments_get(msg, "as", &array))
     {
        printf("Error getting arguments.");
        return;
     }

   while (eldbus_message_iter_get_and_next(array, 's', &txt))
     {
        string[i] = txt;
        i++;
     }

   for (z = 0; z < i; z++)
     printf("string = %s\n", string[z]);
}

static void
on_receive_array_with_size(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname;
   const char *errmsg;

   printf("on_receive_array_with_size()\n");
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
     }
}

static void
on_send_variant(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   printf("on_send_variant()\n\n");
}

static void
on_receive_array(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname;
   const char *errmsg;

   printf("on_receive_array()\n");
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
     }
}

int
main(void)
{
   Eldbus_Object *test2_obj;
   Eldbus_Proxy *test2_proxy;
   Eldbus_Pending *pending;
   Eldbus_Message_Iter *iter, *array_of_string, *variant;
   Eldbus_Message_Iter *array_itr, *structure;
   Eldbus_Message *msg;
   int size_of_array = 5;
   const char *array[5] = { "aaaa", "bbbb", "cccc", "dddd", "eeee" };
   int i;
   int plus_one = 24;

   ecore_init();
   eldbus_init();

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);

   test2_obj = eldbus_object_get(conn, BUS, PATH);
   test2_proxy = eldbus_proxy_get(test2_obj, IFACE);

   msg = eldbus_proxy_method_call_new(test2_proxy, "ReceiveArray");
   iter = eldbus_message_iter_get(msg);
   array_of_string = eldbus_message_iter_container_new(iter, 'a',"s");
   if (!array_of_string) printf("array_of_string == NULL\n\n");
   for (i = 0; i < 5; i++)
     eldbus_message_iter_basic_append(array_of_string, 's', array[i]);
   eldbus_message_iter_container_close(iter, array_of_string);
   pending = eldbus_proxy_send(test2_proxy, msg, on_receive_array, NULL, -1);
   if (!pending) printf("Error in eldbus_proxy_send()\n\n");

   msg = eldbus_proxy_method_call_new(test2_proxy, "ReceiveArrayOfStringIntWithSize");
   iter = eldbus_message_iter_get(msg);
   if (!eldbus_message_iter_arguments_append(iter, "ia(si)", size_of_array, &array_of_string))
     printf("error on eldbus_massage_iterator_arguments_set()\n\n");
   for (i = 0; i < size_of_array; i++)
     {
         Eldbus_Message_Iter *struct_of_si;
         eldbus_message_iter_arguments_append(array_of_string, "(si)", &struct_of_si);
         eldbus_message_iter_arguments_append(struct_of_si, "si", array[i], i);
         eldbus_message_iter_container_close(array_of_string, struct_of_si);
     }
   eldbus_message_iter_container_close(iter, array_of_string);
   pending = eldbus_proxy_send(test2_proxy, msg, on_receive_array_with_size, NULL, -1);

   msg = eldbus_proxy_method_call_new(test2_proxy, "SendVariantData");
   iter = eldbus_message_iter_get(msg);
   variant = eldbus_message_iter_container_new(iter, 'v', "s");
   eldbus_message_iter_basic_append(variant, 's', "test");
   eldbus_message_iter_container_close(iter, variant);
   pending = eldbus_proxy_send(test2_proxy, msg, on_send_variant, NULL, -1);

   msg = eldbus_proxy_method_call_new(test2_proxy, "DoubleContainner");
   iter = eldbus_message_iter_get(msg);
   /**
    * eldbus_message_iterator_arguments_set(itr, "a(ii)a(ii)", &array_itr, &array_itr2);
    * this will cause a error, we could not open another container until
    * we close the first one
    */
   eldbus_message_iter_arguments_append(iter, "a(ii)", &array_itr);
   for (i = 0; i < 5; i++)
     {
        eldbus_message_iter_arguments_append(array_itr, "(ii)", &structure);
        eldbus_message_iter_arguments_append(structure, "ii", i, i*i);
        eldbus_message_iter_container_close(array_itr, structure);
     }
   eldbus_message_iter_container_close(iter, array_itr);
   eldbus_message_iter_arguments_append(iter, "a(ii)", &array_itr);
   for (i = 0; i < 7; i++)
     {
        eldbus_message_iter_arguments_append(array_itr, "(ii)", &structure);
        eldbus_message_iter_arguments_append(structure, "ii", i, i*i*i);
        eldbus_message_iter_container_close(array_itr, structure);
     }
   eldbus_message_iter_container_close(iter, array_itr);
   eldbus_proxy_send(test2_proxy, msg, NULL, NULL, -1);

   pending = eldbus_proxy_call(test2_proxy, "SendArrayInt", on_send_array_int, NULL,
                                 -1 , "");

   pending = eldbus_proxy_call(test2_proxy, "SendArray", on_send_array, NULL,
                              -1 , "");

   pending = eldbus_proxy_call(test2_proxy, "PlusOne", on_plus_one, NULL,
                              -1 , "i", plus_one);

   pending = eldbus_proxy_property_get(test2_proxy, "Resp2", get_property_resp2, test2_proxy);

   ecore_timer_add(10, _timer1_cb, NULL);

   ecore_main_loop_begin();

   eldbus_connection_unref(conn);

   eldbus_shutdown();
   ecore_shutdown();
   return 0;
}
