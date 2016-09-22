#ifndef EFL_EINA_EINA_ALIGNED_UNION_HH_
#define EFL_EINA_EINA_ALIGNED_UNION_HH_

namespace efl { namespace eina { namespace _mpl {

template <std::size_t...Numbers>
struct max;

template <std::size_t A0>
struct max<A0> : std::integral_constant<std::size_t, A0> {};

template <std::size_t A0, std::size_t A1, std::size_t...Args>
struct max<A0, A1, Args...> : max<(A0 > A1 ? A0 : A1), Args...> {};

}

// Workaround needed for GCC before 5.1
template <std::size_t Min, typename...Args>
struct aligned_union
{
   static constexpr std::size_t alignment_value = _mpl::max<alignof(Args)...>::value;

   typedef typename std::aligned_storage
   < _mpl::max<Min, sizeof(Args)...>::value
     , alignment_value >::type type;
};

} }

#endif

