#ifndef EINA_CXX_EINA_TUPLE_UNWRAP_HH
#define EINA_CXX_EINA_TUPLE_UNWRAP_HH

#include <eina_integer_sequence.hh>

namespace efl { namespace eina {

template <typename Callable, typename T, std::size_t... S>
auto call_tuple_unwrap(Callable const& callable, T const& tuple
                       , eina::index_sequence<S...>)
  -> decltype(callable(std::get<S>(tuple)...))
{
  return callable(std::get<S>(tuple)...);
}

template <typename Callable, typename T, std::size_t... S
          , typename... Args>
auto call_tuple_unwrap_prefix(Callable const& callable, T const& tuple
                              , eina::index_sequence<S...>
                              , Args&&... args)
  -> decltype(callable(std::move(args)..., std::get<S>(tuple)...))
{
  return callable(std::move(args)..., std::get<S>(tuple)...);
}

template <typename Callable, typename T, std::size_t... S
          , typename... Args>
auto call_tuple_unwrap_suffix(Callable const& callable, T const& tuple
                              , eina::index_sequence<S...>
                              , Args&&... args)
  -> decltype(callable(std::get<S>(tuple)..., std::move(args)...))
{
  return callable(std::get<S>(tuple)..., std::move(args)...);
}

} }

#endif
