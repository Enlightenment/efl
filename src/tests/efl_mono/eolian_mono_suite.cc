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
#include <config.h>
#endif

#include "../../bin/eolian_mono/eolian/mono/utils.hh"

#include <check.h>
#include "../efl_check.h"



EFL_START_TEST(eolian_mono_test_util_ends_with)
{
  ck_assert(eolian_mono::utils::ends_with("SomeFlags", "Flags"));
  ck_assert(eolian_mono::utils::ends_with("Flags", "Flags"));
  ck_assert(!eolian_mono::utils::ends_with("Flagz", "Flags"));
  ck_assert(!eolian_mono::utils::ends_with("FlagsSome", "Flags"));
}
EFL_END_TEST

void
eolian_mono_utils_test(TCase* tc)
{
   tcase_add_test(tc, eolian_mono_test_util_ends_with);
}

static const Efl_Test_Case etc[] = {
  { "Eolian mono utils", eolian_mono_utils_test },
  { NULL, NULL }
};

int main(int argc, char *argv[])
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   putenv(const_cast<char*>("EFL_RUN_IN_TREE=1"));

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Eolian C++", etc, NULL, NULL);

   return (failed_count == 0) ? 0 : 255;
}
