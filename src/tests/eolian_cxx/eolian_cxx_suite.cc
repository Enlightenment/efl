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
