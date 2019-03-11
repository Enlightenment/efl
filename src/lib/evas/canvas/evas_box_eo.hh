#ifndef EVAS_BOX_EO_HH
#define EVAS_BOX_EO_HH
#include <Eo.h>

extern "C" {
#include "efl_canvas_group.eo.h"
#include "efl_canvas_object.eo.h"
#include "efl_canvas_pointer.eo.h"
#include "efl_class.eo.h"
#include "efl_gfx_color.eo.h"
#include "efl_gfx_entity.eo.h"
#include "efl_gfx_hint.eo.h"
#include "efl_gfx_mapping.eo.h"
#include "efl_gfx_stack.eo.h"
#include "efl_input_device.eo.h"
#include "efl_input_focus.eo.h"
#include "efl_input_hold.eo.h"
#include "efl_input_interface.eo.h"
#include "efl_input_key.eo.h"
#include "efl_input_pointer.eo.h"
#include "efl_loop.eo.h"
#include "efl_loop_consumer.eo.h"
#include "efl_object.eo.h"
#include "efl_ui_i18n.eo.h"
#include "evas_box_eo.h"
}
#include <Eina.hh>
#include <Eo.hh>
#include "efl_canvas_group.eo.hh"
#include "efl_canvas_object.eo.hh"
#include "efl_canvas_pointer.eo.hh"
#include "efl_class.eo.hh"
#include "efl_gfx_color.eo.hh"
#include "efl_gfx_entity.eo.hh"
#include "efl_gfx_hint.eo.hh"
#include "efl_gfx_mapping.eo.hh"
#include "efl_gfx_stack.eo.hh"
#include "efl_input_device.eo.hh"
#include "efl_input_focus.eo.hh"
#include "efl_input_hold.eo.hh"
#include "efl_input_interface.eo.hh"
#include "efl_input_key.eo.hh"
#include "efl_input_pointer.eo.hh"
#include "efl_loop.eo.hh"
#include "efl_loop_consumer.eo.hh"
#include "efl_object.eo.hh"
#include "efl_ui_i18n.eo.hh"
#ifndef EVAS_BOX_FWD_GUARD
#define EVAS_BOX_FWD_GUARD
namespace evas { 
struct Box;
} 
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Box> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Box&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Box const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Box const&> : ::std::true_type {}; } }
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
#ifndef EFL_GFX_COLOR_FWD_GUARD
#define EFL_GFX_COLOR_FWD_GUARD
namespace efl { namespace gfx { 
struct Color;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Color> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Color&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Color const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Color const&> : ::std::true_type {}; } }
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
#ifndef EFL_GFX_HINT_FWD_GUARD
#define EFL_GFX_HINT_FWD_GUARD
namespace efl { namespace gfx { 
struct Hint;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Hint> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Hint&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Hint const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Hint const&> : ::std::true_type {}; } }
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
#ifndef EFL_UI_I18N_FWD_GUARD
#define EFL_UI_I18N_FWD_GUARD
namespace efl { namespace ui { 
struct I18n;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::ui::I18n> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::ui::I18n&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::ui::I18n const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::ui::I18n const&> : ::std::true_type {}; } }
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
#ifndef EFL_CANVAS_GROUP_FWD_GUARD
#define EFL_CANVAS_GROUP_FWD_GUARD
namespace efl { namespace canvas { 
struct Group;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Group> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Group&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Group const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Group const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_GFX_MAPPING_FWD_GUARD
#define EFL_GFX_MAPPING_FWD_GUARD
namespace efl { namespace gfx { 
struct Mapping;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Mapping> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Mapping&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Mapping const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Mapping const&> : ::std::true_type {}; } }
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
#ifndef EFL_GFX_STACK_FWD_GUARD
#define EFL_GFX_STACK_FWD_GUARD
namespace efl { namespace gfx { 
struct Stack;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Stack> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Stack&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Stack const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Stack const&> : ::std::true_type {}; } }
#endif

namespace eo_cxx {
namespace evas { 
struct Box {
   ::efl::eolian::return_traits<void>::type align_get( ::efl::eolian::out_traits<double>::type horizontal,  ::efl::eolian::out_traits<double>::type vertical) const;
   ::efl::eolian::return_traits<void>::type align_set( ::efl::eolian::in_traits<double>::type horizontal,  ::efl::eolian::in_traits<double>::type vertical) const;
   ::efl::eolian::return_traits<void>::type padding_get( ::efl::eolian::out_traits<int>::type horizontal,  ::efl::eolian::out_traits<int>::type vertical) const;
   ::efl::eolian::return_traits<void>::type padding_set( ::efl::eolian::in_traits<int>::type horizontal,  ::efl::eolian::in_traits<int>::type vertical) const;
   ::efl::eolian::return_traits<void>::type layout_set( ::efl::eolian::in_traits<Evas_Object_Box_Layout>::type cb,  ::efl::eolian::in_traits<void const*>::type data,  ::efl::eolian::in_traits<Eina_Free_Cb>::type free_data) const;
   ::efl::eolian::return_traits<void>::type layout_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<void>::type layout_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<void>::type layout_homogeneous_max_size_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type internal_remove( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<void>::type layout_flow_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<void>::type internal_option_free( ::efl::eolian::in_traits<Evas_Object_Box_Option const*>::type opt) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type insert_after( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const;
   ::efl::eolian::return_traits<bool>::type remove_all( ::efl::eolian::in_traits<bool>::type clear) const;
   ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type iterator_new() const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type add_to() const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type append( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<int>::type option_property_id_get( ::efl::eolian::in_traits< ::efl::eina::string_view>::type name) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type prepend( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type accessor_new() const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_append( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type internal_remove_at( ::efl::eolian::in_traits< unsigned int>::type pos) const;
   ::efl::eolian::return_traits<bool>::type remove_at( ::efl::eolian::in_traits< unsigned int>::type pos) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_insert_at( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned int>::type pos) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type insert_before( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const;
   ::efl::eolian::return_traits< ::efl::eina::string_view>::type option_property_name_get( ::efl::eolian::in_traits<int>::type property) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_insert_before( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const;
   ::efl::eolian::return_traits<void>::type layout_homogeneous_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_option_new( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<void>::type layout_homogeneous_max_size_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_insert_after( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type insert_at( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned int>::type pos) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_prepend( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<bool>::type remove( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<void>::type layout_stack( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<void>::type layout_homogeneous_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<void>::type layout_flow_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<int>::type count() const;
   static Efl_Class const* _eo_class()
   {
      return EVAS_BOX_CLASS;
   }
   Eo* _eo_ptr() const { return *(reinterpret_cast<Eo **>(const_cast<Box *>(this))); }
   operator ::evas::Box() const;
   operator ::evas::Box&();
   operator ::evas::Box const&() const;
   /// @cond LOCAL
    ::efl::eolian::address_of_operator<Box,  ::efl::Loop_Consumer,  ::efl::Object,  ::efl::canvas::Group,  ::efl::canvas::Object,  ::efl::canvas::Pointer,  ::efl::gfx::Color,  ::efl::gfx::Entity,  ::efl::gfx::Hint,  ::efl::gfx::Mapping,  ::efl::gfx::Stack,  ::efl::input::Interface,  ::efl::ui::I18n> operator&() { return {this}; }
    ::efl::eolian::address_of_operator<Box const ,  ::efl::Loop_Consumer const ,  ::efl::Object const ,  ::efl::canvas::Group const ,  ::efl::canvas::Object const ,  ::efl::canvas::Pointer const ,  ::efl::gfx::Color const ,  ::efl::gfx::Entity const ,  ::efl::gfx::Hint const ,  ::efl::gfx::Mapping const ,  ::efl::gfx::Stack const ,  ::efl::input::Interface const ,  ::efl::ui::I18n const > operator&() const { return {this}; }
   /// @endcond
};
} 
}
namespace evas { 
struct Box : private ::efl::eo::concrete
   , EO_CXX_INHERIT(::efl::Loop_Consumer)
   , EO_CXX_INHERIT(::efl::Object)
   , EO_CXX_INHERIT(::efl::canvas::Group)
   , EO_CXX_INHERIT(::efl::canvas::Object)
   , EO_CXX_INHERIT(::efl::canvas::Pointer)
   , EO_CXX_INHERIT(::efl::gfx::Color)
   , EO_CXX_INHERIT(::efl::gfx::Entity)
   , EO_CXX_INHERIT(::efl::gfx::Hint)
   , EO_CXX_INHERIT(::efl::gfx::Mapping)
   , EO_CXX_INHERIT(::efl::gfx::Stack)
   , EO_CXX_INHERIT(::efl::input::Interface)
   , EO_CXX_INHERIT(::efl::ui::I18n)
{
   explicit Box( ::Eo* eo)
      : ::efl::eo::concrete(eo) {}
   Box(std::nullptr_t)
      : ::efl::eo::concrete(nullptr) {}
   explicit Box() = default;
   Box(Box const&) = default;
   Box(Box&&) = default;
   Box& operator=(Box const&) = default;
   Box& operator=(Box&&) = default;
   template <typename Derived>
   Box(Derived&& derived
      , typename std::enable_if<
         ::efl::eo::is_eolian_object<Derived>::value
          && std::is_base_of< Box, Derived>::value>::type* = 0)
         : ::efl::eo::concrete(derived._eo_ptr()) {}

   Box( ::efl::eo::instantiate_t)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class());
   }
   template <typename T>
   explicit Box( ::efl::eo::instantiate_t, T&& parent, typename std::enable_if< ::efl::eo::is_eolian_object<T>::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, parent, _eo_class());
   }
   template <typename F> Box( ::efl::eo::instantiate_t, F&& f, typename ::std::enable_if< ::efl::eolian::is_constructor_lambda<F, Box >::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class(), *this, std::forward<F>(f));
   }
   template <typename T, typename F> Box(  ::efl::eo::instantiate_t, T&& parent, F&& f, typename ::std::enable_if< ::efl::eolian::is_constructor_lambda<F, Box >::value && ::efl::eo::is_eolian_object<T>::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, parent, _eo_class(), *this, std::forward<F>(f));
   }

   ::efl::eolian::return_traits<void>::type align_get( ::efl::eolian::out_traits<double>::type horizontal,  ::efl::eolian::out_traits<double>::type vertical) const;
   ::efl::eolian::return_traits<void>::type align_set( ::efl::eolian::in_traits<double>::type horizontal,  ::efl::eolian::in_traits<double>::type vertical) const;
   ::efl::eolian::return_traits<void>::type padding_get( ::efl::eolian::out_traits<int>::type horizontal,  ::efl::eolian::out_traits<int>::type vertical) const;
   ::efl::eolian::return_traits<void>::type padding_set( ::efl::eolian::in_traits<int>::type horizontal,  ::efl::eolian::in_traits<int>::type vertical) const;
   ::efl::eolian::return_traits<void>::type layout_set( ::efl::eolian::in_traits<Evas_Object_Box_Layout>::type cb,  ::efl::eolian::in_traits<void const*>::type data,  ::efl::eolian::in_traits<Eina_Free_Cb>::type free_data) const;
   ::efl::eolian::return_traits<void>::type layout_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<void>::type layout_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<void>::type layout_homogeneous_max_size_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type internal_remove( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<void>::type layout_flow_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<void>::type internal_option_free( ::efl::eolian::in_traits<Evas_Object_Box_Option const*>::type opt) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type insert_after( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const;
   ::efl::eolian::return_traits<bool>::type remove_all( ::efl::eolian::in_traits<bool>::type clear) const;
   ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type iterator_new() const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type add_to() const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type append( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<int>::type option_property_id_get( ::efl::eolian::in_traits< ::efl::eina::string_view>::type name) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type prepend( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type accessor_new() const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_append( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type internal_remove_at( ::efl::eolian::in_traits< unsigned int>::type pos) const;
   ::efl::eolian::return_traits<bool>::type remove_at( ::efl::eolian::in_traits< unsigned int>::type pos) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_insert_at( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned int>::type pos) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type insert_before( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const;
   ::efl::eolian::return_traits< ::efl::eina::string_view>::type option_property_name_get( ::efl::eolian::in_traits<int>::type property) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_insert_before( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const;
   ::efl::eolian::return_traits<void>::type layout_homogeneous_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_option_new( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<void>::type layout_homogeneous_max_size_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_insert_after( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type insert_at( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned int>::type pos) const;
   ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type internal_prepend( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<bool>::type remove( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<void>::type layout_stack( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<void>::type layout_homogeneous_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<void>::type layout_flow_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const;
   ::efl::eolian::return_traits<int>::type count() const;
   static Efl_Class const* _eo_class()
   {
      return EVAS_BOX_CLASS;
   }
   Eo* _eo_ptr() const { return *(reinterpret_cast<Eo **>(const_cast<Box *>(this))); }
   static struct child_added_event
   {
      static Efl_Event_Description const* description()
      { return EVAS_BOX_EVENT_CHILD_ADDED; }
      typedef Evas_Object_Box_Option const* parameter_type;
   } const child_added_event;
#ifdef EFL_CXXPERIMENTAL
   template <typename F>
   typename std::enable_if<std::is_bind_expression<F>::value, ::efl::eolian::signal_connection>::type
   child_added_event_cb_add(F function)
   {
      return ::efl::eolian::event_add(child_added_event, *this, function);
   }
   template <typename F>
   typename std::enable_if<!std::is_bind_expression<F>::value, ::efl::eolian::signal_connection>::type
   child_added_event_cb_add(F function)
   {
      return ::efl::eolian::event_add(child_added_event, *this, std::bind(function));
   }
#endif
   static struct child_removed_event
   {
      static Efl_Event_Description const* description()
      { return EVAS_BOX_EVENT_CHILD_REMOVED; }
      typedef  ::efl::canvas::Object parameter_type;
   } const child_removed_event;
#ifdef EFL_CXXPERIMENTAL
   template <typename F>
   typename std::enable_if<std::is_bind_expression<F>::value, ::efl::eolian::signal_connection>::type
   child_removed_event_cb_add(F function)
   {
      return ::efl::eolian::event_add(child_removed_event, *this, function);
   }
   template <typename F>
   typename std::enable_if<!std::is_bind_expression<F>::value, ::efl::eolian::signal_connection>::type
   child_removed_event_cb_add(F function)
   {
      return ::efl::eolian::event_add(child_removed_event, *this, std::bind(function));
   }
#endif
   /// @cond LOCAL
    ::efl::eolian::address_of_operator<Box,  ::efl::Loop_Consumer,  ::efl::Object,  ::efl::canvas::Group,  ::efl::canvas::Object,  ::efl::canvas::Pointer,  ::efl::gfx::Color,  ::efl::gfx::Entity,  ::efl::gfx::Hint,  ::efl::gfx::Mapping,  ::efl::gfx::Stack,  ::efl::input::Interface,  ::efl::ui::I18n> operator&() { return {this}; }
    ::efl::eolian::address_of_operator<Box const ,  ::efl::Loop_Consumer const ,  ::efl::Object const ,  ::efl::canvas::Group const ,  ::efl::canvas::Object const ,  ::efl::canvas::Pointer const ,  ::efl::gfx::Color const ,  ::efl::gfx::Entity const ,  ::efl::gfx::Hint const ,  ::efl::gfx::Mapping const ,  ::efl::gfx::Stack const ,  ::efl::input::Interface const ,  ::efl::ui::I18n const > operator&() const { return {this}; }
   /// @endcond
   ::efl::eo::wref<Box> _get_wref() const { return ::efl::eo::wref<Box>(*this); }
   ::efl::eo::concrete const& _get_concrete() const { return *this; }
   ::efl::eo::concrete& _get_concrete() { return *this; }
   using ::efl::eo::concrete::_eo_ptr;
   using ::efl::eo::concrete::_release;
   using ::efl::eo::concrete::_reset;
   using ::efl::eo::concrete::_delete;
   using ::efl::eo::concrete::operator bool;
#ifdef EFL_CXXPERIMENTAL
   const Box* operator->() const { return this; }
   Box* operator->() { return this; }
   operator Eo*() const { return _eo_ptr(); }
#endif 
   friend bool operator==(Box const& lhs, Box const& rhs)
   { return lhs._get_concrete() == rhs._get_concrete(); }
   friend bool operator!=(Box const& lhs, Box const& rhs)
   { return !(lhs == rhs); }
};
static_assert(sizeof(Box) == sizeof(Eo*), "");
static_assert(std::is_standard_layout<Box>::value, "");
} 
#endif
