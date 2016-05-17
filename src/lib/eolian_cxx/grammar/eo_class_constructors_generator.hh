
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
        out << tab(2) << ", EO_CXX_INHERIT(" << *it << ")" << endl;
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
     first = x._cls.all_constructors.cbegin(),
     last = x._cls.all_constructors.cend();
   for (it = first; it != last; ++it)
     {
        eo_constructor const& c = *it;

        // Hide documentation condition
        out << comment("@cond LOCAL", 1);

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
        out << tab(2) << "explicit " << constructor_functor_type_name(c) << "("
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
        out << tab(2) << "void operator()(Eo* _obj_eo_self)" << endl
            << tab(2) << "{" << endl
            << tab(3) << "::" << c.impl << "(_obj_eo_self" << (c.params.empty() ? "" : ", ")
            << parameters_forward_to_c(c.params) << ");" << endl
            << tab(2) << "}" << endl;

        // Register event to free allocated callbacks when the Eo* is deleted
        out << tab(2) << "void register_ev_del_free_callback(Eo* _eoptr)" << endl
            << tab(2) << "{" << endl
            << tab(3) << "(void) _eoptr;" << endl
            << parameters_cxx_generic(c.params,
                 [](param_data d)
                 {
                    if (d.is_cb)
                      d.out << tab(3)
                            << "eo_event_callback_add(_eoptr, EO_EVENT_DEL, "
                            << "&::efl::eolian::free_callback_callback<"
                            << parameter_no_ref_type(d.type, d.name)
                            << ">, " << callback_tmp(d.name) << ");" << endl;
                 })
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
        out << tab(1) << "};" << endl;

        // End documentation condition
        out << comment("@endcond", 1) << endl;
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
     first = x._cls.all_constructors.cbegin(),
     last = x._cls.all_constructors.cend();
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

        out << comment(c.comment, 1)
            << template_parameters_declaration(c.params, 1)
            << tab(1) << "static " << constructor_functor_type_decl(c)
            << " " << c.name << "("
            << parameters_declaration(c.params) << ");" << endl << endl;
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
     first = x._cls.all_constructors.cbegin(),
     last = x._cls.all_constructors.cend();
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
            << parameters_declaration(c.params) << ")" << endl
            << "{" << endl
            << tab(1) << "return " << constructor_functor_type_decl(c) << "("
            << parameters_forward(c.params) << ");" << endl
            << "}" << endl << endl;
     }

   return out;
}

struct comment_constructor_with_constructor_methods
{
   eo_class const& _cls;
   comment_constructor_with_constructor_methods(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, comment_constructor_with_constructor_methods const& x)
{
   out << tab(1) << "/**" << endl
       << tab(2) << "@brief Constructs a new " << full_name(x._cls, false) << " object." << endl
       << endl
       << tab(2) << "Constructs a new " << full_name(x._cls, false) << " object. If you want this object to be a child" << endl
       << tab(2) << "of another Eo object, use an @ref efl::eo::parent expression, like the example." << endl
       << endl;

   if (x._cls.constructors.size())
     {
        bool singular = (x._cls.constructors.size() == 1);
        out << tab(2) << "Since this class have " << (singular ? "a " : "")
            << "necessary constructor method" << (singular ? "" : "s")
            <<  ", you must call " << (singular ? "it" : "each one of them") << endl
            << tab(2) << "in the right place within this constructor parameters." << endl
            << endl;
     }

   if (!x._cls.optional_constructors.empty())
     {
        out << tab(2) << "Optional constructors may be called in any combination as the" << endl
            << tab(2) << "last parameters." << endl
            << endl;
     }

   out << tab(2) << "Example:" << endl
       << tab(2) << "@code" << endl
       << tab(2) << full_name(x._cls, false) << " my_" << x._cls.name << "(efl::eo::parent = parent_object";

   for (eo_constructor const& c : x._cls.all_constructors)
      out << "," << endl
          << tab(3) << "my_" << x._cls.name << "." << c.name << "(" << parameters_names(c.params) << ")";

   out << ");" << endl
       << tab(2) << "@endcode" << endl
       << endl;

   for (eo_constructor const& c : x._cls.all_constructors)
     out << tab(2) << "@see " << x._cls.name << "::" << c.name << endl;
   out << tab(2) << "@see " << x._cls.name << "(Eo* eo)" << endl;

   return out << tab(1) << "*/" << endl;
}

struct constructor_with_constructor_methods
{
   eo_class const& _cls;
   bool _with_parent;
   constructor_with_constructor_methods(eo_class const& cls, bool with_parent)
     : _cls(cls)
     , _with_parent(with_parent)
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

   if (cb_count != 0 || !x._cls.optional_constructors.empty())
     {
        out << tab(1) << "template <";
        for (unsigned i = 0; i != cb_count; ++i)
          {
             if (i != 0)
               out << ", ";
             out << "typename F" << i;
          }
        if (!x._cls.optional_constructors.empty())
          {
             if (cb_count != 0)
               out << ", ";
             out << "typename... FOpts";
          }
        out << ">" << endl;
     }

   out << tab(1) << "explicit " << x._cls.name << "(";

   if (x._with_parent)
     out << "::efl::eo::parent_type _p";

   {
      unsigned cb_idx = 0;
      for (it = first; it != last; ++it)
        {
           if (x._with_parent || it != first)
             out << ", ";
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
           out << " _c" << (it-first);
        }
        assert(cb_idx == cb_count);
   }

   if (!x._cls.optional_constructors.empty())
     {
        if (x._with_parent || first != last)
          out << ", ";
        out << "FOpts&&... _opts";
     }

   out << ")" << endl
       << tab(2) << ": " << x._cls.name << "(_ctors_call("
       << (x._with_parent ? "_p" : "::efl::eo::parent = nullptr");
   for (it = first; it != last; ++it)
     {
        out << ", _c" << (it-first);
     }
   if (!x._cls.optional_constructors.empty())
     {
        out << ", std::forward<FOpts>(_opts)...";
     }
   out << "))" << endl
       << tab(1) << "{}" << endl;

   return out;
}

struct constructors_with_constructor_methods
{
   eo_class const& _cls;
   constructors_with_constructor_methods(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, constructors_with_constructor_methods const& x)
{
   out << tab(1) << "//@{" << endl
       << comment_constructor_with_constructor_methods(x._cls)
       << constructor_with_constructor_methods(x._cls, true) << endl
       << constructor_with_constructor_methods(x._cls, false)
       << tab(1) << "//@}" << endl << endl;
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
   out << comment("@internal", 1);

   unsigned cb_count = 0;

   constructors_container_type::const_iterator it,
     first = x._cls.constructors.cbegin(),
     last = x._cls.constructors.cend();
   for (it = first; it != last; ++it)
     {
        cb_count += parameters_count_callbacks((*it).params);
     }
   if (cb_count != 0 || !x._cls.optional_constructors.empty())
     {
        out << tab(1) << "template <";
        for (unsigned i = 0; i != cb_count; ++i)
          {
             if (i != 0)
               out << ", ";
             out << "typename F" << i;
          }
        if (!x._cls.optional_constructors.empty())
          {
             if (cb_count != 0)
               out << ", ";
             out << "typename... FOpts";
          }
        out << ">" << endl;
     }

   unsigned cb_idx = 0;
   out << tab(1) << "static Eo* _ctors_call(::efl::eo::parent_type _p";
   for (it = first; it != last; ++it)
     {
        out << ", " << constructor_functor_type_name(*it);

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
        out << " _c" << (it-first);
     }
   assert(cb_idx == cb_count);

   if (!x._cls.optional_constructors.empty())
     out << ", FOpts&&... _opts";

   out << ")" << endl
       << tab(1) << "{" << endl
       << tab(2) << "Eo* _ret_eo = eo_add_ref(" << x._cls.eo_name << ", _p._eo_raw";
   for (it = first; it != last; ++it)
     {
        out << ", _c" << (it-first) << "(eo_self)";
     }
   if (!x._cls.optional_constructors.empty())
     out << ", ::efl::eolian::call_ctors(eo_self, _opts...)";
   out << ");" << endl << endl;

   for (it = first; it != last; ++it)
     out << tab(2) << "_c" << (it-first) << ".register_ev_del_free_callback(_ret_eo);" << endl;

   if (!x._cls.optional_constructors.empty())
     out << tab(2) << "::efl::eolian::register_ev_del_free_callback(_ret_eo, _opts...);" << endl;

   out << tab(2) << "return _ret_eo;" << endl
       << tab(1) << "}" << endl << endl;

   return out;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_EO_CLASS_CONSTRUCTORS_GENERATOR_HH
