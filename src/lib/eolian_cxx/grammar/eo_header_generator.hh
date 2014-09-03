
#ifndef EOLIAN_CXX_STD_EO_HEADER_GENERATOR_HH
#define EOLIAN_CXX_STD_EO_HEADER_GENERATOR_HH

#include <set>
#include <algorithm>
#include <string>
#include <ostream>
#include <iosfwd>
#include <cctype>

#include "eo_types.hh"
#include "tab.hh"
#include "eo_class_generator.hh"
#include "inheritance_base_generator.hh"

namespace {
std::string
_onceguard_key(efl::eolian::eo_class const& cls)
{
   std::string key;
   if (cls.name_space != "")
     {
        std::string ns = cls.name_space;
        size_t pos = 0;
        while ((pos = ns.find("::")) != std::string::npos)
          {
             key += ns.substr(0, pos) + "_";
             ns.erase(0, pos+2);
          }
        key += ns + "_";
     }
   key += cls.name;
   std::transform(key.begin(), key.end(), key.begin(), ::toupper);
   return key;
}
}

namespace efl { namespace eolian { namespace grammar {

struct include_dependencies
{
   eo_class const& _cls;
   include_dependencies(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, include_dependencies const& x)
{
   std::set<std::string> headers;
   eo_class const& cls = x._cls;

   for (auto it = cls.constructors.cbegin(), last = cls.constructors.cend();
        it != last; ++it)
     for (auto it_p = (*it).params.begin(), last_p = (*it).params.end();
          it_p != last_p; ++it_p)
       for (eolian_type const& subtype : (*it_p).type)
         for (std::string header : subtype.includes)
           headers.insert(header);

   for (auto it = cls.functions.begin(), last  = cls.functions.end();
        it != last; ++it)
        for (auto it_p = (*it).params.begin(), last_p = (*it).params.end();
             it_p != last_p; ++it_p)
          for (eolian_type const& subtype : (*it_p).type)
            for (std::string header : subtype.includes)
              headers.insert(header);

   for (std::string header : headers)
     out << "#include <" << header << ">" << endl;

   return out;
}

inline void
onceguard_head(std::ostream& out, eo_class const& cls)
{
   std::string key = ::_onceguard_key(cls);
   out << "#ifndef EFL_GENERATED_" << key << "_HH" << endl
       << "#define EFL_GENERATED_" <<  key << "_HH" << endl << endl;
}

inline void
onceguard_tail(std::ostream& out, eo_class const& cls)
{
   std::string key = ::_onceguard_key(cls);
   out << "#endif // EFL_GENERATED_" << key << "_HH" << endl;
}

inline void
namespace_head(std::ostream& out, eo_class const& cls)
{
   if (cls.name_space != "")
     {
        std::string ns = cls.name_space;
        size_t pos = 0;
        while ((pos = ns.find("::")) != std::string::npos)
          {
             out << "namespace " << ns.substr(0, pos) << " { ";
             ns.erase(0, pos+2);
          }
        out << "namespace " << ns << " {" << endl << endl;
     }
}

inline void
namespace_tail(std::ostream& out, eo_class const& cls)
{
   if (cls.name_space != "")
     {
        std::string ns = cls.name_space;
        size_t pos = 0;
        while ((pos = ns.find("::")) != std::string::npos)
          {
             out << "} ";
             ns.erase(0, pos+2);
          }
        out << "} " << endl << endl;
     }
}

inline void
include_headers(std::ostream& out,
                eo_class const& cls EINA_UNUSED,
                eo_generator_options const& opts)
{
   out << "extern \"C\"" << endl
       << "{" << endl
       << "#include <Efl.h>" << endl
       << "}" << endl
       << "#include <Eo.hh>" << endl << endl
       << "#include <eo_cxx_interop.hh>" << endl << endl
       << "extern \"C\"" << endl
       << "{" << endl;
   for (auto c_header : opts.c_headers)
     {
       out << "#include \"" << c_header << "\"" << endl;
     }
   out << "}" << endl << endl;
   for (auto cxx_header : opts.cxx_headers)
     {
       out << "#include \"" << cxx_header << "\"" << endl;
     }
   out << include_dependencies(cls) << endl;
}

inline void
eo_header_generator(std::ostream& out, eo_class const& cls, eo_generator_options const& opts)
{
   onceguard_head(out, cls);
   include_headers(out, cls, opts);
   namespace_head(out, cls);
   eo_class_generator(out, cls);
   namespace_tail(out, cls);
   eo_inheritance_detail_generator(out, cls);
   onceguard_tail(out, cls);
   out << endl;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_EO_HEADER_GENERATOR_HH
