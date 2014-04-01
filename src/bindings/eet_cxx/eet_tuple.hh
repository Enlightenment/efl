#ifndef EFL_EET_EET_TUPLE_HH_
#define EFL_EET_EET_TUPLE_HH_

namespace efl { namespace eet {

namespace _mpl {

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

template <typename A>
struct pop_front;

template <template <typename...> class C, typename T, typename... Args>
struct pop_front<C<T, Args...> >
{
  typedef C<Args...> type;
};

}

} }

#endif
