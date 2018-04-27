#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST (elm_colorselector_legacy_type_check)
{
   Evas_Object *win, *colorselector;
   const char *type;

   win = win_add(NULL, "colorselector", ELM_WIN_BASIC);

   colorselector = elm_colorselector_add(win);

   type = elm_object_widget_type_get(colorselector);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Colorselector"));

   type = evas_object_type_get(colorselector);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_colorselector"));

}
EFL_END_TEST

EFL_START_TEST (elm_colorselector_palette)
{
   Evas_Object *win, *c;
   unsigned int palette_cnt;

   win = win_add(NULL, "check", ELM_WIN_BASIC);

   c = elm_colorselector_add(win);
   /* Get the count of default palettes */
   palette_cnt = eina_list_count(elm_colorselector_palette_items_get(c));
   evas_object_del(c);

   c = elm_colorselector_add(win);
   ck_assert(eina_list_count(elm_colorselector_palette_items_get(c)) == palette_cnt);
   elm_colorselector_palette_color_add(c, 255, 255, 255, 255);
   ck_assert(eina_list_count(elm_colorselector_palette_items_get(c)) == 1);
   evas_object_del(c);

   c = elm_colorselector_add(win);
   ck_assert(eina_list_count(elm_colorselector_palette_items_get(c)) == palette_cnt);
   evas_object_del(c);

}
EFL_END_TEST

EFL_START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *c;
   Efl_Access_Role role;

   win = win_add(NULL, "colorselector", ELM_WIN_BASIC);

   c = elm_colorselector_add(win);
   role = efl_access_object_role_get(c);

   ck_assert(role == EFL_ACCESS_ROLE_COLOR_CHOOSER);

}
EFL_END_TEST

void elm_test_colorselector(TCase *tc)
{
   tcase_add_test(tc, elm_colorselector_legacy_type_check);
   tcase_add_test(tc, elm_colorselector_palette);
   tcase_add_test(tc, elm_atspi_role_get);
}
