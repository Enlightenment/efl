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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "eina_hash.h"
#include "eina_array.h"
#include "eina_bench.h"

static void
eina_bench_lookup_superfast(int request)
{
   Eina_Hash *hash = NULL;
   Eina_Array *array = NULL;
   int *tmp_val;
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

	eina_array_append(array, tmp_val);
     }

   srand(time(NULL));

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	snprintf(tmp_key, 10, "%i", rand() % request);

	tmp_val = eina_hash_find(hash, tmp_key);
     }

   eina_hash_free(hash);

   EINA_ARRAY_ITER_NEXT(array, i, tmp_val)
     free(tmp_val);

   eina_array_free(array);
}

static void
eina_bench_lookup_djb2(int request)
{
   Eina_Hash *hash = NULL;
   Eina_Array *array = NULL;
   int *tmp_val;
   unsigned int i;

   array = eina_array_new(1000);

   hash = eina_hash_string_djb2_new();

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char *tmp_key[10];

	tmp_val = malloc(sizeof (int));

	if (!tmp_key || !tmp_val) continue ;

	snprintf(tmp_key, 10, "%i", i);
	*tmp_val = i;

	eina_hash_add(hash, tmp_key, tmp_val);

	eina_array_append(array, tmp_val);
     }

   srand(time(NULL));

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char tmp_key[10];

	snprintf(tmp_key, 10, "%i", rand() % request);

	tmp_val = eina_hash_find(hash, tmp_key);
     }

   eina_hash_free(hash);

   EINA_ARRAY_ITER_NEXT(array, i, tmp_val)
     free(tmp_val);

   eina_array_free(array);
}

static void
eina_bench_lookup_djb2_inline(int request)
{
   Eina_Hash *hash = NULL;
   Eina_Array *array = NULL;
   int *tmp_val;
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

	eina_array_append(array, tmp_val);
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

   EINA_ARRAY_ITER_NEXT(array, i, tmp_val)
     free(tmp_val);

   eina_array_free(array);
}

void eina_bench_hash(Eina_Bench *bench)
{
   eina_bench_register(bench, "superfast-lookup", EINA_BENCH(eina_bench_lookup_superfast), 1000, 180000, 2500);
   eina_bench_register(bench, "djb2-lookup", EINA_BENCH(eina_bench_lookup_djb2), 1000, 180000, 2500);
   eina_bench_register(bench, "djb2-lookup-inline", EINA_BENCH(eina_bench_lookup_djb2_inline), 1000, 180000, 2500);
}
