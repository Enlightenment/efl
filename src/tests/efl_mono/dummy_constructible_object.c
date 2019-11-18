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
#include "libefl_mono_native_test.h"

typedef struct _Dummy_Constructible_Object_Data
{
   Eo *internal_obj;
   int native_construction_count;
   int default_construction_count;
   int special_construction_count;
} Dummy_Constructible_Object_Data;


EOLIAN static Eo *
_dummy_constructible_object_efl_object_constructor(Eo *obj, Dummy_Constructible_Object_Data *pd)
{
   ++(pd->native_construction_count);
   return efl_constructor(efl_super(obj, DUMMY_CONSTRUCTIBLE_OBJECT_CLASS));
}

EOLIAN static void
_dummy_constructible_object_efl_object_destructor(Eo *obj, Dummy_Constructible_Object_Data *pd)
{
   if (pd->internal_obj)
     efl_unref(pd->internal_obj);
   efl_destructor(efl_super(obj, DUMMY_CONSTRUCTIBLE_OBJECT_CLASS));
}

EOLIAN static Efl_Object *
_dummy_constructible_object_construct_type_and_store(Eo *obj EINA_UNUSED, Dummy_Constructible_Object_Data *pd, const Efl_Class *klass)
{
   pd->internal_obj = efl_add_ref(klass, NULL);
   return pd->internal_obj;
}

EOLIAN static void
_dummy_constructible_object_increment_default_construction_count(Eo *obj EINA_UNUSED, Dummy_Constructible_Object_Data *pd)
{
   ++(pd->default_construction_count);
}

EOLIAN static void
_dummy_constructible_object_increment_special_construction_count(Eo *obj EINA_UNUSED, Dummy_Constructible_Object_Data *pd)
{
   ++(pd->special_construction_count);
}

EOLIAN static int
_dummy_constructible_object_native_construction_count_get(const Eo *obj EINA_UNUSED, Dummy_Constructible_Object_Data *pd)
{
   return pd->native_construction_count;
}

EOLIAN static int
_dummy_constructible_object_default_construction_count_get(const Eo *obj EINA_UNUSED, Dummy_Constructible_Object_Data *pd)
{
   return pd->default_construction_count;
}

EOLIAN static int
_dummy_constructible_object_special_construction_count_get(const Eo *obj EINA_UNUSED, Dummy_Constructible_Object_Data *pd)
{
   return pd->special_construction_count;
}

EOLIAN static Efl_Object *
_dummy_constructible_object_internal_object_get(const Eo *obj EINA_UNUSED, Dummy_Constructible_Object_Data *pd)
{
   return pd->internal_obj;
}


EOLIAN static int
_dummy_constructible_object_multiply_integer_value(const Eo *obj EINA_UNUSED, Dummy_Constructible_Object_Data *pd EINA_UNUSED, int v)
{
   return 2 * v;
}

#include "dummy_constructible_object.eo.c"
