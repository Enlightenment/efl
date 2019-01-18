#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.hh>
#include <Eo.hh>

#include "eina_cxx_suite.h"

extern "C" {
#include "simple.eo.h"
}

struct wrapper : efl::eo::concrete
{
  explicit wrapper(Eo* o)
    : concrete(o) {}
};

namespace efl { namespace eo {

template <>
struct is_eolian_object< ::wrapper> : std::true_type {};
    
} }

EFL_START_TEST(eina_cxx_accessor_indexing)
{
  efl::eina::eina_init eina_init;

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
EFL_END_TEST

EFL_START_TEST(eina_cxx_eo_accessor_indexing)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  efl::eina::list<wrapper> list;

  wrapper const w1(efl_new(SIMPLE_CLASS));
  wrapper const w2(efl_new(SIMPLE_CLASS));
  wrapper const w3(efl_new(SIMPLE_CLASS));
  wrapper const w4(efl_new(SIMPLE_CLASS));

  list.push_back(w1);
  list.push_back(w2);
  list.push_back(w3);
  list.push_back(w4);

  efl::eina::accessor<wrapper> accessor(list.accessor());

  ck_assert(accessor[0] == w1);
  ck_assert(accessor[1] == w2);
  ck_assert(accessor[2] == w3);
  ck_assert(accessor[3] == w4);
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_accessor_iterator)
{
  efl::eina::eina_init eina_init;

  efl::eina::ptr_list<int> list;
  list.push_back(new int(5));
  list.push_back(new int(10));
  list.push_back(new int(15));
  list.push_back(new int(20));

  std::size_t pos = 0u;
  for(efl::eina::accessor_iterator<int> first (list.accessor())
        , last (list.accessor(), list.size()); first != last; ++first, ++pos)
    {
       if(pos >= 4u)
         {
            ck_assert_msg(0, "accessor_iterator out of bounds");
            break;
         }

      ck_assert(pos != 0u || *first == 5);
      ck_assert(pos != 1u || *first == 10);
      ck_assert(pos != 2u || *first == 15);
      ck_assert(pos != 3u || *first == 20);
    }
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_eo_accessor_iterator)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  efl::eina::list<wrapper> list;

  wrapper const w1(efl_new(SIMPLE_CLASS));
  wrapper const w2(efl_new(SIMPLE_CLASS));
  wrapper const w3(efl_new(SIMPLE_CLASS));
  wrapper const w4(efl_new(SIMPLE_CLASS));

  list.push_back(w1);
  list.push_back(w2);
  list.push_back(w3);
  list.push_back(w4);

  std::size_t pos = 0u;
  for(efl::eina::accessor_iterator<wrapper> first (list.accessor())
        , last (list.accessor(), list.size()); first != last; ++first, ++pos)
    {
       if(pos >= 4u)
         {
            ck_assert_msg(0, "accessor_iterator out of bounds");
            break;
         }

       ck_assert(pos != 0u || *first == w1);
       ck_assert(pos != 1u || *first == w2);
       ck_assert(pos != 2u || *first == w3);
       ck_assert(pos != 3u || *first == w4);
    }
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_accessor_relops)
{
  efl::eina::eina_init eina_init;

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
EFL_END_TEST

void
eina_test_accessor(TCase* tc)
{
  tcase_add_test(tc, eina_cxx_accessor_indexing);
  tcase_add_test(tc, eina_cxx_eo_accessor_indexing);
  tcase_add_test(tc, eina_cxx_accessor_iterator);
  tcase_add_test(tc, eina_cxx_eo_accessor_iterator);
  tcase_add_test(tc, eina_cxx_accessor_relops);
}
