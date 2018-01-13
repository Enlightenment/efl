#ifndef EFL_EINA_EINA_TUPLE_HH_
#define EFL_EINA_EINA_TUPLE_HH_

#include <eina_integer_sequence.hh>
#include <eina_logical.hh>

#include <tuple>

namespace efl { namespace eina { namespace _mpl {

template <typename A, typename... Args>
struct push_back;

template <template <typename... Args> class C, typename... Args, typename... AArgs>
struct push_back<C<Args...>, AArgs...>
{
  typedef C<Args..., AArgs...> type;
};

template <typename A, typename... Args>
struct push_front;

template <template <typename... Args> class C, typename... Args, typename... AArgs>
struct push_front<C<Args...>, AArgs...>
{
  typedef C<Args..., AArgs...> type;
};

template <typename A, std::size_t N = 1>
struct pop_front;
      
template <template <typename...> class C, typename T, typename... Args>
struct pop_front<C<T, Args...>, 1>
{
  typedef C<Args...> type;
};

template <typename A, std::size_t N>
struct pop_front : pop_front<typename pop_front<A, 1>::type, N-1>
{
};

template <typename T, typename F, std::size_t... I>
void for_each_impl(T&& t, F&& f, eina::index_sequence<I...>)
{
  std::initializer_list<int> l = { (f(std::get<I>(t)), 0)...};
  static_cast<void>(l);
}

template <typename T, typename F>
void for_each(T&& t, F&& f)
{
  _mpl::for_each_impl(t, f, eina::make_index_sequence
                      <std::tuple_size<typename std::remove_reference<T>::type>::value>());
}

template <typename T, typename Transform>
struct tuple_transform;

template <typename...T, typename Transform>
struct tuple_transform<std::tuple<T...>, Transform>
{
  typedef std::tuple<typename Transform::template apply<T>::type...> type;
};

template <typename T, typename Tuple>
struct tuple_contains;


      
template <typename T, typename...Ts>
struct tuple_contains<T, std::tuple<Ts...> >
  : _mpl::or_<std::is_same<T, Ts>::value...>
{
};

template <typename T, typename Tuple>
struct tuple_find : std::integral_constant<int, -1> {};

template <typename T, typename... Ts>
struct tuple_find<T, std::tuple<T, Ts...> > : std::integral_constant<std::size_t, 0> {};

template <typename T, typename T1, typename... Ts>
struct tuple_find<T, std::tuple<T1, Ts...> > : std::integral_constant
 <std::size_t, 1 + tuple_find<T, std::tuple<Ts...> >::value> {};


template <typename T, typename I, typename Fold>
struct tuple_fold;

template <typename I, typename Fold, typename...T>
struct tuple_fold_aux;


template <typename I, typename Fold, typename T>
struct tuple_fold_aux<I, Fold, T> : Fold::template apply<I, T>
{
};

template <typename I, typename Fold, typename T, typename...Ts>
struct tuple_fold_aux<I, Fold, T, Ts...> : tuple_fold_aux
  <typename Fold::template apply<I, T>::type
   , Fold, Ts...>
{
  
};
      
template <typename...T, typename I, typename Fold>
struct tuple_fold<std::tuple<T...>, I, Fold> : tuple_fold_aux<I, Fold, T...>
{};

template <typename Pred, typename...Args>
struct min;

template <typename Pred, typename A0>
struct min<Pred, A0>
{
  typedef A0 type;
};

template <typename Pred, typename A0, typename A1, typename...Args>
struct min<Pred, A0, A1, Args...>
{
  typedef typename min<Pred, A1, Args...>::type rest_result;
  typedef typename std::conditional
    <Pred::template apply<A0, rest_result>::value
     , A0, rest_result>::type type;
};

template <typename Pred, typename...Args>
struct max;

template <typename Pred, typename A0>
struct max<Pred, A0>
{
  typedef A0 type;
};

template <typename Pred, typename A0, typename A1, typename...Args>
struct max<Pred, A0, A1, Args...>
{
  typedef typename max<Pred, A1, Args...>::type rest_result;
  typedef typename std::conditional
    <Pred::template apply<rest_result, A0>::value
     , A0, rest_result>::type type;
};
      
template <typename T, typename Removed>
struct tuple_remove;

template <typename Removed, typename Partial, typename...T>
struct tuple_remove_aux;

template <typename Removed, typename Partial>
struct tuple_remove_aux<Removed, Partial>
{
  typedef Partial type;
};
      
template <typename Removed, typename...Partials, typename...T>
struct tuple_remove_aux<Removed, std::tuple<Partials...>, Removed, T...>
{
  typedef std::tuple<Partials..., T...> type;
};
      
template <typename Removed, typename...Partials, typename A0, typename...T>
struct tuple_remove_aux<Removed, std::tuple<Partials...>, A0, T...>
  : tuple_remove_aux<Removed, std::tuple<Partials..., A0>, T...>
{
};
      
template <typename...T, typename Removed>
struct tuple_remove<std::tuple<T...>, Removed>
  : tuple_remove_aux<Removed, std::tuple<>, T...>
{
};
      ///
      
template <typename T, typename Removed>
struct tuple_remove_all;

template <typename Removed, typename Partial, typename...T>
struct tuple_remove_all_aux;

template <typename Removed, typename Partial>
struct tuple_remove_all_aux<Removed, Partial>
{
  typedef Partial type;
};
      
template <typename Removed, typename...Partials, typename...T>
struct tuple_remove_all_aux<Removed, std::tuple<Partials...>, Removed, T...>
  : tuple_remove_all_aux<Removed, std::tuple<Partials...>, T...>
{
};
      
template <typename Removed, typename...Partials, typename A0, typename...T>
struct tuple_remove_all_aux<Removed, std::tuple<Partials...>, A0, T...>
  : tuple_remove_all_aux<Removed, std::tuple<Partials..., A0>, T...>
{
};
      
template <typename...T, typename Removed>
struct tuple_remove_all<std::tuple<T...>, Removed>
  : tuple_remove_all_aux<Removed, std::tuple<>, T...>
{
};

template <typename Tuple, typename Pred, typename Partial>
struct tuple_sort_unique_aux;

template <typename T0, typename...T, typename Pred, typename...Partials>
struct tuple_sort_unique_aux<std::tuple<T0, T...>, Pred, std::tuple<Partials...>>
{
  typedef typename min<Pred, T0, T...>::type min_type;
  typedef typename tuple_remove_all<std::tuple<T0, T...>, min_type>::type new_tuple_type;
  typedef typename tuple_sort_unique_aux<new_tuple_type, Pred, std::tuple<Partials..., min_type>>::type type;
};

template <typename Pred, typename Partial>
struct tuple_sort_unique_aux<std::tuple<>, Pred, Partial>
{
  typedef Partial type;
};
      
template <typename T, typename Pred>
struct tuple_sort_unique
  : tuple_sort_unique_aux<T, Pred, std::tuple<>>
{};

template <typename Tuple, typename Pred, typename Partial>
struct tuple_sort_aux;

template <typename T0, typename...T, typename Pred, typename...Partials>
struct tuple_sort_aux<std::tuple<T0, T...>, Pred, std::tuple<Partials...>>
{
  typedef typename min<Pred, T0, T...>::type min_type;
  typedef typename tuple_remove<std::tuple<T0, T...>, min_type>::type new_tuple_type;
  typedef typename tuple_sort_aux<new_tuple_type, Pred, std::tuple<Partials..., min_type>>::type type;
};

template <typename Pred, typename Partial>
struct tuple_sort_aux<std::tuple<>, Pred, Partial>
{
  typedef Partial type;
};
      
template <typename T, typename Pred>
struct tuple_sort
  : tuple_sort_aux<T, Pred, std::tuple<>>
{};
      
} } }

#endif
