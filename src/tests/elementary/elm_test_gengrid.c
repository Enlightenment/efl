#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"
#include "elm_test_helper.h"

EFL_START_TEST (elm_gengrid_legacy_type_check)
{
   Evas_Object *win, *gengrid;
   const char *type;

   win = win_add(NULL, "gengrid", ELM_WIN_BASIC);

   gengrid = elm_gengrid_add(win);

   type = elm_object_widget_type_get(gengrid);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Gengrid"));

   type = evas_object_type_get(gengrid);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_gengrid"));

}
EFL_END_TEST

EFL_START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *gengrid;
   Efl_Access_Role role;

   win = win_add(NULL, "gengrid", ELM_WIN_BASIC);

   gengrid = elm_gengrid_add(win);
   role = efl_access_object_role_get(gengrid);

   ck_assert(role == EFL_ACCESS_ROLE_TREE_TABLE);

}
EFL_END_TEST

// Temporary commnted since gengrid fields_update function do not call content callbacks
// (different behaviour then genlist - which calls)
#if 0
static Evas_Object *content;

static Evas_Object *
gl_content_get(void *data EINA_UNUSED, Evas_Object *obj, const char *part EINA_UNUSED)
{
   content = elm_gengrid_add(obj);
   evas_object_show(content);
   return content;
}

/**
 * Validate if gengrid implementation properly reset AT-SPI parent to Elm_Gengrid_Item
 * from Elm_Gengrid
 */
EFL_START_TEST(elm_atspi_children_parent)
{
   elm_config_atspi_mode_set(EINA_TRUE);
   static Elm_Gengrid_Item_Class itc;

   Evas_Object *win = win_add(NULL, "gengrid", ELM_WIN_BASIC);
   evas_object_resize(win, 100, 100);
   Evas_Object *gengrid = elm_gengrid_add(win);
   evas_object_resize(gengrid, 100, 100);

   Efl_Access_Object *parent;
   content = NULL;

   itc.item_style = "default";
   itc.func.content_get = gl_content_get;

   evas_object_show(win);
   evas_object_show(gengrid);

   Elm_Object_Item *it = elm_gengrid_item_append(gengrid, &itc, NULL, NULL, NULL);
   elm_gengrid_item_fields_update(it, "*.", ELM_GENGRID_ITEM_FIELD_CONTENT);

   ck_assert(content != NULL);
   parent = efl_provider_find(efl_parent_get(content), EFL_ACCESS_OBJECT_MIXIN);
   ck_assert(it == parent);

}
EFL_END_TEST
#endif

void elm_test_gengrid(TCase *tc)
{
   tcase_add_test(tc, elm_gengrid_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
#if 0
   tcase_add_test(tc, elm_atspi_children_parent);
#endif
}
