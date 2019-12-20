/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef EOLIAN_CXX_QUALIFIER_DEF_HH
#define EOLIAN_CXX_QUALIFIER_DEF_HH

#include "grammar/type_traits.hpp"

#include <Eolian.h>

namespace efl { namespace eolian { namespace grammar { namespace attributes {

enum class qualifier_info {
  is_none
, is_own = 1
, is_const = 2
, is_optional = 4
, is_ref = 8
};

struct qualifier_bool
{
  qualifier_info v;
  qualifier_bool(qualifier_info v)
    : v(v) {}

  typedef qualifier_info(qualifier_bool::*unspecified_bool_type)() const;
  
  operator unspecified_bool_type() const
  {
    return v != qualifier_info::is_none ? &qualifier_bool::operator qualifier_info : nullptr;
  }
  operator qualifier_info() const { return v; }
};
inline qualifier_bool operator|(qualifier_info lhs, qualifier_info rhs)
{
  return static_cast<qualifier_info>(static_cast<int>(lhs) | static_cast<int>(rhs));
}
inline qualifier_bool operator&(qualifier_info lhs, qualifier_info rhs)
{
  return static_cast<qualifier_info>(static_cast<int>(lhs) & static_cast<int>(rhs));
}
inline qualifier_info operator^(qualifier_info lhs, qualifier_info rhs)
{
  return static_cast<qualifier_info>(static_cast<int>(lhs) & ~static_cast<int>(rhs));
}
inline qualifier_info& operator|=(qualifier_info& lhs, qualifier_info rhs)
{
  lhs = static_cast<qualifier_info>(static_cast<int>(lhs) | static_cast<int>(rhs));
  return lhs;
}
inline qualifier_info& operator&=(qualifier_info& lhs, qualifier_info rhs)
{
  lhs = static_cast<qualifier_info>(static_cast<int>(lhs) & static_cast<int>(rhs));
  return lhs;
}
inline qualifier_info& operator^=(qualifier_info& lhs, qualifier_info rhs)
{
  lhs = static_cast<qualifier_info>(static_cast<int>(lhs) & ~static_cast<int>(rhs));
  return lhs;
}
inline qualifier_bool operator|(qualifier_bool lhs, qualifier_info rhs)
{
  lhs.v |= rhs;
  return lhs;
}
inline qualifier_bool operator&(qualifier_bool lhs, qualifier_info rhs)
{
  lhs.v &= rhs;
  return lhs;
}
inline qualifier_bool operator^(qualifier_bool lhs, qualifier_info rhs)
{
  lhs.v ^= rhs;
  return lhs;
}

inline qualifier_info qualifiers(Eolian_Type const* type, bool is_moved, bool is_by_ref)
{
  qualifier_info is_own = is_moved ? qualifier_info::is_own : qualifier_info::is_none;
  qualifier_info is_const = ::eolian_type_is_const(type) ? qualifier_info::is_const : qualifier_info::is_none;
  qualifier_info is_ref = is_by_ref ? qualifier_info::is_ref : qualifier_info::is_none;
  return is_own | is_const | is_ref;
}
        
struct qualifier_def
{
   qualifier_info qualifier;
   std::string free_function;

   qualifier_def() : qualifier(qualifier_info::is_none) {}
   qualifier_def(qualifier_info info, std::string free_function)
     : qualifier(info), free_function(std::move(free_function)) {}

  typedef qualifier_info(qualifier_bool::*unspecified_bool_type)() const;
  operator unspecified_bool_type() const
  {
    return qualifier != qualifier_info::is_none ? &qualifier_bool::operator qualifier_info : nullptr;
  }
};

inline qualifier_def operator|(qualifier_def lhs, qualifier_info rhs)
{
  lhs.qualifier = lhs.qualifier | rhs;
  return lhs;
}
inline qualifier_def operator&(qualifier_def lhs, qualifier_info rhs)
{
  lhs.qualifier = lhs.qualifier & rhs;
  return lhs;
}
inline qualifier_def operator^(qualifier_def lhs, qualifier_info rhs)
{
  lhs.qualifier = lhs.qualifier ^ rhs;
  return lhs;
}

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

}
using attributes::qualifier_info;
} } }

#endif
