
#include "Eina.hh"

#include <check.h>

START_TEST(eina_cxx_value_constructors)
{
  efl::eina::eina_init init;

  efl::eina::value v;

  char c = 'c';
  efl::eina::value vchar(c);

  short s = 5;
  efl::eina::value vshort(s);

  efl::eina::value vint(5);

  efl::eina::value vlong(5l);

  unsigned char uc = 'b';
  efl::eina::value vuchar(uc);

  unsigned short us = 5;
  efl::eina::value vushort(us);

  efl::eina::value vuint(5u);

  efl::eina::value vulong(5ul);

  efl::eina::value vu64(uint64_t(5ul));

  efl::eina::value vfloat(5.0f);

  efl::eina::value vdouble(5.0);
}
END_TEST

START_TEST(eina_cxx_value_get)
{
  efl::eina::eina_init init;

  char c = 'c';
  efl::eina::value vchar(c);
  ck_assert(efl::eina::get<char>(vchar) == 'c');

  short s = 5;
  efl::eina::value vshort(s);
  ck_assert(efl::eina::get<short>(vshort) == 5);

  efl::eina::value vint(6);
  ck_assert(efl::eina::get<int>(vint) == 6);

  efl::eina::value vlong(7l);
  ck_assert(efl::eina::get<long>(vlong) == 7l);

  unsigned char uc = 'b';
  efl::eina::value vuchar(uc);
  ck_assert(efl::eina::get<unsigned char>(vuchar) == 'b');

  unsigned short us = 8;
  efl::eina::value vushort(us);
  ck_assert(efl::eina::get<unsigned short>(vushort) == 8);

  efl::eina::value vuint(9u);
  ck_assert(efl::eina::get<unsigned int>(vuint) == 9u);

  efl::eina::value vulong(10ul);
  ck_assert(efl::eina::get<unsigned long>(vulong) == 10ul);

  efl::eina::value vu64((uint64_t)10ul);
  ck_assert(efl::eina::get<uint64_t>(vu64) == 10ul);

  efl::eina::value vfloat(11.0f);
  ck_assert(efl::eina::get<float>(vfloat) == 11.0f);

  efl::eina::value vdouble(12.0);
  ck_assert(efl::eina::get<double>(vdouble) == 12.0f);
}
END_TEST

START_TEST(eina_cxx_value_wrong_get)
{
  efl::eina::eina_init init;

  char c = 'c';
  efl::eina::value vchar(c);
  try
  {
    efl::eina::get<int>(vchar);
    std::abort();
  }
  catch(efl::eina::system_error const&)
  {
  }
}
END_TEST

START_TEST(eina_cxx_value_comparison_operators)
{
  efl::eina::eina_init init;

  efl::eina::value v;

  char c = 5;
  efl::eina::value vchar(c);

  short s = 5;
  efl::eina::value vshort(s);

  efl::eina::value vint(5);

  efl::eina::value vlong(5l);

  unsigned char uc = 5;
  efl::eina::value vuchar(uc);

  unsigned short us = 5;
  efl::eina::value vushort(us);

  efl::eina::value vuint(5u);

  efl::eina::value vulong(5ul);

  efl::eina::value vu64((uint64_t)5ul);

  efl::eina::value vfloat(5.0f);

  efl::eina::value vdouble(5.0);

  ck_assert(vchar == vchar);
  ck_assert(vshort == vshort);
  ck_assert(vint == vint);
  ck_assert(vlong == vlong);
  ck_assert(vuchar == vuchar);
  ck_assert(vushort == vushort);
  ck_assert(vuint == vuint);
  ck_assert(vulong == vulong);
  ck_assert(vu64 == vu64);
  ck_assert(vfloat == vfloat);
  ck_assert(vdouble == vdouble);

  ck_assert(vchar != vshort);
  ck_assert(vshort != vint);
  ck_assert(vint != vlong);
  ck_assert(vlong != vuchar);
  ck_assert(vuchar != vushort);
  ck_assert(vushort != vuint);
  ck_assert(vuint != vulong);
  ck_assert(vulong != vfloat);
  ck_assert(vfloat != vdouble);
  ck_assert(vdouble != vchar);

  ck_assert(vchar != vuchar);
  ck_assert(vshort != vushort);
  ck_assert(vint != vuint);
  ck_assert(vlong != vulong);
  ck_assert(vfloat != vdouble);
  ck_assert(vdouble != vfloat);
}
END_TEST

START_TEST(eina_cxx_value_copying)
{
  char c = 5;

  efl::eina::value vchar(c);
  efl::eina::value vchar2(vchar);
  ck_assert(vchar == vchar2);
  ck_assert(efl::eina::get<char>(vchar) == 5);
  ck_assert(efl::eina::get<char>(vchar2) == 5);

  efl::eina::value vint(10);
  vchar = vint;
  ck_assert(vchar != vchar2);
  ck_assert(vint == vchar);
  ck_assert(efl::eina::get<int>(vchar) == 10);
  ck_assert(efl::eina::get<int>(vint) == 10);
}
END_TEST

void
eina_test_value(TCase* tc)
{
  tcase_add_test(tc, eina_cxx_value_constructors);
  tcase_add_test(tc, eina_cxx_value_get);
  tcase_add_test(tc, eina_cxx_value_wrong_get);
  tcase_add_test(tc, eina_cxx_value_comparison_operators);
  tcase_add_test(tc, eina_cxx_value_copying);
}
