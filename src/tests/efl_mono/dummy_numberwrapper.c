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

typedef struct Dummy_Numberwrapper_Data
{
   int number;
} Dummy_Numberwrapper_Data;

void _dummy_numberwrapper_number_set(EINA_UNUSED Eo *obj, Dummy_Numberwrapper_Data *pd, int n)
{
   pd->number = n;
}

int _dummy_numberwrapper_number_get(EINA_UNUSED const Eo *obj, Dummy_Numberwrapper_Data *pd)
{
   return pd->number;
}

#include "dummy_numberwrapper.eo.c"
