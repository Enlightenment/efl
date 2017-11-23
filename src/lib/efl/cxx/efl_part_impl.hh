//#ifndef EFL_PART_IMPL_HH
//#define EFL_PART_IMPL_HH

namespace efl {
inline ::efl::eolian::return_traits< ::efl::Object>::type Part::part( ::efl::eolian::in_traits< ::efl::eina::string_view>::type name) const
{
   Eo *handle = ::efl_part(this->_eo_ptr(), name.c_str());
   ::efl_auto_unref_set(handle, false);
   return ::efl::Object{handle};
}
}

namespace eo_cxx {
namespace efl { 
inline ::efl::eolian::return_traits< ::efl::Object>::type Part::part( ::efl::eolian::in_traits< ::efl::eina::string_view>::type name) const
{
   Eo *handle = ::efl_part(this->_eo_ptr(), name.c_str());
   ::efl_auto_unref_set(handle, false);
   return ::efl::Object{handle};
}
inline efl::Part::operator ::efl::Part() const { return *static_cast< ::efl::Part const*>(static_cast<void const*>(this)); }
inline efl::Part::operator ::efl::Part&() { return *static_cast< ::efl::Part*>(static_cast<void*>(this)); }
inline efl::Part::operator ::efl::Part const&() const { return *static_cast< ::efl::Part const*>(static_cast<void const*>(this)); }
} }

//#endif
