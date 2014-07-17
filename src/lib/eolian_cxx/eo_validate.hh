
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
   return !value.empty() and (isalpha(value[0]) || value[0] == '_');
}

inline bool
_is_valid(eolian_type_instance const& type)
{
   if (type.empty() || (*type.rbegin()).category == eolian_type::complex_)
     return false;
   for (auto rit = ++type.rbegin(), last = type.rend(); rit != last; ++rit)
     {
        if ((*rit).binding.empty() || (*rit).category != eolian_type::complex_)
          return false;
        else if (rit != type.rbegin() && (*rit).category != eolian_type::complex_)
          return false;
     }
   return true;
}

inline bool
_is_valid(parameters_container_type const& parameters)
{
   unsigned int n_callbacks = parameters_count_callbacks(parameters);
   return n_callbacks == 0 || n_callbacks == 1;
}

inline bool
_is_valid(events_container_type const& events)
{
   for (eo_event event : events)
     {
        if (event.name.empty() || event.eo_name.empty())
          return false;
     }
   return true;
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
        _validate((*it).params, cls);
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
        _validate((*it).params, cls);
        // parameters
        for (auto it_p = (*it).params.begin(), last_p = (*it).params.end();
             it_p != last_p; ++it_p)
          {
             _validate((*it_p).name, cls);
             _validate((*it_p).type, cls);
          }
     }
   // events
   _validate(cls.events, cls);
}

} } // namespace efl { namespace eolian {

#endif // EOLIAN_CXX_EO_CLASS_VALIDATE_HH
