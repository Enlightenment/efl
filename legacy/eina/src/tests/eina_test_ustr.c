/* EINA - EFL data type library
 * Copyright (C) 2010 Brett Nash
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "eina_suite.h"
#include "Eina.h"

static const Eina_Unicode STR1[] = {'P', 'a', 'n', 't', 's',' ', 'O', 'n', 0};
static const Eina_Unicode STR2[] = {'P', 'a', 'n', 't', 's',' ', 'O', 'f', 'f', 0};
static const Eina_Unicode STR3[] = {'P', 'a', 'n', 't', 's',' ', 'O', 'n', 0};
static const Eina_Unicode STR4[] = {'A', 0};
static const Eina_Unicode EMPTYSTR[] = {0};

START_TEST(eina_unicode_strcmp_test)
{
   eina_init();

   /* 1 & 2 */
   fail_if(eina_unicode_strcmp(STR1,STR2) == 0);
   fail_if(eina_unicode_strcmp(STR1,STR2) < 1);

   /* 1 & 3 */
   fail_if(eina_unicode_strcmp(STR1, STR3) != 0);

   /* 1 & 4 */
   fail_if(eina_unicode_strcmp(STR1, STR4) == 0);
   fail_if(eina_unicode_strcmp(STR1, STR4) > 1);

   /* 1 & empty */
   fail_if(eina_unicode_strcmp(STR1, EMPTYSTR) < 1);

   /* Self tests */
   fail_if(eina_unicode_strcmp(STR1, STR1) != 0);
   fail_if(eina_unicode_strcmp(STR2, STR2) != 0);
   fail_if(eina_unicode_strcmp(STR3, STR3) != 0);
   fail_if(eina_unicode_strcmp(STR4, STR4) != 0);
   fail_if(eina_unicode_strcmp(EMPTYSTR, EMPTYSTR) != 0);

   eina_shutdown();
}
END_TEST

START_TEST(eina_unicode_strcpy_test)
{
   Eina_Unicode buf[10] = { 0 };
   Eina_Unicode *rv;

   eina_init();

   rv = eina_unicode_strcpy(buf,STR1);
   fail_if(rv != buf);
   fail_if(eina_unicode_strcmp(buf,STR1) != 0);

   rv = eina_unicode_strcpy(buf,STR2);
   fail_if(rv != buf);
   fail_if(eina_unicode_strcmp(buf,STR2) != 0);

   /* Now a shorter string */
   rv = eina_unicode_strcpy(buf,STR2);
   fail_if(rv != buf);
   fail_if(eina_unicode_strcmp(buf,STR2) != 0);

   /* Really short string */
   rv = eina_unicode_strcpy(buf,STR4);
   fail_if(rv != buf);
   fail_if(eina_unicode_strcmp(buf,STR4) != 0);
   fail_if(buf[2] != 'n'); /* check old buf is there */

   buf[1] = '7';
   rv = eina_unicode_strcpy(buf,EMPTYSTR);
   fail_if(rv != buf);
   fail_if(buf[0] != 0);
   fail_if(buf[1] != '7');

   eina_shutdown();
}
END_TEST

START_TEST(eina_unicode_strncpy_test)
{
   Eina_Unicode buf[10] = { 0 };
   Eina_Unicode *rv;

   eina_init();

   rv = eina_unicode_strncpy(buf,STR1,9);
   fail_if(rv != buf);
   fail_if(eina_unicode_strcmp(buf,STR1) != 0);

   buf[1] = '7';
   rv = eina_unicode_strncpy(buf,STR1,1);
   fail_if(rv != buf);
   fail_if(buf[1] != '7');
   fail_if(buf[0] != STR1[0]);

   buf[9] = '7';
   rv = eina_unicode_strncpy(buf, STR4, 10);
   fail_if(rv != buf);
   fail_if(eina_unicode_strcmp(buf,STR4) != 0);
   fail_if(buf[9] != 0);

   buf[0] = '7';
   rv = eina_unicode_strncpy(buf, STR1, 0);
   fail_if(buf[0] != '7');

   /* may segfault */
   buf[0] = '7';
   rv = eina_unicode_strncpy(buf, NULL, 0);
   fail_if(buf[0] != '7');

   /* Hopefully won't segfault */
   rv = eina_unicode_strncpy(NULL, STR1, 0);
   fail_if(rv != NULL);

   eina_shutdown();
}
END_TEST



START_TEST(eina_ustr_strlen_test)
{

   eina_init();

   fail_if(eina_unicode_strlen(STR1) != 8);
   fail_if(eina_unicode_strlen(STR2) != 9);
   fail_if(eina_unicode_strlen(STR3) != 8);
   fail_if(eina_unicode_strlen(STR4) != 1);
   fail_if(eina_unicode_strlen(EMPTYSTR) != 0);
   /* Eina unicode doesn't take NULL */
   // fail_if(eina_unicode_strlen(NULL));

   eina_shutdown();
}
END_TEST

START_TEST(eina_unicode_strnlen_test)
{
   eina_init();

   /* Strlen style tests*/
   fail_if(eina_unicode_strnlen(STR1,10) != 8);
   fail_if(eina_unicode_strnlen(STR2,10) != 9);
   fail_if(eina_unicode_strnlen(STR3,10) != 8);
   fail_if(eina_unicode_strnlen(STR4,10) != 1);
   fail_if(eina_unicode_strnlen(EMPTYSTR,10) != 0);

   /* Too short tests */
   fail_if(eina_unicode_strnlen(STR1,3) != 3);
   fail_if(eina_unicode_strnlen(STR2,3) != 3);
   fail_if(eina_unicode_strnlen(STR3,3) != 3);
   fail_if(eina_unicode_strnlen(EMPTYSTR,1) != 0);
   fail_if(eina_unicode_strnlen(NULL,0) != 0);

   eina_shutdown();
}
END_TEST

START_TEST(eina_unicode_strdup_test)
{
   Eina_Unicode *buf;

   eina_init();

   buf = eina_unicode_strdup(STR1);
   fail_if(!buf);
   fail_if(eina_unicode_strlen(buf) != eina_unicode_strlen(STR1));
   fail_if(eina_unicode_strcmp(buf, STR1));
   free(buf);

   buf = eina_unicode_strdup(EMPTYSTR);
   fail_if(!buf);
   fail_if(buf[0] != 0);

   eina_shutdown();
}
END_TEST

START_TEST(eina_unicode_strstr_test)
{
   Eina_Unicode *buf;
   Eina_Unicode on[] = { 'O', 'n', 0 };

   eina_init();

   buf = eina_unicode_strstr(STR1,on);
   fail_if(!buf);
   fail_if(buf != STR1 + 6);
   fail_if(eina_unicode_strcmp(buf,on) != 0);

   buf = eina_unicode_strstr(STR2,on);
   fail_if(buf);

   buf = eina_unicode_strstr(EMPTYSTR, on);
   fail_if(buf);

   buf = eina_unicode_strstr(STR1, EMPTYSTR);
   fail_if(!buf);
   fail_if(buf != STR1);

   eina_shutdown();
}
END_TEST

START_TEST(eina_unicode_utf8)
{
   int ind;
   unsigned char ch;
   eina_init();

   /* Valid utf-8 cases */
   /* First possible sequence of a certain length */
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\x00", &ind) != 0x00) ||
           (ind != 0));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\x01", &ind) != 0x01) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xC2\x80", &ind) != 0x80) ||
           (ind != 2));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xE0\xA0\x80", &ind) != 0x800) ||
           (ind != 3));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xF0\x90\x80\x80", &ind) != 0x10000) ||
           (ind != 4));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xF8\x88\x80\x80\x80", &ind) != 0x200000) || (ind != 5));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xFC\x84\x80\x80\x80\x80", &ind) != 0x4000000) || (ind != 6));

   /* Last possible sequence of a certain length */
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\x7F", &ind) != 0x7F) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xDF\xBF", &ind) != 0x7FF) ||
           (ind != 2));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xEF\xBF\xBF", &ind) != 0xFFFF) ||
           (ind != 3));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xF7\xBF\xBF\xBF", &ind) != 0x1FFFFF) ||
           (ind != 4));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xFB\xBF\xBF\xBF\xBF", &ind) != 0x3FFFFFF) || (ind != 5));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xFD\xBF\xBF\xBF\xBF\xBF", &ind) != 0x7FFFFFFF) || (ind != 6));

   /* Other boundary conditions */
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xED\x9F\xBF", &ind) != 0xD7FF) ||
           (ind != 3));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xEE\x80\x80", &ind) != 0xE000) ||
           (ind != 3));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xEF\xBF\xBD", &ind) != 0xFFFD) ||
           (ind != 3));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xF4\x8F\xBF\xBF", &ind) != 0x10FFFF) ||
           (ind != 4));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xF4\x90\x80\x80", &ind) != 0x110000) ||
           (ind != 4));

   /* Error cases */
   /* Standalone continuation bytes */
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\x80", &ind) != 0xDC80) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xBF", &ind) != 0xDCBF) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\x80\xBF", &ind) != 0xDC80) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xBF\x80", &ind) != 0xDCBF) ||
           (ind != 1));
   /* All possible continuation bytes */
   for (ch = 0x80 ; ch <= 0xBF ; ch++)
     {
        char buf[] = {ch, 0};
        ind = 0;
        fail_if((eina_unicode_utf8_get_next(buf, &ind) != (0xDC00 | ch)) ||
                (ind != 1));
     }

   /* Isolated starting sequences */
#define _FIRST_SEQUENCES(start, end) \
   do \
     { \
        int i; \
        char *buf = alloca(((end - start + 1) * 2) + 1); \
        for (i = 0, ch = start ; ch <= end ; i++, ch++) \
          { \
             buf[i * 2] = ch; \
             buf[(i * 2) + 1] = ' '; \
          } \
        ind = 0; \
        for (i = 0, ch = start ; ch <= end ; ch++) \
          { \
             fail_if((eina_unicode_utf8_get_next(buf, &ind) != (0xDC00 | ch)) || \
                     (ind != ++i)); \
             fail_if((eina_unicode_utf8_get_next(buf, &ind) != 0x20) || \
                     (ind != ++i)); \
          } \
     } \
   while (0)
   /* all first bytes of 2-byte sequences separated by spaces. */
   _FIRST_SEQUENCES(0xC0, 0xDF);
   /* all first bytes of 3-byte sequences separated by spaces. */
   _FIRST_SEQUENCES(0xE0, 0xEF);
   /* all first bytes of 4-byte sequences separated by spaces. */
   _FIRST_SEQUENCES(0xF0, 0xF7);
   /* all first bytes of 5-byte sequences separated by spaces. */
   _FIRST_SEQUENCES(0xF8, 0xFB);
   /* all first bytes of 6-byte sequences separated by spaces. */
   _FIRST_SEQUENCES(0xFC, 0xFD);

   /* Incomplete sequences first means the first utf8 char, len means
    * the correct length */
#define _INCOMPLETE_SEQUENCES(first, conti, len) \
   do \
     { \
        int i, j; \
        char *buf = alloca(len + 1); \
        i = 0; \
        buf[i++] = first; \
        for ( ; i < len ; i++) \
          { \
             Eina_Unicode val; \
             for (j = 1 ; j < i ; j++) \
               { \
                  buf[j] = conti; \
               } \
             buf[j] = 0; \
             ind = 0; \
             fail_if( \
                (eina_unicode_utf8_get_next(buf, &ind) != (0xDC00 | first))); \
             while ((val = eina_unicode_utf8_get_next(buf, &ind))) \
               { \
                  fail_if(val != (0xDC00 | conti)); \
               } \
             fail_if(ind != i); \
          } \
     } \
   while (0)

   /* Sequences with missing continuation */
   _INCOMPLETE_SEQUENCES(0xC0, 0x81, 2);
   _INCOMPLETE_SEQUENCES(0xDF, 0xBF, 2);
   _INCOMPLETE_SEQUENCES(0xE0, 0x81, 3);
   _INCOMPLETE_SEQUENCES(0xEF, 0xBF, 3);
   _INCOMPLETE_SEQUENCES(0xF0, 0x81, 4);
   _INCOMPLETE_SEQUENCES(0xF7, 0xBF, 4);
   _INCOMPLETE_SEQUENCES(0xF8, 0x81, 5);
   _INCOMPLETE_SEQUENCES(0xFB, 0xBF, 5);
   _INCOMPLETE_SEQUENCES(0xFC, 0x81, 6);
   _INCOMPLETE_SEQUENCES(0xFD, 0xBF, 6);

   /* Impossible bytes */
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xFE", &ind) != 0xDCFE) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xFF", &ind) != 0xDCFF) ||
           (ind != 1));

   /* Overlong sequences */
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xC0\xAF", &ind) != 0xDCC0) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xE0\x80\xAF", &ind) != 0xDCE0) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xF0\x80\x80\xAF", &ind) != 0xDCF0) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xF8\x80\x80\x80\xAF", &ind) != 0xDCF8) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xFC\x80\x80\x80\x80\xAF", &ind) != 0xDCFC) ||
           (ind != 1));

   /* Maximum overlong sequences */
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xC1\xBF", &ind) != 0xDCC1) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xE0\x9F\xBF", &ind) != 0xDCE0) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xF0\x8F\xBF\xBF", &ind) != 0xDCF0) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xF8\x87\xBF\xBF\xBF", &ind) != 0xDCF8) ||
           (ind != 1));
   ind = 0;
   fail_if((eina_unicode_utf8_get_next("\xFC\x83\xBF\xBF\xBF\xBF", &ind) != 0xDCFC) ||
           (ind != 1));
   /* Add some more error cases here */

   /* Just to cover prev/len. General utf-8 parsing was covered above */
   fail_if(eina_unicode_utf8_get_len("\xF4\x90\x80\x80\xF4\x8F\xBF\xBF") != 2);
   ind = 0;
   fail_if((eina_unicode_utf8_get_prev("\xED\x9F\xBF", &ind) != 0xD7FF) ||
           (ind != 0));
   ind = 3;
   fail_if((eina_unicode_utf8_get_prev("\xED\x9F\xBF", &ind) != 0x00) ||
           (ind != 0));

   eina_shutdown();
}
END_TEST

START_TEST(eina_unicode_utf8_conversion)
{
   Eina_Unicode uni_in[] = {0x5D0, 0xFDF6, 0xDC80, 0x1F459, 0x3FFFFFF,
        0x7FFFFFFF, 'a', 'b', 'c', 0};
   Eina_Unicode *uni_out;
   char c_in[] = "\xD7\x90""\xEF\xB7\xB6""\x80""\xF0\x9F\x91\x99"
      "\xFB\xBF\xBF\xBF\xBF""\xFD\xBF\xBF\xBF\xBF\xBF""abc";
   char *c_out;
   int len;

   eina_init();

   uni_out = eina_unicode_utf8_to_unicode(c_in, &len);
   fail_if((len != 9) || eina_unicode_strcmp(uni_in, uni_out));
   free(uni_out);

   c_out = eina_unicode_unicode_to_utf8(uni_in, &len);
   fail_if((len != 24) || strcmp(c_in, c_out));
   free(c_out);

   eina_shutdown();
}
END_TEST

void
eina_test_ustr(TCase *tc)
{
   printf("ustr test\n");
   tcase_add_test(tc,eina_unicode_strcmp_test);
   tcase_add_test(tc,eina_unicode_strcpy_test);
   tcase_add_test(tc,eina_unicode_strncpy_test);
   tcase_add_test(tc,eina_ustr_strlen_test);
   tcase_add_test(tc,eina_unicode_strnlen_test);
   tcase_add_test(tc,eina_unicode_strdup_test);
   tcase_add_test(tc,eina_unicode_strstr_test);
   tcase_add_test(tc,eina_unicode_utf8);
   tcase_add_test(tc,eina_unicode_utf8_conversion);

}

