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
#ifndef EFL_EINA_EINA_ALIGNED_UNION_HH_
#define EFL_EINA_EINA_ALIGNED_UNION_HH_

namespace efl { namespace eina { namespace _mpl {

template <std::size_t...Numbers>
struct max;

template <std::size_t A0>
struct max<A0> : std::integral_constant<std::size_t, A0> {};

template <std::size_t A0, std::size_t A1, std::size_t...Args>
struct max<A0, A1, Args...> : max<(A0 > A1 ? A0 : A1), Args...> {};

}

// Workaround needed for GCC before 5.1
template <std::size_t Min, typename...Args>
struct aligned_union
{
   static constexpr std::size_t alignment_value = _mpl::max<alignof(Args)...>::value;

   typedef typename std::aligned_storage
   < _mpl::max<Min, sizeof(Args)...>::value
     , alignment_value >::type type;
};

} }

#endif

