/* EINA - EFL data type library
 * Copyright (C) 2010 Sebastian Dransfeld
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

#include <Eina.h>

#include "eina_suite.h"

EFL_START_TEST(eina_test_strbuf_simple)
{
   Eina_Strbuf *buf;
   char *txt;
   Eina_Slice ro_slice;
   Eina_Rw_Slice rw_slice;
#define TEST_TEXT \
  "This test should be so long that it is longer than the initial size of strbuf"

   buf = eina_strbuf_new();
   fail_if(!buf);

   eina_strbuf_append(buf, TEST_TEXT);
   fail_if(strcmp(eina_strbuf_string_get(buf), TEST_TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_append(buf, TEST_TEXT);
   fail_if(strcmp(eina_strbuf_string_get(buf), TEST_TEXT TEST_TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   txt = eina_strbuf_string_steal(buf);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(txt, TEST_TEXT TEST_TEXT));
   free(txt);
   fail_if(eina_strbuf_length_get(buf) != 0);
   fail_if(!strcmp(eina_strbuf_string_get(buf), TEST_TEXT TEST_TEXT));
   eina_strbuf_append(buf, TEST_TEXT);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_reset(buf);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(eina_strbuf_length_get(buf) != 0);
   fail_if(!strcmp(eina_strbuf_string_get(buf), TEST_TEXT));

   ro_slice = eina_strbuf_slice_get(buf);
   fail_if(ro_slice.len != eina_strbuf_length_get(buf));
   fail_if(ro_slice.mem != eina_strbuf_string_get(buf));

   rw_slice = eina_strbuf_rw_slice_get(buf);
   fail_if(rw_slice.len != eina_strbuf_length_get(buf));
   fail_if(rw_slice.mem != eina_strbuf_string_get(buf));

   eina_strbuf_string_free(buf);
   fail_if(eina_strbuf_length_get(buf));
   eina_strbuf_append(buf, TEST_TEXT);
   fail_if(strcmp(eina_strbuf_string_get(buf), TEST_TEXT));

   eina_strbuf_free(buf);

#undef TEST_TEXT
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_manage_simple)
{
   Eina_Strbuf *buf;
   char *txt;
#define TEST_TEXT \
  "This test should be so long that it is longer than the initial size of strbuf"

   txt = strdup(TEST_TEXT);

   buf = eina_strbuf_manage_new(txt);
   fail_if(!buf);

   fail_if(strcmp(eina_strbuf_string_get(buf), TEST_TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_append(buf, TEST_TEXT);
   fail_if(strcmp(eina_strbuf_string_get(buf), TEST_TEXT TEST_TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   txt = eina_strbuf_string_steal(buf);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(txt, TEST_TEXT TEST_TEXT));
   free(txt);
   fail_if(eina_strbuf_length_get(buf) != 0);
   fail_if(!strcmp(eina_strbuf_string_get(buf), TEST_TEXT TEST_TEXT));
   eina_strbuf_append(buf, TEST_TEXT);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_reset(buf);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(eina_strbuf_length_get(buf) != 0);
   fail_if(!strcmp(eina_strbuf_string_get(buf), TEST_TEXT));

   eina_strbuf_free(buf);

   buf = eina_strbuf_manage_read_only_new_length(TEST_TEXT, strlen(TEST_TEXT));
   fail_if(!buf);

   fail_if(strcmp(eina_strbuf_string_get(buf), TEST_TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_append(buf, TEST_TEXT);
   fail_if(strcmp(eina_strbuf_string_get(buf), TEST_TEXT TEST_TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   txt = eina_strbuf_string_steal(buf);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(txt, TEST_TEXT TEST_TEXT));
   free(txt);

#undef TEST_TEXT
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_remove)
{
   Eina_Strbuf *buf;

   buf = eina_strbuf_new();
   fail_if(!buf);

   eina_strbuf_append(buf, "123 456 789 abc");
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_remove(buf, 0, 4);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "456 789 abc"));
   eina_strbuf_remove(buf, 8, 1000);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "456 789 "));
   eina_strbuf_remove(buf, 7, eina_strbuf_length_get(buf));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "456 789"));
   eina_strbuf_remove(buf, 2, 4);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "45789"));
   eina_strbuf_remove(buf, 4, 1);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "45789"));
   eina_strbuf_remove(buf, 0, eina_strbuf_length_get(buf));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), ""));

#define TEST_TEXT \
  "This test should be so long that it is longer than the initial size of strbuf"
   eina_strbuf_append(buf, TEST_TEXT TEST_TEXT);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_remove(buf, 0, eina_strbuf_length_get(buf) - 1);
   fail_if(strcmp(eina_strbuf_string_get(buf), "f"));
#undef TEST_TEXT

   eina_strbuf_free(buf);
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_append)
{
   Eina_Strbuf *buf;
   Eina_Slice ro_slice = EINA_SLICE_STR_LITERAL("somethingELSE");

   ro_slice.len -= strlen("ELSE");

   buf = eina_strbuf_new();
   fail_if(!buf);

   eina_strbuf_append(buf, "abc");
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "abc"));
   eina_strbuf_reset(buf);

   eina_strbuf_append_escaped(buf, "abc");
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "abc"));
   eina_strbuf_reset(buf);

   eina_strbuf_append_escaped(buf, "abc '\\");
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "abc\\ \\'\\\\"));
   eina_strbuf_reset(buf);

   eina_strbuf_append_n(buf, "abc", 2);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "ab"));
   eina_strbuf_reset(buf);

   eina_strbuf_append_char(buf, 'a');
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "a"));
   eina_strbuf_reset(buf);

   eina_strbuf_append_length(buf, "something", strlen("something"));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "something"));
   eina_strbuf_reset(buf);

   eina_strbuf_append_slice(buf, ro_slice);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "something"));
   eina_strbuf_reset(buf);

   eina_strbuf_free(buf);
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_insert)
{
   Eina_Strbuf *buf;
   Eina_Slice ro_slice = EINA_SLICE_STR_LITERAL("EINA");

   ro_slice.len = 2;

   buf = eina_strbuf_new();
   fail_if(!buf);

   eina_strbuf_insert(buf, "abc", 10);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "abc"));

   eina_strbuf_insert(buf, "123", 0);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "123abc"));

   eina_strbuf_insert(buf, "xyz", eina_strbuf_length_get(buf));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "123abcxyz"));

   eina_strbuf_insert(buf, "xyz", 1);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "1xyz23abcxyz"));

   eina_strbuf_insert_n(buf, "ABCDEF", 2, 1);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "1ABxyz23abcxyz"));

   eina_strbuf_insert_slice(buf, ro_slice, 3);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "1ABEIxyz23abcxyz"));

   eina_strbuf_insert_escaped(buf, "678", 3);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strncmp(eina_strbuf_string_get(buf) + 3, "678", 3));

   eina_strbuf_insert_escaped(buf, "089 '\\", 9);
   fail_if(strlen(eina_strbuf_string_get(
                    buf)) != eina_strbuf_length_get(buf));
   fail_if(strncmp(eina_strbuf_string_get(buf) + 9,
                   "089\\ \\'\\\\",
                   strlen("089\\ \\'\\\\")));
   eina_strbuf_reset(buf);

   eina_strbuf_free(buf);
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_replace)
{
   Eina_Strbuf *buf;

   buf = eina_strbuf_new();
   fail_if(!buf);

   eina_strbuf_append(buf, "aaa");
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "aaa"));

   fail_if(!eina_strbuf_replace(buf, "a", "b", 1));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "baa"));

   fail_if(eina_strbuf_replace_all(buf, "a", "b") != 2);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "bbb"));

   fail_if(!eina_strbuf_replace(buf, "b", "cc", 2));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "bccb"));

   fail_if(eina_strbuf_replace_all(buf, "c", "aa") != 2);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "baaaab"));

   fail_if(eina_strbuf_replace(buf, "c", "aa", 0));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "baaaab"));

   fail_if(eina_strbuf_replace(buf, "c", "aa", 2));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "baaaab"));

   fail_if(eina_strbuf_replace_all(buf, "c", "aa") != 0);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "baaaab"));

   eina_strbuf_free(buf);
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_realloc)
{
   Eina_Strbuf *buf;
   char pattern[1024 * 16];
   unsigned int i;
   size_t sz;

   for (i = 0; i < sizeof(pattern) - 1; i++)
     pattern[i] = 'a' + (i % 26);
   pattern[i] = '\0';

   buf = eina_strbuf_new();
   fail_if(!buf);

   sz = 0;

   eina_strbuf_append_length(buf, pattern, 1);
   fail_if(eina_strbuf_length_get(buf) != sz + 1);
   fail_if(memcmp(eina_strbuf_string_get(buf) + sz, pattern, 1));
   sz += 1;

   eina_strbuf_append_length(buf, pattern, 32);
   fail_if(eina_strbuf_length_get(buf) != sz + 32);
   fail_if(memcmp(eina_strbuf_string_get(buf) + sz, pattern, 32));
   sz += 32;

   eina_strbuf_append_length(buf, pattern, 64);
   fail_if(eina_strbuf_length_get(buf) != sz + 64);
   fail_if(memcmp(eina_strbuf_string_get(buf) + sz, pattern, 64));
   sz += 64;

   eina_strbuf_append_length(buf, pattern, 128);
   fail_if(eina_strbuf_length_get(buf) != sz + 128);
   fail_if(memcmp(eina_strbuf_string_get(buf) + sz, pattern, 128));
   sz += 128;

   eina_strbuf_append_length(buf, pattern, 4096);
   fail_if(eina_strbuf_length_get(buf) != sz + 4096);
   fail_if(memcmp(eina_strbuf_string_get(buf) + sz, pattern, 4096));
   sz += 4096;

   eina_strbuf_append_length(buf, pattern, sizeof(pattern) - 1);
   fail_if(eina_strbuf_length_get(buf) != sz + sizeof(pattern) - 1);
   fail_if(memcmp(eina_strbuf_string_get(buf) + sz, pattern, sizeof(pattern) -
                  1));
   sz += sizeof(pattern) - 1;

   eina_strbuf_remove(buf, 1024, 1024 + 1234);
   fail_if(eina_strbuf_length_get(buf) != sz - 1234);
   sz -= 1234;

   eina_strbuf_remove(buf, 0, 0 + 8192);
   fail_if(eina_strbuf_length_get(buf) != sz - 8192);
   sz -= 8192;

   eina_strbuf_remove(buf, 0, 0 + 32);
   fail_if(eina_strbuf_length_get(buf) != sz - 32);

   eina_strbuf_free(buf);
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_append_realloc)
{
   Eina_Strbuf *buf;
   const size_t runs = 3 * 10;
   const char target_pattern[] = "stringstrsstr";
   const char *str;
   size_t i, target_pattern_size;

   buf = eina_strbuf_new();
   fail_if(!buf);

   for (i = 0; i < runs; i++)
     {
        ck_assert_int_ne(eina_strbuf_append(buf, "string"), 0); //6
        ck_assert_int_ne(eina_strbuf_append_n(buf, "string", 3), 0); //3
        ck_assert_int_ne(eina_strbuf_append_char(buf, 's'), 0); //1
        ck_assert_int_ne(eina_strbuf_append_length(buf, "string", 3), 0); //3
     }

   target_pattern_size = strlen(target_pattern);
   ck_assert_int_eq(eina_strbuf_length_get(buf), runs * target_pattern_size);

   str = eina_strbuf_string_get(buf);
   ck_assert_ptr_ne(str, NULL);
   for (i = 0; i < runs; i++, str += target_pattern_size)
     ck_assert_int_eq(memcmp(str, target_pattern, target_pattern_size), 0);

   eina_strbuf_free(buf);
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_prepend_realloc)
{
   Eina_Strbuf *buf;
   const size_t runs = 3 * 10;
   const char target_pattern[] = "strsstrstring";
   const char *str;
   size_t i, target_pattern_size;

   buf = eina_strbuf_new();
   fail_if(!buf);

   for (i = 0; i < runs; i++)
     {
        ck_assert_int_ne(eina_strbuf_prepend(buf, "string"), 0); //6
        ck_assert_int_ne(eina_strbuf_prepend_n(buf, "string", 3), 0); //3
        ck_assert_int_ne(eina_strbuf_prepend_char(buf, 's'), 0); //1
        ck_assert_int_ne(eina_strbuf_prepend_length(buf, "string", 3), 0); //3
     }

   target_pattern_size = strlen(target_pattern);
   ck_assert_int_eq(eina_strbuf_length_get(buf), runs * target_pattern_size);

   str = eina_strbuf_string_get(buf);
   ck_assert_ptr_ne(str, NULL);
   for (i = 0; i < runs; i++, str += target_pattern_size)
     ck_assert_int_eq(memcmp(str, target_pattern, target_pattern_size), 0);

   eina_strbuf_free(buf);
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_trim)
{
   Eina_Strbuf *buf;
   const char *str;

   buf = eina_strbuf_new();
   fail_unless(buf != NULL);

   eina_strbuf_append(buf, "  string     ");
   eina_strbuf_trim(buf);
   str = eina_strbuf_string_get(buf);
   fail_if(str == NULL || strcmp(str, "string") != 0);
   eina_strbuf_reset(buf);

   eina_strbuf_append(buf, "  string     ");
   eina_strbuf_ltrim(buf);
   str = eina_strbuf_string_get(buf);
   fail_if(str == NULL || strcmp(str, "string     ") != 0);
   eina_strbuf_rtrim(buf);
   str = eina_strbuf_string_get(buf);
   fail_if(str == NULL || strcmp(str, "string") != 0);
   eina_strbuf_reset(buf);

   eina_strbuf_append(buf, "             ");
   eina_strbuf_trim(buf);
   str = eina_strbuf_string_get(buf);
   fail_if(str == NULL || strcmp(str, "") != 0);
   eina_strbuf_reset(buf);

   eina_strbuf_append(buf, "             ");
   eina_strbuf_rtrim(buf);
   str = eina_strbuf_string_get(buf);
   fail_if(str == NULL || strcmp(str, "") != 0);
   eina_strbuf_reset(buf);

   eina_strbuf_append(buf, "             ");
   eina_strbuf_ltrim(buf);
   str = eina_strbuf_string_get(buf);
   fail_if(str == NULL || strcmp(str, "") != 0);
   eina_strbuf_reset(buf);

   eina_strbuf_free(buf);
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_tolower)
{
   Eina_Strbuf *buf;
   const char *str;

   buf = eina_strbuf_new();
   fail_unless(buf != NULL);

   eina_strbuf_append(buf, "UPPER");
   eina_strbuf_tolower(buf);
   str = eina_strbuf_string_get(buf);
   fail_unless(str || strcmp(str, "upper"));

   eina_strbuf_tolower(buf);
   str = eina_strbuf_string_get(buf);
   fail_unless(str || strcmp(str, "upper"));

   eina_strbuf_append(buf, "1@ ");
   eina_strbuf_tolower(buf);
   str = eina_strbuf_string_get(buf);
   fail_unless(str || strcmp(str, "upper1@ "));

   eina_strbuf_free(buf);
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_substr_get)
{
   Eina_Strbuf *buf, *substr;
   const char *str;

   buf = eina_strbuf_new();
   fail_unless(buf != NULL);

   eina_strbuf_append(buf, "string");
   substr = eina_strbuf_substr_get(buf, 2, 3);
   str = eina_strbuf_string_get(substr);
   fail_unless(str && !strcmp(str, "rin"));
   eina_strbuf_free(substr);

   substr = eina_strbuf_substr_get(buf, 0, 6);
   str = eina_strbuf_string_get(substr);
   fail_unless(str && !strcmp(str, "string"));
   eina_strbuf_free(substr);

   substr = eina_strbuf_substr_get(buf, 6, 0);
   str = eina_strbuf_string_get(substr);
   fail_unless(str && !strcmp(str, ""));
   eina_strbuf_free(substr);

   substr = eina_strbuf_substr_get(buf, 6, 1);
   str = eina_strbuf_string_get(substr);
   fail_if(str);
   eina_strbuf_free(substr);

   substr = eina_strbuf_substr_get(buf, 0, 7);
   str = eina_strbuf_string_get(substr);
   fail_if(str);
   eina_strbuf_free(substr);

   substr = eina_strbuf_substr_get(NULL, 0, 7);
   fail_if(substr);

   eina_strbuf_free(buf);
}
EFL_END_TEST

static const char *
test_prepend_vprintf(Eina_Strbuf *buf, char *fmt, va_list ap)
{
   const char *str;

   eina_strbuf_prepend_vprintf(buf, fmt, ap);
   str = eina_strbuf_string_get(buf);
   return str;
}

static const char *
test_prepend(Eina_Strbuf *buf, ...)
{
   const char *str;

   va_list ap;

   va_start(ap, buf);
   str = test_prepend_vprintf(buf, "%s %s %s %s", ap);
   return str;
}

EFL_START_TEST(eina_test_strbuf_prepend_print)
{
   Eina_Bool ret;
   Eina_Strbuf *buf;
   const char *str;

   buf = eina_strbuf_new();
   fail_unless(buf != NULL);

   ret = eina_strbuf_prepend_printf(buf, "%s", "string");
   fail_if(ret != EINA_TRUE);
   str = eina_strbuf_string_get(buf);
   fail_if(str == NULL || strcmp(str, "string") != 0);

   ret = eina_strbuf_prepend_printf(buf, "This is %d ", 1);
   fail_if(ret != EINA_TRUE);
   str = eina_strbuf_string_get(buf);
   fail_if(str == NULL || strcmp(str, "This is 1 string") != 0);

   ret = eina_strbuf_prepend_printf(buf, "%s   ", "hello");
   fail_if(ret != EINA_TRUE);
   str = eina_strbuf_string_get(buf);
   fail_if(str == NULL || strcmp(str, "hello   This is 1 string") != 0);
   eina_strbuf_reset(buf);

   str = test_prepend(buf, "This", "is", "test", "string");
   fail_if(str == NULL || strcmp(str, "This is test string") != 0);

   eina_strbuf_free(buf);
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_release_test)
{
   Eina_Strbuf *buf;
   char *string;

   buf = eina_strbuf_new();
   ck_assert_ptr_ne(buf, NULL);
   eina_strbuf_append(buf, "strbuf_release_test");

   string = eina_strbuf_release(buf);
   ck_assert_str_eq(string, "strbuf_release_test");
}
EFL_END_TEST

EFL_START_TEST(eina_test_strbuf_strftime_test)
{
   Eina_Strbuf *buf;
   time_t curr_time;
   struct tm *info;
   char cbuf[32];
   const char *str;

   curr_time = time(NULL);
   info = localtime(&curr_time);

   strftime(cbuf, 32, "%I:%M%p", info);

   buf = eina_strbuf_new();
   eina_strbuf_append_strftime(buf, "%I:%M%p", info);
   str = eina_strbuf_string_get(buf);
   fail_if(str == NULL || strcmp(str, cbuf) != 0);
   eina_strbuf_reset(buf);

   buf = eina_strbuf_new();
   eina_strbuf_append(buf, "Hours: Minutes");
   eina_strbuf_prepend_strftime(buf, "%I ", info);
   eina_strbuf_insert_strftime(buf, "%M ", info, 10);
   strftime(cbuf, 32, "%I Hours: %M Minutes", info);
   str = eina_strbuf_string_get(buf);
   fail_if(str == NULL || strcmp(str, cbuf) != 0);

   eina_strbuf_free(buf);
}
EFL_END_TEST

void
eina_test_strbuf(TCase *tc)
{
   tcase_add_test(tc, eina_test_strbuf_simple);
   tcase_add_test(tc, eina_test_strbuf_remove);
   tcase_add_test(tc, eina_test_strbuf_append);
   tcase_add_test(tc, eina_test_strbuf_insert);
   tcase_add_test(tc, eina_test_strbuf_replace);
   tcase_add_test(tc, eina_test_strbuf_realloc);
   tcase_add_test(tc, eina_test_strbuf_append_realloc);
   tcase_add_test(tc, eina_test_strbuf_prepend_realloc);
   tcase_add_test(tc, eina_test_strbuf_manage_simple);
   tcase_add_test(tc, eina_test_strbuf_trim);
   tcase_add_test(tc, eina_test_strbuf_tolower);
   tcase_add_test(tc, eina_test_strbuf_substr_get);
   tcase_add_test(tc, eina_test_strbuf_prepend_print);
   tcase_add_test(tc, eina_test_strbuf_release_test);
   tcase_add_test(tc, eina_test_strbuf_strftime_test);
}

