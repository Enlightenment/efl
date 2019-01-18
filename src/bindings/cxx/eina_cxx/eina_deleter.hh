#ifndef EINA_DELETERS_HH_
#define EINA_DELETERS_HH_

#include <eina_workarounds.hh>

#include <Eina.h>

namespace efl { namespace eina {

struct malloc_deleter
{
  template <typename T>
  void operator()(T* object) const
  {
    object->~T();
    free(object);
  }
  void operator()(Eina_Future*) const
  {
    // workaround until we substitute Efl_Future with Eina_Future
  }
  void operator()(Eina_Binbuf* /*object*/) const
  {
    // how to free binbuf?
  }
  void operator()(Eina_Promise*) const
  {
    // workaround until we manually handle efl_loop_promise_new
  }
};

template <typename T>
using unique_malloc_ptr = std::unique_ptr<T, malloc_deleter>;
    
} }

#endif
