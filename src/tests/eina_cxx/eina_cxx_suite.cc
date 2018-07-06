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
