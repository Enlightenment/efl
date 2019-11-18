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

extern "C" {
typedef Eo Cyclic1;
typedef Eo Cyclic2;
}

#include <cyclic1.eo.h>
#include <cyclic2.eo.h>
#include <cyclic1.eo.hh>
#include <cyclic2.eo.hh>

#include "eolian_cxx_suite.h"

EFL_START_TEST(eolian_cxx_test_cyclic_call)
{
}
EFL_END_TEST

void
eolian_cxx_test_cyclic(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_cyclic_call);
}
