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

#include "eina_cxx_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
   { "Ptr_List", eina_test_ptrlist },
   { "Ptr_Array", eina_test_ptrarray },
   { "Inlist", eina_test_inlist },
   { "Inarray", eina_test_inarray },
   { "Iterator", eina_test_iterator },
   { "Stringshare", eina_test_stringshare },
   { "Error", eina_test_error },
   { "Accessor", eina_test_accessor },
   { "Thread", eina_test_thread },
   { "Optional", eina_test_optional },
   { "Value", eina_test_value },
   { "Log", eina_test_log },
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
                                           "Eina C++", etc, NULL, NULL);

   return (failed_count == 0) ? 0 : 255;
}
