/* EINA - EFL data type library
 * Copyright (C) 2026 Cedric Bail
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

#include <stdio.h>
#include <stdlib.h>

#include <Eina.h>

#include "eina_suite.h"

/* Promises need somewhere safe to deliver from. Outside of a running loop
 * there is no such place, so the tests bring their own: a scheduler that
 * queues deliveries and hands them over when the test says so.
 *
 * Being able to say when is the point. A test that drives the loop can only
 * ask whether the end state is right; this one can stop between the promise
 * being resolved and the value being delivered, which is where the
 * interesting mistakes live.
 */
typedef struct
{
   Eina_Future_Scheduler  scheduler;
   Eina_List             *queue;   /* Scheduled_Delivery * */
} Test_Scheduler;

typedef struct
{
   Eina_Future_Scheduler_Cb cb;
   Eina_Future             *future;
   Eina_Value               value;
   Test_Scheduler          *owner;
} Scheduled_Delivery;

static Eina_Future_Schedule_Entry *
_test_schedule(Eina_Future_Scheduler *s, Eina_Future_Scheduler_Cb cb,
               Eina_Future *f, Eina_Value value)
{
   Test_Scheduler *ts = (Test_Scheduler *) s;
   Scheduled_Delivery *d = calloc(1, sizeof (Scheduled_Delivery));

   if (!d)
     {
        eina_value_flush(&value);
        return NULL;
     }

   d->cb = cb;
   d->future = f;
   d->value = value;
   d->owner = ts;
   ts->queue = eina_list_append(ts->queue, d);

   return (Eina_Future_Schedule_Entry *) d;
}

static void
_test_recall(Eina_Future_Schedule_Entry *entry)
{
   Scheduled_Delivery *d = (Scheduled_Delivery *) entry;

   d->owner->queue = eina_list_remove(d->owner->queue, d);
   eina_value_flush(&d->value);
   free(d);
}

static void
_test_scheduler_setup(Test_Scheduler *ts)
{
   memset(ts, 0, sizeof (*ts));
   ts->scheduler.schedule = _test_schedule;
   ts->scheduler.recall = _test_recall;
}

/* Deliver everything queued, including anything queued while delivering. */
static void
_test_scheduler_pump(Test_Scheduler *ts)
{
   while (ts->queue)
     {
        Scheduled_Delivery *d = eina_list_data_get(ts->queue);

        ts->queue = eina_list_remove_list(ts->queue, ts->queue);
        d->cb(d->future, d->value);
        free(d);
     }
}

static void
_test_scheduler_flush(Test_Scheduler *ts)
{
   Scheduled_Delivery *d;

   EINA_LIST_FREE(ts->queue, d)
     {
        eina_value_flush(&d->value);
        free(d);
     }
}

static void
_dummy_promise_cancel(void *data EINA_UNUSED, const Eina_Promise *dead EINA_UNUSED)
{
}

/* ------------------------------------------------------------------------ */

static int chained_calls;
static int inner_calls;
static int outer_calls;
static int outer_error;

static Eina_Value
_count_cb(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   chained_calls++;
   return v;
}

/* A future callback that answers with another future, which is what
 * eina_future_as_value is for. */
static Eina_Value
_return_inner_future(void *data, const Eina_Value v,
                     const Eina_Future *dead EINA_UNUSED)
{
   Eina_Promise **inner = data;

   /* An error on the way in is not a reason to start more work, and a
      callback that answered a cancellation with a fresh future would be
      keeping the chain alive past the point it was told to stop. Real ones
      use .success for this; a raw eina_future_then callback has to look. */
   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ERROR) return v;

   inner_calls++;
   return eina_future_as_value(eina_future_new(*inner));
}

static Eina_Value
_outer_done(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   outer_calls++;
   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ERROR)
     eina_value_error_get(&v, &outer_error);
   return v;
}

static void
_counters_reset(void)
{
   chained_calls = inner_calls = outer_calls = 0;
   outer_error = 0;
}

/* The ordinary case, and the one that has to stay working: a chain whose
 * middle link answers with another future. The outer chain waits for the
 * inner one and carries its value onwards. */
EFL_START_TEST(eina_test_promise_future_as_value_resolves)
{
   Test_Scheduler ts;
   Eina_Promise *outer, *inner;
   Eina_Future *f;

   _test_scheduler_setup(&ts);
   _counters_reset();

   outer = eina_promise_new(&ts.scheduler, _dummy_promise_cancel, NULL);
   inner = eina_promise_new(&ts.scheduler, _dummy_promise_cancel, NULL);
   fail_if(!outer || !inner);

   f = eina_future_new(outer);
   f = eina_future_then(f, _return_inner_future, &inner, NULL);
   f = eina_future_then(f, _outer_done, NULL, NULL);
   fail_if(!f);

   /* The outer resolves; the middle link hands back the inner future, and
      the chain stops there until the inner one answers. */
   eina_promise_resolve(outer, eina_value_int_init(1));
   _test_scheduler_pump(&ts);
   ck_assert_int_eq(inner_calls, 1);
   ck_assert_int_eq(outer_calls, 0);

   /* When it does, the value travels the rest of the way. */
   eina_promise_resolve(inner, eina_value_int_init(42));
   _test_scheduler_pump(&ts);
   ck_assert_int_eq(outer_calls, 1);
   ck_assert_int_eq(outer_error, 0);

   _test_scheduler_flush(&ts);
}
EFL_END_TEST

/* The same, with the inner future rejected: the error is what travels. */
EFL_START_TEST(eina_test_promise_future_as_value_rejects)
{
   Test_Scheduler ts;
   Eina_Promise *outer, *inner;
   Eina_Future *f;

   _test_scheduler_setup(&ts);
   _counters_reset();

   outer = eina_promise_new(&ts.scheduler, _dummy_promise_cancel, NULL);
   inner = eina_promise_new(&ts.scheduler, _dummy_promise_cancel, NULL);
   fail_if(!outer || !inner);

   f = eina_future_new(outer);
   f = eina_future_then(f, _return_inner_future, &inner, NULL);
   f = eina_future_then(f, _outer_done, NULL, NULL);
   fail_if(!f);

   eina_promise_resolve(outer, eina_value_int_init(1));
   _test_scheduler_pump(&ts);
   ck_assert_int_eq(inner_calls, 1);

   eina_promise_reject(inner, EINVAL);
   _test_scheduler_pump(&ts);
   ck_assert_int_eq(outer_calls, 1);
   ck_assert_int_eq(outer_error, EINVAL);

   _test_scheduler_flush(&ts);
}
EFL_END_TEST

/* Cancelling the chain before anything has been answered, which is the
 * shape every widget teardown has. */
EFL_START_TEST(eina_test_promise_future_as_value_cancel_before_resolve)
{
   Test_Scheduler ts;
   Eina_Promise *outer, *inner;
   Eina_Future *f;

   _test_scheduler_setup(&ts);
   _counters_reset();

   outer = eina_promise_new(&ts.scheduler, _dummy_promise_cancel, NULL);
   inner = eina_promise_new(&ts.scheduler, _dummy_promise_cancel, NULL);
   fail_if(!outer || !inner);

   f = eina_future_new(outer);
   f = eina_future_then(f, _return_inner_future, &inner, NULL);
   f = eina_future_then(f, _outer_done, NULL, NULL);
   fail_if(!f);

   /* Nothing has answered yet, so the middle link has not run and there is
      no inner future in play. */
   eina_future_cancel(f);
   ck_assert_int_eq(inner_calls, 0);
   ck_assert_int_eq(outer_calls, 1);
   ck_assert_int_eq(outer_error, ECANCELED);

   eina_promise_reject(inner, ECANCELED);
   _test_scheduler_pump(&ts);

   _test_scheduler_flush(&ts);
}
EFL_END_TEST

/* The one this file was written for.
 *
 * A future callback answers with a second future, and the chain waiting on
 * it is then cancelled while that second future is still outstanding. The
 * proxy promise standing between them belongs to the cancelled side and goes
 * with it; the second future does not know that, and answers anyway.
 *
 * eina's own documentation says what makes this go wrong, under
 * Eina_Promise_Cancel_Cb: an empty cancel callback "will likely result in
 * memory corruption and invalid access", and the way out is to hold the
 * promise behind a pointer that the cancel callback clears and the
 * resolution checks. eina_future_as_value did not do that.
 */
EFL_START_TEST(eina_test_promise_future_as_value_cancel_while_inner_pending)
{
   Test_Scheduler ts;
   Eina_Promise *outer, *inner;
   Eina_Future *f;

   _test_scheduler_setup(&ts);
   _counters_reset();

   outer = eina_promise_new(&ts.scheduler, _dummy_promise_cancel, NULL);
   inner = eina_promise_new(&ts.scheduler, _dummy_promise_cancel, NULL);
   fail_if(!outer || !inner);

   f = eina_future_new(outer);
   f = eina_future_then(f, _return_inner_future, &inner, NULL);
   f = eina_future_then(f, _outer_done, NULL, NULL);
   fail_if(!f);

   /* Run as far as the inner future: the chain is now parked on it. */
   eina_promise_resolve(outer, eina_value_int_init(1));
   _test_scheduler_pump(&ts);
   ck_assert_int_eq(inner_calls, 1);
   ck_assert_int_eq(outer_calls, 0);

   /* The consumer gives up. Everything after the proxy is torn down. */
   eina_future_cancel(f);
   ck_assert_int_eq(outer_calls, 1);
   ck_assert_int_eq(outer_error, ECANCELED);

   /* And now the inner future answers into a proxy that is no longer there.
      Under a poisoning allocator this is where it falls over. */
   eina_promise_resolve(inner, eina_value_int_init(42));
   _test_scheduler_pump(&ts);

   /* The cancelled chain stays cancelled: nothing is delivered twice. */
   ck_assert_int_eq(outer_calls, 1);

   _test_scheduler_flush(&ts);
}
EFL_END_TEST

/* The same race the other way round: the inner future is rejected rather
 * than resolved after the consumer has gone. */
EFL_START_TEST(eina_test_promise_future_as_value_cancel_then_inner_rejects)
{
   Test_Scheduler ts;
   Eina_Promise *outer, *inner;
   Eina_Future *f;

   _test_scheduler_setup(&ts);
   _counters_reset();

   outer = eina_promise_new(&ts.scheduler, _dummy_promise_cancel, NULL);
   inner = eina_promise_new(&ts.scheduler, _dummy_promise_cancel, NULL);
   fail_if(!outer || !inner);

   f = eina_future_new(outer);
   f = eina_future_then(f, _return_inner_future, &inner, NULL);
   f = eina_future_then(f, _outer_done, NULL, NULL);
   fail_if(!f);

   eina_promise_resolve(outer, eina_value_int_init(1));
   _test_scheduler_pump(&ts);
   ck_assert_int_eq(inner_calls, 1);

   eina_future_cancel(f);
   ck_assert_int_eq(outer_calls, 1);

   eina_promise_reject(inner, EINVAL);
   _test_scheduler_pump(&ts);
   ck_assert_int_eq(outer_calls, 1);

   _test_scheduler_flush(&ts);
}
EFL_END_TEST

/* A plain chain with no proxy in it, so that the tests above are known to be
 * about the proxy and not about cancellation in general. */
EFL_START_TEST(eina_test_promise_cancel_plain_chain)
{
   Test_Scheduler ts;
   Eina_Promise *p;
   Eina_Future *f;

   _test_scheduler_setup(&ts);
   _counters_reset();

   p = eina_promise_new(&ts.scheduler, _dummy_promise_cancel, NULL);
   fail_if(!p);

   f = eina_future_new(p);
   f = eina_future_then(f, _count_cb, NULL, NULL);
   f = eina_future_then(f, _outer_done, NULL, NULL);
   fail_if(!f);

   eina_future_cancel(f);
   ck_assert_int_eq(chained_calls, 1);
   ck_assert_int_eq(outer_calls, 1);
   ck_assert_int_eq(outer_error, ECANCELED);

   _test_scheduler_flush(&ts);
}
EFL_END_TEST

/* ------------------------------------------------------------------------ */

static int combined_cancels;

static void
_count_promise_cancel(void *data EINA_UNUSED, const Eina_Promise *dead EINA_UNUSED)
{
   combined_cancels++;
}

/* Cancelling what eina_future_all or eina_future_race hands back cancels the
 * futures it waits on, whose callbacks then report to the combined promise --
 * which is the one being cancelled. Each pending promise hears its cancel
 * once, the combined future answers ECANCELED once, and nothing is freed
 * twice (valgrind shows the last; the counts show the rest). */
static void
_combined_cancel_check(Eina_Bool race)
{
   Test_Scheduler ts;
   Eina_Promise *p1, *p2;
   Eina_Future *f;

   _test_scheduler_setup(&ts);
   _counters_reset();
   combined_cancels = 0;

   p1 = eina_promise_new(&ts.scheduler, _count_promise_cancel, NULL);
   p2 = eina_promise_new(&ts.scheduler, _count_promise_cancel, NULL);
   fail_if(!p1 || !p2);

   if (race)
     f = eina_future_race(eina_future_new(p1), eina_future_new(p2));
   else
     f = eina_future_all(eina_future_new(p1), eina_future_new(p2));
   fail_if(!f);
   f = eina_future_then(f, _outer_done, NULL, NULL);
   fail_if(!f);

   eina_future_cancel(f);
   _test_scheduler_pump(&ts);

   ck_assert_int_eq(combined_cancels, 2);
   ck_assert_int_eq(outer_calls, 1);
   ck_assert_int_eq(outer_error, ECANCELED);

   _test_scheduler_flush(&ts);
}

EFL_START_TEST(eina_test_promise_all_cancel)
{
   _combined_cancel_check(EINA_FALSE);
}
EFL_END_TEST

EFL_START_TEST(eina_test_promise_race_cancel)
{
   _combined_cancel_check(EINA_TRUE);
}
EFL_END_TEST

void
eina_test_promise(TCase *tc)
{
   tcase_add_test(tc, eina_test_promise_cancel_plain_chain);
   tcase_add_test(tc, eina_test_promise_future_as_value_resolves);
   tcase_add_test(tc, eina_test_promise_future_as_value_rejects);
   tcase_add_test(tc, eina_test_promise_future_as_value_cancel_before_resolve);
   tcase_add_test(tc, eina_test_promise_future_as_value_cancel_while_inner_pending);
   tcase_add_test(tc, eina_test_promise_future_as_value_cancel_then_inner_rejects);
   tcase_add_test(tc, eina_test_promise_all_cancel);
   tcase_add_test(tc, eina_test_promise_race_cancel);
}
