#ifndef EVAS_IMAGE_EO_HH
#define EVAS_IMAGE_EO_HH
#include <Eo.h>

extern "C" {
#include "efl_canvas_filter_internal.eo.h"
#include "efl_canvas_image_internal.eo.h"
#include "efl_canvas_object.eo.h"
#include "efl_canvas_pointer.eo.h"
#include "efl_class.eo.h"
#include "efl_file.eo.h"
#include "efl_file_save.eo.h"
#include "efl_gfx_buffer.eo.h"
#include "efl_gfx_color.eo.h"
#include "efl_gfx_entity.eo.h"
#include "efl_gfx_fill.eo.h"
#include "efl_gfx_filter.eo.h"
#include "efl_gfx_hint.eo.h"
#include "efl_gfx_image.eo.h"
#include "efl_gfx_mapping.eo.h"
#include "efl_gfx_stack.eo.h"
#include "efl_gfx_view.eo.h"
#include "efl_input_device.eo.h"
#include "efl_input_focus.eo.h"
#include "efl_input_hold.eo.h"
#include "efl_input_interface.eo.h"
#include "efl_input_key.eo.h"
#include "efl_input_pointer.eo.h"
#include "efl_loop.eo.h"
#include "efl_loop_consumer.eo.h"
#include "efl_object.eo.h"
#include "efl_gfx_orientable.eo.h"
#include "efl_ui_i18n.eo.h"
#include "evas_image_eo.h"
}
#include <Eina.hh>
#include <Eo.hh>
#include "efl_canvas_filter_internal.eo.hh"
#include "efl_canvas_image_internal.eo.hh"
#include "efl_canvas_object.eo.hh"
#include "efl_canvas_pointer.eo.hh"
#include "efl_class.eo.hh"
#include "efl_file.eo.hh"
#include "efl_file_save.eo.hh"
#include "efl_gfx_buffer.eo.hh"
#include "efl_gfx_color.eo.hh"
#include "efl_gfx_entity.eo.hh"
#include "efl_gfx_fill.eo.hh"
#include "efl_gfx_filter.eo.hh"
#include "efl_gfx_hint.eo.hh"
#include "efl_gfx_image.eo.hh"
#include "efl_gfx_mapping.eo.hh"
#include "efl_gfx_stack.eo.hh"
#include "efl_gfx_view.eo.hh"
#include "efl_input_device.eo.hh"
#include "efl_input_focus.eo.hh"
#include "efl_input_hold.eo.hh"
#include "efl_input_interface.eo.hh"
#include "efl_input_key.eo.hh"
#include "efl_input_pointer.eo.hh"
#include "efl_loop.eo.hh"
#include "efl_loop_consumer.eo.hh"
#include "efl_object.eo.hh"
#include "efl_gfx_orientable.eo.hh"
#include "efl_ui_i18n.eo.hh"
#ifndef EVAS_IMAGE_FWD_GUARD
#define EVAS_IMAGE_FWD_GUARD
namespace evas { 
struct Image;
} 
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Image> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Image&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Image const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::evas::Image const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_GFX_BUFFER_FWD_GUARD
#define EFL_GFX_BUFFER_FWD_GUARD
namespace efl { namespace gfx { 
struct Buffer;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Buffer> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Buffer&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Buffer const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Buffer const&> : ::std::true_type {}; } }
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
#ifndef EFL_FILE_FWD_GUARD
#define EFL_FILE_FWD_GUARD
namespace efl { 
struct File;
} 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::File> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::File&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::File const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::File const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_FILE_SAVE_FWD_GUARD
#define EFL_FILE_SAVE_FWD_GUARD
namespace efl { 
struct File_Save;
} 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::File_Save> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::File_Save&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::File_Save const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::File_Save const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_CANVAS_FILTER_INTERNAL_FWD_GUARD
#define EFL_CANVAS_FILTER_INTERNAL_FWD_GUARD
namespace efl { namespace canvas { namespace filter { 
struct Internal;
} } } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::filter::Internal> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::filter::Internal&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::filter::Internal const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::filter::Internal const&> : ::std::true_type {}; } }
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
#ifndef EFL_GFX_FILL_FWD_GUARD
#define EFL_GFX_FILL_FWD_GUARD
namespace efl { namespace gfx { 
struct Fill;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Fill> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Fill&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Fill const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Fill const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_GFX_FILTER_FWD_GUARD
#define EFL_GFX_FILTER_FWD_GUARD
namespace efl { namespace gfx { 
struct Filter;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Filter> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Filter&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Filter const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Filter const&> : ::std::true_type {}; } }
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
#ifndef EFL_GFX_IMAGE_FWD_GUARD
#define EFL_GFX_IMAGE_FWD_GUARD
namespace efl { namespace gfx { 
struct Image;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Image> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Image&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Image const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::Image const&> : ::std::true_type {}; } }
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
#ifndef EFL_ORIENTATION_FWD_GUARD
#define EFL_ORIENTATION_FWD_GUARD
namespace efl { 
struct Orientation;
} 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Orientation> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Orientation&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Orientation const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::Orientation const&> : ::std::true_type {}; } }
#endif
#ifndef EFL_CANVAS_IMAGE_INTERNAL_FWD_GUARD
#define EFL_CANVAS_IMAGE_INTERNAL_FWD_GUARD
namespace efl { namespace canvas { 
struct Image_Internal;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Image_Internal> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Image_Internal&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Image_Internal const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::canvas::Image_Internal const&> : ::std::true_type {}; } }
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
#ifndef EFL_GFX_VIEW_FWD_GUARD
#define EFL_GFX_VIEW_FWD_GUARD
namespace efl { namespace gfx { 
struct View;
} } 
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::View> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::View&> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::View const> : ::std::true_type {}; } }
namespace efl { namespace eo { template<> struct is_eolian_object< ::efl::gfx::View const&> : ::std::true_type {}; } }
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
struct Image {
   static Efl_Class const* _eo_class()
   {
      return EVAS_IMAGE_CLASS;
   }
   Eo* _eo_ptr() const { return *(reinterpret_cast<Eo **>(const_cast<Image *>(this))); }
   operator ::evas::Image() const;
   operator ::evas::Image&();
   operator ::evas::Image const&() const;
   /// @cond LOCAL
    ::efl::eolian::address_of_operator<Image,  ::efl::File,  ::efl::File_Save,  ::efl::Loop_Consumer,  ::efl::Object,  ::efl::Orientation,  ::efl::canvas::Image_Internal,  ::efl::canvas::Object,  ::efl::canvas::Pointer,  ::efl::canvas::filter::Internal,  ::efl::gfx::Buffer,  ::efl::gfx::Color,  ::efl::gfx::Entity,  ::efl::gfx::Fill,  ::efl::gfx::Filter,  ::efl::gfx::Hint,  ::efl::gfx::Image,  ::efl::gfx::Mapping,  ::efl::gfx::Stack,  ::efl::gfx::View,  ::efl::input::Interface,  ::efl::ui::I18n> operator&() { return {this}; }
    ::efl::eolian::address_of_operator<Image const ,  ::efl::File const ,  ::efl::File_Save const ,  ::efl::Loop_Consumer const ,  ::efl::Object const ,  ::efl::Orientation const ,  ::efl::canvas::Image_Internal const ,  ::efl::canvas::Object const ,  ::efl::canvas::Pointer const ,  ::efl::canvas::filter::Internal const ,  ::efl::gfx::Buffer const ,  ::efl::gfx::Color const ,  ::efl::gfx::Entity const ,  ::efl::gfx::Fill const ,  ::efl::gfx::Filter const ,  ::efl::gfx::Hint const ,  ::efl::gfx::Image const ,  ::efl::gfx::Mapping const ,  ::efl::gfx::Stack const ,  ::efl::gfx::View const ,  ::efl::input::Interface const ,  ::efl::ui::I18n const > operator&() const { return {this}; }
   /// @endcond
};
} 
}
namespace evas { 
struct Image : private ::efl::eo::concrete
   , EO_CXX_INHERIT(::efl::File)
   , EO_CXX_INHERIT(::efl::File_Save)
   , EO_CXX_INHERIT(::efl::Loop_Consumer)
   , EO_CXX_INHERIT(::efl::Object)
   , EO_CXX_INHERIT(::efl::Orientation)
   , EO_CXX_INHERIT(::efl::canvas::Image_Internal)
   , EO_CXX_INHERIT(::efl::canvas::Object)
   , EO_CXX_INHERIT(::efl::canvas::Pointer)
   , EO_CXX_INHERIT(::efl::canvas::filter::Internal)
   , EO_CXX_INHERIT(::efl::gfx::Buffer)
   , EO_CXX_INHERIT(::efl::gfx::Color)
   , EO_CXX_INHERIT(::efl::gfx::Entity)
   , EO_CXX_INHERIT(::efl::gfx::Fill)
   , EO_CXX_INHERIT(::efl::gfx::Filter)
   , EO_CXX_INHERIT(::efl::gfx::Hint)
   , EO_CXX_INHERIT(::efl::gfx::Image)
   , EO_CXX_INHERIT(::efl::gfx::Mapping)
   , EO_CXX_INHERIT(::efl::gfx::Stack)
   , EO_CXX_INHERIT(::efl::gfx::View)
   , EO_CXX_INHERIT(::efl::input::Interface)
   , EO_CXX_INHERIT(::efl::ui::I18n)
{
   explicit Image( ::Eo* eo)
      : ::efl::eo::concrete(eo) {}
   Image(std::nullptr_t)
      : ::efl::eo::concrete(nullptr) {}
   explicit Image() = default;
   Image(Image const&) = default;
   Image(Image&&) = default;
   Image& operator=(Image const&) = default;
   Image& operator=(Image&&) = default;
   template <typename Derived>
   Image(Derived&& derived
      , typename std::enable_if<
         ::efl::eo::is_eolian_object<Derived>::value
          && std::is_base_of< Image, Derived>::value>::type* = 0)
         : ::efl::eo::concrete(derived._eo_ptr()) {}

   Image( ::efl::eo::instantiate_t)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class());
   }
   template <typename T>
   explicit Image( ::efl::eo::instantiate_t, T&& parent, typename std::enable_if< ::efl::eo::is_eolian_object<T>::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, parent, _eo_class());
   }
   template <typename F> Image( ::efl::eo::instantiate_t, F&& f, typename ::std::enable_if< ::efl::eolian::is_constructor_lambda<F, Image >::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class(), *this, std::forward<F>(f));
   }
   template <typename T, typename F> Image(  ::efl::eo::instantiate_t, T&& parent, F&& f, typename ::std::enable_if< ::efl::eolian::is_constructor_lambda<F, Image >::value && ::efl::eo::is_eolian_object<T>::value>::type* = 0)
   {
      ::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, parent, _eo_class(), *this, std::forward<F>(f));
   }

   static Efl_Class const* _eo_class()
   {
      return EVAS_IMAGE_CLASS;
   }
   Eo* _eo_ptr() const { return *(reinterpret_cast<Eo **>(const_cast<Image *>(this))); }
   /// @cond LOCAL
    ::efl::eolian::address_of_operator<Image,  ::efl::File,  ::efl::File_Save,  ::efl::Loop_Consumer,  ::efl::Object,  ::efl::Orientation,  ::efl::canvas::Image_Internal,  ::efl::canvas::Object,  ::efl::canvas::Pointer,  ::efl::canvas::filter::Internal,  ::efl::gfx::Buffer,  ::efl::gfx::Color,  ::efl::gfx::Entity,  ::efl::gfx::Fill,  ::efl::gfx::Filter,  ::efl::gfx::Hint,  ::efl::gfx::Image,  ::efl::gfx::Mapping,  ::efl::gfx::Stack,  ::efl::gfx::View,  ::efl::input::Interface,  ::efl::ui::I18n> operator&() { return {this}; }
    ::efl::eolian::address_of_operator<Image const ,  ::efl::File const ,  ::efl::File_Save const ,  ::efl::Loop_Consumer const ,  ::efl::Object const ,  ::efl::Orientation const ,  ::efl::canvas::Image_Internal const ,  ::efl::canvas::Object const ,  ::efl::canvas::Pointer const ,  ::efl::canvas::filter::Internal const ,  ::efl::gfx::Buffer const ,  ::efl::gfx::Color const ,  ::efl::gfx::Entity const ,  ::efl::gfx::Fill const ,  ::efl::gfx::Filter const ,  ::efl::gfx::Hint const ,  ::efl::gfx::Image const ,  ::efl::gfx::Mapping const ,  ::efl::gfx::Stack const ,  ::efl::gfx::View const ,  ::efl::input::Interface const ,  ::efl::ui::I18n const > operator&() const { return {this}; }
   /// @endcond
   ::efl::eo::wref<Image> _get_wref() const { return ::efl::eo::wref<Image>(*this); }
   ::efl::eo::concrete const& _get_concrete() const { return *this; }
   ::efl::eo::concrete& _get_concrete() { return *this; }
   using ::efl::eo::concrete::_eo_ptr;
   using ::efl::eo::concrete::_release;
   using ::efl::eo::concrete::_reset;
   using ::efl::eo::concrete::_delete;
   using ::efl::eo::concrete::operator bool;
#ifdef EFL_CXXPERIMENTAL
   const Image* operator->() const { return this; }
   Image* operator->() { return this; }
   operator Eo*() const { return _eo_ptr(); }
#endif 
   friend bool operator==(Image const& lhs, Image const& rhs)
   { return lhs._get_concrete() == rhs._get_concrete(); }
   friend bool operator!=(Image const& lhs, Image const& rhs)
   { return !(lhs == rhs); }
};
static_assert(sizeof(Image) == sizeof(Eo*), "");
static_assert(std::is_standard_layout<Image>::value, "");
} 
#endif
