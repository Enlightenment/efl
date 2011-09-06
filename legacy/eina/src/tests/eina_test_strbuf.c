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

#include "eina_suite.h"
#include "Eina.h"

START_TEST(strbuf_simple)
{
   Eina_Strbuf *buf;
   char *txt;
#define TEXT \
   "This test should be so long that it is longer than the initial size of strbuf"

   eina_init();

   buf = eina_strbuf_new();
   fail_if(!buf);

   eina_strbuf_append(buf, TEXT);
   fail_if(strcmp(eina_strbuf_string_get(buf), TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_append(buf, TEXT);
   fail_if(strcmp(eina_strbuf_string_get(buf), TEXT TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   txt = eina_strbuf_string_steal(buf);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(txt, TEXT TEXT));
   free(txt);
   fail_if(eina_strbuf_length_get(buf) != 0);
   fail_if(!strcmp(eina_strbuf_string_get(buf), TEXT TEXT));
   eina_strbuf_append(buf, TEXT);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_reset(buf);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(eina_strbuf_length_get(buf) != 0);
   fail_if(!strcmp(eina_strbuf_string_get(buf), TEXT));

   eina_strbuf_free(buf);

   eina_shutdown();
#undef TEXT
}
END_TEST

START_TEST(strbuf_manage_simple)
{
   Eina_Strbuf *buf;
   char *txt;
#define TEXT \
   "This test should be so long that it is longer than the initial size of strbuf"

   eina_init();

   txt = strdup(TEXT);

   buf = eina_strbuf_manage_new(txt);
   fail_if(!buf);

   fail_if(strcmp(eina_strbuf_string_get(buf), TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_append(buf, TEXT);
   fail_if(strcmp(eina_strbuf_string_get(buf), TEXT TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   txt = eina_strbuf_string_steal(buf);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(txt, TEXT TEXT));
   free(txt);
   fail_if(eina_strbuf_length_get(buf) != 0);
   fail_if(!strcmp(eina_strbuf_string_get(buf), TEXT TEXT));
   eina_strbuf_append(buf, TEXT);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_reset(buf);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(eina_strbuf_length_get(buf) != 0);
   fail_if(!strcmp(eina_strbuf_string_get(buf), TEXT));

   eina_strbuf_free(buf);

   eina_shutdown();
#undef TEXT
}
END_TEST

START_TEST(strbuf_remove)
{
   Eina_Strbuf *buf;

   eina_init();

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

#define TEXT \
   "This test should be so long that it is longer than the initial size of strbuf"
   eina_strbuf_append(buf, TEXT TEXT);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_remove(buf, 0, eina_strbuf_length_get(buf) - 1);
   fail_if(strcmp(eina_strbuf_string_get(buf), "f"));
#undef TEXT

   eina_strbuf_free(buf);

   eina_shutdown();
}
END_TEST

START_TEST(strbuf_append)
{
   Eina_Strbuf *buf;

   eina_init();

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

   eina_strbuf_append_length(buf, "somethingELSE", strlen("something"));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "something"));
   eina_strbuf_reset(buf);

   eina_strbuf_free(buf);

   eina_shutdown();
}
END_TEST

START_TEST(strbuf_insert)
{
   Eina_Strbuf *buf;

   eina_init();

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

   eina_strbuf_insert_n(buf, "EINA", 2, 3);
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

   eina_shutdown();
}
END_TEST

START_TEST(strbuf_replace)
{
   Eina_Strbuf *buf;

   eina_init();

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

   eina_shutdown();
}
END_TEST

START_TEST(strbuf_realloc)
{
   Eina_Strbuf *buf;
   char pattern[1024 * 16];
   unsigned int i;
   size_t sz;

   for (i = 0; i < sizeof(pattern) - 1; i++)
      pattern[i] = 'a' + (i % 26);
   pattern[i] = '\0';

   eina_init();

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
   sz -= 32;


   eina_strbuf_free(buf);

   eina_shutdown();
}
END_TEST

START_TEST(strbuf_append_realloc)
{
   Eina_Strbuf *buf;
   const size_t runs = 40960;
   const char target_pattern[] = "stringstrsstr";
   const char *str;
   size_t i, target_pattern_size;

   eina_init();

   buf = eina_strbuf_new();
        fail_if(!buf);

   for (i = 0; i < runs; i++)
     {
        fail_if(!eina_strbuf_append(buf, "string"));
        fail_if(!eina_strbuf_append_n(buf, "string", 3));
        fail_if(!eina_strbuf_append_char(buf, 's'));
        fail_if(!eina_strbuf_append_length(buf, "string", 3));
     }

   target_pattern_size = strlen(target_pattern);
        fail_if(eina_strbuf_length_get(buf) != (runs * target_pattern_size));

   str = eina_strbuf_string_get(buf);
        fail_if(str == NULL);
   for (i = 0; i < runs; i++, str += target_pattern_size)
        fail_if(memcmp(str, target_pattern, target_pattern_size));

        eina_strbuf_free(buf);

   eina_shutdown();
}
END_TEST

START_TEST(strbuf_prepend_realloc)
{
   Eina_Strbuf *buf;
   const size_t runs = 40960;
   const char target_pattern[] = "strsstrstring";
   const char *str;
   size_t i, target_pattern_size;

   eina_init();

   buf = eina_strbuf_new();
        fail_if(!buf);

   for (i = 0; i < runs; i++)
     {
        fail_if(!eina_strbuf_prepend(buf, "string"));
        fail_if(!eina_strbuf_prepend_n(buf, "string", 3));
        fail_if(!eina_strbuf_prepend_char(buf, 's'));
        fail_if(!eina_strbuf_prepend_length(buf, "string", 3));
     }

   target_pattern_size = strlen(target_pattern);
        fail_if(eina_strbuf_length_get(buf) != (runs * target_pattern_size));

   str = eina_strbuf_string_get(buf);
        fail_if(str == NULL);
   for (i = 0; i < runs; i++, str += target_pattern_size)
        fail_if(memcmp(str, target_pattern, target_pattern_size));

        eina_strbuf_free(buf);

   eina_shutdown();
}
END_TEST

void
eina_test_strbuf(TCase *tc)
{
   tcase_add_test(tc, strbuf_simple);
   tcase_add_test(tc, strbuf_remove);
   tcase_add_test(tc, strbuf_append);
   tcase_add_test(tc, strbuf_insert);
   tcase_add_test(tc, strbuf_replace);
   tcase_add_test(tc, strbuf_realloc);
   tcase_add_test(tc, strbuf_append_realloc);
   tcase_add_test(tc, strbuf_prepend_realloc);
   tcase_add_test(tc, strbuf_manage_simple);
}
