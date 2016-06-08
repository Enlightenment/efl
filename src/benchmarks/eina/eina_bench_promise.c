/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
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
#include <time.h>

#include "Evas_Data.h"
#include "Ecore_Data.h"

#include "eina_stringshare.h"
#include "eina_bench.h"
#include "eina_convert.h"
#include "eina_main.h"

#include <assert.h>

#include <Eina.h>

struct value_type
{
  int x;
  int y;
  int w;
  int h;
};

void cb(void* data EINA_UNUSED, void* value)
{
  struct value_type* p = value;
  volatile int v = p->x * p->y * p->w * p->h;
  (void)v;
}

void pointer_cb(void* data EINA_UNUSED, void* value)
{
  struct value_type* p = value;
  volatile int v = p->x * p->y * p->w * p->h;
  (void)v;
}

static void
eina_bench_promise_sync_then(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   eina_init();

   Eina_Promise_Owner* owner = eina_promise_add();
   Eina_Promise* promise = eina_promise_owner_promise_get(owner);
   eina_promise_ref(promise);

   struct value_type v = { 0, 0, 0, 0 };
   eina_promise_owner_value_set(owner, &v, NULL);
   
   for (j = 0; j != 200; ++j)
      for (i = 0; i != request; ++i)
        {
           eina_promise_then(promise, &cb, NULL, NULL);
        }

   /* Suppress warnings as we really don't want to do anything. */
   (void) tmp;

   eina_promise_unref(promise);
   eina_shutdown();
}

static void indirect_free(void* p)
{
  free(p);
}

static void
eina_bench_promise_pointer_value_set_before_then(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   eina_init();

   for (j = 0; j != 200; ++j)
      for (i = 0; i != request; ++i)
        {
           Eina_Promise_Owner* owner = eina_promise_add();
           Eina_Promise* promise = eina_promise_owner_promise_get(owner);

           struct value_type* p = malloc(sizeof(struct value_type));
           p->x = p->y = p->w = p->h = 0;

           eina_promise_then(promise, &pointer_cb, NULL, NULL);
           eina_promise_owner_value_set(owner, p, &indirect_free);
        }

   /* Suppress warnings as we really don't want to do anything. */
   (void) tmp;

   eina_shutdown();
}

static void
eina_bench_promise_pointer_value_set_after_then(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   eina_init();

   for (j = 0; j != 200; ++j)
      for (i = 0; i != request; ++i)
        {
           Eina_Promise_Owner* owner = eina_promise_add();
           Eina_Promise* promise = eina_promise_owner_promise_get(owner);

           struct value_type* p = malloc(sizeof(struct value_type));
           p->x = p->y = p->w = p->h = 0;

           eina_promise_owner_value_set(owner, p, &indirect_free);
           eina_promise_then(promise, &pointer_cb, NULL, NULL);
        }

   /* Suppress warnings as we really don't want to do anything. */
   (void) tmp;

   eina_shutdown();
}

static Eina_Mempool* mempool;

static void indirect_mempool_free(void* p)
{
   eina_mempool_free(mempool, *(void**)p);
}

static void
eina_bench_promise_pointer_value_set_before_then_pooled(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   eina_init();

   mempool = eina_mempool_add("chained_mempool", "", NULL, sizeof(struct value_type), 10);
   assert(mempool != NULL);

   for (j = 0; j != 200; ++j)
      for (i = 0; i != request; ++i)
        {
           Eina_Promise_Owner* owner = eina_promise_add();
           Eina_Promise* promise = eina_promise_owner_promise_get(owner);

           struct value_type* p = eina_mempool_malloc(mempool, sizeof(struct value_type));
           p->x = p->y = p->w = p->h = 0;

           eina_promise_then(promise, &pointer_cb, NULL, NULL);
           eina_promise_owner_value_set(owner, p, &indirect_mempool_free);
        }

   /* Suppress warnings as we really don't want to do anything. */
   (void) tmp;

   eina_mempool_del(mempool);
   
   eina_shutdown();
}

static void
eina_bench_promise_pointer_value_set_after_then_pooled(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   eina_init();

   mempool = eina_mempool_add("chained_mempool", "", NULL, sizeof(struct value_type), 10);
   assert(mempool != NULL);

   for (j = 0; j != 200; ++j)
      for (i = 0; i != request; ++i)
        {
           Eina_Promise_Owner* owner = eina_promise_add();
           Eina_Promise* promise = eina_promise_owner_promise_get(owner);

           struct value_type* p = eina_mempool_malloc(mempool, sizeof(struct value_type));
           p->x = p->y = p->w = p->h = 0;

           eina_promise_owner_value_set(owner, p, &indirect_mempool_free);
           eina_promise_then(promise, &pointer_cb, NULL, NULL);
        }

   /* Suppress warnings as we really don't want to do anything. */
   (void) tmp;

   eina_mempool_del(mempool);
   eina_shutdown();
}

static void
eina_bench_promise_pointer_value_set_before_then_non_alloc(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   eina_init();

   mempool = eina_mempool_add("chained_mempool", "", NULL, sizeof(struct value_type), 10);
   assert(mempool != NULL);

   for (j = 0; j != 200; ++j)
      for (i = 0; i != request; ++i)
        {
           Eina_Promise_Owner* owner = eina_promise_add();
           Eina_Promise* promise = eina_promise_owner_promise_get(owner);

           struct value_type v = {0,0,0,0};

           eina_promise_then(promise, &pointer_cb, NULL, NULL);
           eina_promise_owner_value_set(owner, &v, NULL);
        }

   /* Suppress warnings as we really don't want to do anything. */
   (void) tmp;

   eina_mempool_del(mempool);
   
   eina_shutdown();
}

static void
eina_bench_promise_pointer_value_set_after_then_non_alloc(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   eina_init();

   mempool = eina_mempool_add("chained_mempool", "", NULL, sizeof(struct value_type), 10);
   assert(mempool != NULL);

   for (j = 0; j != 200; ++j)
      for (i = 0; i != request; ++i)
        {
           Eina_Promise_Owner* owner = eina_promise_add();
           Eina_Promise* promise = eina_promise_owner_promise_get(owner);

           struct value_type v = {0,0,0,0};

           eina_promise_owner_value_set(owner, &v, NULL);
           eina_promise_then(promise, &pointer_cb, NULL, NULL);
        }

   /* Suppress warnings as we really don't want to do anything. */
   (void) tmp;

   eina_mempool_del(mempool);
   eina_shutdown();
}

void eina_bench_promise(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "promise synchronous then",
                           EINA_BENCHMARK(
                              eina_bench_promise_sync_then), 100, 20100, 500);
   eina_benchmark_register(bench, "promise pointer value set after then mempool",
                           EINA_BENCHMARK(
                              eina_bench_promise_pointer_value_set_after_then_pooled), 100, 20100, 500);
   eina_benchmark_register(bench, "promise pointer value set before then mempool",
                           EINA_BENCHMARK(
                              eina_bench_promise_pointer_value_set_before_then_pooled), 100, 20100, 500);
   eina_benchmark_register(bench, "promise pointer value set after then",
                           EINA_BENCHMARK(
                              eina_bench_promise_pointer_value_set_after_then), 100, 20100, 500);
   eina_benchmark_register(bench, "promise pointer value set before then",
                           EINA_BENCHMARK(
                              eina_bench_promise_pointer_value_set_before_then), 100, 20100, 500);
   eina_benchmark_register(bench, "promise pointer value set after then non alloc",
                           EINA_BENCHMARK(
                              eina_bench_promise_pointer_value_set_after_then_non_alloc), 100, 20100, 500);
   eina_benchmark_register(bench, "promise pointer value set before then non alloc",
                           EINA_BENCHMARK(
                              eina_bench_promise_pointer_value_set_before_then_non_alloc), 100, 20100, 500);
}
