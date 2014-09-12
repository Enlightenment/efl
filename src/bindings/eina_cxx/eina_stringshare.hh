#ifndef _EINA_STRINGSHARE_HH
#define _EINA_STRINGSHARE_HH

#include <Eina.h>
#include <eina_type_traits.hh>

#include <cstring>
#include <stdexcept>

/**
 * @addtogroup Eina_Cxx_Data_Types_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Stringshare_Group Stringshare
 * @ingroup Eina_Cxx_Data_Types_Group
 *
 * C++ Binding to Eina_Stringshare.
 *
 * This class allows you to store a single copy of a string, and use in
 * multiple places throughout your program.
 *
 * This is a method to reduce the number of duplicated strings kept in
 * memory. It's pretty common for the same strings to be dynamically
 * allocated repeatedly between applications and libraries, especially in
 * circumstances where you could have multiple copies of a structure that
 * allocates the string. So rather than duplicating and freeing these
 * strings, you request a read-only pointer to an existing string and
 * only incur the overhead of a hash look-up.
 *
 * It sounds like micro-optimizing, but profiling has shown this can have
 * a significant impact as you scale the number of copies up. It improves
 * string creation/destruction speed, reduces memory use and decreases
 * memory fragmentation, so a win all-around.
 *
 * @{
 */

/**
 * Type for stealing the ownership of a string that was previously shared.
 */
struct steal_stringshare_ref_t {};

/**
 * Constant instance of @c steal_stringshare_ref_t for quick reference.
 */
steal_stringshare_ref_t const steal_stringshare_ref = {};

/**
 * Stringshare class. It provides an OOP interface to the
 * @c Eina_Stringshare functions, and automatically take care of sharing
 * the string upon construction and deleting it upon destruction using
 * the RAII programming idiom.
 *
 * It also provides additional member functions to facilitate the access
 * to the string content, much like a STL string.
 */
struct stringshare
{
  typedef char value_type;
  typedef value_type& reference;
  typedef value_type const& const_reference;
  typedef value_type* pointer;
  typedef value_type const* const_pointer;
  typedef const_pointer const_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::ptrdiff_t difference_type;
  typedef std::size_t size_type;

  /**
   * @brief Default constructor. Creates a new object holding an shared empty string.
   * @see stringshare(const char* str)
   */
  stringshare()
    : _string( ::eina_stringshare_add("") )
  {}

  /**
   * @brief Share an instance of the given string wrapped by the newly created @c stringshare object.
   * @param <tt>NULL</tt>-terminated string to be shared.
   *
   * This constructor creates an <tt>eina::stringshare</tt> object that
   * shares the given string and wraps the shared pointer, providing an
   * OOP interface to the string content. If the string is already
   * shared this constructor simple increment its reference counter and
   * wraps the shared pointer.
   *
   * @see stringshare(char* str, steal_stringshare_ref_t)
   */
  stringshare(const char* str)
    : _string( ::eina_stringshare_add(str) )
  {
  }

  /**
   * @brief Create an <tt>eina::stringshare</tt> that steal the ownership of the given shared string.
   * @param str Shared string whose ownership should be stolen.
   *
   * This constructor creates an <tt>eina::stringshare</tt> object that
   * steals the ownership of the given shared string. At destruction
   * time, the reference counter for the shared string will be
   * decremented.
   *
   * The second parameter is an empty object of a specific type that
   * should be supplied to explicitly inform that this is the intended
   * constructor; and to differentiate this from
   * @ref stringshare(const char* str).
   *
   * @warning @p str should be a string that was previously shared (most
   * likely by an call to the native @c eina_stringshare_add function).
   * If the string is not shared, upon destruction time bad things will
   * happen, likely a segmentation fault.
   *
   * @see stringshare(const char* str)
   */
  stringshare(char* str, steal_stringshare_ref_t)
    : _string( str )
  {
  }

  /**
   * @brief Share the string between the iterator.
   * @param i Iterator to the initial position of the string (inclusive).
   * @param j Iterator to the final position of the string (exclusive).
   * @note The ending position (pointed by @p j) is not considered.
   * @see stringshare(const char* str)
   */
  template <typename InputIterator>
  stringshare(InputIterator i, InputIterator j
              , typename eina::enable_if
              <!eina::is_integral<InputIterator>::value
              && !eina::is_contiguous_iterator<InputIterator>::value
              >::type* = 0)
  {
    std::string tmp;
    while(i != j)
      {
        tmp.push_back(*i);
        ++i;
      }
    _string = ::eina_stringshare_add(tmp.c_str());
  }

  /**
   * @brief Share the string between the iterator.
   * @param i Iterator to the initial position of the string (inclusive).
   * @param j Iterator to the final position of the string (exclusive).
   * @note The ending position (pointed by @p j) is not considered.
   * @see stringshare(const char* str)
   */
  template <typename ContiguousMemoryIterator>
  stringshare(ContiguousMemoryIterator i, ContiguousMemoryIterator j
              , typename eina::enable_if
              <eina::is_contiguous_iterator<ContiguousMemoryIterator>::value>::type* = 0)
    : _string( ::eina_stringshare_add_length(&*i, j - i) )
  {
  }

  /**
   * @brief Destructor. Delete the shared string.
   *
   * Decreases the reference counter associated with the shared string.
   * If the reference counter reaches 0, the memory associated with the
   * string is freed.
   */
  ~stringshare()
  {
    ::eina_stringshare_del(_string);
  }

  /**
   * @brief Copy constructor. Creates a new <tt>eina::stringshare</tt> associated with the same shared string.
   * @param other Another <tt>eina::stringshare</tt>.
   *
   * This constructor increments the reference counter to the shared
   * string associated with @p other.
   *
   * @see stringshare(const char* str)
   */
  stringshare(stringshare const& other)
    : _string( eina_stringshare_ref(other._string) )
  {}

  /**
   * @brief Replace the current shared string.
   * @param other Another <tt>eina::stringshare</tt>.
   *
   * This operator replaces the current shared string by the string
   * shared by @p other. The reference counter of the older shared
   * string is decremented (the string is released if needed) and the
   * reference counter of the given shared string is incremented.
   */
  stringshare& operator=(stringshare const& other)
  {
    ::eina_stringshare_refplace(&_string, other._string);
    return *this;
  }

  /**
   * @brief Replace the current shared string.
   * @param c_string <tt>NULL</tt>-terminated string.
   *
   * This operator replaces the shared string currently associated with
   * this object by a shared instance of @p c_string.
   *
   * @see stringshare(const char* str)
   */
  stringshare& operator=(const char* c_string)
  {
    ::eina_stringshare_replace(&_string, c_string);
    return *this;
  }

  /**
   * @brief Get a constant iterator pointing to the first character of the string.
   * @return Constant iterator to the initial position of the string.
   *
   * This member function returns a constant iterator pointing to the
   * first character of the string. If the string is empty the iterator
   * is equal to the one returned by @ref end() const.
   */
  const_iterator begin() const
  {
    return _string;
  }

  /**
   * @brief Get a constant iterator to the position following the last character of the string.
   * @return Constant iterator to the final position of the string.
   *
   * This member function returns an constant iterator to the position
   * following the last character in the string. If the string is empty
   * the iterator is equal to the one returned by @ref begin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  const_iterator end() const
  {
    return _string + size();
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the string.
   * @return Constant reverse iterator pointing to the reverse begin of the string.
   *
   * This member function returns a constant reverse iterator pointing
   * to the last character of the string. If the string is empty the
   * returned reverse iterator is the same as the one returned by
   * @ref rend() const.
   */
  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator(end());
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the string.
   * @return Constant reverse iterator pointing to the reverse end of the string.
   *
   * This member function returns a constant reverse iterator pointing
   * to the position before the first character of the string. If the
   * string is empty the returned iterator is the same as the one
   * returned by @ref rbegin() const.
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  const_reverse_iterator rend() const
  {
    return const_reverse_iterator(begin());
  }

  /**
   * @brief Get a constant iterator pointing to the first character of the string.
   * @return Constant iterator to the initial position of the string.
   *
   * This member function works just like @ref begin() const. But it is
   * granted to always return a constant iterator.
   */
  const_iterator cbegin() const
  {
    return begin();
  }

  /**
   * @brief Get a constant iterator to the position following the last character of the string.
   * @return Constant iterator to the final position of the string.
   *
   * This member function works just like @ref end() const. But it is
   * granted to always return a constant iterator.
   */
  const_iterator cend() const
  {
    return end();
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the string.
   * @return Constant reverse iterator pointing to the reverse begin of the string.
   *
   * This member function works just like @ref rbegin() const. But it is
   * granted to always return a constant reverse iterator.
   */
  const_reverse_iterator crbegin() const
  {
    return rbegin();
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the string.
   * @return Constant reverse iterator pointing to the reverse end of the string.
   *
   * This member function works just like @ref rend() const. But it is
   * granted to always return a constant reverse iterator.
   */
  const_reverse_iterator crend() const
  {
    return rend();
  }

  /**
   * @brief Get the size of the string.
   * @return Number of characters in the string.
   */
  size_type size() const
  {
    return eina_stringshare_strlen(_string);
  }

  /**
   * @brief Alias to @ref size() const.
   */
  size_type length() const
  {
    return size();
  }

  /**
   * @brief Get the maximum number of characters a string can hold.
   * @return Maximum number of characters a string can hold.
   */
  size_type max_size() const
  {
    return -1;
  }

  /**
   * @brief Check if the string has no characters.
   * @return @c true if the string has no characters, @c false otherwise.
   */
  bool empty() const
  {
    return _string[0] == 0;
  }

  /**
   * @brief Get the character at the given position.
   * @param i Position of the character in the string.
   * @return Constant reference to the character at the given position.
   * @note Do not check if the given position exceeds the string size.
   */
  const_reference operator[](size_type i) const
  {
    return _string[i];
  }

  /**
   * @brief Get the character at the given position.
   * @param i Position of the character in the string.
   * @return Constant reference to the character at the given position.
   * @throw <tt>std::out_of_range</tt> if the given position exceeds the string size.
   *
   * This member function returns a constant reference to the character
   * at the position @p i. If @p i exceeds the string size this function
   * will throw a <tt>std::out_of_range</tt>.
   */
  const_reference at(size_type i) const
  {
    if(i < size())
      return (*this)[i];
    else
       EFL_CXX_THROW(std::out_of_range(""));
  }

  /**
   * @brief Get the last character of the string.
   * @return Constant reference to the last character of the string.
   */
  const_reference back() const
  {
    return _string[size()-1];
  }

  /**
   * @brief Get the first character of the string.
   * @return Constant reference to the first character of the string.
   */
  const_reference front() const
  {
    return _string[0];
  }

  /**
   * @brief Swap shared strings with other <tt>eina::stringshare</tt>.
   */
  void swap(stringshare& other)
  {
    std::swap(_string, other._string);
  }

  /**
   * @brief Get the c-like shared string currently associated with the object.
   * @return Pointer to the shared string.
   * @note The pointer returned may be invalidated by calls to non-const member functions.
   */
  const char* c_str() const
  {
    return _string;
  }

  /**
   * @brief Alias to @ref c_str() const.
   */
  const char* data() const
  {
    return _string;
  }

private:
  /**
   * @internal
   */
  Eina_Stringshare* _string;
};

/**
 * Specialization of the default template to define the
 * <tt>stringshare::const_iterator</tt> as a contiguous iterator.
 */
template <>
struct is_contiguous_iterator<stringshare::const_iterator> : true_type {};

/**
 * @brief Check if two <tt>eina::stringshare</tt> objects represent the same string.
 * @return @c true if the strings of the objects are equal, @c false otherwise.
 *
 * This operator checks if two <tt>eina::stringshare</tt> objects
 * represent the same string. Because of the nature of the objects,
 * this operation falls into a simple pointer comparison, since
 * identical strings are represented by the same instance.
 */
inline bool operator==(stringshare const& lhs, stringshare const& rhs)
{
  return lhs.c_str() == rhs.c_str();
}

/**
 * @brief Check if two <tt>eina::stringshare</tt> objects represent different strings.
 * @return @c true if the strings of the objects are different, @c false otherwise.
 *
 * This function essentially returns the opposite of
 * @ref operator==(stringshare const& lhs, stringshare const& rhs).
 */
inline bool operator!=(stringshare const& lhs, stringshare const& rhs)
{
  return !(lhs == rhs);
}

/**
 * @brief Compare an <tt>eina::stringshare</tt> object with a c-like string.
 * @return @c true if the content of the <tt>eina::stringshare</tt>
 *         string is equal the content of the given string, @c false
 *         otherwise.
 */
inline bool operator==(stringshare const& lhs, const char* rhs)
{
  return lhs.c_str() == rhs || std::strcmp(lhs.c_str(), rhs) == 0;
}

/**
 * @brief Compare an <tt>eina::stringshare</tt> object with a c-like string.
 * @return @c true if the content of the <tt>eina::stringshare</tt>
 *         string is different from content of the given string,
 *         @c false otherwise.
 */
inline bool operator!=(stringshare const& lhs, const char* rhs)
{
  return !(lhs == rhs);
}

/**
 * @brief Compare an <tt>eina::stringshare</tt> object with a c-like string.
 * @return @c true if the content of the <tt>eina::stringshare</tt>
 *         string is equal the content of the given string, @c false
 *         otherwise.
 */
inline bool operator==(const char* lhs, stringshare const& rhs)
{
  return rhs == lhs;
}

/**
 * @brief Compare an <tt>eina::stringshare</tt> object with a c-like string.
 * @return @c true if the content of the <tt>eina::stringshare</tt>
 *         string is different from content of the given string,
 *         @c false otherwise.
 */
inline bool operator!=(const char* lhs, stringshare const& rhs)
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
