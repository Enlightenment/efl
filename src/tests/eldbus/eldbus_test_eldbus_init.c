#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>
#include <Eldbus.h>

#include "eldbus_suite.h"

static Eina_Bool
_quit_cb(void *data EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EINA_FALSE;
}

EFL_START_TEST(eldbus_test_eldbus)
{
}
EFL_END_TEST

EFL_START_TEST(eldbus_test_eldbus_conn)
{
   Eldbus_Connection *conn;

   /*
    * let's use connection type == system, so it works without a session,
    * however security rules may apply differently depending on the
    * machine/user
    */
   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   fail_if(conn == NULL);

   eldbus_connection_unref(conn);
}
EFL_END_TEST

EFL_START_TEST(eldbus_test_eldbus_conn_object)
{
   Eldbus_Connection *conn;
   Eldbus_Object *obj;

   /*
    * let's use connection type == system, so it works without a D-Bus session.
    * However security rules may apply differently depending on the
    * machine/user
    */
   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   fail_if(conn == NULL);

   obj = eldbus_object_get(conn, "org.buu", "/org/buu");
   fail_if(obj == NULL);

   eldbus_object_unref(obj);

   eldbus_connection_unref(conn);

}
EFL_END_TEST

void name_owner_changed_cb(void *data, const char *bus EINA_UNUSED, const char *old_id EINA_UNUSED, const char *new_id)
{
   const char **id = data;
   *id = new_id;
}

EFL_START_TEST(eldbus_test_eldbus_name_owner_changed)
{
   Eldbus_Connection *conn;
   const char *id = NULL;

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   fail_if(conn == NULL);

   eldbus_name_owner_changed_callback_add(conn, "org.bus.that.not.exist",
                                         name_owner_changed_cb, &id, EINA_TRUE);
   ecore_timer_add(0.5, _quit_cb, NULL);

   ecore_main_loop_begin();

   fail_if(id == NULL);

   eldbus_connection_unref(conn);

}
EFL_END_TEST

void eldbus_test_eldbus_init(TCase *tc)
{
   tcase_add_test(tc, eldbus_test_eldbus);
   tcase_add_test(tc, eldbus_test_eldbus_conn);
   tcase_add_test(tc, eldbus_test_eldbus_conn_object);
   tcase_add_test(tc, eldbus_test_eldbus_name_owner_changed);
}
