#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "eina_hash.h"
#include "eina_array.h"
#include "eina_bench.h"

static void
eina_bench_insert_superfast(int request)
{
   Eina_Hash *hash = NULL;
   Eina_Array *array = NULL;
   int *tmp_val;
   unsigned int i;

   array = eina_array_new(1000);

   hash = eina_hash_string_superfast_new();

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char *tmp_key = malloc(10);

	tmp_val = malloc(sizeof (int));

	if (!tmp_key || !tmp_val) continue ;

	snprintf(tmp_key, 10, "%i", i);
	*tmp_val = i;

	eina_hash_add(hash, tmp_key, tmp_val);

	eina_array_append(array, tmp_val);

	free(tmp_key);
     }

   eina_hash_free(hash);

   EINA_ARRAY_ITER_NEXT(array, i, tmp_val)
     free(tmp_val);

   eina_array_free(array);
}

static void
eina_bench_insert_djb2(int request)
{
   Eina_Hash *hash = NULL;
   Eina_Array *array = NULL;
   int *tmp_val;
   unsigned int i;

   array = eina_array_new(1000);

   hash = eina_hash_string_djb2_new();

   for (i = 0; i < (unsigned int) request; ++i)
     {
	char *tmp_key = malloc(10);

	tmp_val = malloc(sizeof (int));

	if (!tmp_key || !tmp_val) continue ;

	snprintf(tmp_key, 10, "%i", i);
	*tmp_val = i;

	eina_hash_add(hash, tmp_key, tmp_val);

	eina_array_append(array, tmp_val);

	free(tmp_key);
     }

   eina_hash_free(hash);

   EINA_ARRAY_ITER_NEXT(array, i, tmp_val)
     free(tmp_val);

   eina_array_free(array);
}

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
	char *tmp_key = malloc(10);

	tmp_val = malloc(sizeof (int));

	if (!tmp_key || !tmp_val) continue ;

	snprintf(tmp_key, 10, "%i", i);
	*tmp_val = i;

	eina_hash_add(hash, tmp_key, tmp_val);

	eina_array_append(array, tmp_val);

	free(tmp_key);
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

void eina_bench_hash(Eina_Bench *bench)
{
   eina_bench_register(bench, "superfast-insert", EINA_BENCH(eina_bench_insert_superfast), 1000, 180000, 2500);
   eina_bench_register(bench, "djb2-insert", EINA_BENCH(eina_bench_insert_djb2), 1000, 180000, 2500);
   eina_bench_register(bench, "superfast-lookup", EINA_BENCH(eina_bench_lookup_superfast), 1000, 180000, 2500);
   eina_bench_register(bench, "djb2-lookup", EINA_BENCH(eina_bench_lookup_djb2), 1000, 180000, 2500);
}
