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

   eina_shutdown();
}
END_TEST

START_TEST(str_split)
{
   char **result;
   unsigned int elements;

   eina_init();

   result = eina_str_split_full("nomatch", "", -1, &elements);
        fail_if(result != NULL);
        fail_if(elements != 0);

   result = eina_str_split_full("nomatch", "x", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 1);
        fail_if(strcmp(result[0], "nomatch") != 0);
        free(result[0]);
        free(result);

   result = eina_str_split_full("nomatch", "xyz", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 1);
        fail_if(strcmp(result[0], "nomatch") != 0);
        free(result[0]);
        free(result);

   result = eina_str_split_full("match:match:match", ":", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 3);
   while (elements >= 1)
     {
        elements--;
        fail_if(strcmp(result[elements], "match") != 0);
     }
        free(result[0]);
        free(result);

   result = eina_str_split_full("a:b:c", ":", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 3);
        fail_if(strcmp(result[0], "a") != 0);
        fail_if(strcmp(result[1], "b") != 0);
        fail_if(strcmp(result[2], "c") != 0);
        free(result[0]);
        free(result);

   result = eina_str_split_full("a:b:", ":", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 3);
        fail_if(strcmp(result[0], "a") != 0);
        fail_if(strcmp(result[1], "b") != 0);
        fail_if(strcmp(result[2], "") != 0);
        free(result[0]);
        free(result);

   result = eina_str_split_full(":b:c", ":", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 3);
        fail_if(strcmp(result[0], "") != 0);
        fail_if(strcmp(result[1], "b") != 0);
        fail_if(strcmp(result[2], "c") != 0);
        free(result[0]);
        free(result);

   result = eina_str_split_full(":", ":", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 2);
        fail_if(strcmp(result[0], "") != 0);
        fail_if(strcmp(result[1], "") != 0);
        free(result[0]);
        free(result);

   result = eina_str_split_full("a", "!!!!!!!!!", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 1);
        fail_if(strcmp(result[0], "a") != 0);
        free(result[0]);
        free(result);

   result = eina_str_split_full("aaba", "ab", -1, &elements);
        fail_if(result == NULL);
        fail_if(elements != 2);
        fail_if(strcmp(result[0], "a") != 0);
        fail_if(strcmp(result[1], "a") != 0);
        free(result[0]);
        free(result);

        eina_shutdown();
}
END_TEST

void
eina_test_str(TCase *tc)
{
   tcase_add_test(tc, str_simple);
   tcase_add_test(tc, str_split);
}
