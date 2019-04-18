#ifndef EVAS_CANVAS_EO_HH
#define EVAS_CANVAS_EO_HH
#include <Eo.h>

extern "C" {
#include "efl_canvas_object.eo.h"
#include "efl_canvas_pointer.eo.h"
#include "efl_canvas_scene.eo.h"
#include "efl_class.eo.h"
#include "efl_gfx_entity.eo.h"
#include "efl_input_device.eo.h"
#include "efl_input_focus.eo.h"
#include "efl_input_hold.eo.h"
#include "efl_input_interface.eo.h"
#include "efl_input_key.eo.h"
#include "efl_input_pointer.eo.h"
#include "efl_loop.eo.h"
#include "efl_loop_consumer.eo.h"
#include "efl_object.eo.h"
#include "evas_canvas_eo.h"
}
#include <Eina.hh>
#include <Eo.hh>
#include "efl_canvas_object.eo.hh"
#include "efl_canvas_pointer.eo.hh"
#include "efl_canvas_scene.eo.hh"
#include "efl_class.eo.hh"
#include "efl_gfx_entity.eo.hh"
#include "efl_input_device.eo.hh"
#include "efl_input_focus.eo.hh"
#include "efl_input_hold.eo.hh"
#include "efl_input_interface.eo.hh"
#include "efl_input_key.eo.hh"
#include "efl_input_pointer.eo.hh"
#include "efl_loop.eo.hh"
#include "efl_loop_consumer.eo.hh"
#include "efl_object.eo.hh"
#ifndef EVAS_CANVAS_FWD_GUARD
#define EVAS_CANVAS_FWD_GUARD
namespace evas { 
struct Canvas;
} 
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Canvas> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Canvas&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Canvas const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Canvas const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_CLASS_FWD_GUARD
#define EFL_CLASS_FWD_GUARD
namespace efl { 
struct Class;
} 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Class> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Class&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Class const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Class const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_CANVAS_OBJECT_FWD_GUARD
#define EFL_CANVAS_OBJECT_FWD_GUARD
namespace efl { namespace canvas { 
struct Object;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Object> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Object&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Object const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Object const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_CANVAS_POINTER_FWD_GUARD
#define EFL_CANVAS_POINTER_FWD_GUARD
namespace efl { namespace canvas { 
struct Pointer;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Pointer> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Pointer&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Pointer const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Pointer const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_GFX_ENTITY_FWD_GUARD
#define EFL_GFX_ENTITY_FWD_GUARD
namespace efl { namespace gfx { 
struct Entity;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Entity> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Entity&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Entity const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Entity const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_INPUT_DEVICE_FWD_GUARD
#define EFL_INPUT_DEVICE_FWD_GUARD
namespace efl { namespace input { 
struct Device;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Device> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Device&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Device const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Device const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_INPUT_FOCUS_FWD_GUARD
#define EFL_INPUT_FOCUS_FWD_GUARD
namespace efl { namespace input { 
struct Focus;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Focus> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Focus&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Focus const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Focus const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_INPUT_HOLD_FWD_GUARD
#define EFL_INPUT_HOLD_FWD_GUARD
namespace efl { namespace input { 
struct Hold;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Hold> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Hold&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Hold const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Hold const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_INPUT_INTERFACE_FWD_GUARD
#define EFL_INPUT_INTERFACE_FWD_GUARD
namespace efl { namespace input { 
struct Interface;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Interface> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Interface&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Interface const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Interface const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_INPUT_KEY_FWD_GUARD
#define EFL_INPUT_KEY_FWD_GUARD
namespace efl { namespace input { 
struct Key;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Key> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Key&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Key const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Key const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_LOOP_FWD_GUARD
#define EFL_LOOP_FWD_GUARD
namespace efl { 
struct Loop;
} 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Loop> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Loop&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Loop const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Loop const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_LOOP_CONSUMER_FWD_GUARD
#define EFL_LOOP_CONSUMER_FWD_GUARD
namespace efl { 
struct Loop_Consumer;
} 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Loop_Consumer> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Loop_Consumer&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Loop_Consumer const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Loop_Consumer const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_OBJECT_FWD_GUARD
#define EFL_OBJECT_FWD_GUARD
namespace efl { 
struct Object;
} 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Object> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Object&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Object const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Object const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_INPUT_POINTER_FWD_GUARD
#define EFL_INPUT_POINTER_FWD_GUARD
namespace efl { namespace input { 
struct Pointer;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Pointer> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Pointer&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Pointer const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::input::Pointer const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_CANVAS_SCENE_FWD_GUARD
#define EFL_CANVAS_SCENE_FWD_GUARD
namespace efl { namespace canvas { 
struct Scene;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Scene> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Scene&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Scene const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Scene const&> : ::std::true_type {}; } }
#endif

namespace eo_cxx {
namespace evas { 
struct Canvas {
   ::efl::eolian::return_traits<int>::type image_cache_get() const;
   ::efl::eolian::return_traits<void>::type image_cache_set( ::efl::eolian::in_traits<int>::type size) const;
   ::efl::eolian::return_traits<Efl_Input_Flags>::type event_default_flags_get() const;
   ::efl::eolian::return_traits<void>::type event_default_flags_set( ::efl::eolian::in_traits<Efl_Input_Flags>::type flags) const;
   ::efl::eolian::return_traits<int>::type font_cache_get() const;
   ::efl::eolian::return_traits<void>::type font_cache_set( ::efl::eolian::in_traits<int>::type size) const;
   ::efl::eolian::return_traits<void*>::type data_attach_get() const;
   ::efl::eolian::return_traits<void>::type data_attach_set( ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type focus_get() const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type seat_focus_get( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type object_top_get() const;
   ::efl::eolian::return_traits<void>::type pointer_canvas_xy_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev,  ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const;
   ::efl::eolian::return_traits<void>::type pointer_canvas_xy_get( ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const;
   ::efl::eolian::return_traits<int>::type event_down_count_get() const;
   ::efl::eolian::return_traits<int>::type smart_objects_calculate_count_get() const;
   ::efl::eolian::return_traits<bool>::type focus_state_get() const;
   ::efl::eolian::return_traits<bool>::type seat_focus_state_get( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<bool>::type changed_get() const;
   ::efl::eolian::return_traits<void>::type pointer_output_xy_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev,  ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const;
   ::efl::eolian::return_traits<void>::type pointer_output_xy_get( ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type object_bottom_get() const;
   ::efl::eolian::return_traits< unsigned int>::type pointer_button_down_mask_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev) const;
   ::efl::eolian::return_traits< unsigned int>::type pointer_button_down_mask_get() const;
   ::efl::eolian::return_traits< ::efl::input::Device>::type default_device_get( ::efl::eolian::in_traits<Efl_Input_Device_Type>::type type) const;
   ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::canvas::Object>>::type tree_objects_at_xy_get( ::efl::eolian::in_traits< ::efl::canvas::Object>::type stop,  ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y) const;
   ::efl::eolian::return_traits<void>::type key_lock_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type seat_key_lock_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<void>::type seat_key_lock_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<void>::type key_modifier_add( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type key_modifier_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<bool>::type render_async() const;
   ::efl::eolian::return_traits<void>::type focus_out() const;
   ::efl::eolian::return_traits<void>::type norender() const;
   ::efl::eolian::return_traits<void>::type nochange_pop() const;
   ::efl::eolian::return_traits<void>::type key_lock_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type nochange_push() const;
   ::efl::eolian::return_traits<void>::type font_cache_flush() const;
   ::efl::eolian::return_traits<void>::type key_modifier_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type seat_key_modifier_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<void>::type seat_key_modifier_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type font_available_list() const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type object_name_find( ::efl::eolian::in_traits< ::efl::eina::string_view>::type name) const;
   ::efl::eolian::return_traits<void>::type font_path_append( ::efl::eolian::in_traits< ::efl::eina::string_view>::type path) const;
   ::efl::eolian::return_traits<void>::type font_path_clear() const;
   ::efl::eolian::return_traits<void>::type touch_point_list_nth_xy_get( ::efl::eolian::in_traits< unsigned int>::type n,  ::efl::eolian::out_traits<double>::type x,  ::efl::eolian::out_traits<double>::type y) const;
   ::efl::eolian::return_traits<void>::type key_lock_del( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type damage_rectangle_add( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const;
   ::efl::eolian::return_traits<void>::type sync() const;
   ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type font_path_list() const;
   ::efl::eolian::return_traits<void>::type image_cache_reload() const;
   ::efl::eolian::return_traits<int>::type coord_world_x_to_screen( ::efl::eolian::in_traits<int>::type x) const;
   ::efl::eolian::return_traits<  ::efl::eina::list<Eina_Rect const&>>::type render_updates() const;
   ::efl::eolian::return_traits<void>::type image_cache_flush() const;
   ::efl::eolian::return_traits<int>::type coord_screen_y_to_world( ::efl::eolian::in_traits<int>::type y) const;
   ::efl::eolian::return_traits<void>::type key_modifier_del( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type focus_in() const;
   ::efl::eolian::return_traits<void>::type seat_focus_in( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<void>::type seat_focus_out( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<void>::type obscured_rectangle_add( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const;
   ::efl::eolian::return_traits<void>::type render_dump() const;
   ::efl::eolian::return_traits<void>::type render() const;
   ::efl::eolian::return_traits<void>::type font_path_prepend( ::efl::eolian::in_traits< ::efl::eina::string_view>::type path) const;
   ::efl::eolian::return_traits<void>::type obscured_clear() const;
   ::efl::eolian::return_traits<int>::type coord_screen_x_to_world( ::efl::eolian::in_traits<int>::type x) const;
   ::efl::eolian::return_traits<void>::type key_lock_add( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type render_idle_flush() const;
   ::efl::eolian::return_traits<int>::type coord_world_y_to_screen( ::efl::eolian::in_traits<int>::type y) const;
   static Efl_Class const* _eo_class()
   {
      return EVAS_CANVAS_CLASS;
   }
   Eo* _eo_ptr() const { return *(reinterpret_cast<Eo **>(const_cast<Canvas *>(this))); }
   operator ::evas::Canvas() const;
   operator ::evas::Canvas&();
   operator ::evas::Canvas const&() const;
   /// @cond LOCAL
    ::efl::eolian::address_of_operator<Canvas,  ::efl::Loop_Consumer,  ::efl::Object,  ::efl::canvas::Pointer,  ::efl::canvas::Scene,  ::efl::input::Interface> operator&() { return {this}; }
    ::efl::eolian::address_of_operator<Canvas const ,  ::efl::Loop_Consumer const ,  ::efl::Object const ,  ::efl::canvas::Pointer const ,  ::efl::canvas::Scene const ,  ::efl::input::Interface const > operator&() const { return {this}; }
   /// @endcond
};
} 
}
namespace evas { 
struct Canvas : private ::efl::eo::concrete
   , EO_CXX_INHERIT(::efl::Loop_Consumer)
   , EO_CXX_INHERIT(::efl::Object)
   , EO_CXX_INHERIT(::efl::canvas::Pointer)
   , EO_CXX_INHERIT(::efl::canvas::Scene)
   , EO_CXX_INHERIT(::efl::input::Interface)
{
   explicit Canvas( ::Eo* eo)
      : ::efl::eo::concrete(eo) {}
   Canvas(std::nullptr_t)
      : ::efl::eo::concrete(nullptr) {}
   explicit Canvas() = default;
   Canvas(Canvas const&) = default;
   Canvas(Canvas&&) = default;
   Canvas& operator=(Canvas const&) = default;
   Canvas& operator=(Canvas&&) = default;
   template <typename Derived>
   Canvas(Derived&& derived
      , typename std::enable_if<
         ::efl::eo::is_eolian_object<Derived>::value
          && std::is_base_of< Canvas, Derived>::value>::type* = 0)
         : ::efl::eo::concrete(derived._eo_ptr()) {}

   Canvas( ::efl::eo::instantiate_t)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class());
   }
   template <typename T>
   explicit Canvas( ::efl::eo::instantiate_t, T&& parent, typename std::enable_if< ::efl::eo::is_eolian_object<T>::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, parent, _eo_class());
   }
   template <typename F> Canvas( ::efl::eo::instantiate_t, F&& f, typename ::std::enable_if< ::efl::eolian::is_constructor_lambda<F, Canvas >::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class(), *this, std::forward<F>(f));
   }
   template <typename T, typename F> Canvas(  ::efl::eo::instantiate_t, T&& parent, F&& f, typename ::std::enable_if< ::efl::eolian::is_constructor_lambda<F, Canvas >::value && ::efl::eo::is_eolian_object<T>::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, parent, _eo_class(), *this, std::forward<F>(f));
   }

   ::efl::eolian::return_traits<int>::type image_cache_get() const;
   ::efl::eolian::return_traits<void>::type image_cache_set( ::efl::eolian::in_traits<int>::type size) const;
   ::efl::eolian::return_traits<Efl_Input_Flags>::type event_default_flags_get() const;
   ::efl::eolian::return_traits<void>::type event_default_flags_set( ::efl::eolian::in_traits<Efl_Input_Flags>::type flags) const;
   ::efl::eolian::return_traits<int>::type font_cache_get() const;
   ::efl::eolian::return_traits<void>::type font_cache_set( ::efl::eolian::in_traits<int>::type size) const;
   ::efl::eolian::return_traits<void*>::type data_attach_get() const;
   ::efl::eolian::return_traits<void>::type data_attach_set( ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type focus_get() const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type seat_focus_get( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type object_top_get() const;
   ::efl::eolian::return_traits<void>::type pointer_canvas_xy_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev,  ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const;
   ::efl::eolian::return_traits<void>::type pointer_canvas_xy_get( ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const;
   ::efl::eolian::return_traits<int>::type event_down_count_get() const;
   ::efl::eolian::return_traits<int>::type smart_objects_calculate_count_get() const;
   ::efl::eolian::return_traits<bool>::type focus_state_get() const;
   ::efl::eolian::return_traits<bool>::type seat_focus_state_get( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<bool>::type changed_get() const;
   ::efl::eolian::return_traits<void>::type pointer_output_xy_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev,  ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const;
   ::efl::eolian::return_traits<void>::type pointer_output_xy_get( ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type object_bottom_get() const;
   ::efl::eolian::return_traits< unsigned int>::type pointer_button_down_mask_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev) const;
   ::efl::eolian::return_traits< unsigned int>::type pointer_button_down_mask_get() const;
   ::efl::eolian::return_traits< ::efl::input::Device>::type default_device_get( ::efl::eolian::in_traits<Efl_Input_Device_Type>::type type) const;
   ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::canvas::Object>>::type tree_objects_at_xy_get( ::efl::eolian::in_traits< ::efl::canvas::Object>::type stop,  ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y) const;
   ::efl::eolian::return_traits<void>::type key_lock_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type seat_key_lock_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<void>::type seat_key_lock_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<void>::type key_modifier_add( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type key_modifier_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<bool>::type render_async() const;
   ::efl::eolian::return_traits<void>::type focus_out() const;
   ::efl::eolian::return_traits<void>::type norender() const;
   ::efl::eolian::return_traits<void>::type nochange_pop() const;
   ::efl::eolian::return_traits<void>::type key_lock_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type nochange_push() const;
   ::efl::eolian::return_traits<void>::type font_cache_flush() const;
   ::efl::eolian::return_traits<void>::type key_modifier_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type seat_key_modifier_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<void>::type seat_key_modifier_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type font_available_list() const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type object_name_find( ::efl::eolian::in_traits< ::efl::eina::string_view>::type name) const;
   ::efl::eolian::return_traits<void>::type font_path_append( ::efl::eolian::in_traits< ::efl::eina::string_view>::type path) const;
   ::efl::eolian::return_traits<void>::type font_path_clear() const;
   ::efl::eolian::return_traits<void>::type touch_point_list_nth_xy_get( ::efl::eolian::in_traits< unsigned int>::type n,  ::efl::eolian::out_traits<double>::type x,  ::efl::eolian::out_traits<double>::type y) const;
   ::efl::eolian::return_traits<void>::type key_lock_del( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type damage_rectangle_add( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const;
   ::efl::eolian::return_traits<void>::type sync() const;
   ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type font_path_list() const;
   ::efl::eolian::return_traits<void>::type image_cache_reload() const;
   ::efl::eolian::return_traits<int>::type coord_world_x_to_screen( ::efl::eolian::in_traits<int>::type x) const;
   ::efl::eolian::return_traits<  ::efl::eina::list<Eina_Rect const&>>::type render_updates() const;
   ::efl::eolian::return_traits<void>::type image_cache_flush() const;
   ::efl::eolian::return_traits<int>::type coord_screen_y_to_world( ::efl::eolian::in_traits<int>::type y) const;
   ::efl::eolian::return_traits<void>::type key_modifier_del( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type focus_in() const;
   ::efl::eolian::return_traits<void>::type seat_focus_in( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<void>::type seat_focus_out( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const;
   ::efl::eolian::return_traits<void>::type obscured_rectangle_add( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const;
   ::efl::eolian::return_traits<void>::type render_dump() const;
   ::efl::eolian::return_traits<void>::type render() const;
   ::efl::eolian::return_traits<void>::type font_path_prepend( ::efl::eolian::in_traits< ::efl::eina::string_view>::type path) const;
   ::efl::eolian::return_traits<void>::type obscured_clear() const;
   ::efl::eolian::return_traits<int>::type coord_screen_x_to_world( ::efl::eolian::in_traits<int>::type x) const;
   ::efl::eolian::return_traits<void>::type key_lock_add( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const;
   ::efl::eolian::return_traits<void>::type render_idle_flush() const;
   ::efl::eolian::return_traits<int>::type coord_world_y_to_screen( ::efl::eolian::in_traits<int>::type y) const;
   static Efl_Class const* _eo_class()
   {
      return EVAS_CANVAS_CLASS;
   }
   Eo* _eo_ptr() const { return *(reinterpret_cast<Eo **>(const_cast<Canvas *>(this))); }
   /// @cond LOCAL
    ::efl::eolian::address_of_operator<Canvas,  ::efl::Loop_Consumer,  ::efl::Object,  ::efl::canvas::Pointer,  ::efl::canvas::Scene,  ::efl::input::Interface> operator&() { return {this}; }
    ::efl::eolian::address_of_operator<Canvas const ,  ::efl::Loop_Consumer const ,  ::efl::Object const ,  ::efl::canvas::Pointer const ,  ::efl::canvas::Scene const ,  ::efl::input::Interface const > operator&() const { return {this}; }
   /// @endcond
   ::efl::eo::wref<Canvas> _get_wref() const { return ::efl::eo::wref<Canvas>(*this); }
   ::efl::eo::concrete const& _get_concrete() const { return *this; }
   ::efl::eo::concrete& _get_concrete() { return *this; }
   using ::efl::eo::concrete::_eo_ptr;
   using ::efl::eo::concrete::_release;
   using ::efl::eo::concrete::_reset;
   using ::efl::eo::concrete::_delete;
   using ::efl::eo::concrete::operator bool;
#ifdef EFL_CXXPERIMENTAL
   const Canvas* operator->() const { return this; }
   Canvas* operator->() { return this; }
   operator Eo*() const { return _eo_ptr(); }
#endif 
   friend bool operator==(Canvas const& lhs, Canvas const& rhs)
   { return lhs._get_concrete() == rhs._get_concrete(); }
   friend bool operator!=(Canvas const& lhs, Canvas const& rhs)
   { return !(lhs == rhs); }
};
static_assert(sizeof(Canvas) == sizeof(Eo*), "");
static_assert(std::is_standard_layout<Canvas>::value, "");
} 
#endif
