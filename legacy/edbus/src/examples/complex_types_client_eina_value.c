#include "EDBus.h"
#include <Ecore.h>

#define BUS "com.profusion"
#define PATH "/com/profusion/Test"
#define IFACE "com.profusion.Test"

#define size_of_array 5
static const char *array_string[] = {
   "aaaa", "bbbb", "cccc", "dddd", "eeee"
};

typedef struct _sub_struct
{
   char *txt;
   int num;
} sub_struct;

typedef struct _main_struct
{
   int size;
   sub_struct array[];
} main_struct;

static unsigned int
_type_offset(unsigned base, unsigned size)
{
   unsigned padding;
   if (!(base % size))
     return base;
   padding = abs(base - size);
   return base + padding;
}

static void
_fill_receive_array_of_string_int_with_size(EDBus_Message *msg, int size, const char *array[])
{
   Eina_Value *value_struct, *value_array;
   int i;
   unsigned offset;
   Eina_Value_Struct_Member main_members[2];
   Eina_Value_Struct_Member sub_members[] = {
      EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_STRING, sub_struct, txt),
      EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, sub_struct, num)
   };
   Eina_Value_Struct_Desc desc_sub_struct = {
      EINA_VALUE_STRUCT_DESC_VERSION,
      NULL, // no special operations
      sub_members,
      2,
      sizeof(sub_struct)
   };
   Eina_Value_Struct_Desc desc_struct = {
      EINA_VALUE_STRUCT_DESC_VERSION,
      NULL, // no special operations
      main_members,
      2,
      0//will be set below
   };

   offset = _type_offset(sizeof(int), sizeof(Eina_Value_Array));
   main_members[0].name = "size";
   main_members[0].type = EINA_VALUE_TYPE_INT, 0;
   main_members[0].offset = 0;
   main_members[1].name = "array";
   main_members[1].type = EINA_VALUE_TYPE_ARRAY;
   main_members[1].offset = offset;
   desc_struct.size = offset + sizeof(Eina_Value_Array);

   value_struct = eina_value_struct_new(&desc_struct);
   eina_value_struct_set(value_struct, "size", size);

   value_array = eina_value_array_new(EINA_VALUE_TYPE_STRUCT, size);
   for (i = 0; i < size; i++)
     {
        Eina_Value *value_sub_struct = eina_value_struct_new(&desc_sub_struct);
        Eina_Value_Struct st;
        eina_value_struct_set(value_sub_struct, "txt", array[i]);
        eina_value_struct_set(value_sub_struct, "num", i);
        eina_value_get(value_sub_struct, &st);
        eina_value_array_append(value_array, st);
        eina_value_free(value_sub_struct);
     }
   eina_value_struct_value_set(value_struct, "array", value_array);

   edbus_message_from_eina_value("ia(si)", msg, value_struct);

   eina_value_free(value_struct);
   eina_value_free(value_array);
}

static void
on_send_array(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   Eina_Value *v, array;
   const char *txt;
   unsigned i;
   printf("2 - on_send_array()\n");

   if (edbus_message_error_get(msg, NULL, NULL))
     {
        printf("Message error\n\n");
        return;
     }

   v = edbus_message_to_eina_value(msg);
   eina_value_struct_value_get(v, "arg0", &array);
   for (i = 0; i < eina_value_array_count(&array); i++)
     {
        eina_value_array_get(&array, i, &txt);
        printf("\t%s\n", txt);
     }

   eina_value_free(v);
   eina_value_flush(&array);
}

static void
on_receive_array_with_size(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname;
   const char *errmsg;

   printf("1 - on_receive_array_with_size()\n");
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
     }
}

static void
on_plus_one(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   Eina_Value *v;
   int num2;

   if (edbus_message_error_get(msg, NULL, NULL))
     {
        printf("Message error\n\n");
        return;
     }

   v = edbus_message_to_eina_value(msg);
   eina_value_struct_get(v, "arg0", &num2);

   printf("3 - on_plus_one() %d\n", num2);
   eina_value_free(v);
}

static void
_property_changed(void *data, EDBus_Proxy *proxy, void *event_info)
{
   EDBus_Proxy_Event_Property_Changed *event = event_info;
   const char *name;
   const Eina_Value *value;
   printf("\nproperty changed\n");

   name = event->name;
   value = event->value;

   if (!strcmp(name, "text") || !strcmp(name, "Resp2"))
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
   else if (!strcmp(name, "st"))
     {
        const char *txt;
        eina_value_struct_get(value, "arg0", &txt);
        printf("[%s] %s | ", name, txt);
        eina_value_struct_get(value, "arg1", &txt);
        printf("%s\n", txt);
     }
}

static Eina_Bool
_read_cache(void *data)
{
   EDBus_Proxy *proxy = data;
   const char *txt;
   int num;
   Eina_Value *v;

   v = edbus_proxy_property_local_get(proxy, "text");
   if (v)
     {
        eina_value_get(v, &txt);
        printf("Read cache: [txt] = %s\n", txt);
     }

   v = edbus_proxy_property_local_get(proxy, "int32");
   if (v)
     {
        eina_value_get(v, &num);
        printf("Read cache: [int32] = %d\n", num);
     }

   v = edbus_proxy_property_local_get(proxy, "st");
   if (v)
     {
        eina_value_struct_get(v, "arg0", &txt);
        printf("Read cache: [st] %s | ", txt);
        eina_value_struct_get(v, "arg1", &txt);
        printf("%s\n", txt);
     }

   return EINA_FALSE;
}

static void
_fill_plus_one(EDBus_Message *msg, int num)
{
   Eina_Value *v;
   Eina_Value_Struct_Member main_members[] = {
      {"num", EINA_VALUE_TYPE_INT, 0}
   };
   Eina_Value_Struct_Desc desc_struct = {
         EINA_VALUE_STRUCT_DESC_VERSION,
         NULL, // no special operations
         main_members,
         1,
         sizeof(int)
   };
   v = eina_value_struct_new(&desc_struct);
   eina_value_struct_set(v, "num", num);

   edbus_message_from_eina_value("i", msg, v);

   eina_value_free(v);
}

int
main(void)
{
   EDBus_Connection *conn;
   EDBus_Object *obj;
   EDBus_Proxy *proxy;
   EDBus_Message *msg;

   ecore_init();
   edbus_init();

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);
   obj = edbus_object_get(conn, BUS, PATH);
   proxy = edbus_proxy_get(obj, IFACE);

   msg = edbus_proxy_method_call_new(proxy, "ReceiveArrayOfStringIntWithSize");
   _fill_receive_array_of_string_int_with_size(msg, size_of_array, array_string);
   edbus_proxy_send(proxy, msg, on_receive_array_with_size, NULL, -1);
   edbus_message_unref(msg);

   edbus_proxy_call(proxy, "SendArray", on_send_array, NULL, -1 , "");

   msg = edbus_proxy_method_call_new(proxy, "PlusOne");
   _fill_plus_one(msg, 14);
   edbus_proxy_send(proxy, msg, on_plus_one, NULL, -1);
   edbus_message_unref(msg);

   edbus_proxy_event_callback_add(proxy,
                                  EDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                  _property_changed, NULL);

   edbus_proxy_properties_monitor(proxy, EINA_TRUE);
   ecore_timer_add(10, _read_cache, proxy);

   ecore_main_loop_begin();

   edbus_proxy_event_callback_del(proxy, EDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                  _property_changed, NULL);
   edbus_connection_unref(conn);

   edbus_shutdown();
   ecore_shutdown();
   return 0;
}
