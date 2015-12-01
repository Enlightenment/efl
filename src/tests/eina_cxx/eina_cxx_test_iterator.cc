#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.hh>
#include <Eo.hh>

#include <algorithm>

#include "eina_cxx_suite.h"
#include "simple.eo.hh"

START_TEST(eina_cxx_iterator_equal)
{
  efl::eina::eina_init eina_init;

  efl::eina::ptr_list<int> list;
  ck_assert(list.size() == 0);
  ck_assert(list.empty());

  list.push_back(new int(5));
  list.push_back(new int(10));
  list.push_back(new int(15));
  list.push_back(new int(20));

  efl::eina::iterator<int> iterator = list.ibegin()
    , last_iterator = list.iend();

  int result[] = {5, 10, 15, 20};

  ck_assert(std::equal(iterator, last_iterator, result));
}
END_TEST

START_TEST(eina_cxx_eo_iterator_equal)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  efl::eina::list<simple> list;

  simple const w1;
  simple const w2;
  simple const w3;
  simple const w4;

  list.push_back(w1);
  list.push_back(w2);
  list.push_back(w3);
  list.push_back(w4);

  efl::eina::iterator<simple> iterator = list.ibegin()
    , last_iterator = list.iend();

  simple const result[] = {w1, w2, w3, w4};

  ck_assert(std::equal(iterator, last_iterator, result));
}
END_TEST

void
eina_test_iterator(TCase *tc)
{
  tcase_add_test(tc, eina_cxx_iterator_equal);
  tcase_add_test(tc, eina_cxx_eo_iterator_equal);
}
