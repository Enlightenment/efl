#ifndef EOLIAN_MONO_NAME_HELPERS_HH
#define EOLIAN_MONO_NAME_HELPERS_HH

#include <algorithm>
#include <cctype>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include "utils.hh"

#include "grammar/klass_def.hpp"

namespace eolian_mono {

/* Utility functions for naming things. Compared to the utils.hh, this header has higher level
 * functions, dealing with the knowledge of how to convert the items to the C# style we are using, for
 * example, while being too short to be implemented as full-fledged generators.
 */
namespace name_helpers {

namespace attributes = efl::eolian::grammar::attributes;

static const std::vector<std::string> verbs =
  {
    "add",
    "get",
    "is",
    "del",
    "thaw",
    "freeze",
    "save",
    "wait",
    "eject",
    "raise",
    "lower",
    "load",
    "dup",
    "reset",
    "unload",
    "close",
    "set",
    "interpolate",
    "has",
    "grab",
    "check",
    "find",
    "ungrab",
    "unset",
    "clear",
    "pop",
    "new",
    "peek",
    "push",
    "update",
    "show",
    "move",
    "hide",
    "calculate",
    "resize",
    "attach",
    "pack",
    "unpack",
    "emit",
    "call",
    "append"
  };

const std::vector<std::string> not_verbs =
  {
    "below",
    "above",
    "name",
    "unfreezable",
    "value",
    "r",
    "g",
    "b",
    "a",
    "finalize",
    "destructor",
    "to",
    "circle",
    "rect",
    "path",
    "commands",
    "type",
    "colorspace"
    "op",
    "type",
    "properties",
    "status",
    "status",
    "relative",
    "ptr",
    "pair",
    "pos",
    "end"
  };

void reorder_verb(std::vector<std::string> &names)
{
  if (names.size() <= 1)
    return;

  std::string verb = names.back();

  if (std::find(verbs.begin(), verbs.end(), verb) != verbs.end())
    {
       names.pop_back();
       names.insert(names.begin(), verb);
    }
}

inline std::string managed_event_name(std::string const& name)
{
   return utils::to_pascal_case(utils::split(name, ','), "") + "Evt";
}

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

inline std::string klass_get_name(attributes::klass_name const &clsname)
{
  std::ostringstream output;

  output << klass_name_to_csharp(clsname);
  output << "Concrete.";

  for (auto namesp : clsname.namespaces)
    output << utils::to_lowercase(namesp) << "_";
  output << utils::to_lowercase(clsname.eolian_name);
  output << "_class_get";

  return output.str();
}

} // namespace name_helpers

} // namespace eolian_mono

#endif
