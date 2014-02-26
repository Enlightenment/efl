#ifndef EINA_REF_HH
#define EINA_REF_HH

#include <functional>

namespace efl { namespace eina {

using std::ref;
using std::cref;
using std::reference_wrapper;

template <typename T>
T& unref(T& t)
{
   return t;
}
		
template <typename T>
T& unref(reference_wrapper<T> t)
{
   return t.get();
}

}}

#endif
