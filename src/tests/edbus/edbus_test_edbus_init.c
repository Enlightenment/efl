#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>
#include <EDBus.h>

#include "edbus_suite.h"

static Eina_Bool
_quit_cb(void *data EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(edbus_test_edbus)
{
   int ret;

   ret = edbus_init();
   fail_if(ret != 1);

   ret = edbus_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(edbus_test_edbus_main_loop)
{
   Ecore_Timer *timer;
   int ret;

   ecore_init();
   ret = edbus_init();
   fail_if(ret != 1);

   timer = ecore_timer_add(0.1, _quit_cb, NULL);
   fail_if(timer == NULL);

   ecore_main_loop_begin();

   ret = edbus_shutdown();
   fail_if(ret != 0);

   ecore_shutdown();

}
END_TEST

#if 0
/* TODO: This test assumes an existing dbus-daemon running. It
 * shouldn't do this, instead we should launch dbus-daemon ourselves
 * and create our private instance, use it and then kill it
 * afterwards.
 */

START_TEST(edbus_test_edbus_conn)
{
   EDBus_Connection *conn;
   int ret;

   ecore_init();

   ret = edbus_init();
   fail_if(ret != 1);

   /*
    * let's use connection type == system, so it works without a session,
    * however security rules may apply differently depending on the
    * machine/user
    */
   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SYSTEM);
   fail_if(conn == NULL);

   edbus_connection_unref(conn);

   ret = edbus_shutdown();
   fail_if(ret != 0);

   ecore_shutdown();

}
END_TEST

START_TEST(edbus_test_edbus_conn_object)
{
   EDBus_Connection *conn;
   EDBus_Object *obj;
   int ret;

   ecore_init();

   ret = edbus_init();
   fail_if(ret != 1);

   /*
    * let's use connection type == system, so it works without a D-Bus session.
    * However security rules may apply differently depending on the
    * machine/user
    */
   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SYSTEM);
   fail_if(conn == NULL);

   obj = edbus_object_get(conn, "org.buu", "/org/buu");
   fail_if(obj == NULL);

   edbus_object_unref(obj);

   edbus_connection_unref(conn);

   ret = edbus_shutdown();
   fail_if(ret != 0);

   ecore_shutdown();

}
END_TEST

void name_owner_changed_cb(void *data, const char *bus EINA_UNUSED, const char *old_id EINA_UNUSED, const char *new_id)
{
   const char **id = data;
   *id = new_id;
}

START_TEST(edbus_test_edbus_name_owner_changed)
{
   EDBus_Connection *conn;
   const char *id = NULL;

   ecore_init();
   edbus_init();

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SYSTEM);
   fail_if(conn == NULL);

   edbus_name_owner_changed_callback_add(conn, "org.bus.that.not.exist",
                                         name_owner_changed_cb, &id, EINA_TRUE);
   ecore_timer_add(0.5, _quit_cb, NULL);

   ecore_main_loop_begin();

   fail_if(id == NULL);

   edbus_connection_unref(conn);

   edbus_shutdown();
   ecore_shutdown();
}
END_TEST
#endif

void edbus_test_edbus_init(TCase *tc)
{
   tcase_add_test(tc, edbus_test_edbus);
   tcase_add_test(tc, edbus_test_edbus_main_loop);
#if 0
   tcase_add_test(tc, edbus_test_edbus_conn);
   tcase_add_test(tc, edbus_test_edbus_conn_object);
   tcase_add_test(tc, edbus_test_edbus_name_owner_changed);
#endif
}
