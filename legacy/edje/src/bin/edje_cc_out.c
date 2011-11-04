#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

#include <Ecore_Evas.h>

#include "edje_cc.h"
#include "edje_convert.h"
#include "edje_multisense_convert.h"

#include <lua.h>
#include <lauxlib.h>

typedef struct _External_Lookup External_Lookup;
typedef struct _Part_Lookup Part_Lookup;
typedef struct _Program_Lookup Program_Lookup;
typedef struct _Group_Lookup Group_Lookup;
typedef struct _Image_Lookup Image_Lookup;
typedef struct _Slave_Lookup Slave_Lookup;
typedef struct _Code_Lookup Code_Lookup;


struct _External_Lookup
{
   char *name;
};

struct _Part_Lookup
{
   Edje_Part_Collection *pc;
   char *name;
   int *dest;
};

struct _Program_Lookup
{
   Edje_Part_Collection *pc;

   union
   {
      char *name;
      Edje_Program *ep;
   } u;

   int *dest;

   Eina_Bool anonymous : 1;
};

struct _Group_Lookup
{
   char *name;
};

struct _String_Lookup
{
   char *name;
   int *dest;
};

struct _Image_Lookup
{
   char *name;
   int *dest;
   Eina_Bool *set;
};

struct _Slave_Lookup
{
   int *master;
   int *slave;
};

struct _Code_Lookup
{
   char *ptr;
   int   len;
   int   val;
   Eina_Bool set;
};

static void data_process_string(Edje_Part_Collection *pc, const char *prefix, char *s, void (*func)(Edje_Part_Collection *pc, char *name, char *ptr, int len));

Edje_File *edje_file = NULL;
Eina_List *edje_collections = NULL;
Eina_List *externals = NULL;
Eina_List *fonts = NULL;
Eina_List *codes = NULL;
Eina_List *code_lookups = NULL;
Eina_List *aliases = NULL;

static Eet_Data_Descriptor *edd_edje_file = NULL;
static Eet_Data_Descriptor *edd_edje_part_collection = NULL;

static Eina_List *part_lookups = NULL;
static Eina_List *program_lookups = NULL;
static Eina_List *group_lookups = NULL;
static Eina_List *image_lookups = NULL;
static Eina_List *part_slave_lookups = NULL;
static Eina_List *image_slave_lookups= NULL;

#define ABORT_WRITE(eet_file, file) \
   eet_close(eet_file); \
   unlink(file); \
   exit(-1);

void
error_and_abort(Eet_File *ef, const char *fmt, ...)
{
   va_list ap;

   fprintf(stderr, "%s: Error. ", progname);

   va_start(ap, fmt);
   vfprintf(stderr, fmt, ap);
   va_end(ap);
   ABORT_WRITE(ef, file_out);
}

void
data_setup(void)
{
   edd_edje_file = _edje_edd_edje_file;
   edd_edje_part_collection = _edje_edd_edje_part_collection;
}

static void
check_image_part_desc (Edje_Part_Collection *pc, Edje_Part *ep,
                       Edje_Part_Description_Image *epd, Eet_File *ef)
{
   unsigned int i;

#if 0 /* FIXME: This check sounds like not a useful one */
   if (epd->image.id == -1)
     ERR(ef, "Collection %s(%i): image attributes missing for "
	 "part \"%s\", description \"%s\" %f\n",
	 pc->part, pc->id, ep->name, epd->common.state.name, epd->common.state.value);
#endif

   for (i = 0; i < epd->image.tweens_count; ++i)
     {
	if (epd->image.tweens[i]->id == -1)
	  error_and_abort(ef, "Collection %i: tween image id missing for "
			  "part \"%s\", description \"%s\" %f\n",
			  pc->id, ep->name, epd->common.state.name, epd->common.state.value);
    }
}

static void
check_packed_items(Edje_Part_Collection *pc, Edje_Part *ep, Eet_File *ef)
{
   unsigned int i;

   for (i = 0; i < ep->items_count; ++i)
     {
	if (ep->items[i]->type == EDJE_PART_TYPE_GROUP && !ep->items[i]->source)
	  error_and_abort(ef, "Collection %i: missing source on packed item "
			  "of type GROUP in part \"%s\"\n",
			  pc->id, ep->name);
	if (ep->type == EDJE_PART_TYPE_TABLE && (ep->items[i]->col < 0 || ep->items[i]->row < 0))
	  error_and_abort(ef, "Collection %i: missing col/row on packed item "
			  "for part \"%s\" of type TABLE\n",
			  pc->id, ep->name);
     }
}

static void
check_nameless_state(Edje_Part_Collection *pc, Edje_Part *ep, Edje_Part_Description_Common *ed, Eet_File *ef)
{
   if (!ed->state.name)
      error_and_abort(ef, "Collection %i: description with state missing on part \"%s\"\n",
                      pc->id, ep->name);
}

static void
check_part (Edje_Part_Collection *pc, Edje_Part *ep, Eet_File *ef)
{
   unsigned int i;
   /* FIXME: check image set and sort them. */
   if (!ep->default_desc)
     error_and_abort(ef, "Collection %i: default description missing "
		     "for part \"%s\"\n", pc->id, ep->name);

   for (i = 0; i < ep->other.desc_count; ++i)
     check_nameless_state(pc, ep, ep->other.desc[i], ef);

   if (ep->type == EDJE_PART_TYPE_IMAGE)
     {
	check_image_part_desc(pc, ep, (Edje_Part_Description_Image*) ep->default_desc, ef);

	for (i = 0; i < ep->other.desc_count; ++i)
	  check_image_part_desc (pc, ep, (Edje_Part_Description_Image*) ep->other.desc[i], ef);
     }
   else if ((ep->type == EDJE_PART_TYPE_BOX) ||
	    (ep->type == EDJE_PART_TYPE_TABLE))
     check_packed_items(pc, ep, ef);
}

static void
check_program (Edje_Part_Collection *pc, Edje_Program *ep, Eet_File *ef)
{
   switch (ep->action)
     {
      case EDJE_ACTION_TYPE_STATE_SET:
      case EDJE_ACTION_TYPE_ACTION_STOP:
      case EDJE_ACTION_TYPE_DRAG_VAL_SET:
      case EDJE_ACTION_TYPE_DRAG_VAL_STEP:
      case EDJE_ACTION_TYPE_DRAG_VAL_PAGE:
	 if (!ep->targets)
	   error_and_abort(ef, "Collection %i: target missing in program "
			   "\"%s\"\n", pc->id, ep->name);
	 break;
      default:
	 break;
     }
}

static int
data_write_header(Eet_File *ef)
{
   int bytes = 0;

   if (edje_file)
     {
	if (edje_file->collection)
	  {
	     Edje_Part_Collection_Directory_Entry *ce;

	     /* copy aliases into collection directory */
	     EINA_LIST_FREE(aliases, ce)
	       {
		  Edje_Part_Collection_Directory_Entry *sce;
		  Eina_Iterator *it;

		  if (!ce->entry)
		    error_and_abort(ef, "Collection %i: name missing.\n", ce->id);

		  it = eina_hash_iterator_data_new(edje_file->collection);

		  EINA_ITERATOR_FOREACH(it, sce)
		    if (ce->id == sce->id)
		      {
			 memcpy(&ce->count, &sce->count, sizeof (ce->count));
			 break;
		      }

		  if (!sce)
		    error_and_abort(ef, "Collection %s (%i) can't find an correct alias.\n", ce->entry, ce->id);

		  eina_iterator_free(it);

		  eina_hash_direct_add(edje_file->collection, ce->entry, ce);
	       }
	  }
	bytes = eet_data_write(ef, edd_edje_file, "edje/file", edje_file, 1);
	if (bytes <= 0)
	  error_and_abort(ef, "Unable to write \"edje_file\" entry to \"%s\" \n",
			  file_out);
     }

   if (verbose)
     {
	printf("%s: Wrote %9i bytes (%4iKb) for \"edje_file\" header\n",
	       progname, bytes, (bytes + 512) / 1024);
     }

   return bytes;
}

static int
data_write_fonts(Eet_File *ef, int *font_num, int *input_bytes, int *input_raw_bytes)
{
   Eina_Iterator *it;
   int bytes = 0;
   int total_bytes = 0;
   Font *fn;

   if (!edje_file->fonts)
     return 0;

   it = eina_hash_iterator_data_new(edje_file->fonts);
   EINA_ITERATOR_FOREACH(it, fn)
     {
	void *fdata = NULL;
	int fsize = 0;
	Eina_List *ll;
	FILE *f;

	f = fopen(fn->file, "rb");
	if (f)
	  {
	     long pos;

	     fseek(f, 0, SEEK_END);
	     pos = ftell(f);
	     rewind(f);
	     fdata = malloc(pos);
	     if (fdata)
	       {
		  if (fread(fdata, pos, 1, f) != 1)
		    error_and_abort(ef, "Unable to read all of font "
				    "file \"%s\"\n", fn->file);
		  fsize = pos;
	       }
	     fclose(f);
	  }
	else
	  {
	     char *data;

	     EINA_LIST_FOREACH(fnt_dirs, ll, data)
	       {
		  char buf[4096];

		  snprintf(buf, sizeof(buf), "%s/%s", data, fn->file);
		  f = fopen(buf, "rb");
		  if (f)
		    {
		       long pos;

		       fseek(f, 0, SEEK_END);
		       pos = ftell(f);
		       rewind(f);
		       fdata = malloc(pos);
		       if (fdata)
			 {
			    if (fread(fdata, pos, 1, f) != 1)
			      error_and_abort(ef, "Unable to read all of font "
					      "file \"%s\"\n", buf);
			    fsize = pos;
			 }
		       fclose(f);
		       if (fdata) break;
		    }
	       }
	  }
	if (!fdata)
	  {
	     error_and_abort(ef, "Unable to load font part \"%s\" entry "
			     "to %s \n", fn->file, file_out);
	  }
	else
	  {
	     char buf[4096];

	     snprintf(buf, sizeof(buf), "edje/fonts/%s", fn->name);
	     bytes = eet_write(ef, buf, fdata, fsize, 1);
	     if (bytes <= 0)
	       error_and_abort(ef, "Unable to write font part \"%s\" as \"%s\" "
			       "part entry to %s \n", fn->file, buf, file_out);

	     *font_num += 1;
	     total_bytes += bytes;
	     *input_bytes += fsize;
	     *input_raw_bytes += fsize;

	     if (verbose)
	       {
		  printf("%s: Wrote %9i bytes (%4iKb) for \"%s\" font entry \"%s\" compress: [real: %2.1f%%]\n",
			 progname, bytes, (bytes + 512) / 1024, buf, fn->file,
			 100 - (100 * (double)bytes) / ((double)(fsize))
			 );
	       }
	     free(fdata);
	  }
     }
   eina_iterator_free(it);

   return total_bytes;
}

static void
error_and_abort_image_load_error(Eet_File *ef, const char *file, int error)
{
   const char *errmsg = evas_load_error_str(error);
   char hint[1024] = "";

   if (error == EVAS_LOAD_ERROR_DOES_NOT_EXIST)
     {
	snprintf
	  (hint, sizeof(hint),
	   " Check if path to file \"%s\" is correct "
	   "(both directory and file name).",
	   file);
     }
   else if (error == EVAS_LOAD_ERROR_CORRUPT_FILE)
     {
	snprintf
	  (hint, sizeof(hint),
	   " Check if file \"%s\" is consistent.",
	   file);
     }
   else if (error == EVAS_LOAD_ERROR_UNKNOWN_FORMAT)
     {
	const char *ext = strrchr(file, '.');
	const char **itr, *known_loaders[] = {
	  /* list from evas_image_load.c */
	  "png",
	  "jpg",
	  "jpeg",
	  "jfif",
	  "eet",
	  "edj",
	  "eap",
	  "edb",
	  "xpm",
	  "tiff",
	  "tif",
	  "svg",
	  "svgz",
	  "gif",
	  "pbm",
	  "pgm",
	  "ppm",
	  "pnm",
	  NULL
	};

	if (!ext)
	  {
	     snprintf
	       (hint, sizeof(hint),
		" File \"%s\" does not have an extension, "
		"maybe it should?",
		file);
	     goto show_err;
	  }

	ext++;
	for (itr = known_loaders; *itr; itr++)
	  {
	     if (strcasecmp(ext, *itr) == 0)
	       {
		  snprintf
		    (hint, sizeof(hint),
		     " Check if Evas was compiled with %s module enabled and "
		     "all required dependencies exist.",
		     ext);
		  goto show_err;
	       }
	  }

	snprintf(hint, sizeof(hint),
		 " Check if Evas supports loading files of type \"%s\" (%s) "
		 "and this module was compiled and all its dependencies exist.",
		 ext, file);
     }
 show_err:
   error_and_abort
     (ef, "Unable to load image \"%s\" used by file \"%s\": %s.%s\n",
      file, file_out, errmsg, hint);
}

static int
data_write_images(Eet_File *ef, int *image_num, int *input_bytes, int *input_raw_bytes)
{
   unsigned int i;
   int bytes = 0;
   int total_bytes = 0;

   if ((edje_file) && (edje_file->image_dir))
     {
	Ecore_Evas *ee;
	Evas *evas;
	Edje_Image_Directory_Entry *img;

	ecore_init();
	ecore_evas_init();

	ee = ecore_evas_buffer_new(1, 1);
	if (!ee)
	  error_and_abort(ef, "Cannot create buffer engine canvas for image "
			  "load.\n");

	evas = ecore_evas_get(ee);
	for (i = 0; i < edje_file->image_dir->entries_count; i++)
	  {
	     img = &edje_file->image_dir->entries[i];

	     if (img->source_type == EDJE_IMAGE_SOURCE_TYPE_EXTERNAL)
	       {
	       }
	     else
	       {
		  Evas_Object *im;
		  Eina_List *ll;
		  char *data;
		  int load_err = EVAS_LOAD_ERROR_NONE;

		  im = NULL;
		  EINA_LIST_FOREACH(img_dirs, ll, data)
		    {
		       char buf[4096];

		       snprintf(buf, sizeof(buf), "%s/%s",
				data, img->entry);
		       im = evas_object_image_add(evas);
		       if (im)
			 {
			    evas_object_image_file_set(im, buf, NULL);
			    load_err = evas_object_image_load_error_get(im);
			    if (load_err == EVAS_LOAD_ERROR_NONE)
			      break;
			    evas_object_del(im);
			    im = NULL;
			    if (load_err != EVAS_LOAD_ERROR_DOES_NOT_EXIST)
			      break;
			 }
		    }
		  if ((!im) && (load_err == EVAS_LOAD_ERROR_DOES_NOT_EXIST))
		    {
		       im = evas_object_image_add(evas);
		       if (im)
			 {
			    evas_object_image_file_set(im, img->entry, NULL);
			    load_err = evas_object_image_load_error_get(im);
			    if (load_err != EVAS_LOAD_ERROR_NONE)
			      {
				 evas_object_del(im);
				 im = NULL;
			      }
			 }
		    }
		  if (im)
		    {
		       void *im_data;
		       int  im_w, im_h;
		       int  im_alpha;
		       char buf[256];

		       evas_object_image_size_get(im, &im_w, &im_h);
		       im_alpha = evas_object_image_alpha_get(im);
		       im_data = evas_object_image_data_get(im, 0);
		       if ((im_data) && (im_w > 0) && (im_h > 0))
			 {
			    int mode, qual;

			    snprintf(buf, sizeof(buf), "edje/images/%i", img->id);
			    qual = 80;
			    if ((img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT) &&
				(img->source_param == 0))
			      mode = 0; /* RAW */
			    else if ((img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT) &&
				     (img->source_param == 1))
			      mode = 1; /* COMPRESS */
			    else
			      mode = 2; /* LOSSY */
			    if ((mode == 0) && (no_raw))
			      {
				 mode = 1; /* promote compression */
				 img->source_param = 95;
			      }
			    if ((mode == 2) && (no_lossy)) mode = 1; /* demote compression */
			    if ((mode == 1) && (no_comp))
			      {
				 if (no_lossy) mode = 0; /* demote compression */
				 else if (no_raw)
				   {
				      img->source_param = 90;
				      mode = 2; /* no choice. lossy */
				   }
			      }
			    if (mode == 2)
			      {
				 qual = img->source_param;
				 if (qual < min_quality) qual = min_quality;
				 if (qual > max_quality) qual = max_quality;
			      }
			    if (mode == 0)
			      bytes = eet_data_image_write(ef, buf,
							   im_data, im_w, im_h,
							   im_alpha,
							   0, 0, 0);
			    else if (mode == 1)
			      bytes = eet_data_image_write(ef, buf,
							   im_data, im_w, im_h,
							   im_alpha,
							   1, 0, 0);
			    else if (mode == 2)
			      bytes = eet_data_image_write(ef, buf,
							   im_data, im_w, im_h,
							   im_alpha,
							   0, qual, 1);
			    if (bytes <= 0)
			      error_and_abort(ef, "Unable to write image part "
					      "\"%s\" as \"%s\" part entry to "
					      "%s\n", img->entry, buf,
					      file_out);

			    *image_num += 1;
			    total_bytes += bytes;
			 }
		       else
			 {
			    error_and_abort_image_load_error
			      (ef, img->entry, load_err);
			 }

		       if (verbose)
			 {
			    struct stat st;
			    const char *file = NULL;

			    evas_object_image_file_get(im, &file, NULL);
			    if (!file || (stat(file, &st) != 0))
			      st.st_size = 0;
			    *input_bytes += st.st_size;
			    *input_raw_bytes += im_w * im_h * 4;
			    printf("%s: Wrote %9i bytes (%4iKb) for \"%s\" image entry \"%s\" compress: [raw: %2.1f%%] [real: %2.1f%%]\n",
				   progname, bytes, (bytes + 512) / 1024, buf, img->entry,
				   100 - (100 * (double)bytes) / ((double)(im_w * im_h * 4)),
				   100 - (100 * (double)bytes) / ((double)(st.st_size))
				   );
			 }
		       evas_object_del(im);
		    }
		  else
		    {
		       error_and_abort_image_load_error
			 (ef, img->entry, load_err);
		    }
	       }
	  }
	ecore_evas_free(ee);
	ecore_evas_shutdown();
	ecore_shutdown();
     }

   return total_bytes;
}

static int
data_write_sounds(Eet_File * ef, int *sound_num, int *input_bytes, int *input_raw_bytes)
{
   int bytes = 0;
   int total_bytes = 0;
   
   if ((edje_file) && (edje_file->sound_dir))
     {
        Eina_List *ll;
        Edje_Sound_Sample *sample;
#ifdef HAVE_LIBSNDFILE
        Edje_Sound_Encode *enc_info;
#endif
        char *dir_path = NULL;
        char snd_path[PATH_MAX];
        char sndid_str[15];
        void *fdata;
        FILE *fp = NULL;
        struct stat st;
        int size = 0;
        int i;

        for (i = 0; i < (int)edje_file->sound_dir->samples_count; i++)
          {
             sample = &edje_file->sound_dir->samples[i];
             memset(&st, 0, sizeof(struct stat));
             
             // Search the Sound file in all the -sd ( sound directory )
             EINA_LIST_FOREACH(snd_dirs, ll, dir_path)
               {
                  snprintf((char *)snd_path, sizeof(snd_path), "%s/%s", dir_path,
                           sample->snd_src);
                  stat(snd_path, &st);
                  if (st.st_size) break;
               }
             if (!st.st_size)
               {
                  snprintf((char *)snd_path, sizeof(snd_path), "%s",
                           sample->snd_src);
                  stat(snd_path, &st);
               }
             size = st.st_size;
             if (!size)
               {
                  ERR("%s: Error. Unable to load sound source file : %s",
                      progname, sample->snd_src);
                  exit(-1);
               }
#ifdef HAVE_LIBSNDFILE
             enc_info = _edje_multisense_encode(snd_path, sample, sample->quality);
             
             stat(enc_info->file, &st);
             size = st.st_size;
             fp = fopen(enc_info->file, "rb");
#else
             fp = fopen(snd_path, "rb");
#endif
             if (!fp)
               {
                  ERR("%s: Error: Unable to load sound data of: %s",
                      progname, sample->name);
                  exit(-1);
               }
             
             snprintf(sndid_str, sizeof(sndid_str), "edje/sounds/%i", sample->id);
             fdata = malloc(size);
             if (!fdata)
               {
                  ERR("%s: Error. %s:%i while allocating memory to load file \"%s\"",
                      progname, file_in, line, snd_path);
                  exit(-1);
               }
             if (fread(fdata, size, 1, fp))
               bytes = eet_write(ef, sndid_str, fdata, size, EINA_FALSE);
             free(fdata);
             fclose(fp);
             
#ifdef HAVE_LIBSNDFILE
             //If encoded temporary file, delete it.
             if (enc_info->encoded) unlink(enc_info->file);
#endif
             *sound_num += 1;
             total_bytes += bytes;
             *input_bytes += size;
             *input_raw_bytes += size;

             if (verbose)
               {
#ifdef HAVE_LIBSNDFILE
                  printf ("%s: Wrote %9i bytes (%4iKb) for \"%s\" %s sound entry"
                          "\"%s\" \n", progname, bytes, (bytes + 512) / 1024,
                          sndid_str, enc_info->comp_type, sample->name);
#else
                  printf ("%s: Wrote %9i bytes (%4iKb) for \"%s\" %s sound entry"
                          "\"%s\" \n", progname, bytes, (bytes + 512) / 1024,
                          sndid_str, "RAW PCM", sample->name);
#endif
               }
#ifdef HAVE_LIBSNDFILE
             if ((enc_info->file) && (!enc_info->encoded)) eina_stringshare_del(enc_info->file);
             if (enc_info) free(enc_info);
             enc_info = NULL;
#endif
          }
     }
   return total_bytes;
}

static void
check_groups(Eet_File *ef)
{
   Edje_Part_Collection *pc;
   Eina_List *l;

   /* sanity checks for parts and programs */
   EINA_LIST_FOREACH(edje_collections, l, pc)
     {
	unsigned int i;

	for (i = 0; i < pc->parts_count; ++i)
	  check_part(pc, pc->parts[i], ef);

#define CHECK_PROGRAM(Type, Pc, It)				\
	for (It = 0; It < Pc->programs.Type ## _count; ++It)	\
	  check_program(Pc, Pc->programs.Type[i], ef);		\

	CHECK_PROGRAM(fnmatch, pc, i);
	CHECK_PROGRAM(strcmp, pc, i);
	CHECK_PROGRAM(strncmp, pc, i);
	CHECK_PROGRAM(strrncmp, pc, i);
	CHECK_PROGRAM(nocmp, pc, i);
     }
}

static int
data_write_groups(Eet_File *ef, int *collection_num)
{
   Eina_List *l;
   Edje_Part_Collection *pc;
   int bytes = 0;
   int total_bytes = 0;

   EINA_LIST_FOREACH(edje_collections, l, pc)
     {
	char buf[4096];

	snprintf(buf, sizeof(buf), "edje/collections/%i", pc->id);
	bytes = eet_data_write(ef, edd_edje_part_collection, buf, pc, 1);
	if (bytes <= 0)
	  error_and_abort(ef, "Error. Unable to write \"%s\" part entry "
			  "to %s\n", buf, file_out);

	*collection_num += 1;
	total_bytes += bytes;

	if (verbose)
	  {
	     printf("%s: Wrote %9i bytes (%4iKb) for \"%s\" collection entry\n",
		    progname, bytes, (bytes + 512) / 1024, buf);
	  }
     }

   return total_bytes;
}

static void
create_script_file(Eet_File *ef, const char *filename, const Code *cd, int fd)
{
   FILE *f = fdopen(fd, "wb");
   if (!f)
     error_and_abort(ef, "Unable to open temp file \"%s\" for script "
		     "compilation.\n", filename);

   Eina_List *ll;
   Code_Program *cp;

   fprintf(f, "#include <edje>\n");
   int ln = 2;

   if (cd->shared)
     {
	while (ln < (cd->l1 - 1))
	  {
	     fprintf(f, " \n");
	     ln++;
	  }
	{
	   char *sp;
	   int hash = 0;
	   int newlined = 0;

	   for (sp = cd->shared; *sp; sp++)
	     {
		if ((sp[0] == '#') && (newlined))
		  {
		     hash = 1;
		  }
		newlined = 0;
		if (sp[0] == '\n') newlined = 1;
		if (!hash) fputc(sp[0], f);
		else if (sp[0] == '\n') hash = 0;
	     }
	   fputc('\n', f);
	}
	ln += cd->l2 - cd->l1 + 1;
     }
   EINA_LIST_FOREACH(cd->programs, ll, cp)
     {
	if (cp->script)
	  {
	     while (ln < (cp->l1 - 1))
	       {
		  fprintf(f, " \n");
		  ln++;
	       }
	     /* FIXME: this prototype needs to be */
	     /* formalised and set in stone */
	     fprintf(f, "public _p%i(sig[], src[]) {", cp->id);
	     {
		char *sp;
		int hash = 0;
		int newlined = 0;

		for (sp = cp->script; *sp; sp++)
		  {
		     if ((sp[0] == '#') && (newlined))
		       {
			  hash = 1;
		       }
		     newlined = 0;
		     if (sp[0] == '\n') newlined = 1;
		     if (!hash) fputc(sp[0], f);
		     else if (sp[0] == '\n') hash = 0;
		  }
	     }
	     fprintf(f, "}\n");
	     ln += cp->l2 - cp->l1 + 1;
	  }
     }

   fclose(f);
}

static void
compile_script_file(Eet_File *ef, const char *source, const char *output,
		    int script_num, int fd)
{
   FILE *f;
   char buf[4096];
   int ret;

   snprintf(buf, sizeof(buf),
	    "embryo_cc -i %s/include -o %s %s",
	    eina_prefix_data_get(pfx), output, source);
   ret = system(buf);

   /* accept warnings in the embryo code */
   if (ret < 0 || ret > 1)
     error_and_abort(ef, "Compiling script code not clean.\n");

   f = fdopen(fd, "rb");
   if (!f)
     error_and_abort(ef, "Unable to open script object \"%s\" for reading.\n",
		     output);

   fseek(f, 0, SEEK_END);
   int size = ftell(f);
   rewind(f);

   if (size > 0)
     {
	void *data = malloc(size);

	if (data)
	  {
	     if (fread(data, size, 1, f) != 1)
	       error_and_abort(ef, "Unable to read all of script object "
			       "\"%s\"\n", output);

	     snprintf(buf, sizeof(buf), "edje/scripts/embryo/compiled/%i", script_num);
	     eet_write(ef, buf, data, size, 1);
	     free(data);
	  }
     }

   fclose(f);
}

static void
data_write_scripts(Eet_File *ef)
{
   Eina_List *l;
   int i;

   if (!tmp_dir)
#ifdef HAVE_EVIL
     tmp_dir = (char *)evil_tmpdir_get();
#else
     tmp_dir = "/tmp";
#endif

   for (i = 0, l = codes; l; l = eina_list_next(l), i++)
     {
	char tmpn[PATH_MAX];
	char tmpo[PATH_MAX];
	int fd;
	Code *cd = eina_list_data_get(l);

	if (cd->is_lua)
	  continue;
	if ((!cd->shared) && (!cd->programs))
	  continue;

	snprintf(tmpn, PATH_MAX, "%s/edje_cc.sma-tmp-XXXXXX", tmp_dir);
	fd = mkstemp(tmpn);
	if (fd < 0)
	  error_and_abort(ef, "Unable to open temp file \"%s\" for script "
			  "compilation.\n", tmpn);

	create_script_file(ef, tmpn, cd, fd);

	snprintf(tmpo, PATH_MAX, "%s/edje_cc.amx-tmp-XXXXXX", tmp_dir);
	fd = mkstemp(tmpo);
	if (fd < 0)
	  {
	     unlink(tmpn);
	     error_and_abort(ef, "Unable to open temp file \"%s\" for script "
			     "compilation.\n", tmpn);
	  }
	compile_script_file(ef, tmpn, tmpo, i, fd);

	unlink(tmpn);
	unlink(tmpo);

        if (!no_save)
          {
             char buf[PATH_MAX];
             Eina_List *ll;
             Code_Program *cp;

             if (cd->original)
               {
                  snprintf(buf, PATH_MAX, "edje/scripts/embryo/source/%i", i);
                  eet_write(ef, buf, cd->original, strlen(cd->original) + 1, 1);
               }
             EINA_LIST_FOREACH(cd->programs, ll, cp)
               {
                  if (!cp->original)
                    continue;
                  snprintf(buf, PATH_MAX, "edje/scripts/embryo/source/%i/%i", i,
                           cp->id);
                  eet_write(ef, buf, cp->original, strlen(cp->original) + 1, 1);
               }
          }
     }
}

typedef struct _Edje_Lua_Script_Writer_Struct Edje_Lua_Script_Writer_Struct;

struct _Edje_Lua_Script_Writer_Struct
{
   char *buf;
   int size;
};

#ifdef LUA_BINARY
static int
_edje_lua_script_writer(lua_State *L __UNUSED__, const void *chunk_buf, size_t chunk_size, void *_data)
{
   Edje_Lua_Script_Writer_Struct *data;
   void *old;

   data = (Edje_Lua_Script_Writer_Struct *)_data;
   old = data->buf;
   data->buf = malloc (data->size + chunk_size);
   memcpy (data->buf, old, data->size);
   memcpy (&((data->buf)[data->size]), chunk_buf, chunk_size);
   if (old)
     free (old);
   data->size += chunk_size;

   return 0;
}
#endif

void
_edje_lua_error_and_abort(lua_State * L, int err_code, Eet_File *ef)
{
   char *err_type;

   switch (err_code)
     {
     case LUA_ERRRUN:
	err_type = "runtime";
	break;
     case LUA_ERRSYNTAX:
	err_type = "syntax";
	break;
     case LUA_ERRMEM:
	err_type = "memory allocation";
	break;
     case LUA_ERRERR:
	err_type = "error handler";
	break;
     default:
	err_type = "unknown";
	break;
     }
   error_and_abort(ef, "Lua %s error: %s\n", err_type, lua_tostring(L, -1));
}


static void
data_write_lua_scripts(Eet_File *ef)
{
   Eina_List *l;
   Eina_List *ll;
   Code_Program *cp;
   int i;

   for (i = 0, l = codes; l; l = eina_list_next(l), i++)
     {
	char buf[4096];
	Code *cd;
	lua_State *L;
	int ln = 1;
	luaL_Buffer b;
	Edje_Lua_Script_Writer_Struct data;
#ifdef LUA_BINARY
	int err_code;
#endif

	cd = (Code *)eina_list_data_get(l);
	if (!cd->is_lua)
	  continue;
	if ((!cd->shared) && (!cd->programs))
	  continue;
	
	L = luaL_newstate();
	if (!L)
	  error_and_abort(ef, "Lua error: Lua state could not be initialized\n");

	luaL_buffinit(L, &b);

	data.buf = NULL;
	data.size = 0;
	if (cd->shared)
	  {
	     while (ln < (cd->l1 - 1))
	       {
		  luaL_addchar(&b, '\n');
		  ln++;
	       }
	     luaL_addstring(&b, cd->shared);
	     ln += cd->l2 - cd->l1;
	  }

	EINA_LIST_FOREACH(cd->programs, ll, cp)
	  {
	     if (cp->script)
	       {
		  while (ln < (cp->l1 - 1))
		    {
		       luaL_addchar(&b, '\n');
		       ln++;
		    }
		  luaL_addstring(&b, "_G[");
		  lua_pushnumber(L, cp->id);
		  luaL_addvalue(&b);
		  luaL_addstring(&b, "] = function (ed, signal, source)");
		  luaL_addstring(&b, cp->script);
		  luaL_addstring(&b, "end\n");
		  ln += cp->l2 - cp->l1 + 1;
	       }
	  }
	luaL_pushresult(&b);
#ifdef LUA_BINARY
	if (err_code = luaL_loadstring(L, lua_tostring (L, -1)))
	  _edje_lua_error_and_abort(L, err_code, ef);
	lua_dump(L, _edje_lua_script_writer, &data);
#else // LUA_PLAIN_TEXT
	data.buf = (char *)lua_tostring(L, -1);
	data.size = strlen(data.buf);
#endif
	//printf("lua chunk size: %d\n", data.size);

	/* 
	 * TODO load and test Lua chunk
	 */

	/*
	   if (luaL_loadbuffer(L, globbuf, globbufsize, "edje_lua_script"))
	   printf("lua load error: %s\n", lua_tostring (L, -1));
	   if (lua_pcall(L, 0, 0, 0))
	   printf("lua call error: %s\n", lua_tostring (L, -1));
	 */
	
	snprintf(buf, sizeof(buf), "edje/scripts/lua/%i", i);
	eet_write(ef, buf, data.buf, data.size, 1);
#ifdef LUA_BINARY
	free(data.buf);
#endif
	lua_close(L);
     }
}

void
data_write(void)
{
   Eet_File *ef;
   int input_bytes = 0;
   int total_bytes = 0;
   int src_bytes = 0;
   int fmap_bytes = 0;
   int input_raw_bytes = 0;
   int image_num = 0;
   int sound_num = 0;
   int font_num = 0;
   int collection_num = 0;

   if (!edje_file)
     {
	ERR("%s: Error. No data to put in \"%s\"",
	    progname, file_out);
	exit(-1);
     }

   ef = eet_open(file_out, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	ERR("%s: Error. Unable to open \"%s\" for writing output",
	    progname, file_out);
	exit(-1);
     }

   check_groups(ef);

   total_bytes += data_write_header(ef);
   total_bytes += data_write_fonts(ef, &font_num, &input_bytes,
				   &input_raw_bytes);
   total_bytes += data_write_images(ef, &image_num, &input_bytes,
				    &input_raw_bytes);
   total_bytes += data_write_sounds(ef, &sound_num, &input_bytes,
                &input_raw_bytes);

   total_bytes += data_write_groups(ef, &collection_num);
   data_write_scripts(ef);
   data_write_lua_scripts(ef);

   src_bytes = source_append(ef);
   total_bytes += src_bytes;
   fmap_bytes = source_fontmap_save(ef, fonts);
   total_bytes += fmap_bytes;

   eet_close(ef);

   if (verbose)
     {
	struct stat st;

	if (stat(file_in, &st) != 0)
	  st.st_size = 0;
	input_bytes += st.st_size;
	input_raw_bytes += st.st_size;
	printf("Summary:\n"
	       "  Wrote %i collections\n"
	       "  Wrote %i images\n"
          "  Wrote %i sounds\n"
	       "  Wrote %i fonts\n"
	       "  Wrote %i bytes (%iKb) of original source data\n"
	       "  Wrote %i bytes (%iKb) of original source font map\n"
	       "Conservative compression summary:\n"
	       "  Wrote total %i bytes (%iKb) from %i (%iKb) input data\n"
	       "  Output file is %3.1f%% the size of the input data\n"
	       "  Saved %i bytes (%iKb)\n"
	       "Raw compression summary:\n"
	       "  Wrote total %i bytes (%iKb) from %i (%iKb) raw input data\n"
	       "  Output file is %3.1f%% the size of the raw input data\n"
	       "  Saved %i bytes (%iKb)\n"
	       ,
	       collection_num,
	       image_num,
          sound_num,
	       font_num,
	       src_bytes, (src_bytes + 512) / 1024,
	       fmap_bytes, (fmap_bytes + 512) / 1024,
	       total_bytes, (total_bytes + 512) / 1024,
	       input_bytes, (input_bytes + 512) / 1024,
	       (100.0 * (double)total_bytes) / (double)input_bytes,
	       input_bytes - total_bytes,
	       (input_bytes - total_bytes + 512) / 1024,
	       total_bytes, (total_bytes + 512) / 1024,
	       input_raw_bytes, (input_raw_bytes + 512) / 1024,
	       (100.0 * (double)total_bytes) / (double)input_raw_bytes,
	       input_raw_bytes - total_bytes,
	       (input_raw_bytes - total_bytes + 512) / 1024);
     }
}

void
reorder_parts(void)
{
   Edje_Part_Collection *pc;
   Edje_Part **parts;
   Edje_Part_Parser *ep, *ep2;
   Eina_List *l;

   /* sanity checks for parts and programs */
   EINA_LIST_FOREACH(edje_collections, l, pc)
     {
        unsigned int i, j, k;
	Eina_Bool found = EINA_FALSE;

	for (i = 0; i < pc->parts_count; i++)
          {
             ep = (Edje_Part_Parser *)pc->parts[i];
             if (ep->reorder.insert_before && ep->reorder.insert_after)
               ERR("%s: Error. Unable to use together insert_before and insert_after in part \"%s\".", progname, pc->parts[i]->name);

             if (ep->reorder.done)
               {
                  continue;
               }
             if (ep->reorder.insert_before || ep->reorder.insert_after)
               {
                  found = EINA_FALSE;
                  for (j = 0; j < pc->parts_count; j++)
                    {
                       if (ep->reorder.insert_before &&
                           !strcmp(ep->reorder.insert_before, pc->parts[j]->name))
                         {
                            ep2 = (Edje_Part_Parser *)pc->parts[j];
                            if (ep2->reorder.after)
                              ERR("%s: Error. The part \"%s\" is ambiguous ordered part.", progname, pc->parts[i]->name);
                            if (ep2->reorder.linked_prev)
                              ERR("%s: Error. Unable to insert two or more parts in same part \"%s\".", progname, pc->parts[j]->name);
                            k = j - 1;
			    found = EINA_TRUE;
                            ep2->reorder.linked_prev += ep->reorder.linked_prev + 1;
                            ep->reorder.before = (Edje_Part_Parser *)pc->parts[j];
                            while (ep2->reorder.before)
                              {
                                 ep2->reorder.before->reorder.linked_prev = ep2->reorder.linked_prev + 1;
                                 ep2 = ep2->reorder.before;
                              }
                            break;
                         }
                       else if (ep->reorder.insert_after &&
                           !strcmp(ep->reorder.insert_after, pc->parts[j]->name))
                         {
                            ep2 = (Edje_Part_Parser *)pc->parts[j];
                            if (ep2->reorder.before)
                              ERR("%s: Error. The part \"%s\" is ambiguous ordered part.", progname, pc->parts[i]->name);
                            if (ep2->reorder.linked_next)
                              ERR("%s: Error. Unable to insert two or more parts in same part \"%s\".", progname, pc->parts[j]->name);
                            k = j;
			    found = EINA_TRUE;
                            ep2->reorder.linked_next += ep->reorder.linked_next + 1;
                            ep->reorder.after = (Edje_Part_Parser *)pc->parts[j];
                            while (ep2->reorder.after)
                              {
                                 ep2->reorder.after->reorder.linked_next = ep2->reorder.linked_next + 1;
                                 ep2 = ep2->reorder.after;
                              }
                            break;
                         }
                    }
                  if (found)
                    {
		       unsigned int amount, linked;

                       if (((i > k) && ((i - ep->reorder.linked_prev) <= k))
                           || ((i < k) && ((i + ep->reorder.linked_next) >= k)))
                         ERR("%s: Error. The part order is wrong. It has circular dependency.",
                             progname);

                       amount = ep->reorder.linked_prev + ep->reorder.linked_next + 1;
                       linked = i - ep->reorder.linked_prev;
                       parts = malloc(amount * sizeof(Edje_Part));
                       for (j = 0 ; j < amount ; j++)
                         {
                            parts[j] = pc->parts[linked];
                            linked++;
                         }
                       if (i > k)
                         {
                            for (j = i - ep->reorder.linked_prev - 1 ; j >= k ; j--)
                              {
                                 pc->parts[j + amount] = pc->parts[j];
                                 pc->parts[j + amount]->id = j + amount;
                              }
                            for (j = 0 ; j < amount ; j++)
                              {
                                 pc->parts[j + k] = parts[j];
                                 pc->parts[j + k]->id = j + k;
                              }
                         }
                       else if (i < k)
                         {
                            for (j = i + ep->reorder.linked_next + 1 ; j <= k ; j++)
                              {
                                 pc->parts[j - amount] = pc->parts[j];
                                 pc->parts[j - amount]->id = j - amount;
                              }
                            for (j = 0 ; j < amount ; j++)
                              {
                                 pc->parts[j + k - amount + 1] = parts[j];
                                 pc->parts[j + k - amount + 1]->id = j + k - amount + 1;
                              }
                            i -= amount;
                         }
                       ep->reorder.done = EINA_TRUE;
                       free(parts);
                    }
               }
          }
     }
}

void
data_queue_group_lookup(char *name)
{
   Group_Lookup *gl;

   gl = mem_alloc(SZ(Group_Lookup));
   group_lookups = eina_list_append(group_lookups, gl);
   gl->name = mem_strdup(name);
}

void
data_queue_part_lookup(Edje_Part_Collection *pc, const char *name, int *dest)
{
   Eina_List *l;
   Part_Lookup *pl;

   EINA_LIST_FOREACH(part_lookups, l, pl)
     {
        if ((pl->pc == pc) && (pl->dest == dest))
          {
             free(pl->name);
             if (strlen(name) > 0)
               pl->name = mem_strdup(name);
             else
               {
                  part_lookups = eina_list_remove(part_lookups, pl);
                  free(pl);
               }
             return;
          }
     }

   pl = mem_alloc(SZ(Part_Lookup));
   part_lookups = eina_list_append(part_lookups, pl);
   pl->pc = pc;
   pl->name = mem_strdup(name);
   pl->dest = dest;
}

void
data_queue_copied_part_lookup(Edje_Part_Collection *pc, int *src, int *dest)
{
   Eina_List *l;
   Part_Lookup *pl;

   EINA_LIST_FOREACH(part_lookups, l, pl)
     {
        if (pl->dest == src)
          data_queue_part_lookup(pc, pl->name, dest);
     }
}

void
data_queue_anonymous_lookup(Edje_Part_Collection *pc, Edje_Program *ep, int *dest)
{
   Eina_List *l, *l2;
   Program_Lookup *pl;

   if (!ep) return ; /* FIXME: should we stop compiling ? */

   EINA_LIST_FOREACH(program_lookups, l, pl)
     {
        if (pl->u.ep == ep)
          {
             Code *cd;
             Code_Program *cp;

             cd = eina_list_data_get(eina_list_last(codes));

             EINA_LIST_FOREACH(cd->programs, l2, cp)
               {
                  if (&(cp->id) == pl->dest)
                    {
                       cd->programs = eina_list_remove(cd->programs, cp);
                       free(cp);
                       cp = NULL;
                    }
               }
             program_lookups = eina_list_remove(program_lookups, pl);
             free(pl);
          }
     }

   if (dest)
     {
        pl = mem_alloc(SZ(Program_Lookup));
        program_lookups = eina_list_append(program_lookups, pl);
        pl->pc = pc;
        pl->u.ep = ep;
        pl->dest = dest;
        pl->anonymous = EINA_TRUE;
     }
}

void
data_queue_copied_anonymous_lookup(Edje_Part_Collection *pc, int *src, int *dest)
{
   Eina_List *l;
   Program_Lookup *pl;
   unsigned int i;

   EINA_LIST_FOREACH(program_lookups, l, pl)
     {
        if (pl->dest == src)
          {
             for (i = 0 ; i < pc->programs.fnmatch_count ; i++)
               {
                  if (!strcmp(pl->u.ep->name, pc->programs.fnmatch[i]->name))
                    data_queue_anonymous_lookup(pc, pc->programs.fnmatch[i], dest);
               }
             for (i = 0 ; i < pc->programs.strcmp_count ; i++)
               {
                  if (!strcmp(pl->u.ep->name, pc->programs.strcmp[i]->name))
                    data_queue_anonymous_lookup(pc, pc->programs.strcmp[i], dest);
               }
             for (i = 0 ; i < pc->programs.strncmp_count ; i++)
               {
                  if (!strcmp(pl->u.ep->name, pc->programs.strncmp[i]->name))
                    data_queue_anonymous_lookup(pc, pc->programs.strncmp[i], dest);
               }
             for (i = 0 ; i < pc->programs.strrncmp_count ; i++)
               {
                  if (!strcmp(pl->u.ep->name, pc->programs.strrncmp[i]->name))
                    data_queue_anonymous_lookup(pc, pc->programs.strrncmp[i], dest);
               }
             for (i = 0 ; i < pc->programs.nocmp_count ; i++)
               {
                  if (!strcmp(pl->u.ep->name, pc->programs.nocmp[i]->name))
                    data_queue_anonymous_lookup(pc, pc->programs.nocmp[i], dest);
               }
          }
     }
}

void
data_queue_program_lookup(Edje_Part_Collection *pc, const char *name, int *dest)
{
   Program_Lookup *pl;

   if (!name) return ; /* FIXME: should we stop compiling ? */

   pl = mem_alloc(SZ(Program_Lookup));
   program_lookups = eina_list_append(program_lookups, pl);
   pl->pc = pc;
   pl->u.name = mem_strdup(name);
   pl->dest = dest;
   pl->anonymous = EINA_FALSE;
}

void
data_queue_copied_program_lookup(Edje_Part_Collection *pc, int *src, int *dest)
{
   Eina_List *l;
   Program_Lookup *pl;

   EINA_LIST_FOREACH(program_lookups, l, pl)
     {
        if (pl->dest == src)
          data_queue_program_lookup(pc, pl->u.name, dest);
     }
}

void
data_queue_image_lookup(char *name, int *dest, Eina_Bool *set)
{
   Image_Lookup *il;

   il = mem_alloc(SZ(Image_Lookup));
   image_lookups = eina_list_append(image_lookups, il);
   il->name = mem_strdup(name);
   il->dest = dest;
   il->set = set;
}

void
data_queue_copied_image_lookup(int *src, int *dest, Eina_Bool *set)
{
   Eina_List *l;
   Image_Lookup *il;

   EINA_LIST_FOREACH(image_lookups, l, il)
     {
        if (il->dest == src)
          data_queue_image_lookup(il->name, dest, set);
     }
}
void
data_queue_part_slave_lookup(int *master, int *slave)
{
   Slave_Lookup *sl;

   sl = mem_alloc(SZ(Slave_Lookup));
   part_slave_lookups = eina_list_append(part_slave_lookups, sl);
   sl->master = master;
   sl->slave = slave;
}

void
data_queue_image_slave_lookup(int *master, int *slave)
{
   Slave_Lookup *sl;

   sl = mem_alloc(SZ(Slave_Lookup));
   image_slave_lookups = eina_list_append(image_slave_lookups, sl);
   sl->master = master;
   sl->slave = slave;
}

void
handle_slave_lookup(Eina_List *list, int *master, int value)
{
   Eina_List *l;
   Slave_Lookup *sl;

   EINA_LIST_FOREACH(list, l, sl)
     if (sl->master == master)
       *sl->slave = value;
}

void
data_process_lookups(void)
{
   Edje_Part_Collection *pc;
   Part_Lookup *part;
   Program_Lookup *program;
   Group_Lookup *group;
   Image_Lookup *image;
   Eina_List *l;
   void *data;

   EINA_LIST_FOREACH(edje_collections, l, pc)
     {
	unsigned int count = 0;
	unsigned int i;

#define PROGRAM_ID_SET(Type, Pc, It, Count)				\
	for (It = 0; It < Pc->programs.Type ## _count; ++It)		\
	  {								\
	     Pc->programs.Type[It]->id = Count++;			\
	  }

	PROGRAM_ID_SET(fnmatch, pc, i, count);
	PROGRAM_ID_SET(strcmp, pc, i, count);
	PROGRAM_ID_SET(strncmp, pc, i, count);
	PROGRAM_ID_SET(strrncmp, pc, i, count);
	PROGRAM_ID_SET(nocmp, pc, i, count);

#undef PROGRAM_ID_SET
     }

   EINA_LIST_FREE(part_lookups, part)
     {
	Edje_Part *ep;
	unsigned int i;

	for (i = 0; i < part->pc->parts_count; ++i)
	  {
	     ep = part->pc->parts[i];

	     if ((ep->name) && (!strcmp(ep->name, part->name)))
	       {
		  handle_slave_lookup(part_slave_lookups, part->dest, ep->id);
		  *(part->dest) = ep->id;
		  break;
	       }
	  }

	if (i == part->pc->parts_count)
	  {
	     ERR("%s: Error. Unable to find part name \"%s\".",
		 progname, part->name);
	     exit(-1);
	  }

	free(part->name);
	free(part);
     }

   EINA_LIST_FREE(program_lookups, program)
     {
	unsigned int i;
	Eina_Bool find = EINA_FALSE;

#define PROGRAM_MATCH(Type, Pl, It)					\
	for (It = 0; It < Pl->pc->programs.Type ## _count; ++It)	\
	  {								\
	     Edje_Program *ep;						\
	     								\
	     ep = Pl->pc->programs.Type[It];				\
	     								\
	     if ((Pl->anonymous && ep == Pl->u.ep) ||			\
		 ((!Pl->anonymous) && (ep->name) && (!strcmp(ep->name, Pl->u.name)))) \
	       {							\
		  *(Pl->dest) = ep->id;					\
		  find = EINA_TRUE;					\
		  break;						\
	       }							\
	  }

	PROGRAM_MATCH(fnmatch, program, i);
	PROGRAM_MATCH(strcmp, program, i);
	PROGRAM_MATCH(strncmp, program, i);
	PROGRAM_MATCH(strrncmp, program, i);
	PROGRAM_MATCH(nocmp, program, i);

#undef PROGRAM_MATCH

	if (!find)
	  {
	     if (!program->anonymous)
	       ERR("%s: Error. Unable to find program name \"%s\".",
		   progname, program->u.name);
	     else
	       ERR("%s: Error. Unable to find anonymous program.",
		   progname);
	     exit(-1);
	  }

	if (!program->anonymous)
	  free(program->u.name);
	free(program);
     }

   EINA_LIST_FREE(group_lookups, group)
     {
	Edje_Part_Collection_Directory_Entry *de;

	de = eina_hash_find(edje_file->collection, group->name);

	if (!de)
          {
             ERR("%s: Error. Unable to find group name \"%s\".",
		 progname, group->name);
             exit(-1);
          }

        free(group->name);
        free(group);
     }

   EINA_LIST_FREE(image_lookups, image)
     {
	Edje_Image_Directory_Entry *de;
	Eina_Bool find = EINA_FALSE;

	if (edje_file->image_dir)
	  {
	     unsigned int i;

	     for (i = 0; i < edje_file->image_dir->entries_count; ++i)
	       {
		  de = edje_file->image_dir->entries + i;

		  if ((de->entry) && (!strcmp(de->entry, image->name)))
		    {
		       handle_slave_lookup(image_slave_lookups, image->dest, de->id);
		       if (de->source_type == EDJE_IMAGE_SOURCE_TYPE_EXTERNAL)
			 *(image->dest) = -de->id - 1;
		       else
			 *(image->dest) = de->id;
		       *(image->set) = EINA_FALSE;
		       find = EINA_TRUE;
		       break;
		    }
	       }

	     if (!find)
	       {
		 Edje_Image_Directory_Set *set;

		 for (i = 0; i < edje_file->image_dir->sets_count; ++i)
		   {
		      set = edje_file->image_dir->sets + i;

		      if ((set->name) && (!strcmp(set->name, image->name)))
			{
			   handle_slave_lookup(image_slave_lookups, image->dest, set->id);
			   *(image->dest) = set->id;
			   *(image->set) = EINA_TRUE;
			   find = EINA_TRUE;
			   break;
			}
		   }
	       }
	  }

	if (!find)
	  {
	     ERR("%s: Error. Unable to find image name \"%s\".",
		 progname, image->name);
	     exit(-1);
	  }

	free(image->name);
	free(image);
     }

   EINA_LIST_FREE(part_slave_lookups, data)
     free(data);

   EINA_LIST_FREE(image_slave_lookups, data)
     free(data);
}

static void
data_process_string(Edje_Part_Collection *pc, const char *prefix, char *s, void (*func)(Edje_Part_Collection *pc, char *name, char* ptr, int len))
{
   char *p;
   char *key;
   int keyl;
   int quote, escape;

   key = alloca(strlen(prefix) + 2 + 1);
   if (!key) return;
   strcpy(key, prefix);
   strcat(key, ":\"");
   keyl = strlen(key);
   quote = 0;
   escape = 0;
   for (p = s; (p) && (*p); p++)
     {
	if (!quote)
	  {
	     if (*p == '\"')
	       {
		  quote = 1;
		  p++;
	       }
	  }
	if (!quote)
	  {
	     if (!strncmp(p, key, keyl))
	       {
                  char *ptr;
                  int len;
                  int inesc = 0;
		  char *name;

                  ptr = p;
                  p += keyl;
		  while ((*p))
		    {
		       if (!inesc)
		         {
		  	    if (*p == '\\') inesc = 1;
			    else if (*p == '\"')
			      {
			         /* string concatenation, see below */
				 if (*(p + 1) != '\"')
				   break;
				 else
				   p++;
			      }
			 }
                       else
                            inesc = 0;
                       p++;
		    }
		  len = p - ptr + 1;
		  name = alloca(len);
		  if (name)
		    {
		       char *pp;
		       int i;

		       name[0] = 0;
		       pp = ptr + keyl;
		       inesc = 0;
		       i = 0;
		       while (*pp)
		         {
		    	    if (!inesc)
			      {
			         if (*pp == '\\') inesc = 1;
			         else if (*pp == '\"')
			    	   {
				      /* concat strings like "foo""bar" to "foobar" */
				      if (*(pp + 1) == '\"')
				        pp++;
				      else
				        {
				   	   name[i] = 0;
					   break;
					}
				   }
				 else
				   {
				      name[i] = *pp;
				      name[i + 1] = 0;
				      i++;
				   }
			      }
			    else
                              inesc = 0;
			    pp++;
			}
		      func(pc, name, ptr, len);
		   }
              }
	  }
	else
	  {
	     if (!escape)
	       {
		  if (*p == '\"') quote = 0;
		  else if (*p == '\\') escape = 1;
	       }
	     else if (escape)
	       {
		  escape = 0;
	       }
	  }
     }
}

static void
_data_queue_part_lookup(Edje_Part_Collection *pc, char *name, char *ptr, int len)
{
   Code_Lookup *cl;
   cl = mem_alloc(SZ(Code_Lookup));
   cl->ptr = ptr;
   cl->len = len;

   data_queue_part_lookup(pc, name, &(cl->val));

   code_lookups = eina_list_append(code_lookups, cl);
}
static void
_data_queue_program_lookup(Edje_Part_Collection *pc, char *name, char *ptr, int len)
{
   Code_Lookup *cl;

   cl = mem_alloc(SZ(Code_Lookup));
   cl->ptr = ptr;
   cl->len = len;

   data_queue_program_lookup(pc, name, &(cl->val));

   code_lookups = eina_list_append(code_lookups, cl);
}
static void
_data_queue_group_lookup(Edje_Part_Collection *pc __UNUSED__, char *name, char *ptr __UNUSED__, int len __UNUSED__)
{
   data_queue_group_lookup(name);
}
static void
_data_queue_image_pc_lookup(Edje_Part_Collection *pc __UNUSED__, char *name, char *ptr, int len)
{
   Code_Lookup *cl;

   cl = mem_alloc(SZ(Code_Lookup));
   cl->ptr = ptr;
   cl->len = len;

   data_queue_image_lookup(name, &(cl->val),  &(cl->set));

   code_lookups = eina_list_append(code_lookups, cl);
}

void
data_process_scripts(void)
{
   Eina_List *l, *l2;

   for (l = codes, l2 = edje_collections; (l) && (l2); l = eina_list_next(l), l2 = eina_list_next(l2))
     {
	Edje_Part_Collection *pc;
	Code *cd;

	cd = eina_list_data_get(l);
	pc = eina_list_data_get(l2);

	if ((cd->shared) && (!cd->is_lua))
	  {
	     data_process_string(pc, "PART",    cd->shared, _data_queue_part_lookup);
	     data_process_string(pc, "PROGRAM", cd->shared, _data_queue_program_lookup);
	     data_process_string(pc, "IMAGE",   cd->shared, _data_queue_image_pc_lookup);
	     data_process_string(pc, "GROUP",   cd->shared, _data_queue_group_lookup);
	  }

	if (cd->programs)
	  {
	     Code_Program *cp;
	     Eina_List *ll;

	     EINA_LIST_FOREACH(cd->programs, ll, cp)
	       {
		  if (cp->script)
		    {
		       data_process_string(pc, "PART",    cp->script, _data_queue_part_lookup);
		       data_process_string(pc, "PROGRAM", cp->script, _data_queue_program_lookup);
		       data_process_string(pc, "IMAGE",   cp->script, _data_queue_image_pc_lookup);
		       data_process_string(pc, "GROUP",   cp->script, _data_queue_group_lookup);
		    }
	       }
	  }
     }
}

void
data_process_script_lookups(void)
{
   Eina_List *l;
   Code_Lookup *cl;

   EINA_LIST_FOREACH(code_lookups, l, cl)
     {
	char buf[12];
	int n;

	/* FIXME !! Handle set in program */
	n = eina_convert_itoa(cl->val, buf);
	if (n > cl->len)
	  {
	     ERR("%s: Error. The unexpected happened. A numeric replacement string was larger than the original!",
		 progname);
	     exit(-1);
	  }
	memset(cl->ptr, ' ', cl->len);
	strncpy(cl->ptr, buf, n);
     }
}
