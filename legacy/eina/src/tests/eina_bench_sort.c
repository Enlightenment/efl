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
#include <string.h>

#ifdef EINA_BENCH_HAVE_GLIB
# include <glib.h>
#endif

#ifdef EINA_BENCH_HAVE_EVAS
# include <Evas.h>
#endif

#ifdef EINA_BENCH_HAVE_ECORE
# include <Ecore.h>
# include <Ecore_Data.h>
#endif

#include "eina_bench.h"
#include "eina_list.h"
#include "eina_convert.h"

static int
_eina_cmp_str(const char *a, const char *b)
{
   return strcmp(a, b);
}

static void
eina_bench_sort_eina(int request)
{
   Eina_List *list = NULL;
   int i;

   eina_list_init();

   srand(time(NULL));

   for (i = 0; i < request; ++i)
     {
	char buffer[10];

	eina_convert_itoa(rand() % request, buffer);

	list = eina_list_prepend(list, strdup(buffer));
     }

   list = eina_list_sort(list, -1, EINA_COMPARE_CB(_eina_cmp_str));

   while (list)
     {
	free(eina_list_data_get(list));
	list = eina_list_remove_list(list, list);
     }

   eina_list_shutdown();
}

#ifdef EINA_BENCH_HAVE_EVAS
static void
eina_bench_sort_evas(int request)
{
   Evas_List *list = NULL;
   int i;

   evas_init();

   srand(time(NULL));

   for (i = 0; i < request; ++i)
     {
	char buffer[10];

	eina_convert_itoa(rand() % request, buffer);

	list = evas_list_prepend(list, strdup(buffer));
     }

   list = evas_list_sort(list, -1, (void*) _eina_cmp_str);

   while (list)
     {
	free(evas_list_data(list));
	list = evas_list_remove_list(list, list);
     }

   evas_shutdown();
}
#endif

#ifdef EINA_BENCH_HAVE_GLIB
static void
eina_bench_sort_glist(int request)
{
   GList *list = NULL;
   int i;

   srand(time(NULL));

   for (i = 0; i < request; ++i)
     {
	char buffer[10];

	eina_convert_itoa(rand() % request, buffer);

	list = g_list_prepend(list, strdup(buffer));
     }

   list = g_list_sort(list, (void*) _eina_cmp_str);

   while (list)
     {
	free(list->data);
	list = g_list_delete_link(list, list);
     }
}
#endif

#ifdef EINA_BENCH_HAVE_ECORE
static void
eina_bench_sort_ecore_default(int request)
{
   Ecore_List *list = NULL;
   int i;

   ecore_init();
   list = ecore_list_new();
   ecore_list_free_cb_set(list, free);

   for (i = 0; i < request; ++i)
     {
	char buffer[10];

	eina_convert_itoa(rand() % request, buffer);

	ecore_list_prepend(list, strdup(buffer));
     }

   ecore_list_sort(list, ECORE_COMPARE_CB(_eina_cmp_str), 0);

   ecore_list_destroy(list);

   ecore_shutdown();
}

static void
eina_bench_sort_ecore_merge(int request)
{
   Ecore_List *list = NULL;
   int i;

   ecore_init();
   list = ecore_list_new();
   ecore_list_free_cb_set(list, free);

   for (i = 0; i < request; ++i)
     {
	char buffer[10];

	eina_convert_itoa(rand() % request, buffer);

	ecore_list_prepend(list, strdup(buffer));
     }

   ecore_list_mergesort(list, ECORE_COMPARE_CB(_eina_cmp_str), 0);

   ecore_list_destroy(list);

   ecore_shutdown();
}

static void
eina_bench_sort_ecore_heap(int request)
{
   Ecore_List *list = NULL;
   int i;

   ecore_init();
   list = ecore_list_new();
   ecore_list_free_cb_set(list, free);

   for (i = 0; i < request; ++i)
     {
	char buffer[10];

	eina_convert_itoa(rand() % request, buffer);

	ecore_list_prepend(list, strdup(buffer));
     }

   ecore_list_heapsort(list, ECORE_COMPARE_CB(_eina_cmp_str), 0);

   ecore_list_destroy(list);

   ecore_shutdown();
}
#endif

void eina_bench_sort(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "eina", EINA_BENCHMARK(eina_bench_sort_eina), 10, 10000, 100);
#ifdef EINA_BENCH_HAVE_GLIB
   eina_benchmark_register(bench, "glist", EINA_BENCHMARK(eina_bench_sort_glist), 10, 10000, 100);
#endif
#ifdef EINA_BENCH_HAVE_ECORE
   eina_benchmark_register(bench, "ecore", EINA_BENCHMARK(eina_bench_sort_ecore_default), 10, 10000, 100);
   eina_benchmark_register(bench, "ecore-merge", EINA_BENCHMARK(eina_bench_sort_ecore_merge), 10, 10000, 100);
   eina_benchmark_register(bench, "ecore-heap", EINA_BENCHMARK(eina_bench_sort_ecore_heap), 10, 10000, 100);
#endif
#ifdef EINA_BENCH_HAVE_EVAS
   eina_benchmark_register(bench, "evas", EINA_BENCHMARK(eina_bench_sort_evas), 10, 10000, 100);
#endif
}



