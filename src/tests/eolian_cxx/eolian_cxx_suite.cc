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

#include "eolian_cxx_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
  { "Eolian-Cxx Parsing", eolian_cxx_test_parse },
  { "Eolian-Cxx Wrapper", eolian_cxx_test_wrapper },
  { "Eolian-Cxx Generation", eolian_cxx_test_generate },
  { "Eolian-Cxx Address_of", eolian_cxx_test_address_of },
  { "Eolian-Cxx Inheritance", eolian_cxx_test_inheritance },
  { "Eolian-Cxx Binding", eolian_cxx_test_binding },
  { "Eolian-Cxx Cyclic", eolian_cxx_test_cyclic },
  { "Eolian-Cxx Documentation", eolian_cxx_test_documentation },
  { NULL, NULL }
};

int
main(int argc, char* argv[])
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   putenv(const_cast<char*>("EFL_RUN_IN_TREE=1"));

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Eolian C++", etc, NULL, NULL);

   return (failed_count == 0) ? 0 : 255;
}
