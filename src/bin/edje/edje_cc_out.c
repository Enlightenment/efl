#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
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
typedef struct _Part_Lookup_Key Part_Lookup_Key;
typedef struct _Program_Lookup Program_Lookup;
typedef struct _Group_Lookup Group_Lookup;
typedef struct _Image_Lookup Image_Lookup;
typedef struct _Slave_Lookup Slave_Lookup;
typedef struct _Code_Lookup Code_Lookup;


struct _External_Lookup
{
   char *name;
};

struct _Part_Lookup_Key
{
   Edje_Part_Collection *pc;
   
   union {
      int *dest;
      struct {
         unsigned char **base;
         int offset;
      } reallocated;
   } mem;

   Eina_Bool stable : 1;
};

struct _Part_Lookup
{
   Part_Lookup_Key key;
   char *name;
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
   Edje_Part *part;
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

typedef struct _Script_Lua_Writer Script_Lua_Writer;

struct _Script_Lua_Writer
{
   char *buf;
   int size;
};

typedef struct _Script_Write Script_Write;
typedef struct _Head_Write Head_Write;
typedef struct _Fonts_Write Fonts_Write;
typedef struct _Image_Write Image_Write;
typedef struct _Sound_Write Sound_Write;
typedef struct _Vibration_Write Vibration_Write;
typedef struct _Group_Write Group_Write;
typedef struct _License_Write License_Write;

struct _Script_Write
{
   Eet_File *ef;
   Code *cd;
   int i;
   Ecore_Exe *exe;
   int tmpn_fd, tmpo_fd;
   char tmpn[PATH_MAX];
   char tmpo[PATH_MAX];
   char *errstr;
};

struct _Head_Write
{
   Eet_File *ef;
   char *errstr;
};

struct _Fonts_Write
{
   Eet_File *ef;
   Edje_Font *fn;
   char *errstr;
};

struct _Image_Write
{
   Eet_File *ef;
   Edje_Image_Directory_Entry *img;
   Evas_Object *im;
   int w, h;
   int alpha;
   unsigned int *data;
   char *path;
   char *errstr;
};

struct _Sound_Write
{
   Eet_File *ef;
   Edje_Sound_Sample *sample;
   int i;
};

struct _Vibration_Write
{
   Eet_File *ef;
   Edje_Vibration_Sample *sample;
   int i;
};

struct _Group_Write
{
   Eet_File *ef;
   Edje_Part_Collection *pc;
   char *errstr;
};

struct _License_Write
{
   Eet_File *ef;
   const char *file;
   Eina_Bool master;
};

struct _Image_Unused_Ids
{
   int old_id;
   int new_id;
};

typedef struct _Image_Unused_Ids Image_Unused_Ids;

static int pending_threads = 0;

static void data_process_string(Edje_Part_Collection *pc, const char *prefix, char *s, void (*func)(Edje_Part_Collection *pc, char *name, char* ptr, int len));

Edje_File *edje_file = NULL;
Eina_List *edje_collections = NULL;
Eina_Hash *edje_collections_lookup = NULL;
Eina_List *externals = NULL;
Eina_List *fonts = NULL;
Eina_List *codes = NULL;
Eina_List *code_lookups = NULL;
Eina_List *aliases = NULL;

static Eet_Data_Descriptor *edd_edje_file = NULL;
static Eet_Data_Descriptor *edd_edje_part_collection = NULL;

static Eina_List *program_lookups = NULL;
static Eina_List *group_lookups = NULL;
static Eina_List *face_group_lookups = NULL;
static Eina_List *image_lookups = NULL;
static Eina_List *part_slave_lookups = NULL;
static Eina_List *image_slave_lookups= NULL;

static Eina_Hash *part_dest_lookup = NULL;
static Eina_Hash *part_pc_dest_lookup = NULL;

void
error_and_abort(Eet_File *ef EINA_UNUSED, const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   eina_log_vprint(_edje_cc_log_dom, EINA_LOG_LEVEL_CRITICAL,
                   "unknown", "unknown", 0, fmt, ap);
   va_end(ap);
   unlink(file_out);
   if (watchfile) unlink(watchfile);
   exit(-1);
}

static unsigned int
_part_lookup_key_length(const void *key EINA_UNUSED)
{
   return sizeof (Part_Lookup_Key);
}

static int
_part_lookup_key_pc_cmp(const void *key1, int key1_length EINA_UNUSED,
                        const void *key2, int key2_length EINA_UNUSED)
{
   const Part_Lookup_Key *a = key1;
   const Part_Lookup_Key *b = key2;
   uintptr_t delta;

   delta = a->pc - b->pc;
   if (delta) return delta;

   if (a->stable) return a->mem.dest - b->mem.dest;

   delta = a->mem.reallocated.base - b->mem.reallocated.base;
   if (delta) return delta;
   return a->mem.reallocated.offset - b->mem.reallocated.offset;
}

static int
_part_lookup_key_pc_hash(const void *key, int key_length EINA_UNUSED)
{
   const Part_Lookup_Key *a = key;

   if (a->stable)
     {
#ifdef EFL64
        return eina_hash_int64((unsigned long long int *) &a->pc, sizeof (void*)) ^
          eina_hash_int64((unsigned long long int *) &a->mem.dest, sizeof (void*));
#else
        return eina_hash_int32((uintptr_t *) &a->pc, sizeof (void*)) ^
          eina_hash_int32((uintptr_t *) &a->mem.dest, sizeof (void*));
#endif
     }
   else
     {
#ifdef EFL64
        return eina_hash_int64((unsigned long long int *) &a->pc, sizeof (void*)) ^
          eina_hash_int64((unsigned long long int *) &a->mem.reallocated.base, sizeof (void*)) ^
          eina_hash_int32((unsigned int *) &a->mem.reallocated.offset, sizeof (int));
#else
        return eina_hash_int32((uintptr_t *) &a->pc, sizeof (void*)) ^
          eina_hash_int32((uintptr_t *) &a->mem.reallocated.base, sizeof (void*)) ^
          eina_hash_int32((unsigned int *) &a->mem.reallocated.offset, sizeof (int));
#endif
     }
}

static int
_part_lookup_key_cmp(const void *key1, int key1_length EINA_UNUSED,
                     const void *key2, int key2_length EINA_UNUSED)
{
   const Part_Lookup_Key *a = key1;
   const Part_Lookup_Key *b = key2;
   uintptr_t delta;

   if (a->stable) return a->mem.dest - b->mem.dest;

   delta = a->mem.reallocated.base - b->mem.reallocated.base;
   if (delta) return delta;
   return a->mem.reallocated.offset - b->mem.reallocated.offset;
}

static int
_part_lookup_key_hash(const void *key, int key_length EINA_UNUSED)
{
   const Part_Lookup_Key *a = key;

   if (a->stable)
     {
#ifdef EFL64
        return eina_hash_int64((unsigned long long int *) &a->mem.dest, sizeof (void*));
#else
        return eina_hash_int32((uintptr_t *) &a->mem.dest, sizeof (void*));
#endif
     }
   else
     {
#ifdef EFL64
        return eina_hash_int64((unsigned long long int *) &a->mem.reallocated.base, sizeof (void*)) ^
          eina_hash_int32((unsigned int *) &a->mem.reallocated.offset, sizeof (int));
#else
        return eina_hash_int32((uintptr_t *) &a->mem.reallocated.base, sizeof (void*)) ^
          eina_hash_int32((unsigned int *) &a->mem.reallocated.offset, sizeof (int));
#endif
     }
}

static void
data_part_lookup_free(Part_Lookup *pl)
{
   free(pl->name);
   free(pl);
}

void
data_setup(void)
{
   edd_edje_file = _edje_edd_edje_file;
   edd_edje_part_collection = _edje_edd_edje_part_collection;

   part_dest_lookup = eina_hash_new(EINA_KEY_LENGTH(_part_lookup_key_length),
                                    EINA_KEY_CMP(_part_lookup_key_cmp),
                                    EINA_KEY_HASH(_part_lookup_key_hash),
                                    EINA_FREE_CB(eina_list_free),
                                    8);
   part_pc_dest_lookup = eina_hash_new(EINA_KEY_LENGTH(_part_lookup_key_length),
                                       EINA_KEY_CMP(_part_lookup_key_pc_cmp),
                                       EINA_KEY_HASH(_part_lookup_key_pc_hash),
                                       EINA_FREE_CB(data_part_lookup_free),
                                       8);
}

static void
check_image_part_desc(Edje_Part_Collection *pc, Edje_Part *ep,
                      Edje_Part_Description_Image *epd, Eet_File *ef)
{
   unsigned int i;

   if (epd->image.id == -1 && epd->common.visible)
     WRN("Collection %s(%i): image attributes missing for "
	 "part \"%s\", description \"%s\" %f",
	 pc->part, pc->id, ep->name, epd->common.state.name, epd->common.state.value);

   for (i = 0; i < epd->image.tweens_count; ++i)
     {
	if (epd->image.tweens[i]->id == -1)
	  error_and_abort(ef, "Collection %i: tween image id missing for "
			  "part \"%s\", description \"%s\" %f",
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
			  "of type GROUP in part \"%s\"",
			  pc->id, ep->name);
	if (ep->type == EDJE_PART_TYPE_TABLE && (ep->items[i]->col < 0 || ep->items[i]->row < 0))
	  error_and_abort(ef, "Collection %i: missing col/row on packed item "
			  "for part \"%s\" of type TABLE",
			  pc->id, ep->name);
     }
}

static void
check_nameless_state(Edje_Part_Collection *pc, Edje_Part *ep, Edje_Part_Description_Common *ed, Eet_File *ef)
{
   Edje_Part_Collection_Directory_Entry *de;

   if (ed->state.name) return;

   de = eina_hash_find(edje_collections_lookup, &pc->id);
   error_and_abort(ef, "Group '%s': part \"%s\" has description with missing state",
                   de->entry, ep->name);
}

static void
check_part(Edje_Part_Collection *pc, Edje_Part *ep, Eet_File *ef)
{
   unsigned int i;
   /* FIXME: check image set and sort them. */
   if (!ep->default_desc)
     error_and_abort(ef, "Collection %i: default description missing "
		     "for part \"%s\"", pc->id, ep->name);

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

   /* FIXME: When mask are supported remove this check */
   if (ep->clip_to_id != -1 &&
       pc->parts[ep->clip_to_id]->type != EDJE_PART_TYPE_RECTANGLE)
     error_and_abort(ef, "Collection %i: clip_to point to a non RECT part '%s' !",
                     pc->id, pc->parts[ep->clip_to_id]->name);
}

static void
check_program(Edje_Part_Collection *pc, Edje_Program *ep, Eet_File *ef)
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
			   "\"%s\"", pc->id, ep->name);
	 break;
      default:
	 break;
     }
}

static void
data_thread_head(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Head_Write *hw = data;
   int bytes = 0;
   char buf[PATH_MAX];

   if (edje_file)
     {
	if (edje_file->collection)
	  {
	     Edje_Part_Collection_Directory_Entry *ce;

	     EINA_LIST_FREE(aliases, ce)
	       {
		  Edje_Part_Collection_Directory_Entry *sce;

		  if (!ce->entry)
                    {
                       snprintf(buf, sizeof(buf),
                                "Collection %i: name missing.", ce->id);
                       hw->errstr = strdup(buf);
                       return;
                    }

		  sce = eina_hash_find(edje_collections_lookup, &ce->id);
		  if (sce)
                    {
                       memcpy(&ce->count, &sce->count, sizeof (ce->count));
                    }
		  else
                    {
                       snprintf(buf, sizeof(buf),
                                "Collection %s (%i) can't find an correct alias.",
                                ce->entry, ce->id);
                       hw->errstr = strdup(buf);
                       return;
                    }

		  eina_hash_direct_add(edje_file->collection, ce->entry, ce);
	       }
	  }
	bytes = eet_data_write(hw->ef, edd_edje_file, "edje/file", edje_file,
                               compress_mode);
	if (bytes <= 0)
          {
             snprintf(buf, sizeof(buf),
                      "Unable to write \"edje_file\" entry to \"%s\"",
                      file_out);
             hw->errstr = strdup(buf);
             return;
          }
     }

   INF("Wrote %9i bytes (%4iKb) for \"edje_file\" header",
       bytes, (bytes + 512) / 1024);
}

static void
data_thread_head_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Head_Write *hw = data;

   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
   if (hw->errstr)
     {
        error_and_abort(hw->ef, hw->errstr);
        free(hw->errstr);
     }
   free(hw);
}

static void
data_write_header(Eet_File *ef)
{
   Head_Write  *hw;

   hw = calloc(1, sizeof(Head_Write));
   hw->ef = ef;
   pending_threads++;
   if (threads)
     ecore_thread_run(data_thread_head, data_thread_head_end, NULL, hw);
   else
     {
        data_thread_head(hw, NULL);
        data_thread_head_end(hw, NULL);
     }
}

static void
data_thread_fonts(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Fonts_Write *fc = data;
   Eina_List *ll;
   Eina_File *f = NULL;
   void *m = NULL;
   int bytes = 0;
   char buf[PATH_MAX];
   char buf2[PATH_MAX];

   f = eina_file_open(fc->fn->file, 0);
   if (f)
     {
       using_file(fc->fn->file, 'F');
        m = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
     }
   else
     {
        char *dat;

        EINA_LIST_FOREACH(fnt_dirs, ll, dat)
          {
             snprintf(buf, sizeof(buf), "%s/%s", dat, fc->fn->file);
             f = eina_file_open(buf, 0);
             if (f)
               {
		 using_file(buf, 'F');
                  m = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
                  if (m) break;
                  eina_file_close(f);
                  f = NULL;
               }
          }
     }
   if (!m)
     {
        if (f) eina_file_close(f);
        snprintf(buf, sizeof(buf),
                 "Unable to load font part \"%s\" entry to %s",
                 fc->fn->file, file_out);
        fc->errstr = strdup(buf);
        return;
     }

   snprintf(buf, sizeof(buf), "edje/fonts/%s", fc->fn->name);
   bytes = eet_write(fc->ef, buf, m, eina_file_size_get(f), compress_mode);

   if ((bytes <= 0) || eina_file_map_faulted(f, m))
     {
        eina_file_map_free(f, m);
        eina_file_close(f);
        snprintf(buf2, sizeof(buf2),
                 "Unable to write font part \"%s\" as \"%s\" "
                 "part entry to %s", fc->fn->file, buf, file_out);
        fc->errstr = strdup(buf2);
        return;
     }

   INF("Wrote %9i bytes (%4iKb) for \"%s\" font entry \"%s\" compress: [real: %2.1f%%]",
       bytes, (bytes + 512) / 1024, buf, fc->fn->file,
       100 - (100 * (double)bytes) / ((double)(eina_file_size_get(f)))
       );
   eina_file_map_free(f, m);
   eina_file_close(f);
}

static void
data_thread_fonts_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Fonts_Write *fc = data;
   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
   if (fc->errstr)
     {
        error_and_abort(fc->ef, fc->errstr);
        free(fc->errstr);
     }
   free(fc);
}

static void
data_write_fonts(Eet_File *ef, int *font_num)
{
   Eina_Iterator *it;
   Edje_Font *fn;

   if (!edje_file->fonts) return;

   it = eina_hash_iterator_data_new(edje_file->fonts);
   EINA_ITERATOR_FOREACH(it, fn)
     {
        Fonts_Write *fc;

        fc = calloc(1, sizeof(Fonts_Write));
        if (!fc) continue;
        fc->ef = ef;
        fc->fn = fn;
        pending_threads++;
        if (threads)
          ecore_thread_run(data_thread_fonts, data_thread_fonts_end, NULL, fc);
        else
          {
             data_thread_fonts(fc, NULL);
             data_thread_fonts_end(fc, NULL);
          }
        *font_num += 1;
     }
   eina_iterator_free(it);
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
	  "bmp",
	  "ico",
	  "tga",
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
     (ef, "Unable to load image \"%s\" used by file \"%s\": %s.%s",
      file, file_out, errmsg, hint);
}

static void
data_thread_image(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Image_Write *iw = data;
   char buf[PATH_MAX], buf2[PATH_MAX];
   unsigned int *start, *end;
   Eina_Bool opaque = EINA_TRUE;
   int bytes = 0;

   if ((iw->data) && (iw->w > 0) && (iw->h > 0))
     {
        int mode, qual;

        snprintf(buf, sizeof(buf), "edje/images/%i", iw->img->id);
        qual = 80;
        if ((iw->img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT) &&
            (iw->img->source_param == 0))
          mode = 0; /* RAW */
        else if ((iw->img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT) &&
                 (iw->img->source_param == 1))
          mode = 1; /* COMPRESS */
        else
          mode = 2; /* LOSSY */
        if ((mode == 0) && (no_raw))
          {
             mode = 1; /* promote compression */
             iw->img->source_param = 95;
          }
        if ((mode == 2) && (no_lossy)) mode = 1; /* demote compression */
        if ((mode == 1) && (no_comp))
          {
             if (no_lossy) mode = 0; /* demote compression */
             else if (no_raw)
               {
                  iw->img->source_param = 90;
                  mode = 2; /* no choice. lossy */
               }
          }
        if (mode == 2)
          {
             qual = iw->img->source_param;
             if (qual < min_quality) qual = min_quality;
             if (qual > max_quality) qual = max_quality;
          }
        if (iw->alpha)
          {
             start = (unsigned int *) iw->data;
             end = start + (iw->w * iw->h);
             while (start < end)
               {
                  if ((*start & 0xff000000) != 0xff000000)
                    {
                       opaque = EINA_FALSE;
                       break;
                    }
                  start++;
               }
             if (opaque) iw->alpha = 0;
          }
        if (mode == 0)
          bytes = eet_data_image_write(iw->ef, buf,
                                       iw->data, iw->w, iw->h,
                                       iw->alpha,
                                       0, 0, 0);
        else if (mode == 1)
          bytes = eet_data_image_write(iw->ef, buf,
                                       iw->data, iw->w, iw->h,
                                       iw->alpha,
                                       compress_mode,
                                       0, 0);
        else if (mode == 2)
          bytes = eet_data_image_write(iw->ef, buf,
                                       iw->data, iw->w, iw->h,
                                       iw->alpha,
                                       0, qual, 1);
        if (bytes <= 0)
          {
             snprintf(buf2, sizeof(buf2),
                      "Unable to write image part "
                      "\"%s\" as \"%s\" part entry to "
                      "%s", iw->img->entry, buf, file_out);
             iw->errstr = strdup(buf2);
             return;
          }
     }
   else
     {
        snprintf(buf, sizeof(buf), "edje/images/%i", iw->img->id);
        snprintf(buf2, sizeof(buf2),
                 "Unable to load image part "
                 "\"%s\" as \"%s\" part entry to "
                 "%s", iw->img->entry, buf, file_out);
        iw->errstr = strdup(buf2);
        return;
     }

   if (eina_log_domain_level_check(_edje_cc_log_dom, EINA_LOG_LEVEL_INFO))
     {
        struct stat st;

        if (!iw->path || (stat(iw->path, &st))) st.st_size = 0;
        if (st.st_size > 0)
          {
             INF("Wrote %9i bytes (%4iKb) for \"%s\" image entry \"%s\" compress: [raw: %2.1f%%] [real: %2.1f%%]",
                 bytes, (bytes + 512) / 1024, buf, iw->img->entry,
                 100 - (100 * (double)bytes) / ((double)(iw->w * iw->h * 4)),
                 100 - (100 * (double)bytes) / ((double)(st.st_size))
                );
          }
     }
}

static void
data_thread_image_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Image_Write *iw = data;

   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
   if (iw->errstr)
     {
        error_and_abort(iw->ef, iw->errstr);
        free(iw->errstr);
     }
   if (iw->path) free(iw->path);
   evas_object_del(iw->im);
   free(iw);
}

static void
data_image_preload_done(void *data, Evas *e EINA_UNUSED, Evas_Object *o, void *event_info EINA_UNUSED)
{
   Image_Write *iw = data;

   evas_object_image_size_get(o, &iw->w, &iw->h);
   iw->alpha = evas_object_image_alpha_get(o);
   iw->data = evas_object_image_data_get(o, 0);
   if (threads)
     ecore_thread_run(data_thread_image, data_thread_image_end, NULL, iw);
   else
     {
        data_thread_image(iw, NULL);
        data_thread_image_end(iw, NULL);
     }
}

static void
data_write_images(Eet_File *ef, int *image_num)
{
   int i;
   Ecore_Evas *ee;
   Evas *evas;

   if (!((edje_file) && (edje_file->image_dir))) return;

   ecore_evas_init();
   ee = ecore_evas_buffer_new(1, 1);
   if (!ee)
     error_and_abort(ef, "Cannot create buffer engine canvas for image load.");
   evas = ecore_evas_get(ee);

   for (i = 0; i < (int)edje_file->image_dir->entries_count; i++)
     {
        Edje_Image_Directory_Entry *img;

        img = &edje_file->image_dir->entries[i];
        if ((img->source_type == EDJE_IMAGE_SOURCE_TYPE_EXTERNAL) ||
            (img->entry == NULL))
          {
          }
        else
          {
             Evas_Object *im;
             Eina_List *ll;
             char *s;
             int load_err = EVAS_LOAD_ERROR_NONE;
             Image_Write *iw;

             iw = calloc(1, sizeof(Image_Write));
             iw->ef = ef;
             iw->img = img;
             iw->im = im = evas_object_image_add(evas);
             if (threads)
               evas_object_event_callback_add(im,
                                              EVAS_CALLBACK_IMAGE_PRELOADED,
                                              data_image_preload_done,
                                              iw);
             EINA_LIST_FOREACH(img_dirs, ll, s)
               {
                  char buf[PATH_MAX];

                  snprintf(buf, sizeof(buf), "%s/%s", s, img->entry);
                  evas_object_image_file_set(im, buf, NULL);
                  load_err = evas_object_image_load_error_get(im);
                  if (load_err == EVAS_LOAD_ERROR_NONE)
                    {
                       *image_num += 1;
                       iw->path = strdup(buf);
                       pending_threads++;
                       if (threads)
                         evas_object_image_preload(im, 0);
                       using_file(buf, 'I');
                       if (!threads)
                         data_image_preload_done(iw, evas, im, NULL);
                       break;
                    }
               }
             if (load_err != EVAS_LOAD_ERROR_NONE)
               {
                  evas_object_image_file_set(im, img->entry, NULL);
                  load_err = evas_object_image_load_error_get(im);
                  if (load_err == EVAS_LOAD_ERROR_NONE)
                    {
                       *image_num += 1;
                       iw->path = strdup(img->entry);
                       pending_threads++;
                       if (threads)
                         evas_object_image_preload(im, 0);
                       using_file(img->entry, 'I');
                       if (!threads)
                         data_image_preload_done(iw, evas, im, NULL);
                    }
                  else
                    {
                       error_and_abort_image_load_error
                         (ef, img->entry, load_err);
                       exit(1); // ensure static analysis tools know we exit
                    }
               }
	  }
     }
}

static void
data_thread_sounds(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Sound_Write *sw = data;
   Eina_List *ll;
#ifdef HAVE_LIBSNDFILE
   Edje_Sound_Encode *enc_info;
#endif
   char *dir_path = NULL;
   char snd_path[PATH_MAX];
   char sndid_str[15];
   Eina_File *f = NULL;
   void *m = NULL;
   int bytes = 0;

   // Search the Sound file in all the -sd ( sound directory )
   EINA_LIST_FOREACH(snd_dirs, ll, dir_path)
     {
        snprintf((char *)snd_path, sizeof(snd_path), "%s/%s", dir_path,
                 sw->sample->snd_src);
        f = eina_file_open(snd_path, 0);
        if (f) break;
     }
   if (!f)
     {
        snprintf((char *)snd_path, sizeof(snd_path), "%s",
                 sw->sample->snd_src);
        f = eina_file_open(snd_path, 0);
     }
#ifdef HAVE_LIBSNDFILE
   if (f) eina_file_close(f);
   enc_info = _edje_multisense_encode(snd_path, sw->sample,
                                      sw->sample->quality);
   f = eina_file_open(enc_info->file, 0);
   if (f) using_file(enc_info->file, 'S');
#else
   if (f) using_file(snd_path, 'S');
#endif
   if (!f)
     {
        ERR("Unable to load sound data of: %s", sw->sample->name);
        exit(-1);
     }

   snprintf(sndid_str, sizeof(sndid_str), "edje/sounds/%i", sw->sample->id);
   m = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (m)
     {
        bytes = eet_write(sw->ef, sndid_str, m, eina_file_size_get(f),
                          EET_COMPRESSION_NONE);
        if (eina_file_map_faulted(f, m))
          {
             ERR("File access error when reading '%s'",
                 eina_file_filename_get(f));
             exit(-1);
          }
        eina_file_map_free(f, m);
     }
   eina_file_close(f);

#ifdef HAVE_LIBSNDFILE
   //If encoded temporary file, delete it.
   if (enc_info->encoded) unlink(enc_info->file);
#endif
#ifdef HAVE_LIBSNDFILE
   INF("Wrote %9i bytes (%4iKb) for \"%s\" %s sound entry \"%s\"",
       bytes, (bytes + 512) / 1024,
       sndid_str, enc_info->comp_type, sw->sample->name);
#else
   INF("Wrote %9i bytes (%4iKb) for \"%s\" %s sound entry \"%s\"",
       bytes, (bytes + 512) / 1024,
       sndid_str, "RAW PCM", sw->sample->name);
#endif

#ifdef HAVE_LIBSNDFILE
   if ((enc_info->file) && (!enc_info->encoded))
     eina_stringshare_del(enc_info->file);
   if (enc_info) free(enc_info);
   enc_info = NULL;
#endif
}

static void
data_thread_sounds_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Sound_Write *sw = data;
   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
   free(sw);
}

static void
data_write_sounds(Eet_File *ef, int *sound_num)
{
   if ((edje_file) && (edje_file->sound_dir))
     {
        int i;

        for (i = 0; i < (int)edje_file->sound_dir->samples_count; i++)
          {
             Sound_Write *sw;

             sw = calloc(1, sizeof(Sound_Write));
             if (!sw) continue;
             sw->ef = ef;
             sw->sample = &edje_file->sound_dir->samples[i];
             sw->i = i;
             *sound_num += 1;
             pending_threads++;
             if (threads)
               ecore_thread_run(data_thread_sounds, data_thread_sounds_end, NULL, sw);
             else
               {
                  data_thread_sounds(sw, NULL);
                  data_thread_sounds_end(sw, NULL);
               }
          }
     }
}

static void
data_thread_vibrations(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Vibration_Write *vw = data;
   Eina_List *ll;
   char *dir_path = NULL;
   char path[PATH_MAX];
   char id_str[30];
   Eina_File *f = NULL;
   void *m = NULL;
   int bytes = 0;

   EINA_LIST_FOREACH(vibration_dirs, ll, dir_path)
     {
        snprintf((char *)path, sizeof(path), "%s/%s", dir_path,
                 vw->sample->src);
        f = eina_file_open(path, 0);
        if (f) break;
     }
   if (!f)
     {
        snprintf((char *)path, sizeof(path), "%s",
                 vw->sample->src);
        f = eina_file_open(path, 0);
     }
   if (f) using_file(path, 'S');
   if (!f)
     {
        ERR("Unable to load vibration data of: %s", vw->sample->src);
        exit(-1);
     }

   snprintf(id_str, sizeof(id_str), "edje/vibrations/%i", vw->sample->id);
   m = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (m)
     {
        bytes = eet_write(vw->ef, id_str, m, eina_file_size_get(f),
                          EET_COMPRESSION_NONE);
        if (eina_file_map_faulted(f, m))
          {
             ERR("File access error when reading '%s'",
                 eina_file_filename_get(f));
             exit(-1);
          }
        eina_file_map_free(f, m);
     }
   eina_file_close(f);

   INF("Wrote %9i bytes (%4iKb) for \"%s\" %s vibration entry \"%s\"",
       bytes, (bytes + 512) / 1024,
       id_str, "RAW", vw->sample->name);
}

static void
data_thread_vibrations_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Vibration_Write *sw = data;
   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
   free(sw);
}

static void
data_write_vibrations(Eet_File *ef, int *num)
{
   if ((edje_file) && (edje_file->vibration_dir))
     {
        int i;

        for (i = 0; i < (int)edje_file->vibration_dir->samples_count; i++)
          {
             Vibration_Write *vw;

             vw = calloc(1, sizeof(Vibration_Write));
             if (!vw) continue;
             vw->ef = ef;
             vw->sample = &edje_file->vibration_dir->samples[i];
             vw->i = i;
             *num += 1;
             pending_threads++;
             if (threads)
               ecore_thread_run(data_thread_vibrations, data_thread_vibrations_end, NULL, vw);
             else
               {
                  data_thread_vibrations(vw, NULL);
                  data_thread_vibrations_end(vw, NULL);
               }
          }
     }
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

static void
data_thread_group(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Group_Write *gw = data;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "edje/collections/%i", gw->pc->id);
   eet_data_write(gw->ef, edd_edje_part_collection, buf, gw->pc,
                  compress_mode);
   return;
}

static void
data_thread_group_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Group_Write *gw = data;
   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
   if (gw->errstr)
     {
        error_and_abort(gw->ef, gw->errstr);
        free(gw->errstr);
     }
   free(gw);
}

static void
data_write_groups(Eet_File *ef, int *collection_num)
{
   Eina_List *l;
   Edje_Part_Collection *pc;

   EINA_LIST_FOREACH(edje_collections, l, pc)
     {
        Group_Write *gw;

        gw = calloc(1, sizeof(Group_Write));
        if (!gw)
          {
             error_and_abort(ef, "Cannot allocate memory for group writer");
             return;
          }
        gw->ef = ef;
        gw->pc = pc;
        pending_threads++;
        if (threads)
          ecore_thread_run(data_thread_group, data_thread_group_end, NULL, gw);
        else
          {
             data_thread_group(gw, NULL);
             data_thread_group_end(gw, NULL);
          }
        *collection_num += 1;
     }
}

static void
create_script_file(Eet_File *ef, const char *filename, const Code *cd, int fd)
{
   FILE *f = fdopen(fd, "wb");
   if (!f)
     error_and_abort(ef, "Unable to open temp file \"%s\" for script "
		     "compilation.", filename);

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
data_thread_script(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Script_Write *sc = data;
   FILE *f;
   int size;
   char buf[PATH_MAX];

   f = fdopen(sc->tmpo_fd, "rb");
   if (!f)
     {
        snprintf(buf, sizeof(buf),
                 "Unable to open script object \"%s\" for reading.",
                 sc->tmpo);
        sc->errstr = strdup(buf);
        return;
     }

   fseek(f, 0, SEEK_END);
   size = ftell(f);
   rewind(f);

   if (size > 0)
     {
	void *dat = malloc(size);

	if (dat)
	  {
	     if (fread(dat, size, 1, f) != 1)
               {
                  snprintf(buf, sizeof(buf),
                           "Unable to read all of script object \"%s\"",
                           sc->tmpo);
                  sc->errstr = strdup(buf);
                  free(dat);
                  fclose(f);
                  return;
               }
	     snprintf(buf, sizeof(buf), "edje/scripts/embryo/compiled/%i",
                      sc->i);
	     eet_write(sc->ef, buf, dat, size, compress_mode);
	     free(dat);
	  }
        else
          {
             snprintf(buf, sizeof(buf),
                      "Alloc failed for %lu bytes", (unsigned long)size);
             sc->errstr = strdup(buf);
             fclose(f);
             return;
          }
     }
   fclose(f);

   if (!no_save)
     {
        Eina_List *ll;
        Code_Program *cp;

        if (sc->cd->original)
          {
             snprintf(buf, PATH_MAX, "edje/scripts/embryo/source/%i", sc->i);
             eet_write(sc->ef, buf, sc->cd->original,
                       strlen(sc->cd->original) + 1, compress_mode);
          }
        EINA_LIST_FOREACH(sc->cd->programs, ll, cp)
          {
             if (!cp->original) continue;
             snprintf(buf, PATH_MAX, "edje/scripts/embryo/source/%i/%i",
                      sc->i, cp->id);
             eet_write(sc->ef, buf, cp->original,
                       strlen(cp->original) + 1, compress_mode);
          }
     }

   unlink(sc->tmpn);
   unlink(sc->tmpo);
   close(sc->tmpn_fd);
   close(sc->tmpo_fd);
}

static void
data_thread_script_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Script_Write *sc = data;
   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
   if (sc->errstr)
     {
        error_and_abort(sc->ef, sc->errstr);
        free(sc->errstr);
     }
   free(sc);
}

static Eina_Bool
data_scripts_exe_del_cb(void *data EINA_UNUSED, int evtype EINA_UNUSED, void *evinfo)
{
   Script_Write *sc = data;
   Ecore_Exe_Event_Del *ev = evinfo;

   if (!ev->exe) return ECORE_CALLBACK_RENEW;
   if (ecore_exe_data_get(ev->exe) != sc) return ECORE_CALLBACK_RENEW;
   if (ev->exit_code != 0)
     {
        error_and_abort(sc->ef, "Compiling script code not clean.");
        return ECORE_CALLBACK_CANCEL;
     }
   if (threads)
     {
        ecore_thread_run(data_thread_script, data_thread_script_end, NULL, sc);
     }
   else
     {
        data_thread_script(sc, NULL);
        data_thread_script_end(sc, NULL);
     }
   if (pending_threads <= 0) ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

static void
data_write_scripts(Eet_File *ef)
{
   Eina_List *l;
   char embryo_cc_path[PATH_MAX] = "";
   char inc_path[PATH_MAX] = "";
   int i;

   if (!tmp_dir)
#ifdef HAVE_EVIL
     tmp_dir = (char *)evil_tmpdir_get();
#else
     tmp_dir = "/tmp";
#endif

#ifdef _WIN32
# define BIN_EXT ".exe"
#else
# define BIN_EXT
#endif
     if (getenv("EFL_RUN_IN_TREE"))
       {
          snprintf(embryo_cc_path, sizeof(embryo_cc_path),
                   "%s/src/bin/embryo/embryo_cc" BIN_EXT,
                   PACKAGE_BUILD_DIR);
          snprintf(inc_path, sizeof(inc_path),
                   "%s/data/edje/include", PACKAGE_SRC_DIR);
          if (!ecore_file_exists(embryo_cc_path))
            embryo_cc_path[0] = '\0';
       }

     if (embryo_cc_path[0] == '\0')
       {
          snprintf(embryo_cc_path, sizeof(embryo_cc_path),
                   "%s/embryo_cc" BIN_EXT,
                   eina_prefix_bin_get(pfx));
          snprintf(inc_path, sizeof(inc_path),
                   "%s/include",
                   eina_prefix_data_get(pfx));
       }
#undef BIN_EXT

   for (i = 0, l = codes; l; l = eina_list_next(l), i++)
     {
	Code *cd = eina_list_data_get(l);
        Script_Write *sc;
        char buf[PATH_MAX];

	if (cd->is_lua)
	  continue;
	if ((!cd->shared) && (!cd->programs))
	  continue;
        sc = calloc(1, sizeof(Script_Write));
        sc->ef = ef;
        sc->cd = cd;
        sc->i = i;
        snprintf(sc->tmpn, PATH_MAX, "%s/edje_cc.sma-tmp-XXXXXX", tmp_dir);
        sc->tmpn_fd = mkstemp(sc->tmpn);
        if (sc->tmpn_fd < 0)
          error_and_abort(ef, "Unable to open temp file \"%s\" for script "
                          "compilation.", sc->tmpn);
        snprintf(sc->tmpo, PATH_MAX, "%s/edje_cc.amx-tmp-XXXXXX", tmp_dir);
        sc->tmpo_fd = mkstemp(sc->tmpo);
        if (sc->tmpo_fd < 0)
          {
             unlink(sc->tmpn);
             error_and_abort(ef, "Unable to open temp file \"%s\" for script "
                             "compilation.", sc->tmpn);
          }
        create_script_file(ef, sc->tmpn, cd, sc->tmpn_fd);
        snprintf(buf, sizeof(buf),
                 "%s -i %s -o %s %s", embryo_cc_path, inc_path,
                 sc->tmpo, sc->tmpn);
        pending_threads++;
        sc->exe = ecore_exe_run(buf, sc);
        ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                data_scripts_exe_del_cb, sc);
     }
}

#ifdef LUA_BINARY
static int
_edje_lua_script_writer(lua_State *L EINA_UNUSED, const void *chunk_buf, size_t chunk_size, void *_data)
{
   Script_Lua_Writer *data;
   void *old;

   data = (Script_Lua_Writer *)_data;
   old = data->buf;
   data->buf = malloc(data->size + chunk_size);
   memcpy(data->buf, old, data->size);
   memcpy(&((data->buf)[data->size]), chunk_buf, chunk_size);
   if (old) free(old);
   data->size += chunk_size;

   return 0;
}
#endif

void
_edje_lua_error_and_abort(lua_State *L, int err_code, Script_Write *sc)
{
   char buf[PATH_MAX];
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
   snprintf(buf, sizeof(buf),
            "Lua %s error: %s", err_type, lua_tostring(L, -1));
   sc->errstr = strdup(buf);
}

static void
data_thread_lua_script(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Script_Write *sc = data;
   char buf[PATH_MAX];
   lua_State *L;
   int ln = 1;
   luaL_Buffer b;
   Script_Lua_Writer dat;
   Eina_List *ll;
   Code_Program *cp;
#ifdef LUA_BINARY
   int err_code;
#endif

   L = luaL_newstate();
   if (!L)
     {
        snprintf(buf, sizeof(buf),
                 "Lua error: Lua state could not be initialized");
        sc->errstr = strdup(buf);
        return;
     }

   luaL_buffinit(L, &b);

   dat.buf = NULL;
   dat.size = 0;
   if (sc->cd->shared)
     {
        while (ln < (sc->cd->l1 - 1))
          {
             luaL_addchar(&b, '\n');
             ln++;
          }
        luaL_addstring(&b, sc->cd->shared);
        ln += sc->cd->l2 - sc->cd->l1;
     }

   EINA_LIST_FOREACH(sc->cd->programs, ll, cp)
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
     {
        _edje_lua_error_and_abort(L, err_code, sc);
        return;
     }
   lua_dump(L, _edje_lua_script_writer, &dat);
#else // LUA_PLAIN_TEXT
   dat.buf = (char *)lua_tostring(L, -1);
   dat.size = strlen(dat.buf);
#endif
   //printf("lua chunk size: %d\n", dat.size);

   /*
    * TODO load and test Lua chunk
    */

   /*
    if (luaL_loadbuffer(L, globbuf, globbufsize, "edje_lua_script"))
    printf("lua load error: %s\n", lua_tostring (L, -1));
    if (lua_pcall(L, 0, 0, 0))
    printf("lua call error: %s\n", lua_tostring (L, -1));
    */

   snprintf(buf, sizeof(buf), "edje/scripts/lua/%i", sc->i);
   if (eet_write(sc->ef, buf, dat.buf, dat.size, compress_mode) <= 0)
     {
        snprintf(buf, sizeof(buf),
                 "Unable to write script %i", sc->i);
        sc->errstr = strdup(buf);
        return;
     }
#ifdef LUA_BINARY
   free(dat.buf);
#endif
   lua_close(L);
}

static void
data_thread_lua_script_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Script_Write *sc = data;
   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
   if (sc->errstr)
     {
        error_and_abort(sc->ef, sc->errstr);
        free(sc->errstr);
     }
   free(sc);
}

static void
data_write_lua_scripts(Eet_File *ef)
{
   Eina_List *l;
   int i;

   for (i = 0, l = codes; l; l = eina_list_next(l), i++)
     {
        Code *cd;
        Script_Write *sc;

        cd = (Code *)eina_list_data_get(l);
        if (!cd->is_lua)
          continue;
        if ((!cd->shared) && (!cd->programs))
          continue;

        sc = calloc(1, sizeof(Script_Write));
        sc->ef = ef;
        sc->cd = cd;
        sc->i = i;
        pending_threads++;
        if (threads)
          ecore_thread_run(data_thread_lua_script, data_thread_lua_script_end, NULL, sc);
        else
          {
             data_thread_lua_script(sc, NULL);
             data_thread_lua_script_end(sc, NULL);
          }
     }
}

static void
data_thread_source(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eet_File *ef = data;
   source_append(ef);
}

static void
data_thread_source_end(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED)
{
   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
}

static void
data_thread_license(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   License_Write *lw = data;
   Eet_File *ef = lw->ef;
   Eina_File *f;
   void *m;
   int bytes;

   f = eina_file_open(lw->file, 0);
   if (!f) return ;

   m = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!m) goto on_error;

   if (lw->master)
     {
        bytes = eet_write(ef, "edje/license", m, eina_file_size_get(f), compress_mode);
     }
   else
     {
        char *s = alloca(strlen(lw->file) + 1 + 13);

        strcpy(s, lw->file);
        sprintf(s, "edje/license/%s", basename(s));

        bytes = eet_write(ef, s, m, eina_file_size_get(f), compress_mode);
     }

   if ((bytes <= 0) || eina_file_map_faulted(f, m))
     {
        ERR("Unable to write license part \"%s\".", lw->file);
     }
   else
     {
        INF("Wrote %9i bytes (%4iKb) for \"%s\" license entry compress: [real: %2.1f%%]",
            bytes, (bytes + 512) / 1024, license,
            100 - (100 * (double)bytes) / ((double)(eina_file_size_get(f))));
     }

   eina_file_map_free(f, m);

 on_error:
   eina_file_close(f);
}

static void
data_thread_license_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
   free(data);
}

static void
data_write_license(Eet_File *ef)
{
   License_Write *lw;
   Eina_List *l;
   const char *file;

   if (!license) return ;

   lw = calloc(1, sizeof (License_Write));
   if (!lw) return ;

   lw->ef = ef;
   lw->file = license;
   lw->master = EINA_TRUE;

   pending_threads++;
   if (threads)
     ecore_thread_run(data_thread_license, data_thread_license_end, NULL, lw);
   else
     {
        data_thread_license(lw, NULL);
        data_thread_license_end(lw, NULL);
     }

   EINA_LIST_FOREACH(licenses, l, file)
     {
        lw = calloc(1, sizeof (License_Write));
        if (!lw) return ;

        lw->ef = ef;
        lw->file = file;
        lw->master = EINA_FALSE;

        pending_threads++;
        if (threads)
          ecore_thread_run(data_thread_license, data_thread_license_end, NULL, lw);
        else
          {
             data_thread_license(lw, NULL);
             data_thread_license_end(lw, NULL);
          }
     }
}

static void
data_thread_authors(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eet_File *ef = data;
   Eina_File *f;
   void *m;
   int bytes;

   f = eina_file_open(authors, 0);
   if (!f) return ;

   m = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!m) goto on_error;

   bytes = eet_write(ef, "edje/authors", m, eina_file_size_get(f), compress_mode);
   if ((bytes <= 0) || eina_file_map_faulted(f, m))
     {
        ERR("Unable to write license part \"%s\".", authors);
     }
   else
     {
        INF("Wrote %9i bytes (%4iKb) for \"%s\" license entry compress: [real: %2.1f%%]",
            bytes, (bytes + 512) / 1024, license,
            100 - (100 * (double)bytes) / ((double)(eina_file_size_get(f))));
     }

   eina_file_map_free(f, m);

 on_error:
   eina_file_close(f);
}

static void
data_thread_authors_end(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED)
{
   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
}

static void
data_thread_fontmap(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eet_File *ef = data;
   source_fontmap_save(ef, fonts);
}

static void
data_thread_fontmap_end(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED)
{
   pending_threads--;
   if (pending_threads <= 0) ecore_main_loop_quit();
}

void
data_write(void)
{
   Eet_File *ef;
   Eet_Error err;
   int image_num = 0;
   int sound_num = 0;
   int vibration_num = 0;
   int font_num = 0;
   int collection_num = 0;
   double t;

   if (!edje_file)
     {
	ERR("No data to put in \"%s\"", file_out);
	exit(-1);
     }

   ef = eet_open(file_out, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	ERR("Unable to open \"%s\" for writing output", file_out);
	exit(-1);
     }

   check_groups(ef);

   ecore_thread_max_set(ecore_thread_max_get() * 2);

   pending_threads++;
   t = ecore_time_get();
   data_write_header(ef);

   INF("header: %3.5f", ecore_time_get() - t); t = ecore_time_get();
   data_write_groups(ef, &collection_num);
   INF("groups: %3.5f", ecore_time_get() - t); t = ecore_time_get();
   data_write_scripts(ef);
   INF("scripts: %3.5f", ecore_time_get() - t); t = ecore_time_get();
   data_write_lua_scripts(ef);
   INF("lua scripts: %3.5f", ecore_time_get() - t); t = ecore_time_get();

   pending_threads++;
   if (threads)
     ecore_thread_run(data_thread_source, data_thread_source_end, NULL, ef);
   else
     {
        data_thread_source(ef, NULL);
        data_thread_source_end(ef, NULL);
     }
   INF("source: %3.5f", ecore_time_get() - t); t = ecore_time_get();
   pending_threads++;
   if (threads)
     ecore_thread_run(data_thread_fontmap, data_thread_fontmap_end, NULL, ef);
   else
     {
        data_thread_fontmap(ef, NULL);
        data_thread_fontmap_end(ef, NULL);
     }
   INF("fontmap: %3.5f", ecore_time_get() - t); t = ecore_time_get();
   data_write_images(ef, &image_num);
   INF("images: %3.5f", ecore_time_get() - t); t = ecore_time_get();
   data_write_fonts(ef, &font_num);
   INF("fonts: %3.5f", ecore_time_get() - t); t = ecore_time_get();
   data_write_sounds(ef, &sound_num);
   INF("sounds: %3.5f", ecore_time_get() - t); t = ecore_time_get();
   data_write_vibrations(ef, &vibration_num);
   INF("vibrations: %3.5f", ecore_time_get() - t); t = ecore_time_get();
   data_write_license(ef);
   INF("license: %3.5f", ecore_time_get() - t); t = ecore_time_get();
   if (authors)
     {
        pending_threads++;
        if (threads)
          ecore_thread_run(data_thread_authors, data_thread_authors_end, NULL, ef);
        else
          {
             data_thread_authors(ef, NULL);
             data_thread_authors_end(ef, NULL);
          }
     }
   pending_threads--;
   if (pending_threads > 0) ecore_main_loop_begin();
   INF("THREADS: %3.5f", ecore_time_get() - t);

   err = eet_close(ef);
   if (err)
     {
        ERR("Couldn't write file: \"%s\"", file_out);
        exit(-1);
     }

   if (eina_log_domain_level_check(_edje_cc_log_dom, EINA_LOG_LEVEL_INFO))
     {
        printf("Summary:\n"
               "  Wrote %i collections\n"
               "  Wrote %i images\n"
               "  Wrote %i sounds\n"
               "  Wrote %i fonts\n"
               ,
               collection_num,
               image_num,
               sound_num,
               font_num);
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
               ERR("Unable to use together insert_before and insert_after in part \"%s\".", pc->parts[i]->name);

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
                              ERR("The part \"%s\" is ambiguous ordered part.",
                                  pc->parts[i]->name);
                            if (ep2->reorder.linked_prev)
                              ERR("Unable to insert two or more parts in same part \"%s\".",
                                  pc->parts[j]->name);
                            /* Need it to be able to insert an element before the first */
                            if (j == 0) k = 0;
                            else k = j - 1;
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
                              ERR("The part \"%s\" is ambiguous ordered part.", pc->parts[i]->name);
                            if (ep2->reorder.linked_next)
                              ERR("Unable to insert two or more parts in same part \"%s\".", pc->parts[j]->name);
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
                         ERR("The part order is wrong. It has circular dependency.");

                       amount = ep->reorder.linked_prev + ep->reorder.linked_next + 1;
                       linked = i - ep->reorder.linked_prev;
                       parts = malloc(amount * sizeof(Edje_Part *));
                       for (j = 0 ; j < amount ; j++)
                         {
                            parts[j] = pc->parts[linked];
                            linked++;
                         }
                       if (i > k)
                         {
                            for (j = i - ep->reorder.linked_prev ; j > k; j--)
                              {
                                 pc->parts[j + amount - 1] = pc->parts[j - 1];
                                 pc->parts[j + amount - 1]->id = j + amount - 1;
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
data_queue_group_lookup(const char *name, Edje_Part *part)
{
   Group_Lookup *gl;

   if (!name || !name[0]) return;

   gl = mem_alloc(SZ(Group_Lookup));
   group_lookups = eina_list_append(group_lookups, gl);
   gl->name = mem_strdup(name);
   gl->part = part;
}

void
data_queue_face_group_lookup(const char *name)
{
   char *group_name;

   if (!name || !name[0]) return;

   group_name = mem_strdup(name);
   face_group_lookups = eina_list_append(face_group_lookups, group_name);
}

void
data_queue_part_lookup(Edje_Part_Collection *pc, const char *name, int *dest)
{
   Part_Lookup_Key key;
   Part_Lookup *pl = NULL;
   Eina_List *list;

   key.pc = pc;
   key.mem.dest = dest;
   key.stable = EINA_TRUE;

   pl = eina_hash_find(part_pc_dest_lookup, &key);
   if (pl)
     {
        if (name[0])
          {
             free(pl->name);
             pl->name = mem_strdup(name);
          }
        else
          {
             list = eina_hash_find(part_dest_lookup, &pl->key);
             list = eina_list_remove(list, pl);
             eina_hash_set(part_dest_lookup, &pl->key, list);
             eina_hash_del(part_pc_dest_lookup, &key, pl);
          }
        return;
     }

   if (!name[0]) return;

   pl = mem_alloc(SZ(Part_Lookup));
   pl->name = mem_strdup(name);
   pl->key.pc = pc;
   pl->key.mem.dest = dest;
   pl->key.stable = EINA_TRUE;

   eina_hash_add(part_pc_dest_lookup, &key, pl);

   list = eina_hash_find(part_dest_lookup, &pl->key);
   list = eina_list_prepend(list, pl);
   eina_hash_set(part_dest_lookup, &pl->key, list);
}

void
data_queue_part_reallocated_lookup(Edje_Part_Collection *pc, const char *name,
				   unsigned char **base, int offset)
{
   Part_Lookup_Key key;
   Part_Lookup *pl = NULL;
   Eina_List *list;

   key.pc = pc;
   key.mem.reallocated.base = base;
   key.mem.reallocated.offset = offset;
   key.stable = EINA_FALSE;

   pl = eina_hash_find(part_pc_dest_lookup, &key);
   if (pl)
     {
        if (name[0])
          {
             free(pl->name);
             pl->name = mem_strdup(name);
          }
        else
          {
             list = eina_hash_find(part_dest_lookup, &pl->key);
             list = eina_list_remove(list, pl);
             eina_hash_set(part_dest_lookup, &pl->key, list);
             eina_hash_del(part_pc_dest_lookup, &key, pl);
          }
        return;
     }

   if (!name[0]) return;

   pl = mem_alloc(SZ(Part_Lookup));
   pl->name = mem_strdup(name);
   pl->key.pc = pc;
   pl->key.mem.reallocated.base = base;
   pl->key.mem.reallocated.offset = offset;
   pl->key.stable = EINA_FALSE;

   eina_hash_add(part_pc_dest_lookup, &key, pl);

   list = eina_hash_find(part_dest_lookup, &pl->key);
   list = eina_list_prepend(list, pl);
   eina_hash_set(part_dest_lookup, &pl->key, list);
}

void
data_queue_copied_part_lookup(Edje_Part_Collection *pc, int *src, int *dest)
{
   Eina_List *list;
   Eina_List *l;
   Part_Lookup *pl;
   Part_Lookup_Key key;

   key.pc = NULL;
   key.mem.dest = src;
   key.stable = EINA_TRUE;

   list = eina_hash_find(part_dest_lookup, &key);
   EINA_LIST_FOREACH(list, l, pl)
     if (pl->key.stable)
       data_queue_part_lookup(pc, pl->name, dest);
}

void
data_queue_anonymous_lookup(Edje_Part_Collection *pc, Edje_Program *ep, int *dest)
{
   Eina_List *l, *l1, *l2, *l3;
   Program_Lookup *pl;

   if (!ep) return; /* FIXME: should we stop compiling ? */

   EINA_LIST_FOREACH_SAFE(program_lookups, l, l1, pl)
     {
        if (pl->u.ep == ep)
          {
             Code *cd;
             Code_Program *cp;

             cd = eina_list_data_get(eina_list_last(codes));

             EINA_LIST_FOREACH_SAFE(cd->programs, l2, l3, cp)
               {
                  if (&(cp->id) == pl->dest)
                    {
                       cd->programs = eina_list_remove_list(cd->programs, l2);
                       free(cp);
                       cp = NULL;
                    }
               }
             program_lookups = eina_list_remove_list(program_lookups, l);
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
        if (pl->dest == src && pl->u.ep->name)
          {
             for (i = 0 ; i < pc->programs.fnmatch_count ; i++)
               {
                  if (pc->programs.fnmatch[i]->name &&
                      !strcmp(pl->u.ep->name, pc->programs.fnmatch[i]->name))
                    data_queue_anonymous_lookup(pc, pc->programs.fnmatch[i], dest);
               }
             for (i = 0 ; i < pc->programs.strcmp_count ; i++)
               {
                  if (pc->programs.strcmp[i]->name &&
                      !strcmp(pl->u.ep->name, pc->programs.strcmp[i]->name))
                    data_queue_anonymous_lookup(pc, pc->programs.strcmp[i], dest);
               }
             for (i = 0 ; i < pc->programs.strncmp_count ; i++)
               {
                  if (pc->programs.strncmp[i]->name &&
                      !strcmp(pl->u.ep->name, pc->programs.strncmp[i]->name))
                    data_queue_anonymous_lookup(pc, pc->programs.strncmp[i], dest);
               }
             for (i = 0 ; i < pc->programs.strrncmp_count ; i++)
               {
                  if (pc->programs.strrncmp[i]->name &&
                      !strcmp(pl->u.ep->name, pc->programs.strrncmp[i]->name))
                    data_queue_anonymous_lookup(pc, pc->programs.strrncmp[i], dest);
               }
             for (i = 0 ; i < pc->programs.nocmp_count ; i++)
               {
                  if (pc->programs.nocmp[i]->name &&
                      !strcmp(pl->u.ep->name, pc->programs.nocmp[i]->name))
                    data_queue_anonymous_lookup(pc, pc->programs.nocmp[i], dest);
               }
          }
     }
}

void
data_queue_program_lookup(Edje_Part_Collection *pc, const char *name, int *dest)
{
   Program_Lookup *pl;

   if (!name) return; /* FIXME: should we stop compiling ? */

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
data_queue_image_remove(int *dest, Eina_Bool *set)
{
   Eina_List *l;
   Image_Lookup *il;

   EINA_LIST_FOREACH(image_lookups, l, il)
     {
        if (il->dest == dest && il->set == set)
          {
             image_lookups = eina_list_remove_list(image_lookups, l);
             free(il->name);
             free(il);
             return;
          }
     }
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

static void
data_process_part_set(Part_Lookup *target, int value)
{
   if (target->key.stable)
     {
        *(target->key.mem.dest) = value;
     }
   else
     {
        *((int*)(*target->key.mem.reallocated.base +
                 target->key.mem.reallocated.offset)) = value;
     }
}

static void
_data_image_id_update(Eina_List *images_unused_list)
{
   Image_Unused_Ids *iui;
   Edje_Part_Collection *pc;
   Edje_Part *part;
   Edje_Part_Description_Image *part_desc_image;
   Edje_Part_Image_Id *tween_id;
   unsigned int i, j, desc_it;
   Eina_List *l, *l2, *l3;

#define PART_DESC_IMAGE_ID_UPDATE \
   EINA_LIST_FOREACH(images_unused_list, l3, iui) \
     { \
        if (part_desc_image->image.id == iui->old_id) \
          { \
             part_desc_image->image.id = iui->new_id; \
             break; \
          } \
     } \
   for (desc_it = 0; desc_it < part_desc_image->image.tweens_count; desc_it++) \
     { \
        tween_id = part_desc_image->image.tweens[desc_it]; \
        EINA_LIST_FOREACH(images_unused_list, l3, iui) \
          { \
             if (tween_id->id == iui->old_id) \
               { \
                  tween_id->id = iui->new_id; \
                  break; \
               } \
          } \
     }

   EINA_LIST_FOREACH_SAFE(edje_collections, l, l2, pc)
     {
        for(i = 0; i < pc->parts_count; i++)
          {
             part = pc->parts[i];
             if (part->type == EDJE_PART_TYPE_IMAGE)
               {
                  part_desc_image = (Edje_Part_Description_Image *)part->default_desc;
                  PART_DESC_IMAGE_ID_UPDATE
                  for (j = 0; j < part->other.desc_count; j++)
                     {
                        part_desc_image = (Edje_Part_Description_Image *)part->other.desc[j];
                        PART_DESC_IMAGE_ID_UPDATE
                     }
               }
          }
     }
}

void
data_process_lookups(void)
{
   Edje_Part_Collection *pc;
   Eina_Iterator *it;
   Part_Lookup *part;
   Program_Lookup *program;
   Group_Lookup *group;
   Image_Lookup *image;
   Eina_List *l2;
   Eina_List *l;
   Eina_Hash *images_in_use;
   void *data;
   char *group_name;
   Eina_Bool is_lua = EINA_FALSE;
   Image_Unused_Ids *iui;

   /* remove all unreferenced Edje_Part_Collection */
   EINA_LIST_FOREACH_SAFE(edje_collections, l, l2, pc)
     {
        Edje_Part_Collection_Directory_Entry *alias;
        Edje_Part_Collection_Directory_Entry *find;
        Eina_List *l3;
        unsigned int id = 0;
        unsigned int i;

        if (!pc->part)
          {
             ERR("A collection without a name was detected, that's not allowed.");
             exit(-1);
          }
        find = eina_hash_find(edje_file->collection, pc->part);
        if (find && find->id == pc->id)
          continue ;

        EINA_LIST_FOREACH(aliases, l3, alias)
          if (alias->id == pc->id)
            continue ;

        /* This Edje_Part_Collection is not used at all */
        edje_collections = eina_list_remove_list(edje_collections, l);
        l3 = eina_list_nth_list(codes, pc->id);
        codes = eina_list_remove_list(codes, l3);

        /* Unref all image used by that group */
        for (i = 0; i < pc->parts_count; ++i)
	  part_description_image_cleanup(pc->parts[i]);

        /* Correct all id */
        EINA_LIST_FOREACH(edje_collections, l3, pc)
          {
             Eina_List *l4;

             /* Some group could be removed from the collection, but still be referenced by alias */
             find = eina_hash_find(edje_file->collection, pc->part);
             if (pc->id != find->id) find = NULL;

             /* Update all matching alias */
             EINA_LIST_FOREACH(aliases, l4, alias)
               if (pc->id == alias->id)
                 alias->id = id;

             pc->id = id++;
             if (find) find->id = pc->id;
          }
     }

   EINA_LIST_FOREACH(edje_collections, l, pc)
     {
        unsigned int count = 0;
        unsigned int i;

        if (pc->lua_script_only)
          is_lua = EINA_TRUE;
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

   it = eina_hash_iterator_data_new(part_pc_dest_lookup);
   EINA_ITERATOR_FOREACH(it, part)
     {
        Edje_Part *ep;
        unsigned int i;

        if (!strcmp(part->name, "-"))
          {
             data_process_part_set(part, -1);
          }
        else
          {
             char *alias;
             alias = eina_hash_find(part->key.pc->alias, part->name);
             if (!alias)
               alias = part->name;
             for (i = 0; i < part->key.pc->parts_count; ++i)
               {
                  ep = part->key.pc->parts[i];

                  if ((ep->name) && (!strcmp(ep->name, alias)))
                    {
                       int *master;

                       if (part->key.stable)
                         {
                            master = part->key.mem.dest;
                         }
                       else
                         {
                            master = (int*)(*part->key.mem.reallocated.base +
                                            part->key.mem.reallocated.offset);
                         }
                       handle_slave_lookup(part_slave_lookups,
                                           master,
                                           ep->id);
                       data_process_part_set(part, ep->id);
                       break;
                    }
               }

             if (i == part->key.pc->parts_count)
               {
                  ERR("Unable to find part name \"%s\" needed in group '%s'.",
                      alias, part->key.pc->part);
                  exit(-1);
               }
          }
     }
   eina_iterator_free(it);
   eina_hash_free(part_dest_lookup);
   eina_hash_free(part_pc_dest_lookup);

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
               ERR("Unable to find program name \"%s\".",
                   program->u.name);
             else
               ERR("Unable to find anonymous program.");
             exit(-1);
          }

        if (!program->anonymous)
          free(program->u.name);
        free(program);
     }

   EINA_LIST_FREE(group_lookups, group)
     {
        Edje_Part_Collection_Directory_Entry *de;

        if (group->part)
          {
             if (group->part->type != EDJE_PART_TYPE_GROUP
                 && group->part->type != EDJE_PART_TYPE_TEXTBLOCK
                 && group->part->type != EDJE_PART_TYPE_BOX
                 && group->part->type != EDJE_PART_TYPE_TABLE)
               goto free_group;
          }

        de = eina_hash_find(edje_file->collection, group->name);

        if (!de)
          {
             Eina_Bool found = EINA_FALSE;

             EINA_LIST_FOREACH(aliases, l, de)
               if (strcmp(de->entry, group->name) == 0)
                 {
                    found = EINA_TRUE;
                    break;
                 }
             if (!found) de = NULL;
          }

        if (!de)
          {
             ERR("Unable to find group name \"%s\".", group->name);
             exit(-1);
          }

free_group:
        free(group->name);
        free(group);
     }

   EINA_LIST_FREE(face_group_lookups, group_name)
     {
        Edje_Part_Collection_Directory_Entry *de;

        de = eina_hash_find(edje_file->collection, group_name);

        if (!de)
          {
             Eina_Bool found = EINA_FALSE;

             EINA_LIST_FOREACH(aliases, l, de)
               if (strcmp(de->entry, group_name) == 0)
                 {
                    found = EINA_TRUE;
                    break;
                 }
             if (!found) de = NULL;
          }

        if (!de)
          {
             ERR("Unable to find group name \"%s\".", group_name);
             exit(-1);
          }

        free(group_name);
     }

   images_in_use = eina_hash_string_superfast_new(NULL);

   EINA_LIST_FREE(image_lookups, image)
     {
        Eina_Bool find = EINA_FALSE;

        if (edje_file->image_dir)
          {
             Edje_Image_Directory_Entry *de;
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

                       if (!eina_hash_find(images_in_use, image->name))
                         eina_hash_direct_add(images_in_use, de->entry, de);
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
                            Edje_Image_Directory_Set_Entry *child;
                            Eina_List *lc;

                            handle_slave_lookup(image_slave_lookups, image->dest, set->id);
                            *(image->dest) = set->id;
                            *(image->set) = EINA_TRUE;
                            find = EINA_TRUE;

                            EINA_LIST_FOREACH(set->entries, lc, child)
                               if (!eina_hash_find(images_in_use, child->name))
                                 eina_hash_direct_add(images_in_use, child->name, child);

                            if (!eina_hash_find(images_in_use, image->name))
                              eina_hash_direct_add(images_in_use, set->name, set);
                            break;
                         }
                    }
               }
          }

        if (!find)
          {
             ERR("Unable to find image name \"%s\".", image->name);
             exit(-1);
          }

        free(image->name);
        free(image);
     }

   if (edje_file->image_dir && !is_lua)
     {
        Edje_Image_Directory_Entry *de, *de_last, *img;
        Edje_Image_Directory_Set *set;
        Edje_Image_Directory_Set_Entry *set_e;
        Eina_List *images_unused_list = NULL;
        unsigned int i;

        for (i = 0; i < edje_file->image_dir->entries_count; ++i)
          {
             de = edje_file->image_dir->entries + i;

             if (de->entry && eina_hash_find(images_in_use, de->entry))
               continue ;

             INF("Image '%s' in resource 'edje/image/%i' will not be included as it is unused.",
                 de->entry, de->id);

             /* so as not to write the unused images, moved last image in the
                list to unused image position and check it */
             free((void *)de->entry);
             de->entry = NULL;
             de_last = edje_file->image_dir->entries + edje_file->image_dir->entries_count - 1;
             iui = mem_alloc(SZ(Image_Unused_Ids));
             iui->old_id = de_last->id;
             images_unused_list = eina_list_append(images_unused_list, iui);
             iui->new_id = i;
             de_last->id = i;
             memcpy(de, de_last, sizeof (Edje_Image_Directory_Entry));
             --i; /* need to check a moved image on this index */
             edje_file->image_dir->entries_count--;
             img = realloc(edje_file->image_dir->entries,
                           sizeof (Edje_Image_Directory_Entry) * edje_file->image_dir->entries_count);
             edje_file->image_dir->entries = img;
          }

        for (i = 0; i < edje_file->image_dir->sets_count; ++i)
          {
             set = edje_file->image_dir->sets + i;

             if (set->name && eina_hash_find(images_in_use, set->name))
               continue ;

             INF("Set '%s' will not be included as it is unused.", set->name);

             free((void *)set->name);
             EINA_LIST_FREE(set->entries, set_e)
               {
                  free((void *)set_e->name);
                  free(set_e);
               }
          }
        /* update image id in parts */
        if (images_unused_list) _data_image_id_update(images_unused_list);
        EINA_LIST_FREE(images_unused_list, iui)
           free(iui);
     }

   eina_hash_free(images_in_use);

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

   keyl = strlen(prefix) + 2;
   key = alloca(keyl + 1);
   if (!key) return;
   strcpy(key, prefix);
   strcat(key, ":\"");
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
_data_queue_group_lookup(Edje_Part_Collection *pc EINA_UNUSED, char *name, char *ptr EINA_UNUSED, int len EINA_UNUSED)
{
   data_queue_group_lookup(name, NULL);
}
static void
_data_queue_image_pc_lookup(Edje_Part_Collection *pc EINA_UNUSED, char *name, char *ptr, int len)
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
	     ERR("The unexpected happened. A numeric replacement string was larger than the original!");
	     exit(-1);
	  }
	memset(cl->ptr, ' ', cl->len);
	strncpy(cl->ptr, buf, n);
     }
}

void
using_file(const char *filename, const char type)
{
   FILE *f;

   if (!watchfile) return;
   f = fopen(watchfile, "ab");
   if (!f) return;
   if (anotate)
     {
       fprintf(f, "%c: %s\n", type, filename);
     }
   else
     {
       fputs(filename, f);
       fputc('\n', f);
     }
   fclose(f);
}
