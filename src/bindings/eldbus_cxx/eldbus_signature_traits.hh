#ifndef ELDBUS_SIGNATURE_TRAITS_HH_
#define ELDBUS_SIGNATURE_TRAITS_HH_

#include <eina_fold.hh>
#include <eina_integer_sequence.hh>

namespace efl { namespace eldbus { namespace _detail {

template <typename T>
struct signature_traits;

template <>
struct signature_traits<bool>
{
  typedef Eina_Bool raw_type;
  typedef bool value_type;
  typedef std::integral_constant<int, 1u> sig_size;
  static int const sig = 'b';

  static raw_type to_raw(value_type v) { return v ? EINA_TRUE : EINA_FALSE; }
};

template <>
struct signature_traits<char>
{
  typedef char raw_type;
  typedef raw_type value_type;
  typedef std::integral_constant<int, 1u> sig_size;
  static int const sig = 'y';

  static raw_type to_raw(value_type v) { return v; }
};

template <>
struct signature_traits<int16_t>
{
  typedef int16_t raw_type;
  typedef raw_type value_type;
  typedef std::integral_constant<int, 1u> sig_size;
  static char const sig = 'n';
  
  static raw_type to_raw(value_type v) { return v; }
};

template <>
struct signature_traits<uint16_t>
{
  typedef uint16_t raw_type;
  typedef raw_type value_type;
  typedef std::integral_constant<int, 1u> sig_size;
  static char const sig = 'q';

  static raw_type to_raw(value_type i) { return i; }
};

template <>
struct signature_traits<int32_t>
{
  typedef int32_t raw_type;
  typedef raw_type value_type;
  typedef std::integral_constant<int, 1u> sig_size;
  static char const sig = 'i';

  static raw_type to_raw(value_type i) { return i; }
};

template <>
struct signature_traits<uint32_t>
{
  typedef uint32_t raw_type;
  typedef raw_type value_type;
  typedef std::integral_constant<int, 1u> sig_size;
  static char const sig = 'u';

  static raw_type to_raw(value_type i) { return i; }
};

template <>
struct signature_traits<int64_t>
{
  typedef int64_t raw_type;
  typedef raw_type value_type;
  typedef std::integral_constant<int, 1u> sig_size;
  static char const sig = 'x';

  static raw_type to_raw(value_type i) { return i; }
};

template <>
struct signature_traits<uint64_t>
{
  typedef uint64_t raw_type;
  typedef raw_type value_type;
  typedef std::integral_constant<int, 1u> sig_size;
  static char const sig = 't';

  static raw_type to_raw(value_type i) { return i; }
};

template <>
struct signature_traits<double>
{
  typedef double raw_type;
  typedef raw_type value_type;
  typedef std::integral_constant<int, 1u> sig_size;
  static char const sig = 'd';

  static raw_type to_raw(value_type i) { return i; }
};

template <>
struct signature_traits<std::string>
{
  typedef const char* raw_type;
  typedef std::string value_type;
  typedef std::integral_constant<int, 1u> sig_size;
  static char const sig = 's';

  static raw_type to_raw(std::string const& s) { return s.c_str(); }
};

template <typename T>
struct signature_traits<T const> : signature_traits<T>
{
};

template <typename T>
typename signature_traits<T>::raw_type to_raw(T const& object)
{
  return signature_traits<T>::to_raw(object);
}

constexpr std::size_t add(std::size_t N)
{
  return N;
}

constexpr std::size_t add(std::size_t L, std::size_t R)
{
  return L + R;
}

template <typename... T>
constexpr std::size_t add(std::size_t L, std::size_t R, T ... O)
{
  return L + R + add(O...);
}

template <typename T, typename U>
struct signature_size_impl;

template <typename T, std::size_t... S>
struct signature_size_impl<T, eina::index_sequence<S...> >
  : std::integral_constant
    <std::size_t
     , _detail::add
     (signature_traits<typename std::tuple_element<S, T>::type>::sig_size::value...)>
{
  
};

template <typename T>
struct signature_size : signature_size_impl<T, eina::make_index_sequence<std::tuple_size<T>::value> >
{
  
};

template <typename ... T>
void call_all(T...) {}

template <std::size_t I, typename Tuple, std::size_t N
          , typename = typename std::enable_if<I != N-1>::type>
int init_signature(char (&signature)[N])
{
  signature[I] = signature_traits<typename std::tuple_element<I, Tuple>::type>::sig;
  return 0;
}

template <std::size_t I, typename Tuple>
int init_signature(char (&signature)[I+1])
{
  signature[I] = 0;
  return 0;
}

template <typename... Args, std::size_t... S, std::size_t N>
void init_signature_array(char (&signature)[N], eina::index_sequence<S...>)
{
  call_all(_detail::init_signature<S, std::tuple<Args...> >(signature)...);
}

} } }

#endif
