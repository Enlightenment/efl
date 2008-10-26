/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

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


#include <Ecore_File.h>
#include <Ecore_Evas.h>

#include "edje_decc.h"

char *progname = NULL;
char *file_in = NULL;
char *file_out = NULL;

Edje_File *edje_file = NULL;
SrcFile_List *srcfiles = NULL;
Font_List *fontlist = NULL;

int line = 0;

int        decomp(void);
void       output(void);
static int compiler_cmd_is_sane();
static int root_filename_is_sane();

static void
main_help(void)
{
   printf
     ("Usage:\n"
      "\t%s input_file.edj [-main-out file.edc]\n"
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

   progname = argv[0];
   for (i = 1; i < argc; i++)
     {
	if (!file_in)
	  file_in = argv[i];
	else if ((!strcmp(argv[i], "-main-out")) && (i < (argc - 1)))
	  {
	     i++;
	     file_out = argv[i];
	  }
     }
   if (!file_in)
     {
	fprintf(stderr, "%s: Error: no input file specified.\n", progname);
	main_help();
	exit(-1);
     }

   edje_init();
   eet_init();
   source_edd();

   if (!decomp()) return -1;
   output();

   eet_close(ef);
   eet_shutdown();
   return 0;
}

int
decomp(void)
{
   ef = eet_open(file_in, EET_FILE_MODE_READ);
   if (!ef)
     {
	printf("ERROR: cannot open %s\n", file_in);
	return 0;
     }

   srcfiles = source_load(ef);
   if (!srcfiles || !srcfiles->list)
     {
	printf("ERROR: %s has no decompile information\n", file_in);
	eet_close(ef);
	return 0;
     }
   if (!eina_list_data_get(srcfiles->list) || !root_filename_is_sane())
     {
        printf("ERROR: Invalid root filename: '%s'\n", (char *) eina_list_data_get(srcfiles->list));
	eet_close(ef);
	return 0;
     }
   edje_file = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
   if (!edje_file)
     {
	printf("ERROR: %s does not appear to be an edje file\n", file_in);
	eet_close(ef);
	return 0;
     }
   if (!edje_file->compiler)
     {
	edje_file->compiler = strdup("edje_cc");
     }
   else if (!compiler_cmd_is_sane())
     {
	printf("ERROR: invalid compiler executable: '%s'\n", edje_file->compiler);
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
   Eet_File *ef;
   SrcFile *sf;
   char *outdir, *p;

   p = strrchr(file_in, '/');
   if (p)
     outdir = strdup(p + 1);
   else
     outdir = strdup(file_in);
   p = strrchr(outdir, '.');
   if (p) *p = 0;

   ecore_file_mkpath(outdir);

   ef = eet_open(file_in, EET_FILE_MODE_READ);

   if (edje_file->image_dir)
     {
        Edje_Image_Directory_Entry *ei;

	EINA_LIST_FOREACH(edje_file->image_dir->entries, l, ei)
	  {
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
		       fprintf(stderr, "Error. cannot create buffer engine canvas for image save.\n");
		       exit(-1);
		    }
		  evas = ecore_evas_get(ee);
		  im = evas_object_image_add(evas);
		  if (!im)
		    {
		       fprintf(stderr, "Error. cannot create image object for save.\n");
		       exit(-1);
		    }
		  snprintf(buf, sizeof(buf), "images/%i", ei->id);
		  evas_object_image_file_set(im, file_in, buf);
		  snprintf(out, sizeof(out), "%s/%s", outdir, ei->entry);
		  printf("Output Image: %s\n", out);
		  pp = strdup(out);
		  p = strrchr(pp, '/');
		  *p = 0;
		  if (strstr(pp, "../"))
		    {
		       printf("ERROR: potential security violation. attempt to write in parent dir.\n");
		       exit(-1);
		    }
		  ecore_file_mkpath(pp);
		  free(pp);
		  if (!evas_object_image_save(im, out, NULL, "quality=100 compress=9"))
		    {
		       printf("ERROR: cannot write file %s. Perhaps missing JPEG or PNG saver modules for Evas.\n", out);
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
	printf("Output Source File: %s\n", out);
	pp = strdup(out);
	p = strrchr(pp, '/');
	*p = 0;
	if (strstr(pp, "../"))
	  {
	     printf("ERROR: potential security violation. attempt to write in parent dir.\n");
	     exit (-1);
	  }
	ecore_file_mkpath(pp);
	free(pp);
	if (strstr(out, "../"))
	  {
	     printf("ERROR: potential security violation. attempt to write in parent dir.\n");
	     exit (-1);
	  }
	f = fopen(out, "wb");
	if (!f)
	  {
	     printf("ERROR: unable to write file (%s).\n", out);
	     exit (-1);
	  }

	/* if the file is empty, sf->file will be NULL.
	 * note that that's not an error
	 */
	if (sf->file) fputs(sf->file, f);
	fclose(f);
     }
   if (fontlist)
     {
        Font *fn;

	EINA_LIST_FOREACH(fontlist->list, l, fn)
	  {
	     void *font;
	     int fontsize;
	     char out[4096];

	     snprintf(out, sizeof(out), "fonts/%s", fn->name);
	     font = eet_read(ef, out, &fontsize);
	     if (font)
	       {
		  FILE *f;
		  char *pp;

		  snprintf(out, sizeof(out), "%s/%s", outdir, fn->file);
		  printf("Output Font: %s\n", out);
		  pp = strdup(out);
		  p = strrchr(pp, '/');
		  *p = 0;
		  if (strstr(pp, "../"))
		    {
		       printf("ERROR: potential security violation. attempt to write in parent dir.\n");
		       exit (-1);
		    }
		  ecore_file_mkpath(pp);
		  free(pp);
		  if (strstr(out, "../"))
		    {
		       printf("ERROR: potential security violation. attempt to write in parent dir.\n");
		       exit (-1);
		    }
		  f = fopen(out, "wb");
		  fwrite(font, fontsize, 1, f);
		  fclose(f);
		  free(font);
	       }
	  }
     }
     {
	char out[4096];
	FILE *f;
	SrcFile *sf = eina_list_data_get(srcfiles->list);

	snprintf(out, sizeof(out), "%s/build.sh", outdir);
	printf("Output Build Script: %s\n", out);
	if (strstr(out, "../"))
	  {
	     printf("ERROR: potential security violation. attempt to write in parent dir.\n");
	     exit (-1);
	  }
	f = fopen(out, "wb");
	fprintf(f, "#!/bin/sh\n");
	fprintf(f, "%s $@ -id . -fd . %s -o %s.edj\n", edje_file->compiler, sf->name, outdir);
	fclose(f);

	if (file_out)
	  {
	     snprintf(out, sizeof(out), "%s/%s", outdir, file_out);
	     symlink(sf->name, out);
	  }

	chmod(out, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);

	printf("\n*** CAUTION ***\n"
	      "Please check the build script for anything malicious "
	      "before running it!\n\n");
     }
   eet_close(ef);
}

static int
compiler_cmd_is_sane()
{
   char *c = edje_file->compiler, *ptr;

   if (!c || !*c)
     {
	return 0;
     }

   for (ptr = c; ptr && *ptr; ptr++)
     {
	/* only allow [a-z][A-Z][0-9]_- */
	if (!isalnum(*ptr) && *ptr != '_' && *ptr != '-')
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
