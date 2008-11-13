/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Eet.h>

static void
do_eet_list(const char *file)
{
   int i, num;
   char **list;
   Eet_File *ef;

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
	printf("cannot open for reading: %s\n", file);
	exit(-1);
     }
   list = eet_list(ef, "*", &num);
   if (list)
     {
	for (i = 0; i < num; i++)
	  printf("%s\n",list[i]);
	free(list);
     }
   eet_close(ef);
}

static void
do_eet_extract(const char *file, const char *key, const char *out, const char *crypto_key)
{
   Eet_File *ef;
   void *data;
   int size = 0;
   FILE *f;

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
	printf("cannot open for reading: %s\n", file);
	exit(-1);
     }
   data = eet_read_cipher(ef, key, &size, crypto_key);
   if (!data)
     {
	printf("cannot read key %s\n", key);
	exit(-1);
     }
   f = fopen(out, "w");
   if (!f)
     {
	printf("cannot open %s\n", out);
	exit(-1);
     }
   if (fwrite(data, size, 1, f) != 1)
     {
	printf("cannot write to %s\n", out);
	exit(-1);
     }
   fclose(f);
   free(data);
   eet_close(ef);
}

static void
do_eet_decode_dump(void *data, const char *str)
{
   fputs(str, (FILE *)data);
}

static void
do_eet_decode(const char *file, const char *key, const char *out, const char *crypto_key)
{
   Eet_File *ef;
   FILE *f;

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
	printf("cannot open for reading: %s\n", file);
	exit(-1);
     }
   f = fopen(out, "w");
   if (!f)
     {
	printf("cannot open %s\n", out);
	exit(-1);
     }
   if (!eet_data_dump_cipher(ef, key, crypto_key, do_eet_decode_dump, f))
     {
	printf("cannot write to %s\n", out);
	exit(-1);
     }
   fclose(f);
   eet_close(ef);
}

static void
do_eet_insert(const char *file, const char *key, const char *out, int compress, const char *crypto_key)
{
   Eet_File *ef;
   void *data;
   int size = 0;
   FILE *f;

   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   if (!ef)
     ef = eet_open(file, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	printf("cannot open for read+write: %s\n", file);
	exit(-1);
     }
   f = fopen(out, "r");
   if (!f)
     {
	printf("cannot open %s\n", out);
	exit(-1);
     }
   fseek(f, 0, SEEK_END);
   size = ftell(f);
   rewind(f);
   data = malloc(size);
   if (!data)
     {
	printf("cannot allocate %i bytes\n", size);
	exit(-1);
     }
   if (fread(data, size, 1, f) != 1)
     {
	printf("cannot read file %s\n", out);
	exit(-1);
     }
   fclose(f);
   eet_write_cipher(ef, key, data, size, compress, crypto_key);
   free(data);
   eet_close(ef);
}

static void
do_eet_encode(const char *file, const char *key, const char *out, int compress, const char *crypto_key)
{
   Eet_File *ef;
   char *text;
   int textlen = 0;
   int size = 0;
   FILE *f;

   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   if (!ef)
     ef = eet_open(file, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	printf("cannot open for read+write: %s\n", file);
	exit(-1);
     }
   f = fopen(out, "r");
   if (!f)
     {
	printf("cannot open %s\n", out);
	exit(-1);
     }
   fseek(f, 0, SEEK_END);
   textlen = ftell(f);
   rewind(f);
   text = malloc(textlen);
   if (!text)
     {
	printf("cannot allocate %i bytes\n", size);
	exit(-1);
     }
   if (fread(text, textlen, 1, f) != 1)
     {
	printf("cannot read file %s\n", out);
	exit(-1);
     }
   fclose(f);
   if (!eet_data_undump_cipher(ef, key, crypto_key, text, textlen, compress))
     {
        printf("cannot parse %s\n", out);
	exit(-1);
     }
   free(text);
   eet_close(ef);
}

static void
do_eet_remove(const char *file, const char *key)
{
   Eet_File *ef;

   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   if (!ef)
     {
	printf("cannot open for read+write: %s\n", file);
	exit(-1);
     }
   eet_delete(ef, key);
   eet_close(ef);
}

static void
do_eet_check(const char *file)
{
   Eet_File *ef;
   const void *der;
   int der_length;

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
	fprintf(stdout, "checking signature of `%s` failed\n", file);
	exit(-1);
     }

   der = eet_identity_x509(ef, &der_length);

   eet_identity_certificate_print(der, der_length, stdout);

   eet_close(ef);
}

static void
do_eet_sign(const char *file, const char *private_key, const char *public_key)
{
   Eet_File *ef;
   Eet_Key *key;

   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   if (!ef)
     {
	fprintf(stdout, "cannot open for read+write: %s.\n", file);
	exit(-1);
     }

   key = eet_identity_open(public_key, private_key, NULL);

   fprintf(stdout, "Using the following key to sign `%s`.\n", file);
   eet_identity_print(key, stdout);

   eet_identity_set(ef, key);

   eet_close(ef);
}

int
main(int argc, char **argv)
{
   eet_init();
   if (argc < 2)
     {
	help:
	printf("Usage:\n"
	       "  eet -l FILE.EET				     list all keys in FILE.EET\n"
	       "  eet -x FILE.EET KEY OUT-FILE [CRYPTO_KEY]          extract data stored in KEY in FILE.EET and write to OUT-FILE\n"
	       "  eet -d FILE.EET KEY OUT-FILE [CRYPTO_KEY]          extract and decode data stored in KEY in FILE.EET and write to OUT-FILE\n"
	       "  eet -i FILE.EET KEY IN-FILE COMPRESS [CRYPTO_KEY]  insert data to KEY in FILE.EET from IN-FILE and if COMPRESS is 1, compress it\n"
	       "  eet -e FILE.EET KEY IN-FILE COMPRESS [CRYPTO_KEY]  insert and encode to KEY in FILE.EET from IN-FILE and if COMPRESS is 1, compress it\n"
	       "  eet -r FILE.EET KEY                                remove KEY in FILE.EET\n"
	       "  eet -c FILE.EET                                    report and check the signature information of an eet file\n"
	       "  eet -s FILE.EET PRIVATE_KEY PUBLIC_KEY             sign FILE.EET with PRIVATE_KEY and attach PUBLIC_KEY as it's certificate\n"
	       );
	eet_shutdown();
	return -1;
     }
   if ((!strncmp(argv[1], "-h", 2)))
     {
	goto help;
     }
   else if ((!strcmp(argv[1], "-l")) && (argc > 2))
     {
	do_eet_list(argv[2]);
     }
   else if ((!strcmp(argv[1], "-x")) && (argc > 4))
     {
        if (argc > 5)
	  do_eet_extract(argv[2], argv[3], argv[4], argv[5]);
	else
	  do_eet_extract(argv[2], argv[3], argv[4], NULL);
     }
   else if ((!strcmp(argv[1], "-d")) && (argc > 4))
     {
        if (argc > 5)
	 do_eet_decode(argv[2], argv[3], argv[4], argv[5]);
	else
	 do_eet_decode(argv[2], argv[3], argv[4], NULL);
     }
   else if ((!strcmp(argv[1], "-i")) && (argc > 5))
     {
        if (argc > 6)
	  do_eet_insert(argv[2], argv[3], argv[4], atoi(argv[5]), argv[6]);
	else
	  do_eet_insert(argv[2], argv[3], argv[4], atoi(argv[5]), NULL);
     }
   else if ((!strcmp(argv[1], "-e")) && (argc > 5))
     {
        if (argc > 6)
	  do_eet_encode(argv[2], argv[3], argv[4], atoi(argv[5]), argv[6]);
	else
	  do_eet_encode(argv[2], argv[3], argv[4], atoi(argv[5]), NULL);
     }
   else if ((!strcmp(argv[1], "-r")) && (argc > 3))
     {
	do_eet_remove(argv[2], argv[3]);
     }
   else if ((!strcmp(argv[1], "-c")) && (argc > 2))
     {
	do_eet_check(argv[2]);
     }
   else if ((!strcmp(argv[1], "-s")) && (argc > 4))
     {
	do_eet_sign(argv[2], argv[3], argv[4]);
     }
   else
     {
	goto help;
     }
   eet_shutdown();
   return 0;
}
