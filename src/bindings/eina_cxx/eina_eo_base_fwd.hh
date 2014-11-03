#ifndef EINA_EO_BASE_FWD_HH
#define EINA_EO_BASE_FWD_HH

#include <Eo.h>
#include <type_traits>

namespace efl { namespace eo {

struct base;
    
} }

namespace std {
template <>
struct is_base_of< ::efl::eo::base, ::Eo > : std::false_type {};
template <>
struct is_base_of< ::efl::eo::base, const ::Eo > : std::false_type {};
template <>
struct is_base_of< ::efl::eo::base, volatile ::Eo > : std::false_type {};
template <>
struct is_base_of< ::efl::eo::base, const volatile ::Eo > : std::false_type {};

template <>
struct is_base_of< const ::efl::eo::base, ::Eo > : std::false_type {};
template <>
struct is_base_of< const ::efl::eo::base, const ::Eo > : std::false_type {};
template <>
struct is_base_of< const ::efl::eo::base, volatile ::Eo > : std::false_type {};
template <>
struct is_base_of< const ::efl::eo::base, const volatile ::Eo > : std::false_type {};
}

#endif
