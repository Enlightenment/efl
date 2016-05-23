#ifndef EFL_EINA_FOLD_HH_
#define EFL_EINA_FOLD_HH_

#include <eina_tuple.hh>

namespace efl { namespace eina {

namespace _mpl {

template <typename T, typename F, typename A0, bool B = std::is_same<T, std::tuple<> >::value>
struct fold_impl
{
  typedef typename F::template apply<A0, typename std::tuple_element<0, T>::type>::type result;
  typedef typename fold_impl<typename _mpl::pop_front<T>::type
                             , F, result
                             >::type
  type;
};

template <typename T, typename F, typename A0>
struct fold_impl<T, F, A0, true>
{
  typedef A0 type;
};

template <typename T, typename F, typename A0>
struct fold : fold_impl<T, F, A0>
{};

} } }

#endif
