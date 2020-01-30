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
#ifndef EOLIAN_MONO_DOCUMENTATION_HPP
#define EOLIAN_MONO_DOCUMENTATION_HPP

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/html_escaped_string.hpp"
#include "using_decl.hh"
#include "name_helpers.hh"
#include "generation_contexts.hh"
#include "blacklist.hh"
#include "property_definition.hh"

#include <Eina.h>

static const std::string BETA_REF_SUFFIX = " (object still in beta stage)";
static const std::string BETA_CLASS_REMARK = "This is a \\<b\\>BETA\\</b\\> class. It can be modified or removed in the future. Do not use it for product development.";
static const std::string BETA_PROPERTY_REMARK = "\n\n\\<b\\>This is a BETA property\\</b\\>. It can be modified or removed in the future. Do not use it for product development.";
static const std::string BETA_METHOD_REMARK = "\n\n\\<b\\>This is a BETA method\\</b\\>. It can be modified or removed in the future. Do not use it for product development.";

namespace eolian_mono {

struct documentation_generator
{

   int scope_size = 0;

   documentation_generator(int scope_size = 0)
       : scope_size(scope_size) {}


   // Returns the number of keys that a property method requires
   // Specify if you want the Setter or the Getter method.
   static int property_num_keys(const ::Eolian_Function *function, ::Eolian_Function_Type ftype)
   {
      Eina_Iterator *itr = ::eolian_property_keys_get(function, ftype);
      Eolian_Function_Parameter *pr;
      int n = 0;
      EINA_ITERATOR_FOREACH(itr, pr) { n++; }
      eina_iterator_free(itr);
      return n;
   }

   // Gets the eolian ref for the given class, prepending I for interfaces.
   static std::string object_ref_conversion(const Eolian_Object *cls)
   {
      auto klass_type = ::eolian_class_type_get((const Eolian_Class *)cls);
      auto full_eolian_name = name_helpers::managed_namespace(::eolian_object_name_get(cls));
      if (klass_type == EOLIAN_CLASS_MIXIN || klass_type == EOLIAN_CLASS_INTERFACE)
        {
           size_t pos = full_eolian_name.rfind(".");
           if (pos == std::string::npos)
             pos = 0;
           else
             pos++;
           full_eolian_name.insert(pos, "I");
        }
      return full_eolian_name;
   }


   // Turns a function name from EO convention to EFL# convention.
   // The name_tail parameter is the last 4 chars of the original string, which
   // could be ".set" or ".get" and in this case they are ignored by Eolian.
   // We want them to know what the documentation intended to reference.
   template <typename Context>
   static std::string function_conversion(const ::Eolian_Object *klass, const ::Eolian_Function *function, std::string name_tail, Context const& context)
   {
      ::Eolian_Function_Type ftype = ::eolian_function_type_get(function);
      const char* eo_name = ::eolian_function_name_get(function);
      std::string name = object_ref_conversion(klass);

      // Klass is needed to check the property naming rules
      attributes::klass_def klass_d((const ::Eolian_Class *)klass, eolian_object_unit_get(klass));

      // Comment the block below to enable @see reference conversion for non-public interface members.
      // As they are not generated, this causes a doc warning that fails the build, but can be useful to track
      // public methods referencing protected stuff.
      if (ftype != EOLIAN_PROPERTY)
        {
           bool is_func_public = ::eolian_function_scope_get(function, ftype) == EOLIAN_SCOPE_PUBLIC;

           if (helpers::is_managed_interface(klass_d) && !is_func_public)
             return "";
        }
      else
        {
           bool is_get_public = ::eolian_function_scope_get(function, EOLIAN_PROP_GET) == EOLIAN_SCOPE_PUBLIC;
           bool is_set_public = ::eolian_function_scope_get(function, EOLIAN_PROP_SET) == EOLIAN_SCOPE_PUBLIC;

           if (helpers::is_managed_interface(klass_d) && !(is_get_public || is_set_public))
             return "";
        }

      switch(ftype)
      {
         case ::EOLIAN_METHOD:
           if (blacklist::is_function_blacklisted(
                 ::eolian_function_full_c_name_get(function, ftype))) return "";
           name += ".";
           name += name_helpers::managed_method_name({function, ftype, NULL, eolian_object_unit_get(EOLIAN_OBJECT(function))});
           break;
         case ::EOLIAN_PROP_SET:
         case ::EOLIAN_PROP_GET:
         case ::EOLIAN_PROPERTY:
         {
           efl::eina::optional<attributes::function_def> getter_func;
           efl::eina::optional<attributes::function_def> setter_func;
           auto unit = (const Eolian_Unit*) context_find_tag<eolian_state_context>(context).state;
           if (ftype == ::EOLIAN_PROPERTY || ftype == ::EOLIAN_PROP_GET)
             getter_func = attributes::function_def{function, ::EOLIAN_PROP_GET, nullptr, unit};
           if (ftype == ::EOLIAN_PROPERTY || ftype == ::EOLIAN_PROP_SET)
             setter_func = attributes::function_def{function, ::EOLIAN_PROP_SET, nullptr, unit};      
           attributes::property_def property{function, getter_func, setter_func, unit};

           std::string short_name = name_helpers::property_managed_name(klass_d, eo_name);
           class_context::wrapper_kind klass_kind;
           if (helpers::is_managed_interface(klass_d))
             klass_kind = class_context::interface;
           else
             klass_kind = class_context::inherit;
           auto my_context = grammar::context_replace_tag(class_context{klass_kind}, context);

           if (name_tail == ".get")
           {
             if (property_generate_wrapper_getter (property, my_context))
             {
               name += "." + short_name;
             }
             else
             {
               name += ".Get" + short_name;
             }
           }
           else if (name_tail == ".set")
           {
             if (property_generate_wrapper_setter (property, my_context))
             {
               name += "." + short_name;
             }
             else
             {
               name += ".Set" + short_name;
             }
           }
           else
           {
             switch (ftype)
             {
             case ::EOLIAN_PROPERTY:
             case ::EOLIAN_PROP_GET:
               if (property_generate_wrapper_getter (property, my_context))
                 name += "." + short_name;
               else 
                 name += ".Get" + short_name;
               break;
             default:
               name += ".Set" + short_name;
               break;
             }
           }
         }
           break;
         default:
           break;
      }
      return name;
   }

   template <typename Context>
   static std::string function_conversion(attributes::function_def const& func, Context const& context)
   {
      // This function is called only from the constructor reference conversion, so it does not
      // need to check whether this function non-public in a interface returning an empty reference (yet).
      std::string name = name_helpers::klass_full_concrete_or_interface_name(func.klass);
      switch (func.type)
      {
      // managed_method_name takes care of reordering the function name so the get/set goes first
      // for properties
      case attributes::function_type::method:
        if (blacklist::is_function_blacklisted(func.c_name))return "";
        if (!name.empty()) name += ".";
        name += name_helpers::managed_method_name(func);
        break;
      case attributes::function_type::prop_set:
      case attributes::function_type::prop_get:
      case attributes::function_type::property:
      {
        auto unit = (const Eolian_Unit*) context_find_tag<eolian_state_context>(context).state;
        auto klass = get_klass(func.klass, unit);
        attributes::klass_def klass_d(klass, unit);

        auto eo_name = func.name;
        auto prop_name = eo_name;
        if (prop_name.size () > 4
            &&
            ( prop_name.substr(prop_name.size() - 4) == "_set"
              || prop_name.substr(prop_name.size() - 4) == "_get"))
        {
          prop_name = prop_name.substr(0, prop_name.size() - 4);
        }
        std::string short_name = name_helpers::property_managed_name(klass_d, prop_name);
        assert (prop_name.size() <= 4 ||
                (prop_name.substr(prop_name.size() - 4) != "_set"
                 && prop_name.substr(prop_name.size() - 4) != "_get"));
        assert (short_name.size() <= 3 ||
                (short_name.substr(short_name.size() - 3) != "Set"
                 && short_name.substr(short_name.size() - 3) != "Get"));

        // We need to replace the current class context with the context
        // from the class that originated this property.
        class_context::wrapper_kind klass_kind;
        if (helpers::is_managed_interface(klass_d))
          klass_kind = class_context::interface;
        else
          klass_kind = class_context::inherit;

        auto my_context = grammar::context_replace_tag(class_context{klass_kind}, context);

        auto function = eolian_class_function_by_name_get (klass, prop_name.c_str(), EOLIAN_PROPERTY);
        attributes::function_def getter_func{function, ::EOLIAN_PROP_GET, nullptr, unit};
        attributes::function_def setter_func{function, ::EOLIAN_PROP_SET, nullptr, unit};
        attributes::property_def prop{function, getter_func, setter_func, unit};

        if (func.type == attributes::function_type::prop_get || func.type == attributes::function_type::property)
        {
          if (property_generate_wrapper_getter (prop, my_context))
            name += "." + short_name;
          else
            name += ".Get" + short_name;
        }
        else if (func.type == attributes::function_type::prop_set)
        {
          if (property_generate_wrapper_setter (prop, my_context))
            name += "." + short_name;
          else
            name += ".Set" + short_name;
        }
      }
      break;
      default:
        // No need to deal with property as function_defs are converted to get/set when building a given klass_def.
        break;
      }

      return name;
   }

   // Turns an Eolian reference like @Efl.Input.Pointer.tool into a <see> tag
   template <typename Context>
   static std::string ref_conversion(const ::Eolian_Doc_Token *token, const Eolian_State *state, std::string name_tail,
                                     bool want_beta, Context const& context)
   {
      const Eolian_Object *data, *data2;
      ::Eolian_Object_Type type =
        ::eolian_doc_token_ref_resolve(token, state, &data, &data2);
      std::string ref;
      bool is_beta = false;
      switch(type)
      {
         case ::EOLIAN_OBJECT_STRUCT_FIELD:
           ref = name_helpers::managed_namespace(::eolian_object_name_get(data));
           ref += ".";
           ref += ::eolian_object_name_get(data2);
           is_beta = eolian_object_is_beta(data) || eolian_object_is_beta(data2);
           if (blacklist::is_struct_blacklisted(ref)) return "";
           break;
         case ::EOLIAN_OBJECT_EVENT:
           ref = object_ref_conversion(data);
           ref += ".";
           ref += name_helpers::managed_event_name(::eolian_object_name_get(data2));
           is_beta = eolian_object_is_beta(data) || eolian_object_is_beta(data2);
           break;
         case ::EOLIAN_OBJECT_ENUM_FIELD:
           ref = name_helpers::managed_namespace(::eolian_object_name_get(data));
           ref += ".";
           ref += name_helpers::enum_field_managed_name(::eolian_object_name_get(data2));
           is_beta = eolian_object_is_beta(data) || eolian_object_is_beta(data2);
           break;
         case ::EOLIAN_OBJECT_FUNCTION:
           ref += function_conversion(data, (const ::Eolian_Function *)data2, name_tail, context);
           is_beta = eolian_object_is_beta(data) || eolian_object_is_beta(data2);
           break;
         case ::EOLIAN_OBJECT_CONSTANT:
           {
              auto names = utils::split(name_helpers::managed_namespace(::eolian_object_name_get(data)), '.');
              names.pop_back(); // Remove var name
              ref = name_helpers::join_namespaces(names, '.');
              ref += "Constants.";
              ref += name_helpers::managed_name(::eolian_object_short_name_get(data));
           }
           break;
         case ::EOLIAN_OBJECT_UNKNOWN:
           // If the reference cannot be resolved, just return an empty string and
           // it won't be converted into a <see> tag.
           break;
         case ::EOLIAN_OBJECT_CLASS:
           ref = object_ref_conversion(data);
           is_beta = eolian_object_is_beta(data);
           break;
         default:
           ref = name_helpers::managed_namespace(::eolian_object_name_get(data));
           is_beta = eolian_object_is_beta(data);
           break;
      }

      if (!ref.empty() && !want_beta && is_beta)
        ref += BETA_REF_SUFFIX;
      return ref;
   }

  // Turns EO documentation syntax into C# triple-slash XML comment syntax
   template <typename Context>
   static std::string syntax_conversion(std::string text, const Eolian_State *state, bool want_beta, Context const& context)
   {
      std::string new_text, ref;
      ::Eolian_Doc_Token_Type previous_token_type = ::EOLIAN_DOC_TOKEN_UNKNOWN;
      ::Eina_List *paragraphs = ::eolian_documentation_string_split(text.c_str());
      if (!paragraphs) return new_text;
      ::Eina_List *data = paragraphs;
      // For every paragraph
      do
        {
           char *par = (char *)::eina_list_data_get(data);
           const char *text_ptr = par;
           ::Eolian_Doc_Token token;
           ::eolian_doc_token_init(&token);
           // For every token inside the paragraph
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
                     ref = ref_conversion(&token, state, name_tail, want_beta, context);
                     if (ref != "")
                       {
                          if (utils::ends_with(ref, BETA_REF_SUFFIX))
                            new_text += "<span class=\"text-muted\">" + ref + "</span>";
                          else
                            new_text += "<see cref=\"" + ref + "\"/>";
                       }
                     else
                       // Unresolved references are passed through.
                       // They will appear in the docs as plain text, without link,
                       // but at least they won't be removed by DocFX.
                       new_text += token_text;
                     break;
                   case ::EOLIAN_DOC_TOKEN_MARK_NOTE:
                     new_text += "<b>NOTE: </b>";
                     break;
                   case ::EOLIAN_DOC_TOKEN_MARK_WARNING:
                     new_text += "<b>WARNING: </b>";
                     break;
                   case ::EOLIAN_DOC_TOKEN_MARK_REMARK:
                     new_text += "<b>REMARK: </b>";
                     break;
                   case ::EOLIAN_DOC_TOKEN_MARK_TODO:
                     new_text += "<b>TODO: </b>";
                     break;
                   case ::EOLIAN_DOC_TOKEN_MARKUP_MONOSPACE:
                     new_text += "<c>" + token_text + "</c>";
                     break;
                   default:
                     break;
                }
                previous_token_type = token_type;
              }
           // Free this paragraph
           free(par);
           // Fetch the next paragraph
           data = ::eina_list_next(data);
           // If there's another paragraph afterwards, separate them with a blank line
           if (data) new_text += "\n\n";
        } while (data);
      ::eina_list_free(paragraphs);
      return new_text;
   }

   /// Tag generator helpers
   template<typename OutputIterator, typename Context>
   bool generate_opening_tag(OutputIterator sink, std::string const& tag, Context const& context, std::string tag_params = "") const
   {
      auto tag_separator = tag_params.empty() ? "" : " ";

      return as_generator("<" << tag << tag_separator << tag_params << ">").generate(sink, attributes::unused, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_closing_tag(OutputIterator sink, std::string const& tag, Context const& context) const
   {
      return as_generator("</" << tag << ">").generate(sink, attributes::unused, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_escaped_content(OutputIterator sink, std::string const &text, Context const& context) const
   {
      std::string new_text;
      if (!as_generator(html_escaped_string).generate(std::back_inserter(new_text), text, context))
        return false;
      auto options = context_find_tag<options_context>(context);
      new_text = syntax_conversion( new_text, context_find_tag<eolian_state_context>(context).state, options.want_beta, context);

      std::string tabs;
      as_generator(scope_tab(scope_size) << "/// ").generate (std::back_inserter(tabs), attributes::unused, context);

      std::istringstream ss(new_text);
      std::string para;
      std::string final_text;
      bool first = true;
      while (std::getline(ss, para)) {
        if (first) final_text += para;
        else final_text += "\n" + tabs + para;
        first = false;
      }
      return as_generator(final_text).generate(sink, attributes::unused, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_tag(OutputIterator sink, std::string const& tag, std::string const &text, Context const& context, std::string tag_params = "") const
   {
      if (text == "")
        return true;

      if (!as_generator(scope_tab(scope_size) << "/// ").generate(sink, attributes::unused, context)) return false;
      if (!generate_opening_tag(sink, tag, context, tag_params)) return false;
      if (!generate_escaped_content(sink, text, context)) return false;
      if (!generate_closing_tag(sink, tag, context)) return false;
      return as_generator("\n").generate(sink, attributes::unused, context);
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

   template<typename OutputIterator, typename Context>
   bool generate_tag_example(OutputIterator sink, std::string const& full_object_name, Context const& context) const
   {
      auto options = efl::eolian::grammar::context_find_tag<options_context>(context);
      // Example embedding not requested
      if (options.examples_dir.empty()) return true;
      bool is_plain_code = false;
      std::string file_name = options.examples_dir + full_object_name + ".xml";
      std::ifstream exfile(file_name);
      if (!exfile.good())
        {
           // There is no example XML file for this class, try a CS file
           file_name = options.examples_dir + full_object_name + ".cs";
           exfile.open(file_name);
           // There are no example files for this class or method, just return
           if (!exfile.good()) return true;
           is_plain_code = true;
        }
      std::stringstream example_buff;
      // Start with a newline so the first line renders with same indentation as the rest
      example_buff << std::endl << exfile.rdbuf();

      if (!as_generator(scope_tab(scope_size) << "/// ").generate(sink, attributes::unused, context)) return false;
      if (is_plain_code)
        {
           if (!generate_opening_tag(sink, "example", context)) return false;
           if (!generate_opening_tag(sink, "code", context)) return false;
        }
      if (!generate_escaped_content(sink, example_buff.str(), context)) return false;
      if (is_plain_code)
        {
           if (!generate_closing_tag(sink, "code", context)) return false;
           if (!generate_closing_tag(sink, "example", context)) return false;
        }
      return as_generator("\n").generate(sink, attributes::unused, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_all_tag_examples(OutputIterator sink, std::string const & full_class_name, std::string const& object_name, Context const& context) const
   {
      // Take example from derived class
      auto derived_klass = efl::eolian::grammar::context_find_tag<class_context>(context);
      std::string derived_full_name =
        derived_klass.name.empty() ? object_name : derived_klass.name + "." + object_name;
      std::string base_full_name =
        full_class_name.empty() ? object_name : full_class_name + "." + object_name;
      if (!derived_klass.name.empty())
        {
           if (!generate_tag_example(sink, derived_full_name, context)) return false;
        }
      if (derived_full_name.compare(base_full_name) == 0) return true;
      // Take example from base class
      return generate_tag_example(sink, base_full_name, context);
   }

   // Actual exported generators
   template<typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attr, Context const& context) const
   {
       return generate(sink, attr.documentation, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& klass, Context const& context) const
   {
       if (!generate(sink, klass.documentation, context)) return false;

       if (klass.is_beta)
         {
            if (!generate_tag(sink, "remarks", BETA_CLASS_REMARK, context)) return false;
         }

       std::string klass_name = name_helpers::klass_full_concrete_or_interface_name(klass);
       return generate_tag_example(sink, klass_name, context);
   }

   /*! Generates documentation for tuple-value properties. 
    * 
    * Example:
    * 
    * A tuple containing the following information:
    * <list type="bullet">
    * <item><description><c>a</c>: Parameter a.</description></item>
    * <item><description><c>b</c>: Parameter b.</description></item>
    * </list>
    * 
    * \param index_names If true, tuple items are referenced by their index. If
    * false, they are referenced by their names instead.
    */
   template<typename OutputIterator, typename Context>
   bool generate_tuple_parameters_doc(OutputIterator sink,
                                    std::vector<attributes::parameter_def> const& parameters,
                                    Context const& context,
                                    bool numbered_refs = false) const
   {
       if (!(as_generator(scope_tab(scope_size) << "/// ")
              .generate(sink, attributes::unused, context)
             && as_generator(
                 "A tuple containing the following information:\n"
                 << scope_tab(scope_size) << "/// "
                ).generate(sink, attributes::unused, context)
             && generate_opening_tag(sink, "list", context, "type=\"bullet\"")
             && as_generator("\n" << scope_tab(scope_size) << "/// ")
                .generate(sink, attributes::unused, context)))
         return false;

       auto i = 0u;
       for (auto const& param: parameters)
         {
           auto name = param.param_name;
           if (!(generate_opening_tag(sink, "item", context)
                 && generate_opening_tag(sink, "description", context)
                 && generate_opening_tag(sink, "c", context)
                 && as_generator(name).generate(sink, attributes::unused, context)
                 && generate_closing_tag(sink, "c", context)))
               return false;

           if (numbered_refs && !(
                 as_generator(" (").generate(sink, attributes::unused, context)
                 && generate_opening_tag(sink, "c", context)
                 && as_generator("Item" + std::to_string(i)).generate(sink, attributes::unused, context)
                 && generate_closing_tag(sink, "c", context)
                 && as_generator(")").generate(sink, attributes::unused, context)))
               return false;

           if (!(generate_escaped_content(sink, ": " + param.documentation.full_text, context)
                 && generate_closing_tag(sink, "description", context)
                 && generate_closing_tag(sink, "item", context)
                 && as_generator("\n" << scope_tab(scope_size) << "/// ")
                    .generate(sink, attributes::unused, context)))
             return false;
            ++i;
         }

       return generate_closing_tag(sink, "list", context)
              && as_generator("\n")
                 .generate(sink, attributes::unused, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::property_def const& prop, Context const& context) const
   {
       // Generate docs by merging Property, Accessor, Since and Beta pieces.
       std::string text = prop.documentation.full_text;
       if (!prop.documentation.since.empty())
         text += "\\<br/\\>\nSince EFL " + prop.documentation.since + ".";
       if (prop.setter.is_engaged() && !prop.setter->documentation.full_text.empty())
         text += "\\<br/\\>\n\\<b\\>Note on writing:\\</b\\> " + prop.setter->documentation.full_text;
       if (prop.getter.is_engaged() && !prop.getter->documentation.full_text.empty())
         text += "\\<br/\\>\n\\<b\\>Note on reading:\\</b\\> " + prop.getter->documentation.full_text;
       if (!prop.klass.is_beta)
         {
            if ((prop.setter.is_engaged() && prop.setter->is_beta) ||
                (prop.getter.is_engaged() && prop.getter->is_beta))
              text += BETA_PROPERTY_REMARK;
         }
       if (!generate_tag_summary(sink, text, context))
              return false;

       text = "";
       if (prop.setter.is_engaged())
         {
            if (prop.setter.is_engaged() && prop.setter->values.size() > 1u)
              {
                  if (!(
                    as_generator(scope_tab(scope_size) << "/// ")
                     .generate(sink, attributes::unused, context)
                    && generate_opening_tag(sink, "value", context)
                    && as_generator("\n")
                       .generate(sink, attributes::unused, context)
                    && generate_tuple_parameters_doc(sink, prop.setter->parameters, context, true)
                    && as_generator(scope_tab(scope_size) << "/// ")
                     .generate(sink, attributes::unused, context)
                    && generate_closing_tag(sink, "value", context)
                    && as_generator("\n")
                       .generate(sink, attributes::unused, context)
                    ))
                    return false;
              }
            else
              text = prop.setter->parameters[0].documentation.full_text;
         }
       else if (prop.getter.is_engaged())
         text = prop.getter->return_documentation.full_text;
       // If there are no docs at all, do not generate <value> tag
       if (!text.empty())
         if (!generate_tag_value(
             sink,
             text,
             context)) return false;

       return generate_all_tag_examples(sink,
                                        name_helpers::klass_full_concrete_or_interface_name(prop.klass),
                                        name_helpers::property_managed_name(prop),
                                        context);
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
       if (!func.documentation.full_text.empty() ||
           !func.property_documentation.full_text.empty())
         {
            // Generate docs by merging Property, Accessor, Since and Beta pieces.
            std::string text = func.property_documentation.full_text;
            if (!func.property_documentation.since.empty())
              text += "\\<br/\\>\nSince EFL " + func.property_documentation.since + ".";
            if (!func.documentation.full_text.empty())
              text += "\\<br/\\>\n\\<b\\>Note:\\</b\\> " + func.documentation.full_text;
            if (!func.klass.is_beta && func.is_beta)
              text += BETA_METHOD_REMARK;
            if (!generate_tag_summary(sink, text, context))
              return false;
         }

       for (auto&& param : func.parameters)
         if (!generate_parameter(sink, param, context))
           return false;

       if (!generate_tag_return(sink, func.return_documentation.full_text, context))
         return false;

       return generate_all_tag_examples(sink,
                                        name_helpers::klass_full_concrete_or_interface_name(func.klass),
                                        name_helpers::managed_method_name(func),
                                        context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_function(OutputIterator sink, attributes::function_def const& func, Context const& context) const
   {
       std::string tail_text = "";
       if (!func.klass.is_beta && func.is_beta)
         {
            tail_text = BETA_METHOD_REMARK;
         }

       if (!generate(sink, func.documentation, context, tail_text))
         return false;

       for (auto&& param : func.parameters)
         if (!generate_parameter(sink, param, context))
           return false;

       if (!generate_tag_return(sink, func.return_documentation.full_text, context))
         return false;

       return generate_all_tag_examples(sink,
                                        name_helpers::klass_full_concrete_or_interface_name(func.klass),
                                        name_helpers::managed_method_name(func),
                                        context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_parameter(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      auto text = param.documentation.full_text;
      if (param.default_value.is_engaged())
      {
          auto value = param.default_value->serialized;

          if (param.default_value->is_name_ref)
            {
               value = name_helpers::full_managed_name(value);
               text += "\\<br/\\>The default value is \\<see cref=\\\"" + value + "\\\"/\\>.";
            }
          else
            {
               text += "\\<br/\\>The default value is \\<c\\>" + value + "\\</c\\>.";
            }
      }
      return generate_tag_param(sink, name_helpers::escape_keyword(param.param_name), text, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::documentation_def const& doc, Context const& context, std::string tail_text = "") const
   {
      std::string str = doc.full_text;
      if (!doc.since.empty())
        str += "\\<br/\\>Since EFL " + doc.since + ".";
      str += tail_text;
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
          auto ref = function_conversion(func, context);

          if (!context_find_tag<options_context>(context).want_beta && func.is_beta)
            {
               ref += BETA_REF_SUFFIX;
               ref = "<span class=\"text-muted\">" + ref + "</span>";
            }
          else
            ref = "<see cref=\"" + ref + "\" />";

          if (!as_generator(scope_tab(scope_size) << "/// <param name=\"" << constructor_parameter_name(ctor) << "\">" << summary << " See " << ref <<  "</param>\n")
                            .generate(sink, param, context))
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

/// Escape a single string, HTML-escaping and converting the syntax
struct documentation_string_generator
{
  template<typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, std::string const& text, Context const& context) const
  {
      std::string escaped;
      if (!as_generator(html_escaped_string).generate(std::back_inserter(escaped), text, context))
        return false;

      auto options = context_find_tag<options_context>(context);
      auto state = context_find_tag<eolian_state_context>(context).state;
      if (!as_generator(string).generate(sink, documentation_generator::syntax_conversion(escaped, state, options.want_beta, context), context))
        return false;

      return true;
  }

} const documentation_string {};

namespace documentation_helpers
{

template<typename OutputIterator, typename Indent, typename Context>
bool generate_since_tag_line(OutputIterator sink, attributes::documentation_def const& doc, Indent indentation, Context context)
{
  if (doc.since.empty())
    return true;

  return as_generator
            (
                indentation << ("/// <para>Since EFL " + doc.since + ".</para>\n")
            ).generate(sink, attributes::unused, context);
}

} // documentation_helpers

} // namespace eolian_mono

namespace efl { namespace eolian { namespace grammar {

template<>
struct is_eager_generator<::eolian_mono::documentation_generator> : std::true_type {};
template<>
struct is_generator<::eolian_mono::documentation_generator> : std::true_type {};

template<>
struct is_eager_generator<::eolian_mono::documentation_string_generator> : std::true_type {};
template<>
struct is_generator<::eolian_mono::documentation_string_generator> : std::true_type {};

template<>
struct is_generator<::eolian_mono::documentation_terminal> : std::true_type {};

namespace type_traits {
template<>
struct attributes_needed<struct ::eolian_mono::documentation_generator> : std::integral_constant<int, 1> {};
template<>
struct attributes_needed<struct ::eolian_mono::documentation_terminal> : std::integral_constant<int, 1> {};
template<>
struct attributes_needed<struct ::eolian_mono::documentation_string_generator> : std::integral_constant<int, 1> {};
}
} } }

#endif
