
#ifndef EOLIAN_CXX_NAMESPACE_GENERATOR_HH
#define EOLIAN_CXX_NAMESPACE_GENERATOR_HH

#include "eo_types.hh"
#include "tab.hh"

namespace efl { namespace eolian { namespace grammar {

struct abstract_namespace_type {};

abstract_namespace_type const abstract_namespace = {};

inline std::ostream&
operator<<(std::ostream& out, abstract_namespace_type const&)
{
   return out << "eo_cxx";
}

struct namespace_head
{
   namespace_head(eo_class const& cls) : _cls(cls) {}
   eo_class const& _cls;
};

inline std::ostream&
operator<<(std::ostream& out, namespace_head const& x)
{
   if (x._cls.name_space != "")
     {
        std::string ns = x._cls.name_space;
        size_t pos = 0;
        while ((pos = ns.find("::")) != std::string::npos)
          {
             out << "namespace " << ns.substr(0, pos) << " { ";
             ns.erase(0, pos+2);
          }
        out << "namespace " << ns << " {" << endl << endl;
     }
   return out;
}

struct namespace_tail
{
   namespace_tail(eo_class const& cls) : _cls(cls) {}
   eo_class const& _cls;
};

inline std::ostream&
operator<<(std::ostream& out, namespace_tail const& x)
{
   if (x._cls.name_space != "")
     {
        std::string ns = x._cls.name_space;
        size_t pos = 0;
        while ((pos = ns.find("::")) != std::string::npos)
          {
             out << "} ";
             ns.erase(0, pos+2);
          }
        out << "}" << endl << endl;
     }
   return out;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif
