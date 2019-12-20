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

#ifndef EFL_CXX_DETAIL_EO_OPS_HH
#define EFL_CXX_DETAIL_EO_OPS_HH

extern "C"
{
#include <Eo.h>
}

namespace efl { namespace eo { namespace detail {

inline void
init()
{
   ::efl_object_init();
}

inline void
shutdown()
{
   ::efl_object_shutdown();
}

inline Eo*
ref(Eo *obj)
{
   return ::efl_ref(obj);
}

inline const Eo*
ref(const Eo *obj)
{
   return ::efl_ref(obj);
}

inline void
unref(const Eo *obj)
{
   ::efl_unref(obj);
}

inline void
del(Eo *obj)
{
   ::efl_del(obj);
}

inline int
ref_get(const Eo *obj)
{
   return ::efl_ref_count(obj);
}

inline Eina_Bool
isa(const Eo *obj, const Efl_Class *klass)
{
   return efl_isa(obj, klass);
}

inline Eo*
add(const Efl_Class *klass, Eo *parent = NULL)
{
   Eo *eo = efl_add_ref(klass, parent);
   return eo;
}

inline void
dbg_info_get(Eo *obj, Efl_Dbg_Info *info)
{
   efl_dbg_info_get(obj, info);
}

inline void
base_data_set(Eo *obj, const char *key, const void *data)
{
   efl_key_data_set(obj, key, data);
}

inline void*
base_data_get(const Eo *obj, const char *key)
{
   void *data;
   data = efl_key_data_get(obj, key);
   return data;
}

inline void
base_data_del(Eo *obj, const char *key)
{
   efl_key_data_set(obj, key, NULL);
}

inline void
parent_set(Eo *obj, Eo *parent)
{
   efl_parent_set(obj, parent);
}

inline Eo*
parent_get(const Eo *obj)
{
   Eo *parent;
   parent = efl_parent_get(obj);
   return parent;
}

inline void
event_freeze(Eo *obj)
{
   efl_event_freeze(obj);
}

inline void
event_thaw(Eo *obj)
{
   efl_event_thaw(obj);
}

inline int
event_freeze_get(const Eo *obj)
{
   int count = -1;
   count = efl_event_freeze_count_get(obj);
   return count;
}

inline void
wref_add(Eo *obj, Eo **wref)
{
   efl_wref_add(obj, wref);
}

inline void
wref_del(Eo *obj, Eo **wref)
{
   efl_wref_del(obj, wref);
}

} } }


#endif // EFL_CXX_DETAIL_EO_OPS_HH
