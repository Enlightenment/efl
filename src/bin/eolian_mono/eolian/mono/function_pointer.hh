#ifndef EOLIAN_MONO_FUNCTION_POINTER_HPP
#define EOLIAN_MONO_FUNCTION_POINTER_HPP

#include <Eolian.h>

#include <vector>
#include <string>

namespace eolian_mono {

struct function_pointer {
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::function_def const& f, std::vector<std::string> const &namesp, Context const& context) const
   {
      // FIXME export Typedecl in eolian_cxx API
      std::vector<std::string> namespaces =  escape_namespace(namesp);

      auto open_namespace = *("namespace " << string << " {") << "\n";
      if(!as_generator(open_namespace).generate(sink, namespaces, add_lower_case_context(context))) return false;

      if (!as_generator("public delegate " << type << " " << string
                  << "(IntPtr data, " << (parameter % ", ") << ");\n")
              .generate(sink, std::make_tuple(f.return_type, escape_keyword(f.name), f.parameters), context))
          return false;

      auto close_namespace = *(lit("} ")) << "\n";
      if(!as_generator(close_namespace).generate(sink, namespaces, context)) return false;

      return true;
   }
};

struct function_pointer const function_pointer = {};
}

#endif
