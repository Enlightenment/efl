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
#ifndef EINA_EO_CONCRETE_FWD_HH
#define EINA_EO_CONCRETE_FWD_HH

#include <Eo.h>
#include <type_traits>

namespace efl { namespace eo {

struct concrete;

template <typename T>
struct is_eolian_object : std::false_type {};
template <typename T>
struct is_eolian_object<T const> : is_eolian_object<T> {};
template <>
struct is_eolian_object<eo::concrete> : std::true_type {};
    
} }


#endif
