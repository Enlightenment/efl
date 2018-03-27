#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>

#include "eet_suite.h"
#include "../efl_check.h"
#include <Eet.h>

char argv0[PATH_MAX];

static const Efl_Test_Case etc[] = {
  { "Eet Init", eet_test_init },
  { "Eet Data Encoding/Decoding", eet_test_data },
  { "Eet File", eet_test_file },
  { "Eet Image", eet_test_image },
#ifdef HAVE_SIGNATURE
  { "Eet Identity", eet_test_identity },
#endif
#ifdef HAVE_CIPHER
  { "Eet Cipher", eet_test_cipher },
#endif
  { "Eet Cache", eet_test_cache },
  { "Eet Connection", eet_test_connection },
  { NULL, NULL }
};

SUITE_INIT(eet)
{
   ck_assert_int_eq(eet_init(), 1);
}

SUITE_SHUTDOWN(eet)
{
   ck_assert_int_eq(eet_shutdown(), 0);
}

int
main(int argc, char *argv[])
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   memcpy(argv0, argv[0], strlen(argv[0]) + 1);

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Eet", etc, SUITE_INIT_FN(eet), SUITE_SHUTDOWN_FN(eet));

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
