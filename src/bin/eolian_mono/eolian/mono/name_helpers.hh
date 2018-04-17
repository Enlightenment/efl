#ifndef EOLIAN_MONO_NAME_HELPERS_HH
#define EOLIAN_MONO_NAME_HELPERS_HH

#include <algorithm>
#include <cctype>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include "utils.hh"

#include "grammar/integral.hpp"
#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

namespace eolian_mono {

/* Utility functions for naming things. Compared to the utils.hh, this header has higher level
 * functions, dealing with the knowledge of how to convert the items to the C# style we are using, for
 * example, while being too short to be implemented as full-fledged generators.
 */
namespace name_helpers {

namespace attributes = efl::eolian::grammar::attributes;

namespace detail {
inline bool is_iequal(std::string const& lhs, std::string const& rhs)
{
  return strcasecmp(lhs.c_str(), rhs.c_str()) == 0;
}
}

inline std::string escape_keyword(std::string const& name)
{
  using detail::is_iequal;
  if(is_iequal(name, "delete")
     || is_iequal(name, "do")
     || is_iequal(name,  "lock")
     || is_iequal(name, "event")
     || is_iequal(name, "in")
     || is_iequal(name, "object")
     || is_iequal(name, "interface")
     || is_iequal(name, "string")
     || is_iequal(name, "internal")
     || is_iequal(name, "fixed")
     || is_iequal(name, "base"))
    return "kw_" + name;

  if (is_iequal(name, "Finalize"))
    return name + "Add"; // Eo's Finalize is actually the end of efl_add.
  return name;
}

inline std::vector<std::string> escape_namespace(std::vector<std::string> namespaces)
{
  // if(namespaces.empty())
  //   namespaces.push_back("nonamespace");
  // else
    {
      for(auto&& i : namespaces)
        i = escape_keyword(i);
    }
  return namespaces;
}

inline std::string get_namespaces(std::vector<std::string> const& namespaces, char separator,
                                  std::function<std::string(std::string const&)> func=[] (std::string const& c) { return c; })
{
   std::stringstream s;
   for (auto&& n : namespaces)
     s << func(n) << separator;

   return s.str();
}

inline std::string get_namespaces(attributes::klass_def const& klass, char separator,
                                  std::function<std::string(std::string const&)> func=[] (std::string const& c) { return c; })
{
   return get_namespaces(klass.namespaces, separator, func);
}

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

inline std::string klass_name_to_csharp(attributes::klass_name const& clsname)
{
  std::ostringstream output;

  for (auto namesp : clsname.namespaces)
    output << utils::to_lowercase(namesp) << ".";

  output << clsname.eolian_name;

  return output.str();
}

inline std::string managed_method_name(std::string const& underscore_name)
{
  std::vector<std::string> names = utils::split(underscore_name, '_');

  name_helpers::reorder_verb(names);

  return escape_keyword(utils::to_pascal_case(names));
}

inline std::string managed_event_name(std::string const& name)
{
   return utils::to_pascal_case(utils::split(name, ','), "") + "Evt";
}

inline std::string managed_event_args_short_name(attributes::event_def evt)
{
   return name_helpers::managed_event_name(evt.name) + "_Args";
}

inline std::string managed_event_args_name(attributes::event_def evt)
{
   std::string prefix = name_helpers::klass_name_to_csharp(evt.klass);
   return prefix + "Concrete." + managed_event_args_short_name(evt);
}

inline std::string translate_inherited_event_name(const attributes::event_def &evt, const attributes::klass_def &klass)
{
   std::stringstream s;

   for (auto&& n : klass.namespaces)
     {
        s << n;
        s << '_';
     }
   s << klass.cxx_name << '_' << managed_event_name(evt.name);
   return s.str();
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

inline std::string klass_get_full_name(attributes::klass_def const& klass)
{
  std::ostringstream output;

  for(auto namesp : klass.namespaces)
    output << utils::to_lowercase(escape_keyword(namesp)) << ".";

  output << klass.eolian_name;

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

struct get_csharp_type_visitor
{
    typedef get_csharp_type_visitor visitor_type;
    typedef std::string result_type;
    std::string operator()(attributes::regular_type_def const& type) const
    {
        std::stringstream csharp_name;
        for (auto&& i  : escape_namespace(type.namespaces))
           csharp_name << utils::to_lowercase(i) << ".";
        csharp_name << type.base_type;

        return csharp_name.str();
    }
    std::string operator()(attributes::klass_name const& name) const
    {
        std::stringstream csharp_name;
        for (auto&& i  : escape_namespace(name.namespaces))
           csharp_name << utils::to_lowercase(i) << ".";
        csharp_name << name.eolian_name;

        return csharp_name.str();
    }
    std::string operator()(attributes::complex_type_def const&) const
    {
        return "UNSUPPORTED";
    }
};



} // namespace name_helpers

} // namespace eolian_mono

#endif
