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
#include <limits.h>

#include "eina_bench.h"
#include "eina_inlist.h"
#include "eina_counter.h"

typedef struct _Eina_Run Eina_Run;
struct _Eina_Run
{
   Eina_Inlist __list;

   Eina_Bench_Specimens cb;
   const char *name;
   int start;
   int end;
   int step;
};

struct _Eina_Bench
{
   const char *name;
   const char *run;

   Eina_Inlist *runs;
};

Eina_Bench *
eina_bench_new(const char *name, const char *run)
{
   Eina_Bench *new;

   new = calloc(1, sizeof (Eina_Bench));
   if (!new) return NULL;

   new->name = name;
   new->run = run;

   return new;
}

void
eina_bench_delete(Eina_Bench *bench)
{
   if (!bench) return ;

   while (bench->runs)
     {
	Eina_Run *run = (Eina_Run *) bench->runs;

	bench->runs = eina_inlist_remove(bench->runs, bench->runs);
	free(run);
     }

   free(bench);
}

void
eina_bench_register(Eina_Bench *bench, const char *name, Eina_Bench_Specimens bench_cb,
		    int count_start, int count_end, int count_step)
{
   Eina_Run *run;

   if (!bench) return ;

   run = calloc(1, sizeof (Eina_Run));
   if (!run) return ;

   run->cb = bench_cb;
   run->name = name;
   run->start = count_start;
   run->end = count_end;
   run->step = count_step;

   bench->runs = eina_inlist_append(bench->runs, run);
}

void
eina_bench_run(Eina_Bench *bench)
{
   FILE *main_script;
   FILE *current_data;
   Eina_Run *run;
   char buffer[PATH_MAX];
   Eina_Bool first = EINA_FALSE;

   if (!bench) return ;

   snprintf(buffer, PATH_MAX, "bench_%s_%s.gnuplot", bench->name, bench->run);

   main_script = fopen(buffer, "w");
   if (!main_script) return ;

   fprintf(main_script,
	   "set   autoscale                        # scale axes automatically\n"
	   "unset log                              # remove any log-scaling\n"
	   "unset label                            # remove any previous labels\n"
	   "set xtic auto                          # set xtics automatically\n"
	   "set ytic auto                          # set ytics automatically\n"
/* 	   "set logscale y\n" */
	   "set terminal png size 1024,768\n"
	   "set output \"output_%s_%s.png\"\n"
	   "set title \"%s %s\n"
	   "set xlabel \"tests\"\n"
	   "set ylabel \"time\"\n"
	   "plot ", bench->name, bench->run, bench->name, bench->run);

   eina_counter_init();

   EINA_INLIST_ITER_NEXT(bench->runs, run)
     {
	Eina_Counter *counter;
	int i;

	snprintf(buffer, PATH_MAX, "bench_%s_%s.%s.data", bench->name, bench->run, run->name);

	current_data = fopen(buffer, "w");
	if (!current_data) continue ;

	counter = eina_counter_add(run->name);

	for (i = run->start; i < run->end; i += run->step)
	  {
	     fprintf(stderr, "Run %s: %i\n", run->name, i);
	     eina_counter_start(counter);

	     run->cb(i);

	     eina_counter_stop(counter, i);
	  }

	eina_counter_dump(counter, current_data);

	eina_counter_delete(counter);

	fclose(current_data);

	if (first == EINA_FALSE) first = EINA_TRUE;
	else fprintf(main_script, ", \\\n");

	fprintf(main_script,
		"\"%s\" using 1:2 title \'%s\' with line",
		buffer, run->name);
     }

   fprintf(main_script, "\n");

   eina_counter_shutdown();

   fclose(main_script);
}

typedef struct _Eina_Bench_Case Eina_Bench_Case;
struct _Eina_Bench_Case
{
   const char *bench_case;
   void (*build)(Eina_Bench *bench);
};

static const Eina_Bench_Case etc[] = {
  { "Hash", eina_bench_hash },
  { "Array vs List vs Inlist", eina_bench_array },
  { "Stringshare", eina_bench_stringshare },
  { "Convert", eina_bench_convert },
  { NULL, NULL }
};

int
main(int argc, char **argv)
{
   Eina_Bench *test;
   int i;

   if (argc != 2) return -1;

   for (i = 0; etc[i].bench_case != NULL; ++i)
     {
	test = eina_bench_new(etc[i].bench_case, argv[1]);
	if (!test) continue ;

	etc[i].build(test);

	eina_bench_run(test);

	eina_bench_delete(test);
     }

   return 0;
}
