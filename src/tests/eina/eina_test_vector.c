/* EINA - EFL data type library
 * Copyright (C) 2016 Sergey Osadchy
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

#include <math.h>
#include <float.h>
#include <limits.h>

#include "eina_suite.h"
#include "Eina.h"

START_TEST(eina_test_vector_operations)
{
   Eina_Vector2 out;;
   double x = 1;
   double y = 2;

   eina_init();

   eina_vector2_set(&out, x, y);
   fail_if((out.x != 1) || (out.y != 2));

   eina_shutdown();
}
END_TEST

void
eina_test_vector(TCase *tc)
{
   tcase_add_test(tc, eina_test_vector_operations);
}
