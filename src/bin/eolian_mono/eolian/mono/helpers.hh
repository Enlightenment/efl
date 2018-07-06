#ifndef EOLIAN_MONO_HELPERS_HH
#define EOLIAN_MONO_HELPERS_HH

#include "grammar/klass_def.hpp"
#include "blacklist.hh"
#include "name_helpers.hh"

namespace eolian_mono {

namespace helpers {

/* General helpers, not related directly with generating strings (those go in the name_helpers.hh). */

namespace attributes = efl::eolian::grammar::attributes;

inline bool need_struct_conversion(attributes::regular_type_def const* regular)
{
   return regular && regular->is_struct() && !blacklist::is_struct_blacklisted(*regular);
}

inline bool need_struct_conversion(attributes::parameter_def const& param, attributes::regular_type_def const* regular)
{
   if (param.direction == attributes::parameter_direction::in && param.type.has_own)
     return false;

   return need_struct_conversion(regular);
}

inline bool need_struct_conversion_in_return(attributes::type_def const& ret_type, attributes::parameter_direction const& direction)
{
   auto regular = efl::eina::get<attributes::regular_type_def>(&ret_type.original_type);

   if (!regular->is_struct())
     return false;

   if (regular->is_struct() && (direction == attributes::parameter_direction::out || direction == attributes::parameter_direction::unknown))
     return false;

   if (ret_type.has_own)
     return false;

   return true;
}

inline bool need_pointer_conversion(attributes::regular_type_def const* regular)
{
   if (!regular)
     return false;

   if (regular->is_enum()
       || (regular->is_struct() && name_helpers::type_full_eolian_name(*regular) != "Eina.Binbuf")
      )
     return true;

   std::set<std::string> const types {
     "bool", "char"
     , "byte" , "short" , "int" , "long" , "llong" , "int8" , "int16" , "int32" , "int64" , "ssize"
     , "ubyte", "ushort", "uint", "ulong", "ullong", "uint8", "uint16", "uint32", "uint64", "size"
     , "ptrdiff"
     , "float", "double"
   };
   if (types.find(regular->base_type) != types.end())
     return true;

   return false;
}

} // namespace helpers

} // namespace eolian_mono

#endif
