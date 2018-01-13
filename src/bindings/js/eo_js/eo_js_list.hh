#ifndef EFL_EO_JS_LIST_HH
#define EFL_EO_JS_LIST_HH

namespace efl { namespace eo { namespace js {

template <typename...Items>
struct list
{
};

template <typename List1, typename...Lists>
struct list_concat;

template <typename...List1>
struct list_concat<list<List1...>>
{
  typedef list<List1...> type;
};
      
template <typename...List1, typename...List2>
struct list_concat<list<List1...>, list<List2...>>
{
  typedef list<List1..., List2...> type;
};

template <typename...List1, typename...List2, typename...List3, typename...Lists>
struct list_concat<list<List1...>, list<List2...>, list<List3...>, Lists...>
  : list_concat<list<List1..., List2..., List3...>, Lists...>
{};
      
template <typename T, typename PartialList, typename...Items>
struct list_remove_first_impl;

template <typename T, typename...Partials, typename...Items>
struct list_remove_first_impl<T, list<Partials...>, T, Items...>
{
  typedef list<Partials..., Items...> type;
};

template <typename T, typename PartialList>
struct list_remove_first_impl<T, PartialList>
{
  typedef PartialList type;
};
  
template <typename T, typename...Partials, typename T0, typename...Items>
struct list_remove_first_impl<T, list<Partials...>, T0, Items...>
  : list_remove_first_impl<T, list<Partials..., T0>, Items...>
{
};
      
template <typename T, typename List>
struct list_remove_first;

template <typename T, typename...Items>
struct list_remove_first<T, list<Items...>>
  : list_remove_first_impl<T, list<>, Items...>
{
};
      
template <typename Pred, typename Partial, typename List>
struct list_sort_impl;

template <typename T>
struct message_fail
{
  struct type {};
  static_assert(std::is_same<T, type>::value);
};
      
template <typename Pred, typename...Partials, typename T0, typename...T>
struct list_sort_impl<Pred, list<Partials...>, list<T0, T...>>
{
  typedef typename eina::_mpl::min<Pred, T0, T...>::type min_type;
  typedef typename list_remove_first<min_type, list<T0, T...>>::type new_list_type;
  typedef typename list_sort_impl<Pred, list<Partials..., min_type>, new_list_type>::type type;
  //typedef typename message_fail<min_type>::type fail;
};

template <typename Pred, typename Partial>
struct list_sort_impl<Pred, Partial, list<>>
{
  typedef Partial type;
};
      
template <typename Pred, typename List1, typename...Lists>
struct list_sort
  : list_sort_impl<Pred, list<>, typename list_concat<List1, Lists...>::type>
{};

template <typename T, typename I, typename Fold>
struct list_fold;

template <typename I, typename Fold, typename...T>
struct list_fold_aux;


template <typename I, typename Fold, typename T>
struct list_fold_aux<I, Fold, T> : Fold::template apply<I, T>
{
};

template <typename I, typename Fold, typename T, typename...Ts>
struct list_fold_aux<I, Fold, T, Ts...> : list_fold_aux
  <typename Fold::template apply<I, T>::type
   , Fold, Ts...>
{
  
};
      
template <typename...T, typename I, typename Fold>
struct list_fold<list<T...>, I, Fold> : list_fold_aux<I, Fold, T...>
{};

/// find_if

template <typename List, typename Pred, typename Enable = void>
struct list_find_if_impl;
      
template <typename I, typename...List, typename Pred>
struct list_find_if_impl<list<I, List...>, Pred
                         , typename std::enable_if<(Pred::template apply<I>::type::value)>::type>
{
  typedef I type;
};

template <typename I, typename...List, typename Pred>
struct list_find_if_impl<list<I, List...>, Pred
                         , typename std::enable_if<!(Pred::template apply<I>::type::value)>::type>
  : list_find_if_impl<list<List...>, Pred>
{
};

template <typename Pred>
struct list_find_if_impl<list<>, Pred>
{
};
      
template <typename List, typename Pred>
struct list_find_if : list_find_if_impl<List, Pred>
{
};

template <typename Pred, typename T>
struct list_min;

template <typename Pred, typename T>
struct list_max;

template <typename...T, typename Pred>
struct list_min<Pred, list<T...>>
  : eina::_mpl::min<Pred, T...> {};

template <typename...T, typename Pred>
struct list_max<Pred, list<T...>>
  : eina::_mpl::max<Pred, T...> {};
      
} } }

#endif
