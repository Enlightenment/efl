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

#include <Eo.hh>

#define EOLIANCXXTEST_API
#define EOLIANCXXTEST_API_WEAK

#include <fnptr.eo.hh>
#include <fnptr.eo.impl.hh>

#include "eolian_cxx_suite.h"

/* A callback returning a string converts what the lambda returns, which
 * takes the C type, the C++ type and the ownership. */
EFL_START_TEST(eolian_cxx_test_callbacks_return_string)
{
  efl::eo::eo_init init;

  nonamespace::Fnptr obj(efl::eo::instantiate);

  std::string r = obj.text([](int x) { return x ? "some" : "none"; }, 7);

  ck_assert_str_eq(r.c_str(), "some");
}
EFL_END_TEST

/* A method taking two function pointers: each gets a template parameter of
 * its own, so two callables of different types can be given. */
EFL_START_TEST(eolian_cxx_test_callbacks_two_types)
{
  efl::eo::eo_init init;

  nonamespace::Fnptr obj(efl::eo::instantiate);
  int offset = 100;
  int calls = 0;

  int r = obj.apply([&calls](int x) { ++calls; return x + 1; },
                    [&calls, offset](int x) { ++calls; return x * 2 + offset; },
                    10);

  ck_assert_int_eq(calls, 2);
  ck_assert_int_eq(r, 11 + 120);
}
EFL_END_TEST

void
eolian_cxx_test_callbacks(TCase* tc)
{
  tcase_add_test(tc, eolian_cxx_test_callbacks_return_string);
  tcase_add_test(tc, eolian_cxx_test_callbacks_two_types);
}
