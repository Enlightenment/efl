#ifndef EVAS_TEXT_IMPL_HH
#define EVAS_TEXT_IMPL_HH

#include "evas_text_eo.hh"

namespace evas { 
inline ::efl::eolian::return_traits<void>::type Text::shadow_color_get( ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type g,  ::efl::eolian::out_traits<int>::type b,  ::efl::eolian::out_traits<int>::type a) const
{
   int __out_param_r = {};
   int __out_param_g = {};
   int __out_param_b = {};
   int __out_param_a = {};
    ::evas_obj_text_shadow_color_get(_eo_ptr(),
      & __out_param_r,
      & __out_param_g,
      & __out_param_b,
      & __out_param_a);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(g, __out_param_g);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(a, __out_param_a);
}
inline ::efl::eolian::return_traits<void>::type Text::shadow_color_set( ::efl::eolian::in_traits<int>::type r,  ::efl::eolian::in_traits<int>::type g,  ::efl::eolian::in_traits<int>::type b,  ::efl::eolian::in_traits<int>::type a) const
{
    ::evas_obj_text_shadow_color_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(r),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(g),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(b),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(a));
}
inline ::efl::eolian::return_traits<double>::type Text::ellipsis_get() const
{
   double __return_value =  ::evas_obj_text_ellipsis_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<double>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Text::ellipsis_set( ::efl::eolian::in_traits<double>::type ellipsis) const
{
    ::evas_obj_text_ellipsis_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<double,  ::efl::eolian::in_traits<double>::type>(ellipsis));
}
inline ::efl::eolian::return_traits< ::efl::eina::string_view>::type Text::bidi_delimiters_get() const
{
   const char * __return_value =  ::evas_obj_text_bidi_delimiters_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::string_view>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Text::bidi_delimiters_set( ::efl::eolian::in_traits< ::efl::eina::string_view>::type delim) const
{
    ::evas_obj_text_bidi_delimiters_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(delim));
}
inline ::efl::eolian::return_traits<void>::type Text::outline_color_get( ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type g,  ::efl::eolian::out_traits<int>::type b,  ::efl::eolian::out_traits<int>::type a) const
{
   int __out_param_r = {};
   int __out_param_g = {};
   int __out_param_b = {};
   int __out_param_a = {};
    ::evas_obj_text_outline_color_get(_eo_ptr(),
      & __out_param_r,
      & __out_param_g,
      & __out_param_b,
      & __out_param_a);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(g, __out_param_g);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(a, __out_param_a);
}
inline ::efl::eolian::return_traits<void>::type Text::outline_color_set( ::efl::eolian::in_traits<int>::type r,  ::efl::eolian::in_traits<int>::type g,  ::efl::eolian::in_traits<int>::type b,  ::efl::eolian::in_traits<int>::type a) const
{
    ::evas_obj_text_outline_color_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(r),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(g),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(b),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(a));
}
inline ::efl::eolian::return_traits<void>::type Text::glow2_color_get( ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type g,  ::efl::eolian::out_traits<int>::type b,  ::efl::eolian::out_traits<int>::type a) const
{
   int __out_param_r = {};
   int __out_param_g = {};
   int __out_param_b = {};
   int __out_param_a = {};
    ::evas_obj_text_glow2_color_get(_eo_ptr(),
      & __out_param_r,
      & __out_param_g,
      & __out_param_b,
      & __out_param_a);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(g, __out_param_g);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(a, __out_param_a);
}
inline ::efl::eolian::return_traits<void>::type Text::glow2_color_set( ::efl::eolian::in_traits<int>::type r,  ::efl::eolian::in_traits<int>::type g,  ::efl::eolian::in_traits<int>::type b,  ::efl::eolian::in_traits<int>::type a) const
{
    ::evas_obj_text_glow2_color_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(r),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(g),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(b),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(a));
}
inline ::efl::eolian::return_traits<Evas_Text_Style_Type>::type Text::style_get() const
{
   Evas_Text_Style_Type __return_value =  ::evas_obj_text_style_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Text_Style_Type>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Text::style_set( ::efl::eolian::in_traits<Evas_Text_Style_Type>::type style) const
{
    ::evas_obj_text_style_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Text_Style_Type,  ::efl::eolian::in_traits<Evas_Text_Style_Type>::type>(style));
}
inline ::efl::eolian::return_traits<void>::type Text::glow_color_get( ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type g,  ::efl::eolian::out_traits<int>::type b,  ::efl::eolian::out_traits<int>::type a) const
{
   int __out_param_r = {};
   int __out_param_g = {};
   int __out_param_b = {};
   int __out_param_a = {};
    ::evas_obj_text_glow_color_get(_eo_ptr(),
      & __out_param_r,
      & __out_param_g,
      & __out_param_b,
      & __out_param_a);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(g, __out_param_g);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(a, __out_param_a);
}
inline ::efl::eolian::return_traits<void>::type Text::glow_color_set( ::efl::eolian::in_traits<int>::type r,  ::efl::eolian::in_traits<int>::type g,  ::efl::eolian::in_traits<int>::type b,  ::efl::eolian::in_traits<int>::type a) const
{
    ::evas_obj_text_glow_color_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(r),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(g),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(b),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(a));
}
inline ::efl::eolian::return_traits<int>::type Text::max_descent_get() const
{
   int __return_value =  ::evas_obj_text_max_descent_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Text::style_pad_get( ::efl::eolian::out_traits<int>::type l,  ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type t,  ::efl::eolian::out_traits<int>::type b) const
{
   int __out_param_l = {};
   int __out_param_r = {};
   int __out_param_t = {};
   int __out_param_b = {};
    ::evas_obj_text_style_pad_get(_eo_ptr(),
      & __out_param_l,
      & __out_param_r,
      & __out_param_t,
      & __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(l, __out_param_l);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(t, __out_param_t);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
}
inline ::efl::eolian::return_traits<Efl_Text_Bidirectional_Type>::type Text::direction_get() const
{
   Efl_Text_Bidirectional_Type __return_value =  ::evas_obj_text_direction_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Efl_Text_Bidirectional_Type>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::ascent_get() const
{
   int __return_value =  ::evas_obj_text_ascent_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::horiz_advance_get() const
{
   int __return_value =  ::evas_obj_text_horiz_advance_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::inset_get() const
{
   int __return_value =  ::evas_obj_text_inset_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::max_ascent_get() const
{
   int __return_value =  ::evas_obj_text_max_ascent_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::vert_advance_get() const
{
   int __return_value =  ::evas_obj_text_vert_advance_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::descent_get() const
{
   int __return_value =  ::evas_obj_text_descent_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::last_up_to_pos( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y) const
{
   int __return_value =  ::evas_obj_text_last_up_to_pos(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::char_coords_get( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::out_traits<int>::type cx,  ::efl::eolian::out_traits<int>::type cy,  ::efl::eolian::out_traits<int>::type cw,  ::efl::eolian::out_traits<int>::type ch) const
{
   int __out_param_cx = {};
   int __out_param_cy = {};
   int __out_param_cw = {};
   int __out_param_ch = {};
   int __return_value =  ::evas_obj_text_char_coords_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
      & __out_param_cx,
      & __out_param_cy,
      & __out_param_cw,
      & __out_param_ch);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cx, __out_param_cx);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cy, __out_param_cy);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cw, __out_param_cw);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(ch, __out_param_ch);
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Text::char_pos_get( ::efl::eolian::in_traits<int>::type pos,  ::efl::eolian::out_traits<int>::type cx,  ::efl::eolian::out_traits<int>::type cy,  ::efl::eolian::out_traits<int>::type cw,  ::efl::eolian::out_traits<int>::type ch) const
{
   int __out_param_cx = {};
   int __out_param_cy = {};
   int __out_param_cw = {};
   int __out_param_ch = {};
   Eina_Bool __return_value =  ::evas_obj_text_char_pos_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(pos),
      & __out_param_cx,
      & __out_param_cy,
      & __out_param_cw,
      & __out_param_ch);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cx, __out_param_cx);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cy, __out_param_cy);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cw, __out_param_cw);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(ch, __out_param_ch);
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
} 
namespace eo_cxx {
namespace evas { 
inline ::efl::eolian::return_traits<void>::type Text::shadow_color_get( ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type g,  ::efl::eolian::out_traits<int>::type b,  ::efl::eolian::out_traits<int>::type a) const
{
   int __out_param_r = {};
   int __out_param_g = {};
   int __out_param_b = {};
   int __out_param_a = {};
    ::evas_obj_text_shadow_color_get(_eo_ptr(),
      & __out_param_r,
      & __out_param_g,
      & __out_param_b,
      & __out_param_a);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(g, __out_param_g);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(a, __out_param_a);
}
inline ::efl::eolian::return_traits<void>::type Text::shadow_color_set( ::efl::eolian::in_traits<int>::type r,  ::efl::eolian::in_traits<int>::type g,  ::efl::eolian::in_traits<int>::type b,  ::efl::eolian::in_traits<int>::type a) const
{
    ::evas_obj_text_shadow_color_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(r),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(g),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(b),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(a));
}
inline ::efl::eolian::return_traits<double>::type Text::ellipsis_get() const
{
   double __return_value =  ::evas_obj_text_ellipsis_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<double>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Text::ellipsis_set( ::efl::eolian::in_traits<double>::type ellipsis) const
{
    ::evas_obj_text_ellipsis_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<double,  ::efl::eolian::in_traits<double>::type>(ellipsis));
}
inline ::efl::eolian::return_traits< ::efl::eina::string_view>::type Text::bidi_delimiters_get() const
{
   const char * __return_value =  ::evas_obj_text_bidi_delimiters_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits< ::efl::eina::string_view>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Text::bidi_delimiters_set( ::efl::eolian::in_traits< ::efl::eina::string_view>::type delim) const
{
    ::evas_obj_text_bidi_delimiters_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<const char *,  ::efl::eolian::in_traits< ::efl::eina::string_view>::type>(delim));
}
inline ::efl::eolian::return_traits<void>::type Text::outline_color_get( ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type g,  ::efl::eolian::out_traits<int>::type b,  ::efl::eolian::out_traits<int>::type a) const
{
   int __out_param_r = {};
   int __out_param_g = {};
   int __out_param_b = {};
   int __out_param_a = {};
    ::evas_obj_text_outline_color_get(_eo_ptr(),
      & __out_param_r,
      & __out_param_g,
      & __out_param_b,
      & __out_param_a);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(g, __out_param_g);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(a, __out_param_a);
}
inline ::efl::eolian::return_traits<void>::type Text::outline_color_set( ::efl::eolian::in_traits<int>::type r,  ::efl::eolian::in_traits<int>::type g,  ::efl::eolian::in_traits<int>::type b,  ::efl::eolian::in_traits<int>::type a) const
{
    ::evas_obj_text_outline_color_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(r),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(g),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(b),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(a));
}
inline ::efl::eolian::return_traits<void>::type Text::glow2_color_get( ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type g,  ::efl::eolian::out_traits<int>::type b,  ::efl::eolian::out_traits<int>::type a) const
{
   int __out_param_r = {};
   int __out_param_g = {};
   int __out_param_b = {};
   int __out_param_a = {};
    ::evas_obj_text_glow2_color_get(_eo_ptr(),
      & __out_param_r,
      & __out_param_g,
      & __out_param_b,
      & __out_param_a);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(g, __out_param_g);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(a, __out_param_a);
}
inline ::efl::eolian::return_traits<void>::type Text::glow2_color_set( ::efl::eolian::in_traits<int>::type r,  ::efl::eolian::in_traits<int>::type g,  ::efl::eolian::in_traits<int>::type b,  ::efl::eolian::in_traits<int>::type a) const
{
    ::evas_obj_text_glow2_color_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(r),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(g),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(b),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(a));
}
inline ::efl::eolian::return_traits<Evas_Text_Style_Type>::type Text::style_get() const
{
   Evas_Text_Style_Type __return_value =  ::evas_obj_text_style_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Evas_Text_Style_Type>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Text::style_set( ::efl::eolian::in_traits<Evas_Text_Style_Type>::type style) const
{
    ::evas_obj_text_style_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<Evas_Text_Style_Type,  ::efl::eolian::in_traits<Evas_Text_Style_Type>::type>(style));
}
inline ::efl::eolian::return_traits<void>::type Text::glow_color_get( ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type g,  ::efl::eolian::out_traits<int>::type b,  ::efl::eolian::out_traits<int>::type a) const
{
   int __out_param_r = {};
   int __out_param_g = {};
   int __out_param_b = {};
   int __out_param_a = {};
    ::evas_obj_text_glow_color_get(_eo_ptr(),
      & __out_param_r,
      & __out_param_g,
      & __out_param_b,
      & __out_param_a);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(g, __out_param_g);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(a, __out_param_a);
}
inline ::efl::eolian::return_traits<void>::type Text::glow_color_set( ::efl::eolian::in_traits<int>::type r,  ::efl::eolian::in_traits<int>::type g,  ::efl::eolian::in_traits<int>::type b,  ::efl::eolian::in_traits<int>::type a) const
{
    ::evas_obj_text_glow_color_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(r),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(g),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(b),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(a));
}
inline ::efl::eolian::return_traits<int>::type Text::max_descent_get() const
{
   int __return_value =  ::evas_obj_text_max_descent_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<void>::type Text::style_pad_get( ::efl::eolian::out_traits<int>::type l,  ::efl::eolian::out_traits<int>::type r,  ::efl::eolian::out_traits<int>::type t,  ::efl::eolian::out_traits<int>::type b) const
{
   int __out_param_l = {};
   int __out_param_r = {};
   int __out_param_t = {};
   int __out_param_b = {};
    ::evas_obj_text_style_pad_get(_eo_ptr(),
      & __out_param_l,
      & __out_param_r,
      & __out_param_t,
      & __out_param_b);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(l, __out_param_l);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(r, __out_param_r);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(t, __out_param_t);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(b, __out_param_b);
}
inline ::efl::eolian::return_traits<Efl_Text_Bidirectional_Type>::type Text::direction_get() const
{
   Efl_Text_Bidirectional_Type __return_value =  ::evas_obj_text_direction_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<Efl_Text_Bidirectional_Type>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::ascent_get() const
{
   int __return_value =  ::evas_obj_text_ascent_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::horiz_advance_get() const
{
   int __return_value =  ::evas_obj_text_horiz_advance_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::inset_get() const
{
   int __return_value =  ::evas_obj_text_inset_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::max_ascent_get() const
{
   int __return_value =  ::evas_obj_text_max_ascent_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::vert_advance_get() const
{
   int __return_value =  ::evas_obj_text_vert_advance_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::descent_get() const
{
   int __return_value =  ::evas_obj_text_descent_get(_eo_ptr());
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::last_up_to_pos( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y) const
{
   int __return_value =  ::evas_obj_text_last_up_to_pos(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y));
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<int>::type Text::char_coords_get( ::efl::eolian::in_traits<int>::type x,  ::efl::eolian::in_traits<int>::type y,  ::efl::eolian::out_traits<int>::type cx,  ::efl::eolian::out_traits<int>::type cy,  ::efl::eolian::out_traits<int>::type cw,  ::efl::eolian::out_traits<int>::type ch) const
{
   int __out_param_cx = {};
   int __out_param_cy = {};
   int __out_param_cw = {};
   int __out_param_ch = {};
   int __return_value =  ::evas_obj_text_char_coords_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y),
      & __out_param_cx,
      & __out_param_cy,
      & __out_param_cw,
      & __out_param_ch);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cx, __out_param_cx);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cy, __out_param_cy);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cw, __out_param_cw);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(ch, __out_param_ch);
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<int>::type>(__return_value);
}
inline ::efl::eolian::return_traits<bool>::type Text::char_pos_get( ::efl::eolian::in_traits<int>::type pos,  ::efl::eolian::out_traits<int>::type cx,  ::efl::eolian::out_traits<int>::type cy,  ::efl::eolian::out_traits<int>::type cw,  ::efl::eolian::out_traits<int>::type ch) const
{
   int __out_param_cx = {};
   int __out_param_cy = {};
   int __out_param_cw = {};
   int __out_param_ch = {};
   Eina_Bool __return_value =  ::evas_obj_text_char_pos_get(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(pos),
      & __out_param_cx,
      & __out_param_cy,
      & __out_param_cw,
      & __out_param_ch);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cx, __out_param_cx);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cy, __out_param_cy);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(cw, __out_param_cw);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(ch, __out_param_ch);
   return ::efl::eolian::convert_to_return< ::efl::eolian::return_traits<bool>::type>(__return_value);
}
inline evas::Text::operator ::evas::Text() const { return *static_cast< ::evas::Text const*>(static_cast<void const*>(this)); }
inline evas::Text::operator ::evas::Text&() { return *static_cast< ::evas::Text*>(static_cast<void*>(this)); }
inline evas::Text::operator ::evas::Text const&() const { return *static_cast< ::evas::Text const*>(static_cast<void const*>(this)); }
} }
#endif
