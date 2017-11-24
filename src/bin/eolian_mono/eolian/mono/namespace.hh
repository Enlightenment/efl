#ifndef EOLIAN_MONO_NAMESPACE_HH
#define EOLIAN_MONO_NAMESPACE_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "grammar/type.hpp"
#include "using_decl.hh"
#include "keyword.hh"

namespace eolian_mono {

std::vector<std::string> escape_namespace(std::vector<std::string> namespaces)
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
  
}

#endif
