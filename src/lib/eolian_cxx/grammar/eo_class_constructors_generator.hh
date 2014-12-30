
#ifndef EOLIAN_CXX_STD_EO_CLASS_CONSTRUCTORS_GENERATOR_HH
#define EOLIAN_CXX_STD_EO_CLASS_CONSTRUCTORS_GENERATOR_HH

#include <iosfwd>
#include <string>

#include "eo_types.hh"
#include "tab.hh"
#include "comment.hh"
#include "parameters_generator.hh"
#include "namespace_generator.hh"

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

   ancestors_container_type::const_iterator it,
     first = cls.ancestors.cbegin(),
     last = cls.ancestors.cend();
   for (it = first; it != last; ++it)
     {
        out << tab(2) << ", ::" << abstract_namespace << "::" << *it << endl;
     }
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

   if (parameters_count_callbacks(x._ctor.params) == 0)
     return out;

   bool comma = false;
   out << "<";
   auto first = x._ctor.params.cbegin(), last = x._ctor.params.cend();
   for(auto it = first; it != last; ++it)
     {
        if (type_is_callback((*it).type) && it+1 != last)
          {
             if (comma)
               out << ", ";
             else
               comma = true;
             out << template_parameter_type((*it).type, (*it).name);
          }
     }
   return out << ">";
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

        // Struct declaration
        out << template_parameters_declaration(c.params, 1)
            << tab(1) << "struct " << constructor_functor_type_name(c) << endl
            << tab(1) << "{" << endl;

        // Callbacks typedefs
        out << parameters_cxx_generic(c.params,
                 [](param_data d)
                 {
                    if (d.is_cb)
                      d.out << tab(2)
                            << parameter_remove_reference_typedef(d.type, d.name)
                            << endl;
                 }
               )
            << endl;

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

                     if (d.is_cb)
                       d.out << callback_tmp(d.name) << "(new "
                             << template_parameter_type(d.type, d.name)
                             << "(" << parameter_forward(d.type, d.name) << "))";
                     else
                       d.out << d.name << "(" << parameter_forward(d.type, d.name) << ")";
                  }
               )
            << endl
            << tab(2) << "{}" << endl;

        // Struct operator()
        out << tab(2) << "void operator()()" << endl
            << tab(2) << "{" << endl
            << tab(3) << "::" << c.name << "(" << parameters_forward_to_c(c.params) << ");" << endl
            << tab(2) << "}" << endl;

        // Struct member variables
        out << endl
            << parameters_cxx_generic(c.params,
                  [](param_data d)
                  {
                     d.out << tab(2);
                     if (d.is_cb)
                       d.out << parameter_no_ref_type(d.type, d.name) << "* "
                             << callback_tmp(d.name);
                     else
                       d.out << parameter_type(d.type, d.name) << " " << d.name;
                     d.out << ";" << endl;
                  }
               );

        // Close struct
        out << tab(1) << "};" << endl << endl;
     }

   return out;
}

struct constructor_method_function_declarations
{
   eo_class const& _cls;
   constructor_method_function_declarations(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, constructor_method_function_declarations const& x)
{
   constructors_container_type::const_iterator it,
     first = x._cls.constructors.cbegin(),
     last = x._cls.constructors.cend();
   for (it = first; it != last; ++it)
     {
        eo_constructor const& c = *it;

        // "eo_constructor" is already called in the eo_add_ref macro (used in
        // _ctors_call).
        // Creating a function with this name yields an error in the eo_add_ref
        // macro expansion, because "eo_constructor" will refers to the class
        // function instead of the Eo.Base function which is intended.
        if (c.name == "eo_constructor")
          {
             continue;
          }

        out << comment(c.comment, 0)
            << template_parameters_declaration(c.params, 1)
            << tab(1) << constructor_functor_type_decl(c) << " " << c.name << "("
            << parameters_declaration(c.params) << ") const;" << endl << endl;
     }

   return out;
}

struct constructor_method_function_definitions
{
   eo_class const& _cls;
   constructor_method_function_definitions(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, constructor_method_function_definitions const& x)
{
   constructors_container_type::const_iterator it,
     first = x._cls.constructors.cbegin(),
     last = x._cls.constructors.cend();
   for (it = first; it != last; ++it)
     {
        eo_constructor const& c = *it;

        // Same explanation as the one in constructor_method_function_declarations
        if (c.name == "eo_constructor")
          {
             continue;
          }

        out << template_parameters_declaration(c.params, 0)
            << "inline " << full_name(x._cls)
            << "::" << constructor_functor_type_decl(c) << " "
            << full_name(x._cls, false) << "::" << c.name << "("
            << parameters_declaration(c.params) << ") const" << endl
            << "{" << endl
            << tab(1) << "return " << constructor_functor_type_decl(c) << "("
            << parameters_forward(c.params) << ");" << endl
            << "}" << endl << endl;
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
   unsigned cb_count = 0;

   constructors_container_type::const_iterator it,
     first = x._cls.constructors.cbegin(),
     last = x._cls.constructors.cend();
   for (it = first; it != last; ++it)
     {
        cb_count += parameters_count_callbacks((*it).params);
     }

   if (cb_count != 0)
     {
        out << tab(1) << "template <";
        for (unsigned i = 0; i != cb_count; ++i)
          {
             if (i != 0)
               out << ", ";
             out << "typename F" << i;
          }
        out << ">" << endl;
     }

   out << tab(1) << x._cls.name << "(";
   {
      unsigned cb_idx = 0;
      for (it = first; it != last; ++it)
        {
           out << constructor_functor_type_name(*it);

           if (cb_count != 0 && parameters_count_callbacks((*it).params) != 0)
             {
                out << "<"
                    << parameters_cxx_generic((*it).params,
                         [&cb_idx](param_data d)
                         {
                            if (d.is_cb)
                              d.out << (d.cb_idx ? ", " : "") << "F" << cb_idx++;
                         })
                    << ">";
             }
           out << " _c" << (it-first) << ", ";
        }
        assert(cb_idx == cb_count);
   }
   out << "::efl::eo::parent_type _p = (::efl::eo::parent = nullptr))" << endl
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
       << tab(2) << ": ::efl::eo::concrete(eo)" << endl
       << tab(1) << "{}" << endl << endl;

   out << comment(
                  "@brief nullptr_t Constructor.\n\n"
                  "Constructs an empty (null) object.\n\n"
                  , 1
                 )
       << tab(1)
       << "explicit " << x._cls.name << "(std::nullptr_t)" << endl
       << tab(2) << ": ::efl::eo::concrete(nullptr)" << endl
       << tab(1) << "{}" << endl << endl;
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
       << tab(2) << ": " << x._cls.name
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

struct function_call_constructor_methods
{
   eo_class const& _cls;
   function_call_constructor_methods(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, function_call_constructor_methods const& x)
{
   unsigned cb_count = 0;

   constructors_container_type::const_iterator it,
     first = x._cls.constructors.cbegin(),
     last = x._cls.constructors.cend();
   for (it = first; it != last; ++it)
     {
        unsigned param_cb_count = parameters_count_callbacks((*it).params);
        for (unsigned i = 0; i != param_cb_count; ++i)
          {
             if(cb_count == 0)
               out << tab(1) << "template <";
             else
               out << ", ";
             out << "typename F" << cb_count++;
          }
     }
   if (cb_count != 0)
     out << ">" << endl;

   unsigned cb_idx = 0;
   out << tab(1) << "static Eo* _ctors_call(";
   for (it = first; it != last; ++it)
     {
        out << constructor_functor_type_name(*it);

        if (cb_count != 0 && parameters_count_callbacks((*it).params) != 0)
          {
             out << "<"
                 << parameters_cxx_generic((*it).params,
                      [&cb_idx](param_data d)
                      {
                         if (d.is_cb)
                           d.out << (d.cb_idx ? ", " : "") << "F" << cb_idx++;
                      })
                 << ">";
          }
        out << " _c" << (it-first) << ", ";
     }
   assert(cb_idx == cb_count);

   out << "::efl::eo::parent_type _p)" << endl
       << tab(1) << "{" << endl
       << tab(2) << "Eo* _ret_eo = eo_add_ref(" << x._cls.eo_name << ", _p._eo_raw, ";
   for (it = first; it != last; ++it)
     {
        out << "_c" << (it-first) << "(); ";
     }
   out << ");" << endl << endl;

   cb_idx = 0;
   for (it = first; it != last; ++it)
     {
        if (parameters_count_callbacks((*it).params) == 0)
          continue;

        out << parameters_cxx_generic((*it).params,
                 [&it, &first, &cb_idx](param_data d)
                 {
                    if (d.is_cb)
                      d.out << tab(2)
                            << "eo_do(_ret_eo," << endl
                            << tab(3) << "eo_event_callback_add(EO_EV_DEL, "
                            << "&::efl::eolian::free_callback_calback<F" << cb_idx++
                            << ">, _c" << (it-first) << "." << callback_tmp(d.name)
                            << "));" << endl;
                 })
            << endl;
     }
   assert(cb_idx == cb_count);

   out << tab(2) << "return _ret_eo;" << endl
       << tab(1) << "}" << endl << endl;

   return out;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_EO_CLASS_CONSTRUCTORS_GENERATOR_HH
