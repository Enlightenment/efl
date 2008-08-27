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
#include <glib.h>
#endif

#include "eina_hash.h"
#include "eina_array.h"
#include "eina_bench.h"

static void
eina_bench_lookup_rbtree(int request)
{
   Eina_Hash *hash = NULL;
   Eina_Array *array = NULL;
   int *tmp_val;
   Eina_Array_Iterator it;
   unsigned int i;

   array = eina_array_new(1000);

   hash = eina_hash_string_superfast_new();

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	tmp_val = malloc(sizeof (int));

	if (!tmp_val) continue ;

	snprintf(tmp_key, 10, "%i", i);
	*tmp_val = i;

	eina_hash_add(hash, tmp_key, tmp_val);

	eina_array_push(array, tmp_val);
     }

   srand(time(NULL));

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	snprintf(tmp_key, 10, "%i", rand() % request);

	tmp_val = eina_hash_find(hash, tmp_key);
     }

   eina_hash_free(hash);

   EINA_ARRAY_ITER_NEXT(array, i, tmp_val, it)
     free(tmp_val);

   eina_array_free(array);
}

static void
eina_bench_lookup_superfast(int request)
{
   Eina_Hash *hash = NULL;
   Eina_Array *array = NULL;
   int *tmp_val;
   Eina_Array_Iterator it;
   unsigned int i;

   array = eina_array_new(1000);

   hash = eina_hash_string_superfast_new();

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	tmp_val = malloc(sizeof (int));

	if (!tmp_val) continue ;

	snprintf(tmp_key, 10, "%i", i);
	*tmp_val = i;

	eina_hash_add(hash, tmp_key, tmp_val);

	eina_array_push(array, tmp_val);
     }

   srand(time(NULL));

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	snprintf(tmp_key, 10, "%i", rand() % request);

	tmp_val = eina_hash_find(hash, tmp_key);
     }

   eina_hash_free(hash);

   EINA_ARRAY_ITER_NEXT(array, i, tmp_val, it)
     free(tmp_val);

   eina_array_free(array);
}

static void
eina_bench_lookup_djb2(int request)
{
   Eina_Hash *hash = NULL;
   Eina_Array *array = NULL;
   int *tmp_val;
   Eina_Array_Iterator it;
   unsigned int i;

   array = eina_array_new(1000);

   hash = eina_hash_string_djb2_new();

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	tmp_val = malloc(sizeof (int));

	if (!tmp_key || !tmp_val) continue ;

	snprintf(tmp_key, 10, "%i", i);
	*tmp_val = i;

	eina_hash_add(hash, tmp_key, tmp_val);

	eina_array_push(array, tmp_val);
     }

   srand(time(NULL));

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	snprintf(tmp_key, 10, "%i", rand() % request);

	tmp_val = eina_hash_find(hash, tmp_key);
     }

   eina_hash_free(hash);

   EINA_ARRAY_ITER_NEXT(array, i, tmp_val, it)
     free(tmp_val);

   eina_array_free(array);
}

static void
eina_bench_lookup_djb2_inline(int request)
{
   Eina_Hash *hash = NULL;
   Eina_Array *array = NULL;
   int *tmp_val;
   Eina_Array_Iterator it;
   unsigned int i;

   array = eina_array_new(1000);

   hash = eina_hash_string_djb2_new();

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];
	int length;

	tmp_val = malloc(sizeof (int));

	if (!tmp_key || !tmp_val) continue ;

	length = snprintf(tmp_key, 10, "%i", i) + 1;
	*tmp_val = i;

	eina_hash_add_by_hash(hash, tmp_key, length, eina_hash_djb2(tmp_key, length), tmp_val);

	eina_array_push(array, tmp_val);
     }

   srand(time(NULL));

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];
	int length;

	length = snprintf(tmp_key, 10, "%i", rand() % request) + 1;

	tmp_val = eina_hash_find_by_hash(hash, tmp_key, length,  eina_hash_djb2(tmp_key, length));
     }

   eina_hash_free(hash);

   EINA_ARRAY_ITER_NEXT(array, i, tmp_val, it)
     free(tmp_val);

   eina_array_free(array);
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

   hash = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, free);

   for (i = 0; i < (unsigned int) request; ++i)
     {
	elm = malloc(sizeof (Eina_Bench_Glib) + 10);
	if (!elm) continue ;

	elm->key = (char*) (elm + 1);

	snprintf(elm->key, 10, "%i", i);
	elm->value = i;

	g_hash_table_insert(hash, elm->key, elm);
     }

   srand(time(NULL));

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	snprintf(tmp_key, 10, "%i", rand() % request);

	elm = g_hash_table_lookup(hash, tmp_key);
     }

   g_hash_table_destroy(hash);
}
#endif

void eina_bench_hash(Eina_Bench *bench)
{
   eina_bench_register(bench, "superfast-lookup", EINA_BENCH(eina_bench_lookup_superfast), 1000, 180000, 2500);
   eina_bench_register(bench, "djb2-lookup", EINA_BENCH(eina_bench_lookup_djb2), 1000, 180000, 2500);
   eina_bench_register(bench, "djb2-lookup-inline", EINA_BENCH(eina_bench_lookup_djb2_inline), 1000, 180000, 2500);
#ifdef EINA_BENCH_HAVE_GLIB
   eina_bench_register(bench, "ghash-lookup", EINA_BENCH(eina_bench_lookup_ghash), 1000, 180000, 2500);
#endif
}
