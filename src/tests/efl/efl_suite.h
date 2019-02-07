/* EFL - EFL library
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

#ifndef EFL_SUITE_H_
#define EFL_SUITE_H_

#include <check.h>
#include "../efl_check.h"
void efl_test_case_container_model(TCase *tc);
void efl_test_case_view_model(TCase *tc);
void efl_test_case_boolean_model(TCase *tc);

#endif /* EFL_SUITE_H_ */
