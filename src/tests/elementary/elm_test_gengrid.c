#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

static Elm_Gengrid_Item_Class *gic = NULL;

EFL_START_TEST(elm_gengrid_legacy_type_check)
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

EFL_START_TEST(elm_atspi_role_get)
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

static void
_realized(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   static int i = 0;

   i++;
   //we have two items - each of them are getting realized unrealized and again realized.
   if (i == 4)
     ecore_main_loop_quit();
}

EFL_START_TEST(elm_gengrid_focus)
{
   Evas_Object *win, *grid, *bx, *bt;
   Elm_Object_Item *it;

   win = win_add_focused();

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, 10, 10);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(grid, 600, 600);

   elm_box_pack_end(bx, grid);
   evas_object_show(grid);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";

   elm_gengrid_item_append(grid, gic, NULL, NULL, NULL);
   elm_gengrid_item_append(grid, gic, NULL, NULL, NULL);
   elm_gengrid_item_append(grid, gic, NULL, NULL, NULL);
   it = elm_gengrid_item_append(grid, gic, NULL, NULL, NULL);
   evas_object_smart_callback_add(grid, "realized", _realized, NULL);

   elm_object_item_focus_set(it, EINA_TRUE);
   elm_object_focus_set(grid, EINA_TRUE);

   bt = elm_button_add(win);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 600, 600);
   evas_object_show(win);

   ecore_main_loop_begin();

   ck_assert_ptr_eq(elm_object_focused_item_get(grid), it);
   elm_object_focus_set(bt, EINA_TRUE);
   ck_assert_ptr_eq(elm_object_focused_item_get(grid), NULL);
   ck_assert_int_eq(elm_object_focus_get(grid), EINA_FALSE);
   elm_object_focus_set(grid, EINA_TRUE);
   ck_assert_ptr_eq(elm_object_focused_item_get(grid), it);
   ck_assert_int_eq(elm_object_focus_get(grid), EINA_TRUE);
}
EFL_END_TEST

static void
_gengrid_item_content_test_realize(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static Evas_Object *
_item_content_get(void *data EINA_UNUSED, Evas_Object *obj, const char *part EINA_UNUSED)
{
   Evas_Object *ic = elm_button_add(obj);
   return ic;
}

EFL_START_TEST(elm_gengrid_item_content)
{
   Evas_Object *win, *gengrid;
   Elm_Genlist_Item_Class *gtc;
   Evas_Object *content, *parent;
   Elm_Object_Item *it;

   gtc = elm_gengrid_item_class_new();
   gtc->item_style = "default";
   gtc->func.content_get = _item_content_get;
   gtc->func.state_get = NULL;
   gtc->func.del = NULL;

   win = win_add(NULL, "gengrid", ELM_WIN_BASIC);

   gengrid = elm_gengrid_add(win);
   evas_object_smart_callback_add(gengrid, "realized", _gengrid_item_content_test_realize, NULL);

   it = elm_gengrid_item_append(gengrid, gtc, NULL, NULL, NULL);

   evas_object_resize(gengrid, 100, 100);
   evas_object_resize(win, 150, 150);
   evas_object_show(gengrid);
   evas_object_show(win);

   ecore_main_loop_begin();

   content = elm_object_item_part_content_get(it, "elm.swallow.end");
   parent = elm_object_parent_widget_get(content);
   ck_assert_ptr_eq(parent, gengrid);

   elm_gengrid_item_all_contents_unset(it, NULL);
   parent = elm_object_parent_widget_get(content);
   ck_assert_ptr_eq(parent, win);
}
EFL_END_TEST

void elm_test_gengrid(TCase *tc)
{
   tcase_add_test(tc, elm_gengrid_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_gengrid_focus);
   tcase_add_test(tc, elm_gengrid_item_content);
#if 0
   tcase_add_test(tc, elm_atspi_children_parent);
#endif
}
