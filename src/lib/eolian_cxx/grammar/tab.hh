
#ifndef EOLIAN_CXX_STD_TAB_HH
#define EOLIAN_CXX_STD_TAB_HH

#include <ostream>
#include <iosfwd>

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

} } }

#endif // EOLIAN_CXX_STD_TAB_HH
