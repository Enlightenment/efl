//Compile with:
// gcc -o complex-types-server complex-types-server.c `pkg-config --cflags --libs eldbus ecore`

#include "Eldbus.h"
#include <Ecore.h>

#define BUS "com.profusion"
#define PATH "/com/profusion/Test"
#define IFACE "com.profusion.Test"

static char *resp2;
/* dummy, incremented each time DBus.Properties.Get() is called */
static int int32 = 35;
static Ecore_Timer *timer;

static Eldbus_Message *
_receive_array(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *array;
   const char *txt;

   printf("- receiveArray\n");
   if (!eldbus_message_arguments_get(msg, "as", &array))
     {
        printf("Error on eldbus_message_arguments_get()\n");
        return reply;
     }

   while (eldbus_message_iter_get_and_next(array, 's', &txt))
     printf("%s\n", txt);
   printf("}\n\n");

   return reply;
}

static Eldbus_Message *
_receive_array_of_string_int_with_size(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *array, *struct_si;
   int size, i = 0;

   printf("- receiveArrayOfStringIntWithSize\n{\n");
   if (!eldbus_message_arguments_get(msg, "ia(si)", &size, &array))
     {
        printf("Error on eldbus_message_arguments_get()\n");
        return reply;
     }

   while (eldbus_message_iter_get_and_next(array, 'r', &struct_si))
     {
        const char *txt;
        int num;
        if (!eldbus_message_iter_arguments_get(struct_si, "si", &txt, &num))
          {
             printf("Error on eldbus_message_arguments_get()\n");
             return reply;
          }
        printf("%s | %d\n", txt, num);
        i++;
     }
   printf("size in msg %d | size read %d\n", size, i);
   printf("}\n\n");

   return reply;
}

static Eldbus_Message *
_receive_variant(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *var, *array, *main_iter;

   main_iter = eldbus_message_iter_get(reply);
   var = eldbus_message_iter_container_new(main_iter, 'v', "as");
   eldbus_message_iter_arguments_append(var, "as", &array);

   eldbus_message_iter_arguments_append(array, "s", "item1");
   eldbus_message_iter_arguments_append(array, "s", "item2");
   eldbus_message_iter_arguments_append(array, "s", "item3");

   eldbus_message_iter_container_close(var, array);
   eldbus_message_iter_container_close(main_iter, var);

   return reply;
}

static Eldbus_Message *
_send_variant(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *variant;
   char *type;

   printf("- sendVariantData\n{\n");
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        printf("Error on eldbus_message_arguments_get()\n");
        return reply;
     }

   type = eldbus_message_iter_signature_get(variant);
   if (type[1])
     {
        printf("It is a complex type, not handle yet.\n");
        free(type);
        return reply;
     }

   switch (type[0])
     {
      case 's':
      case 'o':
        {
           char *txt;
           eldbus_message_iter_arguments_get(variant, type, &txt);
           printf("type = %c value = %s\n", type[0], txt);
           break;
        }
      case 'i':
        {
           int num;
           eldbus_message_iter_arguments_get(variant, type, &num);
           printf("type = %c value = %d\n", type[0], num);
           break;
        }
      default:
        {
           printf("Unhandled type\n");
        }
     }

   printf("}\n\n");

   free(type);
   return reply;
}

static Eldbus_Message *
_send_array_int(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *iter, *array;
   int numbers[] = { 10, 9, 8, 7, 6, 5 };

   printf("- sendArrayInt\n\n");

   iter = eldbus_message_iter_get(reply);
   array = eldbus_message_iter_container_new(iter, 'a', "i");
   eldbus_message_iter_fixed_array_append(array, 'i', numbers, EINA_C_ARRAY_LENGTH(numbers));
   eldbus_message_iter_container_close(iter, array);

   return reply;
}

static Eldbus_Message *
_send_array(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *iter, *array;
   const char *array_string[5] = {"qqqq", "wwwww", "eeeeee", "rrrrr", "ttttt"};
   int i;

   printf("sendArray\n\n");

   iter = eldbus_message_iter_get(reply);
   array = eldbus_message_iter_container_new(iter, 'a', "s");
   for (i = 0; i < 5; i++)
     eldbus_message_iter_arguments_append(array, "s", array_string[i]);
   eldbus_message_iter_container_close(iter, array);

   return reply;
}

static Eldbus_Message *
_plus_one(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   int num;

   printf("- plusOne\n\n");
   if (!eldbus_message_arguments_get(msg, "i", &num))
     {
        printf("Error on eldbus_message_arguments_get()\n");
        return reply;
     }
   num++;
   eldbus_message_arguments_append(reply, "i", num);

   return reply;
}

static Eldbus_Message *
_double_container(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message_Iter *array1, *array2, *structure;
   int num1, num2;
   Eldbus_Message *reply;

   if (!eldbus_message_arguments_get(msg, "a(ii)a(ii)", &array1, &array2))
     {
        printf("Error on eldbus_message_arguments_get()\n");
        return NULL;
     }
   reply = eldbus_message_method_return_new(msg);

   printf("DoubleCountainer\n{\nArray1:\n");
   while (eldbus_message_iter_get_and_next(array1, 'r', &structure))
     {
        eldbus_message_iter_arguments_get(structure, "ii", &num1, &num2);
        printf("1 %d - 2 %d\n", num1, num2);
     }

   printf("Array2:\n");
   while (eldbus_message_iter_get_and_next(array2, 'r', &structure))
     {
         eldbus_message_iter_arguments_get(structure, "ii", &num1, &num2);
         printf("1 %d - 2 %d\n", num1, num2);
     }
   printf("}\n\n");
   return reply;
}

static Eina_Bool
_properties_get(const Eldbus_Service_Interface *iface EINA_UNUSED, const char *propname, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   printf("Properties_get - %s\n", propname);
   if (!strcmp(propname, "Resp2"))
     eldbus_message_iter_basic_append(iter, 's', resp2);
   else if (!strcmp(propname, "text"))
     eldbus_message_iter_basic_append(iter, 's', "lalalala");
   else if (!strcmp(propname, "int32"))
     {
        eldbus_message_iter_arguments_append(iter, "i", int32);
        int32++;
     }
   else if (!strcmp(propname, "st"))
     {
        Eldbus_Message_Iter *st;
        eldbus_message_iter_arguments_append(iter, "(ss)", &st);
        eldbus_message_iter_arguments_append(st, "ss", "string1", "string2");
        eldbus_message_iter_container_close(iter, st);
     }
   return EINA_TRUE;
}

static Eldbus_Message *
_properties_set(const Eldbus_Service_Interface *iface EINA_UNUSED, const char *propname, Eldbus_Message_Iter *iter, const Eldbus_Message *msg)
{
   char *type;

   type = eldbus_message_iter_signature_get(iter);

   if (!strcmp(propname, "int32"))
     {
        int num;
        if (type[0] != 'i')
          goto invalid_signature;
        eldbus_message_iter_arguments_get(iter, "i", &num);
        printf("int32 was set to: %d, previously was: %d\n", num, int32);
        int32 = num;
     }
   else if (!strcmp(propname, "Resp2"))
     {
        const char *txt;
        if (type[0] != 's')
          goto invalid_signature;
        eldbus_message_iter_arguments_get(iter, "s", &txt);
        printf("Resp2 was set to: %s, previously was: %s\n", txt, resp2);
        free(resp2);
        resp2 = strdup(txt);
     }
   free(type);
   return eldbus_message_method_return_new(msg);

invalid_signature:
   free(type);
   return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidSignature",
                                  "Invalid type.");
}

static const Eldbus_Method methods[] = {
      {
        "ReceiveArray", ELDBUS_ARGS({"as", "array_of_strings"}),
        NULL, _receive_array, 0
      },
      {
        "ReceiveArrayOfStringIntWithSize",
        ELDBUS_ARGS({"i", "size_of_array"}, {"a(si)", "array"}),
        NULL, _receive_array_of_string_int_with_size, 0
      },
      {
        "SendVariantData", ELDBUS_ARGS({"v", "variant_data"}),
        NULL, _send_variant, 0
      },
      {
       "ReceiveVariantData", NULL, ELDBUS_ARGS({"v", "variant_data"}),
       _receive_variant, 0
      },
      {
        "SendArrayInt", NULL,
        ELDBUS_ARGS({"ai", "array_of_int"}), _send_array_int, 0
      },
      {
        "SendArray", NULL, ELDBUS_ARGS({"as", "array_string"}),
        _send_array, 0
      },
      {
        "PlusOne", ELDBUS_ARGS({"i", "integer"}),
        ELDBUS_ARGS({"i", "integer_plus_one"}), _plus_one, 0
      },
      {
        "DoubleContainner", ELDBUS_ARGS({"a(ii)", "array1"}, {"a(ii)", "array2"}),
        NULL, _double_container, 0
      },
      { }
};

static const Eldbus_Property properties[] = {
      { "Resp2", "s", NULL, _properties_set, 0 },
      { "text", "s", NULL, NULL, 0 },
      { "int32", "i", NULL, _properties_set, 0 },
      { "st", "(ss)", NULL, NULL, 0 },
      { }
};

static const Eldbus_Service_Interface_Desc iface_desc = {
   IFACE, methods, NULL, properties, _properties_get, NULL
};

static Eina_Bool _emit_changed(void *data)
{
   Eldbus_Service_Interface *iface = data;
   eldbus_service_property_changed(iface, "int32");
   eldbus_service_property_invalidate_set(iface, "Resp2", EINA_TRUE);
   return ECORE_CALLBACK_RENEW;
}

static void
on_name_request(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   unsigned int reply;
   Eldbus_Service_Interface *iface = data;

   if (eldbus_message_error_get(msg, NULL, NULL))
     {
        printf("error on on_name_request\n");
        return;
     }

   if (!eldbus_message_arguments_get(msg, "u", &reply))
     {
        printf("error geting arguments on on_name_request\n");
        return;
     }

   if (reply != ELDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER)
     {
        printf("error name already in use\n");
        return;
     }

   timer = ecore_timer_add(3, _emit_changed, iface);
}

int
main(void)
{
   Eldbus_Connection *conn;
   Eldbus_Service_Interface *iface;

   ecore_init();
   eldbus_init();

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);

   resp2 = malloc(sizeof(char) * 5);
   strcpy(resp2, "test");
   iface = eldbus_service_interface_register(conn, PATH, &iface_desc);
   eldbus_name_request(conn, BUS, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                      on_name_request, iface);

   ecore_main_loop_begin();

   free(resp2);
   ecore_timer_del(timer);
   eldbus_service_interface_unregister(iface);
   eldbus_connection_unref(conn);

   eldbus_shutdown();
   ecore_shutdown();
   return 0;
}
