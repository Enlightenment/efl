#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Evas.h>

#include "evas_suite.h"

EFL_START_TEST(evas_free_none)
{
   Evas *evas = evas_new();
   Evas_Object *obj = evas_object_rectangle_add(evas);
   ck_assert(obj != NULL);
}
EFL_END_TEST

void evas_test_new(TCase *tc)
{
   tcase_add_test(tc, evas_free_none);
}
