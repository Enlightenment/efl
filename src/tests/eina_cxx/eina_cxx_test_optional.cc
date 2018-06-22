#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <iostream>

#include <Eina.hh>

#include "eina_cxx_suite.h"

std::size_t nonpod_constructed = 0u
  , nonpod_destructed = 0u;

struct nonpod
{
  nonpod() { nonpod_constructed++; }
  nonpod(nonpod const&) { nonpod_constructed++; }
  nonpod(nonpod&&) { nonpod_constructed++; }
  ~nonpod() { nonpod_destructed++; }
};

EFL_START_TEST(eina_cxx_optional_constructors)
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
EFL_END_TEST

EFL_START_TEST(eina_cxx_optional_rel_ops)
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
EFL_END_TEST

EFL_START_TEST(eina_cxx_optional_assignment)
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
EFL_END_TEST

EFL_START_TEST(eina_cxx_optional_convertible_types)
{
  namespace eina = efl::eina;

  eina::eina_init init;

  eina::optional<int> a(1.0);
  eina::optional<eina::string_view> b("2");
  eina::optional<std::string> c(eina::string_view("3"));

  ck_assert(!!a && !!b && !!c);

  eina::optional<double> a_s(a);
  eina::optional<std::string> b_s(b);
  eina::optional<eina::string_view> c_s(c);

  ck_assert(!!a_s && !!b_s && !!c_s);

  fail_if(1.0 != *a_s);
  fail_if(std::string("2") != *b_s);
  fail_if(eina::string_view("3") != *c_s);

  fail_if(1 != *a);
  fail_if("2" != *b);
  fail_if("3" != *c);

  fail_if(*a != *a_s);
  fail_if(*b != *b_s);
  fail_if(*c != *c_s);

  a_s = 4;
  b_s = "5";
  c_s = "6";

  a = a_s;
  b = b_s;
  c = c_s;

  fail_if(*a != *a_s);
  fail_if(*b != *b_s);
  fail_if(*c != *c_s);

  a = *a_s;
  b = *b_s;
  c = *c_s;

  fail_if(*a != *a_s);
  fail_if(*b != *b_s);
  fail_if(*c != *c_s);
}
EFL_END_TEST


void
eina_test_optional(TCase* tc)
{
  tcase_add_test(tc, eina_cxx_optional_constructors);
  tcase_add_test(tc, eina_cxx_optional_rel_ops);
  tcase_add_test(tc, eina_cxx_optional_assignment);
  tcase_add_test(tc, eina_cxx_optional_convertible_types);
}
