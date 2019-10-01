#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#define EFL_GFX_FILTER_BETA
#define EFL_CANVAS_LAYOUT_BETA

#include "edje_suite.h"
#ifdef _WIN32
# include <windows.h>
#endif
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

}
EFL_END_TEST

static void
_callback(void *data, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
   int *called = data;
   ck_assert_int_eq(*called, 0);
   *called = 1;
}

EFL_START_TEST(edje_test_edje_reload)
{
   Evas *evas = _setup_evas();
   Evas_Object *obj, *rect;
   int called = 0;
   const char *layout = test_layout_get("test_swallows.edj");

   obj = edje_object_add(evas);
   edje_object_signal_callback_add(obj, "load", "", _callback, &called);
   fail_unless(edje_object_file_set(obj, layout, "test_group"));
   rect = evas_object_rectangle_add(evas);
   ck_assert(edje_object_part_swallow(obj, "swallow", rect));
   edje_object_message_signal_process(obj);
   /* load should be called */
   ck_assert_int_eq(called, 1);

   called = 0;
   fail_unless(edje_object_file_set(obj, layout, "test_group"));
   edje_object_message_signal_process(obj);
   /* load should NOT be called */
   ck_assert_int_eq(called, 0);

#ifdef _WIN32
   HANDLE handle;
   FILETIME modtime;
   SYSTEMTIME st;
   wchar_t date[80], time[80];
   handle = CreateFile(layout,
                       GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                       NULL);
   ck_assert(handle != INVALID_HANDLE_VALUE);
   GetSystemTime(&st);
   ck_assert(GetDateFormatW(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, date, sizeof(date) / sizeof(date[0])));
   ck_assert(GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &st, NULL, time, sizeof(time) / sizeof(time[0])));
   ck_assert(SystemTimeToFileTime(&st, &modtime));
   ck_assert(SetFileTime(handle, NULL, NULL, &modtime));
   CloseHandle(handle);
#else
   struct timespec t[2] = {0};
   t[0].tv_nsec = t[1].tv_nsec = UTIME_NOW;
   ck_assert(!utimensat(0, layout, t, 0));
#endif

   called = 0;
   fail_unless(edje_object_file_set(obj, layout, "test_group"));
   edje_object_message_signal_process(obj);
   /* layout mtime has changed; load should be called */
   ck_assert_int_eq(called, 1);

   /* verify that the object has actually loaded */
   ck_assert_ptr_eq(edje_object_part_swallow_get(obj, "swallow"), rect);
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

}
EFL_END_TEST

EFL_START_TEST(edje_test_combine_keywords)
{
   Evas *evas;
   Evas_Object *obj;

   evas = _setup_evas();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_combine_keywords.edj"), "test_group"));

}
EFL_END_TEST

EFL_START_TEST(edje_test_part_caching)
{
   Evas *evas = _setup_evas();
   Evas_Object *ly, *o1, *global_p = NULL;

   ly = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
    efl_file_set(efl_added, test_layout_get("test_swallows.edj")),
    efl_file_key_set(efl_added, "test_group")
   );

   for (int i = 0; i < 10; ++i)
     {
        Evas_Object *p;

        p = efl_part(ly, "swallow");
        o1 = efl_content_get(p);
        ck_assert_ptr_eq(o1, NULL);

        if (global_p)
          ck_assert_ptr_eq(global_p, p);
        global_p = p;

        ck_assert_int_eq(efl_ref_count(p), 1);
        ck_assert_ptr_eq(efl_parent_get(p), NULL);

     }

}
EFL_END_TEST

void edje_test_edje(TCase *tc)
{
   tcase_add_test(tc, edje_test_edje_init);
   tcase_add_test(tc, edje_test_load_simple_layout);
   tcase_add_test(tc, edje_test_edje_load);
   tcase_add_test(tc, edje_test_edje_reload);
   tcase_add_test(tc, edje_test_simple_layout_geometry);
   tcase_add_test(tc, edje_test_complex_layout);
   tcase_add_test(tc, edje_test_calculate_parens);
   tcase_add_test(tc, edje_test_access);
   tcase_add_test(tc, edje_test_combine_keywords);
   tcase_add_test(tc, edje_test_part_caching);
}
