#ifndef EOLIAN_CXX_KEYWORD_HH
#define EOLIAN_CXX_KEYWORD_HH

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
     || is_iequal(name, "event"))
    return "kw_" + name;
  return name;
}
      
}

#endif
