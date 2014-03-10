
#include "Eina.hh"

#include <algorithm>

#include <check.h>

START_TEST(eina_cxx_ptrlist_push_back)
{
  efl::eina::eina_init eina_init;

  efl::eina::ptr_list<int> list;

  list.push_back(new int(5));
  list.push_back(new int(10));
  list.push_back(new int(15));

  int result[] = {5, 10, 15};

  ck_assert(list.size() == 3);
  ck_assert(std::equal(list.begin(), list.end(), result));
}
END_TEST

START_TEST(eina_cxx_ptrlist_pop_back)
{
  efl::eina::eina_init eina_init;

  efl::eina::ptr_list<int> list;

  list.push_back(new int(5));
  list.push_back(new int(10));
  list.push_back(new int(15));
  list.pop_back();

  int result[] = {5, 10};

  ck_assert(list.size() == 2);
  ck_assert(std::equal(list.begin(), list.end(), result));
}
END_TEST

START_TEST(eina_cxx_ptrlist_push_front)
{
  efl::eina::eina_init eina_init;

  efl::eina::ptr_list<int> list;

  list.push_front(new int(5));
  list.push_front(new int(10));
  list.push_front(new int(15));

  int result[] = {15, 10, 5};

  ck_assert(list.size() == 3);
  ck_assert(std::equal(list.begin(), list.end(), result));
}
END_TEST

START_TEST(eina_cxx_ptrlist_pop_front)
{
  efl::eina::eina_init eina_init;

  efl::eina::ptr_list<int> list;

  list.push_front(new int(5));
  list.push_front(new int(10));
  list.push_front(new int(15));
  list.pop_front();

  int result[] = {10, 5};

  ck_assert(list.size() == 2);
  ck_assert(std::equal(list.begin(), list.end(), result));
}
END_TEST

START_TEST(eina_cxx_ptrlist_insert)
{
  efl::eina::eina_init eina_init;

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

  ck_assert(list.size() == 4);
  ck_assert(std::equal(list.begin(), list.end(), result));

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
END_TEST

START_TEST(eina_cxx_ptrlist_constructors)
{
  efl::eina::eina_init eina_init;

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
END_TEST

START_TEST(eina_cxx_ptrlist_erase)
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
END_TEST

START_TEST(eina_cxx_ptrlist_range)
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
  ck_assert(std::equal(range_list.begin(), range_list.end(), result));

  efl::eina::range_ptr_list<int const> const_range_list(list);

  ck_assert(const_range_list.size() == 6u);
  ck_assert(std::equal(range_list.begin(), range_list.end(), result));

  *range_list.begin() = 0;
  ck_assert(*const_range_list.begin() == 0);
  ck_assert(*list.begin() == 0);
}
END_TEST

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
}
