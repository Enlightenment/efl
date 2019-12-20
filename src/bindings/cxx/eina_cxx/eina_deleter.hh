/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
