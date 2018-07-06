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
   char *file = strdup("/tmp/eet_suite_testXXXXXX");
   int size;
   int tmpfd;

   fail_if(-1 == (tmpfd = mkstemp(file)));
   fail_if(!!close(tmpfd));

   /* Crypt an eet file. */
   ef = eet_open(file, EET_FILE_MODE_WRITE);
   fail_if(!ef);

   fail_if(!eet_write_cipher(ef, "keys/tests", buffer, strlen(buffer) + 1, 0,
                             key));

   eet_close(ef);

   /* Decrypt an eet file. */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   test = eet_read_cipher(ef, "keys/tests", &size, key);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(memcmp(test, buffer, strlen(buffer) + 1) != 0);

   eet_close(ef);

   /* Decrypt an eet file. */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   test = eet_read_cipher(ef, "keys/tests", &size, key_bad);

   if (size == (int)strlen(buffer) + 1)
     fail_if(memcmp(test, buffer, strlen(buffer) + 1) == 0);

   eet_close(ef);

   fail_if(unlink(file) != 0);
}
EFL_END_TEST

void eet_test_cipher(TCase *tc)
{
   tcase_add_test(tc, eet_test_cipher_decipher_simple);
}
