/* ugly ugly. avert your eyes. */
#include "edje_decc.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

char *progname = NULL;
char *file_in = NULL;

Edje_File *edje_file = NULL;
SrcFile_List *srcfiles = NULL;
Font_List *fontlist = NULL;

int line = 0;

int        decomp(void);
void       output(void);
int        e_file_is_dir(char *file);
int        e_file_mkdir(char *dir);
int        e_file_mkpath(char *path);

static void
main_help(void)
{
   printf
     ("Usage:\n"
      "\t%s input_file.eet \n"
      "\n"
      ,progname);
}

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

   eet_shutdown();
   return 0;
}

int
decomp(void)
{
   Eet_File *ef;
   ef = eet_open(file_in, EET_FILE_MODE_READ);
   if (!ef) return 0;
   
   srcfiles = source_load(ef);
   if (!srcfiles)
     {
	eet_close(ef);
	return 0;
     }
   edje_file = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
   fontlist = source_fontmap_load(ef);
   eet_close(ef);
   return 1;
}

void
output(void)
{
   Evas_List *l;
   Eet_File *ef;
   char *outdir, *p;
   int i;
   
   p = strrchr(file_in, '/');
   if (p)
     outdir = strdup(p + 1);
   else
     outdir = strdup(file_in);
   p = strrchr(outdir, '.');
   if (p) *p = 0;
   
   e_file_mkpath(outdir);
   
   ef = eet_open(file_in, EET_FILE_MODE_READ);
   if (edje_file->image_dir)
     {
	for (l = edje_file->image_dir->entries; l; l = l->next)
	  {
	     Edje_Image_Directory_Entry *ei;
	     
	     ei = l->data;
	     if ((ei->source_type) && (ei->entry))
	       {
		  DATA32 *pix;
		  int w, h, alpha, comp, qual, lossy;
		  char buf[4096];
		  
		  snprintf(buf, sizeof(buf), "images/%i", ei->id);
		  pix = eet_data_image_read(ef, buf, &w, &h, &alpha, &comp, &qual, &lossy);
		  if (pix)
		    {
		       Imlib_Image im;
		       char out[4096];
		       char *pp;
		       
		       snprintf(out, sizeof(out), "%s/%s", outdir, ei->entry);
		       pp = strdup(out);
		       p = strrchr(pp, '/');
		       *p = 0;
		       e_file_mkpath(pp);
		       free(pp);
		       printf("Output Image: %s\n", out);
		       im = imlib_create_image_using_data(w, h, pix);
		       imlib_context_set_image(im);
		       if (alpha)
			 imlib_image_set_has_alpha(1);
		       if ((lossy) && (!alpha))
			 {
			    imlib_image_set_format("jpg");
			    imlib_image_attach_data_value("quality", NULL, qual, NULL);
			 }
		       else
			 {
			    imlib_image_set_format("png");
			 }
		       imlib_save_image(out);
		       imlib_free_image();
		       free(pix);
		    }
	       }
	  }
     }
   for (l = srcfiles->list; l; l = l->next)
     {
	SrcFile *sf;
	char out[4096];
	FILE *f;
	char *pp;
	
	sf = l->data;
	snprintf(out, sizeof(out), "%s/%s", outdir, sf->name);
	printf("Output Source File: %s\n", out);
	pp = strdup(out);
	p = strrchr(pp, '/');
	*p = 0;
	e_file_mkpath(pp);
	free(pp);
	f = fopen(out, "w");
	fputs(sf->file, f);
	fclose(f);
     }
   if (fontlist)
     {
	for (l = fontlist->list; l; l = l->next)
	  {
	     Font *fn;
	     void *font;
	     int fontsize;
	     char out[4096];
	     
	     fn = l->data;
	     snprintf(out, sizeof(out), "fonts/%s", fn->name);
	     font = eet_read(ef, out, &fontsize);
	     if (font)
	       {
		  FILE *f;
		  
		  snprintf(out, sizeof(out), "%s/%s", outdir, fn->file);
		  printf("Output Font: %s\n", out);
		  f = fopen(out, "w");
		  fwrite(font, fontsize, 1, f);
		  fclose(f);
		  free(font);
	       }
	  }
     }
     {
	char out[4096];
	FILE *f;
	
	snprintf(out, sizeof(out), "%s/build.sh", outdir);
	printf("Output Build Script: %s\n", out);
	f = fopen(out, "w");
	fprintf(f, "#!/bin/sh\n");
	fprintf(f, "edje_cc -id . -fd . main_edje_source.edc %s.eet\n", outdir);
	fclose(f);
	chmod(out, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
     }
   eet_close(ef);
}

int
e_file_is_dir(char *file)
{
   struct stat st;
   
   if (stat(file, &st) < 0) return 0;
   if (S_ISDIR(st.st_mode)) return 1;
   return 0;
}

static mode_t default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

int
e_file_mkdir(char *dir)
{
   if (mkdir(dir, default_mode) < 0) return 0;
   return 1;
}

int
e_file_mkpath(char *path)
{
   char ss[PATH_MAX];
   int  i, ii;
   
   ss[0] = 0;
   i = 0;
   ii = 0;
   while (path[i])
     {
	if (ii == sizeof(ss) - 1) return 0;
	ss[ii++] = path[i];
	ss[ii] = 0;
	if (path[i] == '/')
	  {
	     if (!e_file_is_dir(ss)) e_file_mkdir(ss);
	     else if (!e_file_is_dir(ss)) return 0;
	  }
	i++;
     }
   if (!e_file_is_dir(ss)) e_file_mkdir(ss);
   else if (!e_file_is_dir(ss)) return 0;
   return 1;
}
