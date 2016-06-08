/* EINA - EFL data type library
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Eina.h>

#include "eina_suite.h"

static void
_eina_test_promise_cb(void* data, void* value EINA_UNUSED)
{
   *(Eina_Bool*)data = EINA_TRUE;
}

static void
_eina_test_error_cb(void *data, Eina_Error error)
{
    *(int *)data = error;
}

static void _eina_test_promise_value_all_cb(void* data, void* value)
{
  Eina_Iterator* iterator = value;
  int *i, *j;
  Eina_Bool b;

  b = eina_iterator_next(iterator, (void**)&i);
  ck_assert(!!b);

  b = eina_iterator_next(iterator, (void**)&j);
  ck_assert(!!b);

  ck_assert(i != NULL);
  ck_assert(j != NULL);
  ck_assert(*i == 10);
  ck_assert(*j == 15);

  *(Eina_Bool*)data = EINA_TRUE;
}

static void cancel_callback(void* data, Eina_Promise_Owner* promise EINA_UNUSED)
{
   *(Eina_Bool*)data = EINA_TRUE;
}

static void _cancel_promise_callback(void* data EINA_UNUSED, Eina_Error error EINA_UNUSED)
{
   *(Eina_Bool*)data = EINA_TRUE;
}

static void
_cancel_then_callback(void *data, void *value EINA_UNUSED)
{
   *(int*)data = 1;
}

static void
_cancel_error_callback(void *data, Eina_Error error EINA_UNUSED)
{
   *(int*)data = -1;
}

void progress_callback(void* data, void* value)
{
   int* i = value;
   ck_assert(*i == 1);
   *(Eina_Bool*)data = EINA_TRUE;
}

static void progress_notify(void* data, Eina_Promise_Owner* promise EINA_UNUSED)
{
   ck_assert(!*(Eina_Bool*)data);
   *(Eina_Bool*)data = EINA_TRUE;
}

static void
_eina_promise_progress_notify_fulfilled(void* data, void* value EINA_UNUSED)
{
  *(Eina_Bool*)data = EINA_TRUE;
}

static void
_eina_promise_progress_notify_error(void* data EINA_UNUSED, Eina_Error error EINA_UNUSED)
{
  ck_assert(EINA_FALSE);
}

// pointers
START_TEST(eina_test_pointer_promise_normal_lifetime)
{
   Eina_Promise_Owner* promise_owner;
   Eina_Promise* promise;
   Eina_Bool ran = EINA_FALSE;

   eina_init();

   promise_owner = eina_promise_add();

   promise = eina_promise_owner_promise_get(promise_owner);

   eina_promise_then(promise, &_eina_test_promise_cb, NULL, &ran);
   eina_promise_owner_value_set(promise_owner, NULL, NULL);

   ck_assert(ran == EINA_TRUE);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_normal_lifetime_all)
{
   Eina_Promise_Owner* promise_owner;
   Eina_Promise* first[2] = {NULL, NULL};
   Eina_Promise* promise;
   Eina_Bool ran = EINA_FALSE;

   eina_init();

   promise_owner = eina_promise_add();
   first[0] = eina_promise_owner_promise_get(promise_owner);
   promise = eina_promise_all(eina_carray_iterator_new((void**)&first[0]));

   eina_promise_then(promise, &_eina_test_promise_cb, NULL, &ran);
   eina_promise_owner_value_set(promise_owner, NULL, NULL);

   ck_assert(ran == EINA_TRUE);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_error_set)
{
   Eina_Promise_Owner* promise_owner;
   Eina_Promise* promise;
   int ran = 0;
   int error = 0xdeadbeef;

   eina_init();

   promise_owner = eina_promise_add();

   promise = eina_promise_owner_promise_get(promise_owner);

   eina_promise_ref(promise);

   eina_promise_then(promise, NULL, &_eina_test_error_cb, &ran);

   eina_promise_owner_error_set(promise_owner, error);

   ck_assert_int_eq(error, ran);
   ck_assert_int_eq(error, eina_promise_error_get(promise));
   ck_assert(!eina_promise_pending_is(promise));
   ck_assert(!eina_promise_owner_cancelled_is(promise_owner));

   eina_promise_unref(promise);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_error_set_all)
{
   Eina_Promise_Owner* promise_owner;
   Eina_Promise* first[2] = {NULL, NULL};
   Eina_Promise* promise;
   int ran = 0;
   int error = 0xdeadbeef;

   eina_init();

   promise_owner = eina_promise_add(0);
   first[0] = eina_promise_owner_promise_get(promise_owner);
   promise = eina_promise_all(eina_carray_iterator_new((void**)&first[0]));

   eina_promise_ref(promise);

   eina_promise_then(promise, NULL, &_eina_test_error_cb, &ran);
   eina_promise_owner_error_set(promise_owner, error);

   ck_assert_int_eq(error, ran);
   ck_assert_int_eq(error, eina_promise_error_get(promise));
   ck_assert(!eina_promise_pending_is(promise));
   ck_assert(!eina_promise_owner_cancelled_is(promise_owner));

   eina_promise_unref(promise);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_immediate_set_lifetime)
{
   Eina_Promise_Owner* owner;
   Eina_Promise* promise;
   Eina_Bool ran = EINA_FALSE;

   eina_init();

   owner = eina_promise_add();
   promise = eina_promise_owner_promise_get(owner);

   eina_promise_owner_value_set(owner, NULL, NULL);
   eina_promise_then(promise, &_eina_test_promise_cb, NULL, &ran);

   ck_assert(ran == EINA_TRUE);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_immediate_set_lifetime_all)
{
   Eina_Promise_Owner* owner;
   Eina_Promise* first[2] = {NULL, NULL};
   Eina_Promise* promise;
   Eina_Bool ran = EINA_FALSE;

   eina_init();

   owner = eina_promise_add();
   first[0] = eina_promise_owner_promise_get(owner);
   promise = eina_promise_all(eina_carray_iterator_new((void**)&first[0]));

   eina_promise_owner_value_set(owner, NULL, NULL);
   eina_promise_then(promise, &_eina_test_promise_cb, NULL, &ran);

   ck_assert(ran == EINA_TRUE);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_manual_then)
{
   Eina_Promise_Owner* promise_owner;
   Eina_Promise* promise;
   Eina_Bool ran = EINA_FALSE;

   eina_init();

   promise_owner = eina_promise_add();

   eina_promise_owner_default_manual_then_set(promise_owner, EINA_TRUE);

   promise = eina_promise_owner_promise_get(promise_owner);

   eina_promise_then(promise, &_eina_test_promise_cb, NULL, &ran);

   eina_promise_owner_value_set(promise_owner, NULL, NULL);

   ck_assert(!ran);

   eina_promise_owner_default_call_then(promise_owner);

   ck_assert(ran);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_values_all)
{
   Eina_Promise_Owner* owners[2];
   Eina_Promise* promises[3] = {NULL, NULL, NULL};
   Eina_Promise* promise_all;
   Eina_Bool ran = EINA_FALSE;

   eina_init();

   int* i = malloc(sizeof(int)), *j = malloc(sizeof(int));
   *i = 10;
   *j = 15;

   owners[0] = eina_promise_add();
   owners[1] = eina_promise_add();
   
   promises[0] = eina_promise_owner_promise_get(owners[0]);
   promises[1] = eina_promise_owner_promise_get(owners[1]);

   
   promise_all = eina_promise_all(eina_carray_iterator_new((void**)&promises[0]));

   eina_promise_owner_value_set(owners[0], i, free);
   eina_promise_owner_value_set(owners[1], j, free);

   eina_promise_then(promise_all, &_eina_test_promise_value_all_cb, NULL, &ran);

   ck_assert(ran == EINA_TRUE);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_cancel_promise)
{
   Eina_Bool ran = EINA_FALSE, cancel_ran = EINA_FALSE;
   Eina_Promise_Owner* owner;
   Eina_Promise* promise;

   eina_init();

   owner = eina_promise_add();
   eina_promise_owner_default_cancel_cb_add(owner, &cancel_callback, &cancel_ran, NULL);

   promise = eina_promise_owner_promise_get(owner);

   eina_promise_then(promise, NULL, &_cancel_promise_callback, &ran);

   eina_promise_cancel(promise);

   ck_assert(cancel_ran && ran);
   ck_assert(eina_promise_owner_cancelled_is(owner));
   ck_assert(!eina_promise_pending_is(promise));
   ck_assert_int_eq(EINA_ERROR_PROMISE_CANCEL, eina_promise_error_get(promise));

   // Free the owner
   eina_promise_owner_value_set(owner, NULL, NULL);

   ck_assert(ran);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_cancel_finished_promise)
{
   Eina_Bool cancel_ran = EINA_FALSE;
   int ran = 0;
   Eina_Promise_Owner* owner;
   Eina_Promise* promise;

   eina_init();

   owner = eina_promise_add();
   eina_promise_owner_default_cancel_cb_add(owner, &cancel_callback, &cancel_ran, NULL);

   promise = eina_promise_owner_promise_get(owner);

   eina_promise_then(promise, &_cancel_then_callback, &_cancel_error_callback, &ran);

   eina_promise_ref(promise);
   eina_promise_owner_value_set(owner, NULL, NULL);

   ck_assert(!cancel_ran);
   ck_assert_int_eq(1, ran);
   ck_assert(!eina_promise_owner_cancelled_is(owner));
   ck_assert(!eina_promise_pending_is(promise));
   ck_assert_int_eq(0, eina_promise_error_get(promise));

   eina_promise_cancel(promise);

   // The conditions should not have been changed.
   ck_assert(!cancel_ran);
   ck_assert_int_eq(1, ran);
   ck_assert(!eina_promise_owner_cancelled_is(owner));
   ck_assert(!eina_promise_pending_is(promise));
   ck_assert_int_eq(0, eina_promise_error_get(promise));

   eina_promise_unref(promise);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_double_cancel_promise)
{
   Eina_Bool ran = EINA_FALSE, cancel_ran = EINA_FALSE;
   Eina_Promise_Owner* owner;
   Eina_Promise* promise;

   eina_init();

   owner = eina_promise_add();
   eina_promise_owner_default_cancel_cb_add(owner, &cancel_callback, &cancel_ran, NULL);

   promise = eina_promise_owner_promise_get(owner);

   eina_promise_then(promise, NULL, &_cancel_promise_callback, &ran);

   eina_promise_cancel(promise);

   ck_assert(cancel_ran && ran);
   ck_assert(eina_promise_owner_cancelled_is(owner));
   ck_assert(!eina_promise_pending_is(promise));
   ck_assert_int_eq(EINA_ERROR_PROMISE_CANCEL, eina_promise_error_get(promise));

   cancel_ran = EINA_FALSE;
   ran = EINA_FALSE;

   eina_promise_cancel(promise);

   ck_assert(!cancel_ran && !ran);
   ck_assert(eina_promise_owner_cancelled_is(owner));
   ck_assert(!eina_promise_pending_is(promise));

   // Finally free the owner
   eina_promise_owner_value_set(owner, NULL, NULL);

   eina_shutdown();
}
END_TEST 

START_TEST(eina_test_pointer_promise_progress)
{
   Eina_Bool progress_ran = EINA_FALSE;
   Eina_Promise_Owner* owner;
   Eina_Promise* promise;
   int i = 1;

   eina_init();

   owner = eina_promise_add();

   promise = eina_promise_owner_promise_get(owner);
   eina_promise_progress_cb_add(promise, &progress_callback, &progress_ran, NULL);

   eina_promise_owner_progress(owner, &i);

   ck_assert(progress_ran);

   eina_promise_unref(promise);
   eina_promise_owner_value_set(owner, NULL, NULL);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_progress_notify1)
{
   Eina_Bool progress_notify_ran = EINA_FALSE;
   Eina_Promise_Owner* owner;
   Eina_Promise* promise;

   eina_init();

   owner = eina_promise_add();
   eina_promise_owner_progress_notify(owner, &progress_notify, &progress_notify_ran, NULL);

   promise = eina_promise_owner_promise_get(owner);
   eina_promise_progress_cb_add(promise, &progress_callback, NULL, NULL); // never run
   eina_promise_progress_cb_add(promise, &progress_callback, NULL, NULL); // never run

   ck_assert(progress_notify_ran);

   eina_promise_unref(promise);
   eina_promise_owner_value_set(owner, NULL, NULL);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_progress_notify2)
{
   Eina_Bool progress_notify_ran = EINA_FALSE;
   Eina_Promise_Owner* owner;
   Eina_Promise* promise;

   eina_init();

   owner = eina_promise_add();
   eina_promise_owner_progress_notify(owner, &progress_notify, &progress_notify_ran, NULL);

   promise = eina_promise_owner_promise_get(owner);
   eina_promise_then(promise, NULL, &_cancel_promise_callback, NULL); // never run
   eina_promise_then(promise, NULL, &_cancel_promise_callback, NULL); // never run

   ck_assert(progress_notify_ran);

   eina_promise_owner_value_set(owner, NULL, NULL);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_progress_notify3)
{
   Eina_Bool progress_notify_ran = EINA_FALSE;
   Eina_Promise_Owner* owner;
   Eina_Promise* promise;
   Eina_Promise* promise_progress;

   eina_init();

   owner = eina_promise_add();
   promise_progress = eina_promise_progress_notification(owner);
   eina_promise_then(promise_progress, &_eina_promise_progress_notify_fulfilled,
                     &_eina_promise_progress_notify_error, &progress_notify_ran);

   promise = eina_promise_owner_promise_get(owner);
   eina_promise_progress_cb_add(promise, &progress_callback, NULL, NULL); // never run
   eina_promise_progress_cb_add(promise, &progress_callback, NULL, NULL); // never run

   ck_assert(progress_notify_ran);

   eina_promise_unref(promise);
   eina_promise_owner_value_set(owner, NULL, NULL);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_ignored)
{
   Eina_Promise_Owner* owner;
   Eina_Promise* promise;

   eina_init();

   owner = eina_promise_add();
   promise = eina_promise_owner_promise_get(owner);
   eina_promise_unref(promise);
   eina_promise_owner_value_set(owner, NULL, NULL);

   eina_shutdown();
}
END_TEST

static void
_eina_test_promise_race_cb(void* data, void* value EINA_UNUSED)
{
   *(Eina_Bool*)data = EINA_TRUE;
   ck_assert_int_eq(*(int*)value, 42);
}

START_TEST(eina_test_pointer_promise_race)
{
   Eina_Promise_Owner* promise_owner;
   Eina_Promise* first[2] = {NULL, NULL};
   Eina_Promise* promise;
   Eina_Bool ran = EINA_FALSE;
   int v = 42;

   eina_init();

   promise_owner = eina_promise_add();
   first[0] = eina_promise_owner_promise_get(promise_owner);
   promise = eina_promise_race(eina_carray_iterator_new((void**)&first[0]));

   eina_promise_then(promise, &_eina_test_promise_race_cb, NULL, &ran);
   eina_promise_owner_value_set(promise_owner, &v, NULL);

   ck_assert(!!ran);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_pointer_promise_race_error)
{
   Eina_Promise_Owner* promise_owner;
   Eina_Promise* first[2] = {NULL, NULL};
   Eina_Promise* promise;
   int ran = 0;
   int error = 0xdeadbeef;

   eina_init();

   promise_owner = eina_promise_add();
   first[0] = eina_promise_owner_promise_get(promise_owner);
   promise = eina_promise_race(eina_carray_iterator_new((void**)&first[0]));

   eina_promise_ref(promise);

   eina_promise_then(promise, NULL, &_eina_test_error_cb, &ran);
   eina_promise_owner_error_set(promise_owner, error);

   ck_assert_int_eq(error, ran);
   ck_assert_int_eq(error, eina_promise_error_get(promise));
   ck_assert(!eina_promise_pending_is(promise));
   ck_assert(!eina_promise_owner_cancelled_is(promise_owner));

   eina_promise_unref(promise);

   eina_shutdown();
}
END_TEST

// Null promises
START_TEST(eina_test_promise_error_promise_null_with_cb)
{
    int ran = 0;
    eina_init();

    eina_promise_then(NULL, NULL, _eina_test_error_cb, &ran);

    ck_assert_int_eq(EINA_ERROR_PROMISE_NULL, ran);

    eina_shutdown();
}
END_TEST

START_TEST(eina_test_promise_error_promise_null_without_cb)
{
    eina_init();

    eina_promise_then(NULL, NULL, NULL, NULL);

    ck_assert_int_eq(EINA_ERROR_PROMISE_NULL, eina_error_get());

    eina_shutdown();
}
END_TEST

START_TEST(eina_test_promise_error_promise_all_unref)
{
   Eina_Promise_Owner* promise_owner;
   Eina_Promise* first[2] = {NULL, NULL};
   Eina_Promise* promise;

   eina_init();

   promise_owner = eina_promise_add();
   first[0] = eina_promise_owner_promise_get(promise_owner);
   promise = eina_promise_all(eina_carray_iterator_new((void**)&first[0]));

   eina_promise_unref(promise);

   eina_promise_owner_value_set(promise_owner, NULL, NULL);

   eina_shutdown();
}
END_TEST

static void _eina_test_promise_all_cb_lifetime_cb(void* data, Eina_Error error EINA_UNUSED)
{
   Eina_Promise* promise = data;
   ck_assert_int_eq(eina_promise_ref_get(promise), 4);
}

START_TEST(eina_test_promise_error_all_cb_lifetime)
{
   Eina_Promise_Owner* promise_owner[2];
   Eina_Promise* first[3] = {NULL, NULL, NULL};
   Eina_Promise* promise;

   eina_init();

   promise_owner[0] = eina_promise_add();
   promise_owner[1] = eina_promise_add();
   first[0] = eina_promise_owner_promise_get(promise_owner[0]);
   first[1] = eina_promise_owner_promise_get(promise_owner[1]);
   promise = eina_promise_all(eina_carray_iterator_new((void**)&first[0]));

   ck_assert_int_eq(eina_promise_ref_get(promise), 3);
   eina_promise_then(promise, NULL, &_eina_test_promise_all_cb_lifetime_cb, promise);
   ck_assert_int_eq(eina_promise_ref_get(promise), 3);

   eina_promise_owner_error_set(promise_owner[0], EINA_ERROR_PROMISE_CANCEL);

   ck_assert_int_eq(eina_promise_ref_get(promise), 1);
   
   eina_promise_owner_value_set(promise_owner[1], NULL, NULL);
   ck_assert_int_eq(eina_promise_ref_get(promise), 0);

   eina_shutdown();
}
END_TEST

void
eina_test_promise(TCase *tc)
{
  /* // pointer */
   tcase_add_test(tc, eina_test_pointer_promise_normal_lifetime);
   tcase_add_test(tc, eina_test_pointer_promise_normal_lifetime_all);
   tcase_add_test(tc, eina_test_pointer_promise_error_set);
   tcase_add_test(tc, eina_test_pointer_promise_error_set_all);
   tcase_add_test(tc, eina_test_pointer_promise_immediate_set_lifetime);
   tcase_add_test(tc, eina_test_pointer_promise_immediate_set_lifetime_all);
   tcase_add_test(tc, eina_test_pointer_promise_manual_then);
   tcase_add_test(tc, eina_test_pointer_promise_values_all);
   tcase_add_test(tc, eina_test_pointer_promise_cancel_promise);
   tcase_add_test(tc, eina_test_pointer_promise_cancel_finished_promise);
   tcase_add_test(tc, eina_test_pointer_promise_double_cancel_promise);
   tcase_add_test(tc, eina_test_pointer_promise_progress);
   tcase_add_test(tc, eina_test_pointer_promise_progress_notify1);
   tcase_add_test(tc, eina_test_pointer_promise_progress_notify2);
   tcase_add_test(tc, eina_test_pointer_promise_progress_notify3);
   tcase_add_test(tc, eina_test_pointer_promise_ignored);
   tcase_add_test(tc, eina_test_pointer_promise_race);
   tcase_add_test(tc, eina_test_pointer_promise_race_error);

   // Null promises
   tcase_add_test(tc, eina_test_promise_error_promise_null_with_cb);
   tcase_add_test(tc, eina_test_promise_error_promise_null_without_cb);
   tcase_add_test(tc, eina_test_promise_error_promise_all_unref);

   // Reference of composite promises
   tcase_add_test(tc, eina_test_promise_error_all_cb_lifetime);
}
