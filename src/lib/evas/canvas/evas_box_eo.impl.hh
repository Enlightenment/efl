#ifndef EVAS_BOX_IMPL_HH
#define EVAS_BOX_IMPL_HH

#include "evas_box_eo.hh"

namespace evas { 
inline ::efl::eolian::return_traits<void>::type Box::align_get( ::efl::eolian::out_traits<double>::type horizontal,  ::efl::eolian::out_traits<double>::type vertical) const
{
   double __out_param_horizontal = {};
   double __out_param_vertical = {};
    ::evas_obj_box_align_get(_eo_ptr(),
      & __out_param_horizontal,
      & __out_param_vertical);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(horizontal, __out_param_horizontal);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(vertical, __out_param_vertical);
}
inline ::efl::eolian::return_traits<void>::type Box::align_set( ::efl::eolian::in_traits<double>::type horizontal,  ::efl::eolian::in_traits<double>::type vertical) const
{
    ::evas_obj_box_align_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<double,  ::efl::eolian::in_traits<double>::type>(horizontal),
       ::efl::eolian::convert_to_c<double,  ::efl::eolian::in_traits<double>::type>(vertical));
}
inline ::efl::eolian::return_traits<void>::type Box::padding_get( ::efl::eolian::out_traits<int>::type horizontal,  ::efl::eolian::out_traits<int>::type vertical) const
{
   int __out_param_horizontal = {};
   int __out_param_vertical = {};
    ::evas_obj_box_padding_get(_eo_ptr(),
      & __out_param_horizontal,
      & __out_param_vertical);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(horizontal, __out_param_horizontal);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(vertical, __out_param_vertical);
}
inline ::efl::eolian::return_traits<void>::type Box::padding_set( ::efl::eolian::in_traits<int>::type horizontal,  ::efl::eolian::in_traits<int>::type vertical) const
{
    ::evas_obj_box_padding_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(horizontal),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(vertical));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_set( ::efl::eolian::in_traits<Evas_Object_Box_Layout>::type cb,  ::efl::eolian::in_traits<void const*>::type data,  ::efl::eolian::in_traits<Eina_Free_Cb>::type free_data) const
{
    ::evas_obj_box_layout_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Layout,  ::efl::eolian::in_traits<Evas_Object_Box_Layout>::type>(cb),
       ::efl::eolian::convert_to_c<const void *,  ::efl::eolian::in_traits<void const*>::type>(data),
       ::efl::eolian::convert_to_c<Eina_Free_Cb,  ::efl::eolian::in_traits<Eina_Free_Cb>::type>(free_data));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_horizontal(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_vertical(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_homogeneous_max_size_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_homogeneous_max_size_horizontal(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Box::internal_remove( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_box_internal_remove(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Box::layout_flow_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_flow_vertical(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<void>::type Box::internal_option_free( ::efl::eolian::in_traits<Evas_Object_Box_Option const*>::type opt) const
{
    ::evas_obj_box_internal_option_free(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Option *,  ::efl::eolian::in_traits<Evas_Object_Box_Option const*>::type>(opt));
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::insert_after( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_insert_after(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object const*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(reference));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Box::remove_all( ::efl::eolian::in_traits<bool>::type clear) const
{
   Eina_Bool __return_value =  ::evas_obj_box_remove_all(_eo_ptr(),
       ::efl::eolian::convert_to_c<Eina_Bool,  ::efl::eolian::in_traits<bool>::type>(clear));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type Box::iterator_new() const
{
   Eina_Iterator * __return_value =  ::evas_obj_box_iterator_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Box::add_to() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_box_add_to(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::append( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_append(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Box::option_property_id_get( ::efl::eolian::in_traits< ::efl::eina::string_view>::type name) const
{
   int __return_value =  ::evas_obj_box_option_property_id_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(name));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::prepend( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_prepend(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type Box::accessor_new() const
{
   Eina_Accessor * __return_value =  ::evas_obj_box_accessor_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_append( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_append(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Box::internal_remove_at( ::efl::eolian::in_traits< unsigned int>::type pos) const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_box_internal_remove_at(_eo_ptr(),
       ::efl::eolian::convert_to_c<unsigned int,  ::efl::eolian::in_traits< unsigned int>::type>(pos));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Box::remove_at( ::efl::eolian::in_traits< unsigned int>::type pos) const
{
   Eina_Bool __return_value =  ::evas_obj_box_remove_at(_eo_ptr(),
       ::efl::eolian::convert_to_c<unsigned int,  ::efl::eolian::in_traits< unsigned int>::type>(pos));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_insert_at( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned int>::type pos) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_insert_at(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c<unsigned int,  ::efl::eolian::in_traits< unsigned int>::type>(pos));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::insert_before( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_insert_before(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object const*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(reference));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::eina::string_view>::type Box::option_property_name_get( ::efl::eolian::in_traits<int>::type property) const
{
   const char * __return_value =  ::evas_obj_box_option_property_name_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(property));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::string_view>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_insert_before( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_insert_before(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object const*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(reference));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Box::layout_homogeneous_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_homogeneous_horizontal(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_option_new( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_option_new(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Box::layout_homogeneous_max_size_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_homogeneous_max_size_vertical(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_insert_after( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_insert_after(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object const*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(reference));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::insert_at( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned int>::type pos) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_insert_at(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c<unsigned int,  ::efl::eolian::in_traits< unsigned int>::type>(pos));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_prepend( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_prepend(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Box::remove( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Eina_Bool __return_value =  ::evas_obj_box_remove(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Box::layout_stack( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_stack(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_homogeneous_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_homogeneous_vertical(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_flow_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_flow_horizontal(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<int>::type Box::count() const
{
   int __return_value =  ::evas_obj_box_count(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
} 
namespace eo_cxx {
namespace evas { 
inline ::efl::eolian::return_traits<void>::type Box::align_get( ::efl::eolian::out_traits<double>::type horizontal,  ::efl::eolian::out_traits<double>::type vertical) const
{
   double __out_param_horizontal = {};
   double __out_param_vertical = {};
    ::evas_obj_box_align_get(_eo_ptr(),
      & __out_param_horizontal,
      & __out_param_vertical);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(horizontal, __out_param_horizontal);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(vertical, __out_param_vertical);
}
inline ::efl::eolian::return_traits<void>::type Box::align_set( ::efl::eolian::in_traits<double>::type horizontal,  ::efl::eolian::in_traits<double>::type vertical) const
{
    ::evas_obj_box_align_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<double,  ::efl::eolian::in_traits<double>::type>(horizontal),
       ::efl::eolian::convert_to_c<double,  ::efl::eolian::in_traits<double>::type>(vertical));
}
inline ::efl::eolian::return_traits<void>::type Box::padding_get( ::efl::eolian::out_traits<int>::type horizontal,  ::efl::eolian::out_traits<int>::type vertical) const
{
   int __out_param_horizontal = {};
   int __out_param_vertical = {};
    ::evas_obj_box_padding_get(_eo_ptr(),
      & __out_param_horizontal,
      & __out_param_vertical);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(horizontal, __out_param_horizontal);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(vertical, __out_param_vertical);
}
inline ::efl::eolian::return_traits<void>::type Box::padding_set( ::efl::eolian::in_traits<int>::type horizontal,  ::efl::eolian::in_traits<int>::type vertical) const
{
    ::evas_obj_box_padding_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(horizontal),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(vertical));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_set( ::efl::eolian::in_traits<Evas_Object_Box_Layout>::type cb,  ::efl::eolian::in_traits<void const*>::type data,  ::efl::eolian::in_traits<Eina_Free_Cb>::type free_data) const
{
    ::evas_obj_box_layout_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Layout,  ::efl::eolian::in_traits<Evas_Object_Box_Layout>::type>(cb),
       ::efl::eolian::convert_to_c<const void *,  ::efl::eolian::in_traits<void const*>::type>(data),
       ::efl::eolian::convert_to_c<Eina_Free_Cb,  ::efl::eolian::in_traits<Eina_Free_Cb>::type>(free_data));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_horizontal(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_vertical(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_homogeneous_max_size_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_homogeneous_max_size_horizontal(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Box::internal_remove( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_box_internal_remove(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Box::layout_flow_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_flow_vertical(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<void>::type Box::internal_option_free( ::efl::eolian::in_traits<Evas_Object_Box_Option const*>::type opt) const
{
    ::evas_obj_box_internal_option_free(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Option *,  ::efl::eolian::in_traits<Evas_Object_Box_Option const*>::type>(opt));
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::insert_after( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_insert_after(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object const*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(reference));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Box::remove_all( ::efl::eolian::in_traits<bool>::type clear) const
{
   Eina_Bool __return_value =  ::evas_obj_box_remove_all(_eo_ptr(),
       ::efl::eolian::convert_to_c<Eina_Bool,  ::efl::eolian::in_traits<bool>::type>(clear));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type Box::iterator_new() const
{
   Eina_Iterator * __return_value =  ::evas_obj_box_iterator_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Box::add_to() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_box_add_to(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::append( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_append(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Box::option_property_id_get( ::efl::eolian::in_traits< ::efl::eina::string_view>::type name) const
{
   int __return_value =  ::evas_obj_box_option_property_id_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(name));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::prepend( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_prepend(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type Box::accessor_new() const
{
   Eina_Accessor * __return_value =  ::evas_obj_box_accessor_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_append( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_append(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Box::internal_remove_at( ::efl::eolian::in_traits< unsigned int>::type pos) const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_box_internal_remove_at(_eo_ptr(),
       ::efl::eolian::convert_to_c<unsigned int,  ::efl::eolian::in_traits< unsigned int>::type>(pos));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Box::remove_at( ::efl::eolian::in_traits< unsigned int>::type pos) const
{
   Eina_Bool __return_value =  ::evas_obj_box_remove_at(_eo_ptr(),
       ::efl::eolian::convert_to_c<unsigned int,  ::efl::eolian::in_traits< unsigned int>::type>(pos));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_insert_at( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned int>::type pos) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_insert_at(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c<unsigned int,  ::efl::eolian::in_traits< unsigned int>::type>(pos));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::insert_before( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_insert_before(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object const*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(reference));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::eina::string_view>::type Box::option_property_name_get( ::efl::eolian::in_traits<int>::type property) const
{
   const char * __return_value =  ::evas_obj_box_option_property_name_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(property));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::string_view>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_insert_before( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_insert_before(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object const*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(reference));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Box::layout_homogeneous_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_homogeneous_horizontal(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_option_new( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_option_new(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Box::layout_homogeneous_max_size_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_homogeneous_max_size_vertical(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_insert_after( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type reference) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_insert_after(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object const*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(reference));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::insert_at( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned int>::type pos) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_insert_at(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c<unsigned int,  ::efl::eolian::in_traits< unsigned int>::type>(pos));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type Box::internal_prepend( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Evas_Object_Box_Option * __return_value =  ::evas_obj_box_internal_prepend(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Box_Option*>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Box::remove( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Eina_Bool __return_value =  ::evas_obj_box_remove(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Box::layout_stack( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_stack(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_homogeneous_vertical( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_homogeneous_vertical(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<void>::type Box::layout_flow_horizontal( ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type priv,  ::efl::eolian::in_traits<void*>::type data) const
{
    ::evas_obj_box_layout_flow_horizontal(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Box_Data *,  ::efl::eolian::in_traits<Evas_Object_Box_Data const*>::type>(priv),
       ::efl::eolian::convert_to_c<void *,  ::efl::eolian::in_traits<void*>::type>(data));
}
inline ::efl::eolian::return_traits<int>::type Box::count() const
{
   int __return_value =  ::evas_obj_box_count(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline evas::Box::operator ::evas::Box() const { return *static_cast< ::evas::Box const*>(static_cast<void const*>(this)); }
inline evas::Box::operator ::evas::Box&() { return *static_cast< ::evas::Box*>(static_cast<void*>(this)); }
inline evas::Box::operator ::evas::Box const&() const { return *static_cast< ::evas::Box const*>(static_cast<void const*>(this)); }
} }
#endif
