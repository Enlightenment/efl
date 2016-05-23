#ifndef EFL_ELDBUS_CXX_ELDBUS_RAW_TUPLE_HH
#define EFL_ELDBUS_CXX_ELDBUS_RAW_TUPLE_HH

#include <eldbus_signature_traits.hh>

#include <eina_fold.hh>
#include <eina_tuple.hh>

namespace efl { namespace eldbus { namespace _detail {

template <typename T>
struct raw_tuple;

template <typename... T>
struct raw_tuple<std::tuple<T...> >
{
  struct push_back
  {
    template <typename L, typename R>
    struct apply
      : eina::_mpl::push_back<L, typename signature_traits<R>::raw_type> {};
  };   

  typedef typename eina::_mpl::fold< std::tuple<T...>, push_back
                                     , std::tuple<> >::type type;
};

} } }

#endif
