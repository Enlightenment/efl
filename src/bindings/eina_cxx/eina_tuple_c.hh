#ifndef EFL_EINA_EINA_TUPLE_C_HH_
#define EFL_EINA_EINA_TUPLE_C_HH_

#include <tuple>

namespace efl { namespace eina { namespace _mpl {

template <typename T, T... Ts>
using tuple_c = std::tuple<std::integral_constant<T, Ts>...>;
      
} } }

#endif
