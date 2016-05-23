
#include <Eo.h>

#include "complex.eo.h"
#include "complex.eo.hh"

template <typename T>
struct test1;

template <typename T, typename U>
struct test1<void(T::*)(U) const>
{
  static_assert(std::is_same<efl::eina::range_list<int>, U>::value, "Wrong type");
};

template <typename T>
struct test2;

template <typename T, typename U>
struct test2<U(T::*)() const>
{
  static_assert(std::is_same<efl::eina::range_array<int>, U>::value, "Wrong type");
};

test1<typeof( & nonamespace::Complex::foo )> foo;
test2<typeof( & nonamespace::Complex::bar )> bar;

  
