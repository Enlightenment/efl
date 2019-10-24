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
///
/// @file eo_concrete.hh
///

#ifndef EFL_CXX_EINA_COPY_TRAITS_HH
#define EFL_CXX_EINA_COPY_TRAITS_HH

namespace efl { namespace eina {

namespace _impl {
template<bool...> struct bool_pack;
template <bool...Args>
struct and_ : std::is_same<bool_pack<Args..., true>, bool_pack<true, Args...>> {};
}
    
template <typename... Args>
struct variant;

template <typename T, typename Enable = void>
struct copy_from_c_traits;

template <typename T>
struct copy_from_c_traits<T, typename std::enable_if<std::is_fundamental<T>::value>::type>
{
  static void copy_to_unitialized(T* storage, void const* data)
  {
    std::memcpy(storage, data, sizeof(T));
  }
};

template <typename...Args>
struct copy_from_c_traits<eina::variant<Args...>,
                          typename std::enable_if<_impl::and_<std::is_fundamental<Args>::value...>::value>::type>
{
  template <typename T>
  static void copy_to_unitialized(eina::variant<Args...>* storage, T const* data)
  {
     new (storage) eina::variant<Args...>{*data};
  }
};
    
    
template <typename T, typename Enable = void>
struct alloc_to_c_traits;

template <typename T>
struct alloc_to_c_traits<T, typename std::enable_if<std::is_fundamental<T>::value>::type>
{
  typedef T c_type;
  static c_type* copy_alloc(T const& value)
  {
    c_type* v = static_cast<c_type*>(malloc(sizeof(c_type)));
    std::memcpy(v, &value, sizeof(c_type));
    return v;
  }
  static void free_alloc(void* data)
  {
    ::free(data);
  }
};

} }

#endif
