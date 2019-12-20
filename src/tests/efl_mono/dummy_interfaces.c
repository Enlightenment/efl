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
// Include file for interfaces .eo.c files
#define DUMMY_TEST_IFACE_PROTECTED

#include "libefl_mono_native_test.h"

void _dummy_test_iface_static_prop_set(int data EINA_UNUSED)
{
}

int _dummy_test_iface_static_prop_get()
{
    return -1;
}

#include "dummy_test_iface.eo.c"
#include "dummy_inherit_iface.eo.c"
