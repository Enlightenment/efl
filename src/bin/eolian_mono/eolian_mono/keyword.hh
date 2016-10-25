#ifndef EOLIAN_CXX_KEYWORD_HH
#define EOLIAN_CXX_KEYWORD_HH

namespace eolian_mono {

inline std::string escape_keyword(std::string const& name)
{
  if(name == "delete" || name == "register")
    return "cxx_" + name;
  return name;
}
      
}

#endif
