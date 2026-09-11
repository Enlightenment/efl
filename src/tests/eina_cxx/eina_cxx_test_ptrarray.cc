/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.hh>
#include <eina_array.hh>
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

EFL_START_TEST(eina_cxx_ptrarray_push_back)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  int result[] = {5, 10, 15};
  int rresult[] = {15, 10, 5};

  wrapper const w1(efl_new(EINA_SIMPLE_CLASS));
  wrapper const w2(efl_new(EINA_SIMPLE_CLASS));
  wrapper const w3(efl_new(EINA_SIMPLE_CLASS));

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
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrarray_pop_back)
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
    wrapper const w1(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w2(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w3(efl_new(EINA_SIMPLE_CLASS));

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
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrarray_insert)
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
    wrapper const w0(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w1(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w2(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w3(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w4(efl_new(EINA_SIMPLE_CLASS));

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
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrarray_constructors)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  wrapper const w1(efl_new(EINA_SIMPLE_CLASS));

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
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrarray_erase)
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
    wrapper const w1(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w2(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w3(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w4(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w5(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w6(efl_new(EINA_SIMPLE_CLASS));

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
EFL_END_TEST

EFL_START_TEST(eina_cxx_ptrarray_range)
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
    wrapper const w1(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w2(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w3(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w4(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w5(efl_new(EINA_SIMPLE_CLASS));
    wrapper const w6(efl_new(EINA_SIMPLE_CLASS));

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
EFL_END_TEST

EFL_START_TEST(eina_cxx_array_of_strings)
{
  efl::eina::eina_init eina_init;

  // As it arrives from C: the array holds each string, not a pointer to
  // anything the range could dereference.
  ::Eina_Array* native = ::eina_array_new(4);
  ::eina_array_push(native, ::strdup("first"));
  ::eina_array_push(native, ::strdup("second"));
  ::eina_array_push(native, ::strdup("third"));

  {
    efl::eina::range_array< ::efl::eina::string_view> range(native);

    ck_assert(!range.empty());
    ck_assert(range.size() == 3u);

    ck_assert(range[0] == "first");
    ck_assert(range[1] == "second");
    ck_assert(range[2] == "third");
    ck_assert(range.front() == "first");
    ck_assert(range.back() == "third");

    char const* expected[] = {"first", "second", "third"};
    ck_assert(std::equal(range.begin(), range.end(), expected));

    char const* reversed[] = {"third", "second", "first"};
    ck_assert(std::equal(range.rbegin(), range.rend(), reversed));

    std::size_t walked = 0u;
    for (::efl::eina::string_view s : range)
      {
         ck_assert(s == expected[walked]);
         ++walked;
      }
    ck_assert(walked == 3u);
  }

  {
    unsigned int i;
    ::Eina_Array_Iterator it;
    void* item;
    EINA_ARRAY_ITER_NEXT(native, i, item, it)
      ::free(item);
    ::eina_array_free(native);
  }

  // And the other direction, which owns what it built: the brace list goes out
  // and reads back as itself.
  {
    efl::eina::range_array< ::efl::eina::string_view> built{"*.h", "*.hh"};

    ck_assert(built.size() == 2u);
    ck_assert(built[0] == "*.h");
    ck_assert(built[1] == "*.hh");

    efl::eina::range_array< ::efl::eina::string_view> shared = built;
    ck_assert(shared.size() == 2u);
    ck_assert(shared[1] == "*.hh");
  }

  // An empty array is empty rather than undefined.
  {
    ::Eina_Array* none = ::eina_array_new(1);
    efl::eina::range_array< ::efl::eina::string_view> range(none);
    ck_assert(range.empty());
    ck_assert(range.size() == 0u);
    ck_assert(range.begin() == range.end());
    ::eina_array_free(none);
  }
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_owning_array_of_strings)
{
  efl::eina::eina_init eina_init;

  typedef efl::eina::array< ::efl::eina::string_view> string_array;

  // Taking over an array built the way C builds one.
  {
    ::Eina_Array* native = ::eina_array_new(2);
    ::eina_array_push(native, ::strdup("hello"));
    ::eina_array_push(native, ::strdup("there"));

    string_array owned(native);

    ck_assert(owned.size() == 2u);
    ck_assert(owned[0] == "hello");
    ck_assert(owned[1] == "there");
    ck_assert(owned.front() == "hello");
    ck_assert(owned.back() == "there");
    // and it frees both strings and the array on the way out
  }

  // Building one, and reading it back.
  {
    string_array built;
    ck_assert(built.empty());

    built.push_back("one");
    built.push_back("two");
    built.push_back("three");

    ck_assert(built.size() == 3u);

    char const* expected[] = {"one", "two", "three"};
    ck_assert(std::equal(built.begin(), built.end(), expected));

    char const* reversed[] = {"three", "two", "one"};
    ck_assert(std::equal(built.rbegin(), built.rend(), reversed));

    built.pop_back();
    ck_assert(built.size() == 2u);
    ck_assert(built.back() == "two");

    built.clear();
    ck_assert(built.empty());
  }

  // A copy owns its own strings: what it holds has to survive the original.
  {
    string_array original{"alpha", "beta"};
    string_array copy = original;

    ck_assert(copy.size() == 2u);
    ck_assert(copy == original);

    original.clear();

    ck_assert(original.empty());
    ck_assert(copy.size() == 2u);
    ck_assert(copy[0] == "alpha");
    ck_assert(copy[1] == "beta");
  }

  // Moving hands the strings over rather than copying them.
  {
    string_array source{"x", "y"};
    string_array moved = std::move(source);

    ck_assert(moved.size() == 2u);
    ck_assert(moved[1] == "y");
  }

  // Inserting and erasing, each freeing exactly what it drops.
  {
    string_array a{"first", "third"};

    a.insert(a.begin() + 1, ::efl::eina::string_view("second"));
    ck_assert(a.size() == 3u);
    ck_assert(a[0] == "first");
    ck_assert(a[1] == "second");
    ck_assert(a[2] == "third");

    a.erase(a.begin());
    ck_assert(a.size() == 2u);
    ck_assert(a[0] == "second");

    a.erase(a.begin(), a.end());
    ck_assert(a.empty());
  }

  // Handing the array back out: this one keeps nothing.
  {
    string_array a{"kept"};
    ::Eina_Array* released = a.release_native_handle();

    ck_assert(a.empty());
    ck_assert(::eina_array_count(released) == 1u);

    ::free(::eina_array_data_get(released, 0));
    ::eina_array_free(released);
  }
}
EFL_END_TEST

void
eina_test_ptrarray(TCase* tc)
{
  tcase_add_test(tc, eina_cxx_array_of_strings);
  tcase_add_test(tc, eina_cxx_owning_array_of_strings);
  tcase_add_test(tc, eina_cxx_ptrarray_push_back);
  tcase_add_test(tc, eina_cxx_ptrarray_pop_back);
  tcase_add_test(tc, eina_cxx_ptrarray_insert);
  tcase_add_test(tc, eina_cxx_ptrarray_constructors);
  tcase_add_test(tc, eina_cxx_ptrarray_erase);
  tcase_add_test(tc, eina_cxx_ptrarray_range);
}
