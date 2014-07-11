
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eina.hh"

#include <algorithm>

#include <check.h>

START_TEST(eina_cxx_accessor_indexing)
{
  efl::eina::ptr_list<int> list;
  list.push_back(new int(5));
  list.push_back(new int(10));
  list.push_back(new int(15));
  list.push_back(new int(20));

  efl::eina::accessor<int> accessor(list.accessor());

  ck_assert(accessor[0] == 5);
  ck_assert(accessor[1] == 10);
  ck_assert(accessor[2] == 15);
  ck_assert(accessor[3] == 20);
}
END_TEST

START_TEST(eina_cxx_accessor_iterator)
{
  efl::eina::ptr_list<int> list;
  list.push_back(new int(5));
  list.push_back(new int(10));
  list.push_back(new int(15));
  list.push_back(new int(20));

  std::size_t pos = 0u;
  for(efl::eina::accessor_iterator<int> first (list.accessor())
        , last (list.accessor(), list.size()); first != last; ++first, ++pos)
    {
      ck_assert(pos != 0u || *first == 5);
      ck_assert(pos != 1u || *first == 10);
      ck_assert(pos != 2u || *first == 15);
      ck_assert(pos != 3u || *first == 20);
    }
}
END_TEST

START_TEST(eina_cxx_accessor_relops)
{
  efl::eina::ptr_list<int> list;
  list.push_back(new int(5));
  list.push_back(new int(10));
  list.push_back(new int(15));
  list.push_back(new int(20));

  efl::eina::accessor_iterator<int> first (list.accessor())
    , second(list.accessor(), 1u)
    , third(list.accessor(), 2u)
    , fourth(list.accessor(), 3u)
    ;
  ck_assert(!(first < first)); ck_assert(first < second);
  ck_assert(first < third); ck_assert(first < fourth);
  ck_assert(!(second < first)); ck_assert(!(second < second));
  ck_assert(second < third); ck_assert(second < fourth);
  ck_assert(!(third < first)); ck_assert(!(third < second));
  ck_assert(!(third < third)); ck_assert(third < fourth);
  ck_assert(!(fourth < first)); ck_assert(!(fourth < second));
  ck_assert(!(fourth < third)); ck_assert(!(fourth < fourth));

  ck_assert(first <= first); ck_assert(first <= second);
  ck_assert(first <= third); ck_assert(first <= fourth);
  ck_assert(!(second <= first)); ck_assert(second <= second);
  ck_assert(second <= third); ck_assert(second <= fourth);
  ck_assert(!(third <= first)); ck_assert(!(third <= second));
  ck_assert(third <= third); ck_assert(third <= fourth);
  ck_assert(!(fourth <= first)); ck_assert(!(fourth <= second));
  ck_assert(!(fourth <= third)); ck_assert(fourth <= fourth);

  ck_assert(!(first > first)); ck_assert(!(first > second));
  ck_assert(!(first > third)); ck_assert(!(first > fourth));
  ck_assert(second > first); ck_assert(!(second > second));
  ck_assert(!(second > third)); ck_assert(!(second > fourth));
  ck_assert(third > first); ck_assert(third > second);
  ck_assert(!(third > third)); ck_assert(!(third > fourth));
  ck_assert(fourth > first); ck_assert(fourth > second);
  ck_assert(fourth > third); ck_assert(!(fourth > fourth));

  ck_assert(first >= first); ck_assert(!(first >= second));
  ck_assert(!(first >= third)); ck_assert(!(first >= fourth));
  ck_assert(second >= first); ck_assert(second >= second);
  ck_assert(!(second >= third)); ck_assert(!(second >= fourth));
  ck_assert(third >= first); ck_assert(third >= second);
  ck_assert(third >= third); ck_assert(!(third >= fourth));
  ck_assert(fourth >= first); ck_assert(fourth >= second);
  ck_assert(fourth >= third); ck_assert(fourth >= fourth);
}
END_TEST

void
eina_test_accessor(TCase* tc)
{
  tcase_add_test(tc, eina_cxx_accessor_indexing);
  tcase_add_test(tc, eina_cxx_accessor_iterator);
  tcase_add_test(tc, eina_cxx_accessor_relops);
}
