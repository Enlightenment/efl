
#ifndef EOLIAN_CXX_STD_TAB_HH
#define EOLIAN_CXX_STD_TAB_HH

#include <iosfwd>
#include <ostream>
#include <sstream>

namespace efl { namespace eolian { namespace grammar {

using std::endl;

const int tabsize = 3;

struct tab
{
   int _n;
   tab(int n) : _n(n * tabsize) {}
};

inline std::ostream&
operator<<(std::ostream& out, efl::eolian::grammar::tab tab)
{
   for (int i = tab._n; i; --i)
     out << ' ';
   return out;
}

struct tabify
{
   int _n;
   std::string _text;
   tabify(int n, std::string const& text)
     : _n(n), _text(text)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, efl::eolian::grammar::tabify const& x)
{
   std::string line, tab(tabsize*x._n, ' ');
   std::istringstream ss(x._text);
   while (std::getline(ss, line))
     {
        out << tab << line << endl;
     }
   return out;
}

} } }

#endif // EOLIAN_CXX_STD_TAB_HH
