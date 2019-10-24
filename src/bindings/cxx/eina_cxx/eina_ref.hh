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
#ifndef EINA_REF_HH
#define EINA_REF_HH

#include <functional>

/**
 * @addtogroup Eina_Cxx_Tools_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Ref_Group Reference Wrapper
 * @ingroup Eina_Cxx_Tools_Group
 *
 * @{
 */

/**
 * @brief Creates a @c reference_wrapper to the given object.
 * @return @c reference_wrapper holding a reference to the given object.
 *
 * Creates the appropriate reference_wrapper type to hold a reference to
 * the given object. If the argument is itself a @c reference_wrapper,
 * it creates a copy of it instead.
 */
using std::ref;

/**
 * @brief Creates a @c reference_wrapper to the given object.
 * @return @c reference_wrapper holding a reference to the given object.
 *
 * Specialized version of @ref ref for @c reference_wrapper to constant
 * types.
 */
using std::cref;

/**
 * Class that wraps a reference in a copyable, assignable object.
 */
using std::reference_wrapper;

/**
 * Get a reference from a @c reference_wrapper. If @p t is already a
 * reference just return a reference to @p t.
 *
 * @{
 */
template <typename T>
T& unref(T& t)
{
   return t;
}

template <typename T>
T& unref(reference_wrapper<T> t)
{
   return t.get();
}

/**
 * @}
 */

/**
 * @}
 */

}}

/**
 * @}
 */

#endif
