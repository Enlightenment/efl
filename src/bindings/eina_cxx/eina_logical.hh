#ifndef EFL_EINA_LOGICAL_HH
#define EFL_EINA_LOGICAL_HH

#include <type_traits>

namespace efl { namespace eina { namespace _mpl {

template <bool... N>
struct or_impl
{
  static constexpr bool value()
  {
    return ;
  }
};

template <bool... N>
struct or_;

template <>
struct or_<> : std::integral_constant<bool, false> {};

template <bool B>
struct or_<B> : std::integral_constant<bool, B> {};

template <bool B1, bool B2, bool... Bs>
struct or_<B1, B2, Bs...> : std::integral_constant<bool, B1 || B2 || or_<Bs...>::value> {};

} } }

#endif
