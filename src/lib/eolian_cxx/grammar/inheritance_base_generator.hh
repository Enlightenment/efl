
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
        s.replace(found, 1, "_");
        found = s.find("::");
     }
   return s;
}

struct inheritance_operation
{
   eo_class const& _cls;
   functions_container_type::size_type _idx;
   inheritance_operation(eo_class const& cls, functions_container_type::size_type idx)
     : _cls(cls), _idx(idx)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_operation const& x)
{
   assert(x._idx < x._cls.functions.size());
   eo_function const& func = x._cls.functions[x._idx];
   out << tab(1)
       << "ops[" << x._idx << "].func = reinterpret_cast<void*>(& ::"
       << _ns_as_prefix(x._cls) << "_"
       << x._cls.name << "_" << func.name << "_wrapper<T>);" << endl
       << tab(1) << "ops[" << x._idx << "].api_func = reinterpret_cast<void*>(& ::"
       << func.impl << ");" << endl
       << tab(1) << "ops[" << x._idx << "].op = EO_OP_OVERRIDE;" << endl
       << tab(1) << "ops[" << x._idx << "].op_type = EO_OP_TYPE_REGULAR;" << endl // XXX class ops
       << tab(1) << "ops[" << x._idx << "].doc = NULL;" << endl
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
   out << "template <typename T>"
       << endl << "int initialize_operation_description(::efl::eo::detail::tag<"
       << full_name(x._cls) << ">" << endl
       << tab(11)
       << ", Eo_Op_Description* ops)" << endl
       << "{" << endl
       << tab(1) << "(void)ops;" << endl;
   functions_container_type::size_type n_ops = x._cls.functions.size();
   for (functions_container_type::size_type i=0; i < n_ops; ++i)
     {
        out << inheritance_operation(x._cls, i);
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

        if (!function_is_void(func))
          out << tab(1) << reinterpret_type(func.ret) << " _tmp_ret{};" << endl;

        out << tab(1)
            << "try" << endl
            << tab(2) << "{" << endl
            << tab(3)
            << (!function_is_void(func) ? "_tmp_ret = ": "")
            << "static_cast<T*>(self->this_)->"
            << func.name << "(" << parameters_cxx_list(func.params) << ");" << endl
            << tab(2) << "}" << endl
            << tab(1) << "catch (...)" << endl
            << tab(2) << "{" << endl
            << tab(3) << "eina_error_set( ::efl::eina::unknown_error() );" << endl
            << tab(2) << "}" << endl;

        if (!function_is_void(func))
          out << tab(1) << "return _tmp_ret;" << endl;

        out << "}" << endl << endl;
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
   out << "template<>"
       << endl << "struct operation_description_class_size< "
       << full_name(x._cls) << " >" << endl
       << "{" << endl
       << tab(1) << "static const int value = "
       << x._cls.functions.size()
       << ";" << endl
       << "};" << endl
       << endl;
   return out;
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

   out << callbacks_heap_alloc("static_cast<T*>(this)->_eo_ptr()", func.params, 3)
       << endl;

   out << tab(3)
       << "eo_do_super(static_cast<T*>(this)->_eo_ptr()," << endl
       << tab(5) << "static_cast<T*>(this)->_eo_class()," << endl
       << tab(5) << function_call(func) << ");" << endl;

   if (!is_void)
     out << tab(4) << "return " << to_cxx(func.ret, "_tmp_ret") << ";" << endl;

   return out << tab(2) << "}" << endl << endl;
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
       << tab(1) << "struct type" << endl
       << tab(1) << "{" << endl;
   functions_container_type::const_iterator it,
     first = x._cls.functions.begin(),
     last = x._cls.functions.end();
   for (it = first; it != last; ++it)
     {
        out << inheritance_base_operations_function(x._cls, *it);
     }
   out << tab(1) << "};" << endl
       << "};" << endl << endl;
   return out;
}

struct inheritance_call_constructor_arguments
{
   parameters_container_type const& _params;
   inheritance_call_constructor_arguments(parameters_container_type const& params)
     : _params(params)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_call_constructor_arguments const& x)
{
   parameters_container_type::size_type i, n = x._params.size();
   for (i=0; i<n; i++)
     {
        if(i!=0) out << ", ";
        out << "::efl::eolian::to_c(args.get<" << i << ">())";
     }
   return out;
}

struct inheritance_call_constructors
{
   eo_class const& _cls;
   inheritance_call_constructors(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_call_constructors const& x)
{
   constructors_container_type::const_iterator it,
     first = x._cls.constructors.begin(),
     last = x._cls.constructors.end();
   for (it = first; it != last; ++it)
     {
        eo_constructor const& ctor = *it;
        out << "inline void" << endl
            << "call_constructor(tag< "
            << full_name(x._cls) << " >" << endl
            << tab(5) << ", Eo* eo, Eo_Class const* cls EINA_UNUSED," << endl
            << tab(5) << "args_class<"
            << full_name(x._cls)
            << ", ::std::tuple<"
            << parameters_types(ctor.params)
            << "> > const& args)" << endl
            << "{" << endl
            << tab(1) << "(void)args;" << endl
            << tab(1)
            << "eo_do_super(eo, cls, ::" << ctor.name
            << "(" << inheritance_call_constructor_arguments(ctor.params)
            << "));" << endl
            << "}" << endl << endl;
     }

   out << "inline void" << endl
       << "call_constructor(tag< "
       << full_name(x._cls) << " >" << endl
       << tab(5) << ", Eo* eo, Eo_Class const* cls EINA_UNUSED," << endl
       << tab(5) << "args_class<"
       << full_name(x._cls)
       << ", ::std::tuple<::efl::eo::parent_type> > const& args)" << endl
       << "{" << endl
       << tab(1) << "eo_do_super(eo, cls, ::eo_constructor());" << endl
       << tab(1) << "eo_do(eo, ::eo_parent_set(args.get<0>()._eo_raw));" << endl
       << "}" << endl << endl;

   return out;
}

struct inheritance_extension_function
{
   eo_function const& _func;
   inheritance_extension_function(eo_function const& func) : _func(func) {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_extension_function const& x)
{
   out << template_parameters_declaration(x._func.params, 1);

   bool is_void = function_is_void(x._func);
   out << tab(2)
       << reinterpret_type(x._func.ret) << " "
       << x._func.name << "("
       << parameters_declaration(x._func.params)
       << ")" << endl
       << tab(2) << "{" << endl;

   if (!is_void)
     {
        out << tab(3) << x._func.ret.front().native << " _tmp_ret = {};"  << endl;
     }

   out << callbacks_heap_alloc("static_cast<U*>(this)->_eo_ptr()", x._func.params, 2)
       << endl;

   out << tab(3) << "eo_do(static_cast<U*>(this)->_eo_ptr(), "
       << function_call(x._func) << ");" << endl;

   if (!function_is_void(x._func))
     out << tab(4) << "return " << to_cxx(x._func.ret, "_tmp_ret") << ";" << endl;
   out << tab(2) << "}" << endl
       << endl;

   return out;
}

struct inheritance_extension
{
   eo_class const& _cls;
   inheritance_extension(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_extension const& x)
{
   full_name const cls(x._cls);
   out << "template<>" << endl
       << "struct extension_inheritance< "
       << cls << ">" << endl
       << "{" << endl
       << tab(1) << "template <typename U>" << endl
       << tab(1) << "struct type" << endl
       << tab(1) << "{" << endl
       << tab(2) << "operator " << cls << "() const" << endl
       << tab(2) << "{" << endl
       << tab(3) << "return " << cls
       << "(eo_ref(static_cast<U const*>(this)->_eo_ptr()));" << endl
       << tab(2) << "}" << endl
       << endl;
   functions_container_type::const_iterator it,
     first = x._cls.functions.begin(),
     last = x._cls.functions.end();
   for (it = first; it != last; ++it)
     {
        out << inheritance_extension_function(*it);
     }
   out << events(x._cls, true);
   out << tab(1) << "};" << endl
       << "};" << endl
       << endl;
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
   out << inheritance_wrappers(cls)
       << "namespace efl { namespace eo { namespace detail {" << endl << endl
       << inheritance_base_operations(cls) << endl
       << inheritance_base_operations_size(cls)
       << inheritance_operations_description(cls)
       << inheritance_call_constructors(cls)
       << inheritance_extension(cls)
       << inheritance_eo_class_getter(cls)
       <<  "} } }" << endl;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_INHERITANCE_GENERATOR_HH
