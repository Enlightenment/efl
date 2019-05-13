#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <iostream>

#include <Eina.hh>
#include <eina_list.hh>
#include <Eo.hh>

#include "eina_cxx_suite.h"

extern "C" {
#include "eina_simple.eo.h"
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

EFL_START_TEST(eina_cxx_ptrlist_push_back)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  int result[] = {5, 10, 15};
  int rresult[] = {15, 10, 5};

  wrapper const w1(efl_new(EINA_SIMPLE_CLASS));
  wrapper const w2(efl_new(EINA_SIMPLE_CLASS));
  wrapper const w3(efl_new(EINA_SIMPLE_CLASS));

  {
    efl::eina::ptr_list<int> list;

    list.push_back(new int(5));
    list.push_back(new int(10));
    list.push_back(new int(15));

    ck_assert(list.size() == 3);
    ck_assert(std::equal(list.begin(), list.end(), result));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult));
  }
  {
    efl::eina::list<int> list;
    list.push_back(new int(5));
    list.push_back(new int(10));
    list.push_back(new int(15));

    ck_assert(list.size() == 3);
    ck_assert(std::equal(list.begin(), list.end(), result));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult));
  }
  {
    wrapper result_[] = {w1, w2, w3};
    wrapper rresult_[] = {w3, w2, w1};

    efl::eina::list<wrapper> list;
    list.push_back(w1);
    list.push_back(w2);
    list.push_back(w3);

    ck_assert(list.size() == 3);
    ck_assert(std::equal(list.begin(), list.end(), result_));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult_));
  }
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrlist_pop_back)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  int result[] = {5, 10};
  int rresult[] = {10, 5};

  wrapper const w1(efl_new(EINA_SIMPLE_CLASS));
  wrapper const w2(efl_new(EINA_SIMPLE_CLASS));
  wrapper const w3(efl_new(EINA_SIMPLE_CLASS));

  {
    efl::eina::ptr_list<int> list;

    list.push_back(new int(5));
    list.push_back(new int(10));
    list.push_back(new int(15));
    list.pop_back();

    ck_assert(list.size() == 2);
    ck_assert(std::equal(list.begin(), list.end(), result));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult));
  }
  {
    efl::eina::list<int> list;

    list.push_back(new int(5));
    list.push_back(new int(10));
    list.push_back(new int(15));
    list.pop_back();

    ck_assert(list.size() == 2);
    ck_assert(std::equal(list.begin(), list.end(), result));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult));
  }
  {
    wrapper result_[] = {w1, w2};
    wrapper rresult_[] = {w2, w1};

    efl::eina::list<wrapper> list;

    list.push_back(w1);
    list.push_back(w2);
    list.push_back(w3);
    list.pop_back();

    ck_assert(list.size() == 2);
    ck_assert(std::equal(list.begin(), list.end(), result_));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult_));
  }
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrlist_push_front)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  {
    efl::eina::ptr_list<int> list;

    list.push_front(new int(5));
    list.push_front(new int(10));
    list.push_front(new int(15));

    int result[] = {15, 10, 5};
    int rresult[] = {5, 10, 15};

    ck_assert(list.size() == 3);
    ck_assert(std::equal(list.begin(), list.end(), result));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult));
  }
  {
    wrapper const w1(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w2(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w3(efl_new(EINA_SIMPLE_CLASS));

    efl::eina::list<wrapper> list;

    list.push_front(w1);
    list.push_front(w2);
    list.push_front(w3);

    wrapper result[] = {w3, w2, w1};
    wrapper rresult[] = {w1, w2, w3};

    ck_assert(list.size() == 3);
    ck_assert(std::equal(list.begin(), list.end(), result));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult));
  }
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrlist_pop_front)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  int result[] = {10, 5};
  int rresult[] = {5, 10};

  wrapper const w1(efl_new(EINA_SIMPLE_CLASS));
  wrapper const w2(efl_new(EINA_SIMPLE_CLASS));
  wrapper const w3(efl_new(EINA_SIMPLE_CLASS));

  {
    efl::eina::ptr_list<int> list;

    list.push_front(new int(5));
    list.push_front(new int(10));
    list.push_front(new int(15));
    list.pop_front();

    ck_assert(list.size() == 2);
    ck_assert(std::equal(list.begin(), list.end(), result));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult));
  }
  {
    efl::eina::list<int> list;

    list.push_front(new int(5));
    list.push_front(new int(10));
    list.push_front(new int(15));
    list.pop_front();

    ck_assert(list.size() == 2);
    ck_assert(std::equal(list.begin(), list.end(), result));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult));
  }
  {
    wrapper result_[] = {w2, w1};
    wrapper rresult_[] = {w1, w2};

    efl::eina::list<wrapper> list;

    list.push_front(w1);
    list.push_front(w2);
    list.push_front(w3);
    list.pop_front();

    ck_assert(list.size() == 2);
    ck_assert(std::equal(list.begin(), list.end(), result_));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult_));
  }
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrlist_insert)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  {
    efl::eina::ptr_list<int> list;

    efl::eina::ptr_list<int>::iterator it;

    it = list.insert(list.end(), new int(5)); // first element
    ck_assert(it != list.end());
    ++it;
    ck_assert(it == list.end());

    it = list.insert(list.end(), new int(10));  // equivalent to push_back
    ck_assert(it != list.end());
    ++it;
    ck_assert(it == list.end());

    it = list.insert(list.begin(), new int(15)); // equivalent to push_front
    ck_assert(it == list.begin());

    it = list.end();
    --it;
    list.insert(it, new int(20)); // insert before the last element

    int result[] = {15, 5, 20, 10};
    int rresult[] = {10, 20, 5, 15};

    ck_assert(list.size() == 4);
    ck_assert(std::equal(list.begin(), list.end(), result));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult));

    efl::eina::ptr_list<int, efl::eina::heap_copy_allocator> list2;
    it = list2.insert(list2.end(), list.begin(), list.end());
    ck_assert(it == list2.begin());
    ck_assert(list == list2);

    efl::eina::ptr_list<int, efl::eina::heap_copy_allocator> list3;
    list3.push_back(1);
    it = list3.insert(list3.end(), list.begin(), list.end());
    ck_assert(list3.size() == 5);
    ck_assert(list3.front() == 1);
    it = list3.begin();
    ++it;
    ck_assert(std::equal(it, list3.end(), list.begin()));

    efl::eina::ptr_list<int, efl::eina::heap_copy_allocator> list4;
    list4.push_back(1);
    it = list4.insert(list4.begin(), list.begin(), list.end());
    ck_assert(list4.size() == 5);
    ck_assert(list4.back() == 1);
    ck_assert(std::equal(list.begin(), list.end(), list4.begin()));
  }
  {
    wrapper const w0(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w1(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w2(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w3(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w4(efl_new(EINA_SIMPLE_CLASS));

    efl::eina::list<wrapper> list;

    efl::eina::list<wrapper>::iterator it;

    it = list.insert(list.end(), w1); // first element
    ck_assert(it != list.end());
    ++it;
    ck_assert(it == list.end());

    it = list.insert(list.end(), w2);  // equivalent to push_back
    ck_assert(it != list.end());
    ++it;
    ck_assert(it == list.end());

    it = list.insert(list.begin(), w3); // equivalent to push_front
    ck_assert(it == list.begin());

    it = list.end();
    --it;
    list.insert(it, w4); // insert before the last element

    wrapper result[] = {w3, w1, w4, w2};
    wrapper rresult[] = {w2, w4, w1, w3};

    ck_assert(list.size() == 4);
    ck_assert(std::equal(list.begin(), list.end(), result));
    ck_assert(std::equal(list.rbegin(), list.rend(), rresult));

    efl::eina::list<wrapper> list2;
    it = list2.insert(list2.end(), list.begin(), list.end());
    ck_assert(it == list2.begin());
    ck_assert(list == list2);

    efl::eina::list<wrapper> list3;
    list3.push_back(w0);
    it = list3.insert(list3.end(), list.begin(), list.end());
    ck_assert(list3.size() == 5);
    ck_assert(list3.front() == w0);
    it = list3.begin();
    ++it;
    ck_assert(std::equal(it, list3.end(), list.begin()));

    efl::eina::list<wrapper> list4;
    list4.push_back(w0);
    it = list4.insert(list4.begin(), list.begin(), list.end());
    ck_assert(list4.size() == 5);
    ck_assert(list4.back() == w0);
    ck_assert(std::equal(list.begin(), list.end(), list4.begin()));
  }
}
EFL_END_TEST


EFL_START_TEST(eina_cxx_ptrlist_malloc_clone_allocator)
{
  efl::eina::eina_init eina_init;

  efl::eina::ptr_list<int, efl::eina::malloc_clone_allocator> list1;
  list1.push_back(5);
  list1.push_back(10);

  efl::eina::ptr_list<int, efl::eina::malloc_clone_allocator> list2 = list1;
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrlist_constructors)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  {
    efl::eina::ptr_list<int> list1;
    ck_assert(list1.empty());

    efl::eina::ptr_list<int, efl::eina::heap_copy_allocator> list2(10, 5);
    ck_assert(list2.size() == 10);
    ck_assert(std::find_if(list2.begin(), list2.end()
                           , std::not1(std::bind1st(std::equal_to<int>(), 5))) == list2.end());

    efl::eina::ptr_list<int, efl::eina::heap_copy_allocator> list3(list2);
    ck_assert(list2 == list3);

    efl::eina::ptr_list<int, efl::eina::heap_copy_allocator> list4
      (list2.begin(), list2.end());
    ck_assert(list2 == list4);
  }
  {
    wrapper const w0(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w1(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w2(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w3(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w4(efl_new(EINA_SIMPLE_CLASS));

    efl::eina::list<wrapper> list1;
    ck_assert(list1.empty());

    efl::eina::list<wrapper> list2(10, w1);
    ck_assert(list2.size() == 10);
    ck_assert(std::find_if(list2.begin(), list2.end()
                           , [&list2, w2] (wrapper i)
                           {
                             return  i == w2;
                           }
                           ) == list2.end());

    efl::eina::list<wrapper> list3(list2);
    ck_assert(list2 == list3);

    efl::eina::list<wrapper> list4
      (list2.begin(), list2.end());
    ck_assert(list2 == list4);
  }
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrlist_erase)
{
  efl::eina::eina_init eina_init;

  efl::eina::ptr_list<int> list1;
  list1.push_back(new int(5));
  list1.push_back(new int(10));
  list1.push_back(new int(15));
  list1.push_back(new int(20));
  list1.push_back(new int(25));
  list1.push_back(new int(30));

  efl::eina::ptr_list<int>::iterator it = list1.begin(), it2;

  it = list1.erase(it);
  ck_assert(it == list1.begin());
  ck_assert(list1.size() == 5);
  ck_assert(list1.front() == 10);

  it = list1.begin();
  it2 = list1.begin();
  ++it;
  ++it2; ++it2;
  ck_assert(*it2 == 20);
  it = list1.erase(it);
  ck_assert(it == it2);
  ck_assert(list1.size() == 4);
  ck_assert(*it2 == 20);

  it = list1.end();
  --it;
  it = list1.erase(it);
  ck_assert(it == list1.end());
  ck_assert(list1.size() == 3);
  ck_assert(list1.back() == 25);

  it = list1.begin();
  ++it;
  it2 = list1.end();
  --it2;
  it = list1.erase(it, it2);
  it2 = list1.end();
  --it2;
  ck_assert(it == it2);
  ck_assert(list1.size() == 2);
  ck_assert(list1.front() == 10);
  ck_assert(list1.back() == 25);
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrlist_range)
{
  efl::eina::eina_init eina_init;

  efl::eina::ptr_list<int> list;
  list.push_back(new int(5));
  list.push_back(new int(10));
  list.push_back(new int(15));
  list.push_back(new int(20));
  list.push_back(new int(25));
  list.push_back(new int(30));

  efl::eina::range_ptr_list<int> range_list(list);

  ck_assert(range_list.size() == 6u);

  int result[] = {5, 10, 15, 20, 25, 30};
  int rresult[] = {30, 25, 20, 15, 10, 5};
  ck_assert(std::equal(range_list.begin(), range_list.end(), result));
  ck_assert(std::equal(range_list.rbegin(), range_list.rend(), rresult));

  efl::eina::range_ptr_list<int const> const_range_list(list);

  ck_assert(const_range_list.size() == 6u);
  ck_assert(std::equal(range_list.begin(), range_list.end(), result));
  ck_assert(std::equal(range_list.rbegin(), range_list.rend(), rresult));

  *range_list.begin() = 0;
  ck_assert(*const_range_list.begin() == 0);
  ck_assert(*list.begin() == 0);
}
EFL_END_TEST

void
eina_test_ptrlist(TCase* tc)
{
  tcase_add_test(tc, eina_cxx_ptrlist_push_back);
  tcase_add_test(tc, eina_cxx_ptrlist_pop_back);
  tcase_add_test(tc, eina_cxx_ptrlist_push_front);
  tcase_add_test(tc, eina_cxx_ptrlist_pop_front);
  tcase_add_test(tc, eina_cxx_ptrlist_insert);
  tcase_add_test(tc, eina_cxx_ptrlist_constructors);
  tcase_add_test(tc, eina_cxx_ptrlist_erase);
  tcase_add_test(tc, eina_cxx_ptrlist_range);
  tcase_add_test(tc, eina_cxx_ptrlist_malloc_clone_allocator);
}
