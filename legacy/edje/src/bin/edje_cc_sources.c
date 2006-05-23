/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_cc.h"

static Eet_Data_Descriptor *_srcfile_edd = NULL;
static Eet_Data_Descriptor *_srcfile_list_edd = NULL;

static Eet_Data_Descriptor *_font_edd = NULL;
static Eet_Data_Descriptor *_font_list_edd = NULL;

static SrcFile_List srcfiles = {NULL};

void
source_edd(void)
{
   _srcfile_edd = eet_data_descriptor_new("srcfile", sizeof(SrcFile),
					  (void *(*) (void *))evas_list_next,
					  (void *(*) (void *, void *))evas_list_append,
					  (void *(*) (void *))evas_list_data,
					  (void *(*) (void *))evas_list_free,
					  (void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach,
					  (void *(*) (void *, const char *, void *))evas_hash_add,
					  (void  (*) (void *))evas_hash_free);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_srcfile_edd, SrcFile, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_srcfile_edd, SrcFile, "file", file, EET_T_STRING);
   _srcfile_list_edd = eet_data_descriptor_new("srcfile_list", sizeof(SrcFile_List),
					       (void *(*) (void *))evas_list_next,
					       (void *(*) (void *, void *))evas_list_append,
					       (void *(*) (void *))evas_list_data,
					       (void *(*) (void *))evas_list_free,
					       (void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach,
					       (void *(*) (void *, const char *, void *))evas_hash_add,
					       (void  (*) (void *))evas_hash_free);
   EET_DATA_DESCRIPTOR_ADD_LIST(_srcfile_list_edd, SrcFile_List, "list", list, _srcfile_edd);
   
   _font_edd = eet_data_descriptor_new("font", sizeof(Font),
				       (void *(*) (void *))evas_list_next,
				       (void *(*) (void *, void *))evas_list_append,
				       (void *(*) (void *))evas_list_data,
				       (void *(*) (void *))evas_list_free,
				       (void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach,
				       (void *(*) (void *, const char *, void *))evas_hash_add,
				       (void  (*) (void *))evas_hash_free);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_font_edd, Font, "file", file, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_font_edd, Font, "name", name, EET_T_STRING);
   _font_list_edd = eet_data_descriptor_new("font_list", sizeof(Font_List),
					    (void *(*) (void *))evas_list_next,
					    (void *(*) (void *, void *))evas_list_append,
					    (void *(*) (void *))evas_list_data,
					    (void *(*) (void *))evas_list_free,
					    (void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach,
					    (void *(*) (void *, const char *, void *))evas_hash_add,
					    (void  (*) (void *))evas_hash_free);
   EET_DATA_DESCRIPTOR_ADD_LIST(_font_list_edd, Font_List, "list", list, _font_edd);
}

static void
source_fetch_file(char *fil, char *filname)
{
   FILE *f;
   char buf[256 * 1024], *dir = NULL;
   long sz;
   ssize_t dir_len = 0;
   SrcFile *sf;
   
   f = fopen(fil, "rb");
   if (!f)
     {
	fprintf(stderr, "%s: Warning. Cannot open file '%s'\n",
	      progname, fil);
	exit(-1);
     }

   fseek(f, 0, SEEK_END);
   sz = ftell(f);
   fseek(f, 0, SEEK_SET);
   sf = mem_alloc(SZ(SrcFile));
   sf->name = mem_strdup(filname);
   sf->file = mem_alloc(sz + 1);
   fread(sf->file, sz, 1, f);
   sf->file[sz] = '\0';
   fseek(f, 0, SEEK_SET);
   srcfiles.list = evas_list_append(srcfiles.list, sf);

   while (fgets(buf, sizeof(buf), f))
     {
	char *p = buf, *pp;
	int got_hash = 0;
	int forgetit = 0;
	int haveinclude = 0;
	char *file = NULL, *fname = NULL;
	
	while ((!forgetit) && (*p))
	  {
	     if (!got_hash)
	       {
		  if (!isspace(*p))
		    {
		       if (*p == '#')
			 got_hash = 1;
		       else
			 forgetit = 1;
		    }
		  p++;
	       }

	     if (!haveinclude)
	       {
		  if (!isspace(*p))
		    {
		       if (!strncmp(p, "include", 7))
			 {
			    haveinclude = 1;
			    p += 7;
			 }
		       /* HACK! the logic above should be fixed so
			* preprocessor statements don't have to begin
			* in column 0.
			* otoh, edje_cc should print a warning in that case,
			* since according to the standard, preprocessor
			* statements need to be put in column 0.
			*/
		       else if (!strncmp (p, "#include", 8))
			 {
			    haveinclude = 1;
			    p += 8;
			 }
		       else
			 forgetit = 1;
		    }
	       }
	     else
	       {
		  if (!isspace(*p))
		    {
		       char end = '\0';

		       if (*p == '"')
			    end = '"';
		       else if (*p == '<')
			    end = '>';

		       if (end)
			 {
			    pp = strchr(p + 1, end);
			    if (!pp)
			      forgetit = 1;
			    else
			      {
				 char *slash;
				 ssize_t l = 0;

				 /* get the directory of the current file
				  * if we haven't already done so
				  */
				 if (!dir && strrchr(fil, '/'))
				   {
				      dir = mem_strdup(fil);
				      slash = strrchr(dir, '/');
				      *slash = '\0';
				      dir_len = strlen(dir);
				   }

				 l = pp - p + dir_len + 1;
				 file = mem_alloc(l);

				 if (!dir_len)
				   snprintf(file, l - 1, "%s", p + 1);
				 else
				   snprintf(file, l, "%s/%s", dir, p + 1);

				 fname = strdup(p + 1);
				 pp = strrchr(fname, end);
				 if (pp) *pp = 0;
				 forgetit = 1;
			      }
			 }
		       else
			 forgetit = 1;
		    }
		  else
		    p++;
	       }

	     got_hash = 0;
	  }
	if ((file) && (fname))
	  {
	     source_fetch_file(file, fname);
	     free(file);
	     free(fname);
	  }
     }
   free (dir);
   fclose(f);
}

void
source_fetch(void)
{
   char buf[PATH_MAX] = {0}, *ptr;

   ptr = strrchr(file_in, '/');
   if (ptr)
     {
	snprintf(buf, sizeof (buf), "%s", ptr + 1);
     }

   source_fetch_file(file_in, buf[0] ? buf : file_in);
}

int
source_append(Eet_File *ef)
{
   return eet_data_write(ef, _srcfile_list_edd, "edje_sources", &srcfiles, 1);
}

SrcFile_List *
source_load(Eet_File *ef)
{
   SrcFile_List *s;
   
   s = eet_data_read(ef, _srcfile_list_edd, "edje_sources");
   return s;
}

int
source_fontmap_save(Eet_File *ef, Evas_List *fonts)
{
   Font_List fl;
   
   fl.list = fonts;
   return eet_data_write(ef, _font_list_edd, "edje_source_fontmap", &fl, 1);
}

Font_List *
source_fontmap_load(Eet_File *ef)
{
   Font_List *fl;
   
   fl = eet_data_read(ef, _font_list_edd, "edje_source_fontmap");
   return fl;
}
