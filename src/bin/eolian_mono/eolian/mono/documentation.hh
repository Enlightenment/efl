#ifndef EOLIAN_MONO_DOCUMENTATION_HPP
#define EOLIAN_MONO_DOCUMENTATION_HPP

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/html_escaped_string.hpp"
#include "using_decl.hh"
#include "name_helpers.hh"

#include <Eina.h>

namespace eolian_mono {

struct documentation_generator
{

   int scope_size = 0;

   documentation_generator(int scope_size)
       : scope_size(scope_size) {}

   /// Tag generator helpers
   template<typename OutputIterator, typename Context>
   bool generate_tag(OutputIterator sink, std::string const& tag, std::string const &text, Context const& context) const
   {
      if (text.empty())
        return true;

      return as_generator( scope_tab(scope_size) << "///<" << tag << ">" << html_escaped_string << "</" << tag << ">\n").generate(sink, text, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_tag_summary(OutputIterator sink, std::string const& text, Context const& context) const
   {
      return generate_tag(sink, "summary", text, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_tag_para(OutputIterator sink, std::string const& text, Context const& context) const
   {
      return generate_tag(sink, "para", text, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_tag_param(OutputIterator sink, std::string const& name, std::string const& text, Context const& context) const
   {
      return as_generator( scope_tab(scope_size) << "///<param name=\"" << name << "\">"
                    << html_escaped_string << "</param>\n").generate(sink, text, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate_tag_return(OutputIterator sink, std::string const& text, Context const& context) const
   {
      return generate_tag(sink, "return", text, context);
   }

   // Actual exported generators
   template<typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attr, Context const& context) const
   {
       return generate(sink, attr.documentation, context);
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

       if (!generate_tag_return(sink, func.return_documentation.summary, context))
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

       if (!generate_tag_return(sink, func.return_documentation.summary, context))
         return false;

       return true;
   }

   template<typename OutputIterator, typename Context>
   bool generate_parameter(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      return generate_tag_param(sink, name_helpers::escape_keyword(param.param_name), param.documentation.summary, context);
   }

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::documentation_def const& doc, Context const& context) const
   {
      if (!generate_preamble(sink, doc, context))
        return false;
      if (!generate_body(sink, doc, context))
        return false;
      if (!generate_epilogue(sink, doc, context))
        return false;

      return true;
   }

   template<typename OutputIterator, typename Context>
   bool generate_preamble(OutputIterator sink, attributes::documentation_def const& doc, Context const context) const
   {
      return generate_tag_summary(sink, doc.summary, context);
   }


   template<typename OutputIterator, typename Context>
   bool generate_body(OutputIterator sink, attributes::documentation_def const& doc, Context const context) const
   {
      for (auto&& para : doc.desc_paragraphs)
        {
           if (!generate_tag_para(sink, para, context))
             return false;
        }

      return true;
   }

   template<typename OutputIterator, typename Context>
   bool generate_epilogue(OutputIterator sink, attributes::documentation_def const& doc, Context const context) const
   {
      if (doc.since.empty())
        return true;

      if (!generate_tag_para(sink, doc.since, context))
        return false;

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
