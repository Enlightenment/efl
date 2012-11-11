#include "EDBus.h"
#include <Ecore.h>

#define BUS "com.profusion"
#define PATH "/com/profusion/Test"
#define IFACE "com.profusion.Test"

static char *resp2;
/* dummy, incremented each time DBus.Properties.Get() is called */
static int int32 = 35;
static Ecore_Timer *timer;

static EDBus_Message *
_receive_array(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   EDBus_Message_Iter *array;
   char *txt;

   printf("receiveArray\n");
   if (!edbus_message_arguments_get(msg, "as", &array))
     {
        printf("Error on edbus_message_arguments_get()\n");
        return reply;
     }

   while (edbus_message_iter_get_and_next(array, 's', &txt))
     printf("%s\n", txt);

   printf("\n");

   return reply;
}

static EDBus_Message *
_receive_array_of_string_int_with_size(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   EDBus_Message_Iter *array;
   EDBus_Message_Iter *struct_si;
   int size, i = 0;

   printf("receiveArrayOfStringIntWithSize\n");
   if (!edbus_message_arguments_get(msg, "ia(si)", &size, &array))
     {
        printf("Error on edbus_message_arguments_get()\n");
        return reply;
     }

   while (edbus_message_iter_get_and_next(array, 'r', &struct_si))
     {
        char *txt;
        int num;

        if (!edbus_message_iter_arguments_get(struct_si, "si", &txt, &num))
          {
             printf("Error on edbus_message_arguments_get()\n");
             return reply;
          }
        printf("%s | %d\n", txt, num);
        i++;
     }
   printf("size in msg %d | size read %d\n", size, i);
   printf("\n");

   return reply;
}

static EDBus_Message *
_send_variant(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   EDBus_Message_Iter *variant;
   char *type;

   printf("sendVariantData\n");
   if (!edbus_message_arguments_get(msg, "v", &variant))
     {
        printf("Error on edbus_message_arguments_get()\n");
        return reply;
     }

   type = edbus_message_iter_signature_get(variant);
   if (type[1] || type[0] == 'v')
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
           edbus_message_iter_arguments_get(variant, type, &txt);
           printf("type = %c value = %s\n", type[0], txt);
           break;
        }
      case 'i':
        {
           int num;
           edbus_message_iter_arguments_get(variant, type, &num);
           printf("type = %c value = %d\n", type[0], num);
           break;
        }
      default:
        {
           printf("Unhandled type\n");
        }
     }

   printf("\n");

   free(type);
   return reply;
}

static EDBus_Message *
_send_array_int(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   EDBus_Message_Iter *iter, *array;
   int i;

   printf("sendArrayInt\n\n");

   iter = edbus_message_iter_get(reply);
   array = edbus_message_iter_container_new(iter, 'a', "i");
   for (i = 0; i < 5; i++)
     edbus_message_iter_arguments_set(array, "i", i);
   edbus_message_iter_container_close(iter, array);

   return reply;
}

static EDBus_Message *
_send_array(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   EDBus_Message_Iter *iter, *array;
   const char *array_string[5] = {"qqqq", "wwwww", "eeeeee", "rrrrr", "ttttt"};
   int i;

   printf("sendArray\n\n");

   iter = edbus_message_iter_get(reply);
   array = edbus_message_iter_container_new(iter, 'a', "s");
   for (i = 0; i < 5; i++)
     edbus_message_iter_arguments_set(array, "s", array_string[i]);
   edbus_message_iter_container_close(iter, array);

   return reply;
}

static EDBus_Message *
_plus_one(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   int num;

   printf("plusOne\n\n");
   if (!edbus_message_arguments_get(msg, "i", &num))
     {
        printf("Error on edbus_message_arguments_get()\n");
        return reply;
     }
   num++;
   edbus_message_arguments_set(reply, "i", num);

   return reply;
}

static EDBus_Message *
_double_container(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message_Iter *array1, *array2, *structure;
   int num1, num2;
   EDBus_Message *reply = edbus_message_method_return_new(msg);

   if (!edbus_message_arguments_get(msg, "a(ii)a(ii)", &array1, &array2))
     {
        printf("Error on edbus_message_arguments_get()\n");
        return NULL;
     }

   printf("DoubleCountainer\nArray1:\n");
   while (edbus_message_iter_get_and_next(array1, 'r', &structure))
     {
        edbus_message_iter_arguments_get(structure, "ii", &num1, &num2);
        printf("1 %d - 2 %d\n", num1, num2);
     }

   printf("Array2:\n");
   while (edbus_message_iter_get_and_next(array2, 'r', &structure))
     {
         edbus_message_iter_arguments_get(structure, "ii", &num1, &num2);
         printf("1 %d - 2 %d\n", num1, num2);
     }
   printf("\n");
   return reply;
}

static Eina_Bool
_properties_get(const EDBus_Service_Interface *iface, const char *propname, EDBus_Message_Iter *iter, EDBus_Message **error)
{
   printf("Properties_get - %s\n", propname);
   if (!strcmp(propname, "Resp2"))
     edbus_message_iter_basic_append(iter, 's', resp2);
   else if (!strcmp(propname, "text"))
     edbus_message_iter_basic_append(iter, 's', "lalalala");
   else if (!strcmp(propname, "int32"))
     {
        edbus_message_iter_arguments_set(iter, "i", int32);
        int32++;
     }
   else if (!strcmp(propname, "st"))
     {
        EDBus_Message_Iter *st;
        edbus_message_iter_arguments_set(iter, "(ss)", &st);
        edbus_message_iter_arguments_set(st, "ss", "string1", "string2");
        edbus_message_iter_container_close(iter, st);
     }
   return EINA_TRUE;
}

static EDBus_Message *
_properties_set(const EDBus_Service_Interface *iface, const char *propname, const EDBus_Message *msg)
{
   EDBus_Message *reply;
   char *interface, *property, *type, *txt;
   EDBus_Message_Iter *variant;

   if (!edbus_message_arguments_get(msg, "ssv", &interface, &property, &variant))
     {
        printf("Error on edbus_message_arguments_get()\n");
        return NULL;
     }

   type = edbus_message_iter_signature_get(variant);
   if (type[0] != 's')
     {
        reply = edbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidSignature",
                                         "Invalid type.");
        free(type);
        return reply;
     }

   reply = edbus_message_method_return_new(msg);
   edbus_message_iter_arguments_get(variant, "s", &txt);
   printf("Resp2 was set to: %s, previously was: %s\n", txt, resp2);
   free(type);
   free(resp2);
   resp2 = strdup(txt);

   return reply;
}

static const EDBus_Method methods[] = {
      {
        "ReceiveArray", EDBUS_ARGS({"as", "array_of_strings"}),
        NULL, _receive_array, 0
      },
      {
        "ReceiveArrayOfStringIntWithSize",
        EDBUS_ARGS({"i", "size_of_array"}, {"a(si)", "array"}),
        NULL, _receive_array_of_string_int_with_size, 0
      },
      {
        "SendVariantData", EDBUS_ARGS({"v", "variant_data"}),
        NULL, _send_variant, 0
      },
      {
        "SendArrayInt", NULL,
        EDBUS_ARGS({"ai", "array_of_int"}), _send_array_int, 0
      },
      {
        "SendArray", NULL, EDBUS_ARGS({"as", "array_string"}),
        _send_array, 0
      },
      {
        "PlusOne", EDBUS_ARGS({"i", "integer"}),
        EDBUS_ARGS({"i", "integer_plus_one"}), _plus_one, 0
      },
      {
        "DoubleContainner", EDBUS_ARGS({"a(ii)", "array1"}, {"a(ii)", "array2"}),
        NULL, _double_container, 0
      },
      { }
};

static const EDBus_Property properties[] = {
      { "Resp2", "s", NULL, _properties_set },
      { "text", "s", NULL, NULL },
      { "int32", "i", NULL, NULL },
      { "st", "(ss)", NULL, NULL},
      { }
};

static const EDBus_Service_Interface_Desc iface_desc = {
   IFACE, methods, NULL, properties, _properties_get
};

static Eina_Bool _emit_changed(void *data)
{
   EDBus_Service_Interface *iface = data;
   edbus_service_property_changed(iface, "int32");
   edbus_service_property_invalidate_set(iface, "Resp2", EINA_TRUE);
   return EINA_TRUE;
}

static void
on_name_request(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   unsigned int reply;
   EDBus_Service_Interface *iface = data;

   resp2 = malloc(sizeof(char) * 5);
   strcpy(resp2, "test");

   if (edbus_message_error_get(msg, NULL, NULL))
     {
        printf("error on on_name_request\n");
        return;
     }

   if (!edbus_message_arguments_get(msg, "u", &reply))
     {
        printf("error geting arguments on on_name_request\n");
        return;
     }

   if (reply != EDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER)
     {
        printf("error name already in use\n");
        return;
     }

   timer = ecore_timer_add(3, _emit_changed, iface);
}

int
main(void)
{
   EDBus_Connection *conn;
   EDBus_Service_Interface *iface;

   ecore_init();
   edbus_init();

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);

   iface = edbus_service_interface_register(conn, PATH, &iface_desc);
   edbus_name_request(conn, BUS, EDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                      on_name_request, iface);

   ecore_main_loop_begin();

   free(resp2);
   ecore_timer_del(timer);
   edbus_connection_unref(conn);

   edbus_shutdown();
   ecore_shutdown();
   return 0;
}
