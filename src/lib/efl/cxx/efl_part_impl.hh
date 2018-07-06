#ifndef EFL_PART_MANUAL_IMPL_HH
#define EFL_PART_MANUAL_IMPL_HH

#define EOLIAN_CXX_EFL_PART_DECLARATION \
   ::efl::Object part(::efl::eina::string_view const& name) const;

#define EOLIAN_CXX_EFL_PART_IMPLEMENTATION \
inline ::efl::Object Part::part(::efl::eina::string_view const& name) const \
{ \
   ::Eo *handle = ::efl_part_get(_eo_ptr(), name.c_str()); \
   return ::efl::Object{handle}; \
}

#endif
