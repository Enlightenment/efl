#ifndef EINA_CLONE_ALLOCATORS_HH_
#define EINA_CLONE_ALLOCATORS_HH_

#include <memory>

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
  static void deallocate_clone(T* p)
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

} }

#endif
