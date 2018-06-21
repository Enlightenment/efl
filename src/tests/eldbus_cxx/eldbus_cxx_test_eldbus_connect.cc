#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <algorithm>
#include <iostream>

#include <Ecore.hh>
#include <Eldbus.hh>

#include "eldbus_cxx_suite.h"

START_TEST(eldbus_cxx_session_connection)
{
  efl::ecore::ecore_init ecore_init;
  efl::eldbus::eldbus_init init;

  efl::eldbus::connection c(efl::eldbus::session);
}
END_TEST

void
eldbus_test_connection(TCase* tc)
{
  tcase_add_test(tc, eldbus_cxx_session_connection);
}
