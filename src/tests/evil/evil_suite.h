/* EVIL - EFL library for Windows port
 * Copyright (C) 2015 Vincent Torri
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

#ifndef EVIL_SUITE_H_
#define EVIL_SUITE_H_

#include <check.h>
#include "../efl_check.h"
void evil_test_dlfcn(TCase *tc);
/* void evil_test_fcntl(TCase *tc); */
/* void evil_test_langinfo(TCase *tc); */
void evil_test_libgen(TCase *tc);
void evil_test_main(TCase *tc);
/* void evil_test_mman(TCase *tc); */
/* void evil_test_pwd(TCase *tc); */
void evil_test_stdio(TCase *tc);
void evil_test_stdlib(TCase *tc);
/* void evil_test_string(TCase *tc); */
/* void evil_test_time(TCase *tc); */
void evil_test_unistd(TCase *tc);
/* void evil_test_util(TCase *tc); */

#endif /* EVIL_SUITE_H_ */
