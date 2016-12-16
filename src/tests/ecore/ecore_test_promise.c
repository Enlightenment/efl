#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_suite.h"
#include <time.h>

typedef struct _Future_Ok Future_Ok;
struct _Future_Ok
{
   Eina_Bool then : 1;
   Eina_Bool cancel : 1;
   Eina_Bool progress : 1;
};

static void
_then(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *s = ev->info;
   int *value = s->value;
   Future_Ok *fo = data;

   fail_if(*value != 42);
   fo->then = EINA_TRUE;
}

static void
_time_then(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *s = ev->info;
   Future_Ok *value = s->value;
   Future_Ok *fo = data;

   fail_if(value != fo);
   fo->then = EINA_TRUE;

   ecore_main_loop_quit();
}

static void
_connected_then(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *s = ev->info;
   Efl_Future *f;

   f = efl_loop_timeout(efl_provider_find(ev->object, EFL_LOOP_CLASS), 0.01, data);
   efl_promise_connect(s->next, f);
}

static void
_cancel(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Failure *f = ev->info;
   Future_Ok *fo = data;

   fail_if(f->error != EINA_ERROR_FUTURE_CANCEL);
   fo->cancel = EINA_TRUE;
}

static void
_progress(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Progress *p = ev->info;
   const int *value = p->progress;
   Future_Ok *fo = data;

   fail_if(*value != 7);
   fo->progress = EINA_TRUE;
}

static void
_death(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eina_Bool *death = data;

   *death = EINA_TRUE;
}

// Test value set after then
START_TEST(efl_test_promise_future_success)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo));

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   ecore_main_loop_iterate();

   fail_if(!fo.then || fo.cancel || !fo.progress);
   fail_if(!deadf || deadp);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

// Test value set after then
START_TEST(efl_test_promise_future_connected)
{
   Efl_Promise *p;
   Efl_Future *chain;
   Efl_Future *f;
   Future_Ok fo = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);

   chain = efl_future_then(f, _connected_then, _cancel, _progress, &fo);
   fail_if(!chain);
   fail_if(!efl_future_then(chain, _time_then, _cancel, NULL, &fo));

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   ecore_main_loop_begin();

   ecore_main_loop_iterate();

   fail_if(!fo.then || fo.cancel || !fo.progress);
   fail_if(!deadf || deadp);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

// Test value set before future_get
START_TEST(efl_test_promise_future_success_before_get)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_promise_value_set(p, &value, NULL);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);

   fail_if(deadp || deadf);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo));

   ecore_main_loop_iterate();

   fail_if(f);
   fail_if(!fo.then || fo.cancel || fo.progress);
   fail_if(!deadf || deadp);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_future_cancel)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);
   efl_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo));

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f);

   efl_promise_value_set(p, &value, NULL);

   ecore_main_loop_iterate();

   fail_if(fo.then || !fo.cancel || !fo.progress);
   fail_if(!deadf || deadp);
   fail_if(!none);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

// Test value set before then
START_TEST(efl_test_promise_before_future_success)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo));

   ecore_main_loop_iterate();

   fail_if(deadp || !deadf);
   fail_if(!fo.then || fo.cancel || fo.progress);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_before_future_cancel)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);
   efl_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo));

   fail_if(deadp || !deadf);

   efl_promise_value_set(p, &value, NULL);

   ecore_main_loop_iterate();

   fail_if(fo.then || !fo.cancel || fo.progress);
   fail_if(!none);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

static void
_chain_then(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *s = ev->info;
   int *v = s->value;
   Future_Ok *fo = data;

   fo->then = EINA_TRUE;

   fail_if(*v != 42);

   efl_promise_value_set(s->next, v, NULL);
}

static void
_chain_fail(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Failure *f = ev->info;
   Future_Ok *fo = data;

   fo->cancel = EINA_TRUE;

   efl_promise_failed_set(f->next, f->error);
}

static void
_chain_progress(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Progress *p = ev->info;
   Future_Ok *fo = data;

   fo->progress = EINA_TRUE;

   efl_promise_progress_set(p->next, p->progress);
}

// Test chained then
START_TEST(efl_test_promise_future_chain_success)
{
   Efl_Promise *p;
   Efl_Future *f1, *f2;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf1 = EINA_FALSE, deadf2 = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f1 = efl_promise_future_get(p);
   fail_if(!f1);

   efl_event_callback_add(f1, EFL_EVENT_DEL, _death, &deadf1);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);

   f2 = efl_future_then(f1, _chain_then, _chain_fail, _chain_progress, &fo1);
   fail_if(!f2);

   efl_event_callback_add(f2, EFL_EVENT_DEL, _death, &deadf2);

   fail_if(!efl_future_then(f2, _then, _cancel, _progress, &fo2));

   fail_if(deadp || deadf1 || deadf2);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   ecore_main_loop_iterate();

   fail_if(!fo1.then || fo1.cancel || !fo1.progress);
   fail_if(!fo2.then || fo2.cancel || !fo2.progress);
   fail_if(!deadf1 || !deadf2 || deadp);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_future_chain_cancel)
{
   Efl_Promise *p;
   Efl_Future *f1, *f2;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf1 = EINA_FALSE, deadf2 = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f1, efl_promise_future_get(p));
   fail_if(!f1);

   efl_event_callback_add(f1, EFL_EVENT_DEL, _death, &deadf1);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);
   efl_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   f2 = efl_future_then(f1, _chain_then, _chain_fail, _chain_progress, &fo1);
   fail_if(!f2);

   efl_event_callback_add(f2, EFL_EVENT_DEL, _death, &deadf2);

   fail_if(!efl_future_then(f2, _then, _cancel, _progress, &fo2));

   fail_if(deadp || deadf1 || deadf2);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f1);

   efl_promise_value_set(p, &value, NULL);

   ecore_main_loop_iterate();

   fail_if(fo1.then || !fo1.cancel || !fo1.progress);
   fail_if(fo2.then || !fo2.cancel || !fo2.progress);
   fail_if(!deadf1 || !deadf2 || deadp);
   fail_if(!none);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

// Test value set after multi then
START_TEST(efl_test_promise_future_multi_success)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);

   efl_ref(f);
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo1));
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo2));
   efl_unref(f);

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   ecore_main_loop_iterate();

   fail_if(!fo1.then || fo1.cancel || !fo1.progress);
   fail_if(!fo2.then || fo2.cancel || !fo2.progress);
   fail_if(!deadf || deadp);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_future_multi_success_noref)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo1));
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo2));

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   ecore_main_loop_iterate();

   fail_if(!fo1.then || fo1.cancel || !fo1.progress);
   fail_if(!fo2.then || fo2.cancel || !fo2.progress);
   fail_if(!deadf || deadp);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_future_multi_cancel)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);
   efl_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo1));
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo2));

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f);

   efl_promise_value_set(p, &value, NULL);

   ecore_main_loop_iterate();

   fail_if(fo1.then || !fo1.cancel || !fo1.progress);
   fail_if(fo2.then || !fo2.cancel || !fo2.progress);
   fail_if(!deadf || deadp);
   fail_if(!none);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

// Test value set before multi then
START_TEST(efl_test_promise_before_future_multi_success)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   efl_ref(f);
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo1));
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo2));
   efl_unref(f);

   ecore_main_loop_iterate();

   fail_if(deadp || !deadf);
   fail_if(!fo1.then || fo1.cancel || fo1.progress);
   fail_if(!fo2.then || fo2.cancel || fo2.progress);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_before_future_multi_success_noref)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo1));
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo2));

   ecore_main_loop_iterate();

   fail_if(deadp || !deadf);
   fail_if(!fo1.then || fo1.cancel || fo1.progress);
   fail_if(!fo2.then || fo2.cancel || fo2.progress);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_before_future_multi_cancel)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo3 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);
   efl_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f);

   efl_ref(f);
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo1));
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo2));
   efl_unref(f);

   fail_if(efl_future_then(f, _then, _cancel, _progress, &fo3));

   fail_if(deadp || !deadf);

   efl_promise_value_set(p, &value, NULL);

   ecore_main_loop_iterate();

   fail_if(fo1.then || !fo1.cancel || fo1.progress);
   fail_if(fo2.then || !fo2.cancel || fo2.progress);
   fail_if(fo3.then || fo3.cancel || fo3.progress);
   fail_if(!none);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_before_future_multi_cancel_noref)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo3 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);
   efl_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   efl_promise_progress_set(p, &progress);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo1));
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo2));

   // efl_future_cancel is not delayed, only operation on promise are.
   efl_future_cancel(f);

   fail_if(efl_future_then(f, _then, _cancel, _progress, &fo3));

   fail_if(deadp || !deadf);

   efl_promise_value_set(p, &value, NULL);

   ecore_main_loop_iterate();

   fail_if(fo1.then || !fo1.cancel || fo1.progress);
   fail_if(fo2.then || !fo2.cancel || fo2.progress);
   fail_if(fo3.then || fo3.cancel || fo3.progress);
   fail_if(!none);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

static Eina_Bool cleanup = EINA_FALSE;

static void
_cleanup_called(void *s EINA_UNUSED)
{
   cleanup = EINA_TRUE;
}

// Test optional value set without then
START_TEST(efl_test_promise_future_optional_success)
{
   Efl_Promise *p;
   Efl_Future *f;
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   cleanup = EINA_FALSE;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, _cleanup_called);

   fail_if(deadf || deadp);

   ecore_main_loop_iterate();

   fail_if(!deadf);

   efl_del(p);

   fail_if(!deadp || !cleanup);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_future_optional_cancel)
{
   Efl_Promise *p;
   Efl_Future *f;
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   cleanup = EINA_FALSE;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   efl_event_callback_add(f, EFL_EVENT_DEL, _death, &deadf);
   efl_event_callback_add(p, EFL_EVENT_DEL, _death, &deadp);
   efl_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f);

   efl_promise_value_set(p, &value, _cleanup_called);

   fail_if(deadf || deadp);
   fail_if(!none);

   ecore_main_loop_iterate();

   fail_if(!deadf || deadp);

   efl_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

static int value[] = { 42, 7, 3 };

static void
_then_all(void *data, const Efl_Event *ev)
{
   Future_Ok *fo = data;
   Efl_Future_Event_Success *s = ev->info;
   Eina_Accessor *ac = s->value;
   int *i;

   fail_if(!eina_accessor_data_get(ac, 0, (void**) &i));
   fail_if(i != &value[0]);
   fail_if(!eina_accessor_data_get(ac, 1, (void**) &i));
   fail_if(i != &value[1]);
   fail_if(!eina_accessor_data_get(ac, 2, (void**) &i));
   fail_if(i != &value[2]);

   fo->then = EINA_TRUE;
}

START_TEST(efl_test_promise_all)
{
   Efl_Promise *p1, *p2, *p3;
   Efl_Future *all = NULL, *f1;
   Future_Ok donea = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok donep1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };

   ecore_init();

   p1 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   p2 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   p3 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p1 || !p2 || !p3);

   f1 = efl_ref(efl_promise_future_get(p1));
   fail_if(!efl_future_then(f1, _then, _cancel, _progress, &donep1));
   efl_future_use(&all, efl_future_all(f1, efl_promise_future_get(p2), efl_promise_future_get(p3)));
   efl_unref(f1);

   fail_if(!efl_future_then(all, _then_all, _cancel, _progress, &donea));
   fail_if(!all);

   efl_promise_value_set(p1, &value[0], NULL);

   ecore_main_loop_iterate();

   fail_if(!donep1.then || donep1.cancel || donep1.progress);
   fail_if(donea.then || donea.cancel || donea.progress);

   efl_promise_value_set(p2, &value[1], NULL);
   efl_promise_value_set(p3, &value[2], NULL);

   ecore_main_loop_iterate();

   fail_if(!donea.then || donea.cancel || donea.progress);
   fail_if(all);

   efl_del(p1);
   efl_del(p2);
   efl_del(p3);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_all_after_value_set)
{
   Efl_Promise *p1, *p2, *p3;
   Efl_Future *all = NULL, *f1, *f2, *f3;
   Future_Ok donea = { EINA_FALSE, EINA_FALSE, EINA_FALSE };

   ecore_init();

   p1 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   p2 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   p3 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p1 || !p2 || !p3);

   f1 = efl_promise_future_get(p1);
   f2 = efl_promise_future_get(p2);
   f3 = efl_promise_future_get(p3);

   efl_promise_value_set(p1, &value[0], NULL);
   efl_promise_value_set(p2, &value[1], NULL);
   efl_promise_value_set(p3, &value[2], NULL);

   efl_future_use(&all, efl_future_all(f1, f2, f3));

   fail_if(!all);

   fail_if(!efl_future_then(all, _then_all, _cancel, _progress, &donea));

   ecore_main_loop_iterate();

   fail_if(!donea.then || donea.cancel || donea.progress);

   fail_if(all);

   efl_del(p1);
   efl_del(p2);
   efl_del(p3);

   ecore_shutdown();
}
END_TEST

static void
_then_race(void *data, const Efl_Event *ev)
{
   Future_Ok *fo = data;
   Efl_Future_Event_Success *s = ev->info;
   Efl_Future_Race_Success *rs = s->value;

   fail_if(rs->index != 1);
   fail_if(rs->value != &value[0]);

   fo->then = EINA_TRUE;
}

START_TEST(efl_test_promise_race)
{
   Efl_Promise *p1, *p2, *p3;
   Efl_Future *race = NULL, *f1;
   Future_Ok donea = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok donep1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };

   ecore_init();

   p1 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   p2 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   p3 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p1 || !p2 || !p3);

   f1 = efl_ref(efl_promise_future_get(p1));
   fail_if(!efl_future_then(f1, _then, _cancel, _progress, &donep1));
   efl_future_use(&race, efl_future_race(f1, efl_promise_future_get(p2), efl_promise_future_get(p3)));
   efl_unref(f1);

   fail_if(!efl_future_then(race, _then_race, _cancel, _progress, &donea));
   fail_if(!race);

   efl_promise_value_set(p2, &value[0], NULL);

   ecore_main_loop_iterate();

   fail_if(donep1.then || !donep1.cancel || donep1.progress);
   fail_if(!donea.then || donea.cancel || donea.progress);

   fail_if(race);

   efl_del(p1);
   efl_del(p2);
   efl_del(p3);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_future_link)
{
   Efl_Promise *p;
   Efl_Future *f;
   Eo *o;

   ecore_init();

   o = efl_add(EFL_LOOP_TIMER_CLASS, ecore_main_loop_get());
   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   efl_future_use(&f, efl_promise_future_get(p));
   efl_future_link(o, f);

   fail_if(!o || !p || !f);
   efl_del(o);
   fail_if(f);

   o = efl_add(EFL_LOOP_TIMER_CLASS, ecore_main_loop_get());
   efl_future_use(&f, efl_promise_future_get(p));
   efl_future_cancel(f);

   efl_del(o);
   efl_del(p);

   ecore_shutdown();
}
END_TEST

static Efl_Future *recursive_future = NULL;

static void
_then_cleanup(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_future_cancel(recursive_future);
}

START_TEST(efl_test_recursive_mess)
{
   Efl_Promise *p;
   Future_Ok done = { EINA_FALSE, EINA_FALSE, EINA_FALSE };

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   efl_future_use(&recursive_future, efl_promise_future_get(p));
   efl_future_then(recursive_future, _then_cleanup, _cancel, NULL, &done);

   efl_promise_value_set(p, &value[0], NULL);

   ecore_main_loop_iterate();

   efl_del(p);

   ecore_shutdown();
}
END_TEST

void ecore_test_ecore_promise(TCase *tc)
{
   tcase_add_test(tc, efl_test_promise_future_success);
   tcase_add_test(tc, efl_test_promise_future_cancel);
   tcase_add_test(tc, efl_test_promise_future_connected);
   tcase_add_test(tc, efl_test_promise_future_chain_success);
   tcase_add_test(tc, efl_test_promise_future_chain_cancel);
   tcase_add_test(tc, efl_test_promise_before_future_success);
   tcase_add_test(tc, efl_test_promise_before_future_cancel);
   tcase_add_test(tc, efl_test_promise_future_multi_success);
   tcase_add_test(tc, efl_test_promise_future_multi_success_noref);
   tcase_add_test(tc, efl_test_promise_future_multi_cancel);
   tcase_add_test(tc, efl_test_promise_before_future_multi_success);
   tcase_add_test(tc, efl_test_promise_before_future_multi_success_noref);
   tcase_add_test(tc, efl_test_promise_before_future_multi_cancel);
   tcase_add_test(tc, efl_test_promise_before_future_multi_success_noref);
   tcase_add_test(tc, efl_test_promise_before_future_multi_cancel_noref);
   tcase_add_test(tc, efl_test_promise_future_optional_success);
   tcase_add_test(tc, efl_test_promise_future_optional_cancel);
   tcase_add_test(tc, efl_test_promise_all);
   tcase_add_test(tc, efl_test_promise_all_after_value_set);
   tcase_add_test(tc, efl_test_promise_future_success_before_get);
   tcase_add_test(tc, efl_test_promise_race);
   tcase_add_test(tc, efl_test_future_link);
   tcase_add_test(tc, efl_test_recursive_mess);
}
