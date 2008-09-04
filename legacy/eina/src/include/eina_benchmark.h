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

#ifndef EINA_BENCHMARK_H_
#define EINA_BENCHMARK_H_

#include "eina_array.h"

typedef struct _Eina_Benchmark Eina_Benchmark;
typedef void (*Eina_Benchmark_Specimens)(int request);
#define EINA_BENCHMARK(Function) ((Eina_Benchmark_Specimens)Function)

EAPI Eina_Benchmark *eina_benchmark_new(const char *name, const char *run);
EAPI void eina_benchmark_free(Eina_Benchmark *bench);

EAPI void eina_benchmark_register(Eina_Benchmark *bench, const char *name, Eina_Benchmark_Specimens bench_cb,
				  int count_start, int count_end, int count_set);

EAPI Eina_Array *eina_benchmark_run(Eina_Benchmark *bench);

#endif
