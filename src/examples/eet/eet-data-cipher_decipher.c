//Compile with:
// gcc -o eet-data-cipher_decipher eet-data-cipher_decipher.c `pkg-config --cflags --libs eet eina`

#include <Eina.h>
#include <Eet.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int
main(void)
{
   const char *buffer = "Here is a string of data to save !";
   const char *key = "This is a crypto key";
   const char *key_bad = "This is another crypto key";

   char *file = strdup("/tmp/eet_cipher_example_XXXXXX");
   Eet_File *ef;
   char *test;
   int size;
   int tmpfd;

   eet_init();

   if (-1 == (tmpfd = mkstemp(file)) || !!close(tmpfd))
     {
        fprintf(
                stderr, "ERROR: could not create temporary file (%s) : %s\n",
                file, strerror(errno));
        goto panic;
     }

   /* Crypt an eet file. */
   ef = eet_open(file, EET_FILE_MODE_WRITE);
   if (!ef)
     {
        fprintf(
          stderr, "ERROR: could not access file (%s).\n", file);
        goto error;
     }

   if (!eet_write_cipher(ef, "keys/tests", buffer, strlen(buffer) + 1, 0, key))
     {
        fprintf(
          stderr, "ERROR: could not access file (%s).\n", file);
        goto error;
     }

   eet_close(ef);

   /* Decrypt an eet file. */
   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
        fprintf(
          stderr, "ERROR: could not access file (%s).\n", file);
        goto error;
     }

   test = eet_read_cipher(ef, "keys/tests", &size, key);
   if (!test)
     {
        fprintf(
          stderr, "ERROR: could decript contents on file %s, with key %s.\n",
          file, key);
        goto error;
     }

   if (size != (int)strlen(buffer) + 1)
     {
        fprintf(
          stderr, "ERROR: something is wrong with the decripted data\n");
        goto error;
     }

   if (memcmp(test, buffer, strlen(buffer) + 1) != 0)
     {
        fprintf(
          stderr, "ERROR: something is wrong with the decripted data\n");
        goto error;
     }

   eet_close(ef);

   /* Decrypt an eet file, now using our BAD key!! */
   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
        fprintf(
          stderr, "ERROR: could not access file (%s).\n", file);
        goto error;
     }

   test = eet_read_cipher(ef, "keys/tests", &size, key_bad);

   if (size == (int)strlen(buffer) + 1)
     if (memcmp(test, buffer, strlen(buffer) + 1) == 0)
       {
          fprintf(
            stderr, "ERROR: something is wrong with the contents of %s, as"
                    " we accessed it with a different key and it decripted our"
                    " information right.\n", file);
          goto error;
       }

   eet_close(ef);

error:
   if (unlink(file) != 0)
     {
        fprintf(
          stderr, "ERROR: could not unlink file (%s).\n", file);
     }

panic:
   eet_shutdown();
}

