
#include "Eina.hh"

#include <algorithm>
#include <functional>

#include <check.h>

START_TEST(eina_cxx_stringshare_constructors)
{
  efl::eina::eina_init eina_init;

  efl::eina::stringshare string1;
  ck_assert(string1.empty());

  efl::eina::stringshare string2("string");
  ck_assert(string2.size() == 6);
  ck_assert(string2 == "string");

  efl::eina::stringshare string3(string2);
  ck_assert(string2 == string3);

  efl::eina::stringshare string4(string3.begin(), string3.end());
  ck_assert(string2 == string3);
}
END_TEST

START_TEST(eina_cxx_stringshare_iterators)
{
  efl::eina::eina_init eina_init;

  const char str[] = "string";
  const char rstr[] = "gnirts";

  efl::eina::stringshare string(str);
  ck_assert(string.size() == 6);
  ck_assert(string == str);

  ck_assert(std::equal(string.begin(), string.end(), str));
  ck_assert(std::equal(string.rbegin(), string.rend(), rstr));
  ck_assert(std::equal(string.cbegin(), string.cend(), str));
  ck_assert(std::equal(string.crbegin(), string.crend(), rstr));
}
END_TEST

void
eina_test_stringshare(TCase *tc)
{
  tcase_add_test(tc, eina_cxx_stringshare_constructors);
  tcase_add_test(tc, eina_cxx_stringshare_iterators);
}
