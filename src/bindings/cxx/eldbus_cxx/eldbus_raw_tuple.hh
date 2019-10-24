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
#ifndef EFL_ELDBUS_CXX_ELDBUS_RAW_TUPLE_HH
#define EFL_ELDBUS_CXX_ELDBUS_RAW_TUPLE_HH

#include <eldbus_signature_traits.hh>

#include <eina_fold.hh>
#include <eina_tuple.hh>

namespace efl { namespace eldbus { namespace _detail {

template <typename T>
struct raw_tuple;

template <typename... T>
struct raw_tuple<std::tuple<T...> >
{
  struct push_back
  {
    template <typename L, typename R>
    struct apply
      : eina::_mpl::push_back<L, typename signature_traits<R>::raw_type> {};
  };   

  typedef typename eina::_mpl::fold< std::tuple<T...>, push_back
                                     , std::tuple<> >::type type;
};

} } }

#endif
