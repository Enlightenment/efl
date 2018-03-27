#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>
#include <Elput.h>

#include "elput_suite.h"

EFL_START_TEST(elput_test_elput_init)
{
}
EFL_END_TEST

void elput_test_elput(TCase *tc)
{
   tcase_add_test(tc, elput_test_elput_init);
}
