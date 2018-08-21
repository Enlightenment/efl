#ifndef EINA_EO_CONCRETE_FWD_HH
#define EINA_EO_CONCRETE_FWD_HH

#include <Eo.h>
#include <type_traits>

namespace efl { namespace eo {

struct concrete;

template <typename T>
struct is_eolian_object : std::false_type {};
template <typename T>
struct is_eolian_object<T const> : is_eolian_object<T> {};
template <>
struct is_eolian_object<eo::concrete> : std::true_type {};
    
} }


#endif
