/* EINA - EFL data type library
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

START_TEST(binbuf_simple)
{
   Eina_Binbuf *buf;
   unsigned char *txt;
   const unsigned char cbuf[] = "Null in the middle \0 and more text afterwards and \0 anotehr null just there and another one \0 here.";
   size_t size = sizeof(cbuf) - 1; /* We don't care about the real NULL */


   eina_init();

   buf = eina_binbuf_new();
   fail_if(!buf);

   eina_binbuf_append_length(buf, cbuf, size);
   fail_if(memcmp(eina_binbuf_string_get(buf), cbuf, size));
   fail_if(size != eina_binbuf_length_get(buf));

   eina_binbuf_append_length(buf, cbuf, size);
   fail_if(memcmp(eina_binbuf_string_get(buf), cbuf, size));
   fail_if(memcmp(eina_binbuf_string_get(buf) + size, cbuf, size));
   fail_if(2 * size != eina_binbuf_length_get(buf));

   txt = eina_binbuf_string_steal(buf);
   fail_if(memcmp(txt, cbuf, size));
   fail_if(memcmp(txt + size, cbuf, size));
   free(txt);
   fail_if(eina_binbuf_length_get(buf) != 0);

   eina_binbuf_append_length(buf, cbuf, size);
   fail_if(memcmp(eina_binbuf_string_get(buf), cbuf, size));
   fail_if(size != eina_binbuf_length_get(buf));

   eina_binbuf_reset(buf);
   fail_if(eina_binbuf_length_get(buf) != 0);

   eina_binbuf_free(buf);

   eina_shutdown();
#undef TEXT
}
END_TEST

START_TEST(binbuf_remove)
{
   Eina_Binbuf *buf;
   const unsigned char cbuf[] = "12\0 456 78\0 abcthis is some more random junk here!";
   size_t size = sizeof(cbuf) - 1; /* We don't care about the real NULL */

   eina_init();

   buf = eina_binbuf_new();
   fail_if(!buf);

   eina_binbuf_append_length(buf, cbuf, size);
   fail_if(size != eina_binbuf_length_get(buf));
   eina_binbuf_remove(buf, 0, 4);
   fail_if(size - 4 != eina_binbuf_length_get(buf));
   eina_binbuf_remove(buf, 8, 1000);
   fail_if(8 != eina_binbuf_length_get(buf));
   eina_binbuf_remove(buf, 7, eina_binbuf_length_get(buf));
   fail_if(7 != eina_binbuf_length_get(buf));
   eina_binbuf_remove(buf, 2, 4);
   fail_if(5 != eina_binbuf_length_get(buf));
   eina_binbuf_remove(buf, 4, 1);
   fail_if(5 != eina_binbuf_length_get(buf));
   eina_binbuf_remove(buf, 0, eina_binbuf_length_get(buf));
   fail_if(0 != eina_binbuf_length_get(buf));

   eina_binbuf_free(buf);

   eina_shutdown();
}
END_TEST

START_TEST(binbuf_manage_simple)
{
   Eina_Binbuf *buf;
   const char *_cbuf = "12\0 456 78\0 abcthis is some more random junk here!";
   const unsigned char *cbuf = (const unsigned char *) _cbuf;
   size_t size = sizeof(cbuf) - 1; /* We don't care about the real NULL */
   unsigned char *alloc_buf = malloc(size);
   memcpy(alloc_buf, cbuf, size);

   eina_init();

   buf = eina_binbuf_manage_new_length(alloc_buf, size);
   fail_if(!buf);

   fail_if(memcmp(eina_binbuf_string_get(buf), cbuf, size));
   fail_if(size != eina_binbuf_length_get(buf));
   eina_binbuf_append_length(buf, cbuf, size);
   fail_if(memcmp(eina_binbuf_string_get(buf), cbuf, size));
   fail_if(memcmp(eina_binbuf_string_get(buf) + size, cbuf, size));
   fail_if(2 * size != eina_binbuf_length_get(buf));

   eina_binbuf_free(buf);

   eina_shutdown();
}
END_TEST

START_TEST(binbuf_manage_read_only_simple)
{
   Eina_Binbuf *buf;
   const char *_cbuf = "12\0 456 78\0 abcthis is some more random junk here!";
   const unsigned char *cbuf = (const unsigned char *) _cbuf;
   size_t size = sizeof(cbuf) - 1; /* We don't care about the real NULL */

   eina_init();

   buf = eina_binbuf_manage_read_only_new_length(cbuf, size);
   fail_if(!buf);

   eina_binbuf_free(buf);

   buf = eina_binbuf_manage_read_only_new_length(cbuf, size);
   fail_if(!buf);

   fail_if(eina_binbuf_string_get(buf) != cbuf);
   fail_if(size != eina_binbuf_length_get(buf));
   eina_binbuf_append_length(buf, cbuf, size);
   fail_if(memcmp(eina_binbuf_string_get(buf), cbuf, size));
   fail_if(memcmp(eina_binbuf_string_get(buf) + size, cbuf, size));
   fail_if(2 * size != eina_binbuf_length_get(buf));

   eina_binbuf_free(buf);

   eina_shutdown();
}
END_TEST

START_TEST(binbuf_insert)
{
#if 0
   Eina_Binbuf *buf;

   eina_init();

   buf = eina_binbuf_new();
   fail_if(!buf);

   eina_binbuf_insert(buf, "abc", 10);
   fail_if(strlen(eina_binbuf_string_get(buf)) != eina_binbuf_length_get(buf));
   fail_if(strcmp(eina_binbuf_string_get(buf), "abc"));

   eina_binbuf_insert(buf, "123", 0);
   fail_if(strlen(eina_binbuf_string_get(buf)) != eina_binbuf_length_get(buf));
   fail_if(strcmp(eina_binbuf_string_get(buf), "123abc"));

   eina_binbuf_insert(buf, "xyz", eina_binbuf_length_get(buf));
   fail_if(strlen(eina_binbuf_string_get(buf)) != eina_binbuf_length_get(buf));
   fail_if(strcmp(eina_binbuf_string_get(buf), "123abcxyz"));

   eina_binbuf_insert(buf, "xyz", 1);
   fail_if(strlen(eina_binbuf_string_get(buf)) != eina_binbuf_length_get(buf));
   fail_if(strcmp(eina_binbuf_string_get(buf), "1xyz23abcxyz"));

   eina_binbuf_insert_n(buf, "ABCDEF", 2, 1);
   fail_if(strlen(eina_binbuf_string_get(buf)) != eina_binbuf_length_get(buf));
   fail_if(strcmp(eina_binbuf_string_get(buf), "1ABxyz23abcxyz"));

   eina_binbuf_insert_n(buf, "EINA", 2, 3);
   fail_if(strlen(eina_binbuf_string_get(buf)) != eina_binbuf_length_get(buf));
   fail_if(strcmp(eina_binbuf_string_get(buf), "1ABEIxyz23abcxyz"));

   eina_binbuf_insert_escaped(buf, "678", 3);
   fail_if(strlen(eina_binbuf_string_get(buf)) != eina_binbuf_length_get(buf));
   fail_if(strncmp(eina_binbuf_string_get(buf) + 3, "678", 3));

   eina_binbuf_insert_escaped(buf, "089 '\\", 9);
                   fail_if(strlen(eina_binbuf_string_get(
                     buf)) != eina_binbuf_length_get(buf));
                   fail_if(strncmp(eina_binbuf_string_get(buf) + 9,
                   "089\\ \\'\\\\",
                   strlen("089\\ \\'\\\\")));
   eina_binbuf_reset(buf);

   eina_binbuf_free(buf);

   eina_shutdown();
#endif
}
END_TEST

START_TEST(binbuf_realloc)
{
   Eina_Binbuf *buf;
   unsigned char pattern[1024 * 16];
   unsigned int i;
   size_t sz;

   for (i = 0; i < sizeof(pattern) - 1; i++)
     {
        if (i % 27 == 26)
           pattern[i] = '\0';
        else
           pattern[i] = 'a' + (i % 27);
     }
   pattern[i] = '\0';

   eina_init();

   buf = eina_binbuf_new();
   fail_if(!buf);

   sz = 0;

   eina_binbuf_append_length(buf, pattern, 1);
   fail_if(eina_binbuf_length_get(buf) != sz + 1);
   fail_if(memcmp(eina_binbuf_string_get(buf) + sz, pattern, 1));
   sz += 1;

   eina_binbuf_append_length(buf, pattern, 32);
   fail_if(eina_binbuf_length_get(buf) != sz + 32);
   fail_if(memcmp(eina_binbuf_string_get(buf) + sz, pattern, 32));
   sz += 32;

   eina_binbuf_append_length(buf, pattern, 64);
   fail_if(eina_binbuf_length_get(buf) != sz + 64);
   fail_if(memcmp(eina_binbuf_string_get(buf) + sz, pattern, 64));
   sz += 64;

   eina_binbuf_append_length(buf, pattern, 128);
   fail_if(eina_binbuf_length_get(buf) != sz + 128);
   fail_if(memcmp(eina_binbuf_string_get(buf) + sz, pattern, 128));
   sz += 128;

   eina_binbuf_append_length(buf, pattern, 4096);
   fail_if(eina_binbuf_length_get(buf) != sz + 4096);
   fail_if(memcmp(eina_binbuf_string_get(buf) + sz, pattern, 4096));
   sz += 4096;

   eina_binbuf_append_length(buf, pattern, sizeof(pattern) - 1);
   fail_if(eina_binbuf_length_get(buf) != sz + sizeof(pattern) - 1);
   fail_if(memcmp(eina_binbuf_string_get(buf) + sz, pattern, sizeof(pattern) -
                  1));
   sz += sizeof(pattern) - 1;


   eina_binbuf_remove(buf, 1024, 1024 + 1234);
   fail_if(eina_binbuf_length_get(buf) != sz - 1234);
   sz -= 1234;

   eina_binbuf_remove(buf, 0, 0 + 8192);
   fail_if(eina_binbuf_length_get(buf) != sz - 8192);
   sz -= 8192;

   eina_binbuf_remove(buf, 0, 0 + 32);
   fail_if(eina_binbuf_length_get(buf) != sz - 32);
   sz -= 32;


   eina_binbuf_free(buf);

   eina_shutdown();
}
END_TEST

void
eina_test_binbuf(TCase *tc)
{
   tcase_add_test(tc, binbuf_simple);
   tcase_add_test(tc, binbuf_remove);
   tcase_add_test(tc, binbuf_insert);
   tcase_add_test(tc, binbuf_realloc);
   tcase_add_test(tc, binbuf_manage_simple);
   tcase_add_test(tc, binbuf_manage_read_only_simple);
}
