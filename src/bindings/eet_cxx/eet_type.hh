#ifndef _EET_TYPE_HH
#define _EET_TYPE_HH

#include <Eet.h>
#include <Eina.hh>

#include <type_traits>

namespace efl { namespace eet {

template <typename T>
struct _eet_type;

template <>
struct _eet_type<char> : std::integral_constant<int, EET_T_CHAR>
{};

template <>
struct _eet_type<short> : std::integral_constant<int, EET_T_SHORT>
{};

template <>
struct _eet_type<int> : std::integral_constant<int, EET_T_INT>
{};

template <>
struct _eet_type<long long> : std::integral_constant<int, EET_T_LONG_LONG>
{};

template <>
struct _eet_type<float> : std::integral_constant<int, EET_T_FLOAT>
{};

template <>
struct _eet_type<double> : std::integral_constant<int, EET_T_DOUBLE>
{};

template <>
struct _eet_type<unsigned char> : std::integral_constant<int, EET_T_UCHAR>
{};

template <>
struct _eet_type<unsigned short> : std::integral_constant<int, EET_T_USHORT>
{};

template <>
struct _eet_type<unsigned int> : std::integral_constant<int, EET_T_UINT>
{};

template <>
struct _eet_type<unsigned long long> : std::integral_constant<int, EET_T_ULONG_LONG>
{};

template <>
struct _eet_type<char*> : std::integral_constant<int, EET_T_STRING>
{};

template <>
struct _eet_type<void*> : std::integral_constant<int, EET_T_NULL>
{};

template <>
struct _eet_type<eina::value> : std::integral_constant<int, EET_T_VALUE>
{};

template <typename T>
struct _void { typedef void type; };

template <typename T, typename Enabler = void>
struct is_eet_primitive : std::false_type {};

template <typename T>
struct is_eet_primitive<T, typename _void<typename _eet_type<T>::type>::type>
  : std::true_type {};

} }

#endif
