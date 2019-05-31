#ifndef EOLIAN_MONO_EVENTS_HH
#define EOLINA_MONO_EVENTS_HH

#include <iterator>

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "type_impl.hh" // For call_match
#include "name_helpers.hh"
#include "using_decl.hh"

namespace eolian_mono {

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
      std::string const& arg = "evt.Info";
      std::string arg_type = name_helpers::type_full_managed_name(regular);

      if (regular.is_struct())
        {
           // Structs are usually passed by pointer to events, like having a ptr<> modifier
           // Uses implicit conversion from IntPtr
           return as_generator(
                " evt.Info"
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

      if (eina::optional<bool> b = call_match(match_table, filter_func, accept_func))
        return *b;
      else
        return as_generator("default(" + arg_type + ")").generate(sink, attributes::unused, *context);
   }
   bool operator()(grammar::attributes::klass_name const& cls) const
   {
      return as_generator("(Efl.Eo.Globals.CreateWrapperFor(evt.Info) as " + name_helpers::klass_full_concrete_name(cls) + ")").generate(sink, attributes::unused, *context);
   }
   bool operator()(attributes::complex_type_def const&) const
   {
      return as_generator("new " << eolian_mono::type << "(evt.Info, false, false)").generate(sink, type, *context);
   }
};

template<typename OutputIterator, typename Context>
struct pack_event_info_and_call_visitor
{
   mutable OutputIterator sink;
   Context const* context;
   attributes::type_def const& type;

   static auto constexpr native_call = "Efl.Eo.Globals.efl_event_callback_call(this.NativeHandle, desc, info);\n";

   typedef pack_event_info_and_call_visitor<OutputIterator, Context> visitor_type;
   typedef bool result_type;

   bool operator()(grammar::attributes::regular_type_def const& regular) const
   {
      std::string arg_type = name_helpers::type_full_managed_name(regular);

      auto const& indent = current_indentation(*context);

      if (regular.is_struct())
        {
           return as_generator(
                indent << "IntPtr info = Marshal.AllocHGlobal(Marshal.SizeOf(e.arg));\n"
                << indent << "try\n"
                << indent << "{\n"
                << indent << scope_tab << "Marshal.StructureToPtr(e.arg, info, false);\n"
                << indent << scope_tab << this->native_call
                << indent << "}\n"
                << indent << "finally\n"
                << indent << "{\n"
                << indent << scope_tab << "Marshal.FreeHGlobal(info);\n"
                << indent << "}\n"
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
           {"string", [] { return "e.arg"; }}
           , {"stringshare", [] { return "e.arg"; }}
        };

      auto str_accept_func = [&](std::string const& conversion)
        {
           return as_generator(
             indent << "IntPtr info = Eina.StringConversion.ManagedStringToNativeUtf8Alloc(" << conversion << ");\n"
             << indent << "try\n"
             << indent << "{\n"
             << indent << scope_tab << this->native_call
             << indent << "}\n"
             << indent << "finally\n"
             << indent << "{\n"
             << indent << scope_tab << "Eina.MemoryNative.Free(info);\n"
             << indent << "}\n").generate(sink, attributes::unused, *context);
        };

      if (eina::optional<bool> b = call_match(str_table, filter_func, str_accept_func))
        return *b;

      match const value_table [] =
        {
           {"bool", [] { return "e.arg ? (byte) 1 : (byte) 0"; }}
           , {"Eina.Error", [] { return "(int)e.arg"; }}
           , {nullptr, [] { return "e.arg"; }}
        };

      auto value_accept_func = [&](std::string const& conversion)
        {
           return as_generator(
             indent << "IntPtr info = Eina.PrimitiveConversion.ManagedToPointerAlloc(" << conversion << ");\n"
             << indent << "try\n"
             << indent << "{\n"
             << indent << scope_tab << this->native_call
             << indent << "}\n"
             << indent << "finally\n"
             << indent << "{\n"
             << indent << scope_tab << "Marshal.FreeHGlobal(info);\n"
             << indent << "}\n").generate(sink, attributes::unused, *context);
        };

      if (eina::optional<bool> b = call_match(value_table, filter_func, value_accept_func))
        return *b;

      return value_accept_func("e.args");
   }
   bool operator()(grammar::attributes::klass_name const&) const
   {
      auto const& indent = current_indentation(*context);
      return as_generator(indent << "IntPtr info = e.arg.NativeHandle;\n"
                          << indent << this->native_call).generate(sink, attributes::unused, *context);
   }
   bool operator()(attributes::complex_type_def const&) const
   {
      auto const& indent = current_indentation(*context);
      return as_generator(indent << "IntPtr info = e.arg.Handle;\n"
                          << indent << this->native_call).generate(sink, attributes::unused, *context);
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

      return as_generator("///<summary>Event argument wrapper for event <see cref=\""
                          << join_namespaces(evt.klass.namespaces, '.', managed_namespace)
                          << klass_interface_name(evt.klass) << "." << evt_name << "\"/>.</summary>\n"
                          << "public class " << name_helpers::managed_event_args_short_name(evt) << " : EventArgs {\n"
                          << scope_tab << "///<summary>Actual event payload.</summary>\n"
                          << scope_tab << "public " << type << " arg { get; set; }\n"
                          << "}\n"
                 ).generate(sink, *etype, context);
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

      if (!as_generator(
                documentation(1)
                << scope_tab << "event EventHandler" << wrapper_args_type << " " << evt_name << ";\n"
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
           if (!as_generator(indent.inc().inc() << "Efl.Eo.Globals.efl_event_callback_call(this.NativeHandle, desc, IntPtr.Zero);\n")
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

           if (!as_generator(wrapper_args_type << " args = new " << wrapper_args_type << "();\n"
                             << scope_tab(6) << "args.arg = ").generate(arg_initializer_sink, attributes::unused, context))
             return false;
           if (!(*etype).original_type.visit(unpack_event_args_visitor<decltype(arg_initializer_sink), decltype(sub_context)>{arg_initializer_sink, &sub_context, *etype}))
             return false;

           if (!(*etype).original_type.visit(pack_event_info_and_call_visitor<decltype(event_call_site_sink), decltype(sub_context)>{event_call_site_sink, &sub_context, *etype}))
             return false;

           arg_initializer += ";\n";

           event_args = arg_initializer;
        }

      if(!as_generator(documentation(1)).generate(sink, evt, context))
        return false;

      // Visible event declaration. Either a regular class member or an explicit interface implementation.
      if (klass.type == attributes::class_type::interface_ || klass.type == attributes::class_type::mixin)
      {
         // Public event implementation.
         if (!as_generator(
                     scope_tab << (!use_explicit_impl ? "public " : " ") << "event EventHandler" << wrapper_args_template << " " << (use_explicit_impl ? (klass_name + ".") : "") << managed_evt_name << "\n"
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
               scope_tab << visibility << "event EventHandler" << wrapper_args_template << " " << wrapper_evt_name << "\n"
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
      if (!as_generator(
            scope_tab << "///<summary>Method to raise event "<< event_name << ".</summary>\n"
            << scope_tab << "public void On" << event_name << "(" << event_args_type << " e)\n"
            << scope_tab << "{\n"
            << scope_tab << scope_tab << "var key = \"_" << upper_c_name << "\";\n"
            << scope_tab << scope_tab << "IntPtr desc = Efl.EventDescription.GetNative(" << library_name << ", key);\n"
            << scope_tab << scope_tab << "if (desc == IntPtr.Zero)\n"
            << scope_tab << scope_tab << "{\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Failed to get native event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << "return;\n"
            << scope_tab << scope_tab << "}\n\n"
            << event_native_call
            << scope_tab << "}\n"
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
           scope_tab << "{\n"
           << scope_tab << scope_tab << "add\n"
           << scope_tab << scope_tab << "{\n"
           << scope_tab << scope_tab << scope_tab << "lock (eventLock)\n"
           << scope_tab << scope_tab << scope_tab << "{\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "Efl.EventCb callerCb = (IntPtr data, ref Efl.Event.NativeStruct evt) =>\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "{\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "var obj = Efl.Eo.Globals.WrapperSupervisorPtrToManaged(data).Target;\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "if (obj != null)\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "{\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << event_args
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "try\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "{\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "value?.Invoke(obj, args);\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "}\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "catch (Exception e)\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "{\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "Eina.Log.Error(e.ToString());\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "Eina.Error.Set(Eina.Error.UNHANDLED_EXCEPTION);\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "}\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "}\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "};\n\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "AddNativeEventHandler(" << library_name << ", key, callerCb, value);\n"
           << scope_tab << scope_tab << scope_tab << "}\n" // End of lock block
           << scope_tab << scope_tab << "}\n\n"
           << scope_tab << scope_tab << "remove\n"
           << scope_tab << scope_tab << "{\n"
           << scope_tab << scope_tab << scope_tab << "lock (eventLock)\n"
           << scope_tab << scope_tab << scope_tab << "{\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "RemoveNativeEventHandler(" << library_name << ", key, value);\n"
           << scope_tab << scope_tab << scope_tab << "}\n" // End of lock block
           << scope_tab << scope_tab << "}\n"
           << scope_tab << "}\n"
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
