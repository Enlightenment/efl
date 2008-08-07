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

typedef struct _Eina_Bench Eina_Bench;
typedef void (*Eina_Bench_Specimens)(int request);
#define EINA_BENCH(Function) ((Eina_Bench_Specimens)Function)

void eina_bench_register(Eina_Bench *bench, const char *name, Eina_Bench_Specimens bench_cb,
			 int count_start, int count_end, int count_set);

void eina_bench_hash(Eina_Bench *bench);
void eina_bench_array(Eina_Bench *bench);

#endif
