
#ifdef HAVE_CONFIG_H
#include <config.h>
#include <elementary_config.h>
#endif

#include <Efl.h>
#include <Efl_Config.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Eina.hh>
#include <Eina_Js.hh>
#include <Ecore_Js.hh>
#include <Eio_Js.hh>
#include <Eldbus_Js.hh>
#include <Ethumb_Js.hh>
#include <Elementary.h>

#include <iostream>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
#  define EAPI __declspec(dllexport)
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

EAPI void eina_container_register(v8::Handle<v8::Object>, v8::Isolate* isolate);
EAPI void eina_log_register(v8::Handle<v8::Object>, v8::Isolate* isolate);
EAPI void eina_value_register(v8::Handle<v8::Object>, v8::Isolate* isolate);
EAPI void register_ecore_mainloop(v8::Handle<v8::Object> global, v8::Isolate* isolate);

namespace ecore {
  //EAPI void register_exe(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_idler(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore(v8::Isolate* isolate, v8::Handle<v8::Object> exports);
namespace idle {
EAPI void register_enterer(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_exiter(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}
EAPI void register_job(v8::Handle<v8::Object> global, v8::Isolate* isolate);
  //EAPI void register_parent(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_poller(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_timer(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}

EAPI void register_ecore_audio(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_in(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_in_sndfile(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_in_tone(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_out(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_out_pulse(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_out_sndfile(v8::Handle<v8::Object> global, v8::Isolate* isolate);

namespace efl {
EAPI void register_object(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_animator(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_container(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_control(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_file(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_image(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_orientation(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_player(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_text(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_text_properties(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}

namespace efl { namespace ui {
EAPI void register_video(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_flip(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_frame(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_box(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_image(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}}

namespace efl { namespace ui { namespace win {
EAPI void register_standard(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}}}

namespace efl { namespace canvas {
EAPI void register_object(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_group(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_rectangle(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}}

namespace evas {
EAPI void register_box(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_canvas(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_clickable_interface(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_common_interface(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_draggable_interface(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_grid(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_image(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_line(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_object(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_object_smart(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_out(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_scrollable_interface(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_selectable_interface(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_signal_interface(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_smart_clipped(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_table(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_text(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_textblock(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_textgrid(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_zoomable_interface(v8::Handle<v8::Object> global, v8::Isolate* isolate);

namespace canvas3d {
EAPI void register_camera(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_light(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_material(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_mesh(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_node(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_object(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_scene(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_texture(v8::Handle<v8::Object> global, v8::Isolate* isolate);
} }

namespace efl { namespace canvas {
EAPI void register_polygon(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}}

namespace edje {
EAPI void register_edit(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_object(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}

namespace emotion {
EAPI void register_object(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}
EAPI void register_elm_atspi_app_object(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_accessible(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_action(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_component(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_editable_text(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_image(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_selection(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_text(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_value(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_widget_action(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_atspi_window(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_fileselector(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_interface_scrollable(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_elm_pan(v8::Handle<v8::Object> global, v8::Isolate* isolate);

namespace elm {

EAPI void register_bg(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_button(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_calendar(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_check(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_clock(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_colorselector(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_conformant(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_container(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_combobox(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ctxpopup(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_dayselector(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_entry(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_fileselector(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_flip(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_gengrid(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_gengrid_pan(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_genlist(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_genlist_pan(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_gesture_layer(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_glview(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_hover(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_index(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_label(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_layout(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_list(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_map(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_map_pan(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_menu(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_multibuttonentry(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_notify(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_panel(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_panes(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_photocam(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_photocam_pan(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_player(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_plug(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_popup(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_progressbar(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_radio(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_route(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_scroller(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_slider(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_spinner(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_systray(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_toolbar(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_web(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_widget(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_win(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_widget_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_color_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_dayselector_item(v8::Handle<v8::Object> global, v8::Isolate* isolate);
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
#else
namespace efl_js {
#endif

EAPI void init(v8::Handle<v8::Object> exports)
{
  static char* argv[] = {const_cast<char*>("node")};
  ::elm_init(1, argv);

  ::elm_need_ethumb();

  elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);
  
   try
     {
        eina_container_register(exports, v8::Isolate::GetCurrent());
        eina_log_register(exports, v8::Isolate::GetCurrent());
        eina_value_register(exports, v8::Isolate::GetCurrent());
        //register_ecore_mainloop(exports, v8::Isolate::GetCurrent());
        efl::eio::js::register_eio(v8::Isolate::GetCurrent(), exports);
        efl::eldbus::js::register_eldbus(v8::Isolate::GetCurrent(), exports);
        efl::ethumb::js::register_ethumb(v8::Isolate::GetCurrent(), exports);
        //ecore::register_exe(exports, v8::Isolate::GetCurrent());
        // ecore::register_idler(exports, v8::Isolate::GetCurrent());
        // ecore::idle::register_enterer(exports, v8::Isolate::GetCurrent());
        // ecore::idle::register_exiter(exports, v8::Isolate::GetCurrent());
        // ecore::register_job(exports, v8::Isolate::GetCurrent());
        //ecore::register_parent(exports, v8::Isolate::GetCurrent());
        // ecore::register_poller(exports, v8::Isolate::GetCurrent());
        // ecore::register_timer(exports, v8::Isolate::GetCurrent());
#if 0
        register_ecore_audio(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_in(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_in_sndfile(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_in_tone(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_out(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_out_pulse(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_out_sndfile(exports, v8::Isolate::GetCurrent());
#endif
        // Manual ecore binding initialized last to allow extension of namespace
        // created by eolian.
        efl::ecore::js::register_ecore(v8::Isolate::GetCurrent(), exports);
        efl::register_animator(exports, v8::Isolate::GetCurrent());
        efl::register_control(exports, v8::Isolate::GetCurrent());
        efl::register_file(exports, v8::Isolate::GetCurrent());
        efl::register_image(exports, v8::Isolate::GetCurrent());
        efl::register_orientation(exports, v8::Isolate::GetCurrent());
        efl::register_player(exports, v8::Isolate::GetCurrent());
        efl::register_text(exports, v8::Isolate::GetCurrent());
        // efl::register_text_properties(exports, v8::Isolate::GetCurrent());
        efl::register_object(exports, v8::Isolate::GetCurrent());
#if 1
        // evas::register_box(exports, v8::Isolate::GetCurrent());
        // evas::register_canvas(exports, v8::Isolate::GetCurrent());
        // evas::register_clickable_interface(exports, v8::Isolate::GetCurrent());
        // evas::register_common_interface(exports, v8::Isolate::GetCurrent());
        // evas::register_draggable_interface(exports, v8::Isolate::GetCurrent());
        // evas::register_grid(exports, v8::Isolate::GetCurrent());
        // evas::register_image(exports, v8::Isolate::GetCurrent());
        // evas::register_line(exports, v8::Isolate::GetCurrent());
        efl::canvas::register_object(exports, v8::Isolate::GetCurrent());
        efl::canvas::register_group(exports, v8::Isolate::GetCurrent());
        // evas::register_object_smart(exports, v8::Isolate::GetCurrent());
        // evas::register_out(exports, v8::Isolate::GetCurrent());
        efl::canvas::register_rectangle(exports, v8::Isolate::GetCurrent());
        // evas::register_scrollable_interface(exports, v8::Isolate::GetCurrent());
        // evas::register_selectable_interface(exports, v8::Isolate::GetCurrent());
        // evas::register_signal_interface(exports, v8::Isolate::GetCurrent());
        // evas::register_smart_clipped(exports, v8::Isolate::GetCurrent());
        // evas::register_table(exports, v8::Isolate::GetCurrent());
        // evas::register_text(exports, v8::Isolate::GetCurrent());
        // evas::register_textblock(exports, v8::Isolate::GetCurrent());
        // evas::register_textgrid(exports, v8::Isolate::GetCurrent());
        // evas::register_zoomable_interface(exports, v8::Isolate::GetCurrent());
        // evas::canvas3d::register_camera(exports, v8::Isolate::GetCurrent());
        // evas::canvas3d::register_light(exports, v8::Isolate::GetCurrent());
        // evas::canvas3d::register_material(exports, v8::Isolate::GetCurrent());
        // evas::canvas3d::register_mesh(exports, v8::Isolate::GetCurrent());
        // evas::canvas3d::register_node(exports, v8::Isolate::GetCurrent());
        // evas::canvas3d::register_object(exports, v8::Isolate::GetCurrent());
        // evas::canvas3d::register_scene(exports, v8::Isolate::GetCurrent());
        // evas::canvas3d::register_texture(exports, v8::Isolate::GetCurrent());
        // efl::canvas::register_polygon(exports, v8::Isolate::GetCurrent());
#endif
#if 0
        edje::register_edit(exports, v8::Isolate::GetCurrent());
        edje::register_object(exports, v8::Isolate::GetCurrent());
#endif
#if 0
        emotion::register_object(exports, v8::Isolate::GetCurrent());
#endif

      // elm::register_access(exports, v8::Isolate::GetCurrent());
      // elm::register_actionslider(exports, v8::Isolate::GetCurrent());
      // ::register_elm_atspi_app_object(exports, v8::Isolate::GetCurrent());
      // elm::register_box(exports, v8::Isolate::GetCurrent());

      elm::register_bg(exports, v8::Isolate::GetCurrent());
      elm::register_button(exports, v8::Isolate::GetCurrent());
      elm::register_calendar(exports, v8::Isolate::GetCurrent());

      elm::register_check(exports, v8::Isolate::GetCurrent());

      //  crash
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_clock(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);

      // crash
      // elm::register_colorselector(exports, v8::Isolate::GetCurrent());

      elm::register_conformant(exports, v8::Isolate::GetCurrent());

      // crash
      // elm::register_combobox(exports, v8::Isolate::GetCurrent());

      // doesn't exist
      efl::register_container(exports, v8::Isolate::GetCurrent());
      
      elm::register_ctxpopup(exports, v8::Isolate::GetCurrent());
      elm::register_dayselector(exports, v8::Isolate::GetCurrent());

      // crash
      // elm::register_diskselector(exports, v8::Isolate::GetCurrent());

      elm::register_entry(exports, v8::Isolate::GetCurrent());
      elm::register_fileselector(exports, v8::Isolate::GetCurrent());

      efl::ui::register_frame(exports, v8::Isolate::GetCurrent());
      efl::ui::register_box(exports, v8::Isolate::GetCurrent());
      efl::ui::register_image(exports, v8::Isolate::GetCurrent());

      // crash
      // elm::register_gengrid(exports, v8::Isolate::GetCurrent());

      // doesn't exist
      // elm::register_gengrid_pan(exports, v8::Isolate::GetCurrent());

      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // crash
      elm::register_genlist(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_genlist_pan(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_gesture_layer(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_glview(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_grid(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_index(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);

      // ::register_elm_interface_atspi_accessible(exports, v8::Isolate::GetCurrent());
      // ::register_elm_interface_atspi_action(exports, v8::Isolate::GetCurrent());
      // ::register_elm_interface_atspi_component(exports, v8::Isolate::GetCurrent());
      // ::register_elm_interface_atspi_editable_text(exports, v8::Isolate::GetCurrent());
      // ::register_elm_interface_atspi_image(exports, v8::Isolate::GetCurrent());
      // ::register_elm_interface_atspi_selection(exports, v8::Isolate::GetCurrent());
      // ::register_elm_interface_atspi_text(exports, v8::Isolate::GetCurrent());
      // ::register_elm_interface_atspi_value(exports, v8::Isolate::GetCurrent());
      // ::register_elm_interface_atspi_widget_action(exports, v8::Isolate::GetCurrent());
      // ::register_elm_interface_atspi_window(exports, v8::Isolate::GetCurrent());
      // ::register_elm_interface_fileselector(exports, v8::Isolate::GetCurrent());
      // ::register_elm_interface_scrollable(exports, v8::Isolate::GetCurrent());

      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_label(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_layout(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_list(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_map(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);

      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_map_pan(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      //elm::register_mapbuf(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_menu(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_multibuttonentry(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_notify(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);

      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // ::register_elm_pan(exports, v8::Isolate::GetCurrent());
      elm::register_panel(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_panes(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_photocam(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_photocam_pan(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_player(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_plug(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_popup(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_progressbar(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_radio(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);

      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_route(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_scroller(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_slider(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_spinner(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_systray(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_table(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_toolbar(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      efl::ui::register_flip(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      efl::ui::register_video(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_web(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      elm::register_widget(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);

      // crash
      // elm::register_win(exports, v8::Isolate::GetCurrent());
      fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      efl::ui::win::register_standard(exports, v8::Isolate::GetCurrent());

      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_widget_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_color_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_dayselector_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_hoversel_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_segment_control_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_slideshow_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_flipselector_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_menu_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_ctxpopup_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_index_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_multibuttonentry_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_naviframe_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_genlist_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_gengrid_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_list_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_toolbar_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_diskselector_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
      // elm::register_popup_item(exports, v8::Isolate::GetCurrent());
      // fprintf(stderr, __FILE__ ":%d\n", __LINE__); fflush(stderr);
     }
   catch(...)
     {
        std::cout << "Exception" << std::endl;
     }
}

#ifdef HAVE_NODEJS
}
NODE_MODULE(efl, init)
#else
} // namespace efl
#endif
