#ifndef EFL_EINA_EINA_TUPLE_HH_
#define EFL_EINA_EINA_TUPLE_HH_

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
      
} } }

#endif
