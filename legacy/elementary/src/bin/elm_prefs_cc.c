#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <string.h>
#include <locale.h>
#include <limits.h>
#include <sys/stat.h>

#include "elm_prefs_cc.h"

int _elm_prefs_cc_log_dom = -1;
static void main_help(void);

Eina_Prefix *pfx = NULL;
char *file_in = NULL;
char *tmp_dir = NULL;
char *file_out = NULL;

static const char *progname = NULL;

static void
_elm_prefs_cc_log_cb(const Eina_Log_Domain *d,
                     Eina_Log_Level level,
                     const char *file,
                     const char *fnc,
                     int _line,
                     const char *fmt,
                     EINA_UNUSED void *data,
                     va_list args)
{
   if ((d->name) && (d->namelen == sizeof("elm_prefs_cc") - 1) &&
       (memcmp(d->name, "elm_prefs_cc", sizeof("elm_prefs_cc") - 1) == 0))
     {
        const char *prefix;

        eina_log_console_color_set(stderr, eina_log_level_color_get(level));
        switch (level)
          {
           case EINA_LOG_LEVEL_CRITICAL:
             prefix = "Critical. ";
             break;

           case EINA_LOG_LEVEL_ERR:
             prefix = "Error. ";
             break;

           case EINA_LOG_LEVEL_WARN:
             prefix = "Warning. ";
             break;

           default:
             prefix = "";
          }
        fprintf(stderr, "%s: %s", progname, prefix);
        eina_log_console_color_set(stderr, EINA_COLOR_RESET);

        vfprintf(stderr, fmt, args);
        putc('\n', stderr);
     }
   else
     eina_log_print_cb_stderr(d, level, file, fnc, _line, fmt, NULL, args);
}

static void
main_help(void)
{
   printf
     ("Usage:\n"
      "\t%s [OPTIONS] input_file.epc [output_file.epb]\n"
      "\n"
      "Where OPTIONS is one or more of:\n"
      "\n"
      "-v                       Verbose output\n"
     , progname);
}

int
main(int argc, char **argv)
{
   int i;
   struct stat st;
#ifdef HAVE_REALPATH
   char rpath[PATH_MAX], rpath2[PATH_MAX];
#endif

   setlocale(LC_NUMERIC, "C");

   if (!eina_init())
     return -1;

   _elm_prefs_cc_log_dom = eina_log_domain_register
       ("elm_prefs_cc", ELM_PREFS_CC_DEFAULT_LOG_COLOR);
   if (_elm_prefs_cc_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create a log domain.");
        exit(-1);
     }
   if (!eina_log_domain_level_check(_elm_prefs_cc_log_dom, EINA_LOG_LEVEL_WARN))
     eina_log_domain_level_set("elm_prefs_cc", EINA_LOG_LEVEL_WARN);

   progname = ecore_file_file_get(argv[0]);
   eina_log_print_cb_set(_elm_prefs_cc_log_cb, NULL);

   tmp_dir = getenv("TMPDIR");

   for (i = 1; i < argc; i++)
     {
        if (!strcmp(argv[i], "-h"))
          {
             main_help();
             exit(0);
          }
        else if (!strcmp(argv[i], "-v"))
          {
             eina_log_domain_level_set("elm_prefs_cc", EINA_LOG_LEVEL_INFO);
          }
        else if (!file_in)
          file_in = argv[i];
        else if (!file_out)
          file_out = argv[i];
     }

   if (!file_in)
     {
        ERR("no input file specified.");
        main_help();
        exit(-1);
     }

   pfx = eina_prefix_new(argv[0], /* argv[0] value (optional) */
                         main, /* an optional symbol to check path of */
                         "ELM_PREFS", /* env var prefix to use (XXX_PREFIX, XXX_BIN_DIR etc. */
                         "elementary", /* dir to add after "share" (PREFIX/share/DIRNAME) */
                         "themes/default.edj",    /* a magic file to check for in PREFIX/share/DIRNAME for success */
                         PACKAGE_BIN_DIR, /* package bin dir @ compile time */
                         PACKAGE_LIB_DIR, /* package lib dir @ compile time */
                         PACKAGE_DATA_DIR, /* package data dir @ compile time */
                         PACKAGE_DATA_DIR); /* if locale needed  use LOCALE_DIR */

   /* check whether file_in exists */
#ifdef HAVE_REALPATH
   if (!realpath(file_in, rpath) || stat(rpath, &st) || !S_ISREG(st.st_mode))
#else
   if (stat(file_in, &st) || !S_ISREG(st.st_mode))
#endif
     {
        ERR("file not found: %s.", file_in);
        main_help();
        exit(-1);
     }

   if (!file_out)
     {
        char *suffix;

        if ((suffix = strstr(file_in, ".epc")) && (suffix[4] == 0))
          {
             file_out = strdup(file_in);
             if (file_out)
               {
                  suffix = strstr(file_out, ".epc");
                  strcpy(suffix, ".epb");
               }
          }
     }
   if (!file_out)
     {
        ERR("no output file specified.");
        main_help();
        exit(-1);
     }

#ifdef HAVE_REALPATH
   if (realpath(file_out, rpath2) && !strcmp(rpath, rpath2))
#else
   if (!strcmp(file_in, file_out))
#endif
     {
        ERR("input file equals output file.");
        main_help();
        exit(-1);
     }

   elm_prefs_file = mem_alloc(SZ(Elm_Prefs_File));
   elm_prefs_file->compiler = strdup("elm_prefs_cc");

   compile();

   data_init();
   data_write();
   data_shutdown();

   eina_prefix_free(pfx);
   pfx = NULL;

   eina_log_domain_unregister(_elm_prefs_cc_log_dom);
   eina_shutdown();

   return 0;
}
