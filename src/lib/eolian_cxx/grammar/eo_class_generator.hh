
#ifndef EOLIAN_CXX_EO_CLASS_GENERATOR_HH
#define EOLIAN_CXX_EO_CLASS_GENERATOR_HH

#include <iosfwd>
#include <string>

#include "eo_types.hh"
#include "tab.hh"
#include "comment.hh"
#include "eo_class_constructors_generator.hh"
#include "eo_class_functions_generator.hh"
#include "eo_class_events_generator.hh"

namespace efl { namespace eolian { namespace grammar {

struct eo_class_getter
{
   eo_class const& _cls;
   eo_class_getter(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, eo_class_getter const& x)
{
   out << tab(1) << "static Eo_Class const* _eo_class()" << endl
       << tab(1) << "{" << endl
       << tab(2) << "return("<< x._cls.eo_name << ");" << endl
       << tab(1) << "}" << endl << endl;
   return out;
}

inline void
eo_class_generator(std::ostream& out, eo_class const& cls)
{
   out << comment(cls.comment)
       << "struct " << cls.name << endl
       << tab(2) << ": " << class_inheritance(cls)
       << '{' << endl
       << constructor_eo(cls)
       << constructors(cls)
       << copy_constructor(cls)
       << destructor(cls)
       << functions(cls.functions)
       << events(cls)
       << eo_class_getter(cls)
        << "private:" << endl
       << eo_class_constructors(cls)
       << "};" << endl;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_EO_CLASS_GENERATOR_HH
