#ifndef EOLIAN_CXX_GENERATOR_HH
#define EOLIAN_CXX_GENERATOR_HH

#include <type_traits>

namespace efl { namespace eolian { namespace grammar {

template <typename T, typename Enable = void>
struct is_generator : std::false_type {};
template <typename T, typename Enable = void>
struct is_eager_generator : std::false_type {};

template <typename T>
struct is_generator<T&> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T&> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T const&> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T const&> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T const> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T const> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T volatile> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T volatile> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T volatile&> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T volatile&> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T volatile const> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T volatile const> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T volatile const&> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T volatile const&> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T> : is_eager_generator<T> {};
      
template <typename G, typename Enable = typename std::enable_if<is_eager_generator<G>::value>::type>
G as_generator(G&& g) { return g; }


} } }

#endif
