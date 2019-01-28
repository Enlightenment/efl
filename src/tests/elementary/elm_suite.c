#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"
#include "../efl_check.h"
#include "suite_helpers.h"

static const Efl_Test_Case etc[] = {
  { "elm_config", elm_test_config },
  { "elm_check", elm_test_check },
  { "elm_colorselector", elm_test_colorselector },
  { "elm_entry", elm_test_entry},
  { "elm_atspi", elm_test_atspi},
  { "elm_button", elm_test_button},
  { "elm_image", elm_test_image},
  { "elm_list", elm_test_list},
  { "elm_photo", elm_test_photo},
  { "elm_actionslider", elm_test_actionslider},
  { "elm_box", elm_test_box},
  { "elm_table", elm_test_table},
  { "elm_thumb", elm_test_thumb},
  { "elm_menu", elm_test_menu},
  { "elm_photocam", elm_test_photocam},
  { "elm_win", elm_test_win},
  { "elm_icon", elm_test_icon},
  { "elm_prefs", elm_test_prefs},
  { "elm_map", elm_test_map},
  { "elm_glview", elm_test_glview},
  { "elm_web", elm_test_web},
  { "elm_toolbar", elm_test_toolbar},
  { "elm_grid", elm_test_grid},
  { "elm_diskselector", elm_test_diskselector},
  { "elm_notify", elm_test_notify},
  { "elm_mapbuf", elm_test_mapbuf},
  { "elm_flip", elm_test_flip},
  { "elm_layout", elm_test_layout},
  { "elm_slider", elm_test_slider},
  { "elm_ctxpopup", elm_test_ctxpopup},
  { "elm_separator", elm_test_separator},
  { "elm_calendar", elm_test_calendar},
  { "elm_inwin", elm_test_inwin},
  { "elm_gengrid", elm_test_gengrid},
  { "elm_radio", elm_test_radio},
  { "elm_scroller", elm_test_scroller},
  { "elm_frame", elm_test_frame},
  { "elm_datetime", elm_test_datetime},
  { "elm_player", elm_test_player},
  { "elm_bg", elm_test_bg},
  { "elm_video", elm_test_video},
  { "elm_segmentcontrol", elm_test_segmentcontrol},
  { "elm_progressbar", elm_test_progressbar},
  { "elm_fileselector", elm_test_fileselector},
  { "elm_fileselector_button", elm_test_fileselector_button},
  { "elm_fileselector_entry", elm_test_fileselector_entry},
  { "elm_hoversel", elm_test_hoversel},
  { "elm_multibuttonentry", elm_test_multibuttonentry},
  { "elm_naviframe", elm_test_naviframe},
  { "elm_popup", elm_test_popup},
  { "elm_bubble", elm_test_bubble},
  { "elm_clock", elm_test_clock},
  { "elm_conformant", elm_test_conformant},
  { "elm_dayselector", elm_test_dayselector},
  { "elm_flipselector", elm_test_flipselector},
  { "elm_genlist", elm_test_genlist},
  { "elm_hover", elm_test_hover},
  { "elm_index", elm_test_index},
  { "elm_label", elm_test_label},
  { "elm_panel", elm_test_panel},
  { "elm_panes", elm_test_panes},
  { "elm_slideshow", elm_test_slideshow},
  { "elm_spinner", elm_test_spinner},
  { "elm_plug", elm_test_plug},
  { "elm_code_file_load", elm_code_file_test_load },
  { "elm_code_file_memory", elm_code_file_test_memory },
  { "elm_code_parse", elm_code_test_parse },
  { "elm_code_syntax", elm_code_test_syntax },
  { "elm_code_text", elm_code_test_text },
  { "elm_code_indent", elm_code_test_indent },
  { "elm_code_basic", elm_code_test_basic },
  { "elm_code_widget", elm_code_test_widget },
  { "elm_code_widget_text", elm_code_test_widget_text },
  { "elm_code_widget_selection", elm_code_test_widget_selection },
  { "elm_code_widget_undo", elm_code_test_widget_undo },
  { "elm_focus", elm_test_focus},
  { "elm_focus_sub", elm_test_focus_sub},
  { "elm_widget_focus", elm_test_widget_focus},
  { NULL, NULL }
};

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   failed_count = suite_setup(EINA_TRUE);

   failed_count += _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Elementary", etc, SUITE_INIT_FN(elm2), SUITE_SHUTDOWN_FN(elm));

   return (failed_count == 0) ? 0 : 255;
}
