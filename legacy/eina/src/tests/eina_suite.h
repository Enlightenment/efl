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

#ifndef EINA_SUITE_H_
#define EINA_SUITE_H_

#include <check.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

void eina_test_stringshare(TCase *tc);
void eina_test_array(TCase *tc);
void eina_test_error(TCase *tc);
void eina_test_magic(TCase *tc);
void eina_test_inlist(TCase *tc);
void eina_test_lalloc(TCase *tc);
void eina_test_main(TCase *tc);
void eina_test_counter(TCase *tc);
void eina_test_hash(TCase *tc);
void eina_test_list(TCase *tc);
void eina_test_iterator(TCase *tc);
void eina_test_accessor(TCase *tc);
void eina_test_module(TCase *tc);
void eina_test_convert(TCase *tc);
void eina_test_rbtree(TCase *tc);
void eina_test_file(TCase *tc);
void eina_test_benchmark(TCase *tc);
void eina_test_mempool(TCase *tc);

#endif /* EINA_SUITE_H_ */
