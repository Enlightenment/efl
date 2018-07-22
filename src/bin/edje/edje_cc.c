#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <locale.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/time.h>

#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

#include "edje_cc.h"
int _edje_cc_log_dom = -1;
static void main_help(void);

Eina_Prefix *pfx = NULL;
Eina_List *snd_dirs = NULL;
Eina_List *mo_dirs = NULL;
Eina_List *vibration_dirs = NULL;
Eina_List *img_dirs = NULL;
Eina_List *model_dirs = NULL;
Eina_List *fnt_dirs = NULL;
Eina_List *data_dirs = NULL;
Eina_List *defines = NULL;
char *file_in = NULL;
char *tmp_dir = NULL;
char *file_out = NULL;
char *watchfile = NULL;
char *depfile = NULL;
char *authors = NULL;
char *license = NULL;
Eina_List *licenses = NULL;
Eina_Array *requires;

static const char *progname = NULL;

int no_lossy = 0;
int no_comp = 0;
int no_raw = 0;
int no_save = 0;
int min_quality = 0;
int max_quality = 100;
int compress_mode = EET_COMPRESSION_HI;
int threads = 0;
int annotate = 0;
int no_etc1 = 0;
int no_etc2 = 0;
int beta = 0;
Eina_Bool namespace_verify;

unsigned int max_open_files;

static void
_edje_cc_log_cb(const Eina_Log_Domain *d,
                Eina_Log_Level level,
                const char *file,
                const char *fnc,
                int fline,
                const char *fmt,
                EINA_UNUSED void *data,
                va_list args)
{
   if ((d->name) && (d->namelen == sizeof("edje_cc") - 1) &&
       (memcmp(d->name, "edje_cc", sizeof("edje_cc") - 1) == 0))
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
     eina_log_print_cb_stderr(d, level, file, fnc, fline, fmt, NULL, args);
}

static void
main_help(void)
{
   printf
     ("Usage:\n"
      "\t%s [OPTIONS] input_file.edc [output_file.edj]\n"
      "\n"
      "Where OPTIONS is one or more of:\n"
      "\n"
      "-w files.txt             Dump all sources files path into files.txt\n"
      "-annotate                Annotate the dumped files.\n"
      "-deps files.txt          Dump gnu style include dependencies path into files.txt (overrides -w/-annotate)\n"
      "-id image/directory      Add a directory to look in for relative path images\n"
      "-fd font/directory       Add a directory to look in for relative path fonts\n"
      "-sd sound/directory      Add a directory to look in for relative path sounds samples\n"
      "-vd vibration/directory  Add a directory to look in for relative path vibration samples\n"
      "-dd data/directory       Add a directory to look in for relative path data.file entries\n"
      "-md mo/directory         Add a directory to look in for relative path mo files\n"
      "-td temp/directory       Directory to store temporary files\n"
      "-l license               Specify the license of a theme (file with license text)\n"
      "-a authors               Specify AUTHORS (file with list of authors)\n"
      "-v                       Verbose output\n"
      "-no-lossy                Do NOT allow images to be lossy\n"
      "-no-comp                 Do NOT allow images to be stored with lossless compression\n"
      "-no-raw                  Do NOT allow images to be stored with zero compression (raw)\n"
      "-no-etc1                 Do NOT allow images to be stored as ETC1\n"
      "-no-etc2                 Do NOT allow images to be stored as ETC2\n"
      "-no-save                 Do NOT store the input EDC file in the EDJ file\n"
      "-min-quality VAL         Do NOT allow lossy images with quality < VAL (0-100)\n"
      "-max-quality VAL         Do NOT allow lossy images with quality > VAL (0-100)\n"
      "-Ddefine_val=to          CPP style define to define input macro definitions to the .edc source\n"
      "-fastcomp                Use a faster compression algorithm (LZ4) (mutually exclusive with -fastdecomp)\n"
      "-fastdecomp              Use a faster decompression algorithm (LZ4HC) (mutually exclusive with -fastcomp)\n"
      "-threads                 Compile the edje file using multiple parallel threads (by default)\n"
      "-nothreads               Compile the edje file using only the main loop\n"
      "-N                       Use the first segment of each group name as a namespace to verify parts/signals\n"
      "-V [--version]           show program version\n"
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

   ecore_app_no_system_modules();
   efreet_cache_disable();

   if (!eina_init())
     return -1;

   _edje_cc_log_dom = eina_log_domain_register
       ("edje_cc", EDJE_CC_DEFAULT_LOG_COLOR);
   if (_edje_cc_log_dom < 0)
     {
        EINA_LOG_ERR("Enable to create a log domain.");
        exit(-1);
     }
   if (!eina_log_domain_level_check(_edje_cc_log_dom, EINA_LOG_LEVEL_WARN))
     eina_log_domain_level_set("edje_cc", EINA_LOG_LEVEL_WARN);

   progname = ecore_file_file_get(argv[0]);
   eina_log_print_cb_set(_edje_cc_log_cb, NULL);

   tmp_dir = (char *)eina_environment_tmp_get();

   img_dirs = eina_list_append(img_dirs, ".");

   model_dirs = eina_list_append(model_dirs, ".");

   /* add defines to epp so edc files can detect edje_cc version */
   defines = eina_list_append(defines, mem_strdup("-DEDJE_VERSION_12=12"));

   for (i = 1; i < argc; i++)
     {
        if (!strcmp(argv[i], "-h"))
          {
             main_help();
             exit(0);
          }
        else if ((!strcmp(argv[i], "-V")) || (!strcmp(argv[i], "--version")))
          {
             printf("Version: %s\n", PACKAGE_VERSION);
             exit(0);
          }
        else if (!strcmp(argv[i], "-v"))
          {
             eina_log_domain_level_set("edje_cc", EINA_LOG_LEVEL_INFO);
          }
        else if (!strcmp(argv[i], "-no-lossy"))
          {
             no_lossy = 1;
          }
        else if (!strcmp(argv[i], "-no-comp"))
          {
             no_comp = 1;
          }
        else if (!strcmp(argv[i], "-no-raw"))
          {
             no_raw = 1;
          }
        else if (!strcmp(argv[i], "-no-etc1"))
          {
             no_etc1 = 1;
          }
        else if (!strcmp(argv[i], "-no-etc2"))
          {
             no_etc2 = 1;
          }
        else if (!strcmp(argv[i], "-no-save"))
          {
             no_save = 1;
          }
        else if ((!strcmp(argv[i], "-id") || !strcmp(argv[i], "--image_dir")) && (i < (argc - 1)))
          {
             i++;
             img_dirs = eina_list_append(img_dirs, argv[i]);
          }
        else if ((!strcmp(argv[i], "-mod") || !strcmp(argv[i], "--model_dir")) && (i < (argc - 1)))
          {
             i++;
             model_dirs = eina_list_append(model_dirs, argv[i]);
          }
        else if ((!strcmp(argv[i], "-fd") || !strcmp(argv[i], "--font_dir")) && (i < (argc - 1)))
          {
             i++;
             fnt_dirs = eina_list_append(fnt_dirs, argv[i]);
          }
        else if ((!strcmp(argv[i], "-sd") || !strcmp(argv[i], "--sound_dir")) && (i < (argc - 1)))
          {
             i++;
             snd_dirs = eina_list_append(snd_dirs, argv[i]);
          }
        else if ((!strcmp(argv[i], "-md") || !strcmp(argv[i], "--mo_dir")) && (i < (argc - 1)))
          {
             i++;
             mo_dirs = eina_list_append(mo_dirs, argv[i]);
          }
        else if ((!strcmp(argv[i], "-vd") || !strcmp(argv[i], "--vibration_dir")) && (i < (argc - 1)))
          {
             i++;
             vibration_dirs = eina_list_append(vibration_dirs, argv[i]);
          }
        else if ((!strcmp(argv[i], "-dd") || !strcmp(argv[i], "--data_dir")) && (i < (argc - 1)))
          {
             i++;
             data_dirs = eina_list_append(data_dirs, argv[i]);
          }
        else if ((!strcmp(argv[i], "-td") || !strcmp(argv[i], "--tmp_dir")) && (i < (argc - 1)))
          {
             i++;
             if (!tmp_dir)
               tmp_dir = argv[i];
          }
        else if ((!strcmp(argv[i], "-l") || !strcmp(argv[i], "--license")) && (i < (argc - 1)))
          {
             i++;
             if (!license)
               license = argv[i];
             else
               licenses = eina_list_append(licenses, argv[i]);
          }
        else if ((!strcmp(argv[i], "-a") || !strcmp(argv[i], "--authors")) && (i < (argc - 1)))
          {
             i++;
             if (!authors)
               authors = argv[i];
          }
        else if ((!strcmp(argv[i], "-min-quality")) && (i < (argc - 1)))
          {
             i++;
             min_quality = atoi(argv[i]);
             if (min_quality < 0) min_quality = 0;
             if (min_quality > 100) min_quality = 100;
          }
        else if ((!strcmp(argv[i], "-max-quality")) && (i < (argc - 1)))
          {
             i++;
             max_quality = atoi(argv[i]);
             if (max_quality < 0) max_quality = 0;
             if (max_quality > 100) max_quality = 100;
          }
        else if (!strcmp(argv[i], "-fastcomp"))
          {
             compress_mode = EET_COMPRESSION_SUPERFAST;
          }
        else if (!strcmp(argv[i], "-fastdecomp"))
          {
             compress_mode = EET_COMPRESSION_VERYFAST;
          }
        else if (!strcmp(argv[i], "-threads"))
          {
             threads = 1;
          }
        else if (!strcmp(argv[i], "-nothreads"))
          {
             threads = 0;
          }
        else if (!strncmp(argv[i], "-D", 2))
          {
             defines = eina_list_append(defines, mem_strdup(argv[i]));
          }
        else if ((!strcmp(argv[i], "-o")) && (i < (argc - 1)))
          {
             i++;
             file_out = argv[i];
          }
        else if ((!strcmp(argv[i], "-w")) && (i < (argc - 1)))
          {
             i++;
             watchfile = argv[i];
             unlink(watchfile);
          }
        else if (!strcmp(argv[i], "-annotate"))
          {
             annotate = 1;
          }
        else if ((!strcmp(argv[i], "-deps")) && (i < (argc - 1)))
          {
             i++;
             depfile = argv[i];
             unlink(depfile);
          }
        else if (!strcmp(argv[i], "-beta"))
          {
             beta = 1;
          }
        else if (!strcmp(argv[i], "-N"))
          {
             namespace_verify = 1;
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
                         "EDJE", /* env var prefix to use (XXX_PREFIX, XXX_BIN_DIR etc. */
                         "edje", /* dir to add after "share" (PREFIX/share/DIRNAME) */
                         "include/edje.inc", /* a magic file to check for in PREFIX/share/DIRNAME for success */
                         PACKAGE_BIN_DIR,    /* package bin dir @ compile time */
                         PACKAGE_LIB_DIR,    /* package lib dir @ compile time */
                         PACKAGE_DATA_DIR,   /* package data dir @ compile time */
                         PACKAGE_DATA_DIR    /* if locale needed  use LOCALE_DIR */
                         );

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

        if ((suffix = strstr(file_in, ".edc")) && (suffix[4] == 0))
          {
             file_out = strdup(file_in);
             if (file_out)
               {
                  suffix = strstr(file_out, ".edc");
                  strcpy(suffix, ".edj");
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

   using_file(file_in, 'E');
   if (annotate) using_file(file_out, 'O');

   if (!edje_init())
     exit(-1);

   edje_file = mem_alloc(SZ(Edje_File));
   edje_file->compiler = strdup("edje_cc");
   edje_file->version = EDJE_FILE_VERSION;
   edje_file->minor = EDJE_FILE_MINOR;
   edje_file->feature_ver = 1; /* increment this every time we add a field
                                * or feature to the edje file format that
                                * does not load nicely as a NULL or 0 value
                                * and needs a special fallback initialization
                                */
   /* efl_version is used for specify efl's version
    * which was used for developing a edje file.
    * It is useful if Edje(or other EFL libs) need to keep
    * backward compatibility.
    * efl_version was added to fix backward compatibility issue caused from EFL 1.19.
    * Thus, 1.18 will be default.
    */
   edje_file->efl_version.major = 1;
   edje_file->efl_version.minor = 18;
   edje_file->base_scale = FROM_INT(1);
   requires = eina_array_new(10);

#ifdef HAVE_SYS_RESOURCE_H
   {
      struct rlimit lim;
      if (getrlimit(RLIMIT_NOFILE, &lim))
        fprintf(stderr, "error getting max open file limit: %s\n", strerror(errno));
      max_open_files = lim.rlim_cur;
   }
#else
   max_open_files = 1024;
#endif
   ecore_evas_init();

   source_edd();
   source_fetch();

   data_setup();
   compile();
   reorder_parts();
   data_process_scripts();
   data_process_lookups();
   data_process_script_lookups();
   data_write();

   eina_prefix_free(pfx);
   pfx = NULL;

   edje_shutdown();
   eina_log_domain_unregister(_edje_cc_log_dom);
   eina_shutdown();

   return 0;
}

