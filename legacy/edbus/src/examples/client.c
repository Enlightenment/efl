#include "EDBus.h"
#include <Ecore.h>

#define BUS "org.Enlightenment"
#define PATH "/org/enlightenment"
#define INTERFACE "org.enlightenment.Test"
#define NTESTS 7

static int i = 0;
static EDBus_Signal_Handler *sh;

static void
_on_alive(void *context, const EDBus_Message *msg)
{
   printf("Alive\n\n");
}

static void
_on_alive2(void *context, const EDBus_Message *msg)
{
   printf("Alive2\n\n");
   i++;
   if (i == 2)
     edbus_signal_handler_unref(sh);
}

static void
_on_hello(void *context, const EDBus_Message *msg)
{
   char *txt;

   if (edbus_message_arguments_get(msg, "s", &txt))
     printf("%s\n", txt);
}

#define bool_value EINA_TRUE
#define byte_value 0xAA
#define uint32_value 0xFFFFFFFF
#define int32_value 0xFFFFFFFF
#define int16_value 0x0000FFFF
#define double_value 3.1415926
#define string_value "test"

static void
test(void)
{
   static int n = 0;

   n++;
   if (n == NTESTS)
    printf("Passed in all tests\n");
}

static void
_on_send_bool(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;
   Eina_Bool bool;
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "b", &bool))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   if (bool != bool_value) printf("Error on bool\n");
   else test();
}

static void
_on_send_byte(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;
   unsigned char byte;
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "y", &byte))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   if (byte != byte_value) printf("Error on byte\n");
   else test();
}

static void
_on_send_uint32(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;
   unsigned int uint32;
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "u", &uint32))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   if (uint32 != uint32_value) printf("Error on uint32\n");
   else test();
}

static void
_on_send_int32(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;
   int int32;
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "i", &int32))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   if (int32 != (int)int32_value) printf("Error on int32\n");
   else test();
}

static void
_on_send_int16(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;
   short int int16;
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "n", &int16))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   if (int16 != (short int)int16_value) printf("Error on int16\n");
   else test();
}

static void
_on_send_double(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;
   double d;
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "d", &d))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   if (d != double_value) printf("Error on double\n");
   else test();
}

static void
_on_send_string(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;
   char *str;
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "s", &str))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   if (strcmp(str, string_value)) printf("Error on string\n");
   else test();
}

static void
_on_async_test(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;
   char *str;
   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "s", &str))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   printf("%s\n", str);
}

static void
on_name_owner_changed2(void *data, const char *bus, const char *old_id, const char *new_id)
{
   printf("2 - Bus=%s | old=%s | new=%s\n", bus, old_id, new_id);
}

static void
on_name_owner_changed(void *data, const char *bus, const char *old_id, const char *new_id)
{
   printf("Bus=%s | old=%s | new=%s\n", bus, old_id, new_id);
}

static Eina_Bool
add_name_owner2(void *data)
{
   EDBus_Connection *conn = data;
   edbus_name_owner_changed_callback_add(conn, BUS, on_name_owner_changed2,
					 NULL);
   return EINA_FALSE;
}

int
main(void)
{
   EDBus_Connection *conn;
   EDBus_Object *obj;
   EDBus_Proxy *proxy;

   ecore_init();
   edbus_init();

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);

   obj = edbus_object_get(conn, BUS, PATH);
   proxy = edbus_proxy_get(obj, INTERFACE);
   edbus_proxy_signal_handler_add(proxy, "Alive", _on_alive, NULL);
   sh = edbus_proxy_signal_handler_add(proxy, "Alive", _on_alive2, NULL);
   edbus_proxy_signal_handler_add(proxy, "Hello", _on_hello, NULL);

   edbus_proxy_call(proxy, "SendBool", _on_send_bool, NULL, -1, "b", bool_value);
   edbus_proxy_call(proxy, "SendByte", _on_send_byte, NULL, -1, "y", byte_value);
   edbus_proxy_call(proxy, "SendUint32", _on_send_uint32, NULL, -1, "u", uint32_value);
   edbus_proxy_call(proxy, "SendInt32", _on_send_int32, NULL, -1, "i", int32_value);
   edbus_proxy_call(proxy, "SendInt16", _on_send_int16, NULL, -1, "n", int16_value);
   edbus_proxy_call(proxy, "SendDouble", _on_send_double, NULL, -1, "d", double_value);
   edbus_proxy_call(proxy, "SendString", _on_send_string, NULL, -1, "s", string_value);
   edbus_proxy_call(proxy, "AsyncTest", _on_async_test, NULL, -1, "");

   edbus_name_owner_changed_callback_add(conn, BUS, on_name_owner_changed, conn);
   ecore_timer_add(3, add_name_owner2, conn);

   ecore_main_loop_begin();

   edbus_name_owner_changed_callback_del(conn, BUS, on_name_owner_changed, conn);
   edbus_name_owner_changed_callback_del(conn, BUS, on_name_owner_changed2,
                                         NULL);
   edbus_connection_unref(conn);

   edbus_shutdown();
   ecore_shutdown();
   return 0;
}
