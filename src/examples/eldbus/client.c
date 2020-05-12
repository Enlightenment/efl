//Compile with:
// gcc -o client client.c `pkg-config --cflags --libs eldbus ecore eina`

#include "Eldbus.h"
#include <Ecore.h>

#define BUS "org.Enlightenment"
#define PATH "/org/enlightenment"
#define INTERFACE "org.enlightenment.Test"
#define NTESTS 8

static int _client_log_dom = -1;
#define ERR(...)      EINA_LOG_DOM_ERR(_client_log_dom, __VA_ARGS__)

static Eldbus_Connection *conn = NULL;
static Ecore_Timer *timeout = NULL;

static void
_on_alive(void *context EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED)
{
   printf("Alive\n\n");
}

static void
_on_hello(void *context EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *txt;
   if (eldbus_message_arguments_get(msg, "s", &txt))
     printf("%s\n", txt);
}

#include <inttypes.h>

static struct expected
{
   Eina_Bool b;
   uint8_t y;
   uint32_t u;
   int32_t i;
   int16_t n;
   double d;
   const char *s;
} expected = {
   .b = EINA_TRUE,
   .y = 0xAA,
   .u = 0xFFFFFFFF,
   .i = 0xFFFFFFFF,
   .n = 0xFFFF,
   .d = 3.1415926,
   .s = "test",
};

static void
test(void)
{
   static int n = 0;
   n++;
   if (n >= NTESTS)
     {
        printf("Passed in all tests\nExiting...\n");
        eldbus_connection_unref(conn);
        conn = NULL;
        ecore_main_loop_quit();
     }
   else
    printf("Passed in %d/%d tests\n", n, NTESTS);
}

static void
_on_send_bool(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   Eina_Bool b;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "b", &b))
     {
        ERR("Could not get entry contents");
        return;
     }

   if (b != expected.b)
     {
        ERR("Bool value doesn't match");
        return;
     }

   test();
}

static void
_on_send_byte(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   uint8_t y;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "y", &y))
     {
        ERR("Could not get entry contents");
        return;
     }

   if (y != expected.y)
     {
        ERR("Byte value doesn't match expected value");
        return;
     }

   test();
}

static void
_on_send_uint32(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   unsigned int u;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "u", &u))
     {
        ERR("Could not get entry contents");
        return;
     }

   if (u != expected.u)
     {
        ERR("Uint32 value doesn't match expected value");
        return;
     }

   test();
}

static void
_on_send_int32(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   int32_t i;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "i", &i))
     {
        ERR("Could not get entry contents");
        return;
     }

   if (i != expected.i)
     {
        ERR("Int32 value doesn't match expected value");
        return;
     }

   test();
}

static void
_on_send_int16(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   int16_t n;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "n", &n))
     {
        ERR("Could not get entry contents");
        return;
     }

   if (n != expected.n)
     {
        ERR("Int16 value doesn't match expected value");
        return;
     }

   test();
}

static void
_on_send_double(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   double d;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "d", &d))
     {
        ERR("Could not get entry contents");
        return;
     }

   if (!EINA_DBL_EQ(d, expected.d))
     {
        ERR("Double value doesn't match expected value");
        return;
     }

   test();
}

static void
_on_send_string(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   char *s;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "s", &s))
     {
        ERR("Could not get entry contents");
        return;
     }

   if (strcmp(s, expected.s) != 0)
     {
        ERR("Uint32 value doesn't match expected value");
        return;
     }

   test();
}

static void
_on_async_test(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        return;
     }

   test();
}

static void
on_name_owner_changed(void *data EINA_UNUSED, const char *bus, const char *old_id, const char *new_id EINA_UNUSED)
{
   printf("Bus=%s | old=%s | new=%s\n", bus, old_id, new_id);
}

static Eina_Bool
finish(void *data EINA_UNUSED)
{
   ERR("Timeout\nSome error happened or server is taking too much time to respond.");
   ecore_main_loop_quit();
   timeout = NULL;
   return ECORE_CALLBACK_CANCEL;
}

int
main(void)
{
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;

   eina_init();
   _client_log_dom = eina_log_domain_register("client", EINA_COLOR_CYAN);
   if (_client_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create 'client' log domain");
        goto exit_eina;
     }

   ecore_init();
   eldbus_init();

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);

   obj = eldbus_object_get(conn, BUS, PATH);
   proxy = eldbus_proxy_get(obj, INTERFACE);
   eldbus_proxy_signal_handler_add(proxy, "Alive", _on_alive, NULL);
   eldbus_proxy_signal_handler_add(proxy, "Hello", _on_hello, NULL);

   eldbus_proxy_call(proxy, "SendBool", _on_send_bool, NULL, -1, "b",
                    expected.b);
   eldbus_proxy_call(proxy, "SendByte", _on_send_byte, NULL, -1, "y",
                    expected.y);
   eldbus_proxy_call(proxy, "SendUint32", _on_send_uint32, NULL, -1, "u",
                    expected.u);
   eldbus_proxy_call(proxy, "SendInt32", _on_send_int32, NULL, -1, "i",
                    expected.i);
   eldbus_proxy_call(proxy, "SendInt16", _on_send_int16, NULL, -1, "n",
                    expected.n);
   eldbus_proxy_call(proxy, "SendDouble", _on_send_double, NULL, -1, "d",
                    expected.d);
   eldbus_proxy_call(proxy, "SendString", _on_send_string, NULL, -1, "s",
                    expected.s);
   eldbus_proxy_call(proxy, "AsyncTest", _on_async_test, NULL, -1, "");

   eldbus_name_owner_changed_callback_add(conn, BUS, on_name_owner_changed,
                                         conn, EINA_TRUE);
   timeout = ecore_timer_add(30, finish, NULL);

   ecore_main_loop_begin();

   if (timeout)
      ecore_timer_del(timeout);
   if (conn)
      eldbus_connection_unref(conn);

   eldbus_shutdown();
   ecore_shutdown();

   eina_log_domain_unregister(_client_log_dom);
exit_eina:
   eina_shutdown();

   return 0;
}
