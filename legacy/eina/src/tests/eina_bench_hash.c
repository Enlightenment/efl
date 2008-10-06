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
#include <string.h>
#include <time.h>

#ifdef EINA_BENCH_HAVE_GLIB
# include <glib.h>
#endif

#ifdef EINA_BENCH_HAVE_EVAS
# include <Evas.h>
#endif

#ifdef EINA_BENCH_HAVE_ECORE
# include <Ecore_Data.h>
#endif

#include "eina_hash.h"
#include "eina_array.h"
#include "eina_bench.h"
#include "eina_rbtree.h"
#include "eina_convert.h"

typedef struct _Eina_Bench_Rbtree Eina_Bench_Rbtree;
struct _Eina_Bench_Rbtree
{
   Eina_Rbtree node;
   char key[10];
   int value;
};

static Eina_Rbtree_Direction
_eina_bench_rbtree_cmp(const Eina_Bench_Rbtree *left, const Eina_Bench_Rbtree *right, __UNUSED__ void *data)
{
   if (!left) return EINA_RBTREE_RIGHT;
   if (!right) return EINA_RBTREE_LEFT;

   return strcmp(left->key, right->key) < 0 ? EINA_RBTREE_LEFT : EINA_RBTREE_RIGHT;
}

static inline int
_eina_bench_rbtree_key(const Eina_Bench_Rbtree *node, const char *key, int length, __UNUSED__ void *data)
{
   return strncmp(node->key, key, length);
}

static void
_eina_bench_rbtree_free(Eina_Rbtree *node, __UNUSED__ void *data)
{
   free(node);
}

static void
eina_bench_lookup_rbtree(int request)
{
   Eina_Rbtree *root = NULL;
   Eina_Rbtree *tmp;
   int i;
   int j;

   for (i = 0; i < request; ++i)
     {
	Eina_Bench_Rbtree *tmp;

	tmp = malloc(sizeof (Eina_Bench_Rbtree));
	if (!tmp) continue ;

	tmp->value = i;
	eina_convert_itoa(i, tmp->key);

	root = eina_rbtree_inline_insert(root, &tmp->node, EINA_RBTREE_CMP_NODE_CB(_eina_bench_rbtree_cmp), NULL);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < request; ++i)
       {
	  char tmp_key[10];

	  eina_convert_itoa(rand() % request, tmp_key);

	  tmp = eina_rbtree_inline_lookup(root, tmp_key, 10, EINA_RBTREE_CMP_KEY_CB(_eina_bench_rbtree_key), NULL);
       }

   eina_rbtree_delete(root, EINA_RBTREE_FREE_CB(_eina_bench_rbtree_free), NULL);
}

static void
eina_bench_lookup_superfast(int request)
{
   Eina_Hash *hash = NULL;
   int *tmp_val;
   unsigned int i;
   unsigned int j;

   hash = eina_hash_string_superfast_new(free);

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	tmp_val = malloc(sizeof (int));

	if (!tmp_val) continue ;

	eina_convert_itoa(i, tmp_key);
	*tmp_val = i;

	eina_hash_add(hash, tmp_key, tmp_val);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < (unsigned int) request; ++i)
       {
	  char tmp_key[10];

	  eina_convert_itoa(rand() % request, tmp_key);
	  tmp_val = eina_hash_find(hash, tmp_key);
       }

   eina_hash_free(hash);
}

static void
eina_bench_lookup_djb2(int request)
{
   Eina_Hash *hash = NULL;
   int *tmp_val;
   unsigned int i;
   unsigned int j;

   hash = eina_hash_string_djb2_new(free);

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	tmp_val = malloc(sizeof (int));

	if (!tmp_key || !tmp_val) continue ;

	eina_convert_itoa(i, tmp_key);
	*tmp_val = i;

	eina_hash_add(hash, tmp_key, tmp_val);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < (unsigned int) request; ++i)
       {
	  char tmp_key[10];

	  eina_convert_itoa(rand() % request, tmp_key);

	  tmp_val = eina_hash_find(hash, tmp_key);
       }

   eina_hash_free(hash);
}

typedef struct _Eina_Bench_DJB2 Eina_Bench_DJB2;
struct _Eina_Bench_DJB2
{
   char *key;
   int value;
};

static void
eina_bench_lookup_djb2_inline(int request)
{
   Eina_Hash *hash = NULL;
   Eina_Bench_DJB2 *elm;
   unsigned int i;
   unsigned int j;

   hash = eina_hash_string_djb2_new(free);

   for (i = 0; i < (unsigned int) request; ++i)
     {
	int length;

	elm = malloc(sizeof (Eina_Bench_DJB2) + 10);
	if (!elm) continue ;

	elm->key = (char*) (elm + 1);

	length = eina_convert_itoa(i, elm->key) + 1;
	elm->value = i;

	eina_hash_direct_add_by_hash(hash, elm->key, length, eina_hash_djb2(elm->key, length), elm);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < (unsigned int) request; ++i)
       {
	  char tmp_key[10];
	  int length = 6;

	  length = eina_convert_itoa(rand() % request, tmp_key) + 1;

	  elm = eina_hash_find_by_hash(hash, tmp_key, length, eina_hash_djb2(tmp_key, length));
       }

   eina_hash_free(hash);
}

#ifdef EINA_BENCH_HAVE_GLIB
typedef struct _Eina_Bench_Glib Eina_Bench_Glib;
struct _Eina_Bench_Glib
{
   char *key;
   int value;
};

static void
eina_bench_lookup_ghash(int request)
{
   Eina_Bench_Glib *elm;
   GHashTable *hash;
   unsigned int i;
   unsigned int j;

   hash = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, free);

   for (i = 0; i < (unsigned int) request; ++i)
     {
	elm = malloc(sizeof (Eina_Bench_Glib) + 10);
	if (!elm) continue ;

	elm->key = (char*) (elm + 1);

	eina_convert_itoa(i, elm->key);
	elm->value = i;

	g_hash_table_insert(hash, elm->key, elm);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < (unsigned int) request; ++i)
       {
	  char tmp_key[10];

	  eina_convert_itoa(rand() % request, tmp_key);

	  elm = g_hash_table_lookup(hash, tmp_key);
       }

   g_hash_table_destroy(hash);
}
#endif

#ifdef EINA_BENCH_HAVE_EVAS
static void
eina_bench_lookup_evas(int request)
{
   Evas_Hash *hash = NULL;
   Eina_Array *array = NULL;
   int *tmp_val;
   Eina_Array_Iterator it;
   unsigned int i;
   unsigned int j;

   array = eina_array_new(1000);

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	tmp_val = malloc(sizeof (int));

	if (!tmp_key || !tmp_val) continue ;

	eina_convert_itoa(i, tmp_key);
	*tmp_val = i;

	hash = evas_hash_add(hash, tmp_key, tmp_val);

	eina_array_push(array, tmp_val);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < (unsigned int) request; ++i)
       {
	  char tmp_key[10];

	  eina_convert_itoa(rand() % request, tmp_key);

	  tmp_val = evas_hash_find(hash, tmp_key);
       }

   evas_hash_free(hash);

   EINA_ARRAY_ITER_NEXT(array, i, tmp_val, it)
     free(tmp_val);

   eina_array_free(array);
}
#endif

#ifdef EINA_BENCH_HAVE_ECORE
typedef struct _Eina_Bench_Ecore Eina_Bench_Ecore;
struct _Eina_Bench_Ecore
{
   char *key;
   int value;
};

static void
eina_bench_lookup_ecore(int request)
{
   Ecore_Hash *hash = NULL;
   Eina_Bench_Ecore *elm;
   unsigned int i;
   unsigned int j;

   hash = ecore_hash_new(ecore_str_hash, ecore_str_compare);

   ecore_hash_free_key_cb_set(hash, NULL);
   ecore_hash_free_value_cb_set(hash, free);

   for (i = 0; i < (unsigned int) request; ++i)
     {
	elm = malloc(sizeof (Eina_Bench_Ecore) + 10);
	if (!elm) continue;

	elm->key = (char*) (elm + 1);
	eina_convert_itoa(i, elm->key);
	elm->value = i;

	ecore_hash_set(hash, elm->key, elm);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < (unsigned int) request; ++i)
       {
	  char tmp_key[10];

	  eina_convert_itoa(rand() % request, tmp_key);

	  elm = ecore_hash_get(hash, tmp_key);
       }

   ecore_hash_destroy(hash);
}
#endif

void eina_bench_hash(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "superfast-lookup", EINA_BENCHMARK(eina_bench_lookup_superfast), 10, 3000, 10);
   eina_benchmark_register(bench, "djb2-lookup", EINA_BENCHMARK(eina_bench_lookup_djb2), 10, 3000, 10);
   eina_benchmark_register(bench, "djb2-lookup-inline", EINA_BENCHMARK(eina_bench_lookup_djb2_inline), 10, 3000, 10);
   eina_benchmark_register(bench, "rbtree", EINA_BENCHMARK(eina_bench_lookup_rbtree), 10, 3000, 10);
#ifdef EINA_BENCH_HAVE_GLIB
   eina_benchmark_register(bench, "ghash-lookup", EINA_BENCHMARK(eina_bench_lookup_ghash), 10, 3000, 10);
#endif
#ifdef EINA_BENCH_HAVE_EVAS
   eina_benchmark_register(bench, "evas-lookup", EINA_BENCHMARK(eina_bench_lookup_evas), 10, 3000, 10);
#endif
#ifdef EINA_BENCH_HAVE_ECORE
   eina_benchmark_register(bench, "ecore-lookup", EINA_BENCHMARK(eina_bench_lookup_ecore), 10, 3000, 10);
#endif
}
