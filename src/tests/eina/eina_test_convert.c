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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <locale.h>

#include <Eina.h>

#include "eina_suite.h"

EFL_START_TEST(eina_convert_simple)
{
   char tmp[128];
   char ref[128];

   fail_if(eina_convert_itoa(0, tmp) != 1);
   fail_if(strcmp(tmp, "0") != 0);

   fail_if(eina_convert_itoa(-1, tmp) != 2);
   fail_if(strcmp(tmp, "-1") != 0);

   fail_if(eina_convert_itoa(100, tmp) != 3);
   fail_if(strcmp(tmp, "100") != 0);

   fail_if(eina_convert_itoa(-100, tmp) != 4);
   fail_if(strcmp(tmp, "-100") != 0);

   fail_if(eina_convert_itoa(10000000, tmp) != 8);
   fail_if(strcmp(tmp, "10000000") != 0);

   snprintf(ref, sizeof (ref), "%d", INT_MIN);
   fail_if(eina_convert_itoa(INT_MIN, tmp) != (int) strlen(ref));
   fail_if(strcmp(tmp, ref) != 0);

   fail_if(eina_convert_xtoa(0, tmp) != 1);
   fail_if(strcmp(tmp, "0") != 0);

   fail_if(eina_convert_xtoa(0xA1, tmp) != 2);
   fail_if(strcmp(tmp, "a1") != 0);

   fail_if(eina_convert_xtoa(0xFF00EF0E, tmp) != 8);
   fail_if(strcmp(tmp, "ff00ef0e") != 0);
}
EFL_END_TEST

#define EET_TEST_DOUBLE0 123.45689
#define EET_TEST_DOUBLE1 1.0
#define EET_TEST_DOUBLE2 0.25
#define EET_TEST_DOUBLE3 0.0001234
#define EET_TEST_DOUBLE4 123456789.9876543210

static void
_eina_convert_check(double test, int length)
{
   char tmp[128];
   long long int m = 0;
   long e = 0;
   double r;

   fail_if(eina_convert_dtoa(test, tmp) != length);
   fail_if(eina_convert_atod(tmp, 128, &m, &e) != EINA_TRUE);
   r = ldexp((double)m, e);
   fail_if(fabs(r - test) > DBL_MIN);
}

   EFL_START_TEST(eina_convert_double)
{
   long long int m = 0;
   long e = 0;

   _eina_convert_check(EET_TEST_DOUBLE0,  20);
   _eina_convert_check(-EET_TEST_DOUBLE0, 21);
   _eina_convert_check(EET_TEST_DOUBLE1,   6);
   _eina_convert_check(EET_TEST_DOUBLE2,   6);
   _eina_convert_check(EET_TEST_DOUBLE3,  21);
   _eina_convert_check(EET_TEST_DOUBLE4,  21);

   fail_if(eina_convert_atod("ah ah ah", 8, &m, &e) != EINA_FALSE);
   fail_if(eina_convert_atod("0xjo", 8, &m, &e) != EINA_FALSE);
   fail_if(eina_convert_atod("0xp", 8, &m, &e) != EINA_FALSE);

}
EFL_END_TEST

static void
_eina_convert_fp_check(double d, Eina_F32p32 fp, int length)
{
   char tmp1[128];
   char tmp2[128];
   Eina_F32p32 fpc;
   double fpd;
   int l1;
   int l2;

   l1 = eina_convert_dtoa(d, tmp1);
   l2 = eina_convert_fptoa(fp, tmp2);
/*    fprintf(stderr, "[%s](%i) vs [%s](%i)\n", tmp1, l1, tmp2, l2); */
   fail_if(l1 != l2);
   fail_if(length != l1);
   fail_if(strcmp(tmp1, tmp2) != 0);

   fail_if(!eina_convert_atofp(tmp2, l2, &fpc));
/*    fprintf(stderr, "%016x vs %016x\n", fpc, fp); */
   fail_if(fpc != fp);

   fail_if(!eina_convert_atofp(tmp1, l1, &fpc));
   fpd = eina_f32p32_double_to(fpc);
/*    fprintf(stderr, "%0.16f vs %0.16f\n", fpd, d); */
   fail_if(fabs(fpd - d) > DBL_MIN);

   d = -d;
   fp = -fp;

   l1 = eina_convert_dtoa(d, tmp1);
   l2 = eina_convert_fptoa(fp, tmp2);
   fail_if(l1 != l2);
   fail_if(length + 1 != l1);
   fail_if(strcmp(tmp1, tmp2) != 0);

   fail_if(!eina_convert_atofp(tmp2, l2, &fpc));
/*    fprintf(stderr, "%016x vs %016x\n", fpc, fp); */
   fail_if(fpc != fp);

   fail_if(!eina_convert_atofp(tmp1, l1, &fpc));
   fpd = eina_f32p32_double_to(fpc);
/*    fprintf(stderr, "%0.16f vs %0.16f\n", fpd, d); */
   fail_if(fabs(fpd - d) > DBL_MIN);
}

   EFL_START_TEST(eina_convert_fp)
{
   _eina_convert_fp_check(1.0,     0x0000000100000000,  6);
   _eina_convert_fp_check(0.5,     0x0000000080000000,  8);
   _eina_convert_fp_check(0.625,   0x00000000a0000000,  8);
   _eina_convert_fp_check(256.0,   0x0000010000000000,  6);
   _eina_convert_fp_check(0.5,     0x0000000080000000,  8);
   _eina_convert_fp_check(128.625, 0x00000080a0000000, 10);
}
EFL_END_TEST

static void
_eina_convert_strtod_c_check(const char *str)
{
   double d1;
   double d2;
   char *e1;
   char *e2;

   e1 = NULL;
   d1 = eina_convert_strtod_c(str, &e1);

   e2 = NULL;
   d2 = strtod(str, &e2);

   switch(fpclassify(d2))
     {
     case FP_NAN:
       fail_if(fpclassify(d1) != FP_NAN);
       break;
     case FP_INFINITE:
       fail_if(fpclassify(d1) != FP_INFINITE);
       break;
     default:
       fail_if((fpclassify(d1) != FP_ZERO) &&
               (fpclassify(d1) != FP_SUBNORMAL) &&
               (fpclassify(d1) != FP_NORMAL));
       if (!EINA_DBL_EQ(d1,d2) || (e1 != e2))
         {
            printf("  FP_NORMAL\n");
            printf("  ERR: %s, %s\n", str, strerror(errno));
            printf("    E1 **%.6f**, **%g**, %s\n", d1, d1, e1 ? e1 : "");
            printf("    E2 **%.6f**, **%g**, %s\n", d2, d2, e2 ? e2 : "");
            if (!EINA_DBL_EQ(d1,d2)) printf("different value\n");
            if (e1 != e2) printf("different end position\n");
         }

       fail_if(!EINA_DBL_EQ(d1,d2) || (e1 != e2));
       break;
     }

}

EFL_START_TEST(eina_convert_strtod_c_simple)
{
  char *old;

   old = setlocale(LC_ALL, "C");
   _eina_convert_strtod_c_check("0");
   _eina_convert_strtod_c_check("-0");
   _eina_convert_strtod_c_check(".1");
   _eina_convert_strtod_c_check("  .");
   _eina_convert_strtod_c_check("  1.2e3");
   _eina_convert_strtod_c_check(" +1.2e3");
   _eina_convert_strtod_c_check("1.2e3");
   _eina_convert_strtod_c_check("+1.2e3");
   _eina_convert_strtod_c_check("+1.e3");
   _eina_convert_strtod_c_check("-1.2e3");
   _eina_convert_strtod_c_check("-1.2e3.5");
   _eina_convert_strtod_c_check("-1.2e");
   _eina_convert_strtod_c_check("--1.2e3.5");
   _eina_convert_strtod_c_check("--1-.2e3.5");
   _eina_convert_strtod_c_check("-a");
   _eina_convert_strtod_c_check("a");
   _eina_convert_strtod_c_check(".1e");
   _eina_convert_strtod_c_check(".1e0");
   _eina_convert_strtod_c_check(".1e3");
   _eina_convert_strtod_c_check(".1e-3");
   _eina_convert_strtod_c_check(".1e-");
   _eina_convert_strtod_c_check(" .e-");
   _eina_convert_strtod_c_check(" .e");
   _eina_convert_strtod_c_check(" e");
   _eina_convert_strtod_c_check(" e0");
   _eina_convert_strtod_c_check(" ee");
   _eina_convert_strtod_c_check(" -e");
   _eina_convert_strtod_c_check(" .9");
   _eina_convert_strtod_c_check(" ..9");
   _eina_convert_strtod_c_check("009");
   _eina_convert_strtod_c_check("0.09e02");
    /* http://thread.gmane.org/gmane.editors.vim.devel/19268/ */
   _eina_convert_strtod_c_check("0.9999999999999999999999999999999999");
   _eina_convert_strtod_c_check("2.2250738585072010e-308"); // BUG
    /* PHP (slashdot.jp): http://opensource.slashdot.jp/story/11/01/08/0527259/PHP%E3%81%AE%E6%B5%AE%E5%8B%95%E5%B0%8F%E6%95%B0%E7%82%B9%E5%87%A6%E7%90%86%E3%81%AB%E7%84%A1%E9%99%90%E3%83%AB%E3%83%BC%E3%83%97%E3%81%AE%E3%83%90%E3%82%B0 */
   _eina_convert_strtod_c_check("2.2250738585072011e-308");
    /* Gauche: http://blog.practical-scheme.net/gauche/20110203-bitten-by-floating-point-numbers-again */
   _eina_convert_strtod_c_check("2.2250738585072012e-308");
   _eina_convert_strtod_c_check("2.2250738585072013e-308");
   _eina_convert_strtod_c_check("2.2250738585072014e-308");
   _eina_convert_strtod_c_check(" NaNfoo");
   _eina_convert_strtod_c_check(" -INFfoo");
   _eina_convert_strtod_c_check("  InFiNiTyfoo");
   setlocale(LC_ALL, old);
}
EFL_END_TEST

void
eina_test_convert(TCase *tc)
{
   tcase_add_test(tc, eina_convert_simple);
   tcase_add_test(tc, eina_convert_double);
   tcase_add_test(tc,     eina_convert_fp);
   tcase_add_test(tc, eina_convert_strtod_c_simple);
}
