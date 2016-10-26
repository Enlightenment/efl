#ifndef EOLIAN_CXX_KEYWORD_HH
#define EOLIAN_CXX_KEYWORD_HH

namespace eolian_mono {

inline std::string escape_keyword(std::string const& name)
{
  if(name == "delete" || name == "register" || name == "do")
    return "mono_" + name;
  return name;
}
      
}

#endif
