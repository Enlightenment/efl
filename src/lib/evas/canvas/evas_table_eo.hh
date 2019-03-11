#ifndef EVAS_TABLE_EO_HH
#define EVAS_TABLE_EO_HH
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
#include "evas_table_eo.h"
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
#ifndef EVAS_TABLE_FWD_GUARD
#define EVAS_TABLE_FWD_GUARD
namespace evas { 
struct Table;
} 
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Table> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Table&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Table const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Table const&> : ::std::true_type {}; } }
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
struct Table {
   ::efl::eolian::return_traits<Evas_Object_Table_Homogeneous_Mode>::type homogeneous_get() const;
   ::efl::eolian::return_traits<void>::type homogeneous_set( ::efl::eolian::in_traits<Evas_Object_Table_Homogeneous_Mode>::type homogeneous) const;
   ::efl::eolian::return_traits<void>::type align_get( ::efl::eolian::out_traits<double>::type horizontal,  ::efl::eolian::out_traits<double>::type vertical) const;
   ::efl::eolian::return_traits<void>::type align_set( ::efl::eolian::in_traits<double>::type horizontal,  ::efl::eolian::in_traits<double>::type vertical) const;
   ::efl::eolian::return_traits<void>::type padding_get( ::efl::eolian::out_traits<int>::type horizontal,  ::efl::eolian::out_traits<int>::type vertical) const;
   ::efl::eolian::return_traits<void>::type padding_set( ::efl::eolian::in_traits<int>::type horizontal,  ::efl::eolian::in_traits<int>::type vertical) const;
   ::efl::eolian::return_traits<void>::type col_row_size_get( ::efl::eolian::out_traits<int>::type cols,  ::efl::eolian::out_traits<int>::type rows) const;
   ::efl::eolian::return_traits<  ::efl::eina::list< ::efl::canvas::Object>>::type children_get() const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type child_get( ::efl::eolian::in_traits< unsigned short>::type col,  ::efl::eolian::in_traits< unsigned short>::type row) const;
   ::efl::eolian::return_traits<void>::type clear( ::efl::eolian::in_traits<bool>::type clear) const;
   ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type accessor_new() const;
   ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type iterator_new() const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type add_to() const;
   ::efl::eolian::return_traits<bool>::type pack_get( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::out_traits< unsigned short>::type col,  ::efl::eolian::out_traits< unsigned short>::type row,  ::efl::eolian::out_traits< unsigned short>::type colspan,  ::efl::eolian::out_traits< unsigned short>::type rowspan) const;
   ::efl::eolian::return_traits<bool>::type pack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned short>::type col,  ::efl::eolian::in_traits< unsigned short>::type row,  ::efl::eolian::in_traits< unsigned short>::type colspan,  ::efl::eolian::in_traits< unsigned short>::type rowspan) const;
   ::efl::eolian::return_traits<bool>::type unpack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<int>::type count() const;
   static Efl_Class const* _eo_class()
   {
      return EVAS_TABLE_CLASS;
   }
   Eo* _eo_ptr() const { return *(reinterpret_cast<Eo **>(const_cast<Table *>(this))); }
   operator ::evas::Table() const;
   operator ::evas::Table&();
   operator ::evas::Table const&() const;
   /// @cond LOCAL
    ::efl::eolian::address_of_operator<Table,  ::efl::Loop_Consumer,  ::efl::Object,  ::efl::canvas::Group,  ::efl::canvas::Object,  ::efl::canvas::Pointer,  ::efl::gfx::Color,  ::efl::gfx::Entity,  ::efl::gfx::Hint,  ::efl::gfx::Mapping,  ::efl::gfx::Stack,  ::efl::input::Interface,  ::efl::ui::I18n> operator&() { return {this}; }
    ::efl::eolian::address_of_operator<Table const ,  ::efl::Loop_Consumer const ,  ::efl::Object const ,  ::efl::canvas::Group const ,  ::efl::canvas::Object const ,  ::efl::canvas::Pointer const ,  ::efl::gfx::Color const ,  ::efl::gfx::Entity const ,  ::efl::gfx::Hint const ,  ::efl::gfx::Mapping const ,  ::efl::gfx::Stack const ,  ::efl::input::Interface const ,  ::efl::ui::I18n const > operator&() const { return {this}; }
   /// @endcond
};
} 
}
namespace evas { 
struct Table : private ::efl::eo::concrete
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
   explicit Table( ::Eo* eo)
      : ::efl::eo::concrete(eo) {}
   Table(std::nullptr_t)
      : ::efl::eo::concrete(nullptr) {}
   explicit Table() = default;
   Table(Table const&) = default;
   Table(Table&&) = default;
   Table& operator=(Table const&) = default;
   Table& operator=(Table&&) = default;
   template <typename Derived>
   Table(Derived&& derived
      , typename std::enable_if<
         ::efl::eo::is_eolian_object<Derived>::value
          && std::is_base_of< Table, Derived>::value>::type* = 0)
         : ::efl::eo::concrete(derived._eo_ptr()) {}

   Table( ::efl::eo::instantiate_t)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class());
   }
   template <typename T>
   explicit Table( ::efl::eo::instantiate_t, T&& parent, typename std::enable_if< ::efl::eo::is_eolian_object<T>::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, parent, _eo_class());
   }
   template <typename F> Table( ::efl::eo::instantiate_t, F&& f, typename ::std::enable_if< ::efl::eolian::is_constructor_lambda<F, Table >::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class(), *this, std::forward<F>(f));
   }
   template <typename T, typename F> Table(  ::efl::eo::instantiate_t, T&& parent, F&& f, typename ::std::enable_if< ::efl::eolian::is_constructor_lambda<F, Table >::value && ::efl::eo::is_eolian_object<T>::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, parent, _eo_class(), *this, std::forward<F>(f));
   }

   ::efl::eolian::return_traits<Evas_Object_Table_Homogeneous_Mode>::type homogeneous_get() const;
   ::efl::eolian::return_traits<void>::type homogeneous_set( ::efl::eolian::in_traits<Evas_Object_Table_Homogeneous_Mode>::type homogeneous) const;
   ::efl::eolian::return_traits<void>::type align_get( ::efl::eolian::out_traits<double>::type horizontal,  ::efl::eolian::out_traits<double>::type vertical) const;
   ::efl::eolian::return_traits<void>::type align_set( ::efl::eolian::in_traits<double>::type horizontal,  ::efl::eolian::in_traits<double>::type vertical) const;
   ::efl::eolian::return_traits<void>::type padding_get( ::efl::eolian::out_traits<int>::type horizontal,  ::efl::eolian::out_traits<int>::type vertical) const;
   ::efl::eolian::return_traits<void>::type padding_set( ::efl::eolian::in_traits<int>::type horizontal,  ::efl::eolian::in_traits<int>::type vertical) const;
   ::efl::eolian::return_traits<void>::type col_row_size_get( ::efl::eolian::out_traits<int>::type cols,  ::efl::eolian::out_traits<int>::type rows) const;
   ::efl::eolian::return_traits<  ::efl::eina::list< ::efl::canvas::Object>>::type children_get() const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type child_get( ::efl::eolian::in_traits< unsigned short>::type col,  ::efl::eolian::in_traits< unsigned short>::type row) const;
   ::efl::eolian::return_traits<void>::type clear( ::efl::eolian::in_traits<bool>::type clear) const;
   ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type accessor_new() const;
   ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type iterator_new() const;
   ::efl::eolian::return_traits< ::efl::canvas::Object>::type add_to() const;
   ::efl::eolian::return_traits<bool>::type pack_get( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::out_traits< unsigned short>::type col,  ::efl::eolian::out_traits< unsigned short>::type row,  ::efl::eolian::out_traits< unsigned short>::type colspan,  ::efl::eolian::out_traits< unsigned short>::type rowspan) const;
   ::efl::eolian::return_traits<bool>::type pack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned short>::type col,  ::efl::eolian::in_traits< unsigned short>::type row,  ::efl::eolian::in_traits< unsigned short>::type colspan,  ::efl::eolian::in_traits< unsigned short>::type rowspan) const;
   ::efl::eolian::return_traits<bool>::type unpack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const;
   ::efl::eolian::return_traits<int>::type count() const;
   static Efl_Class const* _eo_class()
   {
      return EVAS_TABLE_CLASS;
   }
   Eo* _eo_ptr() const { return *(reinterpret_cast<Eo **>(const_cast<Table *>(this))); }
   /// @cond LOCAL
    ::efl::eolian::address_of_operator<Table,  ::efl::Loop_Consumer,  ::efl::Object,  ::efl::canvas::Group,  ::efl::canvas::Object,  ::efl::canvas::Pointer,  ::efl::gfx::Color,  ::efl::gfx::Entity,  ::efl::gfx::Hint,  ::efl::gfx::Mapping,  ::efl::gfx::Stack,  ::efl::input::Interface,  ::efl::ui::I18n> operator&() { return {this}; }
    ::efl::eolian::address_of_operator<Table const ,  ::efl::Loop_Consumer const ,  ::efl::Object const ,  ::efl::canvas::Group const ,  ::efl::canvas::Object const ,  ::efl::canvas::Pointer const ,  ::efl::gfx::Color const ,  ::efl::gfx::Entity const ,  ::efl::gfx::Hint const ,  ::efl::gfx::Mapping const ,  ::efl::gfx::Stack const ,  ::efl::input::Interface const ,  ::efl::ui::I18n const > operator&() const { return {this}; }
   /// @endcond
   ::efl::eo::wref<Table> _get_wref() const { return ::efl::eo::wref<Table>(*this); }
   ::efl::eo::concrete const& _get_concrete() const { return *this; }
   ::efl::eo::concrete& _get_concrete() { return *this; }
   using ::efl::eo::concrete::_eo_ptr;
   using ::efl::eo::concrete::_release;
   using ::efl::eo::concrete::_reset;
   using ::efl::eo::concrete::_delete;
   using ::efl::eo::concrete::operator bool;
#ifdef EFL_CXXPERIMENTAL
   const Table* operator->() const { return this; }
   Table* operator->() { return this; }
   operator Eo*() const { return _eo_ptr(); }
#endif 
   friend bool operator==(Table const& lhs, Table const& rhs)
   { return lhs._get_concrete() == rhs._get_concrete(); }
   friend bool operator!=(Table const& lhs, Table const& rhs)
   { return !(lhs == rhs); }
};
static_assert(sizeof(Table) == sizeof(Eo*), "");
static_assert(std::is_standard_layout<Table>::value, "");
} 
#endif
