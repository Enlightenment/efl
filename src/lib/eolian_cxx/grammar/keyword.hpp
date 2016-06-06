#ifndef EOLIAN_CXX_KEYWORD_HH
#define EOLIAN_CXX_KEYWORD_HH

namespace efl { namespace eolian { namespace grammar {

inline std::string escape_keyword(std::string const& name)
{
  if(name == "delete" || name == "register")
    return "cxx_" + name;
  return name;
}
      
} } }

#endif
