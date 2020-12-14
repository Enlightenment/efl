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

#include <Eo.h>
#include <Ecore.h>

#define EOLIANCXXTEST_API
#define EOLIANCXXTEST_API_WEAK

#include <simple.eo.hh>

#include "eolian_cxx_suite.h"

// struct bar
// : efl::eo::inherit<bar, nonamespace::Simple>
// {
//   bar()
//     : inherit_base(efl::eo::parent = nullptr)
//   {}

//   bool simple_get()
//   {
//      printf("calling bar::%s\n", __func__);
//      return false;
//   }
// };

// void foo(nonamespace::Simple is)
// {
//    fail_if(is.simple_get());
// }

// START_TEST(eolian_cxx_test_inheritance_simple)
// {
//   efl::eo::eo_init i;
//   bar b;
//   foo(b);
// }
// END_TEST

void
eolian_cxx_test_inheritance(TCase* /*tc*/)
{
   // tcase_add_test(tc, eolian_cxx_test_inheritance_simple);
}
