#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

typedef struct _Simple_Test_Widget
{
   Evas_Object* (*constructor)(Evas_Object *win);
   const char *name;
} Simple_Test_Widget;


static const Simple_Test_Widget simple_widgets[] = {
  {elm_flip_add, "flip"},
  {elm_frame_add, "frame"},
  {elm_player_add, "player"},
//  {elm_video_add, "video"},
  {elm_ctxpopup_add, "ctxpopup"},
  {elm_fileselector_add, "fileselector"},
  {elm_hoversel_add, "hoversel"},
  {elm_multibuttonentry_add, "multibuttonentry"},
  {elm_naviframe_add, "naviframe"},
  {elm_popup_add, "popup"},
  {elm_actionslider_add, "actionslider"},
  {elm_bg_add, "bg"},
  {elm_box_add, "box"},
  {elm_bubble_add, "bubble"},
  {elm_calendar_add, "calendar"},
  {elm_button_add, "button"},
  {elm_check_add, "check"},
  {elm_clock_add, "clock"},
  {elm_colorselector_add, "colorselector"},
  {elm_conformant_add, "conformant"},
  {elm_dayselector_add, "dayselector"},
  {elm_entry_add, "entry"},
  {elm_flipselector_add, "flipselector"},
  {elm_gengrid_add, "gengrid"},
  {elm_genlist_add, "genlist"},
  {elm_grid_add, "grid"},
  {elm_hover_add, "hover"},
  {elm_icon_add, "icon"},
  {elm_image_add, "image"},
  {elm_index_add, "index"},
  {elm_label_add, "label"},
  {elm_layout_add, "layout"},
  {elm_list_add, "list"},
  {elm_map_add, "map"},
  {elm_mapbuf_add, "mapbuf"},
  {elm_menu_add, "menu"},
  {elm_notify_add, "notify"},
  {elm_panel_add, "panel"},
  {elm_panes_add, "panes"},
  {elm_photo_add, "photo"},
  {elm_photocam_add, "photocam"},
  {elm_plug_add, "plug"},
  {elm_prefs_add, "prefs"},
  {elm_progressbar_add, "progressbar"},
  {elm_radio_add, "radio"},
  {elm_route_add, "route"},
  {elm_separator_add, "seperator"},
  {elm_slider_add, "slider"},
  {elm_slideshow_add, "slideshow"},
  {elm_spinner_add, "spinner"},
  {elm_table_add, "table"},
  {elm_textpath_add, "textpath"},
  {elm_toolbar_add, "toolbar"},
  {elm_web_add, "web"},
  {elm_diskselector_add, "diskselector"},
  {elm_datetime_add, "datetime"},
  //{elm_glview_add, "glview"}, This is causing issues on platforms that do not have gl, like CI, or minimal VMs
  //{elm_combobox_add, "button"}, This is a beta widget which was never public and is written in a few ways that break basic assertions of widgets base class
  //{elm_thumb_add, "button"}, This dies because of a ethumb bug, where the log domain is not correctly inited
  //{elm_systray_add, "button"}, This is not a elm widget, but matches the API regax
  //{elm_factory_add, "button"}, This is a beta widget which was never public but matches the API regax
  {NULL, NULL},
};

EFL_START_TEST(elm_test_widget_creation_easy)
{
   Evas_Object *win, *o;
   win = win_add();

   evas_object_resize(win, 200, 200);
   evas_object_show(win);
   for (int i = 0; simple_widgets[i].name; ++i)
     {
        o = simple_widgets[i].constructor(win);
        ck_assert_ptr_ne(o, NULL);
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_win_resize_object_add(win, o);
        evas_object_show(o);
        evas_object_del(o);
      }
}
EFL_END_TEST

EFL_START_TEST(elm_test_widget_creation_error_parent)
{
   Evas_Object *win, *o, *parent;
   Evas *evas;

   win = win_add();
   evas = evas_object_evas_get(win);
   evas_object_resize(win, 200, 200);
   evas_object_show(win);

   parent = evas_object_rectangle_add(evas);

   for (int i = 0; simple_widgets[i].name; ++i)
     {
        if (eina_streq(simple_widgets[i].name, "gengrid") ||
            eina_streq(simple_widgets[i].name, "genlist"))
          continue;
        if (eina_streq(simple_widgets[i].name, "datetime")) //this crashes in textblock
          continue;

        EXPECT_ERROR_START;
        o = simple_widgets[i].constructor(parent);
        EXPECT_ERROR_END;
        ck_assert_ptr_ne(o, NULL);
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_win_resize_object_add(win, o);
        evas_object_show(o);
        evas_object_del(o);
      }
}
EFL_END_TEST

void elm_test_widget_basics(TCase *tc)
{
   tcase_add_test(tc, elm_test_widget_creation_easy);
   tcase_add_test(tc, elm_test_widget_creation_error_parent);
}
