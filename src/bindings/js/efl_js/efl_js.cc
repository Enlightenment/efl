
#ifdef HAVE_CONFIG_H
#include <config.h>
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
EAPI void register_exe(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_idler(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore(v8::Isolate* isolate, v8::Handle<v8::Object> exports);
namespace idle {
EAPI void register_enterer(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_exiter(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}
EAPI void register_job(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_parent(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_poller(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_timer(v8::Handle<v8::Object> global, v8::Isolate* isolate);
namespace con {
EAPI void register_base(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_client(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_server(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}
}

namespace efl { namespace network {
EAPI void register_connector(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_url(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}}

EAPI void register_ecore_audio(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_in(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_in_sndfile(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_in_tone(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_out(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_out_pulse(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_ecore_audio_out_sndfile(v8::Handle<v8::Object> global, v8::Isolate* isolate);

namespace efl {
EAPI void register_control(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_file(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_image(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_player(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_text(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_text_properties(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}

namespace eo {
EAPI void register_abstract_class(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_base(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}

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
EAPI void register_polygon(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_rectangle(v8::Handle<v8::Object> global, v8::Isolate* isolate);
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

namespace edje {
EAPI void register_edit(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI void register_object(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}

namespace emotion {
EAPI void register_object(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}

#ifdef HAVE_NODEJS

#include <eina_js_node.hh>

namespace {
#else
namespace efl_js {
#endif

EAPI void init(v8::Handle<v8::Object> exports)
{
   ::eina_init();
   ::ecore_init();
   ::ecore_file_init();
   ::ecore_evas_init();
   ::eo_init();
   ::eio_init();
   ::evas_init();
   try
     {
        eina_container_register(exports, v8::Isolate::GetCurrent());
        eina_log_register(exports, v8::Isolate::GetCurrent());
        eina_value_register(exports, v8::Isolate::GetCurrent());
        register_ecore_mainloop(exports, v8::Isolate::GetCurrent());
        efl::ecore::js::register_ecore(v8::Isolate::GetCurrent(), exports);
        efl::eio::js::register_eio(v8::Isolate::GetCurrent(), exports);
        efl::eldbus::js::register_eldbus(v8::Isolate::GetCurrent(), exports);
        efl::ethumb::js::register_ethumb(v8::Isolate::GetCurrent(), exports);
        ecore::register_exe(exports, v8::Isolate::GetCurrent());
        ecore::register_idler(exports, v8::Isolate::GetCurrent());
        ecore::idle::register_enterer(exports, v8::Isolate::GetCurrent());
        ecore::idle::register_exiter(exports, v8::Isolate::GetCurrent());
        ecore::register_job(exports, v8::Isolate::GetCurrent());
        ecore::register_parent(exports, v8::Isolate::GetCurrent());
        ecore::register_poller(exports, v8::Isolate::GetCurrent());
        ecore::register_timer(exports, v8::Isolate::GetCurrent());
        ecore::con::register_base(exports, v8::Isolate::GetCurrent());
        ecore::con::register_client(exports, v8::Isolate::GetCurrent());
        efl::network::register_connector(exports, v8::Isolate::GetCurrent());
        ecore::con::register_server(exports, v8::Isolate::GetCurrent());
        efl::network::register_url(exports, v8::Isolate::GetCurrent());
#if 1
        register_ecore_audio(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_in(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_in_sndfile(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_in_tone(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_out(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_out_pulse(exports, v8::Isolate::GetCurrent());
        register_ecore_audio_out_sndfile(exports, v8::Isolate::GetCurrent());
#endif
        efl::register_control(exports, v8::Isolate::GetCurrent());
        efl::register_file(exports, v8::Isolate::GetCurrent());
        efl::register_image(exports, v8::Isolate::GetCurrent());
        efl::register_player(exports, v8::Isolate::GetCurrent());
        efl::register_text(exports, v8::Isolate::GetCurrent());
        efl::register_text_properties(exports, v8::Isolate::GetCurrent());
        eo::register_abstract_class(exports, v8::Isolate::GetCurrent());
        eo::register_base(exports, v8::Isolate::GetCurrent());
#if 1
        evas::register_box(exports, v8::Isolate::GetCurrent());
        evas::register_canvas(exports, v8::Isolate::GetCurrent());
        evas::register_clickable_interface(exports, v8::Isolate::GetCurrent());
        evas::register_common_interface(exports, v8::Isolate::GetCurrent());
        evas::register_draggable_interface(exports, v8::Isolate::GetCurrent());
        evas::register_grid(exports, v8::Isolate::GetCurrent());
        evas::register_image(exports, v8::Isolate::GetCurrent());
        evas::register_line(exports, v8::Isolate::GetCurrent());
        evas::register_object(exports, v8::Isolate::GetCurrent());
        evas::register_object_smart(exports, v8::Isolate::GetCurrent());
        evas::register_out(exports, v8::Isolate::GetCurrent());
        evas::register_polygon(exports, v8::Isolate::GetCurrent());
        evas::register_rectangle(exports, v8::Isolate::GetCurrent());
        evas::register_scrollable_interface(exports, v8::Isolate::GetCurrent());
        evas::register_selectable_interface(exports, v8::Isolate::GetCurrent());
        evas::register_signal_interface(exports, v8::Isolate::GetCurrent());
        evas::register_smart_clipped(exports, v8::Isolate::GetCurrent());
        evas::register_table(exports, v8::Isolate::GetCurrent());
        evas::register_text(exports, v8::Isolate::GetCurrent());
        evas::register_textblock(exports, v8::Isolate::GetCurrent());
        evas::register_textgrid(exports, v8::Isolate::GetCurrent());
        evas::register_zoomable_interface(exports, v8::Isolate::GetCurrent());
        evas::canvas3d::register_camera(exports, v8::Isolate::GetCurrent());
        evas::canvas3d::register_light(exports, v8::Isolate::GetCurrent());
        evas::canvas3d::register_material(exports, v8::Isolate::GetCurrent());
        evas::canvas3d::register_mesh(exports, v8::Isolate::GetCurrent());
        evas::canvas3d::register_node(exports, v8::Isolate::GetCurrent());
        evas::canvas3d::register_object(exports, v8::Isolate::GetCurrent());
        evas::canvas3d::register_scene(exports, v8::Isolate::GetCurrent());
        evas::canvas3d::register_texture(exports, v8::Isolate::GetCurrent());
#endif
#if 1
        edje::register_edit(exports, v8::Isolate::GetCurrent());
        edje::register_object(exports, v8::Isolate::GetCurrent());
#endif
#if 1
        emotion::register_object(exports, v8::Isolate::GetCurrent());
#endif
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
