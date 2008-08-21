/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_cc.h"

typedef struct _Part_Lookup Part_Lookup;
typedef struct _Program_Lookup Program_Lookup;
typedef struct _Group_Lookup Group_Lookup;
typedef struct _String_Lookup Image_Lookup;
typedef struct _String_Lookup Spectrum_Lookup;
typedef struct _Slave_Lookup Slave_Lookup;
typedef struct _Code_Lookup Code_Lookup;

struct _Part_Lookup
{
   Edje_Part_Collection *pc;
   char *name;
   int *dest;
};

struct _Program_Lookup
{
   Edje_Part_Collection *pc;
   char *name;
   int *dest;
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
};

static void data_process_string(Edje_Part_Collection *pc, const char *prefix, char *s, void (*func)(Edje_Part_Collection *pc, char *name, char *ptr, int len));

Edje_File *edje_file = NULL;
Evas_List *edje_collections = NULL;
Evas_List *fonts = NULL;
Evas_List *codes = NULL;
Evas_List *code_lookups = NULL;
Evas_List *aliases = NULL;

static Eet_Data_Descriptor *edd_edje_file = NULL;
static Eet_Data_Descriptor *edd_edje_image_directory = NULL;
static Eet_Data_Descriptor *edd_edje_image_directory_entry = NULL;
static Eet_Data_Descriptor *edd_edje_spectrum_directory = NULL;
static Eet_Data_Descriptor *edd_edje_spectrum_directory_entry = NULL;
static Eet_Data_Descriptor *edd_edje_program = NULL;
static Eet_Data_Descriptor *edd_edje_program_target = NULL;
static Eet_Data_Descriptor *edd_edje_part_collection_directory = NULL;
static Eet_Data_Descriptor *edd_edje_part_collection_directory_entry = NULL;
static Eet_Data_Descriptor *edd_edje_part_collection = NULL;
static Eet_Data_Descriptor *edd_edje_part = NULL;
static Eet_Data_Descriptor *edd_edje_part_description = NULL;
static Eet_Data_Descriptor *edd_edje_part_image_id = NULL;
static Eet_Data_Descriptor *edd_edje_spectrum_color = NULL;

static Evas_List *part_lookups = NULL;
static Evas_List *program_lookups = NULL;
static Evas_List *group_lookups = NULL;
static Evas_List *image_lookups = NULL;
static Evas_List *spectrum_lookups = NULL;
static Evas_List *part_slave_lookups = NULL;
static Evas_List *image_slave_lookups= NULL;
static Evas_List *spectrum_slave_lookups= NULL;

#define ABORT_WRITE(eet_file, file) \
   eet_close(eet_file); \
   unlink(file); \
   exit(-1);

void
data_setup(void)
{
   edd_edje_file = _edje_edd_edje_file;
   edd_edje_image_directory = _edje_edd_edje_image_directory;
   edd_edje_image_directory_entry = _edje_edd_edje_image_directory_entry;
   edd_edje_spectrum_directory = _edje_edd_edje_spectrum_directory;
   edd_edje_spectrum_directory_entry = _edje_edd_edje_spectrum_directory_entry;
   edd_edje_program = _edje_edd_edje_program;
   edd_edje_program_target = _edje_edd_edje_program_target;
   edd_edje_part_collection_directory = _edje_edd_edje_part_collection_directory;
   edd_edje_part_collection_directory_entry = _edje_edd_edje_part_collection_directory_entry;
   edd_edje_part_collection = _edje_edd_edje_part_collection;
   edd_edje_part = _edje_edd_edje_part;
   edd_edje_part_description = _edje_edd_edje_part_description;
   edd_edje_part_image_id = _edje_edd_edje_part_image_id;
   edd_edje_spectrum_color = _edje_edd_edje_spectrum_color;
}

static void
check_image_part_desc (Edje_Part_Collection *pc, Edje_Part *ep,
                       Edje_Part_Description *epd, Eet_File *ef)
{
   Evas_List *l;

   return;
   if (epd->image.id == -1)
     {
	fprintf(stderr, "%s: Error. collection %i: image attributes missing "
	      "for part \"%s\", description \"%s\" %f\n",
	      progname, pc->id, ep->name, epd->state.name, epd->state.value);
	ABORT_WRITE(ef, file_out);
     }

   for (l = epd->image.tween_list; l; l = l->next)
     {
	Edje_Part_Image_Id *iid = l->data;

	if (iid->id == -1)
	  {
	     fprintf(stderr, "%s: Error. collection %i: tween image id missing "
		   "for part \"%s\", description \"%s\" %f\n",
		   progname, pc->id, ep->name, epd->state.name,
		   epd->state.value);
	     ABORT_WRITE(ef, file_out);
	  }
     }
}

static void
check_part (Edje_Part_Collection *pc, Edje_Part *ep, Eet_File *ef)
{
   Edje_Part_Description *epd = ep->default_desc;
   Evas_List *l;

   if (!epd)
     {
	fprintf(stderr, "%s: Error. collection %i: default description missing "
	      "for part \"%s\"\n", progname, pc->id, ep->name);
	ABORT_WRITE(ef, file_out);
     }

   if (ep->type == EDJE_PART_TYPE_IMAGE)
     {
	check_image_part_desc (pc, ep, epd, ef);

	for (l = ep->other_desc; l; l = l->next)
	  check_image_part_desc (pc, ep, l->data, ef);
     }
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
	   {
	      fprintf(stderr, "%s: Error. collection %i: "
		    "target missing in program %s\n",
		    progname, pc->id, ep->name);
	      ABORT_WRITE(ef, file_out);
	   }
	 break;
      default:
	 break;
     }
}

static void
check_spectrum (Edje_Spectrum_Directory_Entry *se, Eet_File *ef)
{
   if (!se->entry)
     fprintf(stderr, "%s: Error. Spectrum missing a name.\n", progname);
   else if (!se->color_list)
     fprintf(stderr, "%s: Error. Spectrum %s is empty. At least one color must be given.", progname, se->entry);
   else
     return;

   ABORT_WRITE(ef, file_out);
}

static int
data_write_header(Eet_File *ef)
{
   int bytes = 0;

   if (edje_file)
     {

	if (edje_file->collection_dir)
	  {
	     /* copy aliases into collection directory */
	     while (aliases)
	       {
		  edje_file->collection_dir->entries = evas_list_append(edje_file->collection_dir->entries, aliases->data);
		  aliases = evas_list_remove_list(aliases, aliases);
	       }
	  }
	bytes = eet_data_write(ef, edd_edje_file, "edje_file", edje_file, 1);
	if (bytes <= 0)
	  {
	     fprintf(stderr, "%s: Error. unable to write \"edje_file\" entry to \"%s\" \n",
		     progname, file_out);
	     ABORT_WRITE(ef, file_out);
	  }
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
   Evas_List *l;;
   int bytes = 0;
   int total_bytes = 0;

   for (l = fonts; l; l = l->next)
     {
	Font *fn;
	void *fdata = NULL;
	int fsize = 0;
	Evas_List *ll;
	FILE *f;

	fn = l->data;
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
		    {
		       fprintf(stderr, "%s: Error. unable to read all of font file \"%s\"\n",
			       progname, fn->file);
		       ABORT_WRITE(ef, file_out);
		    }
		  fsize = pos;
	       }
	     fclose(f);
	  }
	else
	  {
	     for (ll = fnt_dirs; ll; ll = ll->next)
	       {
		  char buf[4096];

		  snprintf(buf, sizeof(buf), "%s/%s", (char *)(ll->data), fn->file);
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
			      {
				 fprintf(stderr, "%s: Error. unable to read all of font file \"%s\"\n",
					 progname, buf);
				 ABORT_WRITE(ef, file_out);
			      }
			    fsize = pos;
			 }
		       fclose(f);
		       if (fdata) break;
		    }
	       }
	  }
	if (!fdata)
	  {
	     fprintf(stderr, "%s: Error. unable to load font part \"%s\" entry to %s \n",
		     progname, fn->file, file_out);
	     ABORT_WRITE(ef, file_out);
	  }
	else
	  {
	     char buf[4096];

	     snprintf(buf, sizeof(buf), "fonts/%s", fn->name);
	     bytes = eet_write(ef, buf, fdata, fsize, 1);
	     if (bytes <= 0)
	       {
		  fprintf(stderr, "%s: Error. unable to write font part \"%s\" as \"%s\" part entry to %s \n",
			  progname, fn->file, buf, file_out);
		  ABORT_WRITE(ef, file_out);
	       }
	     else
	       {
		  *font_num += 1;
		  total_bytes += bytes;
		  *input_bytes += fsize;
		  *input_raw_bytes += fsize;
	       }
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

   return total_bytes;
}

static int
data_write_images(Eet_File *ef, int *image_num, int *input_bytes, int *input_raw_bytes)
{
   Evas_List *l;
   int bytes = 0;
   int total_bytes = 0;

   if ((edje_file) && (edje_file->image_dir))
     {
	Ecore_Evas *ee;
	Evas *evas;

	ecore_init();
	ecore_evas_init();
	ee = ecore_evas_buffer_new(1, 1);
	if (!ee)
	  {
	     fprintf(stderr, "Error. cannot create buffer engine canvas for image load.\n");
	     ABORT_WRITE(ef, file_out);
	  }
	evas = ecore_evas_get(ee);
	for (l = edje_file->image_dir->entries; l; l = l->next)
	  {
	     Edje_Image_Directory_Entry *img;

	     img = l->data;
	     if (img->source_type == EDJE_IMAGE_SOURCE_TYPE_EXTERNAL)
	       {
	       }
	     else
	       {
		  Evas_Object *im;
		  Evas_List *ll;

		  im = NULL;
		  for (ll = img_dirs; ll; ll = ll->next)
		    {
		       char buf[4096];

		       snprintf(buf, sizeof(buf), "%s/%s",
				(char *)(ll->data), img->entry);
		       im = evas_object_image_add(evas);
		       if (im)
			 {
			    evas_object_image_file_set(im, buf, NULL);
			    if (evas_object_image_load_error_get(im) ==
				EVAS_LOAD_ERROR_NONE)
			      {
				 break;
			      }
			    evas_object_del(im);
			    im = NULL;
			 }
		    }
		  if (!im)
		    {
		       im = evas_object_image_add(evas);
		       if (im)
			 {
			    evas_object_image_file_set(im, img->entry, NULL);
			    if (evas_object_image_load_error_get(im) !=
				EVAS_LOAD_ERROR_NONE)
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

			    snprintf(buf, sizeof(buf), "images/%i", img->id);
			    mode = 2;
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
			      {
				 fprintf(stderr, "%s: Error. unable to write image part \"%s\" as \"%s\" part entry to %s\n",
					 progname, img->entry, buf, file_out);
				 ABORT_WRITE(ef, file_out);
			      }
			    else
			      {
				 *image_num += 1;
				 total_bytes += bytes;
			      }
			 }
		       else
			 {
			    fprintf(stderr, "%s: Error. unable to load image for image part \"%s\" as \"%s\" part entry to %s\n",
				    progname, img->entry, buf, file_out);
			    ABORT_WRITE(ef, file_out);
			 }
		       if (verbose)
			 {
			    struct stat st;
			    const char *file = NULL;

			    evas_object_image_file_get(im, &file, NULL);
			    if ((file) && (stat(file, &st) != 0))
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
		       fprintf(stderr, "%s: Error. unable to load image for image \"%s\" part entry to %s. Missing PNG or JPEG loader modules for Evas or file does not exist, or is not readable.\n",
			       progname, img->entry, file_out);
		       ABORT_WRITE(ef, file_out);
		    }
	       }
	  }
	ecore_evas_free(ee);
	ecore_evas_shutdown();
	ecore_shutdown();
     }

   return total_bytes;
}

static void
check_groups_names(Eet_File *ef)
{
   Evas_List *l;

   if (!edje_file->collection_dir)
     return;

   /* check that all groups have names */
   for (l = edje_file->collection_dir->entries; l; l = l->next)
     {
	Edje_Part_Collection_Directory_Entry *de;
	de = l->data;
	if (!de->entry)
	  {
	     fprintf(stderr, "%s: Error. collection %i: name missing.\n",
		     progname, de->id);
	     ABORT_WRITE(ef, file_out);
	  }
     }
}

static void
check_spectra(Eet_File *ef)
{
   Evas_List *l;

   if (!edje_file->spectrum_dir)
     return;

   /* check that all spectra are valid */
   for (l = edje_file->spectrum_dir->entries; l; l = l->next)
     {
	Edje_Spectrum_Directory_Entry *se;
	se = l->data;
	check_spectrum(se, ef);
     }
}

static void
check_groups(Eet_File *ef)
{
   Evas_List *l;

   /* sanity checks for parts and programs */
   for (l = edje_collections; l; l = l->next)
     {
	Edje_Part_Collection *pc;
	Evas_List *ll;

	pc = l->data;
	for (ll = pc->parts; ll; ll = ll->next)
	  check_part(pc, ll->data, ef);
	for (ll = pc->programs; ll; ll = ll->next)
	  check_program(pc, ll->data, ef);
     }
}

static int
data_write_groups(Eet_File *ef, int *collection_num)
{
   Evas_List *l;
   int bytes = 0;
   int total_bytes = 0;

   for (l = edje_collections; l; l = l->next)
     {
	Edje_Part_Collection *pc;
	char buf[4096];

	pc = l->data;
	snprintf(buf, sizeof(buf), "collections/%i", pc->id);
	bytes = eet_data_write(ef, edd_edje_part_collection, buf, pc, 1);
	if (bytes <= 0)
	  {
	     fprintf(stderr, "%s: Error. unable to write \"%s\" part entry to %s \n",
		     progname, buf, file_out);
	     ABORT_WRITE(ef, file_out);
	  }
	else
	  {
	     *collection_num += 1;
	     total_bytes += bytes;
	  }
	if (verbose)
	  {
	     printf("%s: Wrote %9i bytes (%4iKb) for \"%s\" collection entry\n",
		    progname, bytes, (bytes + 512) / 1024, buf);
	  }
     }

   return total_bytes;
}

static void
create_script_file(Eet_File *ef, const char *filename, const Code *cd)
{
   FILE *f = fopen(filename, "wb");
   if (!f)
     {
	fprintf(stderr, "%s: Error. Unable to open temp file \"%s\" for script compilation \n",
		progname, filename);
	ABORT_WRITE(ef, file_out);
     }

   Evas_List *ll;

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
   for (ll = cd->programs; ll; ll = ll->next)
     {
	Code_Program *cp;

	cp = ll->data;
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
	     fprintf(f, "}");
	     ln += cp->l2 - cp->l1 + 1;
	  }
     }

   fclose(f);
}

static void
compile_script_file(Eet_File *ef, const char *source, const char *output,
		    int script_num)
{
   FILE *f;
   char buf[4096];
   int ret;

   snprintf(buf, sizeof(buf),
	    "embryo_cc -i %s/include -o %s %s",
	    e_prefix_data_get(), output, source);
   ret = system(buf);

   /* accept warnings in the embryo code */
   if (ret < 0 || ret > 1)
     {
	fprintf(stderr, "%s: Warning. Compiling script code not clean.\n",
		progname);
	ABORT_WRITE(ef, file_out);
     }

   f = fopen(output, "rb");
   if (!f)
     {
	fprintf(stderr, "%s: Error. Unable to open script object \"%s\" for reading \n",
		progname, output);
	ABORT_WRITE(ef, file_out);
     }

   int size;
   void *data;

   fseek(f, 0, SEEK_END);
   size = ftell(f);
   rewind(f);

   if (size > 0)
     {
	int bt;

	data = malloc(size);
	if (data)
	  {
	     if (fread(data, size, 1, f) != 1)
	       {
		  fprintf(stderr, "%s: Error. Unable to read all of script object \"%s\"\n",
			  progname, output);
		  ABORT_WRITE(ef, file_out);
	       }
	     snprintf(buf, sizeof(buf), "scripts/%i", script_num);
	     bt = eet_write(ef, buf, data, size, 1);
	     free(data);
	  }
     }

   fclose(f);
}

static void
data_write_scripts(Eet_File *ef)
{
   Evas_List *l;
   int i;

#ifdef HAVE_EVIL
   char *tmpdir = evil_tmpdir_get();
#else
   char *tmpdir = "/tmp";
#endif

   for (i = 0, l = codes; l; l = l->next, i++)
     {
	int fd;
	Code *cd = l->data;

	if ((!cd->shared) && (!cd->programs))
	  continue;

	char tmpn[4096];
	snprintf(tmpn, PATH_MAX, "%s/edje_cc.sma-tmp-XXXXXX", tmpdir);
	fd = mkstemp(tmpn);
	if (fd < 0)
	  {
	     fprintf(stderr, "%s: Error. Unable to open temp file \"%s\" for script compilation \n",
		     progname, tmpn);
	     ABORT_WRITE(ef, file_out);
	  }

	create_script_file(ef, tmpn, cd);
	close(fd);

	char tmpo[4096];
	snprintf(tmpo, PATH_MAX, "%s/edje_cc.amx-tmp-XXXXXX", tmpdir);
	fd = mkstemp(tmpo);
	if (fd < 0)
	  {
	     fprintf(stderr, "%s: Error. Unable to open temp file \"%s\" for script compilation \n",
		     progname, tmpn);
	     ABORT_WRITE(ef, file_out);
	  }
	compile_script_file(ef, tmpn, tmpo, i);
	close(fd);

	unlink(tmpn);
	unlink(tmpo);
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
   int font_num = 0;
   int collection_num = 0;

   ef = eet_open(file_out, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	fprintf(stderr, "%s: Error. unable to open \"%s\" for writing output\n",
		progname, file_out);
	exit(-1);
     }

   total_bytes += data_write_header(ef);
   total_bytes += data_write_fonts(ef, &font_num, &input_bytes,
				   &input_raw_bytes);
   total_bytes += data_write_images(ef, &image_num, &input_bytes,
				    &input_raw_bytes);

   check_groups_names(ef);
   check_spectra(ef);
   check_groups(ef);

   total_bytes += data_write_groups(ef, &collection_num);
   data_write_scripts(ef);

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
data_queue_group_lookup(char *name)
{
   Group_Lookup *gl;

   gl = mem_alloc(SZ(Group_Lookup));
   group_lookups = evas_list_append(group_lookups, gl);
   gl->name = mem_strdup(name);
}

void
data_queue_part_lookup(Edje_Part_Collection *pc, char *name, int *dest)
{
   Part_Lookup *pl;

   pl = mem_alloc(SZ(Part_Lookup));
   part_lookups = evas_list_append(part_lookups, pl);
   pl->pc = pc;
   pl->name = mem_strdup(name);
   pl->dest = dest;
}

void
data_queue_program_lookup(Edje_Part_Collection *pc, char *name, int *dest)
{
   Program_Lookup *pl;

   pl = mem_alloc(SZ(Program_Lookup));
   program_lookups = evas_list_append(program_lookups, pl);
   pl->pc = pc;
   pl->name = mem_strdup(name);
   pl->dest = dest;
}

void
data_queue_image_lookup(char *name, int *dest)
{
   Image_Lookup *il;

   il = mem_alloc(SZ(Image_Lookup));
   image_lookups = evas_list_append(image_lookups, il);
   il->name = mem_strdup(name);
   il->dest = dest;
}

void
data_queue_spectrum_lookup(char *name, int *dest)
{
   Spectrum_Lookup *sl;

   sl = mem_alloc(SZ(Spectrum_Lookup));
   spectrum_lookups = evas_list_append(spectrum_lookups, sl);
   sl->name = mem_strdup(name);
   sl->dest = dest;
}

void
data_queue_part_slave_lookup(int *master, int *slave)
{
   Slave_Lookup *sl;

   sl = mem_alloc(SZ(Slave_Lookup));
   part_slave_lookups = evas_list_append(part_slave_lookups, sl);
   sl->master = master;
   sl->slave = slave;
}

void
data_queue_image_slave_lookup(int *master, int *slave)
{
   Slave_Lookup *sl;

   sl = mem_alloc(SZ(Slave_Lookup));
   image_slave_lookups = evas_list_append(image_slave_lookups, sl);
   sl->master = master;
   sl->slave = slave;
}

void
data_queue_spectrum_slave_lookup(int *master, int *slave)
{
   Slave_Lookup *sl;

   sl = mem_alloc(SZ(Slave_Lookup));
   spectrum_slave_lookups = evas_list_append(spectrum_slave_lookups, sl);
   sl->master = master;
   sl->slave = slave;
}

void
handle_slave_lookup(Evas_List *list, int *master, int value)
{
   Evas_List *l;

   for (l = list; l; l = l->next)
   {
      Slave_Lookup *sl = l->data;

      if (sl->master == master)
	 *sl->slave = value;
   }
}

void
data_process_lookups(void)
{
   Evas_List *l;

   while (part_lookups)
     {
	Part_Lookup *pl;

	pl = part_lookups->data;

	for (l = pl->pc->parts; l; l = l->next)
	  {
	     Edje_Part *ep;

	     ep = l->data;
	     if ((ep->name) && (!strcmp(ep->name, pl->name)))
	       {
		  handle_slave_lookup(part_slave_lookups, pl->dest, ep->id);
		  *(pl->dest) = ep->id;
		  break;
	       }
	  }
	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable to find part name %s\n",
		     progname, pl->name);
	     exit(-1);
	  }
	part_lookups = evas_list_remove(part_lookups, pl);
	free(pl->name);
	free(pl);
     }

   while (program_lookups)
     {
	Program_Lookup *pl;

	pl = program_lookups->data;

	for (l = pl->pc->programs; l; l = l->next)
	  {
	     Edje_Program *ep;

	     ep = l->data;
	     if ((ep->name) && (!strcmp(ep->name, pl->name)))
	       {
		  *(pl->dest) = ep->id;
		  break;
	       }
	  }
	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable to find program name %s\n",
		     progname, pl->name);
	     exit(-1);
	  }
	program_lookups = evas_list_remove(program_lookups, pl);
	free(pl->name);
	free(pl);
     }

   while (group_lookups)
     {
        Group_Lookup *gl;

        gl = group_lookups->data;
        for (l = edje_file->collection_dir->entries; l; l = l->next)
          {
             Edje_Part_Collection_Directory_Entry *de;
             de = l->data;
             if (!strcmp(de->entry, gl->name))
               {
                  break;
               }
          }
        if (!l)
          {
             fprintf(stderr, "%s: Error. unable to find group name %s\n",
                     progname, gl->name);
             exit(-1);
          }
        group_lookups = evas_list_remove(group_lookups, gl);
        free(gl->name);
        free(gl);
     }

   while (image_lookups)
     {
	Image_Lookup *il;

	il = image_lookups->data;

	if (!edje_file->image_dir)
	  l = NULL;
	else
	  {
	     for (l = edje_file->image_dir->entries; l; l = l->next)
	       {
		  Edje_Image_Directory_Entry *de;

		  de = l->data;
		  if ((de->entry) && (!strcmp(de->entry, il->name)))
		    {
		       handle_slave_lookup(image_slave_lookups, il->dest, de->id);
		       if (de->source_type == EDJE_IMAGE_SOURCE_TYPE_EXTERNAL)
			 *(il->dest) = -de->id - 1;
		       else
			 *(il->dest) = de->id;
		       break;
		    }
	       }
	  }

	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable to find image name %s\n",
		     progname, il->name);
	     exit(-1);
	  }
	image_lookups = evas_list_remove(image_lookups, il);
	free(il->name);
	free(il);
     }

   while (spectrum_lookups)
     {
	Spectrum_Lookup *il;

	il = spectrum_lookups->data;

	if (!edje_file->spectrum_dir)
	  l = NULL;
	else
	  {
	     for (l = edje_file->spectrum_dir->entries; l; l = l->next)
	       {
		  Edje_Spectrum_Directory_Entry *de;

		  de = l->data;
		  *(il->dest) = 1;
		  if ((de->entry) && (!strcmp(de->entry, il->name)))
		    {
		       handle_slave_lookup(spectrum_slave_lookups, il->dest, de->id);
		       *(il->dest) = de->id;
		       break;
		    }
	       }
	  }

	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable to find spectrum name %s\n",
		     progname, il->name);
	     exit(-1);
	  }
	spectrum_lookups = evas_list_remove(spectrum_lookups, il);
	free(il->name);
	free(il);
     }

   while (part_slave_lookups)
     {
	free(part_slave_lookups->data);
	part_slave_lookups = evas_list_remove_list(part_slave_lookups, part_slave_lookups);
     }

   while (image_slave_lookups)
     {
	free(image_slave_lookups->data);
	image_slave_lookups = evas_list_remove_list(image_slave_lookups, image_slave_lookups);
     }

   while (spectrum_slave_lookups)
     {
	free(spectrum_slave_lookups->data);
	spectrum_slave_lookups = evas_list_remove_list(spectrum_slave_lookups, spectrum_slave_lookups);
     }
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

   code_lookups = evas_list_append(code_lookups, cl);
}
static void
_data_queue_program_lookup(Edje_Part_Collection *pc, char *name, char *ptr, int len)
{
   Code_Lookup *cl;
   cl = mem_alloc(SZ(Code_Lookup));
   cl->ptr = ptr;
   cl->len = len;

   data_queue_program_lookup(pc, name, &(cl->val));

   code_lookups = evas_list_append(code_lookups, cl);
}
static void
_data_queue_group_lookup(Edje_Part_Collection *pc, char *name, char *ptr, int len)
{
   data_queue_group_lookup(name);	
}
static void
_data_queue_image_pc_lookup(Edje_Part_Collection *pc, char *name, char *ptr, int len)
{
   Code_Lookup *cl;
   cl = mem_alloc(SZ(Code_Lookup));
   cl->ptr = ptr;
   cl->len = len;

   data_queue_image_lookup(name, &(cl->val));

   code_lookups = evas_list_append(code_lookups, cl);
}

void
data_process_scripts(void)
{
   Evas_List *l, *l2;

   for (l = codes, l2 = edje_collections; (l) && (l2); l = l->next, l2 = l2->next)
     {
	Code *cd;
	Edje_Part_Collection *pc;

	cd = l->data;
	pc = l2->data;
	if ((cd->shared) || (cd->programs))
	  {
	     Evas_List *ll;

	     if (cd->shared)
	       {
		  data_process_string(pc, "PART",    cd->shared, _data_queue_part_lookup);
		  data_process_string(pc, "PROGRAM", cd->shared, _data_queue_program_lookup);
		  data_process_string(pc, "IMAGE",   cd->shared, _data_queue_image_pc_lookup);
		  data_process_string(pc, "GROUP",   cd->shared, _data_queue_group_lookup);
	       }
	     for (ll = cd->programs; ll; ll = ll->next)
	       {
		  Code_Program *cp;

		  cp = ll->data;
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
   Evas_List *l;

   for (l = code_lookups; l; l = l->next)
     {
	Code_Lookup *cl;
	char buf[256];
	int i, n;

	cl = l->data;
	snprintf(buf, sizeof(buf), "%i", cl->val);
	n = strlen(buf);
	if (n > cl->len)
	  {
	     fprintf(stderr, "%s: Error. The unexpected happened. A numeric replacement string was larger than the original!\n",
		     progname);
	     exit(-1);
	  }
	for (i = 0; i < cl->len; i++) cl->ptr[i] = ' ';
	strncpy(cl->ptr, buf, n);
     }
}
