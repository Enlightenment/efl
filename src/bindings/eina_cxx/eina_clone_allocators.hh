#ifndef EINA_CLONE_ALLOCATORS_HH_
#define EINA_CLONE_ALLOCATORS_HH_

#include <Eo.h>

#include <memory>
#include <cstring>
#include <cstdlib>
#include <type_traits>

/**
 * @addtogroup Eina_Cxx_Containers_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Clone_Allocators_Group Clone Allocators
 * @ingroup Eina_Cxx_Containers_Group
 *
 * Clone allocators is a formalized way to pointer containers control
 * the memory of the stored objects, allowing users to apply custom
 * allocators/deallocators for the cloned objects.
 *
 * @{
 */

/*
 * @internal
 */
struct default_clone_allocator_placeholder;
    
/**
 * This allocator creates copies of objects on the heap, calling their
 * copy constructor to make then equivalent to the given reference.
 *
 * The created objects are released with the default delete.
 */
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

/**
 * This allocator creates copies of Eo classes through eo_ref
 *
 * The created objects are released using eo_unref
 */
struct eo_clone_allocator
{
  static Eo* allocate_clone(Eo const& v)
  {
    return ::eo_ref(&v);
  }

  static void deallocate_clone(Eo const* p)
  {
    ::eo_unref(const_cast<Eo*>(p));
  }
};

/**
 * This allocator allows users to create custom allocation schemes by
 * overloading the <tt>new_clone(T const& v)</tt> and
 * <tt>delete_clone(T* p)</tt> functions.
 */
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

/**
 * This allocator does not allocate or deallocate anything. It simple
 * gets non-const-qualified pointers for objects, which allow
 * containers to hold elements without having ownership on them.
 *
 * It is commonly used to create a pointer container that is a view into
 * another existing container.
 */
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

/**
 * This allocator does not define an @c allocate_clone member function,
 * so it should be used to disable operations that require elements to
 * be cloned.
 */
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

/**
 * Manages allocation and deallocation of memory using the function
 * @c malloc and @c free. This allocator does not calls constructors,
 * the content of the newly allocated objects are assigned using
 * @c memcpy, so it has to be used with types that have
 * <em>standard-layout</em>.
 */
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

/**
 * @internal
 */
template <typename A>
struct clone_allocator_deleter
{
  template <typename T>
  void operator()(T* object) const
  {
    A::deallocate_clone(object);
  }
};

/**
 * @}
 */

} }

/**
 * @}
 */

#endif
