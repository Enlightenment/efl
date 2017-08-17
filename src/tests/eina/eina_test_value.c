/* EINA - EFL data type library
 * Copyright (C) 2012 ProFUSION embedded systems
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

#include <stdio.h>
#include <inttypes.h>
#include <float.h>

#include <Eina.h>

#include "eina_suite.h"

#define FP_ERR (1)
#define CHECK_FP(a, b) ((a - b) <= FP_ERR)

START_TEST(eina_value_test_simple)
{
   Eina_Value *value;
   char c;
   short s;
   int i;
   long l;
   int64_t i64;
   unsigned char uc;
   unsigned short us;
   unsigned int ui;
   unsigned long ul;
   uint64_t u64;
   float f;
   double d;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_CHAR);
   fail_unless(value != NULL);
   fail_unless(eina_value_set(value, 'x'));
   fail_unless(eina_value_get(value, &c));
   fail_unless(c == 'x');
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_SHORT));
   fail_unless(eina_value_set(value, 300));
   fail_unless(eina_value_get(value, &s));
   fail_unless(s == 300);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_set(value, -12345));
   fail_unless(eina_value_get(value, &i));
   fail_unless(i == -12345);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_LONG));
   fail_unless(eina_value_set(value, 0xb33f));
   fail_unless(eina_value_get(value, &l));
   fail_unless(l == 0xb33f);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_INT64));
   fail_unless(eina_value_set(value, 0x0011223344556677));
   fail_unless(eina_value_get(value, &i64));
   fail_unless(i64 == 0x0011223344556677);
   eina_value_flush(value);

   /* unsigned: */

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_UCHAR));
   fail_unless(eina_value_set(value, 200));
   fail_unless(eina_value_get(value, &uc));
   fail_unless(uc == 200);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_USHORT));
   fail_unless(eina_value_set(value, 65535));
   fail_unless(eina_value_get(value, &us));
   fail_unless(us == 65535);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_set(value, 4000000000U));
   fail_unless(eina_value_get(value, &ui));
   fail_unless(ui == 4000000000U);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_set(value, 3000000001UL));
   fail_unless(eina_value_get(value, &ul));
   fail_unless(ul == 3000000001UL);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_set(value, 0x1122334455667788));
   fail_unless(eina_value_get(value, &u64));
   fail_unless(u64 == 0x1122334455667788);
   eina_value_flush(value);

   /* floating point */
   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_set(value, 0.1234));
   fail_unless(eina_value_get(value, &f));
   fail_unless(CHECK_FP(0.1234, f));
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_set(value, 34567.8));
   fail_unless(eina_value_get(value, &d));
   fail_unless(CHECK_FP(34567.8, d));
   eina_value_flush(value);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_compare)
{
   Eina_Value *a, *b;

   eina_init();

   a = eina_value_new(EINA_VALUE_TYPE_CHAR);
   fail_unless(a != NULL);
   b = eina_value_new(EINA_VALUE_TYPE_CHAR);
   fail_unless(b != NULL);

   fail_unless(eina_value_set(a, 123));
   fail_unless(eina_value_set(b, 123));
   fail_unless(eina_value_compare(a, b) == 0);
   fail_unless(eina_value_set(a, -10));
   fail_unless(eina_value_set(b, 123));
   fail_unless(eina_value_compare(a, b) < 0);
   fail_unless(eina_value_set(a, 123));
   fail_unless(eina_value_set(b, 10));
   fail_unless(eina_value_compare(a, b) > 0);
   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_setup(a, EINA_VALUE_TYPE_SHORT));
   fail_unless(eina_value_setup(b, EINA_VALUE_TYPE_SHORT));
   fail_unless(eina_value_set(a, 1230));
   fail_unless(eina_value_set(b, 1230));
   fail_unless(eina_value_compare(a, b) == 0);
   fail_unless(eina_value_set(a, -100));
   fail_unless(eina_value_set(b, 1230));
   fail_unless(eina_value_compare(a, b) < 0);
   fail_unless(eina_value_set(a, 1230));
   fail_unless(eina_value_set(b, -100));
   fail_unless(eina_value_compare(a, b) > 0);
   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_setup(a, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_setup(b, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_set(a, 300000));
   fail_unless(eina_value_set(b, 300000));
   fail_unless(eina_value_compare(a, b) == 0);
   fail_unless(eina_value_set(a, -100));
   fail_unless(eina_value_set(b, 300000));
   fail_unless(eina_value_compare(a, b) < 0);
   fail_unless(eina_value_set(a, 300000));
   fail_unless(eina_value_set(b, -100));
   fail_unless(eina_value_compare(a, b) > 0);
   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_setup(a, EINA_VALUE_TYPE_LONG));
   fail_unless(eina_value_setup(b, EINA_VALUE_TYPE_LONG));
   fail_unless(eina_value_set(a, 300000L));
   fail_unless(eina_value_set(b, 300000L));
   fail_unless(eina_value_compare(a, b) == 0);
   fail_unless(eina_value_set(a, -100L));
   fail_unless(eina_value_set(b, 300000L));
   fail_unless(eina_value_compare(a, b) < 0);
   fail_unless(eina_value_set(a, 300000L));
   fail_unless(eina_value_set(b, -100L));
   fail_unless(eina_value_compare(a, b) > 0);
   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_setup(a, EINA_VALUE_TYPE_INT64));
   fail_unless(eina_value_setup(b, EINA_VALUE_TYPE_INT64));
   fail_unless(eina_value_set(a, (int64_t)800000));
   fail_unless(eina_value_set(b, (int64_t)800000));
   fail_unless(eina_value_compare(a, b) == 0);
   fail_unless(eina_value_set(a, (int64_t)-100));
   fail_unless(eina_value_set(b, (int64_t)8000000));
   fail_unless(eina_value_compare(a, b) < 0);
   fail_unless(eina_value_set(a, (int64_t)8000000));
   fail_unless(eina_value_set(b, (int64_t)-100));
   fail_unless(eina_value_compare(a, b) > 0);
   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_setup(a, EINA_VALUE_TYPE_UCHAR));
   fail_unless(eina_value_setup(b, EINA_VALUE_TYPE_UCHAR));
   fail_unless(eina_value_set(a, 123));
   fail_unless(eina_value_set(b, 123));
   fail_unless(eina_value_compare(a, b) == 0);
   fail_unless(eina_value_set(a, 10));
   fail_unless(eina_value_set(b, 123));
   fail_unless(eina_value_compare(a, b) < 0);
   fail_unless(eina_value_set(a, 123));
   fail_unless(eina_value_set(b, 10));
   fail_unless(eina_value_compare(a, b) > 0);
   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_setup(a, EINA_VALUE_TYPE_USHORT));
   fail_unless(eina_value_setup(b, EINA_VALUE_TYPE_USHORT));
   fail_unless(eina_value_set(a, 1230));
   fail_unless(eina_value_set(b, 1230));
   fail_unless(eina_value_compare(a, b) == 0);
   fail_unless(eina_value_set(a, 100));
   fail_unless(eina_value_set(b, 1230));
   fail_unless(eina_value_compare(a, b) < 0);
   fail_unless(eina_value_set(a, 1230));
   fail_unless(eina_value_set(b, 100));
   fail_unless(eina_value_compare(a, b) > 0);
   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_setup(a, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_setup(b, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_set(a, 300000));
   fail_unless(eina_value_set(b, 300000));
   fail_unless(eina_value_compare(a, b) == 0);
   fail_unless(eina_value_set(a, 100));
   fail_unless(eina_value_set(b, 300000));
   fail_unless(eina_value_compare(a, b) < 0);
   fail_unless(eina_value_set(a, 300000));
   fail_unless(eina_value_set(b, 100));
   fail_unless(eina_value_compare(a, b) > 0);
   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_setup(a, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_setup(b, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_set(a, 300000UL));
   fail_unless(eina_value_set(b, 300000UL));
   fail_unless(eina_value_compare(a, b) == 0);
   fail_unless(eina_value_set(a, 100UL));
   fail_unless(eina_value_set(b, 300000UL));
   fail_unless(eina_value_compare(a, b) < 0);
   fail_unless(eina_value_set(a, 300000UL));
   fail_unless(eina_value_set(b, 100UL));
   fail_unless(eina_value_compare(a, b) > 0);
   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_setup(a, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_setup(b, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_set(a, (uint64_t)8000000));
   fail_unless(eina_value_set(b, (uint64_t)8000000));
   fail_unless(eina_value_compare(a, b) == 0);
   fail_unless(eina_value_set(a, (uint64_t)100));
   fail_unless(eina_value_set(b, (uint64_t)8000000));
   fail_unless(eina_value_compare(a, b) < 0);
   fail_unless(eina_value_set(a, (uint64_t)8000000));
   fail_unless(eina_value_set(b, (uint64_t)100));
   fail_unless(eina_value_compare(a, b) > 0);
   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_setup(a, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_setup(b, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_set(a, "aaa"));
   fail_unless(eina_value_set(b, "aaa"));
   fail_unless(eina_value_compare(a, b) == 0);
   fail_unless(eina_value_set(a, "abc"));
   fail_unless(eina_value_set(b, "acd"));
   fail_unless(eina_value_compare(a, b) < 0);
   fail_unless(eina_value_set(a, "acd"));
   fail_unless(eina_value_set(b, "abc"));
   fail_unless(eina_value_compare(a, b) > 0);
   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_array_setup(a, EINA_VALUE_TYPE_CHAR, 0));
   fail_unless(eina_value_array_setup(b, EINA_VALUE_TYPE_CHAR, 0));
   fail_unless(eina_value_compare(a, b) == 0);

   fail_unless(eina_value_array_append(a, 1));
   fail_unless(eina_value_array_append(a, 2));
   fail_unless(eina_value_array_append(a, 3));

   fail_unless(eina_value_array_append(b, 1));
   fail_unless(eina_value_array_append(b, 2));
   fail_unless(eina_value_array_append(b, 3));

   fail_unless(eina_value_compare(a, b) == 0);

   fail_unless(eina_value_array_set(a, 0, 0));
   fail_unless(eina_value_compare(a, b) < 0);

   fail_unless(eina_value_array_set(a, 0, 10));
   fail_unless(eina_value_compare(a, b) > 0);

   fail_unless(eina_value_array_set(a, 0, 1));

   fail_unless(eina_value_array_set(b, 0, 0));
   fail_unless(eina_value_compare(a, b) > 0);

   fail_unless(eina_value_array_set(b, 0, 10));
   fail_unless(eina_value_compare(a, b) < 0);

   fail_unless(eina_value_array_set(b, 0, 1));
   fail_unless(eina_value_compare(a, b) == 0);

   /* bigger arrays are greater */
   fail_unless(eina_value_array_append(b, 0));
   fail_unless(eina_value_compare(a, b) < 0);

   fail_unless(eina_value_array_append(a, 0));
   fail_unless(eina_value_array_append(a, 0));
   fail_unless(eina_value_compare(a, b) > 0);

   /* bigger arrays are greater, unless an element says otherwise */
   fail_unless(eina_value_array_set(b, 0, 10));
   fail_unless(eina_value_compare(a, b) < 0);

   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_list_setup(a, EINA_VALUE_TYPE_CHAR));
   fail_unless(eina_value_list_setup(b, EINA_VALUE_TYPE_CHAR));
   fail_unless(eina_value_compare(a, b) == 0);

   fail_unless(eina_value_list_append(a, 1));
   fail_unless(eina_value_list_append(a, 2));
   fail_unless(eina_value_list_append(a, 3));

   fail_unless(eina_value_list_append(b, 1));
   fail_unless(eina_value_list_append(b, 2));
   fail_unless(eina_value_list_append(b, 3));

   fail_unless(eina_value_compare(a, b) == 0);

   fail_unless(eina_value_list_set(a, 0, 0));
   fail_unless(eina_value_compare(a, b) < 0);

   fail_unless(eina_value_list_set(a, 0, 10));
   fail_unless(eina_value_compare(a, b) > 0);

   fail_unless(eina_value_list_set(a, 0, 1));

   fail_unless(eina_value_list_set(b, 0, 0));
   fail_unless(eina_value_compare(a, b) > 0);

   fail_unless(eina_value_list_set(b, 0, 10));
   fail_unless(eina_value_compare(a, b) < 0);

   fail_unless(eina_value_list_set(b, 0, 1));
   fail_unless(eina_value_compare(a, b) == 0);

   /* bigger lists are greater */
   fail_unless(eina_value_list_append(b, 0));
   fail_unless(eina_value_compare(a, b) < 0);

   fail_unless(eina_value_list_append(a, 0));
   fail_unless(eina_value_list_append(a, 0));
   fail_unless(eina_value_compare(a, b) > 0);

   /* bigger lists are greater, unless an element says otherwise */
   fail_unless(eina_value_list_set(b, 0, 10));
   fail_unless(eina_value_compare(a, b) < 0);

   eina_value_flush(a);
   eina_value_flush(b);

   fail_unless(eina_value_hash_setup(a, EINA_VALUE_TYPE_CHAR, 0));
   fail_unless(eina_value_hash_setup(b, EINA_VALUE_TYPE_CHAR, 0));
   fail_unless(eina_value_compare(a, b) == 0);

   fail_unless(eina_value_hash_set(a, "abc", 1));
   fail_unless(eina_value_hash_set(a, "xyz", 2));
   fail_unless(eina_value_hash_set(a, "hello", 3));

   fail_unless(eina_value_hash_set(b, "abc", 1));
   fail_unless(eina_value_hash_set(b, "xyz", 2));
   fail_unless(eina_value_hash_set(b, "hello", 3));

   fail_unless(eina_value_compare(a, b) == 0);

   fail_unless(eina_value_hash_set(a, "abc", 0));
   fail_unless(eina_value_compare(a, b) < 0);

   fail_unless(eina_value_hash_set(a, "abc", 10));
   fail_unless(eina_value_compare(a, b) > 0);

   fail_unless(eina_value_hash_set(a, "abc", 1));

   fail_unless(eina_value_hash_set(b, "abc", 0));
   fail_unless(eina_value_compare(a, b) > 0);

   fail_unless(eina_value_hash_set(b, "abc", 10));
   fail_unless(eina_value_compare(a, b) < 0);

   fail_unless(eina_value_hash_set(b, "abc", 1));
   fail_unless(eina_value_compare(a, b) == 0);

   /* bigger hashs are greater */
   fail_unless(eina_value_hash_set(b,"newkey", 0));
   fail_unless(eina_value_compare(a, b) < 0);

   fail_unless(eina_value_hash_set(a, "newkey", 0));
   fail_unless(eina_value_hash_set(a, "onemorenewkey", 0));
   fail_unless(eina_value_compare(a, b) > 0);

   /* bigger hashs are greater, unless an element says otherwise */
   fail_unless(eina_value_hash_set(b, "abc", 10));
   fail_unless(eina_value_compare(a, b) < 0);

   eina_value_free(a);
   eina_value_free(b);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_string)
{
   Eina_Value *value;
   const char *s;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_STRING);
   fail_unless(value != NULL);
   fail_unless(eina_value_set(value, "hello world!"));
   fail_unless(eina_value_get(value, &s));
   ck_assert_str_eq(s, "hello world!");

   fail_unless(eina_value_set(value, "eina-value"));
   fail_unless(eina_value_get(value, &s));
   ck_assert_str_eq(s, "eina-value");

   eina_value_flush(value);
   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_STRING));

   fail_unless(eina_value_set(value, "profusion"));
   fail_unless(eina_value_get(value, &s));
   ck_assert_str_eq(s, "profusion");

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_pvariant)
{
   Eina_Value *value;
   char c, in_c;
   short s, in_s;
   int i, in_i;
   long l, in_l;
   int64_t i64, in_i64;
   unsigned char uc, in_uc;
   unsigned short us, in_us;
   unsigned int ui, in_ui;
   unsigned long ul, in_ul;
   uint64_t u64, in_u64;
   float f, in_f;
   double d, in_d;
   const char *str, *in_str;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_CHAR);
   fail_unless(value != NULL);
   in_c = 'x';
   fail_unless(eina_value_pset(value, &in_c));
   fail_unless(eina_value_pget(value, &c));
   fail_unless(c == 'x');
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_SHORT));
   in_s = 300;
   fail_unless(eina_value_pset(value, &in_s));
   fail_unless(eina_value_pget(value, &s));
   fail_unless(s == 300);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_INT));
   in_i = -12345;
   fail_unless(eina_value_pset(value, &in_i));
   fail_unless(eina_value_pget(value, &i));
   fail_unless(i == -12345);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_LONG));
   in_l = 0xb33f;
   fail_unless(eina_value_pset(value, &in_l));
   fail_unless(eina_value_pget(value, &l));
   fail_unless(l == 0xb33f);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_INT64));
   in_i64 = 0x0011223344556677;
   fail_unless(eina_value_pset(value, &in_i64));
   fail_unless(eina_value_pget(value, &i64));
   fail_unless(i64 == 0x0011223344556677);
   eina_value_flush(value);

   /* unsigned: */

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_UCHAR));
   in_uc = 200;
   fail_unless(eina_value_pset(value, &in_uc));
   fail_unless(eina_value_pget(value, &uc));
   fail_unless(uc == 200);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_USHORT));
   in_us = 65535;
   fail_unless(eina_value_pset(value, &in_us));
   fail_unless(eina_value_pget(value, &us));
   fail_unless(us == 65535);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_UINT));
   in_ui = 4000000000U;
   fail_unless(eina_value_pset(value, &in_ui));
   fail_unless(eina_value_pget(value, &ui));
   fail_unless(ui == 4000000000U);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_ULONG));
   in_ul = 3000000001UL;
   fail_unless(eina_value_pset(value, &in_ul));
   fail_unless(eina_value_pget(value, &ul));
   fail_unless(ul == 3000000001UL);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_UINT64));
   in_u64 = 0x1122334455667788;
   fail_unless(eina_value_pset(value, &in_u64));
   fail_unless(eina_value_pget(value, &u64));
   fail_unless(u64 == 0x1122334455667788);
   eina_value_flush(value);

   /* floating point */
   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_FLOAT));
   in_f = 0.1234;
   fail_unless(eina_value_pset(value, &in_f));
   fail_unless(eina_value_pget(value, &f));
   fail_unless(CHECK_FP(0.1234, f));
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_DOUBLE));
   in_d = 34567.8;
   fail_unless(eina_value_pset(value, &in_d));
   fail_unless(eina_value_pget(value, &d));
   fail_unless(CHECK_FP(34567.8, d));
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_STRING));
   in_str = "hello world!";
   fail_unless(eina_value_pset(value, &in_str));
   fail_unless(eina_value_pget(value, &str));
   ck_assert_str_eq(str, "hello world!");

   in_str = "eina-value";
   fail_unless(eina_value_pset(value, &in_str));
   fail_unless(eina_value_pget(value, &str));
   ck_assert_str_eq(str, "eina-value");

   eina_value_flush(value);
   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_STRING));

   in_str = "profusion";
   fail_unless(eina_value_pset(value, &in_str));
   fail_unless(eina_value_pget(value, &str));
   ck_assert_str_eq(str, "profusion");

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_to_string)
{
   Eina_Value *value;
   char c, in_c;
   short s, in_s;
   int i, in_i;
   long l, in_l;
   int64_t i64, in_i64;
   unsigned char uc, in_uc;
   unsigned short us, in_us;
   unsigned int ui, in_ui;
   unsigned long ul, in_ul;
   uint64_t u64, in_u64;
   float f, in_f;
   double d, in_d;
   const char *str, *in_str;
   char *out;
   char buf[256];

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_CHAR);
   fail_unless(value != NULL);
   in_c = 'x';
   fail_unless(eina_value_pset(value, &in_c));
   fail_unless(eina_value_pget(value, &c));
   fail_unless(c == 'x');
   snprintf(buf, sizeof(buf), "%hhd", in_c);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(buf, out);
   free(out);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_SHORT));
   in_s = 300;
   fail_unless(eina_value_pset(value, &in_s));
   fail_unless(eina_value_pget(value, &s));
   fail_unless(s == 300);
   snprintf(buf, sizeof(buf), "%hd", in_s);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(buf, out);
   free(out);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_INT));
   in_i = -12345;
   fail_unless(eina_value_pset(value, &in_i));
   fail_unless(eina_value_pget(value, &i));
   fail_unless(i == -12345);
   snprintf(buf, sizeof(buf), "%d", in_i);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(buf, out);
   free(out);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_LONG));
   in_l = 0xb33f;
   fail_unless(eina_value_pset(value, &in_l));
   fail_unless(eina_value_pget(value, &l));
   fail_unless(l == 0xb33f);
   snprintf(buf, sizeof(buf), "%ld", in_l);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(buf, out);
   free(out);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_INT64));
   in_i64 = 0x0011223344556677;
   fail_unless(eina_value_pset(value, &in_i64));
   fail_unless(eina_value_pget(value, &i64));
   fail_unless(i64 == 0x0011223344556677);
   snprintf(buf, sizeof(buf), "%"PRId64, in_i64);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(buf, out);
   free(out);
   eina_value_flush(value);

   /* unsigned: */

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_UCHAR));
   in_uc = 200;
   fail_unless(eina_value_pset(value, &in_uc));
   fail_unless(eina_value_pget(value, &uc));
   fail_unless(uc == 200);
   snprintf(buf, sizeof(buf), "%hhu", in_uc);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(buf, out);
   free(out);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_USHORT));
   in_us = 65535;
   fail_unless(eina_value_pset(value, &in_us));
   fail_unless(eina_value_pget(value, &us));
   fail_unless(us == 65535);
   snprintf(buf, sizeof(buf), "%hu", in_us);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(buf, out);
   free(out);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_UINT));
   in_ui = 4000000000U;
   fail_unless(eina_value_pset(value, &in_ui));
   fail_unless(eina_value_pget(value, &ui));
   fail_unless(ui == 4000000000U);
   snprintf(buf, sizeof(buf), "%u", in_ui);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(buf, out);
   free(out);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_ULONG));
   in_ul = 3000000001UL;
   fail_unless(eina_value_pset(value, &in_ul));
   fail_unless(eina_value_pget(value, &ul));
   fail_unless(ul == 3000000001UL);
   snprintf(buf, sizeof(buf), "%lu", in_ul);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(buf, out);
   free(out);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_UINT64));
   in_u64 = 0x1122334455667788;
   fail_unless(eina_value_pset(value, &in_u64));
   fail_unless(eina_value_pget(value, &u64));
   fail_unless(u64 == 0x1122334455667788);
   snprintf(buf, sizeof(buf), "%"PRIu64, in_u64);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(buf, out);
   free(out);
   eina_value_flush(value);

   /* floating point */
   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_FLOAT));
   in_f = 0.1234;
   fail_unless(eina_value_pset(value, &in_f));
   fail_unless(eina_value_pget(value, &f));
   fail_unless(CHECK_FP(0.1234, f));
   snprintf(buf, sizeof(buf), "%g", in_f);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   fail_unless(strncmp(buf, out, 6) == 0); /* stupid float... */
   free(out);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_DOUBLE));
   in_d = 34567.8;
   fail_unless(eina_value_pset(value, &in_d));
   fail_unless(eina_value_pget(value, &d));
   fail_unless(CHECK_FP(34567.8, d));
   snprintf(buf, sizeof(buf), "%g", in_d);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   fail_unless(strncmp(buf, out, 7) == 0); /* stupid double... */
   free(out);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_STRING));
   in_str = "hello world!";
   fail_unless(eina_value_pset(value, &in_str));
   fail_unless(eina_value_pget(value, &str));
   ck_assert_str_eq(str, "hello world!");
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(in_str, out);
   free(out);

   in_str = "eina-value";
   fail_unless(eina_value_pset(value, &in_str));
   fail_unless(eina_value_pget(value, &str));
   ck_assert_str_eq(str, "eina-value");
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(in_str, out);
   free(out);

   eina_value_flush(value);
   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_STRING));

   in_str = "profusion";
   fail_unless(eina_value_pset(value, &in_str));
   fail_unless(eina_value_pget(value, &str));
   ck_assert_str_eq(str, "profusion");
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   ck_assert_str_eq(in_str, out);
   free(out);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_convert_char)
{
   Eina_Value *value, conv;
   char c;
   short s;
   int i;
   long l;
   int64_t i64;
   unsigned char uc;
   unsigned short us;
   unsigned int ui;
   unsigned long ul;
   uint64_t u64;
   float f;
   double d;
   const char *str;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_CHAR);
   fail_unless(value != NULL);

   fail_unless(eina_value_set(value, 123));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &uc));
   fail_unless(uc == 123);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &us));
   fail_unless(us == 123);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ui));
   fail_unless(ui == 123);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ul));
   fail_unless(ul == 123);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &u64));
   fail_unless(u64 == 123);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &c));
   fail_unless(c == 123);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &s));
   fail_unless(s == 123);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i));
   fail_unless(i == 123);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_LONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &l));
   fail_unless(l == 123);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i64));
   fail_unless(i64 == 123);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, 123));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, 123));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &str));
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "123");
   eina_value_flush(&conv);

   /* negative tests */
   fail_unless(eina_value_set(value, -123));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_convert_uchar)
{
   Eina_Value *value, conv;
   char c;
   short s;
   int i;
   long l;
   int64_t i64;
   unsigned char uc;
   unsigned short us;
   unsigned int ui;
   unsigned long ul;
   uint64_t u64;
   float f;
   double d;
   const char *str;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_UCHAR);
   fail_unless(value != NULL);

   fail_unless(eina_value_set(value, 31));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &uc));
   fail_unless(uc == 31);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &us));
   fail_unless(us == 31);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ui));
   fail_unless(ui == 31);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ul));
   fail_unless(ul == 31);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &u64));
   fail_unless(u64 == 31);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &c));
   fail_unless(c == 31);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &s));
   fail_unless(s == 31);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i));
   fail_unless(i == 31);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_LONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &l));
   fail_unless(l == 31);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i64));
   fail_unless(i64 == 31);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, 31));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, 31));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &str));
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "31");
   eina_value_flush(&conv);

   /* negative tests */
   fail_unless(eina_value_set(value, 200));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_convert_short)
{
   Eina_Value *value, conv;
   short s;
   int i;
   long l;
   int64_t i64;
   unsigned short us;
   unsigned int ui;
   unsigned long ul;
   uint64_t u64;
   float f;
   double d;
   const char *str;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_SHORT);
   fail_unless(value != NULL);

   fail_unless(eina_value_set(value, 12345));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &us));
   fail_unless(us == 12345);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ui));
   fail_unless(ui == 12345);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ul));
   fail_unless(ul == 12345);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &u64));
   fail_unless(u64 == 12345);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &s));
   fail_unless(s == 12345);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i));
   fail_unless(i == 12345);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_LONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &l));
   fail_unless(l == 12345);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i64));
   fail_unless(i64 == 12345);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, 12345));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, 12345));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &str));
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "12345");
   eina_value_flush(&conv);

   /* negative tests */
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   // check negative value
   fail_unless(eina_value_set(value, -12345));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   // value should be positive
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   // value should be positive
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   // value should be positive
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   // value should be positive
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   // value should be positive
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_convert_ushort)
{
   Eina_Value *value, conv;
   int i;
   long l;
   int64_t i64;
   unsigned short us;
   unsigned int ui;
   unsigned long ul;
   uint64_t u64;
   float f;
   double d;
   const char *str;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_USHORT);
   fail_unless(value != NULL);

   fail_unless(eina_value_set(value, 54321));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &us));
   fail_unless(us == 54321);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ui));
   fail_unless(ui == 54321);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ul));
   fail_unless(ul == 54321);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &u64));
   fail_unless(u64 == 54321);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i));
   fail_unless(i == 54321);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_LONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &l));
   fail_unless(l == 54321);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i64));
   fail_unless(i64 == 54321);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, 54321));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, 54321));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &str));
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "54321");
   eina_value_flush(&conv);

   /* negative tests */
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_convert_int)
{
   Eina_Value *value, conv;
   int i;
   long l;
   int64_t i64;
   unsigned int ui;
   unsigned long ul;
   uint64_t u64;
   float f;
   double d;
   const char *str;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_INT);
   fail_unless(value != NULL);

   const int max_positive_signed_4_bytes = 0x7FFFFFFF;

   fail_unless(eina_value_set(value, max_positive_signed_4_bytes));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ui));
   fail_unless(ui == (unsigned int)max_positive_signed_4_bytes);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ul));
   fail_unless(ul == (unsigned long)max_positive_signed_4_bytes);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &u64));
   fail_unless(u64 == (uint64_t)max_positive_signed_4_bytes);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i));
   fail_unless(i == max_positive_signed_4_bytes);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_LONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &l));
   fail_unless(l == max_positive_signed_4_bytes);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i64));
   fail_unless(i64 == max_positive_signed_4_bytes);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, max_positive_signed_4_bytes));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, max_positive_signed_4_bytes));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &str));
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "2147483647"); // "2147483647" == 0x7FFFFFFF
   eina_value_flush(&conv);

   /* negative tests */
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   // check negative value
   fail_unless(eina_value_set(value, -1234567890));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   // value should be positive
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   // value should be positive
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   // value should be positive
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   // value should be positive
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   // value should be positive
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_convert_uint)
{
   Eina_Value *value, conv;
   unsigned int ui;
   unsigned long ul;
   uint64_t u64;
   float f;
   double d;
   const char *str;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_UINT);
   fail_unless(value != NULL);

   const unsigned int max_positive_unsigned_4_bytes = 0xFFFFFFFF;

   fail_unless(eina_value_set(value, max_positive_unsigned_4_bytes));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ui));
   fail_unless(ui == max_positive_unsigned_4_bytes);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ul));
   fail_unless(ul == max_positive_unsigned_4_bytes);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &u64));
   fail_unless(u64 == max_positive_unsigned_4_bytes);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, max_positive_unsigned_4_bytes));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, max_positive_unsigned_4_bytes));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &str));
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "4294967295"); // 4294967295 == 0xFFFFFFFF
   eina_value_flush(&conv);

   /* negative tests */
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   // value too big
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_convert_long)
{
   Eina_Value *value, conv;
   char c;
   short s;
   int i;
   long l;
   int64_t i64;
   unsigned char uc;
   unsigned short us;
   unsigned int ui;
   unsigned long ul;
   uint64_t u64;
   float f;
   double d;
   const char *str;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_LONG);
   fail_unless(value != NULL);

   fail_unless(eina_value_set(value, 32l));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &uc));
   fail_unless(uc == 32);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &us));
   fail_unless(us == 32);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ui));
   fail_unless(ui == 32);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ul));
   fail_unless(ul == 32ul);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &u64));
   fail_unless(u64 == 32ull);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &c));
   fail_unless(c == 32);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &s));
   fail_unless(s == 32);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i));
   fail_unless(i == 32);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_LONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &l));
   fail_unless(l == 32l);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i64));
   fail_unless(i64 == 32ll);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, 32));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, 32));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &str));
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "32");
   eina_value_flush(&conv);

   /* negative tests */
   fail_unless(eina_value_set(value, 128l));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, 256l));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, 32768l));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, 65536l));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, -32l));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_convert_ulong)
{
   Eina_Value *value, conv;
   char c;
   short s;
   int i;
   long l;
   int64_t i64;
   unsigned char uc;
   unsigned short us;
   unsigned int ui;
   unsigned long ul;
   uint64_t u64;
   float f;
   double d;
   const char *str;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_ULONG);
   fail_unless(value != NULL);

   fail_unless(eina_value_set(value, 42ul));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &uc));
   fail_unless(uc == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &us));
   fail_unless(us == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ui));
   fail_unless(ui == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ul));
   fail_unless(ul == 42ul);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &u64));
   fail_unless(u64 == 42ull);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &c));
   fail_unless(c == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &s));
   fail_unless(s == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i));
   fail_unless(i == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_LONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &l));
   fail_unless(l == 42l);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i64));
   fail_unless(i64 == 42ll);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, 42));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, 42));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &str));
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "42");
   eina_value_flush(&conv);

   /* negative tests */
   fail_unless(eina_value_set(value, 128ul));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, 256ul));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, 32768ul));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, 65536ul));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_convert_float)
{
   Eina_Value *value, conv;
   char c;
   short s;
   int i;
   long l;
   int64_t i64;
   unsigned char uc;
   unsigned short us;
   unsigned int ui;
   unsigned long ul;
   uint64_t u64;
   float f;
   double d;
   const char *str;

   float max_float_value = FLT_MAX;
   float min_float_value = FLT_MIN;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_FLOAT);
   fail_unless(value != NULL);

   fail_unless(eina_value_set(value, 42.354645));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &uc));
   fail_unless(uc == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &us));
   fail_unless(us == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ui));
   fail_unless(ui == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_ULONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &ul));
   fail_unless(ul == 42ul);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UINT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &u64));
   fail_unless(ul == 42ull);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &c));
   fail_unless(c == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &s));
   fail_unless(s == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i));
   fail_unless(i == 42);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_LONG));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &l));
   fail_unless(l == 42l);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_INT64));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &i64));
   fail_unless(i64 == 42ll);
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, max_float_value));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, max_float_value));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &str));
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "42.354645");
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, max_float_value));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, max_float_value));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, max_float_value));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, min_float_value));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, min_float_value));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, min_float_value));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, -max_float_value));

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_FLOAT));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &f));
   fail_unless(CHECK_FP(f, -max_float_value));
   eina_value_flush(&conv);

   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_DOUBLE));
   fail_unless(eina_value_convert(value, &conv));
   fail_unless(eina_value_get(&conv, &d));
   fail_unless(CHECK_FP(d, -max_float_value));
   eina_value_flush(&conv);

   /* negative tests */
   fail_unless(eina_value_set(value, -max_float_value));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_CHAR));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, -max_float_value));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_UCHAR));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, -max_float_value));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_SHORT));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   fail_unless(eina_value_set(value, -max_float_value));
   fail_unless(eina_value_setup(&conv, EINA_VALUE_TYPE_USHORT));
   fail_if(eina_value_convert(value, &conv));
   eina_value_flush(&conv);

   eina_value_free(value);
   eina_shutdown();

}
END_TEST

START_TEST(eina_value_test_array)
{
   Eina_Value *value, other;
   Eina_Value_Array desc;
   Eina_Inarray *inarray;
   char c;
   char buf[1024];
   char *str;

   eina_init();

   value = eina_value_array_new(EINA_VALUE_TYPE_CHAR, 0);
   fail_unless(value != NULL);

   fail_unless(eina_value_array_append(value, 'k'));
   fail_unless(eina_value_array_append(value, '-'));
   fail_unless(eina_value_array_append(value, 's'));

   fail_unless(eina_value_array_get(value, 0, &c));
   fail_unless(c == 'k');
   fail_unless(eina_value_array_get(value, 1, &c));
   fail_unless(c == '-');
   fail_unless(eina_value_array_get(value, 2, &c));
   fail_unless(c == 's');

   fail_unless(eina_value_array_insert(value, 0, '!'));
   fail_unless(eina_value_array_get(value, 0, &c));
   fail_unless(c == '!');
   fail_unless(eina_value_array_get(value, 1, &c));
   fail_unless(c == 'k');
   fail_unless(eina_value_array_get(value, 2, &c));
   fail_unless(c == '-');
   fail_unless(eina_value_array_get(value, 3, &c));
   fail_unless(c == 's');

   fail_unless(eina_value_array_set(value, 0, '*'));
   fail_unless(eina_value_array_get(value, 0, &c));
   fail_unless(c == '*');
   fail_unless(eina_value_array_get(value, 1, &c));
   fail_unless(c == 'k');
   fail_unless(eina_value_array_get(value, 2, &c));
   fail_unless(c == '-');
   fail_unless(eina_value_array_get(value, 3, &c));
   fail_unless(c == 's');

   snprintf(buf, sizeof(buf), "[%d, %d, %d, %d]",
            (int) '*', (int) 'k', (int) '-', (int) 's');

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, buf);
   free(str);

   eina_value_flush(value);
   fail_unless(eina_value_array_setup(value, EINA_VALUE_TYPE_STRINGSHARE, 2));

   fail_unless(eina_value_array_append(value, "Enlightenment.org"));
   fail_unless(eina_value_array_append(value, "X11"));
   fail_unless(eina_value_array_append(value, "Pants"));
   fail_unless(eina_value_array_append(value, "on!!!"));
   fail_unless(eina_value_array_append(value, "k-s"));

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "[Enlightenment.org, X11, Pants, on!!!, k-s]");
   free(str);

   eina_value_flush(value);
   fail_unless(eina_value_array_setup(value, EINA_VALUE_TYPE_CHAR, 0));
   fail_unless(eina_value_setup(&other, EINA_VALUE_TYPE_CHAR));

   fail_unless(eina_value_set(&other, 100));
   fail_unless(eina_value_get(&other, &c));
   fail_unless(c == 100);

   fail_unless(eina_value_convert(&other, value));
   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "[100]");
   free(str);

   fail_unless(eina_value_array_set(value, 0, 33));
   fail_unless(eina_value_convert(value, &other));
   fail_unless(eina_value_get(&other, &c));
   fail_unless(c == 33);

   inarray = eina_inarray_new(sizeof(char), 0);
   fail_unless(inarray != NULL);
   c = 11;
   fail_unless(eina_inarray_push(inarray, &c) >= 0);
   c = 21;
   fail_unless(eina_inarray_push(inarray, &c) >= 0);
   c = 31;
   fail_unless(eina_inarray_push(inarray, &c) >= 0);
   desc.subtype = EINA_VALUE_TYPE_CHAR;
   desc.step = 0;
   desc.array = inarray;
   fail_unless(eina_value_set(value, desc)); /* manually configure */
   eina_inarray_free(inarray);

   fail_unless(eina_value_array_get(value, 0, &c));
   fail_unless(c == 11);
   fail_unless(eina_value_array_get(value, 1, &c));
   fail_unless(c == 21);
   fail_unless(eina_value_array_get(value, 2, &c));
   fail_unless(c == 31);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_list)
{
   Eina_Value *value, other;
   Eina_Value_List desc;
   char c;
   char buf[1024];
   char *str;
   const char *s;

   eina_init();

   value = eina_value_list_new(EINA_VALUE_TYPE_CHAR);
   fail_unless(value != NULL);

   fail_unless(eina_value_list_append(value, 'k'));
   fail_unless(eina_value_list_append(value, '-'));
   fail_unless(eina_value_list_append(value, 's'));

   fail_unless(eina_value_list_get(value, 0, &c));
   fail_unless(c == 'k');
   fail_unless(eina_value_list_get(value, 1, &c));
   fail_unless(c == '-');
   fail_unless(eina_value_list_get(value, 2, &c));
   fail_unless(c == 's');

   fail_unless(eina_value_list_insert(value, 0, '!'));
   fail_unless(eina_value_list_get(value, 0, &c));
   fail_unless(c == '!');
   fail_unless(eina_value_list_get(value, 1, &c));
   fail_unless(c == 'k');
   fail_unless(eina_value_list_get(value, 2, &c));
   fail_unless(c == '-');
   fail_unless(eina_value_list_get(value, 3, &c));
   fail_unless(c == 's');

   fail_unless(eina_value_list_set(value, 0, '*'));
   fail_unless(eina_value_list_get(value, 0, &c));
   fail_unless(c == '*');
   fail_unless(eina_value_list_get(value, 1, &c));
   fail_unless(c == 'k');
   fail_unless(eina_value_list_get(value, 2, &c));
   fail_unless(c == '-');
   fail_unless(eina_value_list_get(value, 3, &c));
   fail_unless(c == 's');

   snprintf(buf, sizeof(buf), "[%d, %d, %d, %d]",
            (int) '*', (int) 'k', (int) '-', (int) 's');

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, buf);
   free(str);

   eina_value_flush(value);
   fail_unless(eina_value_list_setup(value, EINA_VALUE_TYPE_STRINGSHARE));

   fail_unless(eina_value_list_append(value, "Enlightenment.org"));
   fail_unless(eina_value_list_append(value, "X11"));
   fail_unless(eina_value_list_append(value, "Pants"));
   fail_unless(eina_value_list_append(value, "on!!!"));
   fail_unless(eina_value_list_append(value, "k-s"));

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "[Enlightenment.org, X11, Pants, on!!!, k-s]");
   free(str);

   eina_value_flush(value);
   fail_unless(eina_value_list_setup(value, EINA_VALUE_TYPE_CHAR));
   fail_unless(eina_value_setup(&other, EINA_VALUE_TYPE_CHAR));

   fail_unless(eina_value_set(&other, 100));
   fail_unless(eina_value_get(&other, &c));
   fail_unless(c == 100);

   fail_unless(eina_value_convert(&other, value));
   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "[100]");
   free(str);

   fail_unless(eina_value_list_set(value, 0, 33));
   fail_unless(eina_value_convert(value, &other));
   fail_unless(eina_value_get(&other, &c));
   fail_unless(c == 33);

   desc.subtype = EINA_VALUE_TYPE_STRING;
   desc.list = NULL;
   desc.list = eina_list_append(desc.list, "hello");
   desc.list = eina_list_append(desc.list, "world");
   desc.list = eina_list_append(desc.list, "eina");
   fail_unless(eina_list_count(desc.list) == 3);
   fail_unless(eina_value_set(value, desc));
   eina_list_free(desc.list);

   fail_unless(eina_value_list_get(value, 0, &s));
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "hello");
   fail_unless(eina_value_list_get(value, 1, &s));
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "world");
   fail_unless(eina_value_list_get(value, 2, &s));
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "eina");

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_hash)
{
   Eina_Value *value, other;
   Eina_Value_Hash desc;
   char c;
   char buf[1024];
   char **ptr;
   char *str;
   const char *s;

   eina_init();

   value = eina_value_hash_new(EINA_VALUE_TYPE_CHAR, 0);
   fail_unless(value != NULL);

   fail_unless(eina_value_hash_set(value, "first", 'k'));
   fail_unless(eina_value_hash_set(value, "second", '-'));
   fail_unless(eina_value_hash_set(value, "third", 's'));

   fail_unless(eina_value_hash_get(value, "first", &c));
   fail_unless(c == 'k');
   fail_unless(eina_value_hash_get(value, "second", &c));
   fail_unless(c == '-');
   fail_unless(eina_value_hash_get(value, "third", &c));
   fail_unless(c == 's');

   fail_unless(eina_value_hash_set(value, "first", '!'));
   fail_unless(eina_value_hash_get(value, "first", &c));
   fail_unless(c == '!');
   fail_unless(eina_value_hash_get(value, "second", &c));
   fail_unless(c == '-');
   fail_unless(eina_value_hash_get(value, "third", &c));
   fail_unless(c == 's');

   str = eina_value_to_string(value);
   fail_unless(str != NULL);

   snprintf(buf, sizeof(buf), "first: %d", (int) '!');
   fail_unless(strstr(str, buf) != NULL, "Couldn't find '%s' in '%s'", buf, str);

   snprintf(buf, sizeof(buf), "second: %d", (int) '-');
   fail_unless(strstr(str, buf) != NULL, "Couldn't find '%s' in '%s'", buf, str);

   snprintf(buf, sizeof(buf), "third: %d", (int) 's');
   fail_unless(strstr(str, buf) != NULL, "Couldn't find '%s' in '%s'", buf, str);

   free(str);

   eina_value_flush(value);
   fail_unless(eina_value_hash_setup(value, EINA_VALUE_TYPE_STRINGSHARE, 0));

   fail_unless(eina_value_hash_set(value, "a", "Enlightenment.org"));
   fail_unless(eina_value_hash_set(value, "b", "X11"));
   fail_unless(eina_value_hash_set(value, "c", "Pants"));
   fail_unless(eina_value_hash_set(value, "d", "on!!!"));
   fail_unless(eina_value_hash_set(value, "e", "k-s"));

   str = eina_value_to_string(value);
   fail_unless(str != NULL);

   eina_strlcpy(buf, "a: Enlightenment.org", sizeof(buf));
   fail_unless(strstr(str, buf) != NULL, "Couldn't find '%s' in '%s'", buf, str);

   eina_strlcpy(buf, "b: X11", sizeof(buf));
   fail_unless(strstr(str, buf) != NULL, "Couldn't find '%s' in '%s'", buf, str);

   eina_strlcpy(buf, "c: Pants", sizeof(buf));
   fail_unless(strstr(str, buf) != NULL, "Couldn't find '%s' in '%s'", buf, str);

   eina_strlcpy(buf, "d: on!!!", sizeof(buf));
   fail_unless(strstr(str, buf) != NULL, "Couldn't find '%s' in '%s'", buf, str);

   eina_strlcpy(buf, "e: k-s", sizeof(buf));
   fail_unless(strstr(str, buf) != NULL, "Couldn't find '%s' in '%s'", buf, str);

   free(str);

   eina_value_flush(value);
   fail_unless(eina_value_hash_setup(value, EINA_VALUE_TYPE_CHAR, 0));
   fail_unless(eina_value_setup(&other, EINA_VALUE_TYPE_CHAR));

   fail_unless(eina_value_set(&other, 100));
   fail_unless(eina_value_get(&other, &c));
   fail_unless(c == 100);

   fail_unless(eina_value_hash_set(value, "first", 33));
   fail_unless(eina_value_convert(value, &other));
   fail_unless(eina_value_get(&other, &c));
   fail_unless(c == 33);

   desc.subtype = EINA_VALUE_TYPE_STRING;
   desc.buckets_power_size = 0;
   desc.hash = eina_hash_string_small_new(NULL);
   fail_unless(desc.hash != NULL);
   /* watch out hash pointer is to a size of subtype->value_size! */
   ptr = malloc(sizeof(char *));
   *ptr = "there";
   fail_unless(eina_hash_add(desc.hash, "hi", ptr));
   ptr = malloc(sizeof(char *));
   *ptr = "y";
   fail_unless(eina_hash_add(desc.hash, "x", ptr));
   fail_unless(eina_value_set(value, desc));

   free(eina_hash_find(desc.hash, "hi"));
   free(eina_hash_find(desc.hash, "x"));
   eina_hash_free(desc.hash);

   fail_unless(eina_value_hash_get(value, "hi", &s));
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "there");

   fail_unless(eina_value_hash_get(value, "x", &s));
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "y");

   eina_value_free(value);
   eina_shutdown();
}
END_TEST


START_TEST(eina_value_test_timeval)
{
   Eina_Value *value, other;
   struct timeval itv, otv;
   time_t t;
   char c;
   char *str;
   char buf[64];

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_TIMEVAL);
   fail_unless(value != NULL);

   itv.tv_sec = 1;
   itv.tv_usec = 123;
   fail_unless(eina_value_set(value, itv));
   fail_unless(eina_value_get(value, &otv));
   fail_unless(itv.tv_sec == otv.tv_sec);
   fail_unless(itv.tv_usec == otv.tv_usec);

   itv.tv_sec = 3;
   itv.tv_usec = -1;
   fail_unless(eina_value_set(value, itv));
   fail_unless(eina_value_get(value, &otv));
   itv.tv_sec = 2;
   itv.tv_usec = 999999;
   fail_unless(itv.tv_sec == otv.tv_sec);
   fail_unless(itv.tv_usec == otv.tv_usec);

   fail_unless(eina_value_setup(&other, EINA_VALUE_TYPE_CHAR));
   fail_unless(eina_value_convert(value, &other));
   fail_unless(eina_value_get(&other, &c));
   fail_unless(c == 2);
   eina_value_flush(&other);

   itv.tv_sec = 12345;
   itv.tv_usec = 6789;
   fail_unless(eina_value_set(value, itv));
   str = eina_value_to_string(value);
   fail_unless(str != NULL);

   t = itv.tv_sec;
   strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&(t)));
   ck_assert_str_eq(str, buf);
   free(str);

   fail_unless(eina_value_setup(&other, EINA_VALUE_TYPE_TIMEVAL));
   fail_unless(eina_value_set(&other, itv));
   fail_unless(eina_value_compare(value, &other) == 0);

   itv.tv_sec++;
   fail_unless(eina_value_set(&other, itv));
   fail_unless(eina_value_compare(value, &other) < 0);

   itv.tv_sec -= 2;
   fail_unless(eina_value_set(&other, itv));
   fail_unless(eina_value_compare(value, &other) > 0);

   itv.tv_sec++;
   fail_unless(eina_value_set(&other, itv));
   fail_unless(eina_value_compare(value, &other) == 0);

   itv.tv_usec++;
   fail_unless(eina_value_set(&other, itv));
   fail_unless(eina_value_compare(value, &other) < 0);

   itv.tv_usec -= 2;
   fail_unless(eina_value_set(&other, itv));
   fail_unless(eina_value_compare(value, &other) > 0);

   itv.tv_usec++;
   fail_unless(eina_value_set(&other, itv));
   fail_unless(eina_value_compare(value, &other) == 0);

   eina_value_flush(&other);


   eina_value_free(value);
   eina_shutdown();
}
END_TEST


START_TEST(eina_value_test_blob)
{
   Eina_Value *value, other;
   Eina_Value_Blob in, out;
   unsigned char blob[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
   int i = 0x11223344;
   char *str;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_BLOB);
   fail_unless(value != NULL);

   in.ops = NULL;
   in.memory = blob;
   in.size = sizeof(blob);
   fail_unless(eina_value_set(value, in));
   fail_unless(eina_value_get(value, &out));
   fail_unless(out.memory == blob);
   fail_unless(out.size == sizeof(blob));
   fail_unless(memcmp(in.memory, out.memory, in.size) == 0);

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "BLOB(10, [01 02 03 04 05 06 07 08 09 0a])");
   free(str);

   fail_unless(eina_value_setup(&other, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_set(&other, i));
   fail_unless(eina_value_convert(&other, value));
   fail_unless(eina_value_get(value, &out));

   fail_unless(out.memory != NULL);
   fail_unless(out.size == sizeof(int));
   fail_unless(memcmp(&i, out.memory, sizeof(int)) == 0);

   eina_value_flush(&other);

   fail_unless(eina_value_setup(&other, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_set(&other, "hi there!"));
   fail_unless(eina_value_convert(&other, value));
   fail_unless(eina_value_get(value, &out));
   fail_unless(out.memory != NULL);
   fail_unless(out.size == sizeof("hi there!"));
   ck_assert_str_eq(out.memory, "hi there!");

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "BLOB(10, [68 69 20 74 68 65 72 65 21 00])");
   free(str);

   eina_value_flush(&other);

   fail_unless(eina_value_array_setup(&other, EINA_VALUE_TYPE_CHAR, 0));
   fail_unless(eina_value_array_append(&other, 0xa));
   fail_unless(eina_value_array_append(&other, 0xb));
   fail_unless(eina_value_array_append(&other, 0xc));
   fail_unless(eina_value_convert(&other, value));
   fail_unless(eina_value_get(value, &out));
   fail_unless(out.memory != NULL);
   fail_unless(out.size == 3);

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "BLOB(3, [0a 0b 0c])");
   free(str);

   eina_value_flush(&other);

   fail_unless(eina_value_setup(&other, EINA_VALUE_TYPE_BLOB));
   fail_unless(eina_value_set(&other, in));
   fail_unless(eina_value_convert(value, &other));
   fail_unless(eina_value_get(&other, &out));
   fail_unless(out.memory != NULL);
   fail_unless(out.size == 3);

   str = eina_value_to_string(&other);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "BLOB(3, [0a 0b 0c])");
   free(str);

   eina_value_flush(&other);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST


START_TEST(eina_value_test_struct)
{
   struct mybigst {
      int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, x;
   };
   const Eina_Value_Struct_Member mybigst_members[] = {
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, a),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, b),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, c),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, d),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, e),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, f),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, g),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, h),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, i),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, j),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, k),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, l),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, m),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, n),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, o),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, p),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, q),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, r),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, s),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, t),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, u),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, v),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct mybigst, x),
     EINA_VALUE_STRUCT_MEMBER_SENTINEL
   };
   const Eina_Value_Struct_Desc mybigst_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH,
     mybigst_members, 23, sizeof(struct mybigst)
   };
   struct myst {
      int i;
      char c;
   };
   const Eina_Value_Struct_Member myst_members[] = {
     {"i", EINA_VALUE_TYPE_INT, 0},
     {"c", EINA_VALUE_TYPE_CHAR, 4},
     {NULL, NULL, 0}
   };
   const Eina_Value_Struct_Desc myst_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     NULL, myst_members, 2, sizeof(struct myst)
   };
   Eina_Value *value, other;
   int i;
   char c;
   char *str;

   eina_init();

   value = eina_value_struct_new(&myst_desc);
   fail_unless(value != NULL);

   fail_unless(eina_value_struct_set(value, "i", 5678));
   fail_unless(eina_value_struct_set(value, "c", 0xf));

   fail_unless(eina_value_struct_get(value, "i", &i));
   fail_unless(i == 5678);
   fail_unless(eina_value_struct_get(value, "c", &c));
   fail_unless(c == 0xf);

   fail_unless(eina_value_struct_member_value_get
               (value, myst_members + 0, &other));
   fail_unless(other.type == EINA_VALUE_TYPE_INT);
   fail_unless(eina_value_get(&other, &i));
   fail_unless(i == 5678);
   eina_value_flush(&other);

   fail_unless(eina_value_struct_member_value_get
               (value, myst_members + 1, &other));
   fail_unless(other.type == EINA_VALUE_TYPE_CHAR);
   fail_unless(eina_value_get(&other, &c));
   fail_unless(c = 0xf);
   eina_value_flush(&other);

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "{i: 5678, c: 15}");
   free(str);

   fail_if(eina_value_struct_get(value, "x", 1234));

   i = 0x11223344;
   fail_unless(eina_value_struct_pset(value, "i", &i));
   i = -1;
   fail_unless(eina_value_struct_pget(value, "i", &i));
   fail_unless(i == 0x11223344);

   fail_unless(eina_value_copy(value, &other));
   str = eina_value_to_string(&other);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "{i: 287454020, c: 15}");
   free(str);

   eina_value_flush(&other);

   fail_unless(eina_value_struct_setup(&other, &mybigst_desc));
   fail_unless(eina_value_struct_set(&other, "a",  1) );
   fail_unless(eina_value_struct_set(&other, "b",  2));
   fail_unless(eina_value_struct_set(&other, "c",  3));
   fail_unless(eina_value_struct_set(&other, "d",  4));
   fail_unless(eina_value_struct_set(&other, "e",  5));
   fail_unless(eina_value_struct_set(&other, "f",  6));
   fail_unless(eina_value_struct_set(&other, "g",  7));
   fail_unless(eina_value_struct_set(&other, "h",  8));
   fail_unless(eina_value_struct_set(&other, "i",  9));
   fail_unless(eina_value_struct_set(&other, "j", 10));
   fail_unless(eina_value_struct_set(&other, "k", 12));
   fail_unless(eina_value_struct_set(&other, "l", 13));
   fail_unless(eina_value_struct_set(&other, "m", 14));
   fail_unless(eina_value_struct_set(&other, "n", 15));
   fail_unless(eina_value_struct_set(&other, "o", 16));
   fail_unless(eina_value_struct_set(&other, "p", 17));
   fail_unless(eina_value_struct_set(&other, "q", 18));
   fail_unless(eina_value_struct_set(&other, "r", 19));
   fail_unless(eina_value_struct_set(&other, "s", 20));
   fail_unless(eina_value_struct_set(&other, "t", 21));
   fail_unless(eina_value_struct_set(&other, "u", 22));
   fail_unless(eina_value_struct_set(&other, "v", 23));
   fail_unless(eina_value_struct_set(&other, "x", 24));

   fail_unless(eina_value_struct_get(&other, "a", &i));
   fail_unless(i ==  1);
   fail_unless(eina_value_struct_get(&other, "b", &i));
   fail_unless(i ==  2);
   fail_unless(eina_value_struct_get(&other, "c", &i));
   fail_unless(i ==  3);
   fail_unless(eina_value_struct_get(&other, "d", &i));
   fail_unless(i ==  4);
   fail_unless(eina_value_struct_get(&other, "e", &i));
   fail_unless(i ==  5);
   fail_unless(eina_value_struct_get(&other, "f", &i));
   fail_unless(i ==  6);
   fail_unless(eina_value_struct_get(&other, "g", &i));
   fail_unless(i ==  7);
   fail_unless(eina_value_struct_get(&other, "h", &i));
   fail_unless(i ==  8);
   fail_unless(eina_value_struct_get(&other, "i", &i));
   fail_unless(i ==  9);
   fail_unless(eina_value_struct_get(&other, "j", &i));
   fail_unless(i == 10);
   fail_unless(eina_value_struct_get(&other, "k", &i));
   fail_unless(i == 12);
   fail_unless(eina_value_struct_get(&other, "l", &i));
   fail_unless(i == 13);
   fail_unless(eina_value_struct_get(&other, "m", &i));
   fail_unless(i == 14);
   fail_unless(eina_value_struct_get(&other, "n", &i));
   fail_unless(i == 15);
   fail_unless(eina_value_struct_get(&other, "o", &i));
   fail_unless(i == 16);
   fail_unless(eina_value_struct_get(&other, "p", &i));
   fail_unless(i == 17);
   fail_unless(eina_value_struct_get(&other, "q", &i));
   fail_unless(i == 18);
   fail_unless(eina_value_struct_get(&other, "r", &i));
   fail_unless(i == 19);
   fail_unless(eina_value_struct_get(&other, "s", &i));
   fail_unless(i == 20);
   fail_unless(eina_value_struct_get(&other, "t", &i));
   fail_unless(i == 21);
   fail_unless(eina_value_struct_get(&other, "u", &i));
   fail_unless(i == 22);
   fail_unless(eina_value_struct_get(&other, "v", &i));
   fail_unless(i == 23);
   fail_unless(eina_value_struct_get(&other, "x", &i));
   fail_unless(i == 24);

   str = eina_value_to_string(&other);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "{a: 1, b: 2, c: 3, d: 4, e: 5, f: 6, g: 7, h: 8, i: 9, j: 10, k: 12, l: 13, m: 14, n: 15, o: 16, p: 17, q: 18, r: 19, s: 20, t: 21, u: 22, v: 23, x: 24}");
   free(str);

   eina_value_flush(&other);
   eina_value_free(value);
   eina_shutdown();
}
END_TEST


START_TEST(eina_value_test_array_of_struct)
{
   struct myst {
      int a, b, c;
      const char *s;
   };
   const Eina_Value_Struct_Member myst_members[] = {
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct myst, a),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct myst, b),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct myst, c),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_STRING, struct myst, s),
     EINA_VALUE_STRUCT_MEMBER_SENTINEL
   };
   const Eina_Value_Struct_Desc myst_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH,
     myst_members, 4, sizeof(struct myst)
   };
   Eina_Value *value, array_item;
   char *str;
   int i;

   eina_init();

   value = eina_value_array_new(EINA_VALUE_TYPE_STRUCT, 0);
   fail_unless(value != NULL);

   for (i = 0; i < 10; i++)
     {
        Eina_Value_Struct desc;
        struct myst st;
        char buf[64];

        snprintf(buf, sizeof(buf), "item%02d", i);
        st.a = i;
        st.b = i * 10;
        st.c = i * 100;
        st.s = buf;

        desc.desc = &myst_desc;
        desc.memory = &st;
        fail_unless(eina_value_array_append(value, desc));
     }

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "["
                      "{a: 0, b: 0, c: 0, s: item00}, "
                      "{a: 1, b: 10, c: 100, s: item01}, "
                      "{a: 2, b: 20, c: 200, s: item02}, "
                      "{a: 3, b: 30, c: 300, s: item03}, "
                      "{a: 4, b: 40, c: 400, s: item04}, "
                      "{a: 5, b: 50, c: 500, s: item05}, "
                      "{a: 6, b: 60, c: 600, s: item06}, "
                      "{a: 7, b: 70, c: 700, s: item07}, "
                      "{a: 8, b: 80, c: 800, s: item08}, "
                      "{a: 9, b: 90, c: 900, s: item09}"
                      "]");
   free(str);

   eina_value_array_value_get(value, 2, &array_item);
   eina_value_struct_get(&array_item, "a", &i);
   ck_assert_int_eq(i, 2);
   eina_value_struct_get(&array_item, "b", &i);
   ck_assert_int_eq(i, 20);
   eina_value_struct_get(&array_item, "c", &i);
   ck_assert_int_eq(i, 200);
   eina_value_struct_get(&array_item, "s", &str);
   ck_assert_str_eq(str, "item02");
   eina_value_flush(&array_item);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_optional_int)
{
   eina_init();

   /* Eina_Value *value = eina_value_new(EINA_VALUE_TYPE_OPTIONAL); */
   /* Eina_Bool is_empty; */
   /* ck_assert(eina_value_optional_empty_is(value, &is_empty)); */
   /* ck_assert(is_empty); */

   /* // sets expectation */
   /* int expected_value = -12345; */
   /* ck_assert(eina_value_optional_pset(value, EINA_VALUE_TYPE_INT, &expected_value)); */
   /* ck_assert(eina_value_optional_empty_is(value, &is_empty)); */
   /* ck_assert(!is_empty); */

   /* // gets the actual value */
   /* int actual_value; */
   /* ck_assert(eina_value_optional_pget(value, &actual_value)); */
   /* ck_assert_int_eq(expected_value, actual_value); */

   /* // resets the optional */
   /* ck_assert(eina_value_optional_reset(value)); */
   /* ck_assert(eina_value_optional_empty_is(value, &is_empty)); */
   /* ck_assert(is_empty); */

   /* // Sets expectation again after reset */
   /* expected_value = -54321; */
   /* ck_assert(eina_value_optional_pset(value, EINA_VALUE_TYPE_INT, &expected_value)); */
   /* ck_assert(eina_value_optional_empty_is(value, &is_empty)); */
   /* ck_assert(!is_empty); */

   /* // gets the actual value */
   /* ck_assert(eina_value_optional_pget(value, &actual_value)); */
   /* ck_assert_int_eq(expected_value, actual_value); */

   /* eina_value_free(value); */
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_optional_string)
{
   eina_init();

   Eina_Value *value = eina_value_new(EINA_VALUE_TYPE_OPTIONAL);
   Eina_Bool is_empty;
   ck_assert(eina_value_optional_empty_is(value, &is_empty));
   ck_assert(is_empty);
   ck_assert(EINA_VALUE_TYPE_OPTIONAL);

   // sets expectation
   const char *expected_value = "Hello world!";
   ck_assert(eina_value_optional_pset(value, EINA_VALUE_TYPE_STRING, &expected_value));
   ck_assert(eina_value_optional_empty_is(value, &is_empty));
   ck_assert(!is_empty);

   // gets the actual value
   const char *actual_value;
   ck_assert(eina_value_optional_pget(value, &actual_value));
   ck_assert_str_eq(expected_value, actual_value);

   // resets the optional
   ck_assert(eina_value_optional_reset(value));
   ck_assert(eina_value_optional_empty_is(value, &is_empty));
   ck_assert(is_empty);

   // Sets expectation again after reset
   expected_value = "!dlrow olleH";
   ck_assert(eina_value_optional_pset(value, EINA_VALUE_TYPE_STRING, &expected_value));
   ck_assert(eina_value_optional_empty_is(value, &is_empty));
   ck_assert(!is_empty);

   // gets the actual value
   ck_assert(eina_value_optional_pget(value, &actual_value));
   ck_assert_str_eq(expected_value, actual_value);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_optional_struct_members)
{
   eina_init();

   struct s {
     int64_t a;
     Eina_Value_Optional text;
     int64_t b;
   };
   const Eina_Value_Struct_Member members[] = {
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT64, struct s, a),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_OPTIONAL, struct s, text),
     EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT64, struct s, b),
     EINA_VALUE_STRUCT_MEMBER_SENTINEL
   };
   const Eina_Value_Struct_Desc desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     NULL, members, 3, sizeof(struct s)
   };

   Eina_Value *value = eina_value_struct_new(&desc);
   ck_assert_ptr_ne(NULL, value);

   int64_t expected_a = 0x1234567887654321ll;
   fail_unless(eina_value_struct_set(value, "a", expected_a));
   int64_t actual_a;
   fail_unless(eina_value_struct_get(value, "a", &actual_a));
   ck_assert_int_eq(expected_a, actual_a);

   int64_t expected_b = 0xEEDCBA9889ABCDEFll;
   fail_unless(eina_value_struct_set(value, "b", expected_b));
   int64_t actual_b;
   fail_unless(eina_value_struct_get(value, "b", &actual_b));
   ck_assert_int_eq(expected_b, actual_b);

   Eina_Value expected_value;
   fail_unless(eina_value_setup(&expected_value, EINA_VALUE_TYPE_OPTIONAL));
   const char* str = "Hello world!";
   fail_unless(eina_value_optional_pset(&expected_value, EINA_VALUE_TYPE_STRING, &str));
   fail_unless(eina_value_struct_value_set(value, "text", &expected_value));

   Eina_Value actual_value;
   fail_unless(eina_value_struct_value_get(value, "text", &actual_value));
   fail_unless(eina_value_compare(&expected_value, &actual_value) == 0);

   // tests if the value have been overriden
   fail_unless(eina_value_struct_get(value, "a", &actual_a));
   ck_assert_int_eq(expected_a, actual_a);
   fail_unless(eina_value_struct_get(value, "b", &actual_b));
   ck_assert_int_eq(expected_b, actual_b);

   eina_value_flush(&actual_value);
   eina_value_flush(&expected_value);

   eina_value_free(value);

   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_value)
{
   Eina_Value composed, v;
   const int ivalue = 3500;
   int i;
   const char *str;

   eina_init();

   fail_if(!eina_value_setup(&composed, EINA_VALUE_TYPE_VALUE));
   fail_if(!eina_value_setup(&v, EINA_VALUE_TYPE_INT));
   fail_if(!eina_value_set(&v, ivalue));
   fail_if(!eina_value_set(&composed, v));

   fail_if(!eina_value_get(&composed, &v));
   fail_if(!eina_value_get(&v, &i));
   fail_if(i != ivalue);

   eina_value_flush(&v);
   fail_if(!eina_value_setup(&v, EINA_VALUE_TYPE_STRING));
   fail_if(!eina_value_convert(&composed, &v));
   fail_if(!eina_value_get(&v, &str));
   ck_assert_str_eq(str, "3500");

   eina_value_flush(&v);
   fail_if(!eina_value_copy(&composed, &v));
   fail_if(eina_value_compare(&composed, &v));

   eina_value_flush(&v);
   eina_value_flush(&composed);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_value_string)
{
   Eina_Value composed, v, str_v;
   const char *msg = "A string", *str;

   eina_init();

   fail_if(!eina_value_setup(&composed, EINA_VALUE_TYPE_VALUE));
   fail_if(!eina_value_setup(&v, EINA_VALUE_TYPE_STRING));
   fail_if(!eina_value_set(&v, msg));
   fail_if(!eina_value_set(&composed, v));

   eina_value_flush(&v);
   fail_if(!eina_value_copy(&composed, &v));
   fail_if(eina_value_compare(&composed, &v));

   fail_if(!eina_value_get(&composed, &str_v));
   fail_if(!eina_value_get(&str_v, &str));
   ck_assert_str_eq(str, msg);
   eina_value_flush(&str_v);

   str = NULL;
   fail_if(!eina_value_get(&v, &str_v));
   fail_if(!eina_value_get(&str_v, &str));
   ck_assert_str_eq(str, msg);

   eina_value_flush(&str_v);
   eina_value_flush(&composed);
   eina_value_flush(&v);
   eina_shutdown();
}
END_TEST

void
eina_test_value(TCase *tc)
{
   tcase_add_test(tc, eina_value_test_simple);
   tcase_add_test(tc, eina_value_test_string);
   tcase_add_test(tc, eina_value_test_pvariant);
   tcase_add_test(tc, eina_value_test_compare);
   tcase_add_test(tc, eina_value_test_to_string);
   tcase_add_test(tc, eina_value_test_convert_char);
   tcase_add_test(tc, eina_value_test_convert_uchar);
   tcase_add_test(tc, eina_value_test_convert_short);
   tcase_add_test(tc, eina_value_test_convert_ushort);
   tcase_add_test(tc, eina_value_test_convert_int);
   tcase_add_test(tc, eina_value_test_convert_uint);
   tcase_add_test(tc, eina_value_test_convert_long);
   tcase_add_test(tc, eina_value_test_convert_ulong);
   tcase_add_test(tc, eina_value_test_convert_float);
   // TODO: other converters...
   tcase_add_test(tc, eina_value_test_array);
   tcase_add_test(tc, eina_value_test_list);
   tcase_add_test(tc, eina_value_test_hash);
   tcase_add_test(tc, eina_value_test_timeval);
   tcase_add_test(tc, eina_value_test_blob);
   tcase_add_test(tc, eina_value_test_struct);
   tcase_add_test(tc, eina_value_test_array_of_struct);
   tcase_add_test(tc, eina_value_test_optional_int);
   tcase_add_test(tc, eina_value_test_optional_string);
   tcase_add_test(tc, eina_value_test_optional_struct_members);
   tcase_add_test(tc, eina_value_test_value);
   tcase_add_test(tc, eina_value_test_value_string);
}
