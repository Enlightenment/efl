/* EINA - EFL data type library
 * Copyright (C) 2017 Cedric Bail
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

EINA_TEST_START(eina_util_fpx)
{
   double a = 42.42, b = 42.41;
   float c = 7.654, d = 8.765;

   ck_assert_int_eq(eina_dbl_exact(a, b), EINA_FALSE);
   ck_assert_int_eq(eina_dbl_exact(a, a), EINA_TRUE);
   ck_assert_int_eq(eina_flt_exact(c, d), EINA_FALSE);
   ck_assert_int_eq(eina_flt_exact(c, c), EINA_TRUE);
   ck_assert_int_eq(eina_dbl_exact(a, c), EINA_FALSE);
}
EINA_TEST_END

EINA_TEST_START(eina_util_fpeq)
{
   double a, b, c;
   double d, e, f;

   a = 6.0 * 7.0 + 7.0;
   b = 7.0 * 7.0;
   c = a - 0.1;

   d = 6.0 * 7.0 + 7.0;
   e = 7.0 * 7.0;
   f = a - 0.1;

   ck_assert_int_ne(EINA_DBL_EQ(a, b), 0);
   ck_assert_int_eq(EINA_DBL_EQ(a, c), 0);
   ck_assert_int_ne(EINA_DBL_EQ(d, e), 0);
   ck_assert_int_eq(EINA_DBL_EQ(d, f), 0);
}
EINA_TEST_END

EINA_TEST_START(eina_util_fpz)
{
   ck_assert_int_eq(EINA_FLT_NONZERO(7.0 - 7.0), 0);
   ck_assert_int_ne(EINA_FLT_NONZERO(7.1 - 7.0), 0);
   ck_assert_int_eq(EINA_FLT_NONZERO(0.0), 0);
   ck_assert_int_eq(EINA_DBL_NONZERO(7.0 - 7.0), 0);
   ck_assert_int_ne(EINA_DBL_NONZERO(7.1 - 7.0), 0);
   ck_assert_int_eq(EINA_DBL_NONZERO(0.0), 0);
}
EINA_TEST_END
