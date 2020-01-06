#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "efl_ui_suite.h"
#include "eo_internal.h"

static const Efl_Test_Case etc[] = {
  //{ "elm_focus", elm_test_focus},
  //{ "elm_focus_sub", elm_test_focus_sub},
  //{ "elm_widget_focus", elm_test_widget_focus},
  { "efl_ui_atspi", efl_ui_test_atspi},
  { "efl_ui_callback", efl_ui_test_callback},
  { "efl_ui_config", efl_ui_test_config},
  { "efl_ui_focus", efl_ui_test_focus},
  { "efl_ui_focus_sub", efl_ui_test_focus_sub},
  { "efl_ui_gesture", efl_ui_test_gesture},
  { "efl_ui_box", efl_ui_test_box},
  { "efl_ui_box_flow", efl_ui_test_box_flow},
  { "efl_ui_box_stack", efl_ui_test_box_stack},
  { "efl_ui_table", efl_ui_test_table},
  { "efl_ui_grid", efl_ui_test_grid},
  { "efl_ui_relative_container", efl_ui_test_relative_container},
  { "efl_ui_image", efl_ui_test_image},
  { "efl_ui_image_zoomable", efl_ui_test_image_zoomable},
  { "efl_ui_layout", efl_ui_test_layout},
  { "Efl_Ui_Model", efl_ui_model },
  { "efl_ui_widget", efl_ui_test_widget },
  { "efl_ui_spotlight", efl_ui_test_spotlight},
  { "efl_ui_check", efl_ui_test_check },
  { "efl_ui_popup", efl_ui_test_popup },
  { "efl_ui_progressbar", efl_ui_test_progressbar },
  { "efl_ui_radio_group", efl_ui_test_radio_group },
  { "efl_ui_scroller", efl_ui_test_scroller },
  { "efl_ui_slider", efl_ui_test_slider },
  { "efl_ui_win", efl_ui_test_win },
  { "efl_ui_spin", efl_ui_test_spin },
  { "efl_ui_spin_button", efl_ui_test_spin_button },
  { "efl_ui_collection", efl_ui_test_item_container },
  { "efl_ui_grid_container", efl_ui_test_grid_container },
  { "efl_ui_list_container", efl_ui_test_list_container },
  { "efl_ui_collection_view", efl_ui_test_collection_view },
  { "efl_ui_select_model", efl_ui_test_select_model },
  { "efl_ui_view_model", efl_ui_test_view_model },
  { "efl_ui_group_item", efl_ui_test_group_item },
  { "efl_ui_text", efl_ui_test_text},
#ifdef BUILD_VG_LOADER_JSON
  { "efl_ui_vg_animation", efl_ui_test_vg_animation},
#endif
  { NULL, NULL }
};

EFL_CLASS_SIMPLE_CLASS(efl_ui_widget, "Efl.Ui.Widget", EFL_UI_WIDGET_CLASS);

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   failed_count = suite_setup(EINA_FALSE);

   failed_count += _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Efl_Ui", etc, SUITE_INIT_FN(elm2), SUITE_SHUTDOWN_FN(elm));

   return (failed_count == 0) ? 0 : 255;
}
