/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <algorithm>
#include <iostream>

#include <Ecore.hh>
#include <Eldbus.hh>

#include "eldbus_cxx_suite.h"

EFL_START_TEST(eldbus_cxx_session_connection)
{
  efl::ecore::ecore_init ecore_init;
  efl::eldbus::eldbus_init init;

  efl::eldbus::connection c(efl::eldbus::session);
}
EFL_END_TEST

void
eldbus_test_connection(TCase* tc)
{
  tcase_add_test(tc, eldbus_cxx_session_connection);
}
