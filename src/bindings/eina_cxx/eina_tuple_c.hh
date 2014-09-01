#ifndef EFL_EINA_EINA_TUPLE_C_HH_
#define EFL_EINA_EINA_TUPLE_C_HH_

#include <tuple>

namespace efl { namespace eina { namespace _mpl {

template <typename T, T... Ts>
using tuple_c = std::tuple<std::integral_constant<T, Ts>...>;

template <typename T, T... Ts>
constexpr std::size_t tuple_c_size(tuple_c<T, Ts...> const&)
{
  return sizeof...(Ts);
}

constexpr std::size_t tuple_c_size(std::tuple<> const&)
{
  return 0;
}

} } }

#endif
