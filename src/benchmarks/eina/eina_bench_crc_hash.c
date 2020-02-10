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

#include <Eina.h>
#include "Evas_Data.h"
#include "Ecore_Data.h"

#include "eina_hash.h"
#include "eina_array.h"
#include "eina_bench.h"
#include "eina_rbtree.h"
#include "eina_convert.h"

#ifdef CITYHASH_BENCH
// Hash function for a byte array.
uint64_t CityHash64(const char *buf, size_t len);
#endif

int key_size;
char *key_str=NULL;

void repchar(int n)
{
   key_str = (char *)malloc(n);
   int i;

   for (i = 0; i < n; i++)
      key_str[i] = 'a';
}

static void
eina_bench_murmur_hash(int request)
{
   unsigned int i;

   for (i = 0; i < (unsigned int)request; ++i)
     {
        char tmp_key[key_size];

        eina_convert_itoa(i, tmp_key);
        eina_strlcat(tmp_key, key_str, key_size);

        eina_hash_murmur3(tmp_key, key_size);
     }
}

#ifdef CITYHASH_BENCH
static void
eina_bench_cityhash(int request)
{
   unsigned int i;

   for (i = 0; i < (unsigned int)request; ++i)
     {
        char tmp_key[key_size];

        eina_convert_itoa(i, tmp_key);
        eina_strlcat(tmp_key, key_str, key_size);

        CityHash64(tmp_key, key_size);
     }
}
#endif

static void
eina_bench_superfast_hash(int request)
{
   unsigned int i;

   for (i = 0; i < (unsigned int)request; ++i)
     {
        char tmp_key[key_size];

        eina_convert_itoa(i, tmp_key);
        eina_strlcat(tmp_key, key_str, key_size);

        eina_hash_superfast(tmp_key, key_size);
     }
}

static void
eina_bench_crchash(int request)
{
   unsigned int i;

   for (i = 0; i < (unsigned int)request; ++i)
     {
        char tmp_key[key_size];

        eina_convert_itoa(i, tmp_key);
        eina_strlcat(tmp_key, key_str, key_size);

        eina_hash_crc(tmp_key, key_size);
     }
}

static void
eina_bench_djb2_hash(int request)
{
   unsigned int i;

   for (i = 0; i < (unsigned int)request; ++i)
     {
        char tmp_key[key_size];

        eina_convert_itoa(i, tmp_key);
        eina_strlcat(tmp_key, key_str, key_size);

        eina_hash_djb2(tmp_key, key_size);
     }
}

#ifdef EINA_BENCH_HAVE_GLIB
static void
eina_bench_ghash(int request)
{
   unsigned int i;

   for (i = 0; i < (unsigned int)request; ++i)
     {
        char tmp_key[key_size];

        eina_convert_itoa(i, tmp_key);
        eina_strlcat(tmp_key, key_str, key_size);

        g_str_hash(key_str);
     }
}
#endif

int
evas_hash_gen(const char *key)
{
   unsigned int hash_num = 5381;
   const unsigned char *ptr;

   if (!key)
      return 0;

   for (ptr = (unsigned char *)key; *ptr; ptr++)
      hash_num = (hash_num * 33) ^ *ptr;

   hash_num &= 0xff;
   return (int)hash_num;
}

static void
eina_bench_evas_hash(int request)
{
   unsigned int i;

   for (i = 0; i < (unsigned int)request; ++i)
     {
        char tmp_key[key_size];

        eina_convert_itoa(i, tmp_key);
        eina_strlcat(tmp_key, key_str, key_size);

        evas_hash_gen(tmp_key);
     }
}

typedef struct _Eina_Bench_Ecore Eina_Bench_Ecore;
struct _Eina_Bench_Ecore
{
   char *key;
   int value;
};

void eina_bench_crc_hash_short(Eina_Benchmark *bench)
{
   key_size = 8; /* Length of string for small strings and pointers */
   key_size -= 5;
   repchar(key_size);

   eina_benchmark_register(bench, "superfast-lookup",
                           EINA_BENCHMARK(
                              eina_bench_superfast_hash),   10, 80000, 10);
   eina_benchmark_register(bench, "djb2-lookup",
                           EINA_BENCHMARK(
                             eina_bench_djb2_hash),        10, 80000, 10);
   eina_benchmark_register(bench, "murmur",
                           EINA_BENCHMARK(
                              eina_bench_murmur_hash),      10, 80000, 10);
   eina_benchmark_register(bench, "crchash",
                           EINA_BENCHMARK(
			      eina_bench_crchash),         10, 80000, 10);
#ifdef CITYHASH_BENCH
   eina_benchmark_register(bench, "cityhash",
                           EINA_BENCHMARK(
                              eina_bench_cityhash),    10, 80000, 10);
#endif

#ifdef EINA_BENCH_HAVE_GLIB
   eina_benchmark_register(bench, "ghash-lookup",
                           EINA_BENCHMARK(
                              eina_bench_ghash),       10, 80000, 10);
#endif
   eina_benchmark_register(bench, "evas-lookup",
                           EINA_BENCHMARK(
                              eina_bench_evas_hash),        10, 80000, 10);
}

void eina_bench_crc_hash_medium(Eina_Benchmark *bench)
{
   key_size = 32; /* Length of medium sized string, normally for filenames */
   key_size -= 5;
   repchar(key_size);

   eina_benchmark_register(bench, "superfast-lookup",
                           EINA_BENCHMARK(
                              eina_bench_superfast_hash),   10, 80000, 10);
   eina_benchmark_register(bench, "djb2-lookup",
                           EINA_BENCHMARK(
                             eina_bench_djb2_hash),        10, 80000, 10);
   eina_benchmark_register(bench, "murmur",
                           EINA_BENCHMARK(
                              eina_bench_murmur_hash),      10, 80000, 10);
   eina_benchmark_register(bench, "crchash",
                           EINA_BENCHMARK(
			      eina_bench_crchash),         10, 80000, 10);
#ifdef CITYHASH_BENCH
   eina_benchmark_register(bench, "cityhash",
                           EINA_BENCHMARK(
                              eina_bench_cityhash),    10, 80000, 10);
#endif

#ifdef EINA_BENCH_HAVE_GLIB
   eina_benchmark_register(bench, "ghash-lookup",
                           EINA_BENCHMARK(
                              eina_bench_ghash),       10, 80000, 10);
#endif
   eina_benchmark_register(bench, "evas-lookup",
                           EINA_BENCHMARK(
                              eina_bench_evas_hash),        10, 80000, 10);
}

void eina_bench_crc_hash_large(Eina_Benchmark *bench)
{
   key_size = 256; /* Length of large strings, normally for filepath */
   key_size -= 5;
   repchar(key_size);

   eina_benchmark_register(bench, "superfast-lookup",
                           EINA_BENCHMARK(
                              eina_bench_superfast_hash),   10, 80000, 10);
   eina_benchmark_register(bench, "djb2-lookup",
                           EINA_BENCHMARK(
                             eina_bench_djb2_hash),        10, 80000, 10);
   eina_benchmark_register(bench, "murmur",
                           EINA_BENCHMARK(
                              eina_bench_murmur_hash),      10, 80000, 10);
   eina_benchmark_register(bench, "crchash",
                           EINA_BENCHMARK(
			      eina_bench_crchash),         10, 80000, 10);
#ifdef CITYHASH_BENCH
   eina_benchmark_register(bench, "cityhash",
                           EINA_BENCHMARK(
                              eina_bench_cityhash),    10, 80000, 10);
#endif

#ifdef EINA_BENCH_HAVE_GLIB
   eina_benchmark_register(bench, "ghash-lookup",
                           EINA_BENCHMARK(
                              eina_bench_ghash),       10, 80000, 10);
#endif
   eina_benchmark_register(bench, "evas-lookup",
                           EINA_BENCHMARK(
                              eina_bench_evas_hash),        10, 80000, 10);
}
