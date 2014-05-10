
#ifndef EOLIAN_CXX_EO_CLASS_VALIDATE_HH
#define EOLIAN_CXX_EO_CLASS_VALIDATE_HH

#include <string>
#include <cassert>

#ifdef DEBUG
#include <iostream>
#endif

#include "eo_types.hh"

namespace efl { namespace eolian {

inline bool
_isvalid(const std::string& name)
{
   return name.size() > 0 and isalpha(name[0]);
}

inline void
eo_class_validate(const eo_class& cls)
{
   assert(_isvalid(cls.name));

   assert(cls.type != eo_class::regular_ ||
          cls.type != eo_class::regular_noninst_ ||
          cls.type != eo_class::interface_ ||
          cls.type != eo_class::mixin_);

   {
      constructors_container_type::const_iterator it,
        first = cls.constructors.begin(),
        last = cls.constructors.end();
      for (it = first; it != last; ++it)
        {
           parameters_container_type::const_iterator
             param = (*it).params.begin(),
             last_param = (*it).params.end();
           assert(_isvalid((*it).name));
           for (; param != last_param; ++param)
             {
                assert(_isvalid((*param).name));
                assert(_isvalid((*param).type));
             }
        }
   }

   {
      functions_container_type::const_iterator it,
        first = cls.functions.begin(),
        last  = cls.functions.end();
      for (it = first; it != last; ++it)
        {
           assert(_isvalid((*it).name));
           assert(_isvalid((*it).impl));
           assert(_isvalid((*it).ret));
           parameters_container_type::const_iterator
             param = (*it).params.begin(),
             last_param = (*it).params.end();
           for (; param != last_param; ++param)
             {
                assert(_isvalid((*param).name));
                assert(_isvalid((*param).type));
             }
        }
   }
}

} } // namespace efl { namespace eolian {

#endif // EOLIAN_CXX_EO_CLASS_VALIDATE_HH
