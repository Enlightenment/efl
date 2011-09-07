/* ugly ugly. avert your eyes. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


#include <Ecore_File.h>
#include <Ecore_Evas.h>

#include "edje_decc.h"

int _edje_cc_log_dom = -1;
char *progname = NULL;
char *file_in = NULL;
char *file_out = NULL;

Edje_File *edje_file = NULL;
SrcFile_List *srcfiles = NULL;
Font_List *fontlist = NULL;

int line = 0;
int build_sh = 1;
int new_dir = 1;

int        decomp(void);
void       output(void);
static int compiler_cmd_is_sane();
static int root_filename_is_sane();

static void
main_help(void)
{
   printf
     ("Usage:\n"
      "\t%s input_file.edj [-main-out file.edc] [-no-build-sh] [-current-dir]\n"
      "\n"
      " -main-out\tCreate a symbolic link to the main edc \n"
      " -no-build-sh\tDon't output build.sh \n"
      " -current-dir\tOutput to current directory \n"
      "\n"
      ,progname);
}

Eet_File *ef;
Eet_Dictionary *ed;

int
main(int argc, char **argv)
{
   int i;

   setlocale(LC_NUMERIC, "C");
   if (!eina_init())
     exit(-1);
   _edje_cc_log_dom = eina_log_domain_register
     ("edje_decc", EDJE_CC_DEFAULT_LOG_COLOR);
   if (_edje_cc_log_dom < 0)
     {
       EINA_LOG_ERR("Impossible to create a log domain.");
       eina_shutdown();
       exit(-1);
     }
   eina_log_level_set(EINA_LOG_LEVEL_INFO);
   progname = argv[0];
   for (i = 1; i < argc; i++)
     {
	if (!strcmp(argv[i], "-h"))
	  {
	     main_help();
	     exit(0);
	  }
	if (!file_in)
	  file_in = argv[i];
	else if ((!strcmp(argv[i], "-main-out")) && (i < (argc - 1)))
	  {
	     i++;
	     file_out = argv[i];
	  }
	else if (!strcmp(argv[i], "-no-build-sh"))
	  build_sh = 0;
	else if (!strcmp(argv[i], "-current-dir"))
	  new_dir = 0;
     }
   if (!file_in)
     {
	ERR("%s: Error: no input file specified.", progname);
	main_help();
	exit(-1);
     }

   if (!edje_init())
     exit(-1);
   source_edd();

   if (!decomp()) return -1;
   output();

   eet_close(ef);
   edje_shutdown();
   eina_log_domain_unregister(_edje_cc_log_dom);
   _edje_cc_log_dom = -1;
   eina_shutdown();
   return 0;
}

int
decomp(void)
{
   ef = eet_open(file_in, EET_FILE_MODE_READ);
   if (!ef)
     {
	ERR("ERROR: cannot open %s", file_in);
	return 0;
     }

   srcfiles = source_load(ef);
   if (!srcfiles || !srcfiles->list)
     {
	ERR("ERROR: %s has no decompile information", file_in);
	eet_close(ef);
	return 0;
     }
   if (!eina_list_data_get(srcfiles->list) || !root_filename_is_sane())
     {
        ERR("ERROR: Invalid root filename: '%s'", (char *) eina_list_data_get(srcfiles->list));
	eet_close(ef);
	return 0;
     }
   edje_file = eet_data_read(ef, _edje_edd_edje_file, "edje/file");
   if (!edje_file)
     {
        ERR("ERROR: %s does not appear to be an edje file", file_in);
	eet_close(ef);
	return 0;
     }
   /* force compiler to be edje_cc */
   edje_file->compiler = strdup("edje_cc");
   if (!edje_file->compiler)
     {
	edje_file->compiler = strdup("edje_cc");
     }
   else if (!compiler_cmd_is_sane())
     {
	ERR("ERROR: invalid compiler executable: '%s'", edje_file->compiler);
	eet_close(ef);
	return 0;
     }
   fontlist = source_fontmap_load(ef);
   return 1;
}

void
output(void)
{
   Eina_List *l;
   Eet_File *tef;
   SrcFile *sf;
   char *outdir, *p;

   if (!new_dir)
     outdir = strdup(".");
   else
     {
	p = strrchr(file_in, '/');
	if (p)
	  outdir = strdup(p + 1);
	else
	  outdir = strdup(file_in);
	p = strrchr(outdir, '.');
	if (p) *p = 0;
	ecore_file_mkpath(outdir);
     }


   tef = eet_open(file_in, EET_FILE_MODE_READ);

   if (edje_file->image_dir)
     {
        Edje_Image_Directory_Entry *ei;
	unsigned int i;

	for (i = 0; i < edje_file->image_dir->entries_count; ++i)
	  {
	     ei = &edje_file->image_dir->entries[i];

	     if ((ei->source_type > EDJE_IMAGE_SOURCE_TYPE_NONE) &&
		 (ei->source_type < EDJE_IMAGE_SOURCE_TYPE_LAST) &&
		 (ei->source_type != EDJE_IMAGE_SOURCE_TYPE_EXTERNAL) &&
		 (ei->entry))
	       {
		  Ecore_Evas *ee;
		  Evas *evas;
		  Evas_Object *im;
		  char buf[4096];
		  char out[4096];
		  char *pp;

		  ecore_init();
		  ecore_evas_init();
		  ee = ecore_evas_buffer_new(1, 1);
		  if (!ee)
		    {
		       ERR("Cannot create buffer engine canvas for image save.");
		       exit(-1);
		    }
		  evas = ecore_evas_get(ee);
		  im = evas_object_image_add(evas);
		  if (!im)
		    {
		       ERR("Cannot create image object for save.");
		       exit(-1);
		    }
		  snprintf(buf, sizeof(buf), "edje/images/%i", ei->id);
		  evas_object_image_file_set(im, file_in, buf);
		  snprintf(out, sizeof(out), "%s/%s", outdir, ei->entry);
		  printf("Output Image: %s\n", out);
		  pp = strdup(out);
		  p = strrchr(pp, '/');
		  *p = 0;
		  if (strstr(pp, "../"))
		    {
		       ERR("Potential security violation. attempt to write in parent dir.");
		       exit(-1);
		    }
		  ecore_file_mkpath(pp);
		  free(pp);
		  if (!evas_object_image_save(im, out, NULL, "quality=100 compress=9"))
		    {
		       ERR("Cannot write file %s. Perhaps missing JPEG or PNG saver modules for Evas.", out);
		       exit(-1);
		    }
		  evas_object_del(im);
		  ecore_evas_free(ee);
		  ecore_evas_shutdown();
		  ecore_shutdown();
	       }
	  }
     }

   EINA_LIST_FOREACH(srcfiles->list, l, sf)
     {
	char out[4096];
	FILE *f;
	char *pp;

	snprintf(out, sizeof(out), "%s/%s", outdir, sf->name);
	INF("Output Source File: %s\n", out);
	pp = strdup(out);
	p = strrchr(pp, '/');
	*p = 0;
	if (strstr(pp, "../"))
	  {
	     ERR("Potential security violation. attempt to write in parent dir.");
	     exit (-1);
	  }
	ecore_file_mkpath(pp);
	free(pp);
	if (strstr(out, "../"))
	  {
	     ERR("Potential security violation. attempt to write in parent dir.");
	     exit (-1);
	  }
	f = fopen(out, "wb");
	if (!f)
	  {
	     ERR("Unable to write file (%s).", out);
	     exit (-1);
	  }

	/* if the file is empty, sf->file will be NULL.
	 * note that that's not an error
	 */
	if (sf->file) fputs(sf->file, f);
	fclose(f);
     }
   if (edje_file->fonts)
     {
        Edje_Font_Directory_Entry *fn;
        Eina_Iterator *it;

        it = eina_hash_iterator_data_new(edje_file->fonts);
	EINA_ITERATOR_FOREACH(it, fn)
	  {
	     void *font;
	     int fontsize;
	     char out[4096];
             /* FIXME!!!! */
                                         /* should be fn->entry -v */
	     snprintf(out, sizeof(out), "edje/fonts/%s", fn->file);
	     font = eet_read(tef, out, &fontsize);
	     if (font)
	       {
		  FILE *f;
		  char *pp;

                                         /* should be fn->file -v */
		  snprintf(out, sizeof(out), "%s/%s", outdir, fn->entry);
		  INF("Output Font: %s", out);
		  pp = strdup(out);
		  p = strrchr(pp, '/');
		  *p = 0;
		  if (strstr(pp, "../"))
		    {
		       ERR("Potential security violation. attempt to write in parent dir.");
		       exit (-1);
		    }
		  ecore_file_mkpath(pp);
		  free(pp);
		  if (strstr(out, "../"))
		    {
		       ERR("Potential security violation. attempt to write in parent dir.");
		       exit (-1);
		    }
		  if (!(f = fopen(out, "wb")))
                    {
                       ERR("Could not open file: %s", out);
		       exit (-1);
                    }
		  if (fwrite(font, fontsize, 1, f) != 1)
		    ERR("Could not write font: %s", strerror(errno));
		  if (f) fclose(f);
		  free(font);
	       }
	  }
        eina_iterator_free(it);
     }
     {
	char out[4096];
	FILE *f;
	sf = eina_list_data_get(srcfiles->list);


	if (build_sh)
	  {
	     snprintf(out, sizeof(out), "%s/build.sh", outdir);
	     printf("Output Build Script: %s\n", out);
	     if (strstr(out, "../"))
	       {
		  ERR("potential security violation. attempt to write in parent dir.\n");
		  exit (-1);
	       }
	     f = fopen(out, "wb");
	     fprintf(f, "#!/bin/sh\n");
	     fprintf(f, "%s $@ -id . -fd . %s -o %s.edj\n", edje_file->compiler, sf->name, outdir);
	     fclose(f);

	     WRN("\n*** CAUTION ***\n"
		 "Please check the build script for anything malicious "
		 "before running it!\n\n");
	  }

	if (file_out)
	  {
	     snprintf(out, sizeof(out), "%s/%s", outdir, file_out);
	     if (ecore_file_symlink(sf->name, out) != EINA_TRUE)
               {
                  ERR("symlink %s -> %s failed\n", sf->name, out);
               }
	  }

	chmod(out, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);

     }
   eet_close(tef);
}

static int
compiler_cmd_is_sane()
{
   const char *c = edje_file->compiler, *ptr;

   if ((!c) || (!*c))
     {
	return 0;
     }

   for (ptr = c; ptr && *ptr; ptr++)
     {
	/* only allow [a-z][A-Z][0-9]_- */
	if ((!isalnum(*ptr)) && (*ptr != '_') && (*ptr != '-'))
	  {
	     return 0;
	  }
     }

   return 1;
}

static int
root_filename_is_sane()
{
   SrcFile *sf = eina_list_data_get(srcfiles->list);
   char *f = sf->name, *ptr;

   if (!f || !*f)
     {
	return 0;
     }

   for (ptr = f; ptr && *ptr; ptr++)
     {
	/* only allow [a-z][A-Z][0-9]_-./ */
	switch (*ptr)
	  {
	   case '_': case '-':  case '.': case '/':
	      break;
	   default:
	      if (!isalnum(*ptr))
		{
		   return 0;
		}
	  }
     }
   return 1;
}
