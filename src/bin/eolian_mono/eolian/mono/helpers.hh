#ifndef EOLIAN_MONO_HELPERS_HH
#define EOLIAN_MONO_HELPERS_HH

#include "grammar/klass_def.hpp"
#include "utils.hh"

/* General helper functions for the main generators.
 *
 * These range from blacklisting structures to 'nano-generators' (functions that receive
 * a binding-specifict structure and returns a string).
 */

namespace eolian_mono {

namespace attributes = efl::eolian::grammar::attributes;

inline std::string type_full_name(attributes::regular_type_def const& type)
{
   std::string full_name;
   for (auto& name : type.namespaces)
     {
        full_name += name + ".";
     }
   full_name += type.base_type;
   return full_name;
}

inline std::string struct_full_name(attributes::struct_def const& struct_)
{
   std::string full_name;
   for (auto& name : struct_.namespaces)
     {
        full_name += name + ".";
     }
   full_name += struct_.cxx_name;
   return full_name;
}

// Blacklist structs that require some kind of manual binding.
inline bool is_struct_blacklisted(std::string const& full_name)
{
   return full_name == "Efl.Event.Description"
       || full_name == "Eina.Binbuf"
       || full_name == "Eina.Slice"
       || full_name == "Eina.Rw_Slice";
}

inline bool is_struct_blacklisted(attributes::struct_def const& struct_)
{
   return is_struct_blacklisted(struct_full_name(struct_));
}

inline bool is_struct_blacklisted(attributes::regular_type_def const& struct_)
{
   return is_struct_blacklisted(type_full_name(struct_));
}

inline bool need_struct_conversion(attributes::regular_type_def const* regular)
{
   return regular && regular->is_struct() && !is_struct_blacklisted(*regular);
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
       || (regular->is_struct() && type_full_name(*regular) != "Eina.Binbuf")
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

inline std::string to_field_name(std::string const& in)
{
  return utils::capitalize(in);
}

inline std::string klass_name_to_csharp(attributes::klass_name const& clsname)
{
  std::ostringstream output;

  for (auto namesp : clsname.namespaces)
    output << utils::to_lowercase(namesp) << ".";

  output << clsname.eolian_name;

  return output.str();
}

}

#endif
