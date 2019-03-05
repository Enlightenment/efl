#ifndef EVAS_CANVAS_IMPL_HH
#define EVAS_CANVAS_IMPL_HH

#include "evas_canvas_eo.hh"

namespace evas { 
inline ::efl::eolian::return_traits<int>::type Canvas::image_cache_get() const
{
   int __return_value =  ::evas_canvas_image_cache_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::image_cache_set( ::efl::eolian::in_traits<int>::type size) const
{
    ::evas_canvas_image_cache_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(size));
}
inline ::efl::eolian::return_traits<Efl_Input_Flags>::type Canvas::event_default_flags_get() const
{
   Efl_Input_Flags __return_value =  ::evas_canvas_event_default_flags_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Efl_Input_Flags>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::event_default_flags_set( ::efl::eolian::in_traits<Efl_Input_Flags>::type flags) const
{
    ::evas_canvas_event_default_flags_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Efl_Input_Flags,  ::efl::eolian::in_traits<Efl_Input_Flags>::type>(flags));
}
inline ::efl::eolian::return_traits<int>::type Canvas::font_cache_get() const
{
   int __return_value =  ::evas_canvas_font_cache_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::font_cache_set( ::efl::eolian::in_traits<int>::type size) const
{
    ::evas_canvas_font_cache_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(size));
}
inline ::efl::eolian::return_traits<void*>::type Canvas::data_attach_get() const
{
   void * __return_value =  ::evas_canvas_data_attach_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<void*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::data_attach_set( ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_canvas_data_attach_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Canvas::focus_get() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_canvas_focus_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Canvas::seat_focus_get( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_canvas_seat_focus_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Canvas::object_top_get() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_canvas_object_top_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::pointer_canvas_xy_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev,  ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const
{
   int __out_param_x = {};
   int __out_param_y = {};
    ::evas_canvas_pointer_canvas_xy_by_device_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(dev),
      & __out_param_x,
      & __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y, __out_param_y);
}
inline ::efl::eolian::return_traits<void>::type Canvas::pointer_canvas_xy_get( ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const
{
   int __out_param_x = {};
   int __out_param_y = {};
    ::evas_canvas_pointer_canvas_xy_get(_eo_ptr(),
      & __out_param_x,
      & __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y, __out_param_y);
}
inline ::efl::eolian::return_traits<int>::type Canvas::event_down_count_get() const
{
   int __return_value =  ::evas_canvas_event_down_count_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Canvas::smart_objects_calculate_count_get() const
{
   int __return_value =  ::evas_canvas_smart_objects_calculate_count_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Canvas::focus_state_get() const
{
   Eina_Bool __return_value =  ::evas_canvas_focus_state_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Canvas::seat_focus_state_get( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
   Eina_Bool __return_value =  ::evas_canvas_seat_focus_state_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Canvas::changed_get() const
{
   Eina_Bool __return_value =  ::evas_canvas_changed_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::pointer_output_xy_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev,  ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const
{
   int __out_param_x = {};
   int __out_param_y = {};
    ::evas_canvas_pointer_output_xy_by_device_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(dev),
      & __out_param_x,
      & __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y, __out_param_y);
}
inline ::efl::eolian::return_traits<void>::type Canvas::pointer_output_xy_get( ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const
{
   int __out_param_x = {};
   int __out_param_y = {};
    ::evas_canvas_pointer_output_xy_get(_eo_ptr(),
      & __out_param_x,
      & __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y, __out_param_y);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Canvas::object_bottom_get() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_canvas_object_bottom_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits< unsigned int>::type Canvas::pointer_button_down_mask_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev) const
{
   unsigned int __return_value =  ::evas_canvas_pointer_button_down_mask_by_device_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(dev));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< unsigned int>::type>(__return_value);
}
inline ::efl::eolian::return_traits< unsigned int>::type Canvas::pointer_button_down_mask_get() const
{
   unsigned int __return_value =  ::evas_canvas_pointer_button_down_mask_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< unsigned int>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::input::Device>::type Canvas::default_device_get( ::efl::eolian::in_traits<Efl_Input_Device_Type>::type type) const
{
    ::Efl_Input_Device* __return_value =  ::evas_canvas_default_device_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<Efl_Input_Device_Type,  ::efl::eolian::in_traits<Efl_Input_Device_Type>::type>(type));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::input::Device>::type>(__return_value);
}
inline ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::canvas::Object>>::type Canvas::tree_objects_at_xy_get( ::efl::eolian::in_traits< ::efl::canvas::Object>::type stop,  ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y) const
{
   Eina_List * __return_value =  ::evas_canvas_tree_objects_at_xy_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(stop),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_lock_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_lock_on(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_key_lock_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_key_lock_on(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_key_lock_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_key_lock_off(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_modifier_add( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_modifier_add(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_modifier_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_modifier_off(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<bool>::type Canvas::render_async() const
{
   Eina_Bool __return_value =  ::evas_canvas_render_async(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::focus_out() const
{
    ::evas_canvas_focus_out(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::norender() const
{
    ::evas_canvas_norender(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::nochange_pop() const
{
    ::evas_canvas_nochange_pop(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_lock_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_lock_off(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::nochange_push() const
{
    ::evas_canvas_nochange_push(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::font_cache_flush() const
{
    ::evas_canvas_font_cache_flush(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_modifier_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_modifier_on(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_key_modifier_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_key_modifier_on(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_key_modifier_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_key_modifier_off(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type Canvas::font_available_list() const
{
   Eina_List * __return_value =  ::evas_canvas_font_available_list(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Canvas::object_name_find( ::efl::eolian::in_traits< ::efl::eina::string_view>::type name) const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_canvas_object_name_find(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(name));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::font_path_append( ::efl::eolian::in_traits< ::efl::eina::string_view>::type path) const
{
    ::evas_canvas_font_path_append(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(path));
}
inline ::efl::eolian::return_traits<void>::type Canvas::font_path_clear() const
{
    ::evas_canvas_font_path_clear(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::touch_point_list_nth_xy_get( ::efl::eolian::in_traits< unsigned int>::type n,  ::efl::eolian::out_traits<double>::type x,  ::efl::eolian::out_traits<double>::type y) const
{
   double __out_param_x = {};
   double __out_param_y = {};
    ::evas_canvas_touch_point_list_nth_xy_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<unsigned int,  ::efl::eolian::in_traits< unsigned int>::type>(n),
      & __out_param_x,
      & __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(y, __out_param_y);
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_lock_del( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_lock_del(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::damage_rectangle_add( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
    ::evas_canvas_damage_rectangle_add(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
}
inline ::efl::eolian::return_traits<void>::type Canvas::sync() const
{
    ::evas_canvas_sync(_eo_ptr());
}
inline ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type Canvas::font_path_list() const
{
   const Eina_List * __return_value =  ::evas_canvas_font_path_list(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::image_cache_reload() const
{
    ::evas_canvas_image_cache_reload(_eo_ptr());
}
inline ::efl::eolian::return_traits<int>::type Canvas::coord_world_x_to_screen( ::efl::eolian::in_traits<int>::type x) const
{
   int __return_value =  ::evas_canvas_coord_world_x_to_screen(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<  ::efl::eina::list<Eina_Rect const&>>::type Canvas::render_updates() const
{
   Eina_List * __return_value =  ::evas_canvas_render_updates(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::list<Eina_Rect const&>>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::image_cache_flush() const
{
    ::evas_canvas_image_cache_flush(_eo_ptr());
}
inline ::efl::eolian::return_traits<int>::type Canvas::coord_screen_y_to_world( ::efl::eolian::in_traits<int>::type y) const
{
   int __return_value =  ::evas_canvas_coord_screen_y_to_world(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_modifier_del( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_modifier_del(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::focus_in() const
{
    ::evas_canvas_focus_in(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_focus_in( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_focus_in(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_focus_out( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_focus_out(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<void>::type Canvas::obscured_rectangle_add( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
    ::evas_canvas_obscured_rectangle_add(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
}
inline ::efl::eolian::return_traits<void>::type Canvas::render_dump() const
{
    ::evas_canvas_render_dump(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::render() const
{
    ::evas_canvas_render(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::font_path_prepend( ::efl::eolian::in_traits< ::efl::eina::string_view>::type path) const
{
    ::evas_canvas_font_path_prepend(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(path));
}
inline ::efl::eolian::return_traits<void>::type Canvas::obscured_clear() const
{
    ::evas_canvas_obscured_clear(_eo_ptr());
}
inline ::efl::eolian::return_traits<int>::type Canvas::coord_screen_x_to_world( ::efl::eolian::in_traits<int>::type x) const
{
   int __return_value =  ::evas_canvas_coord_screen_x_to_world(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_lock_add( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_lock_add(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::render_idle_flush() const
{
    ::evas_canvas_render_idle_flush(_eo_ptr());
}
inline ::efl::eolian::return_traits<int>::type Canvas::coord_world_y_to_screen( ::efl::eolian::in_traits<int>::type y) const
{
   int __return_value =  ::evas_canvas_coord_world_y_to_screen(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
} 
namespace eo_cxx {
namespace evas { 
inline ::efl::eolian::return_traits<int>::type Canvas::image_cache_get() const
{
   int __return_value =  ::evas_canvas_image_cache_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::image_cache_set( ::efl::eolian::in_traits<int>::type size) const
{
    ::evas_canvas_image_cache_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(size));
}
inline ::efl::eolian::return_traits<Efl_Input_Flags>::type Canvas::event_default_flags_get() const
{
   Efl_Input_Flags __return_value =  ::evas_canvas_event_default_flags_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Efl_Input_Flags>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::event_default_flags_set( ::efl::eolian::in_traits<Efl_Input_Flags>::type flags) const
{
    ::evas_canvas_event_default_flags_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Efl_Input_Flags,  ::efl::eolian::in_traits<Efl_Input_Flags>::type>(flags));
}
inline ::efl::eolian::return_traits<int>::type Canvas::font_cache_get() const
{
   int __return_value =  ::evas_canvas_font_cache_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::font_cache_set( ::efl::eolian::in_traits<int>::type size) const
{
    ::evas_canvas_font_cache_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(size));
}
inline ::efl::eolian::return_traits<void*>::type Canvas::data_attach_get() const
{
   void * __return_value =  ::evas_canvas_data_attach_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<void*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::data_attach_set( ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_canvas_data_attach_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Canvas::focus_get() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_canvas_focus_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Canvas::seat_focus_get( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_canvas_seat_focus_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Canvas::object_top_get() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_canvas_object_top_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::pointer_canvas_xy_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev,  ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const
{
   int __out_param_x = {};
   int __out_param_y = {};
    ::evas_canvas_pointer_canvas_xy_by_device_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(dev),
      & __out_param_x,
      & __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y, __out_param_y);
}
inline ::efl::eolian::return_traits<void>::type Canvas::pointer_canvas_xy_get( ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const
{
   int __out_param_x = {};
   int __out_param_y = {};
    ::evas_canvas_pointer_canvas_xy_get(_eo_ptr(),
      & __out_param_x,
      & __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y, __out_param_y);
}
inline ::efl::eolian::return_traits<int>::type Canvas::event_down_count_get() const
{
   int __return_value =  ::evas_canvas_event_down_count_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Canvas::smart_objects_calculate_count_get() const
{
   int __return_value =  ::evas_canvas_smart_objects_calculate_count_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Canvas::focus_state_get() const
{
   Eina_Bool __return_value =  ::evas_canvas_focus_state_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Canvas::seat_focus_state_get( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
   Eina_Bool __return_value =  ::evas_canvas_seat_focus_state_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Canvas::changed_get() const
{
   Eina_Bool __return_value =  ::evas_canvas_changed_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::pointer_output_xy_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev,  ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const
{
   int __out_param_x = {};
   int __out_param_y = {};
    ::evas_canvas_pointer_output_xy_by_device_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(dev),
      & __out_param_x,
      & __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y, __out_param_y);
}
inline ::efl::eolian::return_traits<void>::type Canvas::pointer_output_xy_get( ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y) const
{
   int __out_param_x = {};
   int __out_param_y = {};
    ::evas_canvas_pointer_output_xy_get(_eo_ptr(),
      & __out_param_x,
      & __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y, __out_param_y);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Canvas::object_bottom_get() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_canvas_object_bottom_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits< unsigned int>::type Canvas::pointer_button_down_mask_by_device_get( ::efl::eolian::in_traits< ::efl::input::Device>::type dev) const
{
   unsigned int __return_value =  ::evas_canvas_pointer_button_down_mask_by_device_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(dev));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< unsigned int>::type>(__return_value);
}
inline ::efl::eolian::return_traits< unsigned int>::type Canvas::pointer_button_down_mask_get() const
{
   unsigned int __return_value =  ::evas_canvas_pointer_button_down_mask_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< unsigned int>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::input::Device>::type Canvas::default_device_get( ::efl::eolian::in_traits<Efl_Input_Device_Type>::type type) const
{
    ::Efl_Input_Device* __return_value =  ::evas_canvas_default_device_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<Efl_Input_Device_Type,  ::efl::eolian::in_traits<Efl_Input_Device_Type>::type>(type));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::input::Device>::type>(__return_value);
}
inline ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::canvas::Object>>::type Canvas::tree_objects_at_xy_get( ::efl::eolian::in_traits< ::efl::canvas::Object>::type stop,  ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y) const
{
   Eina_List * __return_value =  ::evas_canvas_tree_objects_at_xy_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(stop),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_lock_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_lock_on(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_key_lock_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_key_lock_on(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_key_lock_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_key_lock_off(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_modifier_add( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_modifier_add(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_modifier_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_modifier_off(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<bool>::type Canvas::render_async() const
{
   Eina_Bool __return_value =  ::evas_canvas_render_async(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::focus_out() const
{
    ::evas_canvas_focus_out(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::norender() const
{
    ::evas_canvas_norender(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::nochange_pop() const
{
    ::evas_canvas_nochange_pop(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_lock_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_lock_off(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::nochange_push() const
{
    ::evas_canvas_nochange_push(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::font_cache_flush() const
{
    ::evas_canvas_font_cache_flush(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_modifier_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_modifier_on(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_key_modifier_on( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_key_modifier_on(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_key_modifier_off( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname,  ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_key_modifier_off(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type Canvas::font_available_list() const
{
   Eina_List * __return_value =  ::evas_canvas_font_available_list(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Canvas::object_name_find( ::efl::eolian::in_traits< ::efl::eina::string_view>::type name) const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_canvas_object_name_find(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(name));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::font_path_append( ::efl::eolian::in_traits< ::efl::eina::string_view>::type path) const
{
    ::evas_canvas_font_path_append(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(path));
}
inline ::efl::eolian::return_traits<void>::type Canvas::font_path_clear() const
{
    ::evas_canvas_font_path_clear(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::touch_point_list_nth_xy_get( ::efl::eolian::in_traits< unsigned int>::type n,  ::efl::eolian::out_traits<double>::type x,  ::efl::eolian::out_traits<double>::type y) const
{
   double __out_param_x = {};
   double __out_param_y = {};
    ::evas_canvas_touch_point_list_nth_xy_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<unsigned int,  ::efl::eolian::in_traits< unsigned int>::type>(n),
      & __out_param_x,
      & __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(y, __out_param_y);
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_lock_del( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_lock_del(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::damage_rectangle_add( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
    ::evas_canvas_damage_rectangle_add(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
}
inline ::efl::eolian::return_traits<void>::type Canvas::sync() const
{
    ::evas_canvas_sync(_eo_ptr());
}
inline ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type Canvas::font_path_list() const
{
   const Eina_List * __return_value =  ::evas_canvas_font_path_list(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::range_list< ::efl::eina::string_view>>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::image_cache_reload() const
{
    ::evas_canvas_image_cache_reload(_eo_ptr());
}
inline ::efl::eolian::return_traits<int>::type Canvas::coord_world_x_to_screen( ::efl::eolian::in_traits<int>::type x) const
{
   int __return_value =  ::evas_canvas_coord_world_x_to_screen(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<  ::efl::eina::list<Eina_Rect const&>>::type Canvas::render_updates() const
{
   Eina_List * __return_value =  ::evas_canvas_render_updates(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::list<Eina_Rect const&>>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::image_cache_flush() const
{
    ::evas_canvas_image_cache_flush(_eo_ptr());
}
inline ::efl::eolian::return_traits<int>::type Canvas::coord_screen_y_to_world( ::efl::eolian::in_traits<int>::type y) const
{
   int __return_value =  ::evas_canvas_coord_screen_y_to_world(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_modifier_del( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_modifier_del(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::focus_in() const
{
    ::evas_canvas_focus_in(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_focus_in( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_focus_in(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<void>::type Canvas::seat_focus_out( ::efl::eolian::in_traits< ::efl::input::Device>::type seat) const
{
    ::evas_canvas_seat_focus_out(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Input_Device*,  ::efl::eolian::in_traits< ::efl::input::Device>::type>(seat));
}
inline ::efl::eolian::return_traits<void>::type Canvas::obscured_rectangle_add( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
    ::evas_canvas_obscured_rectangle_add(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
}
inline ::efl::eolian::return_traits<void>::type Canvas::render_dump() const
{
    ::evas_canvas_render_dump(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::render() const
{
    ::evas_canvas_render(_eo_ptr());
}
inline ::efl::eolian::return_traits<void>::type Canvas::font_path_prepend( ::efl::eolian::in_traits< ::efl::eina::string_view>::type path) const
{
    ::evas_canvas_font_path_prepend(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(path));
}
inline ::efl::eolian::return_traits<void>::type Canvas::obscured_clear() const
{
    ::evas_canvas_obscured_clear(_eo_ptr());
}
inline ::efl::eolian::return_traits<int>::type Canvas::coord_screen_x_to_world( ::efl::eolian::in_traits<int>::type x) const
{
   int __return_value =  ::evas_canvas_coord_screen_x_to_world(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Canvas::key_lock_add( ::efl::eolian::in_traits< ::efl::eina::string_view>::type keyname) const
{
    ::evas_canvas_key_lock_add(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(keyname));
}
inline ::efl::eolian::return_traits<void>::type Canvas::render_idle_flush() const
{
    ::evas_canvas_render_idle_flush(_eo_ptr());
}
inline ::efl::eolian::return_traits<int>::type Canvas::coord_world_y_to_screen( ::efl::eolian::in_traits<int>::type y) const
{
   int __return_value =  ::evas_canvas_coord_world_y_to_screen(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline evas::Canvas::operator ::evas::Canvas() const { return *static_cast< ::evas::Canvas const*>(static_cast<void const*>(this)); }
inline evas::Canvas::operator ::evas::Canvas&() { return *static_cast< ::evas::Canvas*>(static_cast<void*>(this)); }
inline evas::Canvas::operator ::evas::Canvas const&() const { return *static_cast< ::evas::Canvas const*>(static_cast<void const*>(this)); }
} }
#endif
