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
                " evt.Info;"
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
      const match_table [] =
        {
           {"bool", [&arg] { return arg + " != IntPtr.Zero"; }}
           , {"int", [&arg] { return arg + ".ToInt32()"; }}
           , {"uint", [&arg] { return "(uint)" + arg + ".ToInt32()";}}
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
      return as_generator("new " + name_helpers::klass_full_concrete_name(cls) + "(Efl.Eo.Globals.efl_ref(evt.Info))").generate(sink, attributes::unused, *context);
   }
   bool operator()(attributes::complex_type_def const&) const
   {
      return as_generator("new " << eolian_mono::type << "(evt.Info, false, false)").generate(sink, type, *context);
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

struct event_registration_generator
{
   attributes::klass_def const& klass;
   attributes::klass_def const& leaf_klass;
   bool is_inherited_event;

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::event_def const& evt, Context const& context) const
   {
       std::string wrapper_event_name;

      if (blacklist::is_event_blacklisted(evt, context))
        return true;

       if (is_inherited_event && !helpers::is_unique_event(evt, leaf_klass))
            wrapper_event_name = name_helpers::translate_inherited_event_name(evt, klass);
       else
            wrapper_event_name = name_helpers::managed_event_name(evt.name);

       return as_generator(scope_tab << scope_tab << "evt_" << wrapper_event_name << "_delegate = "
                        << "new Efl.EventCb(on_" << wrapper_event_name << "_NativeCallback);\n"
                ).generate(sink, attributes::unused, context);
   }
};

struct event_registration_parameterized
{
   event_registration_generator operator()(attributes::klass_def const& klass, attributes::klass_def const& leaf_klass) const
   {
      bool is_inherited_event = klass != leaf_klass;
      return {klass, leaf_klass, is_inherited_event};
   }
} const event_registration;

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

      efl::eina::optional<grammar::attributes::type_def> etype = evt.type;

      if (etype.is_engaged())
        {
           wrapper_args_type = name_helpers::managed_event_args_name(evt);
           wrapper_args_template = "<" + wrapper_args_type + ">";
           std::string arg_initializer = wrapper_args_type + " args = new " + wrapper_args_type + "();\n";

           arg_initializer += "      args.arg = ";

           auto arg_initializer_sink = std::back_inserter(arg_initializer);

           if (!(*etype).original_type.visit(unpack_event_args_visitor<decltype(arg_initializer_sink), Context>{arg_initializer_sink, &context, *etype}))
             return false;

           arg_initializer += ";\n";

           event_args = arg_initializer;
        }

      if(!as_generator("private static object " << wrapper_evt_name << "Key = new object();\n")
              .generate(sink, attributes::unused, context))
        return false;

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

      if (!generate_event_add_remove(sink, evt, wrapper_evt_name, context))
        return false;

      if (!generate_event_trigger(sink, wrapper_evt_name, wrapper_args_type, wrapper_args_template, context))
        return false;

      // Store the delegate for this event in this instance. This is initialized in RegisterEventProxies()
      // We can't initialize them directly here as they depend on the member methods being valid (i.e.
      // the constructor being called).
      if (!as_generator(scope_tab << "Efl.EventCb evt_" << wrapper_evt_name << "_delegate;\n").generate(sink, attributes::unused, context))
        return false;

      // Callback to be given to C's callback_priority_add
      if (!as_generator(
            scope_tab << "private void on_" << wrapper_evt_name << "_NativeCallback(System.IntPtr data, ref Efl.Event.NativeStruct evt)\n"
            << scope_tab << "{\n"
            << scope_tab << scope_tab << event_args
            << scope_tab << scope_tab << "try {\n"
            << scope_tab << scope_tab << scope_tab << "On_" << wrapper_evt_name << "(args);\n"
            << scope_tab << scope_tab <<  "} catch (Exception e) {\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Log.Error(e.ToString());\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Error.Set(Eina.Error.UNHANDLED_EXCEPTION);\n"
            << scope_tab << scope_tab << "}\n"
            << scope_tab << "}\n\n"
            ).generate(sink, attributes::unused, context))
        return false;

      return true;
   }

   template<typename OutputIterator, typename Context>
   bool generate_event_trigger(OutputIterator sink
                              , std::string const& event_name
                              , std::string const& event_args_type
                              , std::string const& event_template_args
                              , Context const& context) const
   {
      auto delegate_type = "EventHandler" + event_template_args;
      if (!as_generator(
            scope_tab << "///<summary>Method to raise event "<< event_name << ".</summary>\n"
            << scope_tab << "public void On_" << event_name << "(" << event_args_type << " e)\n"
            << scope_tab << "{\n"
            << scope_tab << scope_tab << delegate_type << " evt;\n"
            << scope_tab << scope_tab << "lock (eventLock) {\n"
            << scope_tab << scope_tab << "evt = (" << delegate_type << ")eventHandlers[" << event_name << "Key];\n"
            << scope_tab << scope_tab << "}\n"
            << scope_tab << scope_tab << "evt?.Invoke(this, e);\n"
            << scope_tab << "}\n"
          ).generate(sink, nullptr, context))
       return false;

     return true;
   }

   template<typename OutputIterator, typename Context>
   bool generate_event_add_remove(OutputIterator sink, attributes::event_def const &evt, const std::string& event_name, Context const& context) const
   {
      std::string upper_c_name = utils::to_uppercase(evt.c_name);
      auto unit = (const Eolian_Unit*) context_find_tag<eolian_state_context>(context).state;
      attributes::klass_def klass(get_klass(evt.klass, unit), unit);
      auto library_name = context_find_tag<library_context>(context).actual_library_name(klass.filename);
      return as_generator(
           scope_tab << "{\n"
           << scope_tab << scope_tab << "add {\n"
           << scope_tab << scope_tab << scope_tab << "lock (eventLock) {\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "if (AddNativeEventHandler(" << library_name << ", key, this.evt_" << event_name << "_delegate)) {\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "eventHandlers.AddHandler(" << event_name << "Key , value);\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "} else\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Error adding proxy for event {key}\");\n"
           << scope_tab << scope_tab << scope_tab << "}\n" // End of lock block
           << scope_tab << scope_tab << "}\n"
           << scope_tab << scope_tab << "remove {\n"
           << scope_tab << scope_tab << scope_tab << "lock (eventLock) {\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "if (RemoveNativeEventHandler(key, this.evt_" << event_name << "_delegate)) { \n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "eventHandlers.RemoveHandler(" << event_name << "Key , value);\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << "} else\n"
           << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Error removing proxy for event {key}\");\n"
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
struct is_eager_generator<struct ::eolian_mono::event_registration_generator> : std::true_type {};
template <>
struct is_generator<struct ::eolian_mono::event_registration_generator> : std::true_type {};
template <>
struct is_generator<struct ::eolian_mono::event_registration_parameterized> : std::true_type {};

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
struct attributes_needed<struct ::eolian_mono::event_registration_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed<struct ::eolian_mono::event_registration_parameterized> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed<struct ::eolian_mono::event_definition_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed<struct ::eolian_mono::event_definition_parameterized> : std::integral_constant<int, 1> {};
}
} } }

#endif
