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
#include <Eo.hh>

#include <algorithm>

#include "eina_cxx_suite.h"
#include "eina_simple.eo.hh"

EFL_START_TEST(eina_cxx_iterator_equal)
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
EFL_END_TEST

EFL_START_TEST(eina_cxx_eo_iterator_equal)
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  efl::eina::list<nonamespace::Eina_Simple> list;

  nonamespace::Eina_Simple const w1(efl::eo::instantiate);
  nonamespace::Eina_Simple const w2(efl::eo::instantiate);
  nonamespace::Eina_Simple const w3(efl::eo::instantiate);
  nonamespace::Eina_Simple const w4(efl::eo::instantiate);

  list.push_back(w1);
  list.push_back(w2);
  list.push_back(w3);
  list.push_back(w4);

  efl::eina::iterator<nonamespace::Eina_Simple> iterator = list.ibegin()
    , last_iterator = list.iend();

  nonamespace::Eina_Simple const result[] = {w1, w2, w3, w4};

  ck_assert(std::equal(iterator, last_iterator, result));

  /* Post-increment moves on like pre-increment. */
  efl::eina::iterator<nonamespace::Eina_Simple> it = list.ibegin();
  it++;
  ck_assert((*it)._eo_ptr() == w2._eo_ptr());
}
EFL_END_TEST

static Eina_Bool
_keep_all(const void* /*container*/, void* /*data*/, void* /*fdata*/)
{
  return EINA_TRUE;
}

static void
_count_free(void* fdata)
{
  ++*static_cast<int*>(fdata);
}

/* A native iterator over list that adds one to *freed when it is freed. */
static Eina_Iterator*
_counted(Eina_List* list, int* freed)
{
  return eina_iterator_filter_new(eina_list_iterator_new(list), _keep_all, _count_free, freed);
}

/* Assigning over an iterator frees what it held, and an iterator assigned
 * to itself keeps its handle. */
EFL_START_TEST(eina_cxx_iterator_assign)
{
  efl::eina::eina_init eina_init;

  int values[] = {5, 10, 15};
  Eina_List* list = nullptr;
  for (int& v : values)
    list = eina_list_append(list, &v);

  int freed_a = 0, freed_b = 0;
  {
    efl::eina::iterator<int> a(_counted(list, &freed_a));
    efl::eina::iterator<int> b(_counted(list, &freed_b));
    a = b;
    ck_assert_int_eq(freed_a, 1);
    ck_assert_int_eq(freed_b, 0);
    ck_assert_int_eq(*a, 5);
    ck_assert(b == efl::eina::iterator<int>());

    efl::eina::iterator<int>& self = a;
    a = self;
    ck_assert(a != efl::eina::iterator<int>());
    ck_assert_int_eq(*a, 5);
    ck_assert_int_eq(freed_b, 0);
  }
  ck_assert_int_eq(freed_a, 1);
  ck_assert_int_eq(freed_b, 1);
  eina_list_free(list);
}
EFL_END_TEST

void
eina_test_iterator(TCase *tc)
{
  tcase_add_test(tc, eina_cxx_iterator_assign);
  tcase_add_test(tc, eina_cxx_iterator_equal);
  tcase_add_test(tc, eina_cxx_eo_iterator_equal);
}
