
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eldbus.hh"
#include "Ecore.hh"

#include <algorithm>

#include <iostream>

#include <check.h>

START_TEST(eldbus_cxx_session_connection)
{
  efl::eldbus::eldbus_init init;

  efl::eldbus::connection c(efl::eldbus::session);
}
END_TEST

START_TEST(eldbus_cxx_system_connection)
{
  efl::eldbus::eldbus_init init;

  efl::eldbus::connection c(efl::eldbus::system);
}
END_TEST

void
eldbus_test_connection(TCase* tc)
{
  tcase_add_test(tc, eldbus_cxx_session_connection);
  tcase_add_test(tc, eldbus_cxx_system_connection);
}
