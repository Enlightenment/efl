#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <Eina.h>
#include <Eet.h>

#include "eet_suite.h"
#include "eet_test_common.h"

EFL_START_TEST(eet_test_cipher_decipher_simple)
{
   const char *buffer = "Here is a string of data to save !";
   const char *key = "This is a crypto key";
   const char *key_bad = "This is another crypto key";
   Eet_File *ef;
   char *test;
   Eina_Tmpstr *tmpf = NULL;
   int size;
   int tmpfd;

   fail_if(-1 == (tmpfd = eina_file_mkstemp("eet_suite_testXXXXXX", &tmpf)));
   fail_if(!!close(tmpfd));

   /* Crypt an eet file. */
   ef = eet_open(tmpf, EET_FILE_MODE_WRITE);
   fail_if(!ef);

   fail_if(!eet_write_cipher(ef, "keys/tests", buffer, strlen(buffer) + 1, 0,
                             key));

   eet_close(ef);

   /* Decrypt an eet file. */
   ef = eet_open(tmpf, EET_FILE_MODE_READ);
   fail_if(!ef);

   test = eet_read_cipher(ef, "keys/tests", &size, key);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(memcmp(test, buffer, strlen(buffer) + 1) != 0);

   eet_close(ef);

   /* Decrypt an eet file. */
   ef = eet_open(tmpf, EET_FILE_MODE_READ);
   fail_if(!ef);

   test = eet_read_cipher(ef, "keys/tests", &size, key_bad);

   if (size == (int)strlen(buffer) + 1)
     fail_if(memcmp(test, buffer, strlen(buffer) + 1) == 0);

   eet_close(ef);

   fail_if(unlink(tmpf) != 0);

   eina_tmpstr_del(tmpf);
}
EFL_END_TEST

EFL_START_TEST(eet_test_cipher_decipher_lengths)
{
   const char *key = "123456";
   const char *text = "Lorem ipsum dolor sit amet\n";
   unsigned char buffer[96];
   Eina_Tmpstr *tmpf = NULL;
   Eet_File *ef;
   unsigned char *test;
   int size, tmpfd, i, compression;

   /* Include every length around AES blocks and the four-byte size header. */
   for (i = 0; i < (int)sizeof(buffer); i++)
     buffer[i] = text[i % strlen(text)];

   tmpfd = eina_file_mkstemp("eet_suite_cipherXXXXXX", &tmpf);
   fail_if(tmpfd == -1);
   fail_if(close(tmpfd) != 0);

   for (compression = 0; compression <= 1; compression++)
     {
        ef = eet_open(tmpf, EET_FILE_MODE_WRITE);
        fail_if(!ef);
        fail_if(!eet_write_cipher(ef, "key_passwd", buffer, _i,
                                 compression, key));
        fail_if(eet_close(ef) != EET_ERROR_NONE);

        ef = eet_open(tmpf, EET_FILE_MODE_READ);
        fail_if(!ef);
        test = eet_read_cipher(ef, "key_passwd", &size, key);
        fail_if(!test);
        ck_assert_int_eq(size, _i);
        fail_if(memcmp(test, buffer, _i) != 0);
        free(test);
        fail_if(eet_close(ef) != EET_ERROR_NONE);
     }

   fail_if(unlink(tmpf) != 0);
   eina_tmpstr_del(tmpf);
}
EFL_END_TEST

void eet_test_cipher(TCase *tc)
{
   tcase_add_test(tc, eet_test_cipher_decipher_simple);
   tcase_add_loop_test(tc, eet_test_cipher_decipher_lengths, 1, 97);
}
