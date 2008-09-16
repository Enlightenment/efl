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

#ifndef EINA_BENCH_H_
#define EINA_BENCH_H_

#include "eina_benchmark.h"

void eina_bench_hash(Eina_Benchmark *bench);
void eina_bench_array(Eina_Benchmark *bench);
void eina_bench_stringshare(Eina_Benchmark *bench);
void eina_bench_convert(Eina_Benchmark *bench);

/* Specific benchmark. */
void eina_bench_e17(void);

#endif
