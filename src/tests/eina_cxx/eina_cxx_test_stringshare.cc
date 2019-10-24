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

#include <functional>

#include <Eina.hh>

#include "eina_cxx_suite.h"

EFL_START_TEST(eina_cxx_stringshare_constructors)
{
  efl::eina::eina_init eina_init;

  efl::eina::stringshare string1;
  ck_assert(string1.empty());

  efl::eina::stringshare string2("string");
  ck_assert(string2.size() == 6);
  ck_assert(string2 == "string");

  efl::eina::stringshare string3(string2);
  ck_assert(string2 == string3);

  efl::eina::stringshare string4(string3.begin(), string3.end());
  ck_assert(string2 == string3);
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_stringshare_iterators)
{
  efl::eina::eina_init eina_init;

  const char str[] = "string";
  const char rstr[] = "gnirts";

  efl::eina::stringshare string(str);
  ck_assert(string.size() == 6);
  ck_assert(string == str);

  ck_assert(std::equal(string.begin(), string.end(), str));
  ck_assert(std::equal(string.rbegin(), string.rend(), rstr));
  ck_assert(std::equal(string.cbegin(), string.cend(), str));
  ck_assert(std::equal(string.crbegin(), string.crend(), rstr));
}
EFL_END_TEST

void
eina_test_stringshare(TCase *tc)
{
  tcase_add_test(tc, eina_cxx_stringshare_constructors);
  tcase_add_test(tc, eina_cxx_stringshare_iterators);
}
