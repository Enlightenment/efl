#ifndef _EFL_EINA_EINA_VALUE_HH
#define _EFL_EINA_EINA_VALUE_HH

#include <Eina.h>

#include <eina_stringshare.hh>
#include <eina_type_traits.hh>

/**
 * @addtogroup Eina_Cxx_Data_Types_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Value_Group Generic Value Storage
 * @ingroup Eina_Cxx_Data_Types_Group
 *
 * Abstracts generic data storage and access to it in an extensible
 * and efficient way.
 *
 * It is meant for simple data types, providing uniform access, useful
 * to exchange data preserving their types.
 *
 * @{
 */

template <typename T, typename Enable = void>
struct _eina_value_traits;

template <typename T>
struct _eina_value_traits_base;

template <typename T>
struct _eina_value_traits_aux;

/**
 * @internal
 */
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

// Indirection for uint64_t. uint64_t can be a typedef for unsigned
// long, so we can't specialize on the same template
/**
 * @internal
 */
template <>
struct _eina_value_traits_aux<uint64_t>
  : _eina_value_traits_base<uint64_t>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_UINT64;
  }
};

/**
 * @internal
 */
template <typename T, typename Enable>
struct _eina_value_traits : _eina_value_traits_aux<T>
{
};

/**
 * @internal
 */
template <>
struct _eina_value_traits<unsigned char>
  : _eina_value_traits_base<unsigned char>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_UCHAR;
  }
};

/**
 * @internal
 */
template <>
struct _eina_value_traits<unsigned short>
  : _eina_value_traits_base<unsigned short>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_USHORT;
  }
};

/**
 * @internal
 */
template <>
struct _eina_value_traits<unsigned int>
  : _eina_value_traits_base<unsigned int>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_UINT;
  }
};

/**
 * @internal
 */
template <>
struct _eina_value_traits<unsigned long>
  : _eina_value_traits_base<unsigned long>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_ULONG;
  }
};

/**
 * @internal
 */
template <>
struct _eina_value_traits<char>
  : _eina_value_traits_base<char>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_CHAR;
  }
};

/**
 * @internal
 */
template <>
struct _eina_value_traits<short>
  : _eina_value_traits_base<short>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_SHORT;
  }
};

/**
 * @internal
 */
template <>
struct _eina_value_traits<int>
  : _eina_value_traits_base<int>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_INT;
  }
};

/**
 * @internal
 */
template <>
struct _eina_value_traits<long>
  : _eina_value_traits_base<long>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_LONG;
  }
};

/**
 * @internal
 */
template <>
struct _eina_value_traits<float>
  : _eina_value_traits_base<float>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_FLOAT;
  }
};

/**
 * @internal
 */
template <>
struct _eina_value_traits<double>
  : _eina_value_traits_base<double>
{
  static ::Eina_Value_Type const* value_type()
  {
    return EINA_VALUE_TYPE_DOUBLE;
  }
};

/**
 * @internal
 */
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

/**
 * @internal
 */
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

/**
 * @internal
 */
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

class value;

template <typename T>
T get(value const& v);

/**
 * Store generic value
 */
class value
{
  /**
   * @brief Initialize the <tt>eina::value</tt> with the given argument.
   * @param v Argument that the <tt>eina::value</tt> will store.
   */
  template <typename T>
  void primitive_init(T v)
  {
    _raw = _eina_value_traits<T>::create();
    _eina_value_traits<T>::set(_raw, v);
  }
public:

  /**
   * @brief Default constructor. Create an empty generic value storage.
   */
  value()
    : _raw(_eina_value_traits<char>::create())
  {
  }

  /**
   * @brief Create an generic value storage holding the given argument.
   * @param v Value to be stored.
   */
  template <typename T>
  value(T v)
  {
    primitive_init(v);
  }

  /**
   * @brief Create an generic value storage holding a @c char value.
   * @param v @c char value to be stored.
   */
  value(char v)
  {
    primitive_init(v);
  }

  /**
   * @brief Create an generic value storage holding a @c short value.
   * @param v @c short value to be stored.
   */
  value(short v)
  {
    primitive_init(v);
  }

  /**
   * @brief Create an generic value storage holding a @c int value.
   * @param v @c int value to be stored.
   */
  value(int v)
  {
    primitive_init(v);
  }

  /**
   * @brief Create an generic value storage holding a @c long value.
   * @param v @c long value to be stored.
   */
  value(long v)
  {
    primitive_init(v);
  }

  /**
   * @brief Create an generic value storage holding a <tt>unsigned char</tt> value.
   * @param v <tt>unsigned char</tt> value to be stored.
   */
  value(unsigned char v)
  {
    primitive_init(v);
  }

  /**
   * @brief Create an generic value storage holding a <tt>unsigned short</tt> value.
   * @param v <tt>unsigned short</tt> value to be stored.
   */
  value(unsigned short v)
  {
    primitive_init(v);
  }

  /**
   * @brief Create an generic value storage holding a <tt>unsigned int</tt> value.
   * @param v <tt>unsigned int</tt> value to be stored.
   */
  value(unsigned int v)
  {
    primitive_init(v);
  }

  /**
   * @brief Create an generic value storage holding a <tt>unsigned long</tt> value.
   * @param v <tt>unsigned long</tt> value to be stored.
   */
  value(unsigned long v)
  {
    primitive_init(v);
  }

  /**
   * @brief Create an generic value storage holding a @c float value.
   * @param v @c float value to be stored.
   */
  value(float v)
  {
    primitive_init(v);
  }

  /**
   * @brief Create an generic value storage holding a @c double value.
   * @param v @c double value to be stored.
   */
  value(double v)
  {
    primitive_init(v);
  }

  /**
   * @brief Deallocate stored value.
   */
  ~value()
  {
    eina_value_free(_raw);
  }

  /**
   * @brief Copy Constructor. Create an generic value storage holding the same value of @p other.
   * @param other Another <tt>eina::value</tt> object.
   */
  value(value const& other)
    : _raw(_eina_value_traits<char>::create())
  {
    if(!eina_value_copy(const_cast<Eina_Value const*>(other._raw), _raw))
      throw eina::system_error(eina::get_error_code());
  }

  /**
   * @brief Assignment operator. Replace the current stored value by the value in @p other.
   * @param other Another <tt>eina::value</tt> object.
   */
  value& operator=(value const& other)
  {
    eina_value_flush(_raw);
    if(!eina_value_copy(const_cast<Eina_Value const*>(other._raw), _raw))
      throw eina::system_error(eina::get_error_code());
    return *this;
  }

  /**
   * @brief Swap stored values with the given <tt>eina::value</tt> object.
   * @param other Another <tt>eina::value</tt> object.
   */
  void swap(value& other)
  {
    std::swap(_raw, other._raw);
  }

  /**
   * @brief Get a handle for the wrapped @c Eina_Value.
   * @return Handle for the native @c Eina_Value.
   *
   * This member function returns the native @c Eina_Value handle that
   * is wrapped inside this object.
   *
   * @warning It is important to take care when using it, since the
   * handle will be automatically released upon object destruction.
   */
  typedef Eina_Value* native_handle_type;
  native_handle_type native_handle() const
  {
    return _raw;
  }

  /**
   * Type for a constant pointer to an @c Eina_Value_Type.
   * Describes the type of the data being stored.
   */
  typedef Eina_Value_Type const* type_info_t;

  /**
   * @brief Get an identifier for the type of the value currently stored.
   * @return @c Eina_Value_Type instance or @c NULL if type is invalid.
   */
  type_info_t type_info() const
  {
    return ::eina_value_type_get(_raw);
  }
private:
  ::Eina_Value* _raw;

  /**
   * @brief Get the data stored in the given <tt>eina::value</tt>.
   * @param v <tt>eina::value</tt> object.
   * @param T Type of the value stored.
   * @return Copy of the value stored in @p v.
   * @throw <tt>eina::system_error</tt> with error the code
   *        @c EINA_ERROR_VALUE_FAILED if @p T doesn't match the type of
   *        the value currently stored. Or <tt>eina::system_error</tt>
   *        with an internal Eina error code if the operation fails for
   *        another reason.
   *
   * This function returns the value stored in @p v. The type of the
   * value must be specified via the template parameter @p T, and must
   * match the current stored value type.
   */
  template <typename T>
  friend T get(value const& v)
  {
    return _eina_value_traits<T>::get(v._raw);
  }
};

/**
 * @brief Swap the stored values between the given <tt>eina::value</tt> objects.
 * @param lhs First <tt>eina::value</tt> object.
 * @param rhs Second <tt>eina::value</tt> object.
 */
inline void swap(value& lhs, value& rhs)
{
  lhs.swap(rhs);
}

/**
 * @brief Compare if the stored values are equal.
 * @param lhs <tt>eina::value</tt> object at the left side of the expression.
 * @param rhs <tt>eina::value</tt> object at the right side of the expression.
 * @return @c true if the stored values are of the same type and equals
 *         in content, @c false otherwise.
 */
inline bool operator==(value const& lhs, value const& rhs)
{
  return lhs.type_info() == rhs.type_info()
    && eina_value_compare(lhs.native_handle(), rhs.native_handle()) == 0;
}

/**
 * @brief Less than comparison between two <tt>eina::value</tt> objects.
 * @param lhs <tt>eina::value</tt> object at the left side of the expression.
 * @param rhs <tt>eina::value</tt> object at the right side of the expression.
 * @return For objects holding values of the same type, returns @c true
 *         if @p lhs value is less than @p rhs value. For objects
 *         holding values of different types, returns @c true if the
 *         type identifier of @p lhs comes before the type indentifier
 *         of @p rhs. Returns @c false in all other cases.
 */
inline bool operator<(value const& lhs, value const& rhs)
{
  return std::less<Eina_Value_Type const*>()(lhs.type_info(), rhs.type_info())
    || (lhs.type_info() == rhs.type_info()
        && eina_value_compare(lhs.native_handle(), rhs.native_handle()) < 0);
}

/**
 * @brief More than comparison between two <tt>eina::value</tt> objects.
 * @param lhs <tt>eina::value</tt> object at the left side of the expression.
 * @param rhs <tt>eina::value</tt> object at the right side of the expression.
 * @return For objects holding values of the same type, returns @c true
 *         if @p lhs value is more than @p rhs value. For objects
 *         holding values of different types, returns @c true if the
 *         type identifier of @p lhs comes after the type indentifier
 *         of @p rhs. Returns @c false in all other cases.
 */
inline bool operator>(value const& lhs, value const& rhs)
{
  return std::less<Eina_Value_Type const*>()(rhs.type_info(), lhs.type_info())
    || (rhs.type_info() == lhs.type_info()
        && eina_value_compare(lhs.native_handle(), rhs.native_handle()) > 0);
}

/**
 * @brief Less than or equal comparison between two <tt>eina::value</tt> objects.
 * @param lhs <tt>eina::value</tt> object at the left side of the expression.
 * @param rhs <tt>eina::value</tt> object at the right side of the expression.
 * @return For objects holding values of the same type, returns @c true
 *         if @p lhs value is less than or equal to @p rhs value. For
 *         objects holding values of different types, returns @c true if
 *         the type identifier of @p lhs comes before the type
 *         indentifier of @p rhs. Returns @c false in all other cases.
 */
inline bool operator<=(value const& lhs, value const& rhs)
{
  return !(lhs > rhs);
}

/**
 * @brief More than or equal comparison between two <tt>eina::value</tt> objects.
 * @param lhs <tt>eina::value</tt> object at the left side of the expression.
 * @param rhs <tt>eina::value</tt> object at the right side of the expression.
 * @return For objects holding values of the same type, returns @c true
 *         if @p lhs value is more than or equal to @p rhs value. For
 *         objects holding values of different types, returns @c true if
 *         the type identifier of @p lhs comes after the type
 *         indentifier of @p rhs. Returns @c false in all other cases.
 */
inline bool operator>=(value const& lhs, value const& rhs)
{
  return !(lhs < rhs);
}

/**
 * @brief Compare if the stored values are different.
 * @param lhs <tt>eina::value</tt> object at the left side of the expression.
 * @param rhs <tt>eina::value</tt> object at the right side of the expression.
 * @return @c true if the value types are different or if the value of
 *         @p lhs is different from the value of @rhs, @c false
 *         otherwise.
 */
inline bool operator!=(value const& lhs, value const& rhs)
{
  return !(lhs == rhs);
}

/**
 * @}
 */

} }

/**
 * @}
 */

#endif
