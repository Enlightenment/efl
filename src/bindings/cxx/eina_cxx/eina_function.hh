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
#ifndef EFL_EINA_FUNCTION_HH
#define EFL_EINA_FUNCTION_HH

namespace efl { namespace eina { namespace _mpl {

template <typename T>
struct function_params;

template <typename R, typename... P>
struct function_params<R(*)(P...)>
{
  typedef std::tuple<P...> type;
};

template <typename T>
struct function_return;

template <typename R, typename... P>
struct function_return<R(*)(P...)>
{
  typedef R type;
};
      
} } }

#endif
