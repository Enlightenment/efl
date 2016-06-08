#ifndef EINA_DELETERS_HH_
#define EINA_DELETERS_HH_

#include <eina_workarounds.hh>

namespace efl { namespace eina {

struct malloc_deleter
{
  template <typename T>
  void operator()(T* object) const
  {
    object->~T();
    free(object);
  }
};

template <typename T>
using unique_malloc_ptr = std::unique_ptr<T, malloc_deleter>;
    
} }

#endif
