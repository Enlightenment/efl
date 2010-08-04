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

}

