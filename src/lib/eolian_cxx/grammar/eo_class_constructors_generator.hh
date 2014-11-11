
#ifndef EOLIAN_CXX_STD_EO_CLASS_CONSTRUCTORS_GENERATOR_HH
#define EOLIAN_CXX_STD_EO_CLASS_CONSTRUCTORS_GENERATOR_HH

#include <iosfwd>
#include <string>

#include "eo_types.hh"
#include "tab.hh"
#include "comment.hh"
#include "parameters_generator.hh"

namespace efl { namespace eolian { namespace grammar {

struct class_name
{
   std::string _name;
   class_name(std::string name)
     : _name(name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, class_name const& x)
{
   out << x._name;
   return out;
}

struct class_extensions
{
   eo_class const& _cls;
   class_extensions(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, class_extensions const& x)
{
   eo_class const& cls = x._cls;
   extensions_container_type::const_iterator it, first = cls.extensions.begin();
   extensions_container_type::const_iterator last = cls.extensions.end();
   for (it = first; it != last; ++it)
     {
        if (it != first) out << ",\n";
        out << tab(2)
            << "efl::eo::detail::extension_inheritance<"
            <<  *it << ">::template type< ::";
        if(!cls.name_space.empty())
          out <<  cls.name_space << "::";
        out << cls.name << ">";
     }
   out << endl;
   return out;
}

struct class_inheritance
{
   eo_class const& _cls;
   class_inheritance(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, class_inheritance const& x)
{
   eo_class const& cls = x._cls;

   out << class_name(cls.parent);
   if (cls.extensions.size() > 0)
     {
        out << "," << endl << class_extensions(cls);
     }
   out << endl;
   return out;
}

struct constructor_functor_type_name
{
   eo_constructor const& _ctor;
   constructor_functor_type_name(eo_constructor const& ctor)
     : _ctor(ctor)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, constructor_functor_type_name const& x)
{
   out << "_c_" << x._ctor.name;
   return out;
}

struct constructor_functor_type_decl
{
   eo_constructor const& _ctor;
   constructor_functor_type_decl(eo_constructor const& ctor)
     : _ctor(ctor)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, constructor_functor_type_decl const& x)
{
   out << constructor_functor_type_name(x._ctor);
   auto cb_it = parameters_find_callback(x._ctor.params);
   if(cb_it != x._ctor.params.cend())
     {
        out << "<F>";
     }
   return out;
}

struct functors_constructor_methods
{
   eo_class const& _cls;
   functors_constructor_methods(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, functors_constructor_methods const& x)
{
   constructors_container_type::const_iterator it,
     first = x._cls.constructors.cbegin(),
     last = x._cls.constructors.cend();
   for (it = first; it != last; ++it)
     {
        eo_constructor const& c = *it;
        auto cb_it = parameters_find_callback(c.params);
        bool has_callback = (cb_it != c.params.cend());

        // Struct declaration
        if(has_callback)
          {
             out << tab(1) << "template <typename F>" << endl;
          }
        out << tab(1) << "struct " << constructor_functor_type_name(c) << endl
            << tab(1) << "{" << endl;

        // Struct constructor
        out << tab(2) << constructor_functor_type_name(c) << "("
            << parameters_declaration(c.params) << ")"
            << parameters_cxx_generic(c.params,
                  [](param_data d)
                  {
                     if(d.pos == 0u)
                        d.out << endl << tab(3) << ": ";
                     else
                        d.out << ", ";
                     d.out << d.name << "(" << parameter_forward(d.type, d.name) << ")";
                  }
               )
            << endl
            << tab(2) << "{}" << endl;

        // Struct operator()
        out << tab(2) << "void operator()()" << endl
            << tab(2) << "{" << endl;
        if (has_callback)
          {
             out << tab(3) << "typedef typename std::remove_reference<F>::type function_type;" << endl
                 << tab(3) << "function_type* _tmp_f = new F(this->" << (*cb_it).name << ");" << endl;
          }
        out << tab(3) << "::" << c.name << "(" << parameters_forward_to_c(c.params) << ");" << endl
            << tab(2) << "}" << endl;

        // Struct member variables
        out << tab(1) << "private:" << endl
            << parameters_cxx_generic(c.params,
                  [](param_data d)
                  {
                     d.out << tab(2) << parameter_type(d.type) << " " << d.name << ";" << endl;
                  }
               );

        // Close struct
        out << tab(1) << "};" << endl << endl;
     }

   return out;
}

struct functions_constructor_methods
{
   eo_class const& _cls;
   functions_constructor_methods(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, functions_constructor_methods const& x)
{
   constructors_container_type::const_iterator it,
     first = x._cls.constructors.cbegin(),
     last = x._cls.constructors.cend();
   for (it = first; it != last; ++it)
     {
        eo_constructor const& c = *it;

        out << comment(c.comment, 1);

        unsigned cb_count = parameters_count_callbacks(c.params);
        if(cb_count)
          out << tab(1) << "template <typename F>" << endl;
        out << tab(1) << constructor_functor_type_decl(c) << " " << c.name << "("
            << parameters_declaration(c.params) << ")" << endl
            << tab(1) << "{" << endl
            << tab(2) << "return " << constructor_functor_type_decl(c) << "("
            << parameters_forward(c.params) << ");" << endl
            << tab(1) << "}" << endl << endl;
     }

   return out;
}

struct constructor_with_constructor_methods
{
   eo_class const& _cls;
   constructor_with_constructor_methods(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, constructor_with_constructor_methods const& x)
{
   //
   // TODO Require constructor methods of all base classes ?
   //

   constructors_container_type::const_iterator it,
     first = x._cls.constructors.cbegin(),
     last = x._cls.constructors.cend();

   for (it = first; it != last; ++it)
     {
        unsigned cb_count = parameters_count_callbacks((*it).params);
        if(cb_count)
          {
             out << tab(1) << "template <typename F>" << endl;
             break;
          }
     }

   out << tab(1) << x._cls.name << "(";
   for (it = first; it != last; ++it)
     {
        out << constructor_functor_type_decl(*it)
            << " _c" << (it-first) << ", ";
     }
   out << "efl::eo::parent_type _p = (efl::eo::parent = nullptr))" << endl
       << tab(2) << ": " << x._cls.name << "(_ctors_call(";
   for (it = first; it != last; ++it)
     {
        out << "_c" << (it-first) << ", ";
     }
   out << "_p))" << endl
       << tab(1) << "{}" << endl << endl;

   return out;
}

struct constructor_eo
{
   eo_class const& _cls;
   constructor_eo(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, constructor_eo const& x)
{
   std::string doc = "@brief Eo Constructor.\n\n"
     "Constructs the object from an Eo* pointer stealing its ownership.\n\n"
     "@param eo The Eo object pointer.\n\n";
   out << comment(doc, 1)
       << tab(1)
       << "explicit " << x._cls.name << "(Eo* eo)" << endl
       << tab(2) << ": " << class_name(x._cls.parent) << "(eo)" << endl
       << tab(1) << "{}" << endl << endl;

   out << comment(
                  "@brief nullptr_t Constructor.\n\n"
                  "Constructs an empty (null) object.\n\n"
                  , 1
                 )
       << tab(1)
       << "explicit " << x._cls.name << "(std::nullptr_t)" << endl
       << tab(2) << ": " << class_name(x._cls.parent) << "(nullptr)" << endl
       << tab(1) << "{}" << endl << endl;
   return out;
}

struct constructors
{
   eo_class const& _cls;
   constructors(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, constructors const& x)
{
   constructors_container_type::const_iterator it,
     first = x._cls.constructors.cbegin(),
     last = x._cls.constructors.cend();
   for (it = first; it != last; ++it)
     {
        eo_constructor const& ctor = *it;
        out << comment(ctor.comment, 1);
        if (parameters_count_callbacks(ctor.params))
          out << tab(1) << "template <typename F>" << endl;
        out << tab(1)
            << x._cls.name
            << '(' << parameters_declaration(ctor.params)
            << (ctor.params.size() > 0 ? ", " : "")
            << "efl::eo::parent_type _p = (efl::eo::parent = nullptr))" << endl
            << tab(2) << ": " << class_name(x._cls.name)
            << "(_c" << (it - first) << "(" << constructor_parameters_list(ctor.params)
            << (ctor.params.size() > 0 ? ", " : "")
            << "_p))" << endl
            << tab(1) << "{}" << endl << endl;
     }
   return out;
}

struct copy_constructor
{
   eo_class const& _cls;
   copy_constructor(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, copy_constructor const& x)
{
   std::string doc = "@brief Copy Constructor.\n\n";
   out << comment(doc, 1)
       << tab(1)
       << x._cls.name << "(" << x._cls.name << " const& other)" << endl
       << tab(2) << ": " << class_name(x._cls.parent)
       << "(eo_ref(other._eo_ptr()))" << endl
       << tab(1) << "{}" << endl << endl;
   return out;
}

struct destructor
{
   eo_class const& _cls;
   destructor(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, destructor const& x)
{
   out << tab(1)
       << '~' << x._cls.name << "() {}" << endl << endl;
   return out;
}

struct eo_class_constructors
{
   eo_class const& _cls;
   eo_class_constructors(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, eo_class_constructors const& x)
{
   constructors_container_type::const_iterator it,
     first = x._cls.constructors.begin(),
     last = x._cls.constructors.end();
   for (it = first; it != last; ++it)
     {
        if (parameters_count_callbacks((*it).params))
          out << tab(1) << "template <typename F>" << endl;
        out << tab(1)
            << "static Eo* _c" << (it - first) << "("
            << parameters_declaration((*it).params)
            << ((*it).params.size() > 0 ? ", " : "")
            << "efl::eo::parent_type _p"
            << ')' << endl
            << tab(1) << "{" << endl;

        parameters_container_type::const_iterator callback_iter =
          parameters_find_callback((*it).params);
        if (callback_iter != (*it).params.cend())
          {
             out << tab(2)
                 << "typedef typename std::remove_reference<F>::type function_type;" << endl
                 << "function_type* _tmp_f = new F(std::forward<F>("
                 << (*callback_iter).name << "));"
                 << endl;
          }
        out << tab(2) << "return eo_add_ref("
            << x._cls.eo_name << ", _p._eo_raw, " << (*it).name
            << "(" << parameters_list((*it).params) << "));" << endl
            << tab(1) << "}" << endl << endl;
     }
   return out;
}

struct function_call_constructor_methods
{
   eo_class const& _cls;
   function_call_constructor_methods(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, function_call_constructor_methods const& x)
{
   constructors_container_type::const_iterator it,
     first = x._cls.constructors.cbegin(),
     last = x._cls.constructors.cend();
   for (it = first; it != last; ++it)
     {
        unsigned cb_count = parameters_count_callbacks((*it).params);
        if (cb_count)
          {
             out << tab(1) << "template <typename F>" << endl;
             break;
          }
     }
   out << tab(1) << "static Eo* _ctors_call(";
   for (it = first; it != last; ++it)
     {
        out << constructor_functor_type_decl(*it) << " _c" << (it-first) << ", ";
     }
   out << "efl::eo::parent_type _p)" << endl
       << tab(1) << "{" << endl
       << tab(2) << "return eo_add_ref(" << x._cls.eo_name << ", _p._eo_raw, ";
   for (it = first; it != last; ++it)
     {
        out << "_c" << (it-first) << "(); ";
     }
   out << ");" << endl
       << tab(1) << "}" << endl << endl;

   return out;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_EO_CLASS_CONSTRUCTORS_GENERATOR_HH
