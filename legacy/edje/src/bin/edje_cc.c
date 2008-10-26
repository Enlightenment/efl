/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <string.h>
#include <locale.h>
#include <limits.h>
#include <sys/stat.h>

#include "edje_cc.h"
#include "edje_prefix.h"

static void main_help(void);

Eina_List *img_dirs = NULL;
Eina_List *fnt_dirs = NULL;
Eina_List *defines = NULL;
char      *file_in = NULL;
char      *file_out = NULL;
char      *progname = NULL;
int        verbose = 0;

int        no_lossy = 0;
int        no_comp = 0;
int        no_raw = 0;
int        min_quality = 0;
int        max_quality = 100;

static void
main_help(void)
{
   printf
     ("Usage:\n"
      "\t%s [OPTIONS] input_file.edc [output_file.edj]\n"
      "\n"
      "Where OPTIONS is one or more of:\n"
      "\n"
      "-id image/directory      Add a directory to look in for relative path images\n"
      "-fd font/directory       Add a directory to look in for relative path fonts\n"
      "-v                       Verbose output\n"
      "-no-lossy                Do NOT allow images to be lossy\n"
      "-no-comp                 Do NOT allow images to be stored with lossless compression\n"
      "-no-raw                  Do NOT allow images to be stored with zero compression (raw)\n"
      "-min-quality VAL         Do NOT allow lossy images with quality < VAL (0-100)\n"
      "-max-quality VAL         Do NOT allow lossy images with quality > VAL (0-100)\n"
      "-Ddefine_val=to          CPP style define to define input macro definitions to the .edc source\n"
      ,progname);
}

int
main(int argc, char **argv)
{
   int i;
   struct stat st;
   char rpath[PATH_MAX], rpath2[PATH_MAX];

   setlocale(LC_NUMERIC, "C");

   eina_init();

   progname = argv[0];
   for (i = 1; i < argc; i++)
     {
	if (!strcmp(argv[i], "-h"))
	  {
	     main_help();
	     exit(0);
	  }
	else if (!strcmp(argv[i], "-v"))
	  {
	     verbose = 1;
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
	else if ((!strcmp(argv[i], "-id") || !strcmp(argv[i], "--image_dir")) && (i < (argc - 1)))
	  {
	     i++;
	     img_dirs = eina_list_append(img_dirs, argv[i]);
	  }
	else if ((!strcmp(argv[i], "-fd") || !strcmp(argv[i], "--font_dir")) && (i < (argc - 1)))
	  {
	     i++;
	     fnt_dirs = eina_list_append(fnt_dirs, argv[i]);
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
	else if (!strncmp(argv[i], "-D", 2))
	  {
	     defines = eina_list_append(defines, mem_strdup(argv[i]));
	  }
	else if ((!strcmp(argv[i], "-o")) && (i < (argc - 1)))
	  {
	     i++;
	     file_out = argv[i];
	  }
	else if (!file_in)
	  file_in = argv[i];
	else if (!file_out)
	  file_out = argv[i];
     }
   if (!file_in)
     {
	fprintf(stderr, "%s: Error: no input file specified.\n", progname);
	main_help();
	exit(-1);
     }

   e_prefix_determine(argv[0]);

   /* check whether file_in exists */
#ifdef HAVE_REALPATH
   if (!realpath(file_in, rpath) || stat(rpath, &st) || !S_ISREG(st.st_mode))
#else
   if (stat(file_in, &st) || !S_ISREG(st.st_mode))
#endif
     {
	fprintf(stderr, "%s: Error: file not found: %s.\n", progname, file_in);
	main_help();
	exit(-1);
     }

   if (!file_out)
      {
         char *suffix;

         if ((suffix = strstr(file_in,".edc")) && (suffix[4] == 0))
            {
               file_out = strdup(file_in);
               if (file_out)
                  {
                     suffix = strstr(file_out,".edc");
                     strcpy(suffix,".edj");
                  }
            }
      }
   if (!file_out)
     {
	fprintf(stderr, "%s: Error: no output file specified.\n", progname);
	main_help();
	exit(-1);
     }

#ifdef HAVE_REALPATH
   if (realpath(file_out, rpath2) && !strcmp (rpath, rpath2))
#else
   if (!strcmp (file_in, file_out))
#endif
     {
	fprintf(stderr, "%s: Error: input file equals output file.\n", progname);
	main_help();
	exit(-1);
     }

   edje_init();

   edje_file = mem_alloc(SZ(Edje_File));
   edje_file->compiler = strdup("edje_cc");
   edje_file->version = EDJE_FILE_VERSION;
   edje_file->feature_ver = 1; /* increment this every time we add a field
				* or feature to the edje file format that
				* does not load nicely as a NULL or 0 value
				* and needs a special fallback initialization
				*/

   source_edd();
   source_fetch();

   data_setup();
   compile();
   data_process_scripts();
   data_process_lookups();
   data_process_script_lookups();
   data_write();

   edje_shutdown();

   eina_shutdown();

   return 0;
}
