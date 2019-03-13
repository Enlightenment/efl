#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_layout_test_legacy_type_check)
{
   Evas_Object *win, *layout;
   const char *type;

   win = win_add(NULL, "layout", ELM_WIN_BASIC);

   layout = elm_layout_add(win);

   type = elm_object_widget_type_get(layout);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Layout"));

   type = evas_object_type_get(layout);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_layout"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *layout;
   Efl_Access_Role role;

   win = win_add(NULL, "layout", ELM_WIN_BASIC);

   layout = elm_layout_add(win);
   role = efl_access_object_role_get(layout);

   ck_assert(role == EFL_ACCESS_ROLE_FILLER);

}
EFL_END_TEST

EFL_START_TEST(elm_layout_test_swallows)
{
   char buf[PATH_MAX];
   Evas_Object *win, *ly, *bt, *bt2;

   win = win_add(NULL, "layout", ELM_WIN_BASIC);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   elm_layout_file_set(ly, buf, "layout");
   evas_object_show(ly);

   bt = elm_button_add(ly);
   fail_if(!elm_layout_content_set(ly, "element1", bt));
   ck_assert_ptr_eq(efl_parent_get(bt), ly);

   bt = elm_object_part_content_unset(ly, "element1");
   ck_assert_ptr_eq(efl_parent_get(bt), evas_object_evas_get(bt));

   fail_if(!elm_layout_content_set(ly, "element1", bt));
   ck_assert_ptr_eq(efl_parent_get(bt), ly);

   bt2 = elm_button_add(ly);
   fail_if(!elm_layout_content_set(ly, "element1", bt2));
   ck_assert_ptr_eq(efl_parent_get(bt2), ly);

}
EFL_END_TEST

void elm_test_layout(TCase *tc)
{
   tcase_add_test(tc, elm_layout_test_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_layout_test_swallows);
}
