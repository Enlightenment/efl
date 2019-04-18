#ifndef EVAS_GRID_IMPL_HH
#define EVAS_GRID_IMPL_HH

#include "evas_grid_eo.hh"

namespace evas { 
inline ::efl::eolian::return_traits<void>::type Grid::grid_size_get( ::efl::eolian::out_traits<int>::type w,  ::efl::eolian::out_traits<int>::type h) const
{
   int __out_param_w = {};
   int __out_param_h = {};
    ::evas_obj_grid_size_get(_eo_ptr(),
      & __out_param_w,
      & __out_param_h);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(w, __out_param_w);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(h, __out_param_h);
}
inline ::efl::eolian::return_traits<void>::type Grid::grid_size_set( ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
    ::evas_obj_grid_size_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
}
inline ::efl::eolian::return_traits<  ::efl::eina::list< ::efl::canvas::Object>>::type Grid::children_get() const
{
   Eina_List * __return_value =  ::evas_obj_grid_children_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::list< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type Grid::accessor_new() const
{
   Eina_Accessor * __return_value =  ::evas_obj_grid_accessor_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Grid::clear( ::efl::eolian::in_traits<bool>::type clear) const
{
    ::evas_obj_grid_clear(_eo_ptr(),
       ::efl::eolian::convert_to_c<Eina_Bool,  ::efl::eolian::in_traits<bool>::type>(clear));
}
inline ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type Grid::iterator_new() const
{
   Eina_Iterator * __return_value =  ::evas_obj_grid_iterator_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Grid::add_to() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_grid_add_to(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Grid::unpack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Eina_Bool __return_value =  ::evas_obj_grid_unpack(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Grid::pack_get( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y,  ::efl::eolian::out_traits<int>::type w,  ::efl::eolian::out_traits<int>::type h) const
{
   int __out_param_x = {};
   int __out_param_y = {};
   int __out_param_w = {};
   int __out_param_h = {};
   Eina_Bool __return_value =  ::evas_obj_grid_pack_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
      & __out_param_x,
      & __out_param_y,
      & __out_param_w,
      & __out_param_h);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y, __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(w, __out_param_w);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(h, __out_param_h);
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Grid::pack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
   Eina_Bool __return_value =  ::evas_obj_grid_pack(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
} 
namespace eo_cxx {
namespace evas { 
inline ::efl::eolian::return_traits<void>::type Grid::grid_size_get( ::efl::eolian::out_traits<int>::type w,  ::efl::eolian::out_traits<int>::type h) const
{
   int __out_param_w = {};
   int __out_param_h = {};
    ::evas_obj_grid_size_get(_eo_ptr(),
      & __out_param_w,
      & __out_param_h);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(w, __out_param_w);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(h, __out_param_h);
}
inline ::efl::eolian::return_traits<void>::type Grid::grid_size_set( ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
    ::evas_obj_grid_size_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
}
inline ::efl::eolian::return_traits<  ::efl::eina::list< ::efl::canvas::Object>>::type Grid::children_get() const
{
   Eina_List * __return_value =  ::evas_obj_grid_children_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::list< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type Grid::accessor_new() const
{
   Eina_Accessor * __return_value =  ::evas_obj_grid_accessor_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Grid::clear( ::efl::eolian::in_traits<bool>::type clear) const
{
    ::evas_obj_grid_clear(_eo_ptr(),
       ::efl::eolian::convert_to_c<Eina_Bool,  ::efl::eolian::in_traits<bool>::type>(clear));
}
inline ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type Grid::iterator_new() const
{
   Eina_Iterator * __return_value =  ::evas_obj_grid_iterator_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Grid::add_to() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_grid_add_to(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Grid::unpack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Eina_Bool __return_value =  ::evas_obj_grid_unpack(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Grid::pack_get( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::out_traits<int>::type x,  ::efl::eolian::out_traits<int>::type y,  ::efl::eolian::out_traits<int>::type w,  ::efl::eolian::out_traits<int>::type h) const
{
   int __out_param_x = {};
   int __out_param_y = {};
   int __out_param_w = {};
   int __out_param_h = {};
   Eina_Bool __return_value =  ::evas_obj_grid_pack_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
      & __out_param_x,
      & __out_param_y,
      & __out_param_w,
      & __out_param_h);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x, __out_param_x);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y, __out_param_y);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(w, __out_param_w);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(h, __out_param_h);
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Grid::pack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
   Eina_Bool __return_value =  ::evas_obj_grid_pack(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline evas::Grid::operator ::evas::Grid() const { return *static_cast< ::evas::Grid const*>(static_cast<void const*>(this)); }
inline evas::Grid::operator ::evas::Grid&() { return *static_cast< ::evas::Grid*>(static_cast<void*>(this)); }
inline evas::Grid::operator ::evas::Grid const&() const { return *static_cast< ::evas::Grid const*>(static_cast<void const*>(this)); }
} }
#endif
