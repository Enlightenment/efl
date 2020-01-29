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
#ifndef EOLIAN_MONO_EVENTS_HH
#define EOLIAN_MONO_EVENTS_HH

#include <iterator>

#include <Eina.hh>

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "type_match.hh"
#include "name_helpers.hh"
#include "using_decl.hh"

namespace eolian_mono {

namespace eina = efl::eina;

template<typename OutputIterator, typename Context>
struct unpack_event_args_visitor
{
   mutable OutputIterator sink;
   Context const* context;
   attributes::type_def const& type;

   typedef unpack_event_args_visitor<OutputIterator, Context> visitor_type;
   typedef bool result_type;
   bool operator()(grammar::attributes::regular_type_def const& regular) const
   {
      std::string const& arg = "info";
      std::string arg_type = name_helpers::type_full_managed_name(regular);

      if (regular.is_struct())
        {
           // Structs are usually passed by pointer to events, like having a ptr<> modifier
           // Uses implicit conversion from IntPtr
           return as_generator(
                " info"
              ).generate(sink, attributes::unused, *context);
        }
      else if (type.is_ptr)
        {
           return as_generator("(" + arg_type + ")Marshal.PtrToStructure(" + arg + ", typeof(" + arg_type + "))")
                 .generate(sink, attributes::unused, *context);
        }

      using attributes::regular_type_def;
      struct match
      {
         eina::optional<std::string> name;
         std::function<std::string()> function;
      }
      /// Sizes taken from https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/keywords/sizeof
      const match_table [] =
        {
           {"bool", [&arg] { return "Marshal.ReadByte(" + arg + ") != 0"; }}

           , {"ubyte", [&arg] { return "Marshal.ReadByte(" + arg + ")"; }}
           , {"byte", [&arg] { return "(sbyte) Marshal.ReadByte(" + arg + ")"; }}

           , {"char", [&arg] { return "(char) Marshal.ReadByte(" + arg + ")"; }}

           , {"short", [&arg] { return "Marshal.ReadInt16(" + arg + ")"; }}
           , {"ushort", [&arg] { return "(ushort) Marshal.ReadInt16(" + arg + ")"; }}

           , {"int", [&arg] { return "Marshal.ReadInt32(" + arg + ")"; }}
           , {"uint", [&arg] { return "(uint) Marshal.ReadInt32(" + arg + ")"; }}

           , {"long", [&arg] { return "Marshal.ReadInt64(" + arg + ")"; }}
           , {"ulong", [&arg] { return "(ulong) Marshal.ReadInt64(" + arg + ")"; }}

           , {"llong", [&arg] { return "(long) Marshal.ReadInt64(" + arg + ")"; }}
           , {"ullong", [&arg] { return "(ulong) Marshal.ReadInt64(" + arg + ")"; }}

           , {"int8", [&arg] { return "(sbyte)Marshal.ReadByte(" + arg + ")"; }}
           , {"uint8", [&arg] { return "Marshal.ReadByte(" + arg + ")"; }}

           , {"int16", [&arg] { return "Marshal.ReadInt16(" + arg + ")"; }}
           , {"uint16", [&arg] { return "(ushort)Marshal.ReadInt16(" + arg + ")"; }}

           , {"int32", [&arg] { return "Marshal.ReadInt32(" + arg + ")"; }}
           , {"uint32", [&arg] { return "(uint) Marshal.ReadInt32(" + arg + ")"; }}

           // We don't support int128 as csharp has no similar datatype.
           , {"int64", [&arg] { return "Marshal.ReadInt64(" + arg + ")"; }}
           , {"uint64", [&arg] { return "(ulong) Marshal.ReadInt64(" + arg + ")"; }}

           , {"float", [&arg] { return "Eina.PrimitiveConversion.PointerToManaged<float>(" + arg + ")"; }}
           , {"double", [&arg] { return "Eina.PrimitiveConversion.PointerToManaged<double>(" + arg + ")"; }}

           , {"string", [&arg] { return "Eina.StringConversion.NativeUtf8ToManagedString(" + arg + ")"; }}
           , {"stringshare", [&arg] { return "Eina.StringConversion.NativeUtf8ToManagedString(" + arg + ")"; }}
           , {"Eina.Error", [&arg] { return "(Eina.Error)Marshal.PtrToStructure(" + arg + ", typeof(Eina.Error))"; }}
        };

      std::string full_type_name = name_helpers::type_full_eolian_name(regular);
      auto filter_func = [&regular, &full_type_name] (match const& m)
        {
           return (!m.name || *m.name == regular.base_type || *m.name == full_type_name);
        };

      auto accept_func = [&](std::string const& conversion)
        {
           return as_generator(conversion).generate(sink, attributes::unused, *context);
        };

      if (eina::optional<bool> b = type_match::get_match(match_table, filter_func, accept_func))
        return *b;
      else
        {
           // Type defined in Eo is passed here. (e.g. enum type defined in Eo)
           // Uses conversion from IntPtr with type casting to the given type.
           return as_generator(
                " (" << arg_type << ")info"
              ).generate(sink, attributes::unused, *context);
        }
   }
   bool operator()(grammar::attributes::klass_name const& cls) const
   {
      return as_generator("(Efl.Eo.Globals.CreateWrapperFor(info) as " + name_helpers::klass_full_interface_name(cls) + ")").generate(sink, attributes::unused, *context);
   }
   bool operator()(attributes::complex_type_def const& types) const
   {
      if (types.outer.base_type == "iterator")
        return as_generator("Efl.Eo.Globals.IteratorTo" << eolian_mono::type << "(info)").generate(sink, type, *context);
      else if (types.outer.base_type == "accessor")
        return as_generator("Efl.Eo.Globals.AccessorTo" << eolian_mono::type << "(info)").generate(sink, type, *context);
      else if (types.outer.base_type == "array")
        return as_generator("Efl.Eo.Globals.NativeArrayTo" << eolian_mono::type << "(info)").generate(sink, type, *context);
      else if (types.outer.base_type == "list")
        return as_generator("Efl.Eo.Globals.NativeListTo" << eolian_mono::type << "(info)").generate(sink, type, *context);
      else
        return as_generator("new " << eolian_mono::type << "(info, false, false)").generate(sink, type, *context);
   }
};

template<typename OutputIterator, typename Context>
struct pack_event_info_and_call_visitor
{
   mutable OutputIterator sink;
   Context const* context;
   attributes::type_def const& type;
   std::string library_name;
   std::string evt_c_name;

   typedef pack_event_info_and_call_visitor<OutputIterator, Context> visitor_type;
   typedef bool result_type;

   bool operator()(grammar::attributes::regular_type_def const& regular) const
   {
      std::string arg_type = name_helpers::type_full_managed_name(regular);

      auto const& indent = current_indentation(*context);

      if (regular.is_struct())
        {
           return as_generator(
                indent.inc() << "Contract.Requires(e != null, nameof(e));\n"
                << indent.inc() << "IntPtr info = Marshal.AllocHGlobal(Marshal.SizeOf(e.Arg));\n"
                << indent.inc() << "CallNativeEventCallback(" + library_name + ", \"_" + evt_c_name + "\", info, " << "(p) => Marshal.FreeHGlobal(p));\n"
              ).generate(sink, attributes::unused, *context);
        }

      using attributes::regular_type_def;
      struct match
      {
         eina::optional<std::string> name;
         std::function<std::string()> function;
      };

      std::string full_type_name = name_helpers::type_full_eolian_name(regular);
      auto filter_func = [&regular, &full_type_name] (match const& m)
        {
           return (!m.name || *m.name == regular.base_type || *m.name == full_type_name);
        };

      match const str_table[] =
        {
           {"string", [] { return "e.Arg"; }}
           , {"stringshare", [] { return "e.Arg"; }}
        };

      auto str_accept_func = [&](std::string const& conversion)
        {
           return as_generator(
             indent.inc() << "Contract.Requires(e != null, nameof(e));\n"
             << indent.inc() << "IntPtr info = Eina.StringConversion.ManagedStringToNativeUtf8Alloc(" << conversion << ");\n"
             << indent.inc() << "CallNativeEventCallback(" + library_name + ", \"_" + evt_c_name + "\", info, " << "(p) => Eina.MemoryNative.Free(p));\n"
             ).generate(sink, attributes::unused, *context);
        };

      if (eina::optional<bool> b = type_match::get_match(str_table, filter_func, str_accept_func))
        return *b;

      match const value_table [] =
        {
           {"bool", [] { return "e.Arg ? (byte) 1 : (byte) 0"; }}
           , {"Eina.Error", [] { return "(int)e.Arg"; }}
           , {nullptr, [] { return "e.Arg"; }}
        };

      auto value_accept_func = [&](std::string const& conversion)
        {
           return as_generator(
             indent.inc() << "Contract.Requires(e != null, nameof(e));\n"
             << indent.inc() << "IntPtr info = Eina.PrimitiveConversion.ManagedToPointerAlloc(" << conversion << ");\n"
             << indent.inc() << "CallNativeEventCallback(" + library_name + ", \"_" + evt_c_name + "\", info, " << "(p) => Marshal.FreeHGlobal(p));\n"
             ).generate(sink, attributes::unused, *context);
        };

      if (eina::optional<bool> b = type_match::get_match(value_table, filter_func, value_accept_func))
        return *b;

      return value_accept_func("e.Args");
   }
   bool operator()(grammar::attributes::klass_name const&) const
   {
      auto const& indent = current_indentation(*context);
      return as_generator(
                          indent.inc() << "Contract.Requires(e != null, nameof(e));\n"
                          << indent.inc() << "IntPtr info = e.Arg.NativeHandle;\n"
                          << indent.inc() << "CallNativeEventCallback(" << library_name << ", \"_" << evt_c_name << "\", info, null);\n"
                          ).generate(sink, attributes::unused, *context);
   }
   bool operator()(attributes::complex_type_def const& type) const
   {
      auto const& indent = current_indentation(*context);
      bool is_own = type.outer.base_qualifier & attributes::qualifier_info::is_own;
      std::string info_variable;

      if (type.outer.base_type == "iterator")
        info_variable = std::string("IntPtr info = Efl.Eo.Globals.IEnumerableToIterator(e.Arg, ") + (is_own ? "true" : "false") + ");\n";
      else if (type.outer.base_type == "accessor")
        info_variable = std::string("IntPtr info = Efl.Eo.Globals.IEnumerableToAccessor(e.Arg, ") + (is_own ? "true" : "false") + ");\n";
      else if (type.outer.base_type == "array")
        info_variable = std::string("IntPtr info = Efl.Eo.Globals.IListToNativeArray(e.Arg, ") + (is_own ? "true" : "false") + ");\n";
      else if (type.outer.base_type == "list")
        info_variable = std::string("IntPtr info = Efl.Eo.Globals.IListToNativeList(e.Arg, ") + (is_own ? "true" : "false") + ");\n";
      else
        info_variable = "IntPtr info = e.Arg.Handle;\n";
      return as_generator(indent.inc() << "Contract.Requires(e != null, nameof(e));\n"
                          << indent.inc() << info_variable
                          << indent.inc() << "CallNativeEventCallback(" << library_name << ", \"_" << evt_c_name << "\", info, null);\n"
                          ).generate(sink, attributes::unused, *context);
   }
};

/*
 * Generates a struct wrapping the argument of a given event.
 */
struct event_argument_wrapper_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::event_def const& evt, Context const& context) const
   {
      efl::eina::optional<grammar::attributes::type_def> etype = evt.type;
      if (!etype.is_engaged())
        return true;

      if (blacklist::is_event_blacklisted(evt, context))
        return true;

      std::string evt_name = name_helpers::managed_event_name(evt.name);

      if (!as_generator(scope_tab << "/// <summary>Event argument wrapper for event <see cref=\""
                        << join_namespaces(evt.klass.namespaces, '.', managed_namespace)
                        << klass_interface_name(evt.klass) << "." << evt_name << "\"/>.\n"
             ).generate(sink, nullptr, context))
        return false;

      std::string since;

      if (!evt.is_beta)
        {
           since = evt.documentation.since;

           if (since.empty())
             {
                auto unit = (const Eolian_Unit*) context_find_tag<eolian_state_context>(context).state;
                attributes::klass_def klass(get_klass(evt.klass, unit), unit);
                since = klass.documentation.since;
             }

           if (!since.empty())
             {

                if (!as_generator(
                      scope_tab << lit("/// <para>Since EFL ") << evt.documentation.since << ".</para>\n"
                    ).generate(sink, nullptr, context))
                  return false;
             }
           else
             {
                EINA_CXX_DOM_LOG_ERR(eolian_mono::domain) << "Event " << evt.name << " of class " << evt.klass.eolian_name
                    << " is stable but has no 'Since' information.";
                // We do not bail out here because there are some cases of this happening upstream.
             }
        }

      if (!as_generator(scope_tab << lit("/// </summary>\n")
                        << scope_tab << "[Efl.Eo.BindingEntity]\n"
                        << scope_tab << "public class " << name_helpers::managed_event_args_short_name(evt) << " : EventArgs {\n"
                         << scope_tab(2) << "/// <summary>Actual event payload.\n"
             ).generate(sink, nullptr, context))
        return false;

      if (since != "")
        {
           if (!as_generator(scope_tab(2) << "/// <para>Since EFL " << since << ".</para>\n").generate(sink, nullptr, context))
             return false;
        }

      if (!as_generator(scope_tab(2) << "/// </summary>\n"
                        << scope_tab(2) << "/// <value>" << documentation_string << "</value>\n"
                        << scope_tab(2) << "public " << type << " Arg { get; set; }\n"
                        << scope_tab << "}\n\n"
                 ).generate(sink, std::make_tuple(evt.documentation.summary, *etype), context))
        return false;

      return true;
   }
} const event_argument_wrapper {};

/*
 * Generates an event declaration as a C# Interface member.
 * In regular/abstract classes they are declared directly in their
 * implementation in event_definition_generator.
 */
struct event_declaration_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::event_def const& evt, Context const& context) const
   {
      std::string wrapper_args_type;
      std::string evt_name = name_helpers::managed_event_name(evt.name);

      if (blacklist::is_event_blacklisted(evt, context))
        return true;

      if (evt.type.is_engaged())
        wrapper_args_type = "<" + name_helpers::managed_event_args_name(evt) + ">";

      if (!as_generator(documentation(2))
                        .generate(sink, evt, context)) return false;
      if (evt.type.is_engaged())
        if (!as_generator(
                scope_tab(2) << "/// <value><see cref=\"" << name_helpers::managed_event_args_name(evt) << "\"/></value>\n"
             ).generate(sink, evt, context)) return false;
      if (!as_generator(
              scope_tab(2) << "event EventHandler" << wrapper_args_type << " " << evt_name << ";\n"
           ).generate(sink, evt, context))
        return false;

      return true;
   }
} const event_declaration {};

struct event_definition_generator
{
   attributes::klass_def const& klass;
   attributes::klass_def const& leaf_klass;
   bool is_inherited_event;

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::event_def const& evt, Context const& context) const
   {
      if (blacklist::is_event_blacklisted(evt, context))
        return true;

      auto library_name = context_find_tag<library_context>(context).actual_library_name(klass.filename);
      std::string managed_evt_name = name_helpers::managed_event_name(evt.name);
      auto const& indent = current_indentation(context);

      bool is_unique = helpers::is_unique_event(evt, leaf_klass);
      bool use_explicit_impl = is_inherited_event && !is_unique;

      // The name of the public event that goes in the public API.
      std::string wrapper_evt_name;
      if (use_explicit_impl)
        wrapper_evt_name = name_helpers::translate_inherited_event_name(evt, klass);
      else
        wrapper_evt_name = managed_evt_name;

      std::string klass_name;
      if (is_inherited_event)
        klass_name = name_helpers::klass_full_interface_name(klass);
      else
        klass_name = name_helpers::klass_concrete_name(klass);

      std::string wrapper_args_type = "EventArgs";
      std::string wrapper_args_template = "";
      std::string event_args = "EventArgs args = EventArgs.Empty;\n";
      std::string event_native_call;

      efl::eina::optional<grammar::attributes::type_def> etype = evt.type;

      if (!etype.is_engaged())
        {
           auto event_call_site_sink = std::back_inserter(event_native_call);
           if (!as_generator(indent.inc().inc().inc() << "CallNativeEventCallback(" << library_name << ", \"_" << utils::to_uppercase(evt.c_name) << "\", IntPtr.Zero, null);\n")
                 .generate(event_call_site_sink, attributes::unused, context))
             return false;
        }
      else
        {
           wrapper_args_type = name_helpers::managed_event_args_name(evt);
           wrapper_args_template = "<" + wrapper_args_type + ">";
           std::string arg_initializer;

           auto arg_initializer_sink = std::back_inserter(arg_initializer);
           auto event_call_site_sink = std::back_inserter(event_native_call);

           auto sub_context = change_indentation(indent.inc().inc(), context);

           if (!as_generator(", info => new " << wrapper_args_type << "{ "
                             << "Arg = ").generate(arg_initializer_sink, attributes::unused, context))
             return false;
           if (!(*etype).original_type.visit(unpack_event_args_visitor<decltype(arg_initializer_sink), decltype(sub_context)>{arg_initializer_sink, &sub_context, *etype}))
             return false;

           if (!(*etype).original_type.visit(pack_event_info_and_call_visitor<decltype(event_call_site_sink), decltype(sub_context)>{event_call_site_sink, &sub_context, *etype, library_name, utils::to_uppercase(evt.c_name)}))
             return false;

           arg_initializer += " }";

           event_args = arg_initializer;
        }

      if(!as_generator(documentation(2)).generate(sink, evt, context))
        return false;
      if (etype.is_engaged())
        if (!as_generator(
                scope_tab(2) << "/// <value><see cref=\"" << wrapper_args_type << "\"/></value>\n"
             ).generate(sink, evt, context)) return false;

      // Visible event declaration. Either a regular class member or an explicit interface implementation.
      if (klass.type == attributes::class_type::interface_ || klass.type == attributes::class_type::mixin)
      {
         // Public event implementation.
         if (!as_generator(
                     scope_tab(2) << (!use_explicit_impl ? "public " : " ") << "event EventHandler" << wrapper_args_template << " " << (use_explicit_impl ? (klass_name + ".") : "") << managed_evt_name << "\n"
                  ).generate(sink, attributes::unused, context))
           return false;
      }
      else // For inheritable classes event handling.
      {
         // We may have inherited an event with the same name as this concrete event, thus
         // the concrete event would "hide" the interface one, requiring the new keyword.
         std::string visibility = "public ";
         if (!is_unique)
           visibility += "new ";

         if (!as_generator(
               scope_tab(2) << visibility << "event EventHandler" << wrapper_args_template << " " << wrapper_evt_name << "\n"
               ).generate(sink, attributes::unused, context))
           return false;
      }

      if (!generate_event_add_remove(sink, evt, event_args, context))
        return false;

      if (!generate_event_trigger(sink, evt, wrapper_evt_name, wrapper_args_type, event_native_call, context))
        return false;

      return true;
   }

   template<typename OutputIterator, typename Context>
   bool generate_event_trigger(OutputIterator sink
                              , attributes::event_def const &evt
                              , std::string const& event_name
                              , std::string const& event_args_type
                              , std::string const& event_native_call
                              , Context const& context) const
   {
      auto library_name = context_find_tag<library_context>(context).actual_library_name(klass.filename);
      std::string upper_c_name = utils::to_uppercase(evt.c_name);
      bool is_concrete = context_find_tag<class_context>(context).current_wrapper_kind == class_context::concrete;

      if (!as_generator(
            scope_tab(2) << "/// <summary>Method to raise event "<< event_name << ".\n"
         ).generate(sink, nullptr, context))
        return false;

      if (!evt.is_beta)
        {
           std::string since = evt.documentation.since;

           if (since.empty())
             {
                auto unit = (const Eolian_Unit*) context_find_tag<eolian_state_context>(context).state;
                attributes::klass_def klass(get_klass(evt.klass, unit), unit);
                since = klass.documentation.since;
             }

           if (!since.empty())
             {

                if (!as_generator(
                      scope_tab(2) << "/// <para>Since EFL " << evt.documentation.since << ".</para>\n"
                    ).generate(sink, nullptr, context))
                  return false;
             }
           else
             {
                EINA_CXX_DOM_LOG_ERR(eolian_mono::domain) << "Event " << evt.name << " of class " << evt.klass.eolian_name
                    << " is stable but has no 'Since' information.";
                // We do not bail out here because there are some cases of this happening upstream.
             }
        }

      // Close summary
      if (!as_generator(scope_tab(2) << "/// </summary>\n").generate(sink, nullptr, context))
        return false;

      if (evt.type.is_engaged())
        {
            if (!as_generator(scope_tab(2) << "/// <param name=\"e\">Event to raise.</param>\n"
                 ).generate(sink, nullptr, context))
              return false;
        }

      if (!as_generator(
            scope_tab(2) << (is_concrete ? "public" : "protected virtual") << " void On" << event_name << "(" << (!evt.type.is_engaged() ? "" : event_args_type + " e") << ")\n"
            << scope_tab(2) << "{\n"
            << event_native_call
            << scope_tab(2) << "}\n\n"
          ).generate(sink, nullptr, context))
       return false;

     return true;
   }

   template<typename OutputIterator, typename Context>
   bool generate_event_add_remove(OutputIterator sink
                                 , attributes::event_def const &evt
                                 , std::string const& event_args
                                 , Context const& context) const
   {
      std::string upper_c_name = utils::to_uppercase(evt.c_name);
      auto unit = (const Eolian_Unit*) context_find_tag<eolian_state_context>(context).state;
      attributes::klass_def klass(get_klass(evt.klass, unit), unit);
      auto library_name = context_find_tag<library_context>(context).actual_library_name(klass.filename);
      return as_generator(
           scope_tab(2) << "{\n"
           << scope_tab(2) << scope_tab << "add\n"
           << scope_tab(2) << scope_tab << "{\n"//evt.type.is_engaged()
           << scope_tab(2) << scope_tab << scope_tab << "Efl.EventCb callerCb = GetInternalEventCallback(value"
           << (evt.type.is_engaged() ? event_args : "") << ");\n"
           << scope_tab(2) << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
           << scope_tab(2) << scope_tab << scope_tab << "AddNativeEventHandler(" << library_name << ", key, callerCb, value);\n"
           << scope_tab(2) << scope_tab << "}\n\n"
           << scope_tab(2) << scope_tab << "remove\n"
           << scope_tab(2) << scope_tab << "{\n"
           << scope_tab(2) << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
           << scope_tab(2) << scope_tab << scope_tab << "RemoveNativeEventHandler(" << library_name << ", key, value);\n"
           << scope_tab(2) << scope_tab << "}\n"
           << scope_tab(2) << "}\n\n"
           ).generate(sink, attributes::unused, context);
   }
};

struct event_definition_parameterized
{
   event_definition_generator operator()(attributes::klass_def const& klass, attributes::klass_def const& leaf_klass) const
   {
      bool is_inherited_event = klass != leaf_klass;
      return {klass, leaf_klass, is_inherited_event};
   }
} const event_definition;

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator<struct ::eolian_mono::event_argument_wrapper_generator> : std::true_type {};
template <>
struct is_generator<struct ::eolian_mono::event_argument_wrapper_generator> : std::true_type {};

template <>
struct is_eager_generator<struct ::eolian_mono::event_declaration_generator> : std::true_type {};
template <>
struct is_generator<struct ::eolian_mono::event_declaration_generator> : std::true_type {};

template <>
struct is_eager_generator<struct ::eolian_mono::event_definition_generator> : std::true_type {};
template <>
struct is_generator<struct ::eolian_mono::event_definition_generator> : std::true_type {};
template <>
struct is_generator<struct ::eolian_mono::event_definition_parameterized> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<struct ::eolian_mono::event_argument_wrapper_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed<struct ::eolian_mono::event_declaration_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed<struct ::eolian_mono::event_definition_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed<struct ::eolian_mono::event_definition_parameterized> : std::integral_constant<int, 1> {};
}
} } }

#endif
