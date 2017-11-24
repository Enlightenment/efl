#ifndef EOLIAN_MONO_FUNCTION_POINTER_HPP
#define EOLIAN_MONO_FUNCTION_POINTER_HPP

#include <Eolian.h>

#include <vector>
#include <string>

namespace eolian_mono {

// Blacklist structs that require some kind of manual binding.
static bool is_function_ptr_blacklisted(attributes::function_def const& func, std::vector<std::string> const &namesp)
{
  std::stringstream full_name;

  for (auto&& i : namesp)
    full_name << i << ".";
  full_name << func.name;

  std::string name = full_name.str();

  return name == "Efl.Ui.Format_Func_Cb";
}

struct function_pointer {
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::function_def const& f, std::vector<std::string> const &namesp, Context const& context) const
   {
      // FIXME export Typedecl in eolian_cxx API
      std::vector<std::string> namespaces =  escape_namespace(namesp);

      if (is_function_ptr_blacklisted(f, namesp))
        return true;

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
      if (!as_generator("public class " << f_name << "Wrapper\n"
                  << "{\n\n"
                  << scope_tab << "private " << f_name  << "Internal _cb;\n"
                  << scope_tab << "private IntPtr _cb_data;\n"
                  << scope_tab << "private Eina_Free_Cb _cb_free_cb;\n\n"

                  << scope_tab << "public " << f_name << "Wrapper (" << f_name << "Internal _cb, IntPtr _cb_data, Eina_Free_Cb _cb_free_cb)\n"
                  << scope_tab << "{\n"
                  << scope_tab << scope_tab << "this._cb = _cb;\n"
                  << scope_tab << scope_tab << "this._cb_data = _cb_data;\n"
                  << scope_tab << scope_tab << "this._cb_free_cb = _cb_free_cb;\n"
                  << scope_tab << "}\n\n"

                  << scope_tab << "~" << f_name << "Wrapper()\n"
                  << scope_tab << "{\n"
                  << scope_tab << scope_tab << "if (this._cb_free_cb != null)\n"
                  << scope_tab << scope_tab << scope_tab << "this._cb_free_cb(this._cb_data);\n"
                  << scope_tab << "}\n\n"

                  << scope_tab << "public " << type << " ManagedCb(" << (parameter % ",") << ")\n"
                  << scope_tab << "{\n"
                  << scope_tab << scope_tab << (f.return_type.c_type != "void" ? "return ": "") << "_cb(_cb_data, " << (argument_invocation % ", ") << ");\n"
                  << scope_tab << "}\n\n"

                  << scope_tab << "public static " << type << " Cb(IntPtr cb_data, " << (parameter % ", ") << ")\n"
                  << scope_tab << "{\n"
                  << scope_tab << scope_tab << "GCHandle handle = GCHandle.FromIntPtr(cb_data);\n"
                  << scope_tab << scope_tab << string << " cb = (" << string << ")handle.Target;\n"
                  << scope_tab << scope_tab << (f.return_type.c_type != "void" ? "return " : "") << "cb(" << (argument_invocation % ", ") << ");\n"
                  << scope_tab << "}\n"
                  << "}\n"
                  ).generate(sink, std::make_tuple(f.return_type, f.parameters, f.parameters, f.return_type, f.parameters, f_name, f_name, f.parameters), context))
          return false;

      auto close_namespace = *(lit("} ")) << "\n";
      if(!as_generator(close_namespace).generate(sink, namespaces, context)) return false;

      return true;
   }
};

struct function_pointer const function_pointer = {};
}

#endif
