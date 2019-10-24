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
#ifndef EFL_EINA_FOLD_HH_
#define EFL_EINA_FOLD_HH_

#include <eina_tuple.hh>

namespace efl { namespace eina {

namespace _mpl {

template <typename T, typename F, typename A0, bool B = std::is_same<T, std::tuple<> >::value>
struct fold_impl
{
  typedef typename F::template apply<A0, typename std::tuple_element<0, T>::type>::type result;
  typedef typename fold_impl<typename _mpl::pop_front<T>::type
                             , F, result
                             >::type
  type;
};

template <typename T, typename F, typename A0>
struct fold_impl<T, F, A0, true>
{
  typedef A0 type;
};

template <typename T, typename F, typename A0>
struct fold : fold_impl<T, F, A0>
{};

} } }

#endif
