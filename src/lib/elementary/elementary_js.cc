
#ifdef HAVE_CONFIG_H
#include <elementary_config.h>
#endif

#include <Elementary.h>

#include <Eina.hh>
#include <Eina_Js.hh>

#include <iostream>

#include <Efl.h>
#include <Eo.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#include <Eina.hh>

#ifdef EAPI
# undef EAPI
#endif
#ifdef _WIN32
# define EAPI __declspec(dllimport)
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

EAPI void register_elm_atspi_app_object(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_efl_access(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_efl_access_action(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_efl_access_component(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_text_editable(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_image(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_efl_access_selection(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_text(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_value(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_widget_action(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_efl_access_window(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_fileselector(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_scrollable(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_pan(v8::Handle<v8::Object> global, v8::Isolate* isolate);

namespace elm {

EAPI void register_access(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_actionslider(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_bg(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_box(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_bubble(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_button(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_calendar(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_check(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_clock(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_colorselector(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_conformant(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_container(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_combobox(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ctxpopup(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_datetime(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_dayselector(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_diskselector(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_entry(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_fileselector(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_fileselector_button(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_fileselector_entry(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_flip(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_flipselector(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_frame(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_gengrid(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_gengrid_pan(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_genlist(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_genlist_pan(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_gesture_layer(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_glview(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_grid(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_hover(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_hoversel(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_icon(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_image(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_index(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_inwin(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_label(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_layout(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_list(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_map(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_map_pan(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_mapbuf(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_menu(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_multibuttonentry(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_naviframe(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_notify(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_panel(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_panes(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_photo(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_photocam(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_photocam_pan(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_player(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_plug(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_popup(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_prefs(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_progressbar(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_radio(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_route(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_scroller(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_segment_control(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_separator(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_slider(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_slideshow(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_spinner(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_systray(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_table(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_thumb(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_toolbar(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_video(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_web(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_widget(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_win(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_win_standard(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_widget_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_color_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_dayselector_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_hoversel_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_segment_control_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_slideshow_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_flipselector_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_menu_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ctxpopup_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_index_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_multibuttonentry_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_naviframe_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_genlist_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_gengrid_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_list_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_toolbar_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_diskselector_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_popup_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);

}

#ifdef HAVE_NODEJS
#include <eina_js_node.hh>

namespace {
#endif

void init(v8::Handle<v8::Object> exports)
{
#ifdef HAVE_NODEJS
  static char* argv[] = {const_cast<char*>("node")};
  ::elm_init(1, argv);

  elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);
#endif
  
  try
    {
      elm::register_access(exports, v8::Isolate::GetCurrent());
      elm::register_actionslider(exports, v8::Isolate::GetCurrent());
      ::register_elm_atspi_app_object(exports, v8::Isolate::GetCurrent());
      elm::register_bg(exports, v8::Isolate::GetCurrent());
      elm::register_box(exports, v8::Isolate::GetCurrent());
      elm::register_bubble(exports, v8::Isolate::GetCurrent());
      elm::register_button(exports, v8::Isolate::GetCurrent());
      elm::register_calendar(exports, v8::Isolate::GetCurrent());
      elm::register_check(exports, v8::Isolate::GetCurrent());
      elm::register_clock(exports, v8::Isolate::GetCurrent());
      elm::register_colorselector(exports, v8::Isolate::GetCurrent());
      elm::register_conformant(exports, v8::Isolate::GetCurrent());
      elm::register_combobox(exports, v8::Isolate::GetCurrent());
      elm::register_container(exports, v8::Isolate::GetCurrent());
      elm::register_ctxpopup(exports, v8::Isolate::GetCurrent());
      elm::register_datetime(exports, v8::Isolate::GetCurrent());
      elm::register_dayselector(exports, v8::Isolate::GetCurrent());
      elm::register_diskselector(exports, v8::Isolate::GetCurrent());
      elm::register_entry(exports, v8::Isolate::GetCurrent());
      elm::register_fileselector(exports, v8::Isolate::GetCurrent());
      elm::register_fileselector_button(exports, v8::Isolate::GetCurrent());
      elm::register_fileselector_entry(exports, v8::Isolate::GetCurrent());
      elm::register_flip(exports, v8::Isolate::GetCurrent());
      elm::register_flipselector(exports, v8::Isolate::GetCurrent());
      elm::register_frame(exports, v8::Isolate::GetCurrent());
      elm::register_gengrid(exports, v8::Isolate::GetCurrent());
      elm::register_gengrid_pan(exports, v8::Isolate::GetCurrent());
      elm::register_genlist(exports, v8::Isolate::GetCurrent());
      elm::register_genlist_pan(exports, v8::Isolate::GetCurrent());
      elm::register_gesture_layer(exports, v8::Isolate::GetCurrent());
      elm::register_glview(exports, v8::Isolate::GetCurrent());
      elm::register_grid(exports, v8::Isolate::GetCurrent());
      elm::register_hover(exports, v8::Isolate::GetCurrent());
      elm::register_hoversel(exports, v8::Isolate::GetCurrent());
      elm::register_icon(exports, v8::Isolate::GetCurrent());
      elm::register_image(exports, v8::Isolate::GetCurrent());
      elm::register_index(exports, v8::Isolate::GetCurrent());
      ::register_efl_access(exports, v8::Isolate::GetCurrent());
      ::register_efl_access_action(exports, v8::Isolate::GetCurrent());
      ::register_efl_access_component(exports, v8::Isolate::GetCurrent());
      ::register_elm_interface_atspi_text_editable(exports, v8::Isolate::GetCurrent());
      ::register_elm_interface_atspi_image(exports, v8::Isolate::GetCurrent());
      ::register_efl_access_selection(exports, v8::Isolate::GetCurrent());
      ::register_elm_interface_atspi_text(exports, v8::Isolate::GetCurrent());
      ::register_elm_interface_atspi_value(exports, v8::Isolate::GetCurrent());
      ::register_elm_interface_atspi_widget_action(exports, v8::Isolate::GetCurrent());
      ::register_efl_access_window(exports, v8::Isolate::GetCurrent());
      ::register_elm_interface_fileselector(exports, v8::Isolate::GetCurrent());
      ::register_elm_interface_scrollable(exports, v8::Isolate::GetCurrent());
      elm::register_inwin(exports, v8::Isolate::GetCurrent());
      elm::register_label(exports, v8::Isolate::GetCurrent());
      elm::register_layout(exports, v8::Isolate::GetCurrent());
      elm::register_list(exports, v8::Isolate::GetCurrent());
      elm::register_map(exports, v8::Isolate::GetCurrent());
      elm::register_map_pan(exports, v8::Isolate::GetCurrent());
      elm::register_mapbuf(exports, v8::Isolate::GetCurrent());
      elm::register_menu(exports, v8::Isolate::GetCurrent());
      elm::register_multibuttonentry(exports, v8::Isolate::GetCurrent());
      elm::register_naviframe(exports, v8::Isolate::GetCurrent());
      elm::register_notify(exports, v8::Isolate::GetCurrent());
      ::register_elm_pan(exports, v8::Isolate::GetCurrent());
      elm::register_panel(exports, v8::Isolate::GetCurrent());
      elm::register_panes(exports, v8::Isolate::GetCurrent());
      elm::register_photo(exports, v8::Isolate::GetCurrent());
      elm::register_photocam(exports, v8::Isolate::GetCurrent());
      elm::register_photocam_pan(exports, v8::Isolate::GetCurrent());
      elm::register_player(exports, v8::Isolate::GetCurrent());
      elm::register_plug(exports, v8::Isolate::GetCurrent());
      elm::register_popup(exports, v8::Isolate::GetCurrent());
      elm::register_prefs(exports, v8::Isolate::GetCurrent());
      elm::register_progressbar(exports, v8::Isolate::GetCurrent());
      elm::register_radio(exports, v8::Isolate::GetCurrent());
      elm::register_route(exports, v8::Isolate::GetCurrent());
      elm::register_scroller(exports, v8::Isolate::GetCurrent());
      elm::register_segment_control(exports, v8::Isolate::GetCurrent());
      elm::register_separator(exports, v8::Isolate::GetCurrent());
      elm::register_slider(exports, v8::Isolate::GetCurrent());
      elm::register_slideshow(exports, v8::Isolate::GetCurrent());
      elm::register_spinner(exports, v8::Isolate::GetCurrent());
      elm::register_systray(exports, v8::Isolate::GetCurrent());
      elm::register_table(exports, v8::Isolate::GetCurrent());
      elm::register_thumb(exports, v8::Isolate::GetCurrent());
      elm::register_toolbar(exports, v8::Isolate::GetCurrent());
      elm::register_video(exports, v8::Isolate::GetCurrent());
      elm::register_web(exports, v8::Isolate::GetCurrent());
      elm::register_widget(exports, v8::Isolate::GetCurrent());
      elm::register_win(exports, v8::Isolate::GetCurrent());
      elm::register_win_standard(exports, v8::Isolate::GetCurrent());
      elm::register_widget_item(exports, v8::Isolate::GetCurrent());
      elm::register_color_item(exports, v8::Isolate::GetCurrent());
      elm::register_dayselector_item(exports, v8::Isolate::GetCurrent());
      elm::register_hoversel_item(exports, v8::Isolate::GetCurrent());
      elm::register_segment_control_item(exports, v8::Isolate::GetCurrent());
      elm::register_slideshow_item(exports, v8::Isolate::GetCurrent());
      elm::register_flipselector_item(exports, v8::Isolate::GetCurrent());
      elm::register_menu_item(exports, v8::Isolate::GetCurrent());
      elm::register_ctxpopup_item(exports, v8::Isolate::GetCurrent());
      elm::register_index_item(exports, v8::Isolate::GetCurrent());
      elm::register_multibuttonentry_item(exports, v8::Isolate::GetCurrent());
      elm::register_naviframe_item(exports, v8::Isolate::GetCurrent());
      elm::register_genlist_item(exports, v8::Isolate::GetCurrent());
      elm::register_gengrid_item(exports, v8::Isolate::GetCurrent());
      elm::register_list_item(exports, v8::Isolate::GetCurrent());
      elm::register_toolbar_item(exports, v8::Isolate::GetCurrent());
      elm::register_diskselector_item(exports, v8::Isolate::GetCurrent());
      elm::register_popup_item(exports, v8::Isolate::GetCurrent());
    }
  catch(...)
    {
      std::cout << "Exception" << std::endl;
    }
}
  
#ifdef HAVE_NODEJS
}
NODE_MODULE(elm, init)
#endif
