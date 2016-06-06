#ifndef EOLIAN_CXX_QUALIFIER_DEF_HH
#define EOLIAN_CXX_QUALIFIER_DEF_HH

#include "grammar/type_traits.hpp"

#include <Eolian.h>

namespace efl { namespace eolian { namespace grammar { namespace attributes {

enum class qualifier_info {
  is_none
, is_own = 1
, is_const = 4
, is_const_own
, is_optional = 8
, is_optional_own
, is_optional_const
, is_optional_const_own
};

inline qualifier_info qualifiers(Eolian_Type const* type)
{
  bool is_own = ::eolian_type_is_own(type);
  bool is_const = ::eolian_type_is_const(type);
  if(is_own && is_const)
    return qualifier_info::is_const_own;
  else if(is_own)
    return qualifier_info::is_own;
  else if(is_const)
    return  qualifier_info::is_const;
  else
    return qualifier_info::is_none;
}
        
inline bool is_own(qualifier_info i)
{
  switch(i)
    {
    case qualifier_info::is_own:
    case qualifier_info::is_const_own:
    case qualifier_info::is_optional_own:
    case qualifier_info::is_optional_const_own:
      return true;
    default:
      return false;
    }
}

inline bool is_const(qualifier_info i)
{
  switch(i)
    {
    case qualifier_info::is_const:
    case qualifier_info::is_const_own:
    case qualifier_info::is_optional_const:
    case qualifier_info::is_optional_const_own:
      return true;
    default:
      return false;
    }
}
        
inline bool is_optional(qualifier_info i)
{
  switch(i)
    {
    case qualifier_info::is_optional:
    case qualifier_info::is_optional_own:
    case qualifier_info::is_optional_const:
    case qualifier_info::is_optional_const_own:
      return true;
    default:
      return false;
    }
}
        
struct qualifier_def
{
   qualifier_info qualifier;
   std::string free_function;
};

inline bool operator<(qualifier_def const& lhs, qualifier_def const& rhs)
{
  return lhs.qualifier < rhs.qualifier ||
        (!(rhs.qualifier < lhs.qualifier) && lhs.free_function < rhs.free_function);
}
inline bool operator>(qualifier_def const& lhs, qualifier_def const& rhs)
{
  return rhs < lhs;
}
inline bool operator==(qualifier_def const& lhs, qualifier_def const& rhs)
{
  return rhs.qualifier == lhs.qualifier && rhs.free_function == lhs.free_function;
}
inline bool operator!=(qualifier_def const& lhs, qualifier_def const& rhs)
{
  return !(rhs == lhs);
}

inline void add_optional(qualifier_def& q)
{
  switch (q.qualifier)
    {
    case qualifier_info::is_none:
      q.qualifier = qualifier_info::is_optional;
      break;
    case qualifier_info::is_own:
      q.qualifier = qualifier_info::is_optional_own;
      break;
    case qualifier_info::is_const:
      q.qualifier = qualifier_info::is_optional_const;
      break;
    case qualifier_info::is_const_own:
      q.qualifier = qualifier_info::is_optional_const_own;
      break;
    default:
      break;
    }
}
inline void remove_optional(qualifier_def& q)
{
  switch (q.qualifier)
    {
    case qualifier_info::is_optional:
      q.qualifier = qualifier_info::is_none;
      break;
    case qualifier_info::is_optional_own:
      q.qualifier = qualifier_info::is_own;
      break;
    case qualifier_info::is_optional_const:
      q.qualifier = qualifier_info::is_const;
      break;
    case qualifier_info::is_optional_const_own:
      q.qualifier = qualifier_info::is_const_own;
      break;
    default:
      break;
    }
}
inline void remove_own(qualifier_def& q)
{
  switch (q.qualifier)
    {
    case qualifier_info::is_own:
      q.qualifier = qualifier_info::is_none;
      break;
    case qualifier_info::is_const_own:
      q.qualifier = qualifier_info::is_const;
      break;
    case qualifier_info::is_optional_own:
      q.qualifier = qualifier_info::is_optional;
      break;
    case qualifier_info::is_optional_const_own:
      q.qualifier = qualifier_info::is_optional_const;
      break;
    default:
      break;
    }
}

inline bool is_optional(qualifier_def const& i)
{
  return is_optional(i.qualifier);
}
inline bool is_own(qualifier_def const& i)
{
  return is_own(i.qualifier);
}
inline bool is_const(qualifier_def const& i)
{
  return is_const(i.qualifier);
}
        
        
} } } }

#endif
