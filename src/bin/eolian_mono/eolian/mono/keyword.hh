#ifndef EOLIAN_CXX_KEYWORD_HH
#define EOLIAN_CXX_KEYWORD_HH

#include <string>
#include <strings.h>
#include <vector>

#include "name_helpers.hh"

namespace eolian_mono {
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
     || is_iequal(name,  "register")
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


std::string managed_method_name(std::string const& underscore_name)
{
  std::vector<std::string> names = name_helpers::split(underscore_name, '_');

  name_helpers::reorder_verb(names);

  return escape_keyword(name_helpers::pascal_case(names));
}

}

#endif
