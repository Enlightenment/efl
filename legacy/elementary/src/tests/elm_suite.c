#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

typedef struct _Elementary_Test_Case Elementary_Test_Case;

struct _Elementary_Test_Case
{
   const char *test_case;
   void      (*build)(TCase *tc);
};

static const Elementary_Test_Case etc[] = {
  { "Elementary", elm_test_init },
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
  { NULL, NULL }
};

Suite *
elm_suite()
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Elementary");

   for (i = 0; etc[i].test_case; ++i)
     {
        tc = tcase_create(etc[i].test_case);
        etc[i].build(tc);
        suite_add_tcase(s, tc);
        tcase_set_timeout(tc, 0);
     }

   return s;
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   int failed_count;
   Suite *s;
   SRunner *sr;

   s = elm_suite();
   sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");

   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? 0 : 255;
}
