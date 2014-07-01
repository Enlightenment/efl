
#ifndef EOLIAN_CXX_EO_CLASS_VALIDATE_HH
#define EOLIAN_CXX_EO_CLASS_VALIDATE_HH

#include <algorithm>
#include <string>
#include <cassert>
#include <cstdlib>
#include "eo_types.hh"

namespace efl { namespace eolian {

inline bool
_is_valid(std::string const& value)
{
   return !value.empty() and isalpha(value[0]);
}

template <typename T>
inline void
_validate(T val, const eo_class& cls)
{
   if(!_is_valid(val))
     {
        static_cast<void>(cls);
#ifndef NDEBUG
   std::abort();
#endif
     }
}

inline void
eo_class_validate(const eo_class& cls)
{
   // class name and type
   _validate(cls.name, cls);
   assert(cls.type != eo_class::regular_ ||
          cls.type != eo_class::regular_noninst_ ||
          cls.type != eo_class::interface_ ||
          cls.type != eo_class::mixin_);

   // constructors
   for (auto it = cls.constructors.cbegin(), last = cls.constructors.cend();
        it != last; ++it)
     {
        _validate((*it).name, cls);
        // parameters
        for (auto it_p = (*it).params.begin(), last_p = (*it).params.end();
             it_p != last_p; ++it_p)
          {
             _validate((*it_p).name, cls);
             _validate((*it_p).type, cls);
          }
     }
   // functions
   for (auto it = cls.functions.begin(), last  = cls.functions.end();
        it != last; ++it)
     {
        _validate((*it).name, cls);
        _validate((*it).impl, cls);
        _validate((*it).ret, cls);
        // parameters
        for (auto it_p = (*it).params.begin(), last_p = (*it).params.end();
             it_p != last_p; ++it_p)
          {
             _validate((*it_p).name, cls);
             _validate((*it_p).type, cls);
          }
     }
}

} } // namespace efl { namespace eolian {

#endif // EOLIAN_CXX_EO_CLASS_VALIDATE_HH
