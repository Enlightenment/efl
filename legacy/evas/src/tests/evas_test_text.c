
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "evas_suite.h"
#include "Evas.h"
#include "evas_tests_helpers.h"

#define START_TEXT_TEST() \
   Evas *evas; \
   Evas_Object *to; \
   evas = EVAS_TEST_INIT_EVAS(); \
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO); \
   to = evas_object_text_add(evas); \
do \
{ \
} \
while (0)

#define END_TEXT_TEST() \
do \
{ \
   evas_object_del(to); \
   evas_free(evas); \
   evas_shutdown(); \
} \
while (0)

START_TEST(evas_text_simple)
{
   START_TEXT_TEST();
   const char *buf = "Test - בדיקה";
   evas_object_text_text_set(to, buf);
   fail_if(strcmp(evas_object_text_text_get(to), buf));
   END_TEXT_TEST();
}
END_TEST

void evas_test_text(TCase *tc)
{
   tcase_add_test(tc, evas_text_simple);
}
