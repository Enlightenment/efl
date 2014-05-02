
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
_dbg(const std::string& msg)
{
#ifdef DEBUG
   std::cerr << "eo_validate() - " << msg << std::endl;
#endif
}

inline void
eo_class_validate(const eo_class& cls)
{
   _dbg("class name... " + cls.name);
   assert(_isvalid(cls.name));

   _dbg("class type... " + cls.type);
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
           _dbg("constructor... " + (*it).name);
           assert(_isvalid((*it).name));
           for (; param != last_param; ++param)
             {
                _dbg("constructor parameter... " + (*param).name);
                assert(_isvalid((*param).name));
                _dbg("constructor parameter type... " + (*param).type);
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
           _dbg("function... " + (*it).name);
           assert(_isvalid((*it).name));
           _dbg("function api... " + (*it).impl);
           assert(_isvalid((*it).impl));
           _dbg("function return... " + (*it).ret);
           assert(_isvalid((*it).ret));
           parameters_container_type::const_iterator
             param = (*it).params.begin(),
             last_param = (*it).params.end();
           for (; param != last_param; ++param)
             {
                _dbg("function parameter... " + (*param).name);
                assert(_isvalid((*param).name));
                _dbg("function parameter type... " + (*param).type);
                assert(_isvalid((*param).type));
             }
        }
   }
}

} } // namespace efl { namespace eolian {

#endif // EOLIAN_CXX_EO_CLASS_VALIDATE_HH
