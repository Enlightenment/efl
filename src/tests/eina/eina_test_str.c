/* EINA - EFL data type library
 * Copyright (C) 2010 Gustavo Sverzut Barbieri
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

   START_TEST(str_simple)
{
   size_t i;
   char *str, *ret;
   const char *escape_ret = "\\ a\\\\x\\'";

   eina_init();

   fail_if(!eina_str_has_prefix("", ""));

   fail_if(!eina_str_has_prefix("x", "x"));
   fail_if(!eina_str_has_prefix("xab", "x"));
   fail_if(!eina_str_has_prefix("xab", "xab"));

   fail_if(eina_str_has_prefix("x", "xab"));
   fail_if(eina_str_has_prefix("xab", "xyz"));
   fail_if(eina_str_has_prefix("", "x"));
   fail_if(eina_str_has_prefix("X", "x"));
   fail_if(eina_str_has_prefix("xAb", "X"));
   fail_if(eina_str_has_prefix("xAb", "xab"));


   fail_if(!eina_str_has_suffix("", ""));

   fail_if(!eina_str_has_suffix("x", "x"));
   fail_if(!eina_str_has_suffix("abx", "x"));
   fail_if(!eina_str_has_suffix("xab", "xab"));

   fail_if(eina_str_has_suffix("x", "xab"));
   fail_if(eina_str_has_suffix("xab", "xyz"));
   fail_if(eina_str_has_suffix("", "x"));
   fail_if(eina_str_has_suffix("X", "x"));
   fail_if(eina_str_has_suffix("aBx", "X"));
   fail_if(eina_str_has_suffix("xaB", "Xab"));


   fail_if(!eina_str_has_extension("", ""));

   fail_if(!eina_str_has_extension("x", "x"));
   fail_if(!eina_str_has_extension("abx", "x"));
   fail_if(!eina_str_has_extension("xab", "xab"));
   fail_if(!eina_str_has_extension("x", "X"));
   fail_if(!eina_str_has_extension("abx", "X"));
   fail_if(!eina_str_has_extension("xab", "Xab"));
   fail_if(!eina_str_has_extension("X", "X"));
   fail_if(!eina_str_has_extension("aBx", "X"));
   fail_if(!eina_str_has_extension("xaB", "Xab"));

   fail_if(eina_str_has_extension("x", "xab"));
   fail_if(eina_str_has_extension("xab", "xyz"));
   fail_if(eina_str_has_extension("", "x"));
   fail_if(eina_str_has_extension("x", "xAb"));
   fail_if(eina_str_has_extension("xab", "xYz"));
   fail_if(eina_str_has_extension("", "x"));

   fail_if(eina_strlen_bounded("abc", 1024) != strlen("abc"));
   fail_if(eina_strlen_bounded("abc", 2) != (size_t)-1);

   str = malloc(sizeof(char) * 4);
   strcpy(str, "bSd");
   eina_str_tolower(&str);
   fail_if(strcmp(str, "bsd") != 0);
   eina_str_toupper(&str);
   fail_if(strcmp(str, "BSD") != 0);
   free(str);

   str = malloc(sizeof(char) * 8);
   strcpy(str, " a\\x'");
   ret = eina_str_escape(str);
   fail_if(strlen(ret) != strlen(escape_ret));
   for (i = 0; i <= strlen(ret); i++)
     fail_if(ret[i] != escape_ret[i]);
   free(str);
   free(ret);

   eina_shutdown();
}
END_TEST

START_TEST(str_split)
{
   char **result;
   unsigned int elements;
   char *str;

   eina_init();

   result = eina_str_split_full(NULL, ":", 1, &elements);
        fail_if(result != NULL);
        fail_if(elements != 0);

   result = eina_str_split_full("nomatch", NULL, 1, &elements);
        fail_if(result != NULL);
        fail_if(elements != 0);

   result = eina_str_split_full("match:match", ":", 1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 1);
        fail_if(strcmp(result[0], "match:match") != 0);
        free(result[0]);
        free(result);

   result = eina_str_split_full("match:match:nomatch:nomatch", ":", 3, &elements);
        fail_if(result == NULL);
        fail_if(elements != 3);
        fail_if(strcmp(result[0], "match") != 0);
        fail_if(strcmp(result[1], "match") != 0);
        fail_if(strcmp(result[2], "nomatch:nomatch") != 0);
        free(result[0]);
        free(result);

   result = eina_str_split_full("nomatch", "", -1, &elements);
        fail_if(result != NULL);
        fail_if(elements != 0);

   result = eina_str_split_full("nomatch", "x", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 1);
        fail_if(strcmp(result[0], "nomatch") != 0);
        fail_if(result[1]);
        free(result[0]);
        free(result);

   result = eina_str_split_full("nomatch", "xyz", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 1);
        fail_if(strcmp(result[0], "nomatch") != 0);
        fail_if(result[1]);
        free(result[0]);
        free(result);

   result = eina_str_split_full("match:match:match", ":", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 3);
   for (elements = 0; elements < 3 - 1; elements++)
     fail_if(strcmp(result[elements], "match") != 0);
     fail_if(result[3]);
        free(result[0]);
        free(result);

   str = malloc(sizeof(char) * 1024);
   str[0] = 0;
   for (elements = 0; elements < 300; elements++)
     strcat(str, "ma:");
   strcat(str, "ma");
   result = eina_str_split_full(str, ":", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 301);
        free(result[0]);
        free(result);
   free(str);

   result = eina_str_split_full("a:b:c", ":", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 3);
        fail_if(strcmp(result[0], "a") != 0);
        fail_if(strcmp(result[1], "b") != 0);
        fail_if(strcmp(result[2], "c") != 0);
        fail_if(result[3]);
        free(result[0]);
        free(result);

   result = eina_str_split_full("a:b:", ":", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 3);
        fail_if(strcmp(result[0], "a") != 0);
        fail_if(strcmp(result[1], "b") != 0);
        fail_if(strcmp(result[2], "") != 0);
        fail_if(result[3]);
        free(result[0]);
        free(result);

   result = eina_str_split_full(":b:c", ":", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 3);
        fail_if(strcmp(result[0], "") != 0);
        fail_if(strcmp(result[1], "b") != 0);
        fail_if(strcmp(result[2], "c") != 0);
        fail_if(result[3]);
        free(result[0]);
        free(result);

   result = eina_str_split_full(":", ":", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 2);
        fail_if(strcmp(result[0], "") != 0);
        fail_if(strcmp(result[1], "") != 0);
        fail_if(result[2]);
        free(result[0]);
        free(result);

   result = eina_str_split_full("a", "!!!!!!!!!", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 1);
        fail_if(strcmp(result[0], "a") != 0);
        fail_if(result[1]);
        free(result[0]);
        free(result);

   result = eina_str_split_full("aaba", "ab", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 2);
        fail_if(strcmp(result[0], "a") != 0);
        fail_if(strcmp(result[1], "a") != 0);
        fail_if(result[2]);
        free(result[0]);
        free(result);

   eina_shutdown();
}
END_TEST

START_TEST(str_lcat_lcpy)
{
   char dst[9];
   size_t ret;
   size_t ds = (sizeof(dst) / sizeof(char));

   eina_init();

   dst[0] = '\0';

   ret = eina_strlcat(dst, "cat1", ds);
        fail_if(ret != 4);
        fail_if(strcmp(dst, "cat1") != 0);

   ret = eina_strlcat(dst, NULL, ds);
        fail_if(ret != 4);
        fail_if(strcmp(dst, "cat1") != 0);

   ret = eina_strlcat(dst, "cat234", ds);
        fail_if(ret != (ds - 1 + 2));
        fail_if(strcmp(dst, "cat1cat2") != 0);

   ret = eina_strlcat(dst, "cat3", ds);
        fail_if(ret != (ds - 1 + 4));
        fail_if(strcmp(dst, "cat1cat2") != 0);

   ret = eina_strlcat(dst, "cat3", ds - 1);
        fail_if(ret != (ds - 1 + 4));
        fail_if(strcmp(dst, "cat1cat2") != 0);

   ret = eina_strlcpy(dst, "copycopy", ds);
        fail_if(ret != 8);
        fail_if(strcmp(dst, "copycopy") != 0);

   ret = eina_strlcpy(dst, "copy2copy2", ds);
        fail_if(ret != 10);
        fail_if(strcmp(dst, "copy2cop") != 0);

   eina_shutdown();
}
END_TEST

START_TEST(str_join_len)
{
   char dst[9];
   size_t ret;
   size_t ds = (sizeof(dst) / sizeof(char));

   eina_init();

   dst[0] = '\0';

   ret = eina_str_join_len(dst, ds, '#', "ab", 2, "cde", 3);
        fail_if(ret != 6);
        fail_if(strcmp(dst, "ab#cde") != 0);

   ret = eina_str_join_len(dst, ds, '#', "abcdefghi", 9, "cde", 3);
        fail_if(ret != 13);
        fail_if(strcmp(dst, "abcdefgh") != 0);

   ret = eina_str_join_len(dst, ds, '#', "abcdefgh", 8, "cde", 3);
        fail_if(ret != 12);
        fail_if(strcmp(dst, "abcdefgh") != 0);

   ret = eina_str_join_len(dst, ds, '#', "abcd", 4, "efgh", 4);
        fail_if(ret != 9);
        fail_if(strcmp(dst, "abcd#efg") != 0);

   eina_shutdown();
}
END_TEST

#ifdef HAVE_ICONV
START_TEST(str_convert)
{
   char *utf8 = "\xc3\xa9\xc3\xa1\xc3\xba\xc3\xad\xc3\xa8\xc3\xa0\xc3\xb9\xc3\xac\xc3\xab\xc3\xa4\xc3\xbc\xc3\xaf";
   char *utf16 = "\xe9\x0\xe1\x0\xfa\x0\xed\x0\xe8\x0\xe0\x0\xf9\x0\xec\x0\xeb\x0\xe4\x0\xfc\x0\xef\x0";
   char *ret;
   size_t ret_sz;
   int i;

   eina_init();

   ret = eina_str_convert("UTF-8", "UTF-16LE", utf8);
        fail_if(ret == NULL);
        for( i=0; i<24; i++)
          fail_if(ret[i] != utf16[i]);

   free(ret);

   ret = eina_str_convert_len("UTF-8", "UTF-16LE", utf8, 24, &ret_sz);
        fail_if(ret == NULL);
        fail_if(ret_sz != 24);
        for( i=0; i<24; i++)
          fail_if(ret[i] != utf16[i]);
   free(ret);

   eina_shutdown();
}
END_TEST
#endif

void
eina_test_str(TCase *tc)
{
   tcase_add_test(tc, str_simple);
   tcase_add_test(tc, str_split);
   tcase_add_test(tc, str_lcat_lcpy);
   tcase_add_test(tc, str_join_len);
#ifdef HAVE_ICONV
   tcase_add_test(tc, str_convert);
#endif
}
