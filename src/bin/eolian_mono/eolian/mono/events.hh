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

      // Structs are usually passed by pointer to events, like having a ptr<> modifier
      if (type.is_ptr || regular.is_struct())
        return as_generator("(" + arg_type + ")Marshal.PtrToStructure(" + arg + ", typeof(" + arg_type + "))")
                 .generate(sink, attributes::unused, *context);

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
           , {"string", [&arg] { return "eina.StringConversion.NativeUtf8ToManagedString(" + arg + ")"; }}
           , {"Eina.Error", [&arg] { return "(eina.Error)Marshal.PtrToStructure(" + arg + ", typeof(eina.Error))"; }}
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
      return as_generator("new " + name_helpers::klass_full_concrete_name(cls) + "(evt.Info)").generate(sink, attributes::unused, *context);
   }
   bool operator()(attributes::complex_type_def const&) const
   {
      return as_generator("UNSUPPORTED").generate(sink, attributes::unused, *context);
   }
};

struct event_argument_wrapper_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::event_def const& evt, Context const& context) const
   {
      efl::eina::optional<grammar::attributes::type_def> etype = evt.type;
      if (!etype.is_engaged())
        return true;

      std::string evt_name = name_helpers::managed_event_name(evt.name);
      std::string arg_type;
      if (!as_generator(type).generate(std::back_inserter(arg_type), *etype, efl::eolian::grammar::context_null()))
        {
           EINA_CXX_DOM_LOG_ERR(eolian_mono::domain) << "Failed to get argument type for event " << evt.name;
           return false;
        }

      return as_generator("///<summary>Event argument wrapper for event " << evt_name << ".</summary>\n"
                          << "public class " << name_helpers::managed_event_args_short_name(evt) << " : EventArgs {\n"
                          << scope_tab << "///<summary>Actual event payload.</summary>\n"
                          << scope_tab << "public " << arg_type << " arg { get; set; }\n"
                          << "}\n"
                 ).generate(sink, attributes::unused, context);
   }
} const event_argument_wrapper {};

struct event_declaration_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::event_def const& evt, Context const& context) const
   {
      std::string wrapper_args_type;
      std::string evt_name = name_helpers::managed_event_name(evt.name);
      std::string evt_args_name = name_helpers::managed_event_args_name(evt);

      efl::eina::optional<grammar::attributes::type_def> etype = evt.type;
      if (etype.is_engaged())
        wrapper_args_type = "<" + evt_args_name + ">";

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
   attributes::klass_def const* klass;
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::event_def const& evt, Context const& context) const
   {
       std::string wrapper_event_name;

       if (klass)
            wrapper_event_name = name_helpers::translate_inherited_event_name(evt, *klass);
       else
            wrapper_event_name = name_helpers::managed_event_name(evt.name);

       return as_generator(scope_tab << scope_tab << "evt_" << wrapper_event_name << "_delegate = "
                        << "new efl.Event_Cb(on_" << wrapper_event_name << "_NativeCallback);\n"
                ).generate(sink, attributes::unused, context);
   }
};

struct event_registration_parameterized
{
   event_registration_generator operator()(attributes::klass_def const* klass=NULL) const
   {
      return {klass};
   }
} const event_registration;

struct event_definition_generator
{
   attributes::klass_def const& klass;
   bool is_inherited_event;

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::event_def const& evt, Context context) const
   {
      std::string managed_evt_name = name_helpers::managed_event_name(evt.name);

      std::string wrapper_evt_name;
      if (is_inherited_event)
        wrapper_evt_name = name_helpers::translate_inherited_event_name(evt, klass);
      else
        wrapper_evt_name = managed_evt_name;

      std::string klass_name_with_dot;
      if (is_inherited_event)
        klass_name_with_dot = name_helpers::klass_full_concrete_name(klass) + ".";
      else
        klass_name_with_dot = "";


      std::string upper_c_name = utils::to_uppercase(evt.c_name);
      std::string wrapper_args_type = "EventArgs";
      std::string wrapper_args_template = "";
      std::string event_args = "EventArgs args = EventArgs.Empty;\n";
      std::string visibility = /*is_inherit_context(context) ? "protected" :*/ "protected";

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

      // Wrapper event declaration
      if(!as_generator(documentation(1)).generate(sink, evt, context))
        return false;

      if(!as_generator(
            // scope_tab << visibility << " event EventHandler" << wrapper_args_template << " " << wrapper_evt_name << ";\n"
            // << 
            scope_tab << "///<summary>Method to raise event "<< wrapper_evt_name << ".</summary>\n"
            << scope_tab << visibility << " void On_" << wrapper_evt_name << "(" << wrapper_args_type << " e)\n"
            << scope_tab << "{\n"
            << scope_tab << scope_tab << "EventHandler" << wrapper_args_template << " evt;\n"
            << scope_tab << scope_tab << "lock (eventLock) {\n"
            << scope_tab << scope_tab << scope_tab << "evt = " << wrapper_evt_name << ";\n"
            << scope_tab << scope_tab << "}\n"
            << scope_tab << scope_tab << "if (evt != null) { evt(this, e); }\n"
            << scope_tab << "}\n"
            << scope_tab << "private void on_" << wrapper_evt_name << "_NativeCallback(System.IntPtr data, ref efl.Event evt)\n"
            << scope_tab << "{\n"
            << scope_tab << scope_tab << event_args
            << scope_tab << scope_tab << "try {\n"
            << scope_tab << scope_tab << scope_tab << "On_" << wrapper_evt_name << "(args);\n"
            << scope_tab << scope_tab <<  "} catch (Exception e) {\n"
            << scope_tab << scope_tab << scope_tab << "eina.Log.Error(e.ToString());\n"
            << scope_tab << scope_tab << scope_tab << "eina.Error.Set(eina.Error.EFL_ERROR);\n"
            << scope_tab << scope_tab << "}\n"
            << scope_tab << "}\n"
            << scope_tab << "efl.Event_Cb evt_" << wrapper_evt_name << "_delegate;\n"
            << scope_tab << visibility << " event EventHandler" << wrapper_args_template << " " << klass_name_with_dot << managed_evt_name << "{\n")
              .generate(sink, NULL, context))
          return false;

       if (!as_generator(
                   scope_tab << scope_tab << "add {\n"
                   << scope_tab << scope_tab << scope_tab << "lock (eventLock) {\n"
                   << scope_tab << scope_tab << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
                   << scope_tab << scope_tab << scope_tab << scope_tab << "if (add_cpp_event_handler(key, this.evt_" << wrapper_evt_name << "_delegate))\n"
                   << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << wrapper_evt_name << " += value;\n"
                   << scope_tab << scope_tab << scope_tab << scope_tab << "else\n"
                   << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error($\"Error adding proxy for event {key}\");\n"
                   << scope_tab << scope_tab << scope_tab << "}\n" // End of lock block
                   << scope_tab << scope_tab << "}\n"
                   << scope_tab << scope_tab << "remove {\n"
                   << scope_tab << scope_tab << scope_tab << "lock (eventLock) {\n"
                   << scope_tab << scope_tab << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
                   << scope_tab << scope_tab << scope_tab << scope_tab << "if (remove_cpp_event_handler(key, this.evt_" << wrapper_evt_name << "_delegate))\n"
                   << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << wrapper_evt_name << " -= value;\n"
                   << scope_tab << scope_tab << scope_tab << scope_tab << "else\n"
                   << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error($\"Error removing proxy for event {key}\");\n"
                   << scope_tab << scope_tab << scope_tab << "}\n" // End of lock block
                   << scope_tab << scope_tab << "}\n"
                   << scope_tab << "}\n")
               .generate(sink, NULL, context))
           return false;

      return true;
   }
};

struct event_definition_parameterized
{
   event_definition_generator operator()(attributes::klass_def const& klass, bool is_inherited_event=false) const
   {
      return {klass, is_inherited_event};
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
