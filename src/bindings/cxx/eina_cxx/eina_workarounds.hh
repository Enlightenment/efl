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
#ifndef EINA_WORKAROUNDS_HH_
#define EINA_WORKAROUNDS_HH_

#include <eina_eo_concrete_fwd.hh>

#include <Eo.h>
#include <type_traits>

struct _Elm_Calendar_Mark;
struct Elm_Gen_Item;
struct _Elm_Map_Overlay;

// namespace std {

// template <>
// struct is_base_of< ::efl::eo::concrete, _Elm_Calendar_Mark > : std::false_type {};
// template <>
// struct is_base_of< ::efl::eo::concrete, _Elm_Calendar_Mark const > : std::false_type {};
// template <>
// struct is_base_of< ::efl::eo::concrete, _Elm_Calendar_Mark volatile > : std::false_type {};
// template <>
// struct is_base_of< ::efl::eo::concrete, _Elm_Calendar_Mark const volatile > : std::false_type {};

// template <>
// struct is_base_of< const ::efl::eo::concrete, _Elm_Calendar_Mark > : std::false_type {};
// template <>
// struct is_base_of< const ::efl::eo::concrete, _Elm_Calendar_Mark const > : std::false_type {};
// template <>
// struct is_base_of< const ::efl::eo::concrete, _Elm_Calendar_Mark volatile > : std::false_type {};
// template <>
// struct is_base_of< const ::efl::eo::concrete, _Elm_Calendar_Mark const volatile > : std::false_type {};

// template <>
// struct is_base_of< ::efl::eo::concrete, Elm_Gen_Item > : std::false_type {};
// template <>
// struct is_base_of< ::efl::eo::concrete, Elm_Gen_Item const > : std::false_type {};
// template <>
// struct is_base_of< ::efl::eo::concrete, Elm_Gen_Item volatile > : std::false_type {};
// template <>
// struct is_base_of< ::efl::eo::concrete, Elm_Gen_Item const volatile > : std::false_type {};

// template <>
// struct is_base_of< const ::efl::eo::concrete, Elm_Gen_Item > : std::false_type {};
// template <>
// struct is_base_of< const ::efl::eo::concrete, Elm_Gen_Item const > : std::false_type {};
// template <>
// struct is_base_of< const ::efl::eo::concrete, Elm_Gen_Item volatile > : std::false_type {};
// template <>
// struct is_base_of< const ::efl::eo::concrete, Elm_Gen_Item const volatile > : std::false_type {};

// template <>
// struct is_base_of< ::efl::eo::concrete, _Elm_Map_Overlay > : std::false_type {};
// template <>
// struct is_base_of< ::efl::eo::concrete, _Elm_Map_Overlay const > : std::false_type {};
// template <>
// struct is_base_of< ::efl::eo::concrete, _Elm_Map_Overlay volatile > : std::false_type {};
// template <>
// struct is_base_of< ::efl::eo::concrete, _Elm_Map_Overlay const volatile > : std::false_type {};

// template <>
// struct is_base_of< const ::efl::eo::concrete, _Elm_Map_Overlay > : std::false_type {};
// template <>
// struct is_base_of< const ::efl::eo::concrete, _Elm_Map_Overlay const > : std::false_type {};
// template <>
// struct is_base_of< const ::efl::eo::concrete, _Elm_Map_Overlay volatile > : std::false_type {};
// template <>
// struct is_base_of< const ::efl::eo::concrete, _Elm_Map_Overlay const volatile > : std::false_type {};

// }

#endif
