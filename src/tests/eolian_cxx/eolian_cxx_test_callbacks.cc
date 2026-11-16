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

#include <Eo.hh>

#define EOLIANCXXTEST_API
#define EOLIANCXXTEST_API_WEAK

#include <fnptr.eo.hh>
#include <fnptr.eo.impl.hh>

#include <string>
#include <vector>

#include "eolian_cxx_suite.h"

/* A callback returning a string converts what the lambda returns, which
 * takes the C type, the C++ type and the ownership. */
EFL_START_TEST(eolian_cxx_test_callbacks_return_string)
{
  efl::eo::eo_init init;

  nonamespace::Fnptr obj(efl::eo::instantiate);

  std::string r = obj.text([](int x) { return x ? "some" : "none"; }, 7);

  ck_assert_str_eq(r.c_str(), "some");
}
EFL_END_TEST

/* A method taking two function pointers: each gets a template parameter of
 * its own, so two callables of different types can be given. */
EFL_START_TEST(eolian_cxx_test_callbacks_two_types)
{
  efl::eo::eo_init init;

  nonamespace::Fnptr obj(efl::eo::instantiate);
  int offset = 100;
  int calls = 0;

  int r = obj.apply([&calls](int x) { ++calls; return x + 1; },
                    [&calls, offset](int x) { ++calls; return x * 2 + offset; },
                    10);

  ck_assert_int_eq(calls, 2);
  ck_assert_int_eq(r, 11 + 120);
}
EFL_END_TEST

/* What C hands a callback reaches it converted the way a return value is: an
 * object is a wrapper holding a reference of its own, a string is a string. */
EFL_START_TEST(eolian_cxx_test_callbacks_arguments)
{
  efl::eo::eo_init init;

  nonamespace::Fnptr obj(efl::eo::instantiate);
  Eo* raw = obj._eo_ptr();
  int refs_before = efl_ref_count(raw);
  int refs_inside = 0;
  bool same = false;
  std::string seen;

  int r = obj.visit([&](nonamespace::Fnptr o, std::string text, int n)
                    {
                       same = (o._eo_ptr() == raw);
                       refs_inside = efl_ref_count(raw);
                       seen = text;
                       return n * 3;
                    });

  ck_assert_int_eq(r, 21);
  ck_assert(same);
  ck_assert_str_eq(seen.c_str(), "hello");
  ck_assert_int_eq(refs_inside, refs_before + 1);
  ck_assert_int_eq(efl_ref_count(raw), refs_before);

  /* The converted arguments are temporaries: a const reference binds. */
  r = obj.visit([&](nonamespace::Fnptr const& o, std::string const& text, int n)
                {
                   same = (o._eo_ptr() == raw);
                   seen = text;
                   return n + 1;
                });
  ck_assert_int_eq(r, 8);
  ck_assert(same);
  ck_assert_str_eq(seen.c_str(), "hello");
  ck_assert_int_eq(efl_ref_count(raw), refs_before);
}
EFL_END_TEST

/* Writes over the stack below the caller: a view on a dead frame would read
 * what this leaves there. */
static void __attribute__((noinline))
_clobber_stack()
{
  volatile unsigned char junk[2048];
  for (unsigned i = 0; i < sizeof(junk); ++i)
    junk[i] = 0x55;
}

/* A callback shaped like Efl.Ui.Format_Func: the buffer it gets is C's, so
 * what it appends is seen by C, and the value is a view on C's own value, not
 * on a copy that is gone when the conversion returns. */
EFL_START_TEST(eolian_cxx_test_callbacks_buffer_and_value)
{
  efl::eo::eo_init init;

  nonamespace::Fnptr obj(efl::eo::instantiate);
  int seen = -1;

  int r = obj.format([&](efl::eina::strbuf& str, efl::eina::value_view const& v)
                     {
                        _clobber_stack();
                        seen = efl::eina::get<int>(v);
                        str.append("value ");
                        str.append(std::to_string(seen));
                        return true;
                     });
  ck_assert_int_eq(seen, 42);
  ck_assert_int_eq(r, 1);

  /* The view C++ gets of a value C passes by value is on that value itself,
   * not on a copy of it: the address says so whatever the stack holds. */
  Eina_Value value;
  eina_value_setup(&value, EINA_VALUE_TYPE_INT);
  eina_value_set(&value, 7);
  Eina_Value const& cvalue = value;
  auto view = efl::eolian::convert_to_return<
    efl::eolian::return_traits<efl::eina::value_view const>::type>(cvalue);
  ck_assert(view.native_handle() == &value);
  eina_value_flush(&value);

  /* The same, with the buffer taken by a lambda that gives a verdict. */
  r = obj.format([](efl::eina::strbuf&, efl::eina::value_view const&) { return false; });
  ck_assert_int_eq(r, -1);
}
EFL_END_TEST

/* An iterator lent to C, not handed over: C reads from the element the C++
 * iterator points to, the C++ iterator frees the native one, and it is used
 * up afterwards, as an input iterator read to the end by someone else is. */
EFL_START_TEST(eolian_cxx_test_callbacks_lent_iterator)
{
  efl::eo::eo_init init;

  nonamespace::Fnptr obj(efl::eo::instantiate);
  static int values[] = { 20, 100, 200 };
  Eina_List* list = nullptr;
  for (int& v : values)
    list = eina_list_append(list, &v);

  {
    efl::eina::iterator<int> it(eina_list_iterator_new(list));
    ck_assert_int_eq(obj.sum_lent(it), 320);
    ck_assert(it == efl::eina::iterator<int>());
    /* Nothing left to lend: C gets NULL. */
    ck_assert_int_eq(obj.sum_lent(it), -1);
  }
  {
    efl::eina::iterator<int> it(eina_list_iterator_new(list));
    ++it;
    ck_assert_int_eq(obj.sum_lent(it), 300);
  }
  ck_assert_int_eq(obj.sum_lent(efl::eina::iterator<int>()), -1);
  eina_list_free(list);
}
EFL_END_TEST

static int _callbacks_log_errors = 0;

static void
_callbacks_count_errors(const Eina_Log_Domain *d EINA_UNUSED, Eina_Log_Level level,
                        const char *file EINA_UNUSED, const char *fnc EINA_UNUSED,
                        int line EINA_UNUSED, const char *fmt EINA_UNUSED,
                        void *data EINA_UNUSED, va_list args EINA_UNUSED)
{
  if (level <= EINA_LOG_LEVEL_ERR) _callbacks_log_errors++;
}

/* A callback can be given its object while that object is being destroyed.
 * Eo frees it at the end of its destruction whatever references were taken
 * during it, so the wrapper the callback gets cannot hold one: a copy the
 * callback keeps outlives the object, and copying and dropping it afterwards
 * must not touch Eo, which would log that the id is not a valid object.
 * Both an object without a parent, released by its last wrapper, and one
 * deleted from its parent are covered, with the object reaching C++ through
 * a callback argument or as the emitter of its own del event. */
static void
_visit_while_destroyed(bool parented, bool through_event)
{
  int calls = 0;
  bool same = false;
  Eo* raw = nullptr;
  efl::Object kept;
  nonamespace::Fnptr parent(efl::eo::instantiate);
  eina_log_print_cb_set(_callbacks_count_errors, nullptr);
  _callbacks_log_errors = 0;
  {
    nonamespace::Fnptr obj = parented
      ? nonamespace::Fnptr(efl::eo::instantiate, parent)
      : nonamespace::Fnptr(efl::eo::instantiate);
    raw = obj._eo_ptr();
    efl_wref_add(raw, &raw);
    if (through_event)
      efl::eolian::event_add(efl::Object::del_event, obj, [&](efl::Object o)
                             {
                                efl::Object copy = o;
                                same = (copy._eo_ptr() == o._eo_ptr());
                                kept = o;
                                ++calls;
                             });
    else
      obj.visit_on_destroy([&](nonamespace::Fnptr o, std::string text, int n)
                           {
                              nonamespace::Fnptr copy = o;
                              same = (copy._eo_ptr() == o._eo_ptr());
                              kept = o;
                              ++calls;
                              ck_assert_str_eq(text.c_str(), "bye");
                              return n;
                           });
    if (parented) ::efl_del(obj._eo_ptr());
  }
  ck_assert_ptr_eq(raw, NULL);
  {
    efl::Object again = kept;
    ck_assert_ptr_eq(again._eo_ptr(), kept._eo_ptr());
  }
  kept = efl::Object();
  eina_log_print_cb_set(eina_log_print_cb_stderr, nullptr);
  ck_assert_int_eq(calls, 1);
  ck_assert(same);
  ck_assert_int_eq(_callbacks_log_errors, 0);
}

EFL_START_TEST(eolian_cxx_test_callbacks_object_being_destroyed)
{
  efl::eo::eo_init init;

  _visit_while_destroyed(false, false);
  _visit_while_destroyed(true, false);
  _visit_while_destroyed(false, true);
  _visit_while_destroyed(true, true);
}
EFL_END_TEST

/* A user reference count of 0 is not enough to tell an object is being
 * destroyed. A NOREF handler sees 0 on an object it may still keep, and so
 * does a handler running after another one dropped the last owner, while the
 * emission keeps the object alive: a wrapper kept from there holds a
 * reference and the object lives on. */
EFL_START_TEST(eolian_cxx_test_callbacks_kept_alive_at_no_reference)
{
  efl::eo::eo_init init;

  Eo* raw = nullptr;
  std::vector<efl::Object> cache;
  eina_log_print_cb_set(_callbacks_count_errors, nullptr);
  _callbacks_log_errors = 0;
  {
    nonamespace::Fnptr obj(efl::eo::instantiate);
    raw = obj._eo_ptr();
    efl_wref_add(raw, &raw);
    /* Kept once: dropping it from the cache sends NOREF again. */
    efl::eolian::event_add(efl::Object::noref_event, obj,
                           [&](efl::Object o)
                           {
                              if (cache.empty())
                                cache.push_back(o);
                           });
  }
  ck_assert_ptr_ne(raw, NULL);
  ck_assert_int_eq(cache.size(), 1);
  cache.clear();
  ck_assert_ptr_eq(raw, NULL);

  efl::Object kept;
  {
    nonamespace::Fnptr* owner = new nonamespace::Fnptr(efl::eo::instantiate);
    raw = owner->_eo_ptr();
    efl_wref_add(raw, &raw);
    /* Callbacks of one priority run last added first. */
    efl::eolian::event_add(efl::Object::destruct_event, *owner,
                           [&](efl::Object o) { kept = o; });
    efl::eolian::event_add(efl::Object::destruct_event, *owner,
                           [&](efl::Object)
                           {
                              delete owner;
                              owner = nullptr;
                           });
    efl_event_callback_call(raw, EFL_EVENT_DESTRUCT, NULL);
  }
  ck_assert_ptr_ne(raw, NULL);
  kept = efl::Object();
  ck_assert_ptr_eq(raw, NULL);
  eina_log_print_cb_set(eina_log_print_cb_stderr, nullptr);
  ck_assert_int_eq(_callbacks_log_errors, 0);
}
EFL_END_TEST

/* NULL is no object being destroyed: wrapping it, as an object return does
 * when there is none, is quiet and can be done again and again. */
EFL_START_TEST(eolian_cxx_test_callbacks_null_wrapper_not_dying)
{
  efl::eo::eo_init init;

  nonamespace::Fnptr obj(efl::eo::instantiate);
  eina_log_print_cb_set(_callbacks_count_errors, nullptr);
  _callbacks_log_errors = 0;
  for (int i = 0; i < 3; ++i)
    {
       efl::Object p = obj.parent_get();
       ck_assert(!p);
    }
  eina_log_print_cb_set(eina_log_print_cb_stderr, nullptr);
  ck_assert_int_eq(_callbacks_log_errors, 0);
}
EFL_END_TEST

void
eolian_cxx_test_callbacks(TCase* tc)
{
  tcase_add_test(tc, eolian_cxx_test_callbacks_return_string);
  tcase_add_test(tc, eolian_cxx_test_callbacks_kept_alive_at_no_reference);
  tcase_add_test(tc, eolian_cxx_test_callbacks_null_wrapper_not_dying);
  tcase_add_test(tc, eolian_cxx_test_callbacks_object_being_destroyed);
  tcase_add_test(tc, eolian_cxx_test_callbacks_lent_iterator);
  tcase_add_test(tc, eolian_cxx_test_callbacks_arguments);
  tcase_add_test(tc, eolian_cxx_test_callbacks_buffer_and_value);
  tcase_add_test(tc, eolian_cxx_test_callbacks_two_types);
}
