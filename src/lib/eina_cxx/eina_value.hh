#ifndef _EFL_EINA_EINA_VALUE_HH
#define _EFL_EINA_EINA_VALUE_HH

#include <Eina.h>

#include <eina_stringshare.hh>
#include <eina_type_traits.hh>

namespace efl { namespace eina {

template <typename T>
struct _eina_value_traits_base;

template <typename T>
struct _eina_value_traits_aux;

// Indirection for uint64_t. uint64_t can be a typedef for unsigned
// long, so we can't specialize on the same template
template <>
struct _eina_value_traits_aux<uint64_t>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_UINT64;
  }
};

template <typename T, typename Enable = void>
struct _eina_value_traits : _eina_value_traits_aux<T>
{
};

template <typename T>
struct _eina_value_traits_base
{
  typedef T type;

  static ::Eina_Value* create()
  {
    return eina_value_new(_eina_value_traits<T>::value_type());
  }
  static void set_type( ::Eina_Value* v)
  {
    eina_value_setup(v, _eina_value_traits<T>::value_type());
  }
  static void set( ::Eina_Value* v, type c)
  {
    ::eina_value_set(v, c);
  }
  static type get( ::Eina_Value* v)
  {
    if(_eina_value_traits<T>::value_type() == eina_value_type_get(v))
      {
        type vv;
        if(::eina_value_get(v, &vv))
          return vv;
        else
          throw eina::system_error(eina::get_error_code());
      }
    else
      throw eina::system_error(EINA_ERROR_VALUE_FAILED, eina::eina_error_category());
  }
};

template <>
struct _eina_value_traits<unsigned char>
  : _eina_value_traits_base<unsigned char>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_UCHAR;
  }
};

template <>
struct _eina_value_traits<unsigned short>
  : _eina_value_traits_base<unsigned short>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_USHORT;
  }
};

template <>
struct _eina_value_traits<unsigned int>
  : _eina_value_traits_base<unsigned int>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_UINT;
  }
};

template <>
struct _eina_value_traits<unsigned long>
  : _eina_value_traits_base<unsigned long>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_ULONG;
  }
};

template <>
struct _eina_value_traits<char>
  : _eina_value_traits_base<char>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_CHAR;
  }
};

template <>
struct _eina_value_traits<short>
  : _eina_value_traits_base<short>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_SHORT;
  }
};

template <>
struct _eina_value_traits<int>
  : _eina_value_traits_base<int>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_INT;
  }
};

template <>
struct _eina_value_traits<long>
  : _eina_value_traits_base<long>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_LONG;
  }
};

template <>
struct _eina_value_traits<float>
  : _eina_value_traits_base<float>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_FLOAT;
  }
};

template <>
struct _eina_value_traits<double>
  : _eina_value_traits_base<double>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_DOUBLE;
  }
};

template <>
struct _eina_value_traits<stringshare>
  : _eina_value_traits_base<stringshare>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_STRINGSHARE;
  }
  static void set( ::Eina_Value* v, type c)
  {
    ::eina_value_set(v, c.data());
  }
  static type get( ::Eina_Value* v)
  {
    char* c_str;
    ::eina_value_get(v, &c_str);
    return stringshare(c_str, steal_stringshare_ref);
  }
};

template <>
struct _eina_value_traits<std::string>
  : _eina_value_traits_base<std::string>
{
  typedef typename  _eina_value_traits_base<std::string>::type type;
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_STRING;
  }
  static void set( ::Eina_Value* v, type c)
  {
    ::eina_value_set(v, c.c_str());
  }
  static type get( ::Eina_Value* v)
  {
    char* c_str;
    ::eina_value_get(v, &c_str);
    std::string r(c_str);
    ::free(c_str);
    return r;
  }
};

template <typename T>
struct _eina_value_traits<T[], typename eina::enable_if<eina::is_pod<T>::value>::type>
  : _eina_value_traits_base<T[]>
{
  typedef typename  _eina_value_traits_base<T[]>::type type;
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_ARRAY;
  }
  static void set( ::Eina_Value* v, type c)
  {
    ::eina_value_set(v, c.c_str());
  }
  static type get( ::Eina_Value* v)
  {
    char* c_str;
    ::eina_value_get(v, &c_str);
    std::string r(c_str);
    ::free(c_str);
    return r;
  }
};

class eina_value
{
  template <typename T>
  void primitive_init(T v)
  {
    _raw = _eina_value_traits<T>::create();
    _eina_value_traits<T>::set(_raw, v);
  }
public:
  eina_value()
    : _raw(_eina_value_traits<char>::create())
  {
  }
  eina_value(char v)
  {
    primitive_init(v);
  }
  eina_value(short v)
  {
    primitive_init(v);
  }
  eina_value(int v)
  {
    primitive_init(v);
  }
  eina_value(long v)
  {
    primitive_init(v);
  }
  eina_value(unsigned char v)
  {
    primitive_init(v);
  }
  eina_value(unsigned short v)
  {
    primitive_init(v);
  }
  eina_value(unsigned int v)
  {
    primitive_init(v);
  }
  eina_value(unsigned long v)
  {
    primitive_init(v);
  }
  eina_value(float v)
  {
    primitive_init(v);
  }
  eina_value(double v)
  {
    primitive_init(v);
  }

  ~eina_value()
  {
    eina_value_free(_raw);
  }

  eina_value(eina_value const& other)
    : _raw(_eina_value_traits<char>::create())
  {
    if(!eina_value_copy(const_cast<Eina_Value const*>(other._raw), _raw))
      throw eina::system_error(eina::get_error_code());
  }
  eina_value& operator=(eina_value const& other)
  {
    eina_value_flush(_raw);
    if(!eina_value_copy(const_cast<Eina_Value const*>(other._raw), _raw))
      throw eina::system_error(eina::get_error_code());
    return *this;
  }

  void swap(eina_value& other)
  {
    std::swap(_raw, other._raw);
  }

  typedef Eina_Value* native_handle_type;
  native_handle_type native_handle() const
  {
    return _raw;
  }
  typedef Eina_Value_Type const* type_info_t;
  type_info_t type_info() const
  {
    return ::eina_value_type_get(_raw);
  }
private:
  ::Eina_Value* _raw;

  template <typename T>
  friend T get(eina_value const& v)
  {
    return _eina_value_traits<T>::get(v._raw);
  }
};

inline void swap(eina_value& lhs, eina_value& rhs)
{
  lhs.swap(rhs);
}

inline bool operator==(eina_value const& lhs, eina_value const& rhs)
{
  return lhs.type_info() == rhs.type_info()
    && eina_value_compare(lhs.native_handle(), rhs.native_handle()) == 0;
}

inline bool operator<(eina_value const& lhs, eina_value const& rhs)
{
  return std::less<Eina_Value_Type const*>()(lhs.type_info(), rhs.type_info())
    || (lhs.type_info() == rhs.type_info()
        && eina_value_compare(lhs.native_handle(), rhs.native_handle()) < 0);
}

inline bool operator>(eina_value const& lhs, eina_value const& rhs)
{
  return std::less<Eina_Value_Type const*>()(rhs.type_info(), lhs.type_info())
    || (rhs.type_info() == lhs.type_info()
        && eina_value_compare(lhs.native_handle(), rhs.native_handle()) > 0);
}

inline bool operator<=(eina_value const& lhs, eina_value const& rhs)
{
  return !(lhs > rhs);
}

inline bool operator>=(eina_value const& lhs, eina_value const& rhs)
{
  return !(lhs < rhs);
}

inline bool operator!=(eina_value const& lhs, eina_value const& rhs)
{
  return !(lhs == rhs);
}

} }

#endif
