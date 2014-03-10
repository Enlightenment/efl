#ifndef EINA_CLONE_ALLOCATORS_HH_
#define EINA_CLONE_ALLOCATORS_HH_

#include <memory>
#include <cstring>
#include <cstdlib>
#include <type_traits>

namespace efl { namespace eina {

struct heap_copy_allocator
{
  template <typename T>
  static T* allocate_clone(T const& v)
  {
    return new T(v);
  }

  template <typename T>
  static void deallocate_clone(T* p)
  {
#ifdef EFL_EINA_CXX11
    std::default_delete<T>()(p);
#else
    delete p;
#endif
  }
};

struct heap_clone_allocator
{
  template <typename T>
  static T* allocate_clone(T const& v)
  {
    return new_clone(v);
  }
  template <typename T>
  static void deallocate_clone(T* p)
  {
    delete_clone(p);
  }
};

struct view_clone_allocator
{
  template <typename T>
  static T* allocate_clone(T const& v)
  {
    return const_cast<T*>(&v);
  }
  template <typename T>
  static void deallocate_clone(T*)
  {
  }
};

struct heap_no_copy_allocator
{
  template <typename T>
  static void deallocate_clone(T* p)
  {
#ifdef EFL_EINA_CXX11
    std::default_delete<T>()(p);
#else
    delete p;
#endif
  }
};    

struct malloc_clone_allocator
{
  template <typename T>
  static T* allocate_clone(T const& v)
  {
    static_assert(std::is_pod<T>::value, "malloc_clone_allocator can only be used with POD types");
    T* p = static_cast<T*>(std::malloc(sizeof(T)));
    std::memcpy(p, &v, sizeof(T));
    return p;
  }

  template <typename T>
  static void deallocate_clone(T const* p)
  {
    static_assert(std::is_pod<T>::value, "malloc_clone_allocator can only be used with POD types");
    std::free(const_cast<T*>(p));
  }
};

} }

#endif
