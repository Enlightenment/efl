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
