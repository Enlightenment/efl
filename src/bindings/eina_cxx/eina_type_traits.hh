#ifndef _EINA_TYPE_TRAITS_HH
#define _EINA_TYPE_TRAITS_HH

#include <type_traits>

#include <string>
#include <vector>

namespace efl { namespace eina {

using std::enable_if;
using std::is_integral;
using std::is_pod;
using std::is_const;
using std::remove_cv;
using std::true_type;
using std::false_type;
using std::remove_pointer;
using std::remove_reference;

template <typename T, typename Enable = void>
struct is_contiguous_iterator : false_type {};
template <>
struct is_contiguous_iterator<std::string::const_iterator> : true_type {};
template <>
struct is_contiguous_iterator<std::string::iterator> : true_type {};
template <>
struct is_contiguous_iterator<std::vector<char>::const_iterator> : true_type {};
template <>
struct is_contiguous_iterator<std::vector<char>::iterator> : true_type {};

template <bool, typename T, typename F>
struct if_c
{
  typedef T type;
};

template <typename T, typename F>
struct if_c<false, T, F>
{
  typedef F type;
};

template <typename U, typename T, typename F>
struct if_ : if_c<U::value, T, F>
{
};

} }

#endif
