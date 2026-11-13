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

#include <a.eo.hh>
#include <a.eo.impl.hh>
#include <b.eo.hh>
#include <b.eo.impl.hh>
#include <c.eo.hh>
#include <c.eo.impl.hh>

#include "eolian_cxx_suite.h"

EFL_START_TEST(eolian_cxx_test_wrapper_size)
{
  efl::eo::eo_init init;

  ::efl::eo::concrete b(nullptr);
  ::nonamespace::A c;

  fail_if(sizeof(b) != sizeof(Eo*));
  fail_if(sizeof(b) != sizeof(c));
}
EFL_END_TEST

/* A const object of a derived wrapper converts to a by-value base wrapper. */
static Eo*
take_base(nonamespace::A a)
{
  return a._eo_ptr();
}

EFL_START_TEST(eolian_cxx_test_wrapper_converts_to_base)
{
  efl::eo::eo_init init;

  /* Real objects: a default constructed wrapper holds no Eo and would make
   * every comparison below trivially equal. */
  nonamespace::C c_obj(efl::eo::instantiate);
  fail_if(c_obj._eo_ptr() == nullptr);
  nonamespace::C const& c_const = c_obj;
  nonamespace::B const& b_const = c_obj;

  fail_if(take_base(c_const) != c_obj._eo_ptr());
  fail_if(take_base(c_obj) != c_obj._eo_ptr());
  fail_if(take_base(b_const) != c_obj._eo_ptr());

  nonamespace::C c_temp(efl::eo::instantiate);
  Eo* temp_ptr = c_temp._eo_ptr();
  fail_if(take_base(std::move(c_temp)) != temp_ptr);
  fail_if(take_base(nonamespace::C(efl::eo::instantiate)) == nullptr);

  nonamespace::A const& r = c_const;
  fail_if(r._eo_ptr() != c_obj._eo_ptr());
}
EFL_END_TEST

void
eolian_cxx_test_wrapper(TCase* tc)
{
  tcase_add_test(tc, eolian_cxx_test_wrapper_size);
  tcase_add_test(tc, eolian_cxx_test_wrapper_converts_to_base);
}
