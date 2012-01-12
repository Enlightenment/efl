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

#include "eina_suite.h"
#include "Eina.h"

#define FP_ERR (0.0000001)
#define CHECK_FP(a, b) ((a - b) < FP_ERR)

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
   fail_unless(eina_value_set(value, 3000000001U));
   fail_unless(eina_value_get(value, &ul));
   fail_unless(ul == 3000000001U);
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
   fail_unless(strcmp(s, "hello world!") == 0);

   fail_unless(eina_value_set(value, "eina-value"));
   fail_unless(eina_value_get(value, &s));
   fail_unless(strcmp(s, "eina-value") == 0);

   eina_value_flush(value);
   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_STRING));

   fail_unless(eina_value_set(value, "profusion"));
   fail_unless(eina_value_get(value, &s));
   fail_unless(strcmp(s, "profusion") == 0);

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
   in_ul = 3000000001U;
   fail_unless(eina_value_pset(value, &in_ul));
   fail_unless(eina_value_pget(value, &ul));
   fail_unless(ul == 3000000001U);
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
   fail_unless(strcmp(str, "hello world!") == 0);

   in_str = "eina-value";
   fail_unless(eina_value_pset(value, &in_str));
   fail_unless(eina_value_pget(value, &str));
   fail_unless(strcmp(str, "eina-value") == 0);

   eina_value_flush(value);
   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_STRING));

   in_str = "profusion";
   fail_unless(eina_value_pset(value, &in_str));
   fail_unless(eina_value_pget(value, &str));
   fail_unless(strcmp(str, "profusion") == 0);

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
   fail_unless(strcmp(buf, out) == 0);
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
   fail_unless(strcmp(buf, out) == 0);
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
   fail_unless(strcmp(buf, out) == 0);
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
   fail_unless(strcmp(buf, out) == 0);
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
   fail_unless(strcmp(buf, out) == 0);
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
   fail_unless(strcmp(buf, out) == 0);
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
   fail_unless(strcmp(buf, out) == 0);
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
   fail_unless(strcmp(buf, out) == 0);
   free(out);
   eina_value_flush(value);

   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_ULONG));
   in_ul = 3000000001U;
   fail_unless(eina_value_pset(value, &in_ul));
   fail_unless(eina_value_pget(value, &ul));
   fail_unless(ul == 3000000001U);
   snprintf(buf, sizeof(buf), "%lu", in_ul);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   fail_unless(strcmp(buf, out) == 0);
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
   fail_unless(strcmp(buf, out) == 0);
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
   fail_unless(strcmp(str, "hello world!") == 0);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   fail_unless(strcmp(in_str, out) == 0);
   free(out);

   in_str = "eina-value";
   fail_unless(eina_value_pset(value, &in_str));
   fail_unless(eina_value_pget(value, &str));
   fail_unless(strcmp(str, "eina-value") == 0);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   fail_unless(strcmp(in_str, out) == 0);
   free(out);

   eina_value_flush(value);
   fail_unless(eina_value_setup(value, EINA_VALUE_TYPE_STRING));

   in_str = "profusion";
   fail_unless(eina_value_pset(value, &in_str));
   fail_unless(eina_value_pget(value, &str));
   fail_unless(strcmp(str, "profusion") == 0);
   out = eina_value_to_string(value);
   fail_unless(out != NULL);
   fail_unless(strcmp(in_str, out) == 0);
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
   fail_unless(strcmp(str, "123") == 0);
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
   fail_unless(strcmp(str, "31") == 0);
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

   snprintf(buf, sizeof(buf), "[%hhd, %hhd, %hhd, %hhd]",
            '*', 'k', '-', 's');

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   fail_unless(strcmp(str, buf) == 0);
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
   fail_unless(strcmp(str, "[Enlightenment.org, X11, Pants, on!!!, k-s]") == 0);
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
   fail_unless(strcmp(str, "[100]") == 0);
   free(str);

   fail_unless(eina_value_array_set(value, 0, 33));
   fail_unless(eina_value_convert(value, &other));
   fail_unless(eina_value_get(&other, &c));
   fail_unless(c == 33);

   inarray = eina_inarray_new(sizeof(char), 0);
   fail_unless(inarray != NULL);
   c = 11;
   fail_unless(eina_inarray_append(inarray, &c) >= 0);
   c = 21;
   fail_unless(eina_inarray_append(inarray, &c) >= 0);
   c = 31;
   fail_unless(eina_inarray_append(inarray, &c) >= 0);
   desc.subtype = EINA_VALUE_TYPE_CHAR;
   desc.step = 0;
   desc.array = inarray; /* will be adopted and freed by value */
   fail_unless(eina_value_set(value, desc)); /* manually configure */
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

   snprintf(buf, sizeof(buf), "[%hhd, %hhd, %hhd, %hhd]",
            '*', 'k', '-', 's');

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   fail_unless(strcmp(str, buf) == 0);
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
   fail_unless(strcmp(str, "[Enlightenment.org, X11, Pants, on!!!, k-s]") == 0);
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
   fail_unless(strcmp(str, "[100]") == 0);
   free(str);

   fail_unless(eina_value_list_set(value, 0, 33));
   fail_unless(eina_value_convert(value, &other));
   fail_unless(eina_value_get(&other, &c));
   fail_unless(c == 33);

   desc.subtype = EINA_VALUE_TYPE_STRING;
   desc.list = NULL;
   desc.list = eina_list_append(desc.list, strdup("hello"));
   desc.list = eina_list_append(desc.list, strdup("world"));
   desc.list = eina_list_append(desc.list, strdup("eina"));
   fail_unless(eina_list_count(desc.list) == 3);
   fail_unless(eina_value_set(value, desc));
   fail_unless(eina_value_list_get(value, 0, &s));
   fail_unless(s != NULL);
   fail_unless(strcmp(s, "hello") == 0);
   fail_unless(eina_value_list_get(value, 1, &s));
   fail_unless(s != NULL);
   fail_unless(strcmp(s, "world") == 0);
   fail_unless(eina_value_list_get(value, 2, &s));
   fail_unless(s != NULL);
   fail_unless(strcmp(s, "eina") == 0);

   eina_value_free(value);
   eina_shutdown();
}
END_TEST

START_TEST(eina_value_test_hash)
{
   Eina_Value *value, other;
   char c;
   char buf[1024];
   char *str;

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

   puts("testing hash to string -- may fail due hash algorithm changes!");

   /* watchout, this is the order I got -- hash algorithm changes may change
    * the order!
    */
   snprintf(buf, sizeof(buf), "{first: %hhd, second: %hhd, third: %hhd}",
            '!', '-', 's');

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   printf("want: %s\n", buf);
   printf("got.: %s\n", str);
   fail_unless(strcmp(str, buf) == 0);
   free(str);

   eina_value_flush(value);
   fail_unless(eina_value_hash_setup(value, EINA_VALUE_TYPE_STRINGSHARE, 0));

   fail_unless(eina_value_hash_set(value, "a", "Enlightenment.org"));
   fail_unless(eina_value_hash_set(value, "b", "X11"));
   fail_unless(eina_value_hash_set(value, "c", "Pants"));
   fail_unless(eina_value_hash_set(value, "d", "on!!!"));
   fail_unless(eina_value_hash_set(value, "e", "k-s"));

   /* watchout, this is the order I got -- hash algorithm changes may change
    * the order!
    */
   strcpy(buf, "{e: k-s, d: on!!!, a: Enlightenment.org, b: X11, c: Pants}");

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   printf("want: %s\n", buf);
   printf("got.: %s\n", str);
   fail_unless(strcmp(str, buf) == 0);
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

   eina_value_free(value);
   eina_shutdown();
}
END_TEST


START_TEST(eina_value_test_timeval)
{
   Eina_Value *value, other;
   struct timeval itv, otv;
   char c;
   char *str;

   eina_init();

   value = eina_value_new(EINA_VALUE_TYPE_TIMEVAL);
   fail_unless(value != NULL);

   itv.tv_sec = 1;
   itv.tv_usec = 123;
   fail_unless(eina_value_set(value, itv));
   fail_unless(eina_value_get(value, &otv));
   fail_unless(memcmp(&itv, &otv, sizeof(struct timeval)) == 0);

   itv.tv_sec = 3;
   itv.tv_usec = -1;
   fail_unless(eina_value_set(value, itv));
   fail_unless(eina_value_get(value, &otv));
   itv.tv_sec = 2;
   itv.tv_usec = 999999;
   fail_unless(memcmp(&itv, &otv, sizeof(struct timeval)) == 0);

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
   fail_unless(strcmp(str, "12345.006789") == 0);
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
   fail_unless(memcmp(&in, &out, sizeof(Eina_Value_Blob)) == 0);

   str = eina_value_to_string(value);
   fail_unless(str != NULL);
   fail_unless(strcmp(str, "BLOB(10, [01 02 03 04 05 06 07 08 09 0a])") == 0);
   free(str);

   fail_unless(eina_value_setup(&other, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_set(&other, i));
   fail_unless(eina_value_convert(&other, value));
   fail_unless(eina_value_get(value, &out));

   fail_unless(out.memory != NULL);
   fail_unless(out.size == sizeof(int));
   fail_unless(memcmp(&i, out.memory, sizeof(int)) == 0);

   eina_value_flush(&other);


   eina_value_free(value);
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
   // TODO: other converters...
   tcase_add_test(tc, eina_value_test_array);
   tcase_add_test(tc, eina_value_test_list);
   tcase_add_test(tc, eina_value_test_hash);
   tcase_add_test(tc, eina_value_test_timeval);
   tcase_add_test(tc, eina_value_test_blob);
}
