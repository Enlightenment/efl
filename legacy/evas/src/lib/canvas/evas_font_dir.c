#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"
#ifdef BUILD_FONT_LOADER_EET
#include <Eet.h>
#endif

/* font dir cache */
static Evas_Hash *font_dirs = NULL;

/* private methods for font dir cache */
static Evas_Bool font_cache_dir_free(Evas_Hash *hash, const char *key, void *data, void *fdata);
static Evas_Font_Dir *object_text_font_cache_dir_update(char *dir, Evas_Font_Dir *fd);
static Evas_Font *object_text_font_cache_font_find_x(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find_file(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find_alias(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find(Evas_Font_Dir *fd, char *font);
static Evas_Font_Dir *object_text_font_cache_dir_add(char *dir);
static void object_text_font_cache_dir_del(char *dir, Evas_Font_Dir *fd);
static int evas_object_text_font_string_parse(char *buffer, char dest[14][256]);

void
evas_font_dir_cache_free(void)
{
   if (!font_dirs) return;

   evas_hash_foreach (font_dirs, font_cache_dir_free, NULL);
   evas_hash_free (font_dirs);
   font_dirs = NULL;
}

char *
evas_font_dir_cache_find(char *dir, char *font)
{
   Evas_Font_Dir *fd;
   
   fd = evas_hash_find(font_dirs, dir);
   fd = object_text_font_cache_dir_update(dir, fd);
   if (fd)
     {
	Evas_Font *fn;
	
	fn = object_text_font_cache_font_find(fd, font);
	if (fn) return fn->path;
     }
   return NULL;
}

static Evas_List *
evas_font_set_get(char *name)
{
   Evas_List *fonts = NULL;
   char *p;
   
   p = strchr(name, ',');
   if (!p)
     {
	fonts = evas_list_append(fonts, strdup(name));
     }
   else
     {
	char *nm, *pp;
	
	pp = name;
	while (p)
	  {
	     nm = malloc(p - pp + 1);
	     strncpy(nm, pp, p - pp);
	     nm[p - pp] = 0;
	     fonts = evas_list_append(fonts, nm);
	     pp = p + 1;
	     p = strchr(pp, ',');
	     if (!p) fonts = evas_list_append(fonts, strdup(pp));
	  }
     }
   return fonts;
}

void *
evas_font_load(Evas *evas, char *name, char *source, int size)
{
   void *font = NULL;
   Evas_List *fonts, *l;
   
   fonts = evas_font_set_get(name);
   for (l = fonts; l; l = l->next)
     {
	char *nm;
	
	nm = l->data;
	if (l == fonts)
	  {
#ifdef BUILD_FONT_LOADER_EET
	     if (source)
	       {
		  Eet_File *ef;
		  char *fake_name;
		  
		  fake_name = evas_file_path_join(source, nm);
		  if (fake_name)
		    {
		       font = evas->engine.func->font_load(evas->engine.data.output, fake_name, size);
		       if (!font)
			 {
			    /* read original!!! */
			    ef = eet_open(source, EET_FILE_MODE_READ);
			    if (ef)
			      {
				 void *fdata;
				 int fsize = 0;
				 
				 fdata = eet_read(ef, nm, &fsize);
				 if ((fdata) && (fsize > 0))
				   {
				      font = evas->engine.func->font_memory_load(evas->engine.data.output, fake_name, size, fdata, fsize);
				      free(fdata);
				   }
				 eet_close(ef);
			      }
			 }
		       free(fake_name);
		    }
	       }
	     if (!font)
	       {
#endif
		  if (evas_file_path_is_full_path((char *)nm))
		    font = evas->engine.func->font_load(evas->engine.data.output, (char *)nm, size);
		  else
		    {
		       Evas_List *l;
		       
		       for (l = evas->font_path; l; l = l->next)
			 {
			    char *f_file;
			    
			    f_file = evas_font_dir_cache_find(l->data, (char *)nm);
			    if (f_file)
			      {
				 font = evas->engine.func->font_load(evas->engine.data.output, f_file, size);
				 if (font) break;
			      }
			 }
		    }
#ifdef BUILD_FONT_LOADER_EET
	       }
#endif
	  }
	else
	  {
	     int ok = 0;
	     
#ifdef BUILD_FONT_LOADER_EET
	     if (source)
	       {
		  Eet_File *ef;
		  char *fake_name;
		  
		  fake_name = evas_file_path_join(source, nm);
		  if (fake_name)
		    {
		       /* FIXME: make an engine func */
		       if (!evas_common_font_add(font, fake_name, size))
			 {
			    /* read original!!! */
			    ef = eet_open(source, EET_FILE_MODE_READ);
			    if (ef)
			      {
				 void *fdata;
				 int fsize = 0;
				 
				 fdata = eet_read(ef, nm, &fsize);
				 if ((fdata) && (fsize > 0))
				   {
				      ok = evas_common_font_memory_add(font, fake_name, size, fdata, fsize);
				      free(fdata);
				   }
				 eet_close(ef);
			      }
			 }
		       else
			 ok = 1;
		       free(fake_name);
		    }
	       }
	     if (!ok)
	       {
#endif
		  if (evas_file_path_is_full_path((char *)nm))
		    evas_common_font_add(font, (char *)nm, size);
		  else
		    {
		       Evas_List *l;
		       
		       for (l = evas->font_path; l; l = l->next)
			 {
			    char *f_file;
			    
			    f_file = evas_font_dir_cache_find(l->data, (char *)nm);
			    if (f_file)
			      {
				 if (evas_common_font_add(font, f_file, size))
				   break;
			      }
			 }
		    }
#ifdef BUILD_FONT_LOADER_EET
	       }
#endif
	  }
	free(nm);
     }
   evas_list_free(fonts);
   return font;
}


/* private stuff */
static Evas_Bool
font_cache_dir_free(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   object_text_font_cache_dir_del((char *) key, data);

   return 1;
}

static Evas_Font_Dir *
object_text_font_cache_dir_update(char *dir, Evas_Font_Dir *fd)
{
   DATA64 mt;
   char *tmp;

   if (fd)
     {
	mt = evas_file_modified_time(dir);
	if (mt != fd->dir_mod_time)
	  {
	     object_text_font_cache_dir_del(dir, fd);
	     font_dirs = evas_hash_del(font_dirs, dir, fd);
	  }
	else
	  {
	     tmp = evas_file_path_join(dir, "fonts.dir");
	     if (tmp)
	       {
		  mt = evas_file_modified_time(tmp);
		  free(tmp);
		  if (mt != fd->fonts_dir_mod_time)
		    {
		       object_text_font_cache_dir_del(dir, fd);
		       font_dirs = evas_hash_del(font_dirs, dir, fd);
		    }
		  else
		    {
		       tmp = evas_file_path_join(dir, "fonts.alias");
		       if (tmp)
			 {
			    mt = evas_file_modified_time(tmp);
			    free(tmp);
			 }
		       if (mt != fd->fonts_alias_mod_time)
			 {
			    object_text_font_cache_dir_del(dir, fd);
			    font_dirs = evas_hash_del(font_dirs, dir, fd);
			 }
		       else
			 return fd;
		    }
	       }
	  }
     }
   return object_text_font_cache_dir_add(dir);
}

static Evas_Font *
object_text_font_cache_font_find_x(Evas_Font_Dir *fd, char *font)
{
   Evas_List *l;
   char font_prop[14][256];
   int num;
   
   num = evas_object_text_font_string_parse(font, font_prop);
   if (num != 14) return NULL;
   for (l = fd->fonts; l; l = l->next)
     {
	Evas_Font *fn;
	
	fn = l->data;
	if (fn->type == 1)
	  {
	     int i;
	     int match = 0;
	     
	     for (i = 0; i < 14; i++)
	       {
		  if ((font_prop[i][0] == '*') && (font_prop[i][1] == 0))
		    match++;
		  else
		    {
		       if (!strcasecmp(font_prop[i], fn->x.prop[i])) match++;
		       else break;
		    }
	       }
	     if (match == 14) return fn;
	  }
     }
   return NULL;
}

static Evas_Font *
object_text_font_cache_font_find_file(Evas_Font_Dir *fd, char *font)
{
   Evas_List *l;
   
   for (l = fd->fonts; l; l = l->next)
     {
	Evas_Font *fn;
	
	fn = l->data;
	if (fn->type == 0)
	  {
	     if (!strcasecmp(font, fn->simple.name)) return fn;
	  }
     }
   return NULL;
}

static Evas_Font *
object_text_font_cache_font_find_alias(Evas_Font_Dir *fd, char *font)
{
   Evas_List *l;
   
   for (l = fd->aliases; l; l = l->next)
     {
	Evas_Font_Alias *fa;
	
	fa = l->data;
	if (!strcasecmp(fa->alias, font)) return fa->fn;
     }
   return NULL;
}

static Evas_Font *
object_text_font_cache_font_find(Evas_Font_Dir *fd, char *font)
{
   Evas_Font *fn;
   
   fn = evas_hash_find(fd->lookup, font);
   if (fn) return fn;
   fn = object_text_font_cache_font_find_alias(fd, font);
   if (!fn) fn = object_text_font_cache_font_find_x(fd, font);
   if (!fn) fn = object_text_font_cache_font_find_file(fd, font);
   if (!fn) return NULL;
   fd->lookup = evas_hash_add(fd->lookup, font, fn);
   return fn;
}

static Evas_Font_Dir *
object_text_font_cache_dir_add(char *dir)
{
   Evas_Font_Dir *fd;
   char *tmp;
   Evas_List *fdir;
   
   fd = calloc(1, sizeof(Evas_Font_Dir));
   if (!fd) return NULL;
   font_dirs = evas_hash_add(font_dirs, dir, fd);
   
   /* READ fonts.alias, fonts.dir and directory listing */

   /* fonts.dir */
   tmp = evas_file_path_join(dir, "fonts.dir");
   if (tmp)
     {
	FILE *f;
	
	f = fopen(tmp, "r");
	if (f)
	  {
	     int num;
	     char fname[4096], fdef[4096];
	     
	     if (fscanf(f, "%i\n", &num) != 1) goto cant_read;
	     /* read font lines */
	     while (fscanf(f, "%4090s %[^\n]\n", fname, fdef) == 2)
	       {
		  char font_prop[14][256];
		  int i;
		  
		  /* skip comments */
		  if ((fdef[0] == '!') || (fdef[0] == '#')) continue;
		  /* parse font def */
		  num = evas_object_text_font_string_parse((char *)fdef, font_prop);
		  if (num == 14)
		    {
		       Evas_Font *fn;
		       
		       fn = calloc(1, sizeof(Evas_Font));
		       if (fn)
			 {
			    fn->type = 1;
			    for (i = 0; i < 14; i++)
			      {
				 fn->x.prop[i] = strdup(font_prop[i]);
				 /* FIXME: what if strdup fails! */
			      }
			    fn->path = evas_file_path_join(dir, fname);
			    /* FIXME; what is evas_file_path_join fails! */
			    fd->fonts = evas_list_append(fd->fonts, fn);
			 }
		    }
	       }
	     cant_read: ;
	     fclose(f);
	  }
	free(tmp);
     }
   
   /* directoy listing */
   fdir = evas_file_path_list(dir, "*.ttf", 0);
   while (fdir)
     {
	tmp = evas_file_path_join(dir, fdir->data);
	if (tmp)
	  {
	     Evas_Font *fn;
	     
	     fn = calloc(1, sizeof(Evas_Font));
	     if (fn)
	       {
		  fn->type = 0;
		  fn->simple.name = strdup(fdir->data);
		  if (fn->simple.name)
		    {
		       char *p;
		       
		       p = strrchr(fn->simple.name, '.');
		       if (p) *p = 0;
		    }
		  fn->path = evas_file_path_join(dir, fdir->data);
		  fd->fonts = evas_list_append(fd->fonts, fn);
	       }
	     free(tmp);
	  }
	free(fdir->data);
	fdir = evas_list_remove(fdir, fdir->data);
     }
   
   /* fonts.alias */
   tmp = evas_file_path_join(dir, "fonts.alias");
   if (tmp)
     {
	FILE *f;
	
	f = fopen(tmp, "r");
	if (f)
	  {
	     char fname[4096], fdef[4096];
	     
	     /* read font alias lines */
	     while (fscanf(f, "%4090s %[^\n]\n", fname, fdef) == 2)
	       {
		  Evas_Font_Alias *fa;
		  
		  /* skip comments */
		  if ((fdef[0] == '!') || (fdef[0] == '#')) continue;
		  fa = calloc(1, sizeof(Evas_Font_Alias));
		  if (fa)
		    {
		       fa->alias = strdup(fname);
		       fa->fn = object_text_font_cache_font_find_x(fd, fdef);
		       if ((!fa->alias) || (!fa->fn))
			 {
			    if (fa->alias) free(fa->alias);
			    free(fa);
			 }
		       else
			 fd->aliases = evas_list_append(fd->aliases, fa);
		    }
	       }
	     fclose(f);
	  }
	free(tmp);
     }
   
   fd->dir_mod_time = evas_file_modified_time(dir);
   tmp = evas_file_path_join(dir, "fonts.dir");
   if (tmp)
     {
	fd->fonts_dir_mod_time = evas_file_modified_time(tmp);
	free(tmp);
     }
   tmp = evas_file_path_join(dir, "fonts.alias");
   if (tmp)
     {
	fd->fonts_alias_mod_time = evas_file_modified_time(tmp);
	free(tmp);
     }
   
   return fd;
}

static void
object_text_font_cache_dir_del(char *dir, Evas_Font_Dir *fd)
{
   if (fd->lookup) evas_hash_free(fd->lookup);
   while (fd->fonts)
     {
	Evas_Font *fn;
	int i;
	
	fn = fd->fonts->data;
	fd->fonts = evas_list_remove(fd->fonts, fn);
	for (i = 0; i < 14; i++)
	  {
	     if (fn->x.prop[i]) free(fn->x.prop[i]);
	  }
	if (fn->simple.name) free(fn->simple.name);
	if (fn->path) free(fn->path);
	free(fn);
     }
   while (fd->aliases)
     {
	Evas_Font_Alias *fa;
	
	fa = fd->aliases->data;
	fd->aliases = evas_list_remove(fd->aliases, fa);
	if (fa->alias) free(fa->alias);
	free(fa);
     }
   free(fd);
}

static int
evas_object_text_font_string_parse(char *buffer, char dest[14][256])
{
   char *p;
   int n, m, i;

   n = 0;
   m = 0;
   p = buffer;
   if (p[0] != '-') return 0;
   i = 1;
   while (p[i])
     {
	dest[n][m] = p[i];
	if ((p[i] == '-') || (m == 256))
	  {
	     dest[n][m] = 0;
	     n++;
	     m = -1;
	  }
	i++;
	m++;
	if (n == 14) return n;
     }
   dest[n][m] = 0;
   n++;
   return n;
}
