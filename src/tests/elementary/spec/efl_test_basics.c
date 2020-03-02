#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Ui.Widget",
       "test-widgets": [
             "Efl.Ui.Image",
             "Efl.Ui.Image_Zoomable",
             "Efl.Ui.Box",
             "Efl.Ui.Clock",
             "Efl.Ui.Layout",
             "Efl.Ui.List_View",
             "Efl.Ui.Tab_Bar",
             "Efl.Ui.Tags",
             "Efl.Ui.Textbox",
             "Efl.Ui.Panel",
             "Efl.Ui.Scroller",
             "Efl.Ui.Slider",
             "Efl.Ui.Calendar",
             "Efl.Ui.Check",
             "Efl.Ui.Panes",
             "Efl.Ui.Grid",
             "Efl.Ui.List",
             "Efl.Ui.Spin",
             "Efl.Ui.Spin_Button",
             "Efl.Ui.Spotlight.Container",
             "Efl.Ui.Popup",
             "Efl.Ui.Alert_Popup",
             "Efl.Ui.Slider_Interval",
             "Efl.Ui.Frame",
             "Efl.Ui.Progressbar",
             "Efl.Ui.Video",
             "Efl.Ui.Navigation_Layout",
             "Efl.Ui.Bg",
             "Efl.Ui.Datepicker",
             "Efl.Ui.Grid_Default_Item",
             "Efl.Ui.List_Default_Item",
             "Efl.Ui.List_Placeholder_Item",
             "Efl.Ui.Tab_Page",
             "Efl.Ui.Timepicker",
             "Efl.Ui.Navigation_Bar",
             "Efl.Ui.Relative_Container",
             "Efl.Ui.Vg_Animation",
             "Efl.Ui.Table",
             "Efl.Ui.Flip",
             "Efl.Ui.Stack",
             "Efl.Ui.Pager",
             "Efl.Ui.Separator"
             ],
       "custom-mapping" : {
          "Efl.Ui.Grid" : "EFL_UI_GRID_DEFAULT_ITEM_CLASS",
          "Efl.Ui.List" : "EFL_UI_LIST_DEFAULT_ITEM_CLASS",
          "Efl.Ui.Tab_Bar" : "EFL_UI_TAB_BAR_DEFAULT_ITEM_CLASS"
        }
       }

   spec-meta-end */

EFL_START_TEST(no_leaking_canvas_object)
{
   Eina_List *not_invalidate = NULL;
   Eina_Iterator *iter = eo_objects_iterator_new();
   Eo *obj;

   if (efl_isa(widget, EFL_UI_FLIP_CLASS))
     {
        //FIXME Flip needs more work for this. However, flip should be redone as a spotlight manager, When this is done, we can add these classes to the check here.
        eina_iterator_free(iter);
        return;
     }

   EINA_ITERATOR_FOREACH(iter, obj)
     {
        if (!efl_alive_get(obj)) continue;
        if (!efl_isa(obj, EFL_CANVAS_OBJECT_CLASS)) continue;

        not_invalidate = eina_list_append(not_invalidate, obj);
     }
   eina_iterator_free(iter);

   //Just overwrite the widget pointer, and expect errors, if any error is happening here, we are not interested in it, another testcase will take care of them
   EXPECT_ERROR_START;
   widget = efl_add(widget_klass, win);
   expect_error_start = EINA_TRUE;
   EXPECT_ERROR_END;

   //now try to will those widgets
   if (efl_isa(widget, EFL_PACK_LINEAR_INTERFACE))
     {
        for (int i = 0; i < 30; ++i)
          {
             Efl_Ui_Widget *w = create_test_widget();
             efl_pack_end(widget, w);
          }
     }
   else if (efl_isa(widget, EFL_CONTENT_INTERFACE))
     {
        efl_content_set(widget, create_test_widget());
     }
   else if (efl_isa(widget, EFL_TEXT_INTERFACE))
     {
        efl_text_set(widget, "Test Things!");
     }
   //now reference things, and delete the widget again. This means, the widget will be invalidated.
   efl_ref(widget);
   efl_del(widget);
   ck_assert_int_eq(efl_alive_get(widget), EINA_FALSE);
   ck_assert_int_eq(efl_ref_count(widget), 1);
   iter = eo_objects_iterator_new();

   //now check if there are leaked objects from the widget
   EINA_ITERATOR_FOREACH(iter, obj)
     {
        if (!efl_alive_get(obj)) continue;
        if (!efl_isa(obj, EFL_CANVAS_OBJECT_CLASS)) continue;

        if (eina_list_data_find(not_invalidate, obj) == NULL)
          {
             ck_abort_msg("Leak detected %s Evas-Parent: %s", efl_debug_name_get(obj), efl_class_name_get(efl_canvas_object_render_parent_get(obj)));
          }
     }
   eina_iterator_free(iter);

   efl_unref(widget);
}
EFL_END_TEST


EFL_START_TEST(no_err_on_creation)
{
   widget = efl_add(widget_klass, win);
}
EFL_END_TEST

EFL_START_TEST(no_err_on_shutdown)
{
   efl_ref(widget);
   efl_del(widget);
   efl_unref(widget);
}
EFL_END_TEST

static Eina_Bool
is_this_visible(Eo *obj)
{
   Eina_Size2D sz;

   if (!efl_gfx_entity_visible_get(obj))
     return EINA_FALSE;

   sz = efl_gfx_entity_size_get(obj);
   if ((!sz.w) || (!sz.h)) return EINA_FALSE;

   for (Eo *clipper = efl_canvas_object_clipper_get(obj); clipper ; clipper = efl_canvas_object_clipper_get(clipper))
     {
        int r,g,b,a;

        efl_gfx_color_get(clipper, &r, &g, &b, &a);

        if (!efl_gfx_entity_visible_get(clipper)) return EINA_FALSE;

        if (r == 0 && g == 0 && b == 0) return EINA_FALSE;
        if (a == 0) return EINA_FALSE;
        sz = efl_gfx_entity_size_get(clipper);
        if ((!sz.w) || (!sz.h)) return EINA_FALSE;
     }
   return EINA_TRUE;
}

EFL_START_TEST(correct_visibility_setting)
{
   Eo *checker = NULL;

   if (efl_isa(widget, EFL_UI_FLIP_CLASS)) return; //FIXME Flip needs more work for this. However, flip should be redone as a spotlight manager, When this is done, we can add these classes to the check here.

   efl_gfx_entity_size_set(widget, EINA_SIZE2D(200, 200));
   efl_gfx_hint_size_min_set(widget, EINA_SIZE2D(200, 200));
   efl_gfx_entity_visible_set(widget, EINA_TRUE);
   checker = create_test_widget();
   efl_gfx_hint_size_min_set(checker, EINA_SIZE2D(100, 100));

   if (efl_isa(widget, EFL_PACK_INTERFACE))
     {
         efl_pack(widget, checker);
     }
   else if (efl_isa(widget, EFL_CONTENT_INTERFACE))
     {
        efl_content_set(widget, checker);
     }
   else
     return;

   get_me_to_those_events(widget);

   if (checker)
     ck_assert_int_eq(is_this_visible(checker), EINA_TRUE);

   efl_gfx_entity_visible_set(widget, EINA_FALSE);
   get_me_to_those_events(widget);
   if (checker)
     ck_assert_int_eq(is_this_visible(checker), EINA_FALSE);

   efl_gfx_entity_visible_set(widget, EINA_TRUE);
   get_me_to_those_events(widget);
   if (checker)
     ck_assert_int_eq(is_this_visible(checker), EINA_TRUE);
}
EFL_END_TEST
void
efl_ui_widget_behavior_test(TCase *tc)
{
   tcase_add_test(tc, no_leaking_canvas_object);
   tcase_add_test(tc, no_err_on_shutdown);
   tcase_add_test(tc, correct_visibility_setting);
   tcase_add_test(tc, no_err_on_creation);
}
