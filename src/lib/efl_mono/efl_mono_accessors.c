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



#include "Eina.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
#  define EAPI __declspec(dllexport)
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

// This just a wrapper around carray acessors for pinned managed data
// It uses the free callback to unpin the managed data so it can be
// reclaimed by the GC back in C# world.
struct _Eina_Mono_Owned_Accessor
{
   Eina_Accessor accessor;

   Eina_Accessor *carray_acc;
   void *free_data;
   Eina_Free_Cb free_cb;
};

typedef struct _Eina_Mono_Owned_Accessor Eina_Mono_Owned_Accessor;

static Eina_Bool eina_mono_owned_carray_get_at(Eina_Mono_Owned_Accessor *accessor, unsigned int idx, void **data)
{
   return eina_accessor_data_get(accessor->carray_acc, idx, data);
}

static void** eina_mono_owned_carray_get_container(Eina_Mono_Owned_Accessor *accessor)
{
  // Is another accessor a valid container?
  return (void**)&accessor->carray_acc;
}

static void eina_mono_owned_carray_free_cb(Eina_Mono_Owned_Accessor* accessor)
{
   accessor->free_cb(accessor->free_data);

   free(accessor->carray_acc); // From Eina_CArray_Length_Accessor implementation...

   free(accessor);
}

EAPI Eina_Accessor *eina_mono_owned_carray_length_accessor_new(void** array, unsigned int step, unsigned int length, Eina_Free_Cb free_cb, void *handle)
{
   Eina_Mono_Owned_Accessor *accessor = calloc(1, sizeof(Eina_Mono_Owned_Accessor));
   if (!accessor) return NULL;

   EINA_MAGIC_SET(&accessor->accessor, EINA_MAGIC_ACCESSOR);

   accessor->carray_acc = eina_carray_length_accessor_new(array, step, length);

   accessor->accessor.version = EINA_ACCESSOR_VERSION;
   accessor->accessor.get_at = FUNC_ACCESSOR_GET_AT(eina_mono_owned_carray_get_at);
   accessor->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(eina_mono_owned_carray_get_container);
   accessor->accessor.free = FUNC_ACCESSOR_FREE(eina_mono_owned_carray_free_cb);

   // The managed callback to be called with the pinned data.
   accessor->free_cb = free_cb;
   // The managed pinned data to be unpinned.
   accessor->free_data = handle;

   return &accessor->accessor;
}