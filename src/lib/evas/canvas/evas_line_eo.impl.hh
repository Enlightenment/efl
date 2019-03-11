#ifndef EVAS_LINE_IMPL_HH
#define EVAS_LINE_IMPL_HH

#include "evas_line_eo.hh"

namespace evas { 
inline ::efl::eolian::return_traits<void>::type Line::xy_get( ::efl::eolian::out_traits<int>::type x1,  ::efl::eolian::out_traits<int>::type y1,  ::efl::eolian::out_traits<int>::type x2,  ::efl::eolian::out_traits<int>::type y2) const
{
   int __out_param_x1 = {};
   int __out_param_y1 = {};
   int __out_param_x2 = {};
   int __out_param_y2 = {};
    ::evas_obj_line_xy_get(_eo_ptr(),
      & __out_param_x1,
      & __out_param_y1,
      & __out_param_x2,
      & __out_param_y2);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x1, __out_param_x1);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y1, __out_param_y1);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x2, __out_param_x2);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y2, __out_param_y2);
}
inline ::efl::eolian::return_traits<void>::type Line::xy_set( ::efl::eolian::in_traits<int>::type x1,  ::efl::eolian::in_traits<int>::type y1,  ::efl::eolian::in_traits<int>::type x2,  ::efl::eolian::in_traits<int>::type y2) const
{
    ::evas_obj_line_xy_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x1),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y1),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x2),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y2));
}
} 
namespace eo_cxx {
namespace evas { 
inline ::efl::eolian::return_traits<void>::type Line::xy_get( ::efl::eolian::out_traits<int>::type x1,  ::efl::eolian::out_traits<int>::type y1,  ::efl::eolian::out_traits<int>::type x2,  ::efl::eolian::out_traits<int>::type y2) const
{
   int __out_param_x1 = {};
   int __out_param_y1 = {};
   int __out_param_x2 = {};
   int __out_param_y2 = {};
    ::evas_obj_line_xy_get(_eo_ptr(),
      & __out_param_x1,
      & __out_param_y1,
      & __out_param_x2,
      & __out_param_y2);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x1, __out_param_x1);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y1, __out_param_y1);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(x2, __out_param_x2);
   ::efl::eolian::assign_out< ::efl::eolian::out_traits<int>::type, int>(y2, __out_param_y2);
}
inline ::efl::eolian::return_traits<void>::type Line::xy_set( ::efl::eolian::in_traits<int>::type x1,  ::efl::eolian::in_traits<int>::type y1,  ::efl::eolian::in_traits<int>::type x2,  ::efl::eolian::in_traits<int>::type y2) const
{
    ::evas_obj_line_xy_set(_eo_ptr(),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x1),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y1),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(x2),
       ::efl::eolian::convert_to_c<int,  ::efl::eolian::in_traits<int>::type>(y2));
}
inline evas::Line::operator ::evas::Line() const { return *static_cast< ::evas::Line const*>(static_cast<void const*>(this)); }
inline evas::Line::operator ::evas::Line&() { return *static_cast< ::evas::Line*>(static_cast<void*>(this)); }
inline evas::Line::operator ::evas::Line const&() const { return *static_cast< ::evas::Line const*>(static_cast<void const*>(this)); }
} }
#endif
