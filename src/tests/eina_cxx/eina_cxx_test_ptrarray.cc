
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eina.hh"
#include "Eo.hh"

#include <eina_array.hh>

#include <algorithm>

#include <check.h>

const Eo_Class *simple_class_get(void);
#define MY_CLASS simple_class_get()

struct wrapper : efl::eo::base
{
  explicit wrapper(Eo* o)
    : base(o) {}
};

START_TEST(eina_cxx_ptrarray_push_back)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  int result[] = {5, 10, 15};
  int rresult[] = {15, 10, 5};

  wrapper const w1(eo_add(MY_CLASS, NULL));
  wrapper const w2(eo_add(MY_CLASS, NULL));
  wrapper const w3(eo_add(MY_CLASS, NULL));
  
  {
    efl::eina::ptr_array<int> array;

    array.push_back(new int(5));
    array.push_back(new int(10));
    array.push_back(new int(15));

    ck_assert(array.size() == 3);
    ck_assert(std::equal(array.begin(), array.end(), result));
    ck_assert(std::equal(array.rbegin(), array.rend(), rresult));
  }
  {
    efl::eina::array<int> array;

    array.push_back(new int(5));
    array.push_back(new int(10));
    array.push_back(new int(15));

    ck_assert(array.size() == 3);
    ck_assert(std::equal(array.begin(), array.end(), result));
    ck_assert(std::equal(array.rbegin(), array.rend(), rresult));
  }
  {
    wrapper result_[] = {w1, w2, w3};
    wrapper rresult_[] = {w3, w2, w1};

    efl::eina::array<wrapper> array;

    array.push_back(w1);
    array.push_back(w2);
    array.push_back(w3);

    ck_assert(array.size() == 3);
    ck_assert(std::equal(array.begin(), array.end(), result_));
    ck_assert(std::equal(array.rbegin(), array.rend(), rresult_));
  }
}
END_TEST

START_TEST(eina_cxx_ptrarray_pop_back)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  int result[] = {5, 10};
  int rresult[] = {10, 5};

  {
    efl::eina::ptr_array<int> array;

    array.push_back(new int(5));
    array.push_back(new int(10));
    array.push_back(new int(15));
    array.pop_back();

    ck_assert(array.size() == 2);
    ck_assert(std::equal(array.begin(), array.end(), result));
    ck_assert(std::equal(array.rbegin(), array.rend(), rresult));
  }
  {
    efl::eina::array<int> array;

    array.push_back(new int(5));
    array.push_back(new int(10));
    array.push_back(new int(15));
    array.pop_back();

    ck_assert(array.size() == 2);
    ck_assert(std::equal(array.begin(), array.end(), result));
    ck_assert(std::equal(array.rbegin(), array.rend(), rresult));
  }
  {
    wrapper const w1(eo_add(MY_CLASS, NULL));
    wrapper const w2(eo_add(MY_CLASS, NULL));
    wrapper const w3(eo_add(MY_CLASS, NULL));

    efl::eina::array<wrapper> array;

    array.push_back(w1);
    array.push_back(w2);
    array.push_back(w3);
    array.pop_back();

    wrapper result_[] = {w1, w2};
    wrapper rresult_[] = {w2, w1};

    ck_assert(array.size() == 2);
    ck_assert(std::equal(array.begin(), array.end(), result_));
    ck_assert(std::equal(array.rbegin(), array.rend(), rresult_));
  }
}
END_TEST

START_TEST(eina_cxx_ptrarray_insert)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  {
    efl::eina::ptr_array<int> array;
    ck_assert(std::distance(array.begin(), array.end()) == 0u);
    ck_assert(std::distance(array.rbegin(), array.rend()) == 0u);

    efl::eina::ptr_array<int>::iterator it;

    it = array.insert(array.end(), new int(5)); // first element
    ck_assert(it != array.end());
    ++it;
    ck_assert(it == array.end());
    ck_assert(array[0] == 5);
    ck_assert(std::distance(array.begin(), array.end()) == 1u);

    it = array.insert(array.end(), new int(10));  // equivalent to push_back
    ck_assert(it != array.end());
    ++it;
    ck_assert(it == array.end());
    ck_assert(array[0] == 5);
    ck_assert(array[1] == 10);
    ck_assert(std::distance(array.begin(), array.end()) == 2u);

    it = array.insert(array.begin(), new int(15)); // equivalent to push_front
    ck_assert(it == array.begin());

    ck_assert(array[1] == 5);
    ck_assert(array[2] == 10);
    ck_assert(array[0] == 15);
    ck_assert(std::distance(array.begin(), array.end()) == 3u);

    array.insert(array.end() - 1, new int(20)); // insert before the last element
    ck_assert(array[0] == 15);
    ck_assert(array[1] == 5);
    ck_assert(array[2] == 20);
    ck_assert(array[3] == 10);
    ck_assert(std::distance(array.begin(), array.end()) == 4u);

    int result[] = {15, 5, 20, 10};
    int rresult[] = {10, 20, 5, 15};

    ck_assert(array.size() == 4);
    ck_assert(std::distance(array.begin(), array.end()) == 4u);
    ck_assert(std::distance(array.rbegin(), array.rend()) == 4u);
    ck_assert(std::equal(array.begin(), array.end(), result));
    ck_assert(std::equal(array.rbegin(), array.rend(), rresult));

    efl::eina::ptr_array<int, efl::eina::heap_copy_allocator> array2;
    it = array2.insert(array2.end(), array.begin(), array.end());
    ck_assert(it == array2.begin());
    ck_assert(array == array2);


    efl::eina::ptr_array<int, efl::eina::heap_copy_allocator> array3;
    array3.push_back(1);
    it = array3.insert(array3.end(), array.begin(), array.end());
    ck_assert(array3.size() == 5);
    ck_assert(array3.front() == 1);
    it = array3.begin();
    ++it;
    ck_assert(std::equal(it, array3.end(), array.begin()));

    efl::eina::ptr_array<int, efl::eina::heap_copy_allocator> array4;
    array4.push_back(1);
    it = array4.insert(array4.begin(), array.begin(), array.end());
    ck_assert(array4.size() == 5);
    ck_assert(array4.back() == 1);
    ck_assert(std::equal(array.begin(), array.end(), array4.begin()));
  }
  {
    wrapper const w0(eo_add(MY_CLASS, NULL));
    wrapper const w1(eo_add(MY_CLASS, NULL));
    wrapper const w2(eo_add(MY_CLASS, NULL));
    wrapper const w3(eo_add(MY_CLASS, NULL));
    wrapper const w4(eo_add(MY_CLASS, NULL));

    efl::eina::array<wrapper> array;
    ck_assert(std::distance(array.begin(), array.end()) == 0u);
    ck_assert(std::distance(array.rbegin(), array.rend()) == 0u);

    efl::eina::array<wrapper>::iterator it;

    it = array.insert(array.end(), w1); // first element
    ck_assert(it != array.end());
    ++it;
    ck_assert(it == array.end());
    ck_assert(array[0] == w1);
    ck_assert(std::distance(array.begin(), array.end()) == 1u);

    it = array.insert(array.end(), w2);  // equivalent to push_back
    ck_assert(it != array.end());
    ++it;
    ck_assert(it == array.end());
    ck_assert(array[0] == w1);
    ck_assert(array[1] == w2);
    ck_assert(std::distance(array.begin(), array.end()) == 2u);

    it = array.insert(array.begin(), w3); // equivalent to push_front
    ck_assert(it == array.begin());

    ck_assert(array[1] == w1);
    ck_assert(array[2] == w2);
    ck_assert(array[0] == w3);
    ck_assert(std::distance(array.begin(), array.end()) == 3u);

    array.insert(array.end() - 1, w4); // insert before the last element
    ck_assert(array[0] == w3);
    ck_assert(array[1] == w1);
    ck_assert(array[2] == w4);
    ck_assert(array[3] == w2);
    ck_assert(std::distance(array.begin(), array.end()) == 4u);

    wrapper result[] = {w3, w1, w4, w2};
    wrapper rresult[] = {w2, w4, w1, w3};

    ck_assert(array.size() == 4);
    ck_assert(std::distance(array.begin(), array.end()) == 4u);
    ck_assert(std::distance(array.rbegin(), array.rend()) == 4u);
    ck_assert(std::equal(array.begin(), array.end(), result));
    ck_assert(std::equal(array.rbegin(), array.rend(), rresult));

    efl::eina::array<wrapper> array2;
    it = array2.insert(array2.end(), array.begin(), array.end());
    ck_assert(it == array2.begin());
    ck_assert(array == array2);

    efl::eina::array<wrapper> array3;
    array3.push_back(w0);
    it = array3.insert(array3.end(), array.begin(), array.end());
    ck_assert(array3.size() == 5);
    ck_assert(array3.front() == w0);
    it = array3.begin();
    ++it;
    ck_assert(std::equal(it, array3.end(), array.begin()));

    efl::eina::array<wrapper> array4;
    array4.push_back(w0);
    it = array4.insert(array4.begin(), array.begin(), array.end());
    ck_assert(array4.size() == 5);
    ck_assert(array4.back() == w0);
    ck_assert(std::equal(array.begin(), array.end(), array4.begin()));
  }
}
END_TEST

START_TEST(eina_cxx_ptrarray_constructors)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  wrapper const w1(eo_add(MY_CLASS, NULL));

  {
    efl::eina::ptr_array<int> array1;
    ck_assert(array1.empty());

    efl::eina::ptr_array<int, efl::eina::heap_copy_allocator> array2(10, 5);
    ck_assert(array2.size() == 10);
    ck_assert(std::find_if(array2.begin(), array2.end()
                           , [](int i) { return i != 5; }) == array2.end());

    efl::eina::ptr_array<int, efl::eina::heap_copy_allocator> array3(array2);
    ck_assert(array2 == array3);

    efl::eina::ptr_array<int, efl::eina::heap_copy_allocator> array4
      (array2.begin(), array2.end());
    ck_assert(array2 == array4);
  }
  {
    efl::eina::array<wrapper> array1;
    ck_assert(array1.empty());

    efl::eina::array<wrapper> array2(10, w1);
    ck_assert(array2.size() == 10);
    ck_assert(std::find_if(array2.begin(), array2.end()
                           , [w1](wrapper i) { return i != w1; }) == array2.end());

    efl::eina::array<wrapper> array3(array2);
    ck_assert(array2 == array3);

    efl::eina::array<wrapper> array4
      (array2.begin(), array2.end());
    ck_assert(array2 == array4);
  }
}
END_TEST

START_TEST(eina_cxx_ptrarray_erase)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  {
    efl::eina::ptr_array<int> array1;
    array1.push_back(new int(5));
    array1.push_back(new int(10));
    array1.push_back(new int(15));
    array1.push_back(new int(20));
    array1.push_back(new int(25));
    array1.push_back(new int(30));

    int result[] = {5, 10, 15, 20, 25, 30};
    int rresult[] = {30, 25, 20, 15, 10, 5};
    ck_assert(std::equal(array1.begin(), array1.end(), result));
    ck_assert(std::equal(array1.rbegin(), array1.rend(), rresult));
  
    efl::eina::ptr_array<int>::iterator it = array1.erase(array1.begin());
    ck_assert(it == array1.begin());
    ck_assert(array1.size() == 5);
    ck_assert(array1.front() == 10);

    ck_assert(std::equal(array1.begin(), array1.end(), &result[1]));

    it = array1.erase(array1.begin() + 1);
    ck_assert(*it == 20);
    ck_assert(array1.size() == 4);
  
    it = array1.erase(array1.end() - 1);
    ck_assert(it == array1.end());
    ck_assert(array1.size() == 3);
    ck_assert(array1.back() == 25);

    it = array1.erase(array1.begin() + 1, array1.end() - 1);
    ck_assert(it == array1.end() - 1);
    ck_assert(array1.size() == 2);
    ck_assert(array1.front() == 10);
    ck_assert(array1.back() == 25);
  }
  {
    wrapper const w1(eo_add(MY_CLASS, NULL));
    wrapper const w2(eo_add(MY_CLASS, NULL));
    wrapper const w3(eo_add(MY_CLASS, NULL));
    wrapper const w4(eo_add(MY_CLASS, NULL));
    wrapper const w5(eo_add(MY_CLASS, NULL));
    wrapper const w6(eo_add(MY_CLASS, NULL));
    
    efl::eina::array<wrapper> array1;
    
    array1.push_back(w1);
    array1.push_back(w2);
    array1.push_back(w3);
    array1.push_back(w4);
    array1.push_back(w5);
    array1.push_back(w6);

    wrapper result_[] = {w1, w2, w3, w4, w5, w6};
    wrapper rresult[] = {w6, w5, w4, w3, w2, w1};
    ck_assert(std::equal(array1.begin(), array1.end(), result_));
    ck_assert(std::equal(array1.rbegin(), array1.rend(), rresult));
  
    efl::eina::array<wrapper>::iterator it = array1.erase(array1.begin());
    ck_assert(it == array1.begin());
    ck_assert(array1.size() == 5);
    ck_assert(array1.front() == w2);

    ck_assert(std::equal(array1.begin(), array1.end(), &result_[1]));

    it = array1.erase(array1.begin() + 1);
    ck_assert(*it == w4);
    ck_assert(array1.size() == 4);
  
    it = array1.erase(array1.end() - 1);
    ck_assert(it == array1.end());
    ck_assert(array1.size() == 3);
    ck_assert(array1.back() == w5);

    it = array1.erase(array1.begin() + 1, array1.end() - 1);
    ck_assert(it == array1.end() - 1);
    ck_assert(array1.size() == 2);
    ck_assert(array1.front() == w2);
    ck_assert(array1.back() == w5);
  }
}
END_TEST

START_TEST(eina_cxx_ptrarray_range)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  {
    efl::eina::ptr_array<int> array;
    array.push_back(new int(5));
    array.push_back(new int(10));
    array.push_back(new int(15));
    array.push_back(new int(20));
    array.push_back(new int(25));
    array.push_back(new int(30));

    efl::eina::range_ptr_array<int> range_array(array);
  
    ck_assert(range_array.size() == 6u);

    int result[] = {5, 10, 15, 20, 25, 30};
    int rresult[] = {30, 25, 20, 15, 10, 5};
    ck_assert(std::equal(range_array.begin(), range_array.end(), result));
    ck_assert(std::equal(range_array.rbegin(), range_array.rend(), rresult));

    efl::eina::range_ptr_array<int const> const_range_array(array);

    ck_assert(const_range_array.size() == 6u);
    ck_assert(std::equal(range_array.begin(), range_array.end(), result));
    ck_assert(std::equal(range_array.rbegin(), range_array.rend(), rresult));

    *range_array.begin() = 0;
    ck_assert(*const_range_array.begin() == 0);
    ck_assert(*array.begin() == 0);
  }

  {
    wrapper const w1(eo_add(MY_CLASS, NULL));
    wrapper const w2(eo_add(MY_CLASS, NULL));
    wrapper const w3(eo_add(MY_CLASS, NULL));
    wrapper const w4(eo_add(MY_CLASS, NULL));
    wrapper const w5(eo_add(MY_CLASS, NULL));
    wrapper const w6(eo_add(MY_CLASS, NULL));

    efl::eina::array<wrapper> array;
    array.push_back(w1);
    array.push_back(w2);
    array.push_back(w3);
    array.push_back(w4);
    array.push_back(w5);
    array.push_back(w6);

  //   efl::eina::range_array<wrapper> range_array(array);
  
  //   ck_assert(range_array.size() == 6u);

  //   wrapper result[] = {5, 10, 15, 20, 25, 30};
  //   wrapper rresult[] = {30, 25, 20, 15, 10, 5};
  //   ck_assert(std::equal(range_array.begin(), range_array.end(), result));
  //   ck_assert(std::equal(range_array.rbegin(), range_array.rend(), rresult));

  //   efl::eina::range_array<wrapper const> const_range_array(array);

  //   ck_assert(const_range_array.size() == 6u);
  //   ck_assert(std::equal(range_array.begin(), range_array.end(), result));
  //   ck_assert(std::equal(range_array.rbegin(), range_array.rend(), rresult));

  //   *range_array.begin() = 0;
  //   ck_assert(*const_range_array.begin() == 0);
  //   ck_assert(*array.begin() == 0);
  }
}
END_TEST

void
eina_test_ptrarray(TCase* tc)
{
  tcase_add_test(tc, eina_cxx_ptrarray_push_back);
  tcase_add_test(tc, eina_cxx_ptrarray_pop_back);
  tcase_add_test(tc, eina_cxx_ptrarray_insert);
  tcase_add_test(tc, eina_cxx_ptrarray_constructors);
  tcase_add_test(tc, eina_cxx_ptrarray_erase);
  tcase_add_test(tc, eina_cxx_ptrarray_range);
}
