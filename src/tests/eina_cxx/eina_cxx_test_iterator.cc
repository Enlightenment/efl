
#include "Eina.hh"

#include <algorithm>

#include <check.h>

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

void
eina_test_iterator(TCase *tc)
{
  tcase_add_test(tc, eina_cxx_iterator_equal);
}
