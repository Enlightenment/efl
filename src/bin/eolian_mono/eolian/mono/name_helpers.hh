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
#ifndef EOLIAN_MONO_NAME_HELPERS_HH
#define EOLIAN_MONO_NAME_HELPERS_HH

#include <algorithm>
#include <cctype>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "utils.hh"

#include "grammar/integral.hpp"
#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/list.hpp"
#include "grammar/string.hpp"
#include "grammar/integral.hpp"

using efl::eolian::grammar::as_generator;
using efl::eolian::grammar::string;
using efl::eolian::grammar::lit;
using efl::eolian::grammar::operator*;

namespace eolian_mono {

/* Utility functions for naming things. Compared to the utils.hh, this header has higher level
 * functions, dealing with the knowledge of how to convert the items to the C# style we are using, for
 * example, while being too short to be implemented as full-fledged generators.
 */
namespace name_helpers {

namespace attributes = efl::eolian::grammar::attributes;

namespace detail {
inline bool is_equal(std::string const& lhs, std::string const& rhs)
{
  return lhs == rhs;
}
}

// Forward declarations
template<typename  T>
inline std::string klass_concrete_or_interface_name(T const& klass);

inline std::string identity(std::string const& str)
{
  return str;
}

inline std::string escape_keyword(std::string const& name)
{
  using detail::is_equal;
  if(is_equal(name, "delete")
     || is_equal(name, "do")
     || is_equal(name, "lock")
     || is_equal(name, "event")
     || is_equal(name, "in")
     || is_equal(name, "out")
     || is_equal(name, "object")
     || is_equal(name, "interface")
     || is_equal(name, "string")
     || is_equal(name, "internal")
     || is_equal(name, "fixed")
     || is_equal(name, "var")
     || is_equal(name, "base"))
    return "kw_" + name;

  if (is_equal(name, "Finalize"))
    return name + "Add"; // Eo's Finalize is actually the end of efl_add.
  return name;
}

typedef std::function<std::string(std::string const&)> string_transform_func;

inline std::string join_namespaces(std::vector<std::string> const& namespaces, char separator,
                                   string_transform_func func=identity)
{
   std::stringstream s;
   for (auto&& n : namespaces)
     s << func(n) << separator;

   return s.str();
}

static const std::vector<std::string> verbs =
  {
    "add",
    "get",
    "is",
    "del",
    "thaw",
    "freeze",
    "save",
    "wait",
    "eject",
    "raise",
    "lower",
    "load",
    "dup",
    "reset",
    "unload",
    "close",
    "set",
    "interpolate",
    "has",
    "grab",
    "check",
    "find",
    "ungrab",
    "unset",
    "clear",
    "pop",
    "new",
    "peek",
    "push",
    "update",
    "show",
    "move",
    "hide",
    "calculate",
    "resize",
    "attach",
    "pack",
    "unpack",
    "emit",
    "call",
    "append",
    "apply",
    "bind",
    "cancel",
    "copy",
    "create",
    "cut",
    "delete",
    "deselect",
    "detach",
    "do",
    "gen",
    "insert",
    "iterate",
    "join",
    "leave",
    "limit",
    "paste",
    "parse",
    "prepend",
    "process",
    "query",
    "refresh",
    "remove",
    "register",
    "reject",
    "release",
    "reply",
    "send",
    "select",
    "serialize",
    "steal",
    "sync",
    "toggle",
    "unbind",
    "unregister",
    "unselect"
  };

const std::vector<std::string> not_verbs =
  {
    "below",
    "above",
    "name",
    "unfreezable",
    "value",
    "r",
    "g",
    "b",
    "a",
    "finalize",
    "destructor",
    "to",
    "circle",
    "rect",
    "path",
    "commands",
    "type",
    "colorspace"
    "op",
    "type",
    "properties",
    "status",
    "status",
    "relative",
    "ptr",
    "pair",
    "pos",
    "end"
  };

void reorder_verb(std::vector<std::string> &names)
{
  if (names.size() <= 1)
    return;

  std::string verb = names.back();

  if (std::find(verbs.begin(), verbs.end(), verb) != verbs.end())
    {
       names.pop_back();
       names.insert(names.begin(), verb);
    }
}

inline std::string managed_namespace(std::string const& ns)
{
  return escape_keyword(utils::remove_all(ns, '_'));
}

inline std::string managed_name(std::string const& name, char separator='_')
{
  auto tokens = utils::split(name, separator);
  return utils::to_pascal_case(tokens);
}

inline std::string managed_method_name(attributes::function_def const& f)
{
  std::vector<std::string> names = utils::split(f.name, '_');

  name_helpers::reorder_verb(names);

  std::string candidate = escape_keyword(utils::to_pascal_case(names));

  // Some eolian methods have the same name as their parent class
  if (candidate == klass_concrete_or_interface_name(f.klass))
      candidate = "Do" + candidate;

  // Avoid clashing with System.Object.GetType
  if (candidate == "GetType" || candidate == "SetType")
    {
       candidate.insert(3, managed_name(f.klass.eolian_name));
    }

  return candidate;
}

inline std::string full_managed_name(std::string const& name)
{
  std::stringstream ss;

  auto words = utils::split(name, '.');
  std::transform(words.begin(), words.end(), words.begin(), [](std::string const& word) {
     return managed_name(word);
  });

  auto b = std::begin(words), e = std::end(words);

  if (b != e)
    {
      std::copy(b, std::prev(e), std::ostream_iterator<std::string>(ss, "."));
      b = std::prev(e);
    }

  // Avoid trailing separator
  if (b != e)
    ss << *b;

  return ss.str();
}

inline std::string alias_full_eolian_name(attributes::alias_def const& alias)
{

   std::string eolian_name = utils::remove_all(alias.eolian_name, '_');
   return join_namespaces(alias.namespaces, '.') + eolian_name;
}

inline std::string managed_async_method_name(attributes::function_def const& f)
{
  return managed_method_name(f) + "Async";
}
inline std::string function_ptr_full_eolian_name(attributes::function_def const& func)
{
   return join_namespaces(func.namespaces, '.') + func.name;
}

inline std::string type_full_eolian_name(attributes::regular_type_def const& type)
{
   return join_namespaces(type.namespaces, '.') + type.base_type;
}

inline std::string type_full_managed_name(attributes::regular_type_def const& type)
{
   return join_namespaces(type.namespaces, '.', managed_namespace) + utils::remove_all(type.base_type, '_');
}

inline std::string struct_full_eolian_name(attributes::struct_def const& struct_)
{
   return join_namespaces(struct_.namespaces, '.') + struct_.cxx_name;
}

template<typename T>
inline std::string typedecl_managed_name(T const& item)
{
   return utils::remove_all(item.cxx_name, '_');
}

inline std::string typedecl_managed_name(attributes::function_def const& func)
{
   return utils::remove_all(func.name, '_');
}


inline std::string enum_field_managed_name(std::string name)
{
   std::vector<std::string> names = utils::split(name, '_');
   return utils::to_pascal_case(names);
}

inline std::string to_field_name(std::string const& in)
{
   std::vector<std::string> names = utils::split(in, '_');
   return utils::to_camel_case(names);
}



template<typename T>
inline std::string property_managed_name(T const& klass, std::string const& name)
{
  auto names = utils::split(name, '_');
  // No need to escape keyword here as it will be capitalized and already
  // namespaced inside the owner class.
  auto managed_name = utils::to_pascal_case(names);
  auto managed_klass_name = klass_concrete_or_interface_name(klass);

  if (managed_name == "Type")
    managed_name = managed_klass_name + managed_name;

  return managed_name;
}

inline std::string property_managed_name(attributes::property_def const& property)
{
  return property_managed_name(property.klass, property.name);
}

inline std::string managed_part_name(attributes::part_def const& part)
{
  std::vector<std::string> names = utils::split(part.name, '_');
  return utils::to_pascal_case(names) + "Part";
}

// Class name translation (interface/concrete/inherit/etc)
struct klass_interface_name_generator
{

  template <typename T>
  std::string operator()(T const& klass) const
  {
     return ((klass.type == attributes::class_type::mixin
              || klass.type == attributes::class_type::interface_) ? "I" : "")
       + utils::remove_all(klass.eolian_name, '_');
  }

  template <typename OutputIterator, typename Attr, typename Context>
  bool generate(OutputIterator sink, Attr const& attribute, Context const& context) const
  {
    return as_generator((*this).operator()<Attr>(attribute)).generate(sink, attributes::unused, context);
  }
} const klass_interface_name;

struct klass_full_interface_name_generator
{
  template <typename T>
  std::string operator()(T const& klass) const
  {
    return join_namespaces(klass.namespaces, '.', managed_namespace) + klass_interface_name(klass);
  }

  template <typename OutputIterator, typename Attr, typename Context>
  bool generate(OutputIterator sink, Attr const& attribute, Context const& context) const
  {
    return as_generator((*this).operator()<Attr>(attribute)).generate(sink, attributes::unused, context);
  }
} const klass_full_interface_name;

template<typename T>
inline std::string klass_concrete_name(T const& klass)
{
   return utils::remove_all(klass.eolian_name, '_');
}

template<typename  T>
inline std::string klass_concrete_or_interface_name(T const& klass)
{
    switch(klass.type)
    {
    case attributes::class_type::abstract_:
    case attributes::class_type::regular:
      return klass_concrete_name(klass);
    default:
      return klass_interface_name(klass);
    }
}

struct klass_full_concrete_name_generator
{
  template <typename T>
  std::string operator()(T const& klass) const
  {
    return join_namespaces(klass.namespaces, '.', managed_namespace) + klass_concrete_name(klass);
  }

  template <typename OutputIterator, typename Attr, typename Context>
  bool generate(OutputIterator sink, Attr const& attribute, Context const& context) const
  {
    return as_generator((*this).operator()<Attr>(attribute)).generate(sink, attributes::unused, context);
  }
} const klass_full_concrete_name;

struct klass_full_concrete_or_interface_name_generator
{
  template <typename T>
  std::string operator()(T const& klass) const
  {
    switch(klass.type)
    {
    case attributes::class_type::abstract_:
    case attributes::class_type::regular:
      return klass_full_concrete_name(klass);
    default:
      return klass_full_interface_name(klass);
    }
  }

  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator, attributes::unused_type, Context const&) const
  {
    return true;
  }

  template <typename OutputIterator, typename Attr, typename Context>
  bool generate(OutputIterator sink, Attr const& attribute, Context const& context) const
  {
    return as_generator((*this).operator()<Attr>(attribute)).generate(sink, attributes::unused, context);
  }
} const klass_full_concrete_or_interface_name;

template<typename T>
inline std::string klass_inherit_name(T const& klass)
{
  return klass_concrete_name(klass);
}

template<typename T>
inline std::string klass_native_inherit_name(T const& klass)
{
  return ((klass.type == attributes::class_type::mixin
           || klass.type == attributes::class_type::interface_) ? klass_interface_name(klass) : "") + "NativeMethods";
}

template<typename T>
inline std::string klass_full_native_inherit_name(T const& klass)
{
  if(klass.type == attributes::class_type::mixin
     || klass.type == attributes::class_type::interface_)
    return join_namespaces(klass.namespaces, '.', managed_namespace) + klass_native_inherit_name(klass);

  return klass_full_concrete_name(klass) + "." + klass_native_inherit_name(klass);
}

template<typename T>
inline std::string klass_get_name(T const& clsname)
{
  return clsname.klass_get_name;
}

template<typename T>
inline std::string klass_get_full_name(T const& clsname)
{
  if(clsname.type == attributes::class_type::mixin
     || clsname.type == attributes::class_type::interface_)
    return klass_get_name(clsname);

  return klass_full_concrete_name(clsname) + "." + klass_get_name(clsname);
}

// Events
inline std::string managed_event_name(std::string const& name)
{
   return utils::to_pascal_case(utils::split(name, "_,"), "") + "Event";
}

inline std::string managed_event_args_short_name(attributes::event_def const& evt)
{
   return utils::remove_all(evt.klass.eolian_name, '_') + name_helpers::managed_event_name(evt.name) + "Args";
}

inline std::string managed_event_args_name(attributes::event_def evt)
{
   return join_namespaces(evt.klass.namespaces, '.', managed_namespace) +
          managed_event_args_short_name(evt);
}

inline std::string translate_inherited_event_name(const attributes::event_def &evt, const attributes::klass_def &klass)
{
   return join_namespaces(klass.namespaces, '_') + klass_interface_name(klass) + "_" + managed_event_name(evt.name);
}

// Open/close namespaces
template<typename OutputIterator, typename Context>
bool open_namespaces(OutputIterator sink, std::vector<std::string> namespaces, Context const& context)
{
  std::transform(namespaces.begin(), namespaces.end(), namespaces.begin(), managed_namespace);

  std::string joined_namespace = join_namespaces(namespaces, '.');
  if (joined_namespace.empty()) return true;
  joined_namespace.pop_back();

  return as_generator("namespace " << string << " {\n").generate(sink, joined_namespace, context);
}

template<typename OutputIterator, typename Context>
bool close_namespaces(OutputIterator sink, std::vector<std::string> const& namespaces, Context const& context)
{
     if (namespaces.empty()) return true;
     return as_generator("}\n\n").generate(sink, attributes::unused, context);
}

std::string constructor_managed_name(std::string full_name)
{
    auto tokens = utils::split(full_name, '.');

    return managed_name(tokens.at(tokens.size()-1));
}

std::string translate_value_type(std::string const& name)
{
  static std::map<std::string, std::string> table = {
    {"sbyte", "SByte"},
    {"byte","Byte"},
    {"short","Int16"},
    {"ushort","UInt16"},
    {"int", "Int32"},
    {"uint","UInt32"},
    {"long","Int64"},
    {"ulong","UInt64"},
    {"char","Char"},
    {"float","Single"},
    {"double","Double"},
    {"bool","Boolean"},
    {"decimal","Decimal"},
  };

  auto found = table.find(name);

  if (found != table.end())
    return found->second;

  return name;
}


// Field names //
struct struct_field_name_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
  {
    return as_generator(string).generate(sink, name_helpers::to_field_name(field.name), context);
  }
} const struct_field_name {};

// Property names //
struct struct_property_name_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
  {
    return as_generator(string).generate(sink, name_helpers::managed_name(field.name), context);
  }
} const struct_property_name {};

} // namespace name_helpers

} // namespace eolian_mono


namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator<eolian_mono::name_helpers::klass_interface_name_generator> : std::true_type {};
template <>
struct is_generator<eolian_mono::name_helpers::klass_interface_name_generator> : std::true_type {};

template <>
struct is_eager_generator<eolian_mono::name_helpers::klass_full_interface_name_generator> : std::true_type {};
template <>
struct is_generator<eolian_mono::name_helpers::klass_full_interface_name_generator> : std::true_type {};

template <>
struct is_eager_generator<eolian_mono::name_helpers::klass_full_concrete_or_interface_name_generator> : std::true_type {};
template <>
struct is_generator<eolian_mono::name_helpers::klass_full_concrete_or_interface_name_generator> : std::true_type {};

template <>
struct is_eager_generator<eolian_mono::name_helpers::klass_full_concrete_name_generator> : std::true_type {};
template <>
struct is_generator<eolian_mono::name_helpers::klass_full_concrete_name_generator> : std::true_type {};

template <>
struct is_eager_generator<eolian_mono::name_helpers::struct_field_name_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::name_helpers::struct_field_name_generator> : std::true_type {};

template <>
struct is_eager_generator<eolian_mono::name_helpers::struct_property_name_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::name_helpers::struct_property_name_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<struct ::eolian_mono::name_helpers::klass_full_concrete_or_interface_name_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::name_helpers::struct_field_name_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed< ::eolian_mono::name_helpers::struct_property_name_generator> : std::integral_constant<int, 1> {};

}
      
} } }
      
#endif
