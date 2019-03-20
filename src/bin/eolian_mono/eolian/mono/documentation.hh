#ifndef EOLIAN_MONO_DOCUMENTATION_HPP
#define EOLIAN_MONO_DOCUMENTATION_HPP

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/html_escaped_string.hpp"
#include "using_decl.hh"
#include "name_helpers.hh"
#include "generation_contexts.hh"
#include "blacklist.hh"

#include <Eina.h>

namespace eolian_mono {

struct documentation_generator
{

   int scope_size = 0;

   documentation_generator(int scope_size)
       : scope_size(scope_size) {}


   // Returns the number of parameters (values + keys) that a property method requires
   // Specify if you want the Setter or the Getter method.
   static int property_num_parameters(const ::Eolian_Function *function, ::Eolian_Function_Type ftype)
   {
      Eina_Iterator *itr = ::eolian_property_keys_get(function, ftype);
      Eolian_Function_Parameter *pr;
      int n = 0;
      EINA_ITERATOR_FOREACH(itr, pr) { n++; }
      eina_iterator_free(itr);
      itr = ::eolian_property_values_get(function, ftype);
      EINA_ITERATOR_FOREACH(itr, pr) { n++; }
      eina_iterator_free(itr);
      return n;
   }

   // Turns a function name from EO convention to EFL# convention.
   // The name_tail parameter is the last 4 chars of the original string, which
   // could be ".set" or ".get" and in this case they are ignored by Eolian.
   // We want them to know what the documentation intended to reference.
   static std::string function_conversion(const ::Eolian_Object *klass, const ::Eolian_Function *function, std::string name_tail)
   {
      ::Eolian_Function_Type ftype = ::eolian_function_type_get(function);
      const char* eo_name = ::eolian_function_name_get(function);
      std::string name = name_helpers::managed_namespace(::eolian_object_name_get(klass));
      switch(ftype)
      {
         case ::EOLIAN_METHOD:
           if (blacklist::is_function_blacklisted(
                 ::eolian_function_full_c_name_get(function, ftype))) return "";
           name += ".";
           name += name_helpers::managed_method_name(
             ::eolian_object_short_name_get(klass), eo_name);
           break;
         case ::EOLIAN_PROP_SET:
           name += ".Set";
           name += name_helpers::property_managed_name(eo_name);
           break;
         case ::EOLIAN_PROP_GET:
           name += ".Get";
           name += name_helpers::property_managed_name(eo_name);
           break;
         case ::EOLIAN_PROPERTY:
           {
             int getter_params = property_num_parameters(function, ::EOLIAN_PROP_GET);
             int setter_params = property_num_parameters(function, ::EOLIAN_PROP_SET);
             std::string short_name = name_helpers::property_managed_name(eo_name);
             bool blacklisted = blacklist::is_property_blacklisted(name + "." + short_name);
             // EO properties with keys, with more than one value, or blacklisted, are not
             // converted into C# properties.
             // In these cases we refer to the getter method instead of the property.
             if ((getter_params > 1) || (setter_params > 1) || (blacklisted)) name += ".Get" + short_name;
             else if (name_tail == ".get") name += ".Get" + short_name;
             else if (name_tail == ".set") name += ".Set" + short_name;
             else name += "." + short_name;
           }
           break;
         default:
           break;
      }
      return name;
   }

   static std::string function_conversion(attributes::function_def const& func)
   {
      attributes::klass_def klass(get_klass(func.klass, func.unit), func.unit);
      std::string name = name_helpers::klass_full_concrete_or_interface_name(klass);
      switch (func.type)
      {
          // managed_method_name takes care of reordering the function name so the get/set goes first
          // for properties
          case attributes::function_type::method:
          case attributes::function_type::prop_set:
          case attributes::function_type::prop_get:
            if (blacklist::is_function_blacklisted(func.c_name))return "";
            name += ".";
            name += name_helpers::managed_method_name(klass.eolian_name, func.name);
            break;
          default:
            // No need to deal with property as function_defs are converted to get/set when building a given klass_def.
            break;
      }

      return name;
   }

   // Turns an Eolian reference like @Efl.Input.Pointer.tool into a <see> tag
   static std::string ref_conversion(const ::Eolian_Doc_Token *token, const Eolian_State *state, std::string name_tail)
   {
      const Eolian_Object *data, *data2;
      ::Eolian_Object_Type type =
        ::eolian_doc_token_ref_resolve(token, state, &data, &data2);
      std::string ref;
      switch(type)
      {
         case ::EOLIAN_OBJECT_STRUCT_FIELD:
           ref = name_helpers::managed_namespace(::eolian_object_name_get(data));
           ref += ".";
           ref += ::eolian_object_name_get(data2);
           if (blacklist::is_struct_blacklisted(ref)) return "";
           break;
         case ::EOLIAN_OBJECT_EVENT:
           ref = name_helpers::managed_namespace(::eolian_object_name_get(data));
           ref += ".";
           ref += name_helpers::managed_event_name(::eolian_object_name_get(data2));
           break;
         case ::EOLIAN_OBJECT_ENUM_FIELD:
           ref = name_helpers::managed_namespace(::eolian_object_name_get(data));
           ref += ".";
           ref += name_helpers::enum_field_managed_name(::eolian_object_name_get(data2));
           break;
         case ::EOLIAN_OBJECT_FUNCTION:
           ref += function_conversion(data, (const ::Eolian_Function *)data2, name_tail);
           break;
         case ::EOLIAN_OBJECT_VARIABLE:
           if (::eolian_variable_type_get((::Eolian_Variable *)data) == ::EOLIAN_VAR_CONSTANT)
             {
                auto names = utils::split(name_helpers::managed_namespace(::eolian_object_name_get(data)), '.');
                names.pop_back(); // Remove var name
                ref = name_helpers::join_namespaces(names, '.');
                ref += "Constants.";
                ref += name_helpers::managed_name(::eolian_object_short_name_get(data));
             }
           // Otherwise, do nothing and no <see> tag will be generated. Because, who would
           // reference a global (non-constant) variable in the docs?
           break;
         case ::EOLIAN_OBJECT_UNKNOWN:
           // If the reference cannot be resolved, just return an empty string and
           // it won't be converted into a <see> tag.
           break;
         default:
           ref = name_helpers::managed_namespace(::eolian_object_name_get(data));
           break;
      }
      return ref;
   }

   // Turns EO documentation syntax into C# triple-slash XML comment syntax
   static std::string syntax_conversion(std::string text, const Eolian_State *state)
   {
      std::string new_text, ref;
      ::Eolian_Doc_Token token;
      const char *text_ptr = text.c_str();
      ::eolian_doc_token_init(&token);
      ::Eolian_Doc_Token_Type previous_token_type = ::EOLIAN_DOC_TOKEN_UNKNOWN;
      while ((text_ptr = ::eolian_documentation_tokenize(text_ptr, &token)) != NULL)
        {
           std::string token_text, name_tail;
           char *token_text_cstr = ::eolian_doc_token_text_get(&token);
           if (token_text_cstr)
             {
                token_text = token_text_cstr;
                free(token_text_cstr);
                if (token_text.length() > 4)
                  name_tail = token_text.substr(token_text.length() - 4, 4);
             }
           ::Eolian_Doc_Token_Type token_type = ::eolian_doc_token_type_get(&token);
           switch(token_type)
           {
              case ::EOLIAN_DOC_TOKEN_TEXT:
                // If previous token was a reference and this text token starts with
                // parentheses, remove them, since the reference will be rendered
                // with the parentheses already.
                if ((previous_token_type == ::EOLIAN_DOC_TOKEN_REF) &&
                    (token_text.substr(0, 2)  == "()"))
                  token_text = token_text.substr(2, token_text.length() - 2);
                new_text += token_text;
                break;
              case ::EOLIAN_DOC_TOKEN_REF:
                ref = ref_conversion(&token, state, name_tail);
                if (ref != "")
                  new_text += "<see cref=\"" + ref + "\"/>";
                else
                  // Unresolved references are passed through.
                  // They will appear in the docs as plain text, without link,
                  // but at least they won't be removed by DocFX.
                  new_text += token_text;
                break;
              case ::EOLIAN_DOC_TOKEN_MARK_NOTE:
                new_text += "NOTE: " + token_text;
                break;
              case ::EOLIAN_DOC_TOKEN_MARK_WARNING:
                new_text += "WARNING: " + token_text;
                break;
              case ::EOLIAN_DOC_TOKEN_MARK_REMARK:
                new_text += "REMARK: " + token_text;
                break;
              case ::EOLIAN_DOC_TOKEN_MARK_TODO:
                new_text += "TODO: " + token_text;
                break;
              case ::EOLIAN_DOC_TOKEN_MARKUP_MONOSPACE:
                new_text += "<c>" + token_text + "</c>";
                break;
              default:
                break;
           }
           previous_token_type = token_type;
        }
      return new_text;
   }

   /// Tag generator helpers
   template<typename OutputIterator, typename Context>
   bool generate_tag(OutputIterator sink, std::string const& tag, std::string const &text, Context const& context, std::string tag_params = "") const
   {
      std::string new_text;
      if (!as_generator(html_escaped_string).generate(std::back_inserter(new_text), text, context))
        return false;
      new_text = syntax_conversion( new_text, context_find_tag<eolian_state_context>(context).state );

      std::string tabs;
      as_generator(scope_tab(scope_size) << "/// ").generate (std::back_inserter(tabs), attributes::unused, context);

      std::istringstream ss(new_text);
      std::string para;
      std::string final_text = "<" + tag + tag_params + ">";
      bool first = true;
      while (std::getline(ss, para)) {
        if (first) final_text += para;
        else final_text += "\n" + tabs + para;
        first = false;
      }
      return as_generator(scope_tab(scope_size) << "/// " << final_text << "</" << tag << ">\n").generate(sink, attributes::unused, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_tag_summary(OutputIterator sink, std::string const& text, Context const& context) const
   {
      return generate_tag(sink, "summary", text, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_tag_param(OutputIterator sink, std::string const& name, std::string const& text, Context const& context) const
   {
      return generate_tag(sink, "param", text, context, " name=\"" + name + "\"");
   }

   template<typename OutputIterator, typename Context>
   bool generate_tag_return(OutputIterator sink, std::string const& text, Context const& context) const
   {
      return generate_tag(sink, "returns", text, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_tag_value(OutputIterator sink, std::string const& text, Context const& context) const
   {
      return generate_tag(sink, "value", text, context);
   }

   // Actual exported generators
   template<typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attr, Context const& context) const
   {
       return generate(sink, attr.documentation, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::property_def const& prop, Context const& context) const
   {
       if (!generate(sink, prop.documentation, context))
         return false;

       std::string text;
       if (prop.setter.is_engaged())
         text = prop.setter->parameters[0].documentation.full_text;
       else if (prop.getter.is_engaged())
         text = prop.getter->return_documentation.full_text;
       // If there are no docs at all, do not generate <value> tag
       else return true;

       return generate_tag_value(sink, text, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::function_def const& func, Context const& context) const
   {
       if (func.type == attributes::function_type::prop_get || func.type == attributes::function_type::prop_set)
         return generate_property(sink, func, context);
       else
         return generate_function(sink, func, context);
       return true;
   }

   template<typename OutputIterator, typename Context>
   bool generate_property(OutputIterator sink, attributes::function_def const& func, Context const& context) const
   {

       // First, try the get/set specific documentation
       if (!func.documentation.summary.empty())
         {
            if (!generate(sink, func.documentation, context))
              return false;
         }
       else // fallback to common property documentation
         {
            if (!generate(sink, func.property_documentation, context))
              return false;
         }

       for (auto&& param : func.parameters)
         if (!generate_parameter(sink, param, context))
           return false;

       if (!generate_tag_return(sink, func.return_documentation.full_text, context))
         return false;

       return true;
   }

   template<typename OutputIterator, typename Context>
   bool generate_function(OutputIterator sink, attributes::function_def const& func, Context const& context) const
   {
       if (!generate(sink, func.documentation, context))
         return false;

       for (auto&& param : func.parameters)
         if (!generate_parameter(sink, param, context))
           return false;

       if (!generate_tag_return(sink, func.return_documentation.full_text, context))
         return false;

       return true;
   }

   template<typename OutputIterator, typename Context>
   bool generate_parameter(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      return generate_tag_param(sink, name_helpers::escape_keyword(param.param_name), param.documentation.full_text, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::documentation_def const& doc, Context const& context) const
   {
      std::string str = doc.full_text;
      if (!doc.since.empty())
        str += "\n(Since EFL " + doc.since + ")";
      return generate_tag_summary(sink, str, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::constructor_def const& ctor, Context const& context) const
   {
      // Not sure if this is the best way to generate a reference outside the full doc generator.
      auto unit = (const Eolian_Unit*) context_find_tag<eolian_state_context>(context).state;
      auto func = ctor.function;
      auto eolian_klass = get_klass(func.klass, unit);
      attributes::klass_def klass(eolian_klass, unit);
      std::string summary;

      if (func.type == attributes::function_type::prop_set)
          summary = func.property_documentation.summary;
      else
          summary = func.documentation.summary;

      for (auto &&param : ctor.function.parameters)
        {
          if (!as_generator(
                      scope_tab << "///<param name=\"" << constructor_parameter_name(ctor) << "\">" << summary << " See <see cref=\"" << function_conversion(func) << "\"/></param>\n"
                      ).generate(sink, param, context))
            return false;
        }
      return true;
   }
};

struct documentation_terminal
{
  documentation_generator operator()(int n) const
  {
      return documentation_generator(n);
  }
} const documentation = {};

documentation_generator as_generator(documentation_terminal)
{
    return documentation_generator(0);
}

} // namespace eolian_mono


namespace efl { namespace eolian { namespace grammar {

template<>
struct is_eager_generator<::eolian_mono::documentation_generator> : std::true_type {};
template<>
struct is_generator<::eolian_mono::documentation_generator> : std::true_type {};

template<>
struct is_generator<::eolian_mono::documentation_terminal> : std::true_type {};

namespace type_traits {
template<>
struct attributes_needed<struct ::eolian_mono::documentation_generator> : std::integral_constant<int, 1> {};
template<>
struct attributes_needed<struct ::eolian_mono::documentation_terminal> : std::integral_constant<int, 1> {};
}
} } }

#endif
