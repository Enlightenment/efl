#ifndef EOLIAN_CXX_STD_EO_CLASS_SCOPE_GUARD_GENERATOR_HH
#define EOLIAN_CXX_STD_EO_CLASS_SCOPE_GUARD_GENERATOR_HH

#include <cassert>

#include "type_generator.hh"

namespace efl { namespace eolian { namespace grammar {

template <typename T>
struct _scope_guard_head
{
   eo_class const& _cls;
   T const& _e;
   _scope_guard_head(eo_class const& cls, T const& e)
     : _cls(cls), _e(e) {}
};

template <typename T>
_scope_guard_head<T> scope_guard_head(eo_class const& cls, T const& e)
{
   return _scope_guard_head<T>(cls, e);
}

template <typename T>
inline std::ostream&
operator<<(std::ostream& out, _scope_guard_head<T> const& x)
{
   assert(x._e.scope != eolian_scope::private_);
   if (x._e.scope == eolian_scope::protected_)
     out << "#ifdef " << name_upper(x._cls) << "_PROTECTED" << endl;
   return out;
}

template <typename T>
struct _scope_guard_tail
{
   eo_class const& _cls;
   T const& _e;
   _scope_guard_tail(eo_class const& cls, T const& e)
     : _cls(cls), _e(e) {}
};

template <typename T>
struct _scope_guard_tail<T> scope_guard_tail(eo_class const& cls, T const& e)
{
   return _scope_guard_tail<T>(cls, e);
}

template <typename T>
inline std::ostream&
operator<<(std::ostream& out, _scope_guard_tail<T> const& x)
{
   if (x._e.scope == eolian_scope::protected_)
     out << "#endif" << endl;
   return out;
}

} } }

#endif
