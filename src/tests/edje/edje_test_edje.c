#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#define EFL_GFX_FILTER_BETA
#define EFL_CANVAS_LAYOUT_BETA

#include "edje_suite.h"

#define EVAS_DATA_DIR TESTS_SRC_DIR "/../../lib/evas"

EFL_START_TEST(edje_test_edje_init)
{
}
EFL_END_TEST

EFL_START_TEST(edje_test_edje_load)
{
   Evas *evas = _setup_evas();
   Edje_Load_Error error;
   Evas_Object *obj;

   obj = edje_object_add(evas);
   edje_object_file_set(obj, "blaoeuaeoueoaua.edj", "test");
   error = edje_object_load_error_get(obj);
   fail_if(error != EDJE_LOAD_ERROR_DOES_NOT_EXIST);

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_load_simple_layout)
{
   Evas *evas = _setup_evas();
   Evas_Object *obj;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_layout.edj"), "test_group"));

   fail_if(edje_object_part_exists(obj, "unexistant_part"));
   fail_unless(edje_object_part_exists(obj, "background"));


   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_simple_layout_geometry)
{
   int x, y, w, h;
   int r, g, b, a;
   Evas *evas = _setup_evas();
   Evas_Object *obj;
   const Evas_Object *bg;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_layout.edj"), "test_group"));

   /* rel1.relative: 0.0 0.0;
    * rel2.relative: 1.0 1.0; */
   evas_object_resize(obj, 1000, 1000);
   edje_object_part_geometry_get(obj, "background", &x, &y, &w, &h);

   fail_if(x != 0 || y != 0);
   fail_if(w != 1000 || h != 1000);

   bg = edje_object_part_object_get(obj, "background");
   fail_if(!bg);

   evas_object_color_get(bg, &r, &g, &b, &a);
   fail_if(r != 255 || g != 255 || b != 255 || a != 255);

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_complex_layout)
{
   int x, y, w, h;
   Evas *evas = _setup_evas();
   Evas_Object *obj;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("complex_layout.edj"), "test_group"));
   evas_object_resize(obj, 1000, 1000);

   /* Apparently rel2 offsets are retarded and you have to introduce off-by-one
    * madness (rel2.offset: -1 -1; acts like there is zero offset...)
    * Take that into account when checking w and h */

   /* rel1.relative: 0.0 0.0; rel1.offset: 0 0;
    * rel2.relative: 1.0 1.0; rel2.offset -1 -1; */
   edje_object_part_geometry_get(obj, "background", &x, &y, &w, &h);
   fail_if(x != 0 || y != 0);
   fail_if(w != 1000-1 + 1 || h != 1000-1 + 1);

   /* rel1.relative: 0.0 0.0; rel1.offset: 1 1;
    * rel2.relative: 1.0 1.0; rel2.offset 0 0; */
   edje_object_part_geometry_get(obj, "background2", &x, &y, &w, &h);
   fail_if(x != 1 || y != 1);
   fail_if(w != 1000-1 + 1 || h != 1000-1 + 1);

   /* rel1.relative: 0.0 0.0; rel1.offset: 5 5;
    * rel2.relative: 0.5 0.5; rel2.offset -2 -2; */
   edje_object_part_geometry_get(obj, "ul", &x, &y, &w, &h);
   fail_if(x != 5 || y != 5);
   fail_if(w != 500-5-2 + 1 || h != 500-5-2 + 1);

   /* rel1.relative: 0.5 0.0; rel1.offset: 2 5;
    * rel2.relative: 1.0 0.5; rel2.offset -5 -2; */
   edje_object_part_geometry_get(obj, "ur", &x, &y, &w, &h);
   fail_if(x != 500+2 || y != 5);
   fail_if(w != 500-5-2 + 1 || h != 500-5-2 + 1);

   /* rel1.relative: 0.0 0.5; rel1.offset: 5 2;
    * rel2.relative: 1.0 1.0; rel2.offset -5 -5; */
   edje_object_part_geometry_get(obj, "l", &x, &y, &w, &h);
   fail_if(x != 5 || y != 500+2);
   fail_if(w != 1000-5-5 + 1 || h != 500-5-2 + 1);

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_calculate_parens)
{
   int x, y, w, h;
   Evas *evas = _setup_evas();
   Evas_Object *obj;
   Eina_Rect rect;

   obj = edje_object_add(evas);

   /* A negative test case for efl_layout_calc_parts_extends */
   rect = efl_layout_calc_parts_extends(obj);
   fail_if(rect.w < 0 || rect.h < 0);

   fail_unless(edje_object_file_set(obj, test_layout_get("test_parens.edj"), "test_group"));

   evas_object_resize(obj, 100, 100);
   edje_object_part_geometry_get(obj, "background", &x, &y, &w, &h);
   fail_if(x != 0 || y != 0 || w != 100 || h != 100);

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_access)
{
   Evas *evas = _setup_evas();
   const char *name;
   Evas_Object *obj;
   Eina_Iterator *it;
   Eina_List *list;
   char buf[20];
   int i = 0;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_layout.edj"), "test_group"));

   /* eo api */
   it = efl_canvas_layout_access_part_iterate(obj);
   fail_if(!it);

   EINA_ITERATOR_FOREACH(it, name)
     {
        i++;
        sprintf(buf, "access_%d", i);
        fail_if(!name || strcmp(name, buf) != 0);
     }
   fail_if(i != 2);
   eina_iterator_free(it);

   i = 0;

   /* legacy api */
   list = edje_object_access_part_list_get(obj);
   fail_if(!list);
   EINA_LIST_FREE(list, name)
     {
        i++;
        sprintf(buf, "access_%d", i);
        fail_if(!name || strcmp(name, buf) != 0);
     }
   fail_if(i != 2);

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_combine_keywords)
{
   Evas *evas;
   Evas_Object *obj;

   evas = _setup_evas();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_combine_keywords.edj"), "test_group"));

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_part_caching)
{
   Evas *evas = _setup_evas();
   Evas_Object *ly, *o1, *global_p = NULL;

   ly = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
    efl_file_set(efl_added, test_layout_get("test_swallows.edj"), "test_group")
   );

   for (int i = 0; i < 10; ++i)
     {
        Evas_Object *p;

        p = efl_part(ly, "swallow");
        o1 = efl_content_get(p);
        ck_assert_ptr_ne(o1, NULL);

        if (global_p)
          ck_assert_ptr_eq(global_p, p);
        global_p = p;

        ck_assert_int_eq(efl_ref_count(p), 1);
        ck_assert_ptr_eq(efl_parent_get(p), NULL);

     }

   evas_free(evas);
}
EFL_END_TEST

void edje_test_edje(TCase *tc)
{
   tcase_add_test(tc, edje_test_edje_init);
   tcase_add_test(tc, edje_test_load_simple_layout);
   tcase_add_test(tc, edje_test_edje_load);
   tcase_add_test(tc, edje_test_simple_layout_geometry);
   tcase_add_test(tc, edje_test_complex_layout);
   tcase_add_test(tc, edje_test_calculate_parens);
   tcase_add_test(tc, edje_test_access);
   tcase_add_test(tc, edje_test_combine_keywords);
   tcase_add_test(tc, edje_test_part_caching);
}
