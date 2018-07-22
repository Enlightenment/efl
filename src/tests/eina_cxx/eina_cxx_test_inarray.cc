#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <functional>
#include <iostream>

#include <Eina.hh>

#include "eina_cxx_suite.h"

EFL_START_TEST(eina_cxx_inarray_pod_push_back)
{
  efl::eina::eina_init eina_init;

  efl::eina::inarray<int> array;

  array.push_back(5);
  std::cout << "array size: " << array.size() << std::endl;
  array.push_back(10);
  std::cout << "array size: " << array.size() << std::endl;
  array.push_back(15);

  int result[] = {5, 10, 15};
  int rresult[] = {15, 10, 5};

  ck_assert(array.size() == 3);
  ck_assert(std::equal(array.begin(), array.end(), result));
  ck_assert(std::equal(array.rbegin(), array.rend(), rresult));
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_inarray_pod_pop_back)
{
  efl::eina::eina_init eina_init;

  efl::eina::inarray<int> array;

  array.push_back(5);
  array.push_back(10);
  array.push_back(15);
  array.pop_back();

  int result[] = {5, 10};
  int rresult[] = {10, 5};

  ck_assert(array.size() == 2);
  ck_assert(std::equal(array.begin(), array.end(), result));
  ck_assert(std::equal(array.rbegin(), array.rend(), rresult));
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_inarray_pod_insert)
{
  efl::eina::eina_init eina_init;

  efl::eina::inarray<int> array;

  efl::eina::inarray<int>::iterator it;

  it = array.insert(array.end(), 5); // first element
  ck_assert(it != array.end());
  ++it;
  ck_assert(it == array.end());

  it = array.insert(array.end(), 10);  // equivalent to push_back
  ck_assert(it != array.end());
  ++it;
  ck_assert(it == array.end());

  it = array.insert(array.begin(), 15); // equivalent to push_front
  ck_assert(it == array.begin());

  it = array.end();
  --it;
  array.insert(it, 20); // insert before the last element

  int result[] = {15, 5, 20, 10};
  int rresult[] = {10, 20, 5, 15};

  ck_assert(array.size() == 4);
  ck_assert(std::equal(array.begin(), array.end(), result));
  ck_assert(std::equal(array.rbegin(), array.rend(), rresult));

  efl::eina::inarray<int> array2;
  it = array2.insert(array2.end(), array.begin(), array.end());
  ck_assert(it == array2.begin());
  ck_assert(array == array2);

  efl::eina::inarray<int> array3;
  array3.push_back(1);
  it = array3.insert(array3.end(), array.begin(), array.end());
  ck_assert(array3.size() == 5);
  ck_assert(array3.front() == 1);
  it = array3.begin();
  ++it;
  ck_assert(std::equal(it, array3.end(), array.begin()));

  efl::eina::inarray<int> array4;
  array4.push_back(1);
  it = array4.insert(array4.begin(), array.begin(), array.end());
  ck_assert(array4.size() == 5);
  ck_assert(array4.back() == 1);
  ck_assert(std::equal(array.begin(), array.end(), array4.begin()));
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_inarray_pod_constructors)
{
  efl::eina::eina_init eina_init;

  efl::eina::inarray<int> array1;
  ck_assert(array1.empty());

  efl::eina::inarray<int> array2(10, 5);
  ck_assert(array2.size() == 10);
  ck_assert(std::find_if(array2.begin(), array2.end()
                      , std::not1(std::bind1st(std::equal_to<int>(), 5))) == array2.end());

  efl::eina::inarray<int> array3(array2);
  ck_assert(array2 == array3);

  efl::eina::inarray<int> array4(array2.begin(), array2.end());
  ck_assert(array2 == array4);
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_inarray_pod_erase)
{
  efl::eina::eina_init eina_init;

  efl::eina::inarray<int> array1;
  array1.push_back(5);
  array1.push_back(10);
  array1.push_back(15);
  array1.push_back(20);
  array1.push_back(25);
  array1.push_back(30);

  efl::eina::inarray<int>::iterator it = array1.begin(), it2;

  it = array1.erase(it);
  ck_assert(it == array1.begin());
  ck_assert(array1.size() == 5);
  ck_assert(array1.front() == 10);

  it = array1.begin() + 1;
  ck_assert(*it == 15);
  it = array1.erase(it);
  ck_assert(*it == 20);
  ck_assert(array1.size() == 4);

  it = array1.end() - 1;
  it = array1.erase(it);
  ck_assert(it == array1.end());
  ck_assert(array1.size() == 3);
  ck_assert(array1.back() == 25);

  it = array1.begin() + 1;
  it2 = array1.end() - 1;
  it = array1.erase(it, it2);
  it2 = array1.end() -1;
  ck_assert(it == it2);
  ck_assert(array1.size() == 2);
  ck_assert(array1.front() == 10);
  ck_assert(array1.back() == 25);
}
EFL_END_TEST

unsigned int constructors_called = 0u;
unsigned int destructors_called = 0u;

struct non_pod
{
  non_pod(int x_)
    : x(new int(x_))
  {
    ++::constructors_called;
  }
  ~non_pod()
  {
    ++::destructors_called;
    delete x;
  }
  non_pod(non_pod const& other)
  {
    ++::constructors_called;
    x = new int(*other.x);
  }
  non_pod& operator=(non_pod const& other)
  {
    delete x;
    x = new int(*other.x);
    return *this;
  }

  int* x;
};

bool operator==(non_pod lhs, non_pod rhs)
{
  return *lhs.x == *rhs.x;
}

EFL_START_TEST(eina_cxx_inarray_nonpod_push_back)
{
  efl::eina::eina_init eina_init;
  {
    efl::eina::inarray<non_pod> array;

    array.push_back(5);
    array.push_back(10);
    array.push_back(15);

    int result[] = {5, 10, 15};
    int rresult[] = {15, 10, 5};

    ck_assert(array.size() == 3);
    ck_assert(std::equal(array.begin(), array.end(), result));
    ck_assert(std::equal(array.rbegin(), array.rend(), rresult));
  }
  std::cout << "constructors called " << ::constructors_called
            << "\ndestructors called " << ::destructors_called << std::endl;
  ck_assert(::constructors_called == ::destructors_called);
  ::constructors_called = ::destructors_called = 0;
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_inarray_nonpod_pop_back)
{
  {
    efl::eina::eina_init eina_init;

    efl::eina::inarray<non_pod> array;

    array.push_back(5);
    array.push_back(10);
    array.push_back(15);
    array.pop_back();

    int result[] = {5, 10};
    int rresult[] = {10, 5};

    ck_assert(array.size() == 2);
    ck_assert(std::equal(array.begin(), array.end(), result));
    ck_assert(std::equal(array.rbegin(), array.rend(), rresult));
  }
  std::cout << "constructors called " << ::constructors_called
            << "\ndestructors called " << ::destructors_called << std::endl;
  ck_assert(::constructors_called == ::destructors_called);
  ::constructors_called = ::destructors_called = 0;
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_inarray_nonpod_insert)
{
  {
    efl::eina::eina_init eina_init;

    efl::eina::inarray<non_pod> array;

    efl::eina::inarray<non_pod>::iterator it;

    it = array.insert(array.end(), 5); // first element
    ck_assert(it != array.end());
    ++it;
    ck_assert(it == array.end());

    it = array.insert(array.end(), 10);  // equivalent to push_back
    ck_assert(it != array.end());
    ++it;
    ck_assert(it == array.end());

    it = array.insert(array.begin(), 15); // equivalent to push_front
    ck_assert(it == array.begin());

    it = array.end();
    --it;
    array.insert(it, 20); // insert before the last element

    int result[] = {15, 5, 20, 10};
    int rresult[] = {10, 20, 5, 15};

    ck_assert(array.size() == 4);
    ck_assert(std::equal(array.begin(), array.end(), result));
    ck_assert(std::equal(array.rbegin(), array.rend(), rresult));

    efl::eina::inarray<non_pod> array2;
    it = array2.insert(array2.end(), array.begin(), array.end());
    ck_assert(it == array2.begin());
    ck_assert(array == array2);

    efl::eina::inarray<non_pod> array3;
    array3.push_back(1);
    it = array3.insert(array3.end(), array.begin(), array.end());
    ck_assert(array3.size() == 5);
    ck_assert(array3.front() == 1);
    it = array3.begin();
    ++it;
    ck_assert(std::equal(it, array3.end(), array.begin()));

    efl::eina::inarray<non_pod> array4;
    array4.push_back(1);
    it = array4.insert(array4.begin(), array.begin(), array.end());
    ck_assert(array4.size() == 5);
    ck_assert(array4.back() == 1);
    ck_assert(std::equal(array.begin(), array.end(), array4.begin()));
  }
  std::cout << "constructors called " << ::constructors_called
            << "\ndestructors called " << ::destructors_called << std::endl;
  ck_assert(::constructors_called == ::destructors_called);
  ::constructors_called = ::destructors_called = 0;
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_inarray_nonpod_constructors)
{
  {
    efl::eina::eina_init eina_init;

    efl::eina::inarray<non_pod> array1;
    ck_assert(array1.empty());

    efl::eina::inarray<non_pod> array2(10, 5);
    ck_assert(array2.size() == 10);
    ck_assert(std::find_if(array2.begin(), array2.end()
                        , std::not1(std::bind1st(std::equal_to<non_pod>(), 5))) == array2.end());

    efl::eina::inarray<non_pod> array3(array2);
    ck_assert(array2 == array3);

    efl::eina::inarray<non_pod> array4(array2.begin(), array2.end());
    ck_assert(array2 == array4);
  }
  std::cout << "constructors called " << ::constructors_called
            << "\ndestructors called " << ::destructors_called << std::endl;
  ck_assert(::constructors_called == ::destructors_called);
  ::constructors_called = ::destructors_called = 0;
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_inarray_nonpod_erase)
{
  {
    efl::eina::eina_init eina_init;

    efl::eina::inarray<non_pod> array1;
    array1.push_back(5);
    array1.push_back(10);
    array1.push_back(15);
    array1.push_back(20);
    array1.push_back(25);
    array1.push_back(30);

    efl::eina::inarray<non_pod>::iterator it = array1.begin(), it2;

    it = array1.erase(it);
    ck_assert(it == array1.begin());
    ck_assert(array1.size() == 5);
    ck_assert(array1.front() == 10);

    it = array1.begin() + 1;
    ck_assert(*it == 15);
    it = array1.erase(it);
    ck_assert(*it == 20);
    ck_assert(array1.size() == 4);

    it = array1.end() - 1;
    it = array1.erase(it);
    ck_assert(it == array1.end());
    ck_assert(array1.size() == 3);
    ck_assert(array1.back() == 25);

    it = array1.begin() + 1;
    it2 = array1.end() - 1;
    it = array1.erase(it, it2);
    it2 = array1.end() -1;
    ck_assert(it == it2);
    ck_assert(array1.size() == 2);
    ck_assert(array1.front() == 10);
    ck_assert(array1.back() == 25);
  }
  std::cout << "constructors called " << ::constructors_called
            << "\ndestructors called " << ::destructors_called << std::endl;
  ck_assert(::constructors_called == ::destructors_called);
  ::constructors_called = ::destructors_called = 0;
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_range_inarray)
{
  efl::eina::eina_init eina_init;

  efl::eina::inarray<int> array;
  array.push_back(5);
  array.push_back(10);
  array.push_back(15);

  int result[] = {5, 10, 15};
  int rresult[] = {15, 10, 5};

  efl::eina::range_inarray<int> range_array(array);

  ck_assert(range_array.size() == 3);
  ck_assert(std::equal(range_array.begin(), range_array.end(), result));
  ck_assert(std::equal(range_array.rbegin(), range_array.rend(), rresult));

  ck_assert(range_array[0] == 5);

  *range_array.begin() = 0;

  int result1[] = {0, 10, 15};
  int rresult1[] = {15, 10, 0};

  ck_assert(range_array.size() == 3);
  ck_assert(std::equal(range_array.begin(), range_array.end(), result1));
  ck_assert(std::equal(range_array.rbegin(), range_array.rend(), rresult1));

  ck_assert(range_array[0] == 0);
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_inarray_from_c)
{
  efl::eina::eina_init eina_init;

  Eina_Inarray *c_array = nullptr;
  int values[] = { 11, 22, 33 };

  c_array = ::eina_inarray_new(sizeof(int), sizeof(values)/sizeof(int));
  ck_assert(!!c_array);

  eina_inarray_push(c_array, &values[0]);
  eina_inarray_push(c_array, &values[1]);
  eina_inarray_push(c_array, &values[2]);
  {
      efl::eina::range_inarray<int> range_array(c_array);
  }
  ck_assert(eina_inarray_count(c_array) == 3);
  efl::eina::inarray<int> array(c_array);
}
EFL_END_TEST

void
eina_test_inarray(TCase *tc)
{
  tcase_add_test(tc, eina_cxx_inarray_pod_push_back);
  tcase_add_test(tc, eina_cxx_inarray_pod_pop_back);
  tcase_add_test(tc, eina_cxx_inarray_pod_insert);
  tcase_add_test(tc, eina_cxx_inarray_pod_erase);
  tcase_add_test(tc, eina_cxx_inarray_pod_constructors);
  tcase_add_test(tc, eina_cxx_inarray_nonpod_push_back);
  tcase_add_test(tc, eina_cxx_inarray_nonpod_pop_back);
  tcase_add_test(tc, eina_cxx_inarray_nonpod_insert);
  tcase_add_test(tc, eina_cxx_inarray_nonpod_erase);
  tcase_add_test(tc, eina_cxx_inarray_nonpod_constructors);
  tcase_add_test(tc, eina_cxx_range_inarray);
  tcase_add_test(tc, eina_cxx_inarray_from_c);
}
