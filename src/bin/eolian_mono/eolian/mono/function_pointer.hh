#ifndef EOLIAN_MONO_FUNCTION_POINTER_HPP
#define EOLIAN_MONO_FUNCTION_POINTER_HPP

#include <Eolian.h>

#include <vector>
#include <string>

#include "function_helpers.hh"
#include "documentation.hh"
#include "generation_contexts.hh"

namespace eolian_mono {

// Blacklist structs that require some kind of manual binding.
static bool is_function_ptr_blacklisted(attributes::function_def const& func)
{
  std::string name = name_helpers::function_ptr_full_eolian_name(func);

  return false;
}

struct function_pointer {
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
   {
      EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "function_pointer_generator: " << f.name << std::endl;
      // FIXME export Typedecl in eolian_cxx API
      auto funcptr_ctx = context_add_tag(class_context{class_context::function_ptr}, context);

      std::string return_type;
      if(!as_generator(eolian_mono::type(true)).generate(std::back_inserter(return_type), f.return_type, context))
        return false;

      if (is_function_ptr_blacklisted(f))
        return true;

      if (!name_helpers::open_namespaces(sink, f.namespaces, funcptr_ctx))
        return false;

      // C# visible delegate
      if (!as_generator(documentation
                  << "public delegate " << type << " " << string
                  << "(" << (parameter % ", ") << ");\n")
              .generate(sink, std::make_tuple(f, f.return_type, name_helpers::escape_keyword(f.name), f.parameters), funcptr_ctx))
          return false;
      // "Internal" delegate, 1-to-1 with the Unamaged function type
      if (!as_generator(marshall_native_annotation(true)
                  << "internal delegate " << marshall_type(true) << " " << string // public?
                  << "Internal(IntPtr data" << *grammar::attribute_reorder<-1, -1>((", " << marshall_native_annotation << " " << marshall_parameter)) << ");\n")
              .generate(sink, std::make_tuple(f.return_type, f.return_type, name_helpers::escape_keyword(f.name), f.parameters), funcptr_ctx))
          return false;

      std::string f_name = name_helpers::escape_keyword(f.name);
      // Wrapper type, with callback matching the Unamanaged one
      if (!as_generator("internal class " << f_name << "Wrapper\n"
                  << "{\n\n"
                  << scope_tab << "private " << f_name  << "Internal _cb;\n"
                  << scope_tab << "private IntPtr _cb_data;\n"
                  << scope_tab << "private Eina_Free_Cb _cb_free_cb;\n\n"

                  << scope_tab << "internal " << f_name << "Wrapper (" << f_name << "Internal _cb, IntPtr _cb_data, Eina_Free_Cb _cb_free_cb)\n"
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

                  << scope_tab << "internal " << type << " ManagedCb(" << (parameter % ",") << ")\n"
                  << scope_tab << "{\n"
                  << function_definition_preamble << "_cb(_cb_data, " << (argument_invocation % ", ") << ");\n"
                  << function_definition_epilogue
                  << scope_tab << "}\n\n"


                  << scope_tab << marshall_native_annotation(true)
                  << scope_tab << "internal static " << marshall_type(true) << " Cb(IntPtr cb_data" << *grammar::attribute_reorder<-1, -1>((", " << marshall_native_annotation << " " << marshall_parameter)) << ")\n"
                  << scope_tab << "{\n"
                  << scope_tab << scope_tab << "GCHandle handle = GCHandle.FromIntPtr(cb_data);\n"
                  << scope_tab << scope_tab << string << " cb = (" << string << ")handle.Target;\n"
                  << native_function_definition_preamble
                  << scope_tab << scope_tab << "try {\n"
                  << scope_tab << scope_tab << scope_tab <<  (return_type != " void" ? "_ret_var = " : "") << "cb(" << (native_argument_invocation % ", ") << ");\n"
                  << scope_tab << scope_tab << "} catch (Exception e) {\n"
                  << scope_tab << scope_tab << scope_tab << "eina.Log.Warning($\"Callback error: {e.ToString()}\");\n"
                  << scope_tab << scope_tab << scope_tab << "eina.Error.Set(eina.Error.EFL_ERROR);\n"
                  << scope_tab << scope_tab << "}\n"
                  << native_function_definition_epilogue(nullptr)
                  << scope_tab << "}\n"
                  << "}\n"
                  ).generate(sink, std::make_tuple(f.return_type, f.parameters, f, f.parameters, f, f.return_type, f.return_type, f.parameters, f_name, f_name, f, f.parameters, f), funcptr_ctx))
          return false;

      if (!name_helpers::close_namespaces(sink, f.namespaces, funcptr_ctx))
        return false;

      return true;
   }
};

struct function_pointer const function_pointer = {};
}

#endif
