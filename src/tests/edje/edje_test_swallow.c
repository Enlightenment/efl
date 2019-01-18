#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#define EFL_GFX_FILTER_BETA
#define EFL_CANVAS_LAYOUT_BETA

#include "edje_suite.h"

#define EVAS_DATA_DIR TESTS_SRC_DIR "/../../lib/evas"


EFL_START_TEST(edje_test_swallows)
{
   Evas *evas = _setup_evas();
   Evas_Object *ly, *o1, *o2;

   ly = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas);
   fail_unless(edje_object_file_set(ly, test_layout_get("test_swallows.edj"), "test_group"));

   fail_unless(edje_object_part_exists(ly, "swallow"));


   o1 = efl_add(EFL_CANVAS_LAYOUT_CLASS, ly);
   fail_if(!edje_object_part_swallow(ly, "swallow", o1));
   ck_assert_ptr_eq(efl_parent_get(o1), ly);

   edje_object_part_unswallow(ly, o1);
   ck_assert_ptr_eq(efl_parent_get(o1), evas_object_evas_get(o1));

   fail_if(!edje_object_part_swallow(ly, "swallow", o1));
   ck_assert_ptr_eq(efl_parent_get(o1), ly);

   o2 = efl_add(EFL_CANVAS_LAYOUT_CLASS, ly);
   fail_if(!edje_object_part_swallow(ly, "swallow", o2));
   ck_assert_ptr_eq(efl_parent_get(o2), ly);
   /* o1 is deleted at this point. */
   ck_assert_ptr_eq(efl_parent_get(o1), evas_object_evas_get(o1));

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_swallows_lifetime)
{
   Evas *evas = _setup_evas();
   Evas_Object *ly, *o1;

   ly = edje_object_add(evas);
   fail_unless(edje_object_file_set(ly, test_layout_get("test_swallows.edj"), "test_group"));

   fail_unless(edje_object_part_exists(ly, "swallow"));

   o1 = edje_object_add(evas);
   fail_if(!edje_object_part_swallow(ly, "swallow", o1));

   evas_object_del(ly);
   fail_if(!efl_parent_get(o1));

   evas_free(evas);
}
EFL_END_TEST

static void
edje_test_swallows_invalidate_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   ck_assert(data == edje_object_part_swallow_get(obj, "swallow"));
}

EFL_START_TEST(edje_test_swallows_invalidate)
{
   Evas *evas = _setup_evas();
   Evas_Object *ly, *o1;

   ly = edje_object_add(evas);
   fail_unless(edje_object_file_set(ly, test_layout_get("test_swallows.edj"), "test_group"));

   fail_unless(edje_object_part_exists(ly, "swallow"));

   o1 = edje_object_add(evas);
   fail_if(!edje_object_part_swallow(ly, "swallow", o1));

   evas_object_event_callback_add(ly, EVAS_CALLBACK_DEL, edje_test_swallows_invalidate_del, o1);

   evas_object_del(ly);
   fail_if(!efl_parent_get(o1));

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_swallows_eoapi)
{
   Evas *evas = _setup_evas();
   Evas_Object *ly, *o1, *o2;

   ly = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas);
   fail_unless(edje_object_file_set(ly, test_layout_get("test_swallows.edj"), "test_group"));

   fail_unless(edje_object_part_exists(ly, "swallow"));


   o1 = efl_add(EFL_CANVAS_LAYOUT_CLASS, ly);
   fail_if(!efl_content_set(efl_part(ly, "swallow"), o1));
   ck_assert_ptr_eq(efl_parent_get(o1), ly);

   efl_content_remove(ly, o1);
   ck_assert_ptr_eq(efl_parent_get(o1), evas_object_evas_get(o1));

   fail_if(!efl_content_set(efl_part(ly, "swallow"), o1));
   ck_assert_ptr_eq(efl_parent_get(o1), ly);

   o2 = efl_add(EFL_CANVAS_LAYOUT_CLASS, ly);
   fail_if(!efl_content_set(efl_part(ly, "swallow"), o2));
   ck_assert_ptr_eq(efl_parent_get(o2), ly);
   /* o1 is deleted at this point. */
   ck_assert_ptr_eq(efl_parent_get(o1), evas_object_evas_get(o1));

   evas_free(evas);
}
EFL_END_TEST


void edje_test_swallow(TCase *tc)
{
   tcase_add_test(tc, edje_test_swallows);
   tcase_add_test(tc, edje_test_swallows_lifetime);
   tcase_add_test(tc, edje_test_swallows_invalidate);
   tcase_add_test(tc, edje_test_swallows_eoapi);
}
