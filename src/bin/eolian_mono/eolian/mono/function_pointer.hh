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

      // C# visible delegate
      if (!as_generator("public delegate " << type << " " << string
                  << "(" << (parameter % ", ") << ");\n")
              .generate(sink, std::make_tuple(f.return_type, escape_keyword(f.name), f.parameters), context))
          return false;
      // "Internal" delegate, 1-to-1 with the Unamaged function type
      if (!as_generator("public delegate " << type << " " << string // public?
                  << "Internal(IntPtr data, " << (parameter % ", ") << ");\n")
              .generate(sink, std::make_tuple(f.return_type, escape_keyword(f.name), f.parameters), context))
          return false;

      std::string f_name = escape_keyword(f.name);
      // Wrapper type, with callback matching the Unamanaged one
      if (!as_generator("public class " << string << "Wrapper {\n"
                  << scope_tab << "public static " << type << " Cb(IntPtr cb_data, " << (parameter % ", ") << ") {\n"
                  << scope_tab << scope_tab << "GCHandle handle = GCHandle.FromIntPtr(cb_data);\n"
                  << scope_tab << scope_tab << string << " cb = (" << string << ")handle.Target;\n"
                  << scope_tab << scope_tab << "return cb(" << (argument_invocation % ", ") << ");\n"
                  << scope_tab << "}\n"
                  << "}\n"
                  ).generate(sink, std::make_tuple(f_name, f.return_type, f.parameters, f_name, f_name, f.parameters), context))
          return false;

      auto close_namespace = *(lit("} ")) << "\n";
      if(!as_generator(close_namespace).generate(sink, namespaces, context)) return false;

      return true;
   }
};

struct function_pointer const function_pointer = {};
}

#endif
