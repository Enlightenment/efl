/*
 * Copyright 2026 by its authors. See AUTHORS.
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

/* The typed part accessors are defined in the generated implementation header,
 * which two translation units of this suite include: they have to be inline or
 * the suite does not link. */
#define EFL_CXXPERIMENTAL

#define EFL_PART_PROTECTED
#include <Eo.h>
#include <Eina.h>
#include <Efl.h>
#include <Ecore.h>

#define EOLIANCXXTEST_API
#define EOLIANCXXTEST_API_WEAK

#include <parts_inner.eo.hh>
#include <parts_inner.eo.impl.hh>
#include <parts_owner.eo.hh>
#include <parts_owner.eo.impl.hh>

#include "eolian_cxx_suite.h"

/* Defined in eolian_cxx_test_parts_second.cc, which includes the same headers. */
bool eolian_cxx_test_parts_second(::parts::Owner& owner);

EFL_START_TEST(eolian_cxx_test_parts_typed_accessor)
{
  efl::eo::eo_init init;

  ::parts::Owner owner(efl::eo::instantiate);
  fail_if(owner._eo_ptr() == nullptr);

  {
    /* The accessor answers with the part's own wrapper type. */
    ::parts::Inner inner = owner.inner();
    fail_if(inner._eo_ptr() == nullptr);
    fail_if(!efl_isa(inner._eo_ptr(), PARTS_INNER_CLASS));
  }

  /* The same accessor, from the other translation unit. */
  fail_if(!eolian_cxx_test_parts_second(owner));
}
EFL_END_TEST

void
eolian_cxx_test_parts(TCase* tc)
{
  tcase_add_test(tc, eolian_cxx_test_parts_typed_accessor);
}
