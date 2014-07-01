
#ifndef EOLIAN_CXX_TYPE_LOOKUP_HH
#define EOLIAN_CXX_TYPE_LOOKUP_HH

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <cctype>
#include <iterator>
#include <cassert>
#include <cstddef>

#include <Eolian.h>
#include <Eina.hh>

#include "eo_types.hh"
#include "safe_strings.hh"

namespace eolian_cxx {

inline std::string
type_lookup(Eolian_Type type)
{
   if (type == NULL)
     return "void";
   // XXX add complex types implementation.
   const char *tps = eolian_type_c_type_get(type);
   std::string ret = safe_str(tps);
   ::eina_stringshare_del(tps);
   return ret;
}


} // namespace eolian_cxx {

#endif // EOLIAN_CXX_TYPE_LOOKUP_HH
