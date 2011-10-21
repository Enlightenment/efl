#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* ifdef HAVE_UNISTD_H */

#ifdef HAVE_EVIL
# include <Evil.h>
#endif /* ifdef HAVE_EVIL */

#include <Eet.h>

static int _eet_main_log_dom = -1;

#ifdef EET_DEFAULT_LOG_COLOR
#undef EET_DEFAULT_LOG_COLOR
#endif /* ifdef EET_DEFAULT_LOG_COLOR */
#define EET_DEFAULT_LOG_COLOR EINA_COLOR_CYAN
#ifdef ERR
#undef ERR
#endif /* ifdef ERR */
#define ERR(...)  EINA_LOG_DOM_ERR(_eet_main_log_dom, __VA_ARGS__)
#ifdef DBG
#undef DBG
#endif /* ifdef DBG */
#define DBG(...)  EINA_LOG_DOM_DBG(_eet_main_log_dom, __VA_ARGS__)
#ifdef INF
#undef INF
#endif /* ifdef INF */
#define INF(...)  EINA_LOG_DOM_INFO(_eet_main_log_dom, __VA_ARGS__)
#ifdef WRN
#undef WRN
#endif /* ifdef WRN */
#define WRN(...)  EINA_LOG_DOM_WARN(_eet_main_log_dom, __VA_ARGS__)
#ifdef CRIT
#undef CRIT
#endif /* ifdef CRIT */
#define CRIT(...) EINA_LOG_DOM_CRIT(_eet_main_log_dom, __VA_ARGS__)

static void
do_eet_list(const char *file)
{
   int i, num;
   char **list;
   Eet_File *ef;

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
        ERR("cannot open for reading: %s", file);
        exit(-1);
     }

   list = eet_list(ef, "*", &num);
   if (list)
     {
        for (i = 0; i < num; i++)
          printf("%s\n", list[i]);
        free(list);
     }

   eet_close(ef);
} /* do_eet_list */

static void
do_eet_extract(const char *file,
               const char *key,
               const char *out,
               const char *crypto_key)
{
   Eet_File *ef;
   void *data;
   int size = 0;
   FILE *f;

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
        ERR("cannot open for reading: %s", file);
        exit(-1);
     }

   data = eet_read_cipher(ef, key, &size, crypto_key);
   if (!data)
     {
        ERR("cannot read key %s", key);
        exit(-1);
     }

   f = fopen(out, "wb");
   if (!f)
     {
        ERR("cannot open %s", out);
        exit(-1);
     }

   if (fwrite(data, size, 1, f) != 1)
     {
        ERR("cannot write to %s", out);
        exit(-1);
     }

   fclose(f);
   free(data);
   eet_close(ef);
} /* do_eet_extract */

static void
do_eet_decode_dump(void       *data,
                   const char *str)
{
   fputs(str, (FILE *)data);
} /* do_eet_decode_dump */

static void
do_eet_decode(const char *file,
              const char *key,
              const char *out,
              const char *crypto_key)
{
   Eet_File *ef;
   FILE *f;

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
        ERR("cannot open for reading: %s", file);
        exit(-1);
     }

   f = fopen(out, "wb");
   if (!f)
     {
        ERR("cannot open %s", out);
        exit(-1);
     }

   if (!eet_data_dump_cipher(ef, key, crypto_key, do_eet_decode_dump, f))
     {
        ERR("cannot write to %s", out);
        exit(-1);
     }

   fclose(f);
   eet_close(ef);
} /* do_eet_decode */

static void
do_eet_insert(const char *file,
              const char *key,
              const char *out,
              int         compress,
              const char *crypto_key)
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
        ERR("cannot open for read+write: %s", file);
        exit(-1);
     }

   f = fopen(out, "rb");
   if (!f)
     {
        ERR("cannot open %s", out);
        exit(-1);
     }

   fseek(f, 0, SEEK_END);
   size = ftell(f);
   rewind(f);
   data = malloc(size);
   if (!data)
     {
        ERR("cannot allocate %i bytes", size);
        exit(-1);
     }

   if (fread(data, size, 1, f) != 1)
     {
        ERR("cannot read file %s", out);
        exit(-1);
     }

   fclose(f);
   eet_write_cipher(ef, key, data, size, compress, crypto_key);
   free(data);
   eet_close(ef);
} /* do_eet_insert */

static void
do_eet_encode(const char *file,
              const char *key,
              const char *out,
              int         compress,
              const char *crypto_key)
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
        ERR("cannot open for read+write: %s", file);
        exit(-1);
     }

   f = fopen(out, "rb");
   if (!f)
     {
        ERR("cannot open %s", out);
        exit(-1);
     }

   fseek(f, 0, SEEK_END);
   textlen = ftell(f);
   rewind(f);
   text = malloc(textlen);
   if (!text)
     {
        ERR("cannot allocate %i bytes", size);
        exit(-1);
     }

   if (fread(text, textlen, 1, f) != 1)
     {
        ERR("cannot read file %s", out);
        exit(-1);
     }

   fclose(f);
   if (!eet_data_undump_cipher(ef, key, crypto_key, text, textlen, compress))
     {
        ERR("cannot parse %s", out);
        exit(-1);
     }

   free(text);
   eet_close(ef);
} /* do_eet_encode */

static void
do_eet_remove(const char *file,
              const char *key)
{
   Eet_File *ef;

   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   if (!ef)
     {
        ERR("cannot open for read+write: %s", file);
        exit(-1);
     }

   eet_delete(ef, key);
   eet_close(ef);
} /* do_eet_remove */

static void
do_eet_check(const char *file)
{
   Eet_File *ef;
   const void *der;
   int der_length;
   int sign_length;

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
        ERR("checking signature of `%s` failed", file);
        exit(-1);
     }

   der = eet_identity_x509(ef, &der_length);

   fprintf(stdout, "Certificate length %i.\n", der_length);
   eet_identity_certificate_print(der, der_length, stdout);

   eet_identity_signature(ef, &sign_length);
   fprintf(stdout, "Signature length %i.\n", sign_length);

   eet_close(ef);
} /* do_eet_check */

static void
do_eet_sign(const char *file,
            const char *private_key,
            const char *public_key)
{
   Eet_File *ef;
   Eet_Key *key;

   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   if (!ef)
     {
        ERR("cannot open for read+write: %s.", file);
        exit(-1);
     }

   key = eet_identity_open(public_key, private_key, NULL);
   if (!key)
     {
        ERR("cannot open key '%s:%s'.", public_key, private_key);
        exit(-1);
     }

   fprintf(stdout, "Using the following key to sign `%s`.\n", file);
   eet_identity_print(key, stdout);

   eet_identity_set(ef, key);

   eet_close(ef);
} /* do_eet_sign */

int
main(int    argc,
     char **argv)
{
   if (!eet_init())
     return -1;

   _eet_main_log_dom = eina_log_domain_register("eet_main", EINA_COLOR_CYAN);
   if(_eet_main_log_dom < -1)
     {
        EINA_LOG_ERR("Impossible to create a log domain for eet_main.");
        eet_shutdown();
        return -1;
     }

   if (argc < 2)
     {
help:
        printf(
          "Usage:\n"
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
     goto help;
   else if ((!strcmp(argv[1], "-l")) && (argc > 2))
     do_eet_list(argv[2]);
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
     do_eet_remove(argv[2], argv[3]);
   else if ((!strcmp(argv[1], "-c")) && (argc > 2))
     do_eet_check(argv[2]);
   else if ((!strcmp(argv[1], "-s")) && (argc > 4))
     do_eet_sign(argv[2], argv[3], argv[4]);
   else
     goto help;

   eina_log_domain_unregister(_eet_main_log_dom);
   eet_shutdown();
   return 0;
} /* main */

