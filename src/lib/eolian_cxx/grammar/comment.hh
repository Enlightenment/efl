
#ifndef EOLIAN_CXX_STD_COMMENT_HH
#define EOLIAN_CXX_STD_COMMENT_HH

#include <iosfwd>
#include <ostream>
#include <sstream>

#include "tab.hh"

namespace efl { namespace eolian { namespace grammar {

using std::endl;

const std::string comment_prefix("///");

struct comment
{
   std::string _doc;
   int _tab;
   comment(std::string const& doc, int tab = 0)
     : _doc(doc), _tab(tab)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, comment const& x)
{
   std::istringstream ss(x._doc);
   std::string line;
   while(std::getline(ss, line))
     {
        out << tab(x._tab) << comment_prefix
            << (line.size() ? (" " + line) : "")
            << endl;
     }
   return out;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_COMMENT_HH
