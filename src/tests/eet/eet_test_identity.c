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

#ifndef O_BINARY
# define O_BINARY 0
#endif

static char _key_pem[PATH_MAX] = "";
static char _cert_pem[PATH_MAX] = "";
static char _key_enc[PATH_MAX] = "";
static char _key_enc_pem[PATH_MAX] = "";
static char _key_enc_none_pem[PATH_MAX] = "";

static int
pass_get(char            *pass,
         int              size,
         EINA_UNUSED int   rwflags,
         EINA_UNUSED void *u)
{
   memset(pass, 0, size);

   if ((int)strlen("password") > size)
     return 0;

   snprintf(pass, size, "%s", "password");
   return strlen(pass);
}

static int
badpass_get(char            *pass,
            int              size,
            EINA_UNUSED int   rwflags,
            EINA_UNUSED void *u)
{
   memset(pass, 0, size);

   if ((int)strlen("bad password") > size)
     return 0;

   snprintf(pass, size, "%s", "bad password");
   return strlen(pass);
}

EFL_START_TEST(eet_test_identity_simple)
{
   const char *buffer = "Here is a string of data to save !";
   const void *tmp;
   Eet_File *ef;
   Eet_Key *k;
   FILE *noread;
   char *test;
   char *file;
   int size;
   int fd;

   file = strdup("/tmp/eet_suite_testXXXXXX");

   fail_if(-1 == (fd = mkstemp(file)));
   fail_if(!!close(fd));
   fail_if(!(noread = fopen("/dev/null", "wb")));

   /* Sign an eet file. */
   ef = eet_open(file, EET_FILE_MODE_WRITE);
   fail_if(!ef);

   fail_if(!eet_write(ef, "keys/tests", buffer, strlen(buffer) + 1, 0));

   k = eet_identity_open(_cert_pem, _key_pem, NULL);
   fail_if(!k);

   fail_if(eet_identity_set(ef, k) != EET_ERROR_NONE);
   eet_identity_print(k, noread);

   eet_close(ef);

   /* Open a signed file. */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   /* check that the certificates match */
   fail_if(!eet_identity_verify(ef, _cert_pem));

   test = eet_read(ef, "keys/tests", &size);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(memcmp(test, buffer, strlen(buffer) + 1) != 0);

   tmp = eet_identity_x509(ef, &size);
   fail_if(tmp == NULL);

   eet_identity_certificate_print(tmp, size, noread);

   eet_close(ef);

   /* As we are changing file contain in less than 1s, this could get unnoticed
      by eet cache system. */
   eet_clearcache();

   /* Corrupting the file. */
   fd = open(file, O_WRONLY | O_BINARY);
   fail_if(fd < 0);

   fail_if(lseek(fd, 200, SEEK_SET) != 200);
   fail_if(write(fd, "42", 2) != 2);
   fail_if(lseek(fd, 50, SEEK_SET) != 50);
   fail_if(write(fd, "42", 2) != 2);
   fail_if(lseek(fd, 88, SEEK_SET) != 88);
   fail_if(write(fd, "42", 2) != 2);

   close(fd);

   /* Attempt to open a modified file. */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(ef);

   fail_if(unlink(file) != 0);

}
EFL_END_TEST

EFL_START_TEST(eet_test_identity_open_simple)
{
   Eet_Key *k = NULL;

   k = eet_identity_open(_cert_pem, _key_pem, NULL);
   fail_if(!k);

   if (k)
     eet_identity_close(k);

}
EFL_END_TEST

EFL_START_TEST(eet_test_identity_open_pkcs8)
{
   Eet_Key *k = NULL;

   k = eet_identity_open(_cert_pem, _key_enc_none_pem, NULL);
   fail_if(!k);

   if (k)
     eet_identity_close(k);

}
EFL_END_TEST

static int
pw_cb(char *buf EINA_UNUSED, int size EINA_UNUSED, int rwflag EINA_UNUSED, void *data EINA_UNUSED)
{
   return 0;
}

EFL_START_TEST(eet_test_identity_open_pkcs8_enc)
{
   Eet_Key *k = NULL;

   k = eet_identity_open(_cert_pem, _key_enc_pem, pw_cb);
   fail_if(k);

   if (k)
     eet_identity_close(k);

   k = eet_identity_open(_cert_pem, _key_enc_pem, &badpass_get);
   fail_if(k);

   if (k)
     eet_identity_close(k);

   k = eet_identity_open(_cert_pem, _key_enc_pem, &pass_get);
   fail_if(!k);

   if (k)
     eet_identity_close(k);

}
EFL_END_TEST

static const char *_cert_dir_find(const char *_argv0)
{
   static char base[PATH_MAX] = "";
   char path[PATH_MAX];
   struct stat st;

   eina_strlcpy(base, TESTS_SRC_DIR, sizeof(base));
   eina_str_join(path, sizeof(path), '/', base, "key.pem");
   if (stat(path, &st) == 0)
     return base;

   if (base[0] != '/')
     {
        snprintf(base, sizeof(base), "%s/%s", TESTS_WD, TESTS_SRC_DIR);
        eina_str_join(path, sizeof(path), '/', base, "key.pem");
        if (stat(path, &st) == 0)
          return base;
     }

   eina_strlcpy(base, _argv0, sizeof(base));
   do
     {
        char *p = strrchr(base, '/');
        if (!p)
          {
             base[0] = '\0';
             break;
          }
        *p = '\0';
        eina_str_join(path, sizeof(path), '/', base, "key.pem");
     }
   while (stat(path, &st) != 0);

   return base;
}

void eet_test_identity(TCase *tc)
{
   const char *base;

   base = _cert_dir_find(argv0);
   eina_str_join(_key_pem, sizeof(_key_pem), '/', base, "key.pem");
   eina_str_join(_cert_pem, sizeof(_cert_pem), '/', base,"cert.pem");
   eina_str_join(_key_enc, sizeof(_key_enc), '/', base, "key.enc");
   eina_str_join(_key_enc_pem, sizeof(_key_enc_pem), '/', base, "key_enc.pem");
   eina_str_join(_key_enc_none_pem, sizeof(_key_enc_none_pem), '/',
                 base, "key_enc_none.pem");

   tcase_add_test(tc, eet_test_identity_simple);
   tcase_add_test(tc, eet_test_identity_open_simple);
   tcase_add_test(tc, eet_test_identity_open_pkcs8);
   tcase_add_test(tc, eet_test_identity_open_pkcs8_enc);
}
