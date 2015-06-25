
#ifndef EOLIAN_CXX_STD_INHERITANCE_GENERATOR_HH
#define EOLIAN_CXX_STD_INHERITANCE_GENERATOR_HH

#include <iosfwd>
#include <cassert>
#include <algorithm>

#include "eo_types.hh"
#include "tab.hh"
#include "parameters_generator.hh"
#include "eo_class_functions_generator.hh"

namespace efl { namespace eolian { namespace grammar {

inline std::string
_ns_as_prefix(eo_class const& cls)
{
   // XXX Use eolian_cxx::find_replace() instead.
   std::string s = cls.name_space;
   std::string::size_type found = s.find("::");
   while (found != std::string::npos)
     {
        s.replace(found, 2, "_");
        found = s.find("::");
     }
   return s;
}

struct inheritance_operation
{
   eo_class const& _cls;
   eo_function const& _func;
   inheritance_operation(eo_class const& cls, eo_function const& func)
     : _cls(cls), _func(func)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_operation const& x)
{
   eo_function const& func = x._func;
   out << scope_guard_head(x._cls, func)
       << tab(1)
       << "ops[i].func = reinterpret_cast<void*>(& ::"
       << _ns_as_prefix(x._cls) << "_"
       << x._cls.name << "_" << func.name << "_wrapper<T>);" << endl
       << tab(1) << "ops[i].api_func = reinterpret_cast<void*>(& ::"
       << func.impl << ");" << endl
       << tab(1) << "ops[i].op = EO_OP_OVERRIDE;" << endl
       << tab(1) << "ops[i].op_type = EO_OP_TYPE_REGULAR;" << endl // XXX class ops
       << tab(1) << "ops[i].doc = NULL;" << endl
       << tab(1) << "++i;" << endl
       << scope_guard_tail(x._cls, func)
       << endl;
   return out;
}

struct inheritance_operations_description
{
   eo_class const& _cls;
   inheritance_operations_description(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_operations_description const& x)
{
   std::string s;

   out << "template <typename T>"
       << endl << "int initialize_operation_description(::efl::eo::detail::tag<"
       << full_name(x._cls) << ">" << endl
       << tab(11)
       << ", Eo_Op_Description* ops)" << endl
       << "{" << endl
       << tab(1) << "int i = 0;" << endl
       << tab(1) << "(void)i;" << endl
       << tab(1) << "(void)ops;" << endl;

   for (auto const& f : x._cls.functions)
     {
        out << inheritance_operation(x._cls, f);
     }

   for (std::string const& parent : x._cls.parents)
     {
        out << tab(1)
            << "initialize_operation_description<T>(::efl::eo::detail::tag<::"
            << parent << ">(), &ops[operation_description_class_size< "
            << full_name(x._cls) << " >::value" << s << "]);" << endl;

        s += " + operation_description_class_size<::" + parent + ">::value";
     }

   out << tab(1) << "return 0;" << endl
       << "}" << endl;

   return out;
}

struct inheritance_wrappers
{
   eo_class const& _cls;
   inheritance_wrappers(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_wrappers const& x)
{
   functions_container_type::const_iterator it,
     first = x._cls.functions.begin(),
     last = x._cls.functions.end();
   for (it = first; it != last; ++it)
     {
        eo_function const& func = *it;

        out << scope_guard_head(x._cls, func);

        out << "template <typename T>" << endl
            << reinterpret_type(func.ret) << " "
            << _ns_as_prefix(x._cls) << "_"
            << x._cls.name << "_" << func.name
            << "_wrapper(Eo* objid EINA_UNUSED, "
            << "::efl::eo::detail::Inherit_Private_Data* self"
            << (func.params.size() ? ", " : "")
            << parameters_c_declaration(func.params)
            << ")" << endl
            << "{" << endl;

        out << tab(1)
            << "try" << endl
            << tab(2) << "{" << endl
            << tab(3)
            << (!function_is_void(func) ? "return ": "")
            << "static_cast<T*>(self->this_)->"
            << func.name << "(" << parameters_cxx_list(func.params) << ");" << endl
            << tab(2) << "}" << endl
            << tab(1) << "catch (...)" << endl
            << tab(2) << "{" << endl
            << tab(3) << "eina_error_set( ::efl::eina::unknown_error() );" << endl;

        if (!function_is_void(func))
          out << tab(3) << func.ret.front().native << " _tmp_ret{};" << endl
              << tab(3) << "return " << to_cxx(func.ret, "_tmp_ret") << ";" << endl;

        out << tab(2) << "}" << endl
            << "}" << endl;

        out << scope_guard_tail(x._cls, func) << endl;
     }
   return out;
}

struct inheritance_base_operations_size
{
   eo_class const& _cls;
   inheritance_base_operations_size(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_base_operations_size const& x)
{
   int pcted = 0;
   int beta = 0;
   int pcted_beta = 0;

   auto funcs = x._cls.functions;

   for (auto const& f : funcs)
     {
        if (f.is_beta && f.scope != eolian_scope::public_)
          ++pcted_beta;
        if (f.scope != eolian_scope::public_)
          ++pcted;
        if (f.is_beta)
          ++beta;
     }
   auto all = funcs.size();

   out << "template<>"
       << endl << "struct operation_description_class_size< "
       << full_name(x._cls) << " >" << endl
       << "{" << endl
       << tab(1) << "static constexpr int value = " << endl
       << "#if defined(" << name_upper(x._cls) << "_PROTECTED)"
       << " && defined(" << name_upper(x._cls) << "_BETA)" << endl
       << tab(2) << all << endl
       << "#elif defined(" << name_upper(x._cls) << "_PROTECTED)" << endl
       << tab(2) << (all - beta) << endl
       << "#elif defined(" << name_upper(x._cls) << "_BETA)" << endl
       << tab(2) << (all - pcted) << endl
       << "#else" << endl
       << tab(2) << (all + pcted_beta - beta - pcted) << endl
       << "#endif" << endl;

   for (std::string const& parent : x._cls.parents)
     {
        out << tab(2) << "+ operation_description_class_size<::" << parent << " >::value";
     }

   out << ";" << endl
       << "};" << endl
       << endl;

   return out;
}

struct inheritance_base_operations_extensions
{
   eo_class const& _cls;
   inheritance_base_operations_extensions(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_base_operations_extensions const& x)
{
   eo_class const& cls = x._cls;
   ancestors_container_type::const_iterator it, first = cls.parents.begin();
   ancestors_container_type::const_iterator last = cls.parents.end();

   for (it = first; it != last; ++it)
     {
        out << endl
            << tab(3) << (it == first ? ": " : ", ")
            << "virtual operations< ::" << *it
            << " >::template type<T>";
     }

   return out << endl;
}

struct inheritance_base_operations_function
{
   eo_class const& _cls;
   eo_function const& _func;
   inheritance_base_operations_function(eo_class const& cls, eo_function const& func)
     : _cls(cls) , _func(func)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_base_operations_function const& x)
{
   eo_function const& func = x._func;
   bool is_void = function_is_void(func);

   if (parameters_count_callbacks(func.params))
     out << template_parameters_declaration(func.params, 2) << tab(2);
   else
     out << tab(2) << "virtual ";

   out << reinterpret_type(func.ret) << " "
       << func.name << "("
       << parameters_declaration(func.params) << ")" << endl
       << tab(2) << "{" << endl;

   if (!is_void)
     out << tab(3) << func.ret.front().native << " _tmp_ret = {};"  << endl;

   out << callbacks_heap_alloc("dynamic_cast<T*>(this)->_eo_ptr()", func.params, function_is_static(x._func), 3)
       << endl;

   out << tab(3)
       << "eo_do_super(dynamic_cast<T*>(this)->_eo_ptr()," << endl
       << tab(5) << "dynamic_cast<T*>(this)->_eo_class()," << endl
       << tab(5) << function_call(func) << ");" << endl;

   if (!is_void)
     out << tab(4) << "return " << to_cxx(func.ret, "_tmp_ret") << ";" << endl;

   return out << tab(2) << "}" << endl;
}

struct inheritance_base_operations
{
   eo_class const& _cls;
   inheritance_base_operations(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_base_operations const& x)
{
   out << "template<>" << endl
       << "struct operations< "
       << full_name(x._cls) << " >" << endl
       << "{" << endl
       << tab(1) << "template <typename T>" << endl
       << tab(1) << "struct type" << inheritance_base_operations_extensions(x._cls)
       << tab(1) << "{" << endl;
   functions_container_type::const_iterator it,
     first = x._cls.functions.begin(),
     last = x._cls.functions.end();
   for (it = first; it != last; ++it)
     {
        out << scope_guard_head(x._cls, *it);
        out << inheritance_base_operations_function(x._cls, *it);
        out << scope_guard_tail(x._cls, *it) << endl;
     }
   out << tab(1) << "};" << endl
       << "};" << endl << endl;
   return out;
}

struct inheritance_eo_class_getter
{
   eo_class const& _cls;
   inheritance_eo_class_getter(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_eo_class_getter const& x)
{
   out << "inline Eo_Class const* get_eo_class(tag<"
       << full_name(x._cls) << ">)" << endl
       << "{" << endl
       << tab(1) << "return (" << x._cls.eo_name << ");" << endl
       << "}" << endl << endl;
   return out;
}

inline void
eo_inheritance_detail_generator(std::ostream& out, eo_class const& cls)
{
   if(cls.eo_name != "EO_BASE_CLASS")
     out << inheritance_wrappers(cls)
         << "namespace efl { namespace eo { namespace detail {" << endl << endl
         << inheritance_base_operations(cls) << endl
         << inheritance_base_operations_size(cls)
         << inheritance_operations_description(cls)
         << inheritance_eo_class_getter(cls)
         <<  "} } }" << endl;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_INHERITANCE_GENERATOR_HH
