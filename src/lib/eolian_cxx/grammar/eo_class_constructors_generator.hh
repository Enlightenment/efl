
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
        if (it != first) out << ",";
        out << tab(2)
            << "efl::eo::detail::extension_inheritance<"
            <<  *it << ">::type<"
            <<  cls.name << ">";
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
            << "(_c" << (it - first) << "(" << parameters_cxx_list(ctor.params)
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
                 << "F* _tmp_f = new F(std::move("
                 << (*callback_iter).name << "));"
                 << endl;
          }
        out << tab(2) << "return eo_add_custom("
            << x._cls.eo_name << ", _p._eo_raw, " << (*it).name
            << "(" << parameters_list((*it).params) << "));" << endl
            << tab(1) << "}" << endl << endl;
     }
   return out;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_EO_CLASS_CONSTRUCTORS_GENERATOR_HH
