#ifndef EVAS_TABLE_IMPL_HH
#define EVAS_TABLE_IMPL_HH

#include "evas_table_eo.hh"

namespace evas { 
inline ::efl::eolian::return_traits<Evas_Object_Table_Homogeneous_Mode>::type Table::homogeneous_get() const
{
   Evas_Object_Table_Homogeneous_Mode __return_value =  ::evas_obj_table_homogeneous_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Table_Homogeneous_Mode>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Table::homogeneous_set( ::efl::eolian::in_traits<Evas_Object_Table_Homogeneous_Mode>::type homogeneous) const
{
    ::evas_obj_table_homogeneous_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Table_Homogeneous_Mode,  ::efl::eolian::in_traits<Evas_Object_Table_Homogeneous_Mode>::type>(homogeneous));
}
inline ::efl::eolian::return_traits<void>::type Table::align_get( ::efl::eolian::out_traits<double>::type horizontal,  ::efl::eolian::out_traits<double>::type vertical) const
{
   double __out_param_horizontal = {};
   double __out_param_vertical = {};
    ::evas_obj_table_align_get(_eo_ptr(),
      & __out_param_horizontal,
      & __out_param_vertical);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(horizontal, __out_param_horizontal);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(vertical, __out_param_vertical);
}
inline ::efl::eolian::return_traits<void>::type Table::align_set( ::efl::eolian::in_traits<double>::type horizontal,  ::efl::eolian::in_traits<double>::type vertical) const
{
    ::evas_obj_table_align_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<double,  ::efl::eolian::in_traits<double>::type>(horizontal),
       ::efl::eolian::convert_to_c<double,  ::efl::eolian::in_traits<double>::type>(vertical));
}
inline ::efl::eolian::return_traits<void>::type Table::padding_get( ::efl::eolian::out_traits<int>::type horizontal,  ::efl::eolian::out_traits<int>::type vertical) const
{
   int __out_param_horizontal = {};
   int __out_param_vertical = {};
    ::evas_obj_table_padding_get(_eo_ptr(),
      & __out_param_horizontal,
      & __out_param_vertical);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(horizontal, __out_param_horizontal);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(vertical, __out_param_vertical);
}
inline ::efl::eolian::return_traits<void>::type Table::padding_set( ::efl::eolian::in_traits<int>::type horizontal,  ::efl::eolian::in_traits<int>::type vertical) const
{
    ::evas_obj_table_padding_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(horizontal),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(vertical));
}
inline ::efl::eolian::return_traits<void>::type Table::col_row_size_get( ::efl::eolian::out_traits<int>::type cols,  ::efl::eolian::out_traits<int>::type rows) const
{
   int __out_param_cols = {};
   int __out_param_rows = {};
    ::evas_obj_table_col_row_size_get(_eo_ptr(),
      & __out_param_cols,
      & __out_param_rows);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cols, __out_param_cols);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(rows, __out_param_rows);
}
inline ::efl::eolian::return_traits<  ::efl::eina::list< ::efl::canvas::Object>>::type Table::children_get() const
{
   Eina_List * __return_value =  ::evas_obj_table_children_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::list< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Table::child_get( ::efl::eolian::in_traits< unsigned short>::type col,  ::efl::eolian::in_traits< unsigned short>::type row) const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_table_child_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(col),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(row));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Table::clear( ::efl::eolian::in_traits<bool>::type clear) const
{
    ::evas_obj_table_clear(_eo_ptr(),
       ::efl::eolian::convert_to_c<Eina_Bool,  ::efl::eolian::in_traits<bool>::type>(clear));
}
inline ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type Table::accessor_new() const
{
   Eina_Accessor * __return_value =  ::evas_obj_table_accessor_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type Table::iterator_new() const
{
   Eina_Iterator * __return_value =  ::evas_obj_table_iterator_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Table::add_to() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_table_add_to(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Table::pack_get( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::out_traits< unsigned short>::type col,  ::efl::eolian::out_traits< unsigned short>::type row,  ::efl::eolian::out_traits< unsigned short>::type colspan,  ::efl::eolian::out_traits< unsigned short>::type rowspan) const
{
   unsigned short __out_param_col = {};
   unsigned short __out_param_row = {};
   unsigned short __out_param_colspan = {};
   unsigned short __out_param_rowspan = {};
   Eina_Bool __return_value =  ::evas_obj_table_pack_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
      & __out_param_col,
      & __out_param_row,
      & __out_param_colspan,
      & __out_param_rowspan);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits< unsigned short>::type, unsigned short>(col, __out_param_col);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits< unsigned short>::type, unsigned short>(row, __out_param_row);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits< unsigned short>::type, unsigned short>(colspan, __out_param_colspan);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits< unsigned short>::type, unsigned short>(rowspan, __out_param_rowspan);
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Table::pack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned short>::type col,  ::efl::eolian::in_traits< unsigned short>::type row,  ::efl::eolian::in_traits< unsigned short>::type colspan,  ::efl::eolian::in_traits< unsigned short>::type rowspan) const
{
   Eina_Bool __return_value =  ::evas_obj_table_pack(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(col),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(row),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(colspan),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(rowspan));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Table::unpack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Eina_Bool __return_value =  ::evas_obj_table_unpack(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Table::count() const
{
   int __return_value =  ::evas_obj_table_count(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
} 
namespace eo_cxx {
namespace evas { 
inline ::efl::eolian::return_traits<Evas_Object_Table_Homogeneous_Mode>::type Table::homogeneous_get() const
{
   Evas_Object_Table_Homogeneous_Mode __return_value =  ::evas_obj_table_homogeneous_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Object_Table_Homogeneous_Mode>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Table::homogeneous_set( ::efl::eolian::in_traits<Evas_Object_Table_Homogeneous_Mode>::type homogeneous) const
{
    ::evas_obj_table_homogeneous_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Object_Table_Homogeneous_Mode,  ::efl::eolian::in_traits<Evas_Object_Table_Homogeneous_Mode>::type>(homogeneous));
}
inline ::efl::eolian::return_traits<void>::type Table::align_get( ::efl::eolian::out_traits<double>::type horizontal,  ::efl::eolian::out_traits<double>::type vertical) const
{
   double __out_param_horizontal = {};
   double __out_param_vertical = {};
    ::evas_obj_table_align_get(_eo_ptr(),
      & __out_param_horizontal,
      & __out_param_vertical);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(horizontal, __out_param_horizontal);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<double>::type, double>(vertical, __out_param_vertical);
}
inline ::efl::eolian::return_traits<void>::type Table::align_set( ::efl::eolian::in_traits<double>::type horizontal,  ::efl::eolian::in_traits<double>::type vertical) const
{
    ::evas_obj_table_align_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<double,  ::efl::eolian::in_traits<double>::type>(horizontal),
       ::efl::eolian::convert_to_c<double,  ::efl::eolian::in_traits<double>::type>(vertical));
}
inline ::efl::eolian::return_traits<void>::type Table::padding_get( ::efl::eolian::out_traits<int>::type horizontal,  ::efl::eolian::out_traits<int>::type vertical) const
{
   int __out_param_horizontal = {};
   int __out_param_vertical = {};
    ::evas_obj_table_padding_get(_eo_ptr(),
      & __out_param_horizontal,
      & __out_param_vertical);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(horizontal, __out_param_horizontal);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(vertical, __out_param_vertical);
}
inline ::efl::eolian::return_traits<void>::type Table::padding_set( ::efl::eolian::in_traits<int>::type horizontal,  ::efl::eolian::in_traits<int>::type vertical) const
{
    ::evas_obj_table_padding_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(horizontal),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(vertical));
}
inline ::efl::eolian::return_traits<void>::type Table::col_row_size_get( ::efl::eolian::out_traits<int>::type cols,  ::efl::eolian::out_traits<int>::type rows) const
{
   int __out_param_cols = {};
   int __out_param_rows = {};
    ::evas_obj_table_col_row_size_get(_eo_ptr(),
      & __out_param_cols,
      & __out_param_rows);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cols, __out_param_cols);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(rows, __out_param_rows);
}
inline ::efl::eolian::return_traits<  ::efl::eina::list< ::efl::canvas::Object>>::type Table::children_get() const
{
   Eina_List * __return_value =  ::evas_obj_table_children_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<  ::efl::eina::list< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Table::child_get( ::efl::eolian::in_traits< unsigned short>::type col,  ::efl::eolian::in_traits< unsigned short>::type row) const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_table_child_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(col),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(row));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Table::clear( ::efl::eolian::in_traits<bool>::type clear) const
{
    ::evas_obj_table_clear(_eo_ptr(),
       ::efl::eolian::convert_to_c<Eina_Bool,  ::efl::eolian::in_traits<bool>::type>(clear));
}
inline ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type Table::accessor_new() const
{
   Eina_Accessor * __return_value =  ::evas_obj_table_accessor_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::accessor< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type Table::iterator_new() const
{
   Eina_Iterator * __return_value =  ::evas_obj_table_iterator_new(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::iterator< ::efl::canvas::Object>>::type>(__return_value);
}
inline ::efl::eolian::return_traits< ::efl::canvas::Object>::type Table::add_to() const
{
    ::Efl_Canvas_Object* __return_value =  ::evas_obj_table_add_to(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::canvas::Object>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Table::pack_get( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::out_traits< unsigned short>::type col,  ::efl::eolian::out_traits< unsigned short>::type row,  ::efl::eolian::out_traits< unsigned short>::type colspan,  ::efl::eolian::out_traits< unsigned short>::type rowspan) const
{
   unsigned short __out_param_col = {};
   unsigned short __out_param_row = {};
   unsigned short __out_param_colspan = {};
   unsigned short __out_param_rowspan = {};
   Eina_Bool __return_value =  ::evas_obj_table_pack_get(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
      & __out_param_col,
      & __out_param_row,
      & __out_param_colspan,
      & __out_param_rowspan);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits< unsigned short>::type, unsigned short>(col, __out_param_col);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits< unsigned short>::type, unsigned short>(row, __out_param_row);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits< unsigned short>::type, unsigned short>(colspan, __out_param_colspan);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits< unsigned short>::type, unsigned short>(rowspan, __out_param_rowspan);
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Table::pack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child,  ::efl::eolian::in_traits< unsigned short>::type col,  ::efl::eolian::in_traits< unsigned short>::type row,  ::efl::eolian::in_traits< unsigned short>::type colspan,  ::efl::eolian::in_traits< unsigned short>::type rowspan) const
{
   Eina_Bool __return_value =  ::evas_obj_table_pack(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(col),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(row),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(colspan),
       ::efl::eolian::convert_to_c<unsigned short,  ::efl::eolian::in_traits< unsigned short>::type>(rowspan));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Table::unpack( ::efl::eolian::in_traits< ::efl::canvas::Object>::type child) const
{
   Eina_Bool __return_value =  ::evas_obj_table_unpack(_eo_ptr(),
       ::efl::eolian::convert_to_c< ::Efl_Canvas_Object*,  ::efl::eolian::in_traits< ::efl::canvas::Object>::type>(child));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Table::count() const
{
   int __return_value =  ::evas_obj_table_count(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline evas::Table::operator ::evas::Table() const { return *static_cast< ::evas::Table const*>(static_cast<void const*>(this)); }
inline evas::Table::operator ::evas::Table&() { return *static_cast< ::evas::Table*>(static_cast<void*>(this)); }
inline evas::Table::operator ::evas::Table const&() const { return *static_cast< ::evas::Table const*>(static_cast<void const*>(this)); }
} }
#endif
