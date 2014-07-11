
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eina.hh"

#include <algorithm>

#include <check.h>

#include <iostream>

std::size_t nonpod_constructed = 0u
  , nonpod_destructed = 0u;

struct nonpod
{
  nonpod() { nonpod_constructed++; }
  nonpod(nonpod const&) { nonpod_constructed++; }
  nonpod(nonpod&&) { nonpod_constructed++; }
  ~nonpod() { nonpod_destructed++; }
};

START_TEST(eina_cxx_optional_constructors)
{
  namespace eina = efl::eina;

  eina::eina_init init;

  {
    eina::optional<int> optional;
    ck_assert(!optional);
  }

  {
    eina::optional<int> optional(nullptr);
    ck_assert(!optional);
  }

  {
    eina::optional<int> optional(5);
    ck_assert(!!optional);
    ck_assert(*optional == 5);
  }

  {
    eina::optional<nonpod> optional;
    ck_assert(!optional);
    ck_assert(::nonpod_constructed == 0u);
  }

  {
    ::nonpod object;
    eina::optional<nonpod> optional(object);
    ck_assert(!!optional);
  }
  std::cout << "nonpod_constructed " << nonpod_constructed
            << " nonpod_destructed " << nonpod_destructed << std::endl;
  ck_assert(::nonpod_constructed == ::nonpod_destructed);
}
END_TEST

START_TEST(eina_cxx_optional_rel_ops)
{
  namespace eina = efl::eina;

  eina::eina_init init;

  eina::optional<int> empty;
  eina::optional<int> one(1);
  eina::optional<int> two(2);
  eina::optional<int> one_again(1);

  ck_assert(empty == empty);
  ck_assert(one == one);
  ck_assert(one == one_again);
  ck_assert(one <= one_again);
  ck_assert(one >= one_again);
  ck_assert(empty < one);
  ck_assert(one >= empty);
  ck_assert(one > empty);
  ck_assert(one < two);
  ck_assert(one <= two);
  ck_assert(two > one);
  ck_assert(two >= one);
  ck_assert(!(empty < empty));
  ck_assert(!(one < one_again));
  ck_assert(empty != one);
  ck_assert(!(one != one));
  ck_assert(!(one != one_again));
}
END_TEST

START_TEST(eina_cxx_optional_assignment)
{
  namespace eina = efl::eina;

  eina::eina_init init;

  eina::optional<int> a;
  eina::optional<int> b(1);
  eina::optional<int> c(2);
  eina::optional<int> d(1);

  assert(!a); assert(b); assert(c); assert(d);

  a = a;
  ck_assert(a == a);
  ck_assert(!a);

  assert(!a); assert(b); assert(c); assert(d);

  b = a;
  ck_assert(b == a);
  ck_assert(b != d);
  ck_assert(!b);

  assert(!a); assert(!b); assert(c); assert(d);

  a = d;
  ck_assert(a == d);
  ck_assert(a != b);
  ck_assert(!!a);
  ck_assert(*a == 1);

  assert(a); assert(!b); assert(c); assert(d);

  c = d;

  ck_assert(c == d);
  ck_assert(c != b);
  ck_assert(!!c);
  ck_assert(*c == 1);

  assert(a); assert(!b); assert(c); assert(d);
}
END_TEST

void
eina_test_optional(TCase* tc)
{
  tcase_add_test(tc, eina_cxx_optional_constructors);
  tcase_add_test(tc, eina_cxx_optional_rel_ops);
  tcase_add_test(tc, eina_cxx_optional_assignment);
}
