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
#ifndef ELDBUS_CXX_ELDBUS_MESSAGE_ARGUMENTS_HH
#define ELDBUS_CXX_ELDBUS_MESSAGE_ARGUMENTS_HH

#include <Eldbus.h>

#include <tuple>
#include <iostream>

namespace efl { namespace eldbus { namespace _detail {

template <std::size_t I, typename Seq, typename Tuple>
bool _init_raw_tuple(Eldbus_Message_Iter*, Tuple const&, std::true_type)
{
  return true;
}

template <std::size_t I, typename Seq, typename Tuple>
bool _init_raw_tuple(Eldbus_Message_Iter* iterator, Tuple const& tuple, std::false_type)
{
  typedef signature_traits<typename std::tuple_element<I, Seq>::type> traits;
  typedef typename traits::value_type element_type;

  char* c = eldbus_message_iter_signature_get(iterator);
  if(c[0] != signature_traits<element_type>::sig)
    {
      return false;
    }

  eldbus_message_iter_get_and_next(iterator, *c, &std::get<I>(tuple));

  return _init_raw_tuple<I+1, Seq>
    (iterator, tuple, std::integral_constant<bool, (I+1 == std::tuple_size<Tuple>::value)>());
}

template <std::size_t I, typename Tuple>
bool _append_tuple(Eldbus_Message*, Tuple const&, std::true_type)
{
  return true;
}

template <std::size_t I, typename Tuple>
bool _append_tuple(Eldbus_Message* message, Tuple const& tuple, std::false_type)
{
  typedef signature_traits<typename std::tuple_element<I, Tuple>::type> traits;

  char signature[2] = {traits::sig, 0};
  if(!eldbus_message_arguments_append(message, signature, traits::to_raw(std::get<I>(tuple))))
    return false;

  return _append_tuple<I+1>
    (message, tuple, std::integral_constant<bool, (I+1 == std::tuple_size<Tuple>::value)>());
}

} } }

#endif
