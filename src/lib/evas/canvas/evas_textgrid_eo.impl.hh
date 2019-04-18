#ifndef EVAS_TEXTGRID_IMPL_HH
#define EVAS_TEXTGRID_IMPL_HH

#include "evas_textgrid_eo.hh"

namespace evas { 
inline ::efl::eolian::return_traits<Evas_Textgrid_Font_Style>::type Textgrid::supported_font_styles_get() const
{
   Evas_Textgrid_Font_Style __return_value =  ::evas_obj_textgrid_supported_font_styles_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Textgrid_Font_Style>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Textgrid::supported_font_styles_set( ::efl::eolian::in_traits<Evas_Textgrid_Font_Style>::type styles) const
{
    ::evas_obj_textgrid_supported_font_styles_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Textgrid_Font_Style,  ::efl::eolian::in_traits<Evas_Textgrid_Font_Style>::type>(styles));
}
inline ::efl::eolian::return_traits<void>::type Textgrid::grid_size_get( ::efl::eolian::out_traits<int>::type w,  ::efl::eolian::out_traits<int>::type h) const
{
   int __out_param_w = {};
   int __out_param_h = {};
    ::evas_obj_textgrid_grid_size_get(_eo_ptr(),
      & __out_param_w,
      & __out_param_h);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(w, __out_param_w);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(h, __out_param_h);
}
inline ::efl::eolian::return_traits<void>::type Textgrid::grid_size_set( ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
    ::evas_obj_textgrid_grid_size_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
}
inline ::efl::eolian::return_traits<void>::type Textgrid::cell_size_get( ::efl::eolian::out_traits<int>::type width,  ::efl::eolian::out_traits<int>::type height) const
{
   int __out_param_width = {};
   int __out_param_height = {};
    ::evas_obj_textgrid_cell_size_get(_eo_ptr(),
      & __out_param_width,
      & __out_param_height);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(width, __out_param_width);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(height, __out_param_height);
}
inline ::efl::eolian::return_traits<void>::type Textgrid::update_add( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
    ::evas_obj_textgrid_update_add(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
}
inline ::efl::eolian::return_traits<void>::type Textgrid::cellrow_set( ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<Evas_Textgrid_Cell const&>::type row) const
{
    ::evas_obj_textgrid_cellrow_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
       ::efl::eolian::convert_to_c<const Evas_Textgrid_Cell *,  ::efl::eolian::in_traits<Evas_Textgrid_Cell const&>::type>(row));
}
inline ::efl::eolian::return_traits<Evas_Textgrid_Cell&>::type Textgrid::cellrow_get( ::efl::eolian::in_traits<int>::type y) const
{
   Evas_Textgrid_Cell * __return_value =  ::evas_obj_textgrid_cellrow_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Textgrid_Cell&>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Textgrid::palette_set( ::efl::eolian::in_traits<Evas_Textgrid_Palette>::type pal,  ::efl::eolian::in_traits<int>::type idx,  ::efl::eolian::in_traits<int>::type r,  ::efl::eolian::in_traits<int>::type g,  ::efl::eolian::in_traits<int>::type b,  ::efl::eolian::in_traits<int>::type a) const
{
    ::evas_obj_textgrid_palette_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Textgrid_Palette,  ::efl::eolian::in_traits<Evas_Textgrid_Palette>::type>(pal),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(idx),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(r),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(g),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(b),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(a));
}
inline ::efl::eolian::return_traits<void>::type Textgrid::palette_get( ::efl::eolian::in_traits<Evas_Textgrid_Palette>::type pal,  ::efl::eolian::in_traits<int>::type idx,  ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type g,  ::efl::eolian::out_traits<int>::type b,  ::efl::eolian::out_traits<int>::type a) const
{
   int __out_param_r = {};
   int __out_param_g = {};
   int __out_param_b = {};
   int __out_param_a = {};
    ::evas_obj_textgrid_palette_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Textgrid_Palette,  ::efl::eolian::in_traits<Evas_Textgrid_Palette>::type>(pal),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(idx),
      & __out_param_r,
      & __out_param_g,
      & __out_param_b,
      & __out_param_a);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(g, __out_param_g);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(a, __out_param_a);
}
} 
namespace eo_cxx {
namespace evas { 
inline ::efl::eolian::return_traits<Evas_Textgrid_Font_Style>::type Textgrid::supported_font_styles_get() const
{
   Evas_Textgrid_Font_Style __return_value =  ::evas_obj_textgrid_supported_font_styles_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Textgrid_Font_Style>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Textgrid::supported_font_styles_set( ::efl::eolian::in_traits<Evas_Textgrid_Font_Style>::type styles) const
{
    ::evas_obj_textgrid_supported_font_styles_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Textgrid_Font_Style,  ::efl::eolian::in_traits<Evas_Textgrid_Font_Style>::type>(styles));
}
inline ::efl::eolian::return_traits<void>::type Textgrid::grid_size_get( ::efl::eolian::out_traits<int>::type w,  ::efl::eolian::out_traits<int>::type h) const
{
   int __out_param_w = {};
   int __out_param_h = {};
    ::evas_obj_textgrid_grid_size_get(_eo_ptr(),
      & __out_param_w,
      & __out_param_h);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(w, __out_param_w);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(h, __out_param_h);
}
inline ::efl::eolian::return_traits<void>::type Textgrid::grid_size_set( ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
    ::evas_obj_textgrid_grid_size_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
}
inline ::efl::eolian::return_traits<void>::type Textgrid::cell_size_get( ::efl::eolian::out_traits<int>::type width,  ::efl::eolian::out_traits<int>::type height) const
{
   int __out_param_width = {};
   int __out_param_height = {};
    ::evas_obj_textgrid_cell_size_get(_eo_ptr(),
      & __out_param_width,
      & __out_param_height);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(width, __out_param_width);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(height, __out_param_height);
}
inline ::efl::eolian::return_traits<void>::type Textgrid::update_add( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<int>::type w,  ::efl::eolian::in_traits<int>::type h) const
{
    ::evas_obj_textgrid_update_add(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(w),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(h));
}
inline ::efl::eolian::return_traits<void>::type Textgrid::cellrow_set( ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::in_traits<Evas_Textgrid_Cell const&>::type row) const
{
    ::evas_obj_textgrid_cellrow_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
       ::efl::eolian::convert_to_c<const Evas_Textgrid_Cell *,  ::efl::eolian::in_traits<Evas_Textgrid_Cell const&>::type>(row));
}
inline ::efl::eolian::return_traits<Evas_Textgrid_Cell&>::type Textgrid::cellrow_get( ::efl::eolian::in_traits<int>::type y) const
{
   Evas_Textgrid_Cell * __return_value =  ::evas_obj_textgrid_cellrow_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Textgrid_Cell&>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Textgrid::palette_set( ::efl::eolian::in_traits<Evas_Textgrid_Palette>::type pal,  ::efl::eolian::in_traits<int>::type idx,  ::efl::eolian::in_traits<int>::type r,  ::efl::eolian::in_traits<int>::type g,  ::efl::eolian::in_traits<int>::type b,  ::efl::eolian::in_traits<int>::type a) const
{
    ::evas_obj_textgrid_palette_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Textgrid_Palette,  ::efl::eolian::in_traits<Evas_Textgrid_Palette>::type>(pal),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(idx),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(r),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(g),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(b),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(a));
}
inline ::efl::eolian::return_traits<void>::type Textgrid::palette_get( ::efl::eolian::in_traits<Evas_Textgrid_Palette>::type pal,  ::efl::eolian::in_traits<int>::type idx,  ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type g,  ::efl::eolian::out_traits<int>::type b,  ::efl::eolian::out_traits<int>::type a) const
{
   int __out_param_r = {};
   int __out_param_g = {};
   int __out_param_b = {};
   int __out_param_a = {};
    ::evas_obj_textgrid_palette_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Textgrid_Palette,  ::efl::eolian::in_traits<Evas_Textgrid_Palette>::type>(pal),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(idx),
      & __out_param_r,
      & __out_param_g,
      & __out_param_b,
      & __out_param_a);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(g, __out_param_g);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(a, __out_param_a);
}
inline evas::Textgrid::operator ::evas::Textgrid() const { return *static_cast< ::evas::Textgrid const*>(static_cast<void const*>(this)); }
inline evas::Textgrid::operator ::evas::Textgrid&() { return *static_cast< ::evas::Textgrid*>(static_cast<void*>(this)); }
inline evas::Textgrid::operator ::evas::Textgrid const&() const { return *static_cast< ::evas::Textgrid const*>(static_cast<void const*>(this)); }
} }
#endif
