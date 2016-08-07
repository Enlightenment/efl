///
/// @file eo_concrete.hh
///

#ifndef EFL_CXX_EINA_COPY_TRAITS_HH
#define EFL_CXX_EINA_COPY_TRAITS_HH

namespace efl { namespace eina {

template <typename T, typename Enable = void>
struct copy_from_c_traits;

template <typename T>
struct copy_from_c_traits<T, typename std::enable_if<std::is_fundamental<T>::value>::type>
{
  static void copy_to_unitialized(T* storage, void const* data)
  {
    std::memcpy(storage, data, sizeof(T));
  }
};

template <typename T, typename Enable = void>
struct alloc_to_c_traits;

template <typename T>
struct alloc_to_c_traits<T, typename std::enable_if<std::is_fundamental<T>::value>::type>
{
  typedef T c_type;
  static c_type* copy_alloc(T const& value)
  {
    c_type* v = static_cast<c_type*>(malloc(sizeof(c_type)));
    std::memcpy(v, &value, sizeof(c_type));
    return v;
  }
  static void free_alloc(void* data)
  {
    ::free(data);
  }
};
    
} }

#endif
