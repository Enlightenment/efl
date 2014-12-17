#ifndef EINA_EO_CONCRETE_FWD_HH
#define EINA_EO_CONCRETE_FWD_HH

#include <Eo.h>
#include <type_traits>

namespace efl { namespace eo {

struct concrete;
    
} }

namespace std {
template <>
struct is_base_of< ::efl::eo::concrete, ::Eo > : std::false_type {};
template <>
struct is_base_of< ::efl::eo::concrete, const ::Eo > : std::false_type {};
template <>
struct is_base_of< ::efl::eo::concrete, volatile ::Eo > : std::false_type {};
template <>
struct is_base_of< ::efl::eo::concrete, const volatile ::Eo > : std::false_type {};

template <>
struct is_base_of< const ::efl::eo::concrete, ::Eo > : std::false_type {};
template <>
struct is_base_of< const ::efl::eo::concrete, const ::Eo > : std::false_type {};
template <>
struct is_base_of< const ::efl::eo::concrete, volatile ::Eo > : std::false_type {};
template <>
struct is_base_of< const ::efl::eo::concrete, const volatile ::Eo > : std::false_type {};
}

#endif
